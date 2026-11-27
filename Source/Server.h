#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Connection.h"
#include "Manifest.h"
#include "Processor.h"
#include "Query.h"

/**
 * @class Server
 * @brief Multi-workspace daemon socket server sharing one thread pool across active projects.
 *
 * Listens on a loopback port and writes that port to getPortFile(). Routes
 * incoming connections to a Workspace keyed by project name and broadcasts
 * workspace notifications. Stops the socket, deletes the port file, and quits
 * when idle or when the executable changes on disk.
 */
class Server : public juce::InterprocessConnectionServer,
               private juce::Timer,
               private jam::File::Watcher::Listener
{
public:
    /** @brief Watches the executable folder for binary changes and registers this server as its listener. */
    Server();

    /** @brief Removes the watcher listener and stops the socket. */
    ~Server() override;

    /**
     * @brief Starts listening on a loopback port, writes the port file, and starts the idle timer.
     *
     * Asserts that the socket listens on @c anyPort, writes the bound port to
     * getPortFile(), and starts the timer with @c LINGModel::defaultTimeoutMs.
     */
    void start();

    /**
     * @brief Returns the daemon port file path for the current executable.
     *
     * Places the file inside the base cache directory using
     * @c LINGModel::getDaemonName() and the port file extension.
     *
     * @return Daemon port file path.
     */
    static juce::File getPortFile();

    /** @brief Sentinel written to the port file when the manifest yields zero files. */
    static constexpr int noPort { -1 };
    /** @brief JSON-RPC notification method sent when a workspace discovers zero build files. */
    static constexpr std::string_view noPortMethod { "ling/noPort" };

private:
    /**
     * @class Workspace
     * @brief Owns one project workspace with its model, processor, manifest, and client connections.
     *
     * Starts an idle timer when its last connection leaves or when empty files
     * arrive without connections. Notifies @c owner to erase this workspace when
     * the timer expires.
     */
    class Workspace : private juce::Timer,
                      private juce::ValueTree::Listener
    {
    public:
        /**
         * @brief Creates a workspace for @p manifestFile, attaches overlay parameters, and loads the manifest.
         *
         * @param newOwner           Parent server owning this workspace.
         * @param newKey             Project key identifying this workspace.
         * @param manifestFile       Manifest file of the workspace.
         * @param toolchainArgument  Toolchain flag, or empty when none is given.
         */
        Workspace (Server& newOwner, const juce::String& newKey, const juce::File& manifestFile, const juce::String& toolchainArgument);
        /** @brief Removes this workspace from its model listeners. */
        ~Workspace() override;

        /**
         * @brief Tells whether @p connectionState belongs to this workspace.
         *
         * @param connectionState  Connection ValueTree to test.
         * @return @c true when @p connectionState is a child of @c state.
         */
        bool hasConnection (const juce::ValueTree& connectionState) const;
        /**
         * @brief Attaches @p connection to this workspace and sends current progress or no-port status.
         *
         * Stops the idle timer and appends @p connectionState to @c state. Sends
         * @c noPortMethod when files are empty, or sends active job progress.
         *
         * @param connection       Client connection to attach.
         * @param connectionState  ValueTree representing the connection.
         */
        void addConnection (Connection& connection, juce::ValueTree& connectionState);
        /**
         * @brief Detaches @p connectionState from this workspace and starts the idle timer when empty.
         *
         * @param connectionState  Connection ValueTree to remove.
         */
        void removeConnection (juce::ValueTree& connectionState);

    private:
        /**
         * @brief Tells whether the files tree exists with build files and zero file rows.
         *
         * @return @c true when files exist with @c Id::buildFiles and zero children.
         */
        bool hasEmptyFiles() const;
        /**
         * @brief Broadcasts no-port status and starts the idle timer when an empty files tree arrives.
         *
         * @param parentTree              Parent tree receiving the child.
         * @param childWhichHasBeenAdded  Added child tree.
         */
        void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
        /** @brief Requests @c owner to remove this idle workspace. */
        void timerCallback() override;

        /** @brief Parent server that owns this workspace and broadcasts notifications. */
        Server& owner;
        /** @brief Project key identifying this workspace in @c owner. */
        const juce::String key;
        /** @brief Shared state model for this workspace. */
        LINGModel model;
        /** @brief Runs parse, index, and query jobs on @c owner.pool. */
        Processor processor { model, owner.pool };
        /** @brief Loads and reloads the workspace manifest into @c model. */
        Manifest manifest { model };
        /** @brief Connections child tree of @c model.state holding attached client rows. */
        juce::ValueTree state { model.state.getChildWithName (jam::Format::toType (Id::connections)) };
        /** @brief Parameter attachment broadcasting overlay error messages as JSON-RPC notifications. */
        jam::Model::ParameterAttachment message;
        /** @brief Parameter attachment broadcasting pending job counts as JSON-RPC progress notifications. */
        jam::Model::ParameterAttachment progress;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Workspace)
    };

    /**
     * @brief Creates a Connection with workspace callbacks and schedules ownership transfer to @c connections.
     *
     * @return Raw pointer to the newly created Connection.
     */
    juce::InterprocessConnection* createConnectionObject() override;
    /** @brief Stops the socket, deletes the port file, stops the timer, and quits the daemon. */
    void timerCallback() override;
    /**
     * @brief Triggers daemon shutdown when the running executable changes on disk.
     *
     * @param file   Changed file in the executable folder.
     * @param event  File watcher event kind (unused).
     */
    void fileChanged (const juce::File& file, jam::File::Watcher::Event event) override;
    /**
     * @brief Stops the idle timer, finds or creates the workspace for @p manifestFile, and attaches @p connection.
     *
     * @param connection         Client connection to attach.
     * @param connectionState    ValueTree representing the connection.
     * @param manifestFile       Manifest file identifying the workspace.
     * @param toolchainArgument  Toolchain flag, or empty when none is given.
     */
    void attachConnection (Connection& connection, juce::ValueTree& connectionState, const juce::File& manifestFile, const juce::String& toolchainArgument);
    /**
     * @brief Removes @p connectionState from whichever workspace owns it.
     *
     * @param connectionState  Connection ValueTree to detach.
     */
    void detachConnection (juce::ValueTree& connectionState);
    /**
     * @brief Erases the workspace for @p key and quits or restarts the idle timer when empty.
     *
     * @param key  Project key of the workspace to remove.
     */
    void removeWorkspace (const juce::String& key);
    /**
     * @brief Sends @p message to every connection attached to @p workspaceConnections.
     *
     * @param workspaceConnections  Workspace connections tree filtering target clients.
     * @param message               Serialized JSON-RPC notification payload.
     */
    void broadcast (const juce::ValueTree& workspaceConnections, const juce::MemoryBlock& message);

    /**
     * @brief Serializes a JSON-RPC work-done progress notification.
     *
     * @param progressType  LSP progress kind (begin, report, or end).
     * @param pendingJobs   Number of remaining jobs.
     * @param totalJobs     Total job count for the current batch.
     * @return Serialized JSON-RPC notification block.
     */
    static juce::MemoryBlock getProgress (std::string_view progressType, int pendingJobs, int totalJobs);
    /**
     * @brief Serializes the JSON-RPC @c noPortMethod notification.
     *
     * @return Serialized JSON-RPC notification block.
     */
    static juce::MemoryBlock getNoPortNotification();

    /** @brief Port value passed to beginWaitingForSocket() so the OS assigns a free loopback port. */
    static constexpr int anyPort { 0 };

    /** @brief Shared thread pool passed to every Workspace processor. */
    std::shared_ptr<juce::ThreadPool> pool { std::make_shared<juce::ThreadPool> (Processor::getPoolOptions (LINGModel::getDefaultThreads())) };
    /** @brief Active workspaces keyed by project name. */
    jam::HashMap<juce::String, std::unique_ptr<Workspace>> workspaces;
    /** @brief Owns all active client Connection objects. */
    jam::Owner<Connection> connections;
    /** @brief Watches the executable folder so the daemon exits when its binary updates. */
    jam::File::Watcher watcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Server)
};
