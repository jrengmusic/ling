#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"

/**
 * @class Connection
 * @brief One client connection of the daemon: it reads the requests of the client, and sends each result back.
 *
 * A client message holds a JSON array: the command, the working folder, and the
 * arguments. The connection appends a request row for it below its connection row
 * in the model. When the request row gets its result, the connection sends the
 * result to the client, and it removes the request row.
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
     * @brief Turns one client message into a request row below the connection row.
     *
     * The message is a JSON array. Index 0 is the command, index 1 is the working
     * folder, and index 2 onward are the arguments. A message with fewer than two
     * elements adds no request row. The function asserts that the message is an
     * array of at least two elements.
     *
     * @param message  The bytes of one client message.
     */
    void messageReceived (const juce::MemoryBlock& message) override;

    /**
     * @brief Sends the result of a request row to the client, then removes the row.
     *
     * Only a change of Id::result on a request row acts.
     *
     * @param tree      The tree whose property changed. Only a request row acts.
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
