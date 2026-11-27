#include "Operation.h"

Operation::Operation (LINGModel& newModel, const juce::File& newManifestFile, const juce::String& newToolchainArgument, juce::MemoryBlock&& newRequest)
    : Client (newModel, newManifestFile, newToolchainArgument)
    , request (std::move (newRequest))
{
    state.addListener (this);
}

Operation::~Operation()
{
    state.removeListener (this);
    disconnect();
}

void Operation::connectionMade()
{
    Client::connectionMade();
    sendMessage (request);
}

void Operation::messageReceived (const juce::MemoryBlock& reply)
{
    const auto document { jam::JsonDocument::read (std::string_view { static_cast<const char*> (reply.getData()), reply.getSize() }) };

    jassert (document != nullptr);

    const auto& root { *document->getChildByToken (*document->getRoot(), document->getValue()) };

    if (document->contains (root, lsp::Member::method))
    {
        setMessage (*document, root);
    }
    else
    {
        const auto newline { static_cast<char> (Chars::newline) };

        fwrite (reply.getData(), sizeof (char), reply.getSize(), stdout);
        fwrite (&newline, sizeof (char), 1, stdout);
        disconnect (waitForDisconnect, juce::InterprocessConnection::Notify::no);
        juce::JUCEApplicationBase::getInstance()->setApplicationReturnValue (document->contains (root, lsp::Member::error) ? EXIT_FAILURE : EXIT_SUCCESS);
        juce::JUCEApplicationBase::quit();
    }
}

void Operation::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == state and property == Id::port and static_cast<int> (tree[property]) == Server::noPort)
    {
        std::string json;

        jam::JsonWriter::appendObject (json, [] (std::string& members)
        {
            Query::appendJsonRpc (members);
            jam::JsonWriter::appendKey (members, lsp::Member::id);
            jam::JsonWriter::appendNull (members);
            jam::JsonWriter::appendKey (members, lsp::Member::result);
            jam::JsonWriter::appendNull (members);
        });

        jam::Format::appendUtf8 (json, Chars::newline);
        fwrite (json.data(), sizeof (char), json.size(), stdout);
        juce::JUCEApplicationBase::getInstance()->setApplicationReturnValue (EXIT_SUCCESS);
        juce::JUCEApplicationBase::quit();
    }
}
