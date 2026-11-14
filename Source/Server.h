#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Connection.h"

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
               private juce::ValueTree::Listener
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
     * @brief On the change of the documents count, writes the port file, starts the timeout, and listens to the connections child.
     *
     * The function creates the cache folder, writes the bound port to the port
     * file, and starts the timeout timer. It then removes the server from the
     * listeners of the model, and adds it to the listeners of the connections child.
     *
     * @param tree      The tree whose property changed. Only the state tree of the model acts.
     * @param property  The changed property. Only Id::documents acts.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Restarts the timeout timer when a request row is added below the connections child.
     *
     * The listener receives the child additions of its tree and of every subtree.
     * A request row added below a connection row also restarts the timer.
     *
     * @param parentTree              The tree that received the child. Not read.
     * @param childWhichHasBeenAdded  The added child. Only a request row acts.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    void valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;

    /**
     * @brief Stops the socket, deletes the port file, and quits the application.
     *
     * The timer ends the daemon when no request arrives within the timeout.
     */
    void timerCallback() override;

    /**
     * @brief Returns the port file of the manifest and the toolchain argument in the model.
     *
     * @return The port file.
     */
    juce::File getPortFile() const;

    /** @brief The port number that asks the system for a free port. */
    static constexpr int anyPort { 0 };

    /** @brief The model of the run. The server reads its manifest and toolchain argument from it. */
    LINGModel& model { *LINGModel::getInstance() };

    /** @brief The connections child of the model. The server listens to it after the documents count changes. */
    juce::ValueTree state { model.state.getChildWithName (jam::Format::toType (Id::connections)) };

    /** @brief The open connections. A connection that the client loses removes itself from them. */
    jam::Owner<Connection> connections;

    /** @brief The attachment that sends each change of the overlay message to every open connection, as JSON text. */
    jam::Model::ParameterAttachment message {*model.getParameter<jam::ParameterText> (jam::Format::toType (Id::overlay), Id::message),
                                              [this] (const juce::var& text)
                                              {
                                                  std::string json;

                                                  jam::JsonWriter::appendString (json, jam::Strings::getView (text.toString()), juce::JSON::Encoding::utf8);

                                                  for (const auto& connection : connections)
                                                      connection->sendMessage (juce::MemoryBlock { json.data(), json.size() });
                                              } };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Server)
};
