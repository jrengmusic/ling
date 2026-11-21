#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Connection.h"
#include "Query.h"

/**
 * @class Server
 * @brief The socket server of the daemon: it accepts the connections, writes its port file, and ends the daemon after the timeout.
 *
 * The server listens on a port that the system chooses. On the change of the
 * documents count, it writes the bound port to the port file of the manifest, and
 * it starts the timeout. Each request restarts the timeout. When the timeout ends,
 * the server deletes the port file and quits the application.
 */
class Server : public juce::InterprocessConnectionServer,
               private juce::Timer,
               private juce::ValueTree::Listener,
               private jam::File::Watcher::Listener
{
public:
    /** @brief Listens to the model, so that the change of the documents count reaches the server. */
    Server();

    /** @brief Stops the socket, and removes the server from the listeners of the model and of the connections child. */
    ~Server() override;

    /**
     * @brief Listens for connections on a port that the system chooses, on the local address of this machine.
     *
     * A jassert checks that the socket listens. The bound port is read later, on
     * the change of the documents count.
     */
    void start();

    /**
     * @brief Returns the port file of a manifest and a toolchain argument.
     *
     * The port file is in the cache folder. Its name is the daemon name of the
     * manifest and the toolchain argument, with the port file extension.
     *
     * @param manifest           The manifest file.
     * @param toolchainArgument  The toolchain argument. Empty when none is given.
     * @return The port file.
     */
    static juce::File getPortFile (const juce::File& manifest, std::string_view toolchainArgument);

    static constexpr int noPort { -1 };

private:
    /**
     * @brief Creates a Connection for an accepted socket.
     *
     * The function returns a raw pointer to the connection for the socket server.
     * It moves the owning pointer to the connections owner on the message thread.
     *
     * @return The connection that the socket server runs.
     */
    juce::InterprocessConnection* createConnectionObject() override;

    /**
     * @brief On the files child added to the state of the model, writes the port file, starts the timeout, and listens to the connections child. On a request row below the connections child, stops the timeout timer.
     *
     * The files child writes the bound port to the port file in the cache folder,
     * and starts the timeout timer. It then removes the server from the listeners
     * of the model, and adds it to the listeners of the connections child.
     * The listener receives the child additions of its tree and of every subtree.
     *
     * @param parentTree              The tree that received the child. A files child acts only when it is the state of the model.
     * @param childWhichHasBeenAdded  The added child. Only a files child and a request row act.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    void valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;

    /**
     * @brief Stops the socket, deletes the port file, and quits the application.
     *
     * The timer ends the daemon when no request arrives within the timeout.
     */
    void timerCallback() override;

    void fileChanged (const juce::File& file, jam::File::Watcher::Event event) override;

    /**
     * @brief Returns the port file of the manifest and the toolchain argument in the model.
     *
     * @return The port file.
     */
    juce::File getPortFile() const;

    static juce::MemoryBlock getProgress (std::string_view progressType, int pendingJobs, int totalJobs);

    /** @brief The port number that asks the system for a free port. */
    static constexpr int anyPort { 0 };

    /** @brief The model of the run. The server reads its manifest and toolchain argument from it. */
    LINGModel& model { *LINGModel::getInstance() };

    /** @brief The connections child of the model. The server listens to it after the documents count changes. */
    juce::ValueTree state { model.state.getChildWithName (jam::Format::toType (Id::connections)) };

    /** @brief The open connections. A connection that the client loses removes itself from them. */
    jam::Owner<Connection> connections;

    jam::File::Watcher watcher;

    /** @brief The attachment that sends each change of the overlay message to every open connection, as JSON text. */
    jam::Model::ParameterAttachment message {*model.getParameter<jam::ParameterText> (jam::Format::toType (Id::overlay), Id::message),
                                              [this] (const juce::var& text)
                                              {
                                                  const auto& messageText { text.toString() };
                                                  juce::MemoryBlock json;

                                                  jam::JsonWriter::appendObject (json, [&messageText] (juce::MemoryBlock& notification)
                                                  {
                                                      Query::appendJsonRpc (notification);

                                                      jam::JsonWriter::appendKey (notification, lsp::Member::method);
                                                      jam::JsonWriter::appendString (notification, lsp::Method::showMessage);

                                                      jam::JsonWriter::appendKey (notification, lsp::Member::params);
                                                      jam::JsonWriter::appendObject (notification, [&messageText] (juce::MemoryBlock& params)
                                                      {
                                                          jam::JsonWriter::appendKey (params, lsp::Member::messageType);
                                                          jam::Format::appendDecimal (params, lsp::MessageType::Error);

                                                          jam::JsonWriter::appendKey (params, lsp::Member::message);
                                                          jam::JsonWriter::appendString (params, jam::Strings::getView (messageText), juce::JSON::Encoding::utf8);
                                                      });
                                                  });

                                                  for (const auto& connection : connections)
                                                      connection->sendMessage (json);
                                              } };

    jam::Model::ParameterAttachment progress {*model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::pendingJobCount),
                                              [this] (const juce::var& pendingValue)
                                              {
                                                  const auto pendingJobs { static_cast<int> (pendingValue) };
                                                  const auto totalJobs { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::jobCount)->getValue() };
                                                  const auto progressType { pendingJobs == 0 ? lsp::WorkDoneProgressKind::End : pendingJobs == totalJobs ? lsp::WorkDoneProgressKind::Begin : lsp::WorkDoneProgressKind::Report };
                                                  const auto json { getProgress (progressType, pendingJobs, totalJobs) };

                                                  for (const auto& connection : connections)
                                                      connection->sendMessage (json);
                                              } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Server)
};
