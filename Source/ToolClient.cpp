#include "ToolClient.h"

ToolClient::ToolClient (LINGModel& newModel, const juce::File& newManifestFile, const juce::String& newToolchainArgument, jam::ModelContextServer& newServer)
    : Client (newModel, newManifestFile, newToolchainArgument)
    , server (newServer)
{
    state.addListener (this);
}

ToolClient::~ToolClient()
{
    state.removeListener (this);
    disconnect();
}

void ToolClient::addRequest (const juce::ValueTree& request)
{
    requests.appendChild (request, nullptr);
}

void ToolClient::messageReceived (const juce::MemoryBlock& reply)
{
    const auto document { jam::JsonDocument::read (std::string_view { static_cast<const char*> (reply.getData()), reply.getSize() }) };
    jassert (document != nullptr);
    const auto& root { *document->getChildByToken (*document->getRoot(), document->getValue()) };

    if (document->contains (root, lsp::Member::method))
    {
        setMessage (*document, root);
    }
    else if (document->contains (root, lsp::Member::result))
    {
        const auto id { jam::Strings::fromView (document->getView (document->get (root, lsp::Member::id))) };
        const auto& resultToken { document->get (root, lsp::Member::result) };
        const auto* resultElement { document->getChildByToken (root, resultToken) };
        const auto resultView { resultElement != nullptr ? document->getView (resultElement->get<jam::Document::Tokens> (Id::range)->at (0))
                                                         : document->getView (resultToken) };

        server.sendResponse (id, resultView);
    }
    else if (document->contains (root, lsp::Member::error))
    {
        const auto id { jam::Strings::fromView (document->getView (document->get (root, lsp::Member::id))) };
        const auto& error { *document->getChildByToken (root, document->get (root, lsp::Member::error)) };

        server.sendToolError (id, document->getString (document->get (error, lsp::Member::message)));
    }
}

void ToolClient::valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
{
    if (parentTree == requests)
    {
        if (isConnected())
            sendRequest (childWhichHasBeenAdded);
        else if (static_cast<int> (state[Id::port]) == Server::noPort)
            sendNoDaemonError (childWhichHasBeenAdded);
    }
}

void ToolClient::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == state and property == Id::port and static_cast<int> (tree[property]) == Server::noPort)
    {
        while (requests.getNumChildren() > 0)
        {
            const auto row { requests.getChild (0) };

            sendNoDaemonError (row);
        }
    }
}

void ToolClient::sendNoDaemonError (const juce::ValueTree& row)
{
    if (row.hasType (jam::Format::toType (Id::request)) and row.hasProperty (Id::id))
    {
        std::string errorMessage { text::Diagnostics::failNoDaemon };
        errorMessage.append (jam::Strings::getView (Id::diagnosticSeparator));
        errorMessage.append (jam::Strings::getView (manifestFile.getFullPathName()));
        server.sendToolError (row[Id::id].toString(), errorMessage);
    }

    requests.removeChild (row, nullptr);
}
