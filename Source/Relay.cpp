#include "Relay.h"
#include "Legend.h"

static void appendNameArray (std::string& json, const jam::Strings& names)
{
    jam::JsonWriter::appendArray (json, [&names] (std::string& entries)
    {
        for (const auto& name : names)
        {
            jam::JsonWriter::appendSeparator (entries);
            jam::JsonWriter::appendString (entries, jam::Strings::getView (name));
        }
    });
}

static void addPositionArguments (juce::Array<juce::var>& requestArguments, const jam::JsonDocument& document, const jam::Document::Element& position)
{
    for (const auto member : { lsp::Member::line, lsp::Member::character })
        requestArguments.add (jam::Format::getNumber<int> (document.getView (document.get (position, member))) + 1);
}

static void appendCapabilities (std::string& json, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [encoding] (std::string& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::positionEncoding);
        jam::JsonWriter::appendString (members, encoding);
        jam::JsonWriter::appendKey (members, lsp::Member::textDocumentSync);
        jam::Format::appendDecimal (members, lsp::TextDocumentSyncType::Full);

        for (const auto provider : { lsp::Member::hoverProvider, lsp::Member::definitionProvider, lsp::Member::declarationProvider, lsp::Member::referencesProvider,
                                     lsp::Member::documentSymbolProvider, lsp::Member::workspaceSymbolProvider, lsp::Member::implementationProvider, lsp::Member::callHierarchyProvider,
                                     lsp::Member::inlayHintProvider, lsp::Member::selectionRangeProvider, lsp::Member::documentHighlightProvider, lsp::Member::renameProvider,
                                     lsp::Member::typeDefinitionProvider, lsp::Member::codeActionProvider })
        {
            jam::JsonWriter::appendKey (members, provider);
            jam::JsonWriter::appendBoolean (members, true);
        }

        jam::JsonWriter::appendKey (members, lsp::Member::completionProvider);
        jam::JsonWriter::appendObject (members, [] (std::string&) {});

        jam::JsonWriter::appendKey (members, lsp::Member::semanticTokensProvider);
        jam::JsonWriter::appendObject (members, [] (std::string& provider)
        {
            jam::JsonWriter::appendKey (provider, lsp::Member::legend);
            jam::JsonWriter::appendObject (provider, [] (std::string& legend)
            {
                jam::JsonWriter::appendKey (legend, lsp::Member::tokenTypes);
                appendNameArray (legend, Legend::getTypes());

                jam::JsonWriter::appendKey (legend, lsp::Member::tokenModifiers);
                appendNameArray (legend, Legend::getModifiers());
            });

            jam::JsonWriter::appendKey (provider, lsp::Member::range);
            jam::JsonWriter::appendBoolean (provider, true);

            jam::JsonWriter::appendKey (provider, lsp::Member::full);
            jam::JsonWriter::appendBoolean (provider, true);
        });
    });
}

static void appendServerInfo (std::string& json)
{
    jam::JsonWriter::appendObject (json, [] (std::string& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::name);
        jam::JsonWriter::appendString (members, ProjectInfo::projectName);
        jam::JsonWriter::appendKey (members, lsp::Member::version);
        jam::JsonWriter::appendString (members, ProjectInfo::versionString);
    });
}

Relay::Relay (const juce::File& newManifestFile, const juce::String& newToolchainArgument)
    : Client (newManifestFile, newToolchainArgument)
{
    requestMethods.add (juce::Identifier { jam::Strings::fromView (lsp::Method::initialize) }, [this] (const juce::Identifier&, const jam::JsonDocument& document, const jam::Document::Element& root, const jam::Document::Token& idToken) { initialize (document, root, idToken); });
    notificationMethods.add (juce::Identifier { jam::Strings::fromView (lsp::Method::initialized) }, [this] (const juce::Identifier&, const jam::JsonDocument&, const jam::Document::Element&) { initialized(); });
    requestMethods.add (juce::Identifier { jam::Strings::fromView (lsp::Method::shutdown) }, [this] (const juce::Identifier&, const jam::JsonDocument& document, const jam::Document::Element&, const jam::Document::Token& idToken) { shutdown (document, idToken); });
    notificationMethods.add (juce::Identifier { jam::Strings::fromView (lsp::Method::exit) }, [this] (const juce::Identifier&, const jam::JsonDocument&, const jam::Document::Element&) { exit(); });
    notificationMethods.add (juce::Identifier { jam::Strings::fromView (lsp::Method::cancelRequest) }, [this] (const juce::Identifier&, const jam::JsonDocument& document, const jam::Document::Element& root) { cancelRequest (document, root); });
    notificationMethods.add (juce::Identifier { jam::Strings::fromView (lsp::Method::didOpen) }, [this] (const juce::Identifier&, const jam::JsonDocument& document, const jam::Document::Element& root) { setBuffer (document, root); });
    notificationMethods.add (juce::Identifier { jam::Strings::fromView (lsp::Method::didChange) }, [this] (const juce::Identifier&, const jam::JsonDocument& document, const jam::Document::Element& root) { setBuffer (document, root); });
    notificationMethods.add (juce::Identifier { jam::Strings::fromView (lsp::Method::didClose) }, [this] (const juce::Identifier&, const jam::JsonDocument& document, const jam::Document::Element& root) { removeBuffer (document, root); });

    stdio.onBytesReady = [this] (int numBytes)
    {
        if (numBytes < 0)
        {
            model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::messages), Id::isOpen)->setValue (0);
        }
        else
        {
            auto* numMessages { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::messages), Id::numMessages) };
            numMessages->setValue (numMessages->getValue() + 1);
        }
    };

    model.state.addListener (this);
}

