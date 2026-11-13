#include "Processor.h"

Processor::Processor (LINGModel& newModel)
    : model (newModel)
{
    model.addListener (this);
    registerEvents();
}

Processor::~Processor()
{
    model.removeListener (this);
}

std::string_view Processor::getView (const jam::CodeDocument& document, const jam::Document::Token& token)
{
    return std::string_view (document.getSource()).substr (token.offset(), token.length());
}

bool Processor::isDeclaration (const jam::Document::Element& element)
{
    const auto type { *element.get<int> (Id::type) };

    return type >= map::SyntaxTokenType::namespaceToken and type <= map::SyntaxTokenType::macro;
}

jam::Array<std::string_view> Processor::getName (const jam::CodeDocument& document, const jam::Document::Element& element)
{
    jam::Array<std::string_view> name;

    if (const auto* tokens { element.find<jam::Document::Tokens> (Id::name) }; tokens != nullptr)
        for (const auto& token : *tokens)
            name.add (getView (document, token));

    return name;
}

jam::Array<std::string_view> Processor::getPath (const jam::CodeDocument& document, const jam::Document::Element& element)
{
    jam::Array<std::string_view> path;

    if (element.parent != nullptr)
        path = getPath (document, *element.parent);

    const auto name (getName (document, element));
    path.insertArray (path.size(), name.begin(), name.size());

    return path;
}

jam::Array<std::string_view> Processor::getPath (const jam::Array<std::string_view>& scope, int size, const jam::Array<std::string_view>& name)
{
    jassert (size >= 0 and size <= scope.size());

    jam::Array<std::string_view> path;
    path.insertArray (0, scope.begin(), size);
    path.insertArray (path.size(), name.begin(), name.size());

    return path;
}

bool Processor::isSamePath (const jam::Array<std::string_view>& path, const jam::Array<std::string_view>& otherPath)
{
    return path.size() == otherPath.size()
        and std::equal (path.begin(), path.end(), otherPath.begin(), [] (std::string_view left, std::string_view right)
    {
        return left.compare (right) == 0;
    });
}

int Processor::getPathIndex (const juce::ValueTree& request) const
{
    return paths.indexOf (juce::File { request[Id::directory].toString() }.getChildFile ((*request[Id::arguments].getArray())[fileIndex].toString()).getFullPathName(), false, 0);
}

bool Processor::hasDeclarations (const jam::Array<std::string_view>& path) const
{
    bool isDeclared { false };

    applyToDeclarations (path, [&isDeclared] (int, const jam::CodeDocument&, const jam::Document::Element& declaration)
    {
        const auto type { *declaration.get<int> (Id::type) };

        isDeclared = isDeclared or (type != map::SyntaxTokenType::constructor and type != map::SyntaxTokenType::destructor);
    });

    return isDeclared;
}

bool Processor::isReferenceTo (int referenceIndex, const jam::Document::Element& reference, const jam::Array<std::string_view>& declarationPath) const
{
    bool isDeclared { false };
    const auto* tokens { reference.find<jam::Document::Tokens> (Id::name) };

    if (not isDeclaration (reference) and tokens != nullptr and not tokens->isEmpty()
        and getView (*documents.at (referenceIndex), tokens->last()).compare (declarationPath.last()) == 0)
    {
        applyToDeclarations (referenceIndex, reference, [&isDeclared, &declarationPath] (int, const jam::CodeDocument& declarationDocument, const jam::Document::Element& declaration)
        {
            isDeclared = isDeclared or isSamePath (getPath (declarationDocument, declaration), declarationPath);
        });
    }

    return isDeclared;
}

jam::Array<jam::Array<std::string_view>> Processor::getDeclarationPaths (const jam::CodeDocument& document, const jam::Document::Element& element) const
{
    jam::Array<jam::Array<std::string_view>> declarationPaths;

    if (isDeclaration (element))
        declarationPaths.add (getPath (document, element));
    else
        declarationPaths = getDeclarationPaths (getPath (document, *element.parent), getName (document, element), {});

    return declarationPaths;
}

