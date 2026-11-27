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

ToolRelay::ToolRelay (LINGModel& newModel, const juce::File& newManifestFile, const juce::String& newManifestName, const juce::String& newToolchainArgument)
    : model (newModel)
    , manifestFile (newManifestFile)
    , manifestName (newManifestName)
    , toolchainArgument (newToolchainArgument)
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
        for (const auto& [path, client] : clients)
        {
            juce::ValueTree cancel { jam::Format::toType (Id::cancel) };

            cancel.setProperty (Id::id, requestId, nullptr);
            client->addRequest (cancel);
        }
    };
}

void ToolRelay::start()
{
    getOrCreateClient (manifestFile);
    server.start();
}

void ToolRelay::addRequest (const juce::String& requestId, const jam::JsonDocument& document, const jam::Document::Element& arguments, const juce::Identifier& command, const juce::Identifier& shape)
{
    juce::ValueTree request { jam::Format::toType (Id::request) };

    request.setProperty (Id::command, command.toString(), nullptr);
    request.setProperty (Id::id, requestId, nullptr);
    request.setProperty (Id::positionEncoding, jam::Strings::fromView (lsp::PositionEncodingType::UTF16), nullptr);
    request.setProperty (Id::directory, juce::File::getCurrentWorkingDirectory().getFullPathName(), nullptr);
    request.setProperty (Id::arguments, getRequestArguments (document, arguments, shape), nullptr);
    getOrCreateClient (getManifestFile (request, shape)).addRequest (request);
}

juce::File ToolRelay::getManifestFile (const juce::ValueTree& request, const juce::Identifier& shape) const
{
    if (shape != Id::query)
    {
        const auto requestManifest { LINGModel::getManifestFile (Query::getFile (request).getParentDirectory(), manifestName) };

        if (requestManifest != juce::File {})
            return requestManifest;
    }

    return manifestFile;
}

ToolClient& ToolRelay::getOrCreateClient (const juce::File& requestManifest)
{
    const auto [entry, isAdded] { clients.try_emplace (requestManifest.getFullPathName()) };
    auto& [path, client] { *entry };

    if (isAdded)
    {
        client = std::make_unique<ToolClient> (model, requestManifest, toolchainArgument, server);
        client->start();
    }

    return *client;
}
