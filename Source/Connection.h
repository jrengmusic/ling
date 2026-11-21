#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
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
    /**
     * @brief Creates a connection that makes its callbacks on the message thread, and stores its owner.
     *
     * @param newConnections  The owner of the connections. A lost connection removes itself from it.
     */
    explicit Connection (jam::Owner<Connection>& newConnections);

    /**
     * @brief Removes the listener, and disconnects before the members are destroyed.
     *
     * The JUCE base class requires a derived connection to disconnect in its destructor.
     */
    ~Connection() override;

private:
    /**
     * @brief Creates the connection row, listens to it, and appends it to the connections child of the model.
     */
    void connectionMade() override;

    /**
     * @brief Removes the connection row from the model, and removes this object from the connections owner.
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

    /** @brief The model of the run. The connection writes its rows to the connections child. */
    LINGModel& model { *LINGModel::getInstance() };

    /** @brief The connection row of this connection, in the connections child. Request rows are appended below it. */
    juce::ValueTree state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
};