jam::Array<jam::Array<std::string_view>> Processor::getDeclarationPaths (const jam::Array<std::string_view>& scope, const jam::Array<std::string_view>& name, const jam::Array<const jam::Array<std::string_view>*>& tried) const
{
    for (int size { scope.size() }; size >= 0; --size)
    {
        auto declarationPaths (getMemberPaths (getPath (scope, size, {}), name, tried));

        if (not declarationPaths.isEmpty()) return declarationPaths;
    }

    return {};
}

jam::Array<jam::Array<std::string_view>> Processor::getMemberPaths (const jam::Array<std::string_view>& classPath, const jam::Array<std::string_view>& name, const jam::Array<const jam::Array<std::string_view>*>& tried) const
{
    jam::Array<jam::Array<std::string_view>> memberPaths;
    auto candidate (getPath (classPath, classPath.size(), name));

    if (hasDeclarations (candidate))
    {
        memberPaths.add (std::move (candidate));
    }
    else if (std::none_of (tried.begin(), tried.end(), [&classPath] (const auto* triedPath) { return isSamePath (*triedPath, classPath); }))
    {
        jam::Array<const jam::Array<std::string_view>*> extended;
        extended.insertArray (0, tried.begin(), tried.size());
        extended.add (&classPath);

        applyToBases (classPath, [this, &classPath, &name, &extended, &memberPaths] (const jam::CodeDocument& baseDocument, const jam::Document::Element& base)
        {
            for (const auto& basePath : getDeclarationPaths (classPath, getName (baseDocument, base), extended))
                for (auto& memberPath : getMemberPaths (basePath, name, extended))
                    if (not std::any_of (memberPaths.begin(), memberPaths.end(), [&memberPath] (const auto& path) { return isSamePath (path, memberPath); }))
                        memberPaths.add (std::move (memberPath));
        });
    }

    return memberPaths;
}

const jam::Document::Element* Processor::getElement (const jam::CodeDocument& document, int line, int column) const
{
    jassert (line > 0 and column > 0);

    const auto targetLine { static_cast<uint32_t> (line) };
    const auto targetColumn { static_cast<uint32_t> (column) };
    const jam::Document::Element* found { nullptr };

    for (const auto* child : document)
        child->applyFunctionRecursively ([&document, &found, targetLine, targetColumn] (const jam::Document::Element& element)
        {
            const auto& name { *element.get<jam::Document::Tokens> (Id::name) };

            if (found == nullptr and name.size() > 0)
            {
                const auto& token { name.last() };
                const auto startColumn { document.getColumnNumber (token.offset()) };

                if (document.getLineNumber (token.offset()) == targetLine and targetColumn >= startColumn and targetColumn < startColumn + token.length())
                    found = &element;
            }

            return found == nullptr;
        });

    return found;
}

void Processor::setResult (juce::ValueTree& request, jam::Array<std::pair<int, const jam::Document::Element*>> results) const
{
    setResult (request, std::move (results), [] (std::string&, int, const jam::Document::Element&) {});
}

void Processor::setResult (juce::ValueTree& request, std::string_view json) const
{
    request.setProperty (Id::result, jam::Strings::fromView (json), nullptr);
}

bool Processor::isBefore (int pathIndex, const jam::Document::Element& element, int otherPathIndex, const jam::Document::Element& otherElement) const
{
    const auto order { paths.getView (pathIndex).compare (paths.getView (otherPathIndex)) };

    if (order == 0)
        return element.get<jam::Document::Tokens> (Id::name)->last().offset() < otherElement.get<jam::Document::Tokens> (Id::name)->last().offset();

    return order < 0;
}

void Processor::appendDocumentSymbols (std::string& json, int pathIndex, const jam::Document::Element& parent) const
{
    for (const auto* child : parent)
    {
        if (isDeclaration (*child))
        {
            if (child->get<jam::Document::Tokens> (Id::name)->isEmpty())
            {
                appendDocumentSymbols (json, pathIndex, *child);
            }
            else
            {
                JsonWriter::appendElement (json, paths.getView (pathIndex), *documents.at (pathIndex), *child, [this, pathIndex, child] (std::string& members)
                {
                    JsonWriter::appendKey (members, Id::children);
                    JsonWriter::appendArray (members, [this, pathIndex, child] (std::string& entries)
                    {
                        appendDocumentSymbols (entries, pathIndex, *child);
                    });
                });
            }
        }
    }
}

