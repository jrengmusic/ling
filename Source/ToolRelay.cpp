#include "ToolRelay.h"

juce::Array<juce::var> ToolRelay::getRequestArguments (const jam::JsonDocument& document, const jam::Document::Element& arguments, const juce::Identifier& shape)
{
    juce::Array<juce::var> requestArguments;

    if (shape == Id::position)
    {
        requestArguments.add (jam::Strings::fromView (document.getString (document.get (arguments, jam::Strings::getView (Id::file.toString())))));
        requestArguments.add (jam::Format::getNumber<int> (document.getView (document.get (arguments, jam::Strings::getView (Id::line.toString())))));
        requestArguments.add (jam::Format::getNumber<int> (document.getView (document.get (arguments, jam::Strings::getView (Id::column.toString())))));
    }
    else if (shape == Id::file)
    {
        requestArguments.add (jam::Strings::fromView (document.getString (document.get (arguments, jam::Strings::getView (Id::file.toString())))));
    }
    else if (shape == Id::query)
    {
        requestArguments.add (jam::Strings::fromView (document.getString (document.get (arguments, jam::Strings::getView (Id::query.toString())))));
    }

    return requestArguments;
}

ToolRelay::ToolRelay (const juce::File& newManifestFile, const juce::String& newToolchainArgument)
    : Client (newManifestFile, newToolchainArgument)
{
    for (const auto& command : { Id::goToDefinition, Id::findReferences, Id::hover, Id::completion,
                                 Id::goToImplementation, Id::prepareCallHierarchy, Id::incomingCalls, Id::outgoingCalls,
                                 Id::documentSymbol, Id::switchSourceHeader, Id::inlayHint, Id::diagnostics,
                                 Id::workspaceSymbol })
    {
        server.addTool (jam::Strings::getView (command.toString()),
                        [this] (const juce::Identifier& name, const juce::String& requestId, const jam::JsonDocument& document, const jam::Document::Element& arguments)
                        {
                            addRequest (requestId, document, arguments, name, Query::getArgumentShape (name));
                        });
    }

    server.onInputClosed = []
    {
        juce::JUCEApplicationBase::getInstance()->setApplicationReturnValue (EXIT_SUCCESS);
        juce::JUCEApplicationBase::quit();
    };

    server.onCancelled = [this] (const juce::String& requestId)
    {
        juce::ValueTree cancel { jam::Format::toType (Id::cancel) };

        cancel.setProperty (Id::id, requestId, nullptr);
        requests.appendChild (cancel, nullptr);
    };

    model.state.addListener (this);
}

ToolRelay::~ToolRelay()
{
    model.state.removeListener (this);
    disconnect();
}

void ToolRelay::start()
{
    Client::start();
    server.start();
}

void ToolRelay::connectionMade()
{
    while (requests.getNumChildren() > 0)
    {
        const auto row { requests.getChild (0) };

        sendRequest (row);
    }
}

void ToolRelay::messageReceived (const juce::MemoryBlock& reply)
{
    const auto document { jam::JsonDocument::read (std::string_view { static_cast<const char*> (reply.getData()), reply.getSize() }) };
    jassert (document != nullptr);
    const auto& root { *document->getChildByToken (*document->getRoot(), document->getValue()) };

    if (document->contains (root, lsp::Member::method))
    {
        if (document->getString (document->get (root, lsp::Member::method)).compare (lsp::Method::showMessage) == 0)
        {
            const auto& params { *document->getChildByToken (root, document->get (root, lsp::Member::params)) };

            model.setMessage (jam::Strings::fromView (document->getString (document->get (params, lsp::Member::message))));
        }
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

void ToolRelay::valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
{
    if (parentTree == requests)
    {
        if (isConnected())
            sendRequest (childWhichHasBeenAdded);
        else if (static_cast<int> (model.state[Id::port]) == Server::noPort)
            sendEmptyResult (childWhichHasBeenAdded);
    }
}

void ToolRelay::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == model.state and property == Id::port and static_cast<int> (tree[property]) == Server::noPort)
    {
        while (requests.getNumChildren() > 0)
        {
            const auto row { requests.getChild (0) };

            sendEmptyResult (row);
        }
    }
}

void ToolRelay::addRequest (const juce::String& requestId, const jam::JsonDocument& document, const jam::Document::Element& arguments, const juce::Identifier& command, const juce::Identifier& shape)
{
    juce::ValueTree request { jam::Format::toType (Id::request) };

    request.setProperty (Id::command, command.toString(), nullptr);
    request.setProperty (Id::id, requestId, nullptr);
    request.setProperty (Id::positionEncoding, jam::Strings::fromView (lsp::PositionEncodingType::UTF16), nullptr);
    request.setProperty (Id::directory, juce::File::getCurrentWorkingDirectory().getFullPathName(), nullptr);
    request.setProperty (Id::arguments, getRequestArguments (document, arguments, shape), nullptr);
    requests.appendChild (request, nullptr);
}

void ToolRelay::sendRequest (const juce::ValueTree& row)
{
    juce::MemoryBlock data;
    juce::MemoryOutputStream stream { data, false };

    row.writeToStream (stream);
    stream.flush();
    sendMessage (data);
    requests.removeChild (row, nullptr);
}

void ToolRelay::sendEmptyResult (const juce::ValueTree& row)
{
    if (row.hasType (jam::Format::toType (Id::request)) and row.hasProperty (Id::id))
        server.sendResponse (row[Id::id].toString(), jam::Strings::getView (Id::null.toString()));

    requests.removeChild (row, nullptr);
}