Relay::~Relay()
{
    model.state.removeListener (this);
    disconnect();
}

void Relay::start()
{
    static constexpr int bufferSize { 1 << 20 };
    static constexpr double maximumPeriodMilliseconds { 0.0 };

    Client::start();
    const bool isOpen { stdio.open (nullptr, bufferSize, maximumPeriodMilliseconds) };
    jassert (isOpen);
    juce::ignoreUnused (isOpen);
}

void Relay::connectionMade()
{
    while (requests.getNumChildren() > 0)
    {
        const auto row { requests.getChild (0) };

        sendRequest (row);
    }
}

void Relay::messageReceived (const juce::MemoryBlock& reply)
{
    jam::StdioProtocol::writeMessage (stdio, std::string_view { static_cast<const char*> (reply.getData()), reply.getSize() });
}

void Relay::valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
{
    if (parentTree == requests)
    {
        if (isConnected())
            sendRequest (childWhichHasBeenAdded);
        else if (static_cast<int> (model.state[Id::port]) == Server::noPort)
            sendEmptyResult (childWhichHasBeenAdded);
    }
}

void Relay::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == messages and property == Id::numMessages)
    {
        stdio.applyPendingData ([this] (const char* data, int size)
        {
            return jam::StdioProtocol::applyMessages (data, size, [this] (std::string_view body)
            {
                auto message { jam::JsonDocument::read (body) };
                jassert (message != nullptr);
                setMessage (*message);
                return true;
            });
        });
    }
    else if (tree == messages and property == Id::isOpen and not static_cast<bool> (tree[property]))
        exit();
    else if (tree == model.state and property == Id::port and static_cast<int> (tree[property]) == Server::noPort)
    {
        while (requests.getNumChildren() > 0)
        {
            const auto row { requests.getChild (0) };

            sendEmptyResult (row);
        }
    }
}

void Relay::setMessage (const jam::JsonDocument& document)
{
    const auto& root { *document.getChildByToken (*document.getRoot(), document.getValue()) };
    const auto method { document.getString (document.get (root, lsp::Member::method)) };
    const juce::Identifier methodKey { juce::CharPointer_UTF8 { method.data() }, juce::CharPointer_UTF8 { method.data() + method.size() } };

    if (document.contains (root, lsp::Member::id))
    {
        const auto& idToken { document.get (root, lsp::Member::id) };

        if (requestMethods.contains (methodKey))
            requestMethods.get (methodKey, document, root, idToken);
        else if (operations->contains (methodKey.toString()))
            addRequest (document, root, idToken, operations->at (methodKey.toString()));
        else
            sendError (document, idToken, lsp::ErrorCodes::MethodNotFound);
    }
    else if (notificationMethods.contains (methodKey))
    {
        notificationMethods.get (methodKey, document, root);
    }
}

void Relay::addRequest (const jam::JsonDocument& document, const jam::Document::Element& root, const jam::Document::Token& idToken, const juce::Identifier& operation)
{
    if (static_cast<bool> (messages[Id::isShutDown]))
    {
        sendError (document, idToken, lsp::ErrorCodes::InvalidRequest);
    }
    else if (static_cast<bool> (messages[Id::isInitialized]))
    {
        auto request { getRequest (document, root, operation) };

        request.setProperty (Id::id, jam::Strings::fromView (document.getView (idToken)), nullptr);
        requests.appendChild (request, nullptr);
    }
    else
    {
        sendError (document, idToken, lsp::ErrorCodes::ServerNotInitialized);
    }
}

void Relay::sendRequest (const juce::ValueTree& row)
{
    juce::MemoryBlock data;
    juce::MemoryOutputStream stream { data, false };

    row.writeToStream (stream);
    stream.flush();
    sendMessage (data);
    requests.removeChild (row, nullptr);
}