void Processor::registerEvents()
{
    events.add (Id::goToDefinition, [this] (const juce::Identifier&, juce::ValueTree& request) { goToDefinition (request); });
    events.add (Id::findReferences, [this] (const juce::Identifier&, juce::ValueTree& request) { findReferences (request); });
    events.add (Id::hover, [this] (const juce::Identifier&, juce::ValueTree& request) { hover (request); });
    events.add (Id::documentSymbol, [this] (const juce::Identifier&, juce::ValueTree& request) { documentSymbol (request); });
    events.add (Id::workspaceSymbol, [this] (const juce::Identifier&, juce::ValueTree& request) { workspaceSymbol (request); });
    events.add (Id::goToImplementation, [this] (const juce::Identifier&, juce::ValueTree& request) { goToImplementation (request); });
    events.add (Id::prepareCallHierarchy, [this] (const juce::Identifier&, juce::ValueTree& request) { prepareCallHierarchy (request); });
    events.add (Id::incomingCalls, [this] (const juce::Identifier&, juce::ValueTree& request) { incomingCalls (request); });
    events.add (Id::outgoingCalls, [this] (const juce::Identifier&, juce::ValueTree& request) { outgoingCalls (request); });
}

void Processor::runRequest (juce::ValueTree& request)
{
    jassert (request[Id::arguments].isArray());

    const auto word { request[Id::command].toString() };
    const juce::Identifier key { word.isNotEmpty() ? juce::Identifier { word } : juce::Identifier {} };

    if (events.contains (key))
    {
        events.get (key, request);
    }
    else
    {
        model.setMessage (text::Diagnostics::failArguments);
        setResult (request, jam::Array<std::pair<int, const jam::Document::Element*>> {});
    }
}

void Processor::goToDefinition (juce::ValueTree& request) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToPosition (request, [this, &results] (int pathIndex, const jam::CodeDocument&, const jam::Document::Element& element)
    {
        applyToDeclarations (pathIndex, element, [&results] (int declarationIndex, const jam::CodeDocument&, const jam::Document::Element& declaration)
        {
            results.add ({ declarationIndex, &declaration });
        });
    });

    setResult (request, std::move (results));
}

void Processor::findReferences (juce::ValueTree& request) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToPosition (request, [this, &results] (int pathIndex, const jam::CodeDocument&, const jam::Document::Element& element)
    {
        applyToDeclarations (pathIndex, element, [this, &results] (int declarationIndex, const jam::CodeDocument&, const jam::Document::Element& declaration)
        {
            applyToReferences (declarationIndex, declaration, [&results] (int referenceIndex, const jam::CodeDocument&, const jam::Document::Element& reference)
            {
                results.add ({ referenceIndex, &reference });
            });
        });
    });

    setResult (request, std::move (results));
}

void Processor::hover (juce::ValueTree& request) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToPosition (request, [this, &results] (int pathIndex, const jam::CodeDocument&, const jam::Document::Element& element)
    {
        applyToDeclarations (pathIndex, element, [&results] (int declarationIndex, const jam::CodeDocument&, const jam::Document::Element& declaration)
        {
            results.add ({ declarationIndex, &declaration });
        });
    });

    setResult (request, std::move (results), [this] (std::string& json, int pathIndex, const jam::Document::Element& element)
    {
        const auto path (getPath (*documents.at (pathIndex), element));

        JsonWriter::appendKey (json, Id::scope);
        JsonWriter::appendArray (json, [&path] (std::string& entries)
        {
            for (int index { 0 }; index < path.size() - 1; ++index)
            {
                JsonWriter::appendSeparator (entries);
                JsonWriter::appendString (entries, path.at (index));
            }
        });
    });
}

