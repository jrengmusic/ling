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

    if (const auto document { jam::JsonDocument::parse (std::string_view { static_cast<const char*> (message.getData()), message.getSize() }) }; document != nullptr)
    {
        const auto& words { *document->getChildByToken (*document->getRoot(), document->getValue())->get<jam::Document::Tokens> (Id::tokens) };
        juce::Array<juce::var> arguments;
        juce::ValueTree request { jam::Format::toType (Id::request) };

        for (int index { argumentsIndex }; index < words.size(); ++index)
            arguments.add (jam::Strings::fromView (document->getString (words.at (index))));

        request.setProperty (Id::command, jam::Strings::fromView (document->getString (words.at (commandIndex))), nullptr);
        request.setProperty (Id::directory, jam::Strings::fromView (document->getString (words.at (directoryIndex))), nullptr);
        request.setProperty (Id::arguments, arguments, nullptr);
        state.appendChild (request, nullptr);
    }
}

void Connection::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == Id::result and tree.hasType (jam::Format::toType (Id::request)))
    {
        const auto result { tree[Id::result].toString() };

        const auto newline { static_cast<char> (Chars::newline) };
        juce::MemoryBlock message { result.toRawUTF8(), result.getNumBytesAsUTF8() };

        message.append (&newline, sizeof (newline));
        sendMessage (message);
        state.removeChild (tree, nullptr);
    }
}
