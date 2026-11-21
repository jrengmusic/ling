#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Connection.h"
#include "Query.h"

/**
 * @class Server
 * @brief Socket server of the daemon: accepts client connections, writes the port file, broadcasts notifications, and exits when idle or replaced.
 *
 * Listens on a system-assigned loopback port. When the files child is added to
 * the model, writes the bound port (or noPort if empty) to the port file and
 * starts the idle timer when no connections are open. A connection added to
 * Id::connections stops the idle timer, and removing the last connection restarts
 * it. When the timer expires or the running executable changes on disk, the server
 * stops the socket, deletes the port file, and quits the application.
 */
class Server : public juce::InterprocessConnectionServer,
               private juce::Timer,
               private juce::ValueTree::Listener,
               private jam::File::Watcher::Listener
{
public:
    /** @brief Registers message and progress attachments, starts watching the executable directory, and listens to the model. */
    Server();

    /** @brief Removes the watcher listener, stops the socket, and removes the model and state listeners. */
    ~Server() override;

    /**
     * @brief Starts listening for connections on a system-assigned loopback port.
     *
     * Asserts that the socket begins listening. The bound port is written to the
     * port file later, when the files child is added to the model.
     */
    void start();

    /**
     * @brief Returns the port file for @p manifest and @p toolchainArgument.
     *
     * Places the file inside the project cache directory using the daemon name
     * and the port file extension.
     *
     * @param manifest           Manifest file identifying the project.
     * @param toolchainArgument  Toolchain flag, or empty when none is given.
     * @return Port file path.
     */
    static juce::File getPortFile (const juce::File& manifest, std::string_view toolchainArgument);

    /** @brief Sentinel written to the port file when the manifest yields zero files. */
    static constexpr int noPort { -1 };

private:
    /**
     * @brief Creates a Connection for an accepted client socket.
     *
     * Returns a raw pointer required by juce::InterprocessConnectionServer and
     * transfers ownership to @c connections asynchronously on the message thread.
     *
     * @return Newly created Connection owned by @c connections.
     */
    juce::InterprocessConnection* createConnectionObject() override;

    /**
     * @brief Handles added files and connection rows to manage the port file, idle timer, and initial progress notification.
     *
     * When Id::files is added to @c model.state, creates the port file directory;
     * if empty, writes @c noPort, stops, and quits; otherwise writes @c getBoundPort(),
     * starts the idle timer if @c state has no connections, stops listening to @c model,
     * and listens to @c state. When a connection child is added to @c state, stops
     * the idle timer and broadcasts a @c Begin progress notification if jobs are pending.
     *
     * @param parentTree              Tree receiving the child (@c model.state or @c state).
     * @param childWhichHasBeenAdded  Added files or connection child tree.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Restarts the idle timer when the last connection leaves @c state.
     *
     * @param parentTree                    Parent tree whose child was removed.
     * @param childWhichHasBeenRemoved      Removed child tree (unused).
     * @param indexFromWhichChildWasRemoved Former index of the removed child (unused).
     */
    void valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;

    /**
     * @brief Stops the socket, deletes the port file, stops the timer, and quits the application.
     */
    void timerCallback() override;

    /**
     * @brief Triggers daemon shutdown via timerCallback() when the running executable changes on disk.
     *
     * @param file   File reported by the directory watcher.
     * @param event  File watcher event kind (unused).
     */
    void fileChanged (const juce::File& file, jam::File::Watcher::Event event) override;

    /**
     * @brief Returns the port file for the manifest and toolchain argument in @c model.
     *
     * @return Port file path.
     */
    juce::File getPortFile() const;

    /**
     * @brief Builds an LSP @c $/progress JSON-RPC notification payload.
     *
     * @param progressType  LSP WorkDoneProgressKind string (@c begin, @c report, or @c end).
     * @param pendingJobs   Number of jobs still running or queued.
     * @param totalJobs     High-water job count for the current indexing batch.
     * @return Serialized JSON-RPC notification bytes.
     */
    static juce::MemoryBlock getProgress (std::string_view progressType, int pendingJobs, int totalJobs);

    /** @brief Port number asking the OS to bind an ephemeral port. */
    static constexpr int anyPort { 0 };

    /** @brief Shared model for this run; provides manifest, toolchain argument, timeout, and overlay parameters. */
    LINGModel& model { *LINGModel::getInstance() };

    /** @brief Connections subtree of @c model.state watched after the files child is added. */
    juce::ValueTree state { model.state.getChildWithName (jam::Format::toType (Id::connections)) };

    /** @brief Owns all active client Connection instances; lost connections remove themselves. */
    jam::Owner<Connection> connections;

    /** @brief Watches the executable's directory so replacing the binary shuts down the stale daemon. */
    jam::File::Watcher watcher;

    /** @brief Parameter attachment broadcasting overlay error messages to all open connections as JSON-RPC notifications. */
    jam::Model::ParameterAttachment message;

    /** @brief Parameter attachment broadcasting indexing job progress updates to all open connections. */
    jam::Model::ParameterAttachment progress;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Server)
};
