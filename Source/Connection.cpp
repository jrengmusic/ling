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
    static constexpr int commandIndex { 0 };
    static constexpr int directoryIndex { 1 };
    static constexpr int argumentsIndex { 2 };

    const auto words { juce::JSON::parse (message.toString()) };
    const auto* values { words.getArray() };

    jassert (values != nullptr and values->size() >= argumentsIndex);

    if (values != nullptr and values->size() >= argumentsIndex)
    {
        juce::ValueTree request { jam::Format::toType (Id::request) };

        request.setProperty (Id::command, values->getReference (commandIndex), nullptr);
        request.setProperty (Id::directory, values->getReference (directoryIndex), nullptr);
        request.setProperty (Id::arguments, juce::Array<juce::var> (values->begin() + argumentsIndex, values->size() - argumentsIndex), nullptr);
        state.appendChild (request, nullptr);
    }
}

void Connection::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == Id::result and tree.hasType (jam::Format::toType (Id::request)))
    {
        const auto result { tree[Id::result].toString() };

        sendMessage (juce::MemoryBlock { result.toRawUTF8(), result.getNumBytesAsUTF8() });
        state.removeChild (tree, nullptr);
    }
}
