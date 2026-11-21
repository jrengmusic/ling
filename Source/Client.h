#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Server.h"

/**
 * @class Client
 * @brief Sends one request to the daemon of a manifest, and prints the reply.
 *
 * The client connects to the daemon through the port file. When no daemon
 * answers, it starts one as a detached process. It watches the cache folder, so
 * that it connects when the port file appears.
 */
class Client : public juce::InterprocessConnection,
               private jam::File::Watcher::Listener
{
public:
    /**
     * @brief Stores the manifest, the toolchain argument, and the request, and watches the cache folder for the port file.
     *
     * The constructor creates the cache folder.
     *
     * @param newManifestFile       The manifest of the request.
     * @param newToolchainArgument  The toolchain argument. Empty when none is given.
     */
    Client (const juce::File& newManifestFile, const juce::String& newToolchainArgument);

    /** @brief Removes the client from the listeners of the watcher, and disconnects. */
    ~Client() override;

    /**
     * @brief Connects to the daemon, or starts one when no daemon answers.
     *
     * A started daemon receives the daemon word, the manifest path, and the toolchain
     * argument flag when one is set. When the process does not start, the function
     * sets the message, sets the exit code to failure, and quits.
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
     * @brief Tries to connect when the port file changes.
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

    /** @brief The longest wait, in milliseconds, for one connection attempt. */
    static constexpr int connectTimeoutMs { 200 };

    /** @brief The manifest of the request. Its path names the port file. */
    const juce::File manifestFile;

    /** @brief The toolchain argument of the request. Empty when none is given. */
    const juce::String toolchainArgument;

protected:
    /** @brief The model of the run. The client writes the overlay messages to it. */
    LINGModel& model { *LINGModel::getInstance() };

private:
    /** @brief The file watcher of the cache folder. It reports the change of the port file. */
    jam::File::Watcher watcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Client)
};