void Relay::sendEmptyResult (const juce::ValueTree& row)
{
    if (row.hasType (jam::Format::toType (Id::request)) and row.hasProperty (Id::id))
    {
        const auto id { row[Id::id].toString() };

        sendResponse (jam::Strings::getView (id), lsp::Member::result, [] (std::string& json) { jam::JsonWriter::appendNull (json); });
    }

    requests.removeChild (row, nullptr);
}

void Relay::sendError (const jam::JsonDocument& document, const jam::Document::Token& idToken, int code)
{
    const auto& root { *document.getChildByToken (*document.getRoot(), document.getValue()) };

    sendResponse (document.getView (idToken), lsp::Member::error, [&document, &root, code] (std::string& json)
    {
        jam::JsonWriter::appendObject (json, [&document, &root, code] (std::string& members)
        {
            jam::JsonWriter::appendKey (members, lsp::Member::code);
            jam::Format::appendDecimal (members, code);
            jam::JsonWriter::appendKey (members, lsp::Member::message);
            jam::JsonWriter::appendString (members, document.getString (document.get (root, lsp::Member::method)));
        });
    });
}

void Relay::initialize (const jam::JsonDocument& document, const jam::Document::Element& root, const jam::Document::Token& idToken)
{
    const auto encoding { getPositionEncoding (document, root) };

    messages.setProperty (Id::positionEncoding, jam::Strings::fromView (encoding), nullptr);
    messages.setProperty (Id::isInitialized, true, nullptr);

    sendResponse (document.getView (idToken), lsp::Member::result, [encoding] (std::string& json)
    {
        jam::JsonWriter::appendObject (json, [encoding] (std::string& members)
        {
            jam::JsonWriter::appendKey (members, lsp::Member::capabilities);
            appendCapabilities (members, encoding);
            jam::JsonWriter::appendKey (members, lsp::Member::serverInfo);
            appendServerInfo (members);
        });
    });
}

void Relay::initialized()
{
    juce::ValueTree connection { jam::Format::toType (Id::connection) };

    connection.setProperty (Id::positionEncoding, messages[Id::positionEncoding], nullptr);
    requests.appendChild (connection, nullptr);
}

bool Relay::isSessionActive() const
{
    return static_cast<bool> (messages[Id::isInitialized]) and not static_cast<bool> (messages[Id::isShutDown]);
}

void Relay::cancelRequest (const jam::JsonDocument& document, const jam::Document::Element& root)
{
    if (isSessionActive())
    {
        juce::ValueTree cancel { jam::Format::toType (Id::cancel) };
        const auto& params { *document.getChildByToken (root, document.get (root, lsp::Member::params)) };

        cancel.setProperty (Id::id, jam::Strings::fromView (document.getView (document.get (params, lsp::Member::id))), nullptr);
        requests.appendChild (cancel, nullptr);
    }
}

juce::ValueTree Relay::getRequest (const jam::JsonDocument& document, const jam::Document::Element& root, const juce::Identifier& operation) const
{
    juce::ValueTree request { jam::Format::toType (Id::request) };
    const auto& params { *document.getChildByToken (root, document.get (root, lsp::Member::params)) };

    request.setProperty (Id::command, operation.toString(), nullptr);
    request.setProperty (Id::positionEncoding, messages[Id::positionEncoding], nullptr);
    request.setProperty (Id::directory, juce::File::getCurrentWorkingDirectory().getFullPathName(), nullptr);
    request.setProperty (Id::arguments, getRequestArguments (document, params, operation), nullptr);

    for (const auto& name : { Id::type, Id::scope, Id::newName })
    {
        if (document.contains (params, jam::Strings::getView (name.toString())))
        {
            const auto& token { document.get (params, jam::Strings::getView (name.toString())) };

            request.setProperty (name, jam::Strings::fromView (document.getString (token)), nullptr);
        }
    }

    return request;
}

void Relay::setBuffer (const jam::JsonDocument& document, const jam::Document::Element& root)
{
    if (isSessionActive())
    {
        const auto& params { *document.getChildByToken (root, document.get (root, lsp::Member::params)) };
        auto request { getRequest (document, root, Id::setBuffer) };

        if (document.contains (params, lsp::Member::contentChanges))
        {
            const auto& contentChanges { *document.getChildByToken (params, document.get (params, lsp::Member::contentChanges)) };
            const auto& changes { *contentChanges.get<jam::Document::Tokens> (Id::tokens) };
            const auto& change { *document.getChildByToken (contentChanges, changes.at (0)) };

            request.setProperty (Id::text, jam::Strings::fromView (document.getString (document.get (change, lsp::Member::text))), nullptr);
        }
        else
        {
            const auto& textDocument { *document.getChildByToken (params, document.get (params, lsp::Member::textDocument)) };

            request.setProperty (Id::text, jam::Strings::fromView (document.getString (document.get (textDocument, lsp::Member::text))), nullptr);
        }

        requests.appendChild (request, nullptr);
    }
}

