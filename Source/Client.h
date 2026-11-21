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
     * @brief Stores the manifest and toolchain argument, creates the port directory, and watches it for the port file.
     *
     * @param newManifestFile       The manifest of the run.
     * @param newToolchainArgument  The toolchain argument. Empty when none is given.
     */
    Client (const juce::File& newManifestFile, const juce::String& newToolchainArgument);

    /** @brief Removes the client from the listeners of the watcher. */
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
     * @brief Records Id::port on model.state and tries to connect when the port file changes.
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

    /** @brief The manifest of the run. Its path names the port file. */
    const juce::File manifestFile;

    /** @brief The toolchain argument of the run. Empty when none is given. */
    const juce::String toolchainArgument;

protected:
    /** @brief The model of the run. Holds overlay messages and the daemon port property. */
    LINGModel& model { *LINGModel::getInstance() };

private:
    /** @brief The file watcher of the cache folder. It reports the change of the port file. */
    jam::File::Watcher watcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Client)
};
