#include "Connection.h"

Connection::Connection (jam::Owner<Connection>& newConnections, Attach newOnAttach, Detach newOnDetach)
    : juce::InterprocessConnection (true)
    , connections (newConnections)
    , onAttach (std::move (newOnAttach))
    , onDetach (std::move (newOnDetach))
{
    jassert (onAttach != nullptr and onDetach != nullptr);
}

Connection::~Connection()
{
    state.removeListener (this);
    disconnect();
}

bool Connection::isIn (const juce::ValueTree& connectionsTree) const
{
    return state.getParent() == connectionsTree;
}

void Connection::connectionMade()
{
    state = juce::ValueTree { jam::Format::toType (Id::connection) };
    state.addListener (this);
}

void Connection::connectionLost()
{
    onDetach (*this, state);
    connections.remove (connections.indexOf (this));
}

void Connection::messageReceived (const juce::MemoryBlock& message)
{
    const auto tree { juce::ValueTree::readFromData (message.getData(), message.getSize()) };

    if (tree.hasType (jam::Format::toType (Id::connection)))
    {
        if (not state.getParent().isValid() and tree.hasProperty (Id::manifest))
            onAttach (*this, state, juce::File { tree[Id::manifest].toString() }, tree[Id::toolchainArgument].toString());

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