void Relay::removeBuffer (const jam::JsonDocument& document, const jam::Document::Element& root)
{
    if (isSessionActive())
    {
        auto request { getRequest (document, root, Id::removeBuffer) };

        requests.appendChild (request, nullptr);
    }
}

void Relay::shutdown (const jam::JsonDocument& document, const jam::Document::Token& idToken)
{
    messages.setProperty (Id::isShutDown, true, nullptr);
    sendResponse (document.getView (idToken), lsp::Member::result, [] (std::string& json) { jam::JsonWriter::appendNull (json); });
}

void Relay::exit()
{
    juce::JUCEApplicationBase::getInstance()->setApplicationReturnValue (static_cast<bool> (messages[Id::isShutDown]) ? EXIT_SUCCESS : EXIT_FAILURE);
    juce::JUCEApplicationBase::quit();
}

std::string_view Relay::getPositionEncoding (const jam::JsonDocument& document, const jam::Document::Element& root)
{
    const auto& params { *document.getChildByToken (root, document.get (root, lsp::Member::params)) };
    const auto& capabilities { *document.getChildByToken (params, document.get (params, lsp::Member::capabilities)) };
    std::string_view encoding { lsp::PositionEncodingType::UTF16 };

    if (document.contains (capabilities, lsp::Member::general))
    {
        const auto& generalToken { document.get (capabilities, lsp::Member::general) };
        const auto& general { *document.getChildByToken (capabilities, generalToken) };

        if (document.contains (general, lsp::Member::positionEncodings))
        {
            const auto& encodingsToken { document.get (general, lsp::Member::positionEncodings) };
            const auto& encodings { *document.getChildByToken (general, encodingsToken) };
            const auto& tokens { *encodings.get<jam::Document::Tokens> (Id::tokens) };
            const auto hasUtf8 { std::any_of (tokens.begin(), tokens.end(), [&document] (const jam::Document::Token& token) { return document.getString (token).compare (lsp::PositionEncodingType::UTF8) == 0; }) };

            if (hasUtf8)
                encoding = lsp::PositionEncodingType::UTF8;
        }
    }

    return encoding;
}

juce::File Relay::getFile (const jam::JsonDocument& document, const jam::Document::Element& params)
{
    const auto& owner { document.contains (params, lsp::Member::textDocument) ? *document.getChildByToken (params, document.get (params, lsp::Member::textDocument))
                      : document.contains (params, lsp::Member::item) ? *document.getChildByToken (params, document.get (params, lsp::Member::item))
                      : params };

    return juce::URL { jam::Strings::fromView (document.getString (document.get (owner, lsp::Member::uri))) }.getLocalFile();
}

const jam::Document::Element& Relay::getPosition (const jam::JsonDocument& document, const jam::Document::Element& params)
{
    if (document.contains (params, lsp::Member::position))
    {
        const auto& positionToken { document.get (params, lsp::Member::position) };

        return *document.getChildByToken (params, positionToken);
    }

    if (document.contains (params, lsp::Member::positions))
    {
        const auto& positionsToken { document.get (params, lsp::Member::positions) };
        const auto& positions { *document.getChildByToken (params, positionsToken) };
        const auto& positionTokens { *positions.get<jam::Document::Tokens> (Id::tokens) };

        return *document.getChildByToken (positions, positionTokens.at (0));
    }

    const auto& item { *document.getChildByToken (params, document.get (params, lsp::Member::item)) };
    const auto& selectionRange { *document.getChildByToken (item, document.get (item, lsp::Member::selectionRange)) };

    return *document.getChildByToken (selectionRange, document.get (selectionRange, lsp::Member::start));
}

juce::Array<juce::var> Relay::getRequestArguments (const jam::JsonDocument& document, const jam::Document::Element& params, const juce::Identifier& operation)
{
    juce::Array<juce::var> requestArguments;

    if (document.contains (params, lsp::Member::query))
    {
        const auto& queryToken { document.get (params, lsp::Member::query) };

        requestArguments.add (jam::Strings::fromView (document.getString (queryToken)));
    }
    else
    {
        requestArguments.add (getFile (document, params).getFullPathName());

        if (Query::getArgumentShape (operation) == Id::range)
        {
            const auto& range { *document.getChildByToken (params, document.get (params, lsp::Member::range)) };

            addPositionArguments (requestArguments, document, *document.getChildByToken (range, document.get (range, lsp::Member::start)));
            addPositionArguments (requestArguments, document, *document.getChildByToken (range, document.get (range, lsp::Member::end)));
        }
        else if (document.contains (params, lsp::Member::position) or document.contains (params, lsp::Member::positions) or document.contains (params, lsp::Member::item))
        {
            addPositionArguments (requestArguments, document, getPosition (document, params));
        }
    }

    return requestArguments;
}
