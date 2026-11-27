#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "Query.h"

/**
 * @class Connection
 * @brief One client connection of the daemon: reads serialized ValueTree messages from the client and sends each result back.
 *
 * Deserializes incoming binary ValueTree blocks for connection metadata, request
 * cancellation, and queued request rows under its Id::connection row in the model.
 * When a request row receives Id::result, sends the binary JSON-RPC payload to the
 * client and removes the row.
 */
class Connection : public juce::InterprocessConnection,
                   private juce::ValueTree::Listener
{
public:
    /** @brief Callback attaching this connection to a workspace when its manifest message arrives. */
    using Attach = std::function<void (Connection&, juce::ValueTree&, const juce::File&, const juce::String&)>;
    /** @brief Callback detaching this connection from its workspace when the socket disconnects. */
    using Detach = std::function<void (Connection&, juce::ValueTree&)>;

    /**
     * @brief Creates a connection that makes its callbacks on the message thread, and stores its owner and attach/detach callbacks.
     *
     * @param newConnections  The owner of the connections. A lost connection removes itself from it.
     * @param newOnAttach     Callback invoked when the client sends its manifest and toolchain argument.
     * @param newOnDetach     Callback invoked when the connection is lost.
     */
    Connection (jam::Owner<Connection>& newConnections, Attach newOnAttach, Detach newOnDetach);

    /**
     * @brief Removes the listener, and disconnects before the members are destroyed.
     *
     * The JUCE base class requires a derived connection to disconnect in its destructor.
     */
    ~Connection() override;

    /**
     * @brief Tells whether this connection's state tree is currently parented under @p connectionsTree.
     *
     * @param connectionsTree  Workspace connections ValueTree to test membership against.
     * @return @c true when @c state is a direct child of @p connectionsTree.
     */
    bool isIn (const juce::ValueTree& connectionsTree) const;

private:
    /**
     * @brief Creates the connection @c state tree and listens to it.
     */
    void connectionMade() override;

    /**
     * @brief Invokes @c onDetach to leave the workspace, then removes this object from @c connections.
     *
     * The owner destroys the object during this call.
     */
    void connectionLost() override;

    /**
     * @brief Deserializes a client message ValueTree and updates connection metadata, cancels a request, or queues a request row.
     *
     * An Id::connection tree copies its properties onto @c state. An Id::cancel tree
     * finds the matching pending request child by Id::id and marks it RequestCancelled.
     * Any other tree (an Id::request) is appended as a child of @c state.
     *
     * @param message  The serialized juce::ValueTree bytes of one client message.
     */
    void messageReceived (const juce::MemoryBlock& message) override;

    /**
     * @brief Sends the binary JSON-RPC result of an Id::request row to the client, then removes the row.
     *
     * Only a change of Id::result on an Id::request row acts.
     *
     * @param tree      The tree whose property changed. Only an Id::request row acts.
     * @param property  The changed property. Only Id::result acts.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /** @brief The owner of the connections of the server. This connection removes itself from it when the client is lost. */
    jam::Owner<Connection>& connections;

    /** @brief Callback invoked once on attachment to register this connection with a workspace. */
    Attach onAttach;
    /** @brief Callback invoked in connectionLost() to remove this connection from its workspace. */
    Detach onDetach;
    /** @brief Connection ValueTree holding client metadata and queued Id::request children. */
    juce::ValueTree state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};
