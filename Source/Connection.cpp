#include "Connection.h"

Connection::Connection (jam::Owner<Connection>& newConnections)
    : juce::InterprocessConnection (true)
    , connections (newConnections)
{
}

Connection::~Connection()
{
    state.removeListener (this);
    disconnect();
}

void Connection::connectionMade()
{
    state = juce::ValueTree { jam::Format::toType (Id::connection) };
    state.addListener (this);
    model.state.getChildWithName (jam::Format::toType (Id::connections)).appendChild (state, nullptr);
}

void Connection::connectionLost()
{
    state.getParent().removeChild (state, nullptr);
    connections.remove (connections.indexOf (this));
}

void Connection::messageReceived (const juce::MemoryBlock& message)
{
    const auto tree { juce::ValueTree::readFromData (message.getData(), message.getSize()) };

    if (tree.hasType (jam::Format::toType (Id::connection)))
    {
        state.copyPropertiesFrom (tree, nullptr);
    }
    else if (tree.hasType (jam::Format::toType (Id::cancel)))
    {
        if (auto request { state.getChildWithProperty (Id::id, tree[Id::id]) }; request.isValid())
            Query::setError (request, lsp::ErrorCodes::RequestCancelled);
    }
    else
    {
        state.appendChild (tree, nullptr);
    }
}

void Connection::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == Id::result and tree.hasType (jam::Format::toType (Id::request)))
    {
        sendMessage (*tree[Id::result].getBinaryData());
        state.removeChild (tree, nullptr);
    }
}