void Processor::documentSymbol (juce::ValueTree& request) const
{
    static constexpr int fileArgumentCount { 1 };

    const auto& arguments { *request[Id::arguments].getArray() };
    std::string json;

    JsonWriter::appendArray (json, [this, &request, &arguments] (std::string& entries)
    {
        if (arguments.size() == fileArgumentCount)
        {
            const auto pathIndex { getPathIndex (request) };

            if (documents.contains (pathIndex))
                appendDocumentSymbols (entries, pathIndex, *documents.at (pathIndex)->getRoot());
        }
        else
        {
            model.setMessage (text::Diagnostics::failArguments);
        }
    });

    setResult (request, json);
}

jam::Array<std::tuple<int, int, const jam::Document::Element*>> Processor::getMatches (const jam::Fuzzy::Pattern& pattern)
{
    jam::Array<std::tuple<int, int, const jam::Document::Element*>> scored;

    applyToElements ([this, &scored, &pattern] (int pathIndex, const jam::CodeDocument& document, const jam::Document::Element& element)
    {
        if (isDeclaration (element) and not element.get<jam::Document::Tokens> (Id::name)->isEmpty())
        {
            const auto& token { element.get<jam::Document::Tokens> (Id::name)->last() };
            const auto match { pattern.match (jam::Strings::fromView (getView (document, token)), slab) };

            if (match.isMatch())
                scored.add ({ match.score, pathIndex, &element });
        }
    });

    std::sort (scored.begin(), scored.end(), [this] (const auto& left, const auto& right)
    {
        const auto& [leftScore, leftIndex, leftElement] { left };
        const auto& [rightScore, rightIndex, rightElement] { right };

        return leftScore != rightScore ? leftScore > rightScore : isBefore (leftIndex, *leftElement, rightIndex, *rightElement);
    });

    return scored;
}

void Processor::workspaceSymbol (juce::ValueTree& request)
{
    static constexpr int queryArgumentCount { 1 };
    static constexpr bool shouldNormalize { true };

    const auto& arguments { *request[Id::arguments].getArray() };
    std::string json;

    JsonWriter::appendArray (json, [this, &arguments] (std::string& entries)
    {
        if (arguments.size() == queryArgumentCount)
        {
            const auto pattern { jam::Fuzzy::Pattern::parse (arguments.getFirst().toString(), jam::Fuzzy::CaseMode::smart, shouldNormalize, jam::Fuzzy::Scheme::standard) };

            for (const auto& [score, pathIndex, element] : getMatches (pattern))
                JsonWriter::appendElement (entries, paths.getView (pathIndex), *documents.at (pathIndex), *element);
        }
        else
        {
            model.setMessage (text::Diagnostics::failArguments);
        }
    });

    setResult (request, json);
}

void Processor::goToImplementation (juce::ValueTree& request) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToPosition (request, [this, &results] (int pathIndex, const jam::CodeDocument&, const jam::Document::Element& element)
    {
        applyToDeclarations (pathIndex, element, [this, &results] (int declarationIndex, const jam::CodeDocument&, const jam::Document::Element& declaration)
        {
            applyToImplementations (declarationIndex, declaration, [&results] (int pathIndex, const jam::CodeDocument&, const jam::Document::Element& implementation)
            {
                results.add ({ pathIndex, &implementation });
            });
        });
    });

    setResult (request, std::move (results));
}

void Processor::prepareCallHierarchy (juce::ValueTree& request) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToFunctions (request, [&results] (int declarationIndex, const jam::CodeDocument&, const jam::Document::Element& declaration)
    {
        results.add ({ declarationIndex, &declaration });
    });

    setResult (request, std::move (results));
}

void Processor::incomingCalls (juce::ValueTree& request) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToFunctions (request, [this, &results] (int declarationIndex, const jam::CodeDocument&, const jam::Document::Element& declaration)
    {
        applyToReferences (declarationIndex, declaration, [&results] (int referenceIndex, const jam::CodeDocument&, const jam::Document::Element& reference)
        {
            const auto* name { reference.parent->find<jam::Document::Tokens> (Id::name) };

            if (*reference.get<int> (Id::type) == map::SyntaxTokenType::call and name != nullptr and not name->isEmpty())
                results.add ({ referenceIndex, reference.parent });
        });
    });

    setResult (request, std::move (results));
}

