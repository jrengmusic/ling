#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Server.h"

/**
 * @class Client
 * @brief Base socket client connecting to the daemon of a manifest, or launching one when absent.
 *
 * Connects to the daemon through the port file. When no daemon answers, it starts
 * one as a detached process and watches the cache folder so it connects when the
 * port file appears.
 */
class Client : public juce::InterprocessConnection,
               private jam::File::Watcher::Listener
{
public:
    /**
     * @brief Stores the model, manifest, and toolchain argument, creates the port directory, and watches it for the port file.
     *
     * The constructor also appends state to the connections child of @p newModel.state.
     *
     * @param newModel              The model whose connections and overlay state this client updates.
     * @param newManifestFile       The manifest of the run.
     * @param newToolchainArgument  The toolchain argument. Empty when none is given.
     */
    Client (LINGModel& newModel, const juce::File& newManifestFile, const juce::String& newToolchainArgument);

    /** @brief Removes the client from the listeners of the watcher, and removes state from its parent. */
    ~Client() override;

    /**
     * @brief Connects to the daemon, or starts one when no daemon answers.
     *
     * A started daemon receives the executable path and @c Id::daemon. When the
     * process does not start, the function sets the message, sets the exit code to
     * failure, and quits.
     */
    void start();

private:
    /**
     * @brief Connects to the port that the port file holds.
     *
     * A connection that succeeds removes the client from the listeners of the watcher.
     *
     * @return @c true when the connection succeeds.
     */
    bool connectToDaemon();

    /**
     * @brief Records Id::port on state and tries to connect when the port file changes.
     *
     * A change of any other file does nothing.
     *
     * @param file   The changed file.
     * @param event  The kind of change. Not read.
     */
    void fileChanged (const juce::File& file, jam::File::Watcher::Event event) override;

    /**
     * @brief Sets the failure message, sets the exit code to failure, and quits, when the daemon connection ends.
     */
    void connectionLost() override;

protected:
    /**
     * @brief Sends @c Id::manifest and @c Id::toolchainArgument, then flushes all queued rows in @c requests.
     */
    void connectionMade() override;
    /**
     * @brief Sends a request row to the daemon connection, and removes the row from the requests.
     *
     * The row is written to a memory block as a tree, and the block is sent as one message.
     *
     * @param row  The request row to send.
     */
    void sendRequest (const juce::ValueTree& row);

    /**
     * @brief Forwards a showMessage notification to model.setMessage(). Other methods are ignored.
     *
     * @param document  The parsed JSON-RPC message.
     * @param root      The root element of the message.
     */
    void setMessage (const jam::JsonDocument& document, const jam::Document::Element& root);

    /** @brief The manifest of the run. Sent to the daemon when the connection opens. */
    const juce::File manifestFile;

    /** @brief The model of the run. Holds overlay messages. */
    LINGModel& model;

    /**
     * @brief The connection state of this client. It is a child of the connections child of model.state.
     *
     * Id::port is set from the port file when the file changes.
     */
    juce::ValueTree state { jam::Format::toType (Id::connection) };

    /**
     * @brief The request rows waiting for the daemon connection. Each row is a child of state.
     *
     * connectionMade() sends the rows, and each row leaves the list when it is sent.
     */
    juce::ValueTree requests { state.getOrCreateChildWithName (jam::Format::toType (Id::requests), nullptr) };

    /** @brief Timeout in milliseconds passed to disconnect() to wait for socket teardown. */
    static constexpr int waitForDisconnect { -1 };

private:
    /** @brief The toolchain argument of the run. Empty when none is given. */
    const juce::String toolchainArgument;

    /** @brief The file watcher of the cache folder. It reports the change of the port file. */
    jam::File::Watcher watcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Client)
};
