#pragma once
#include "Client.h"

/**
 * @class Operation
 * @brief Single-shot daemon client that sends one serialized request, prints the JSON-RPC reply to stdout, and exits.
 */
class Operation : public Client,
                  private juce::ValueTree::Listener
{
public:
    /**
     * @brief Stores the serialized request payload and listens to state, the connection state of this client, for Server::noPort.
     *
     * @param newManifestFile       The manifest identifying the target daemon.
     * @param newToolchainArgument  The toolchain argument, or empty when none is given.
     * @param newRequest            Serialized Id::request ValueTree bytes moved into the client.
     */
    Operation (const juce::File& newManifestFile, const juce::String& newToolchainArgument, juce::MemoryBlock&& newRequest);

    /** @brief Removes the listener from state and disconnects from the daemon socket. */
    ~Operation() override;

private:
    /** @brief Sends the serialized @c request block to the daemon once the socket connects. */
    void connectionMade() override;

    /**
     * @brief Handles daemon JSON-RPC frames: forwards showMessage notifications to the model, or prints the final reply and exits.
     *
     * @param reply  JSON-RPC message bytes received from the daemon.
     */
    void messageReceived (const juce::MemoryBlock& reply) override;

    /**
     * @brief Prints a JSON-RPC object with a null id and a null result, and quits with success, when Id::port of state becomes Server::noPort.
     *
     * @param tree      The ValueTree whose property changed.
     * @param property  The changed property identifier.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    juce::MemoryBlock request;///< Serialized Id::request ValueTree bytes sent on connection.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Operation)
};