void Processor::outgoingCalls (juce::ValueTree& request) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToFunctions (request, [this, &results] (int declarationIndex, const jam::CodeDocument&, const jam::Document::Element& declaration)
    {
        declaration.applyFunctionRecursively ([this, &results, declarationIndex] (const jam::Document::Element& element)
        {
            if (*element.get<int> (Id::type) == map::SyntaxTokenType::call)
                applyToDeclarations (declarationIndex, element, [&results] (int pathIndex, const jam::CodeDocument&, const jam::Document::Element& candidate)
                {
                    results.add ({ pathIndex, &candidate });
                });
        });
    });

    setResult (request, std::move (results));
}

void Processor::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& childWhichHasBeenAdded)
{
    if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::files)))
    {
        for (auto entry { documents.begin() }; entry != documents.end();)
        {
            const auto& [pathIndex, document] { *entry };

            if (childWhichHasBeenAdded.getChildWithProperty (Id::path, paths.at (pathIndex)).isValid())
                ++entry;
            else
                entry = documents.erase (entry);
        }

        for (int rowIndex { 0 }; rowIndex < childWhichHasBeenAdded.getNumChildren(); ++rowIndex)
            addJob (childWhichHasBeenAdded.getChild (rowIndex), rowIndex);

        if (pendingJobs == 0)
            setDocuments();
    }
    else if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::request)))
    {
        runRequest (childWhichHasBeenAdded);
    }
}

void Processor::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == Id::path and tree.hasType (jam::Format::toType (Id::file)))
        addJob (tree, tree.getParent().indexOf (tree));
}

void Processor::addJob (const juce::ValueTree& row, int rowIndex)
{
    ++pendingJobs;

    pool.addJob ([this, rowIndex, path = row[Id::path].toString(), family = static_cast<int> (row[Id::family])]
    {
        const juce::File file { path };
        const auto modificationTime { file.getLastModificationTime() };
        auto document { file.existsAsFile() ? jam::SyntaxTree::parse (jam::Strings::getView (file.loadFileAsString()), family) : nullptr };
        const auto result { document != nullptr ? jam::SyntaxTree::isValid (*document, family) : juce::Result::ok() };
        const auto parsed { std::make_shared<std::unique_ptr<jam::CodeDocument>> (std::move (document)) };

        juce::MessageManager::callAsync ([this, rowIndex, path, modificationTime, parsed, result]
        {
            addDocument (rowIndex, path, modificationTime, std::move (*parsed), result);
        });
    });
}

void Processor::addDocument (int rowIndex, const juce::String& path, juce::Time modificationTime, std::unique_ptr<jam::CodeDocument> document, const juce::Result& result)
{
    --pendingJobs;

    const auto row { model.state.getChildWithName (jam::Format::toType (Id::files)).getChild (rowIndex) };

    if (row[Id::path].toString().compare (path) == 0 and juce::File { path }.getLastModificationTime() == modificationTime)
    {
        const auto pathIndex { getOrCreatePathIndex (path) };

        if (document != nullptr)
            documents.addOrReplace (pathIndex, std::move (document));
        else
            documents.erase (pathIndex);

        if (not result.wasOk())
            model.setMessage (path + Id::diagnosticSeparator + result.getErrorMessage());
    }

    if (pendingJobs == 0)
        setDocuments();
}

int Processor::getOrCreatePathIndex (const juce::String& path)
{
    if (const auto pathIndex { paths.indexOf (path, false, 0) }; pathIndex >= 0)
        return pathIndex;

    paths.add (path);
    return paths.size() - 1;
}

void Processor::setDocuments()
{
    const juce::var count { static_cast<int> (documents.size()) };

    if (model.state[Id::documents] == count)
        model.state.sendPropertyChangeMessage (Id::documents);
    else
        model.state.setProperty (Id::documents, count, nullptr);
}
