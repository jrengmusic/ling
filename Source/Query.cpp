#include "Query.h"

void Query::setResult (juce::ValueTree& request, jam::Array<std::pair<int, const jam::Document::Element*>> results, const Index& index) const
{
    setResult (request, std::move (results), [] (std::string&, int, const jam::Document::Element&) {}, index);
}

void Query::setResult (juce::ValueTree& request, std::string_view json) const
{
    request.setProperty (Id::result, jam::Strings::fromView (json), nullptr);
}

bool Query::hasName (const jam::Document::Element& element)
{
    return not element.get<jam::Document::Tokens> (Id::name)->isEmpty();
}

jam::Array<std::string_view> Query::getName (const jam::LanguageDocument& document, const jam::Document::Element& element)
{
    jam::Array<std::string_view> name;

    if (const auto* tokens { element.find<jam::Document::Tokens> (Id::name) }; tokens != nullptr)
        for (const auto& token : *tokens)
            name.add (document.getView (token));

    return name;
}

jam::Array<std::string_view> Query::getPath (const jam::LanguageDocument& document, const jam::Document::Element& element)
{
    jam::Array<std::string_view> path;

    if (element.parent != nullptr)
        path = getPath (document, *element.parent);

    const auto name (getName (document, element));
    path.insertArray (path.size(), name.begin(), name.size());

    return path;
}

jam::Array<std::string_view> Query::getPath (const jam::Array<std::string_view>& scope, int size, const jam::Array<std::string_view>& name)
{
    jassert (size >= 0 and size <= scope.size());

    jam::Array<std::string_view> path;
    path.insertArray (0, scope.begin(), size);
    path.insertArray (path.size(), name.begin(), name.size());

    return path;
}

bool Query::isSamePath (const jam::Array<std::string_view>& path, const jam::Array<std::string_view>& otherPath)
{
    return path.size() == otherPath.size()
        and std::equal (path.begin(), path.end(), otherPath.begin(), [] (std::string_view left, std::string_view right)
    {
        return left.compare (right) == 0;
    });
}

bool Query::hasPath (const jam::Array<jam::Array<std::string_view>>& knownPaths, const jam::Array<std::string_view>& path)
{
    return std::any_of (knownPaths.begin(), knownPaths.end(), [&path] (const auto& knownPath) { return isSamePath (knownPath, path); });
}

juce::File Query::getFile (const juce::ValueTree& request)
{
    return juce::File { request[Id::directory].toString() }.getChildFile ((*request[Id::arguments].getArray())[fileIndex].toString());
}

int Query::getPathIndex (const juce::ValueTree& request, const Index& index) const
{
    return index.paths.indexOf (getFile (request).getFullPathName(), false, 0);
}

const jam::Document::Element* Query::getElement (const jam::LanguageDocument& document, int line, int column) const
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

bool Query::isBefore (int pathIndex, const jam::Document::Element& element, int otherPathIndex, const jam::Document::Element& otherElement, const Index& index) const
{
    const auto order { index.paths.getView (pathIndex).compare (index.paths.getView (otherPathIndex)) };

    if (order == 0)
        return element.get<jam::Document::Tokens> (Id::name)->last().offset() < otherElement.get<jam::Document::Tokens> (Id::name)->last().offset();

    return order < 0;
}

bool Query::isBeforeByPath (int pathIndex, const jam::Document::Element& element, int otherPathIndex, const jam::Document::Element& otherElement, const Index& index) const
{
    const auto path (getPath (*index.documents.at (pathIndex), element));
    const auto otherPath (getPath (*index.documents.at (otherPathIndex), otherElement));

    if (isSamePath (path, otherPath)) return isBefore (pathIndex, element, otherPathIndex, otherElement, index);

    return std::lexicographical_compare (path.begin(), path.end(), otherPath.begin(), otherPath.end(), [] (std::string_view left, std::string_view right)
    {
        return left.compare (right) < 0;
    });
}

void Query::appendDocumentSymbols (std::string& json, int pathIndex, const jam::Document::Element& parent, const Index& index) const
{
    for (const auto* child : parent)
    {
        if (index.documents.at (pathIndex)->hasRole (*child, jam::LanguageRole::declaration))
        {
            if (child->get<jam::Document::Tokens> (Id::name)->isEmpty())
            {
                appendDocumentSymbols (json, pathIndex, *child, index);
            }
            else
            {
                JsonWriter::appendElement (json, index.paths.getView (pathIndex), *index.documents.at (pathIndex), *child, [this, &index, pathIndex, child] (std::string& members)
                {
                    jam::JsonWriter::appendKey (members, Id::children);
                    jam::JsonWriter::appendArray (members, [this, &index, pathIndex, child] (std::string& entries)
                    {
                        appendDocumentSymbols (entries, pathIndex, *child, index);
                    });
                });
            }
        }
    }
}

void Query::appendCompletion (std::string& json, int pathIndex, const jam::LanguageDocument& declarationDocument, const jam::Document::Element& declaration, const jam::LanguageDocument& commentDocument, const jam::Document::Element& commentDeclaration, const Index& index) const
{
    JsonWriter::appendElement (json, index.paths.getView (pathIndex), declarationDocument, declaration, [this, &declarationDocument, &declaration, &commentDocument, &commentDeclaration] (std::string& members)
    {
        JsonWriter::appendComment (members, commentDocument, commentDeclaration);

        if (declarationDocument.hasRole (declaration, jam::LanguageRole::function))
            JsonWriter::appendSnippet (members, declarationDocument, declaration);
    });
}

void Query::goToDefinition (juce::ValueTree& request, const Index& index) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToPosition (request, [this, &index, &results] (int pathIndex, const jam::LanguageDocument&, const jam::Document::Element& element)
    {
        applyToDeclarations (element, [&results] (int declarationIndex, const jam::LanguageDocument&, const jam::Document::Element& declaration)
        {
            results.add ({ declarationIndex, &declaration });
        }, index);
    }, index);

    setResult (request, std::move (results), index);
}

void Query::findReferences (juce::ValueTree& request, const Index& index) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToPosition (request, [this, &index, &results] (int pathIndex, const jam::LanguageDocument&, const jam::Document::Element& element)
    {
        applyToDeclarations (element, [this, &index, &results] (int declarationIndex, const jam::LanguageDocument&, const jam::Document::Element& declaration)
        {
            results.add ({ declarationIndex, &declaration });

            applyToReferences (declaration, [&results] (int referenceIndex, const jam::LanguageDocument&, const jam::Document::Element& reference)
            {
                results.add ({ referenceIndex, &reference });
            }, index);
        }, index);
    }, index);

    setResult (request, std::move (results), index);
}

void Query::hover (juce::ValueTree& request, const Index& index) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToPosition (request, [this, &index, &results] (int pathIndex, const jam::LanguageDocument&, const jam::Document::Element& element)
    {
        applyToDeclarations (element, [&results] (int declarationIndex, const jam::LanguageDocument&, const jam::Document::Element& declaration)
        {
            results.add ({ declarationIndex, &declaration });
        }, index);
    }, index);

    setResult (request, std::move (results), [&index] (std::string& json, int pathIndex, const jam::Document::Element& element)
    {
        const auto path (getPath (*index.documents.at (pathIndex), element));

        jam::JsonWriter::appendKey (json, Id::scope);
        jam::JsonWriter::appendArray (json, [&path] (std::string& entries)
        {
            for (int position { 0 }; position < path.size() - 1; ++position)
            {
                jam::JsonWriter::appendSeparator (entries);
                jam::JsonWriter::appendString (entries, path.at (position));
            }
        });
    }, index);
}

void Query::documentSymbol (juce::ValueTree& request, const Index& index) const
{
    std::string json;

    jam::JsonWriter::appendArray (json, [this, &index, &request] (std::string& entries)
    {
        applyToFile (request, [this, &index, &entries] (const juce::File& file)
        {
            const auto pathIndex { index.paths.indexOf (file.getFullPathName(), false, 0) };

            if (const auto entry { index.documents.find (pathIndex) }; entry != index.documents.end())
            {
                const auto& [documentIndex, document] { *entry };

                appendDocumentSymbols (entries, pathIndex, *document->getRoot(), index);
            }
        }, index);
    });

    setResult (request, json);
}

void Query::switchSourceHeader (juce::ValueTree& request, const Index& index) const
{
    std::string json;

    jam::JsonWriter::appendArray (json, [this, &index, &request] (std::string& entries)
    {
        applyToFile (request, [this, &index, &entries] (const juce::File& file)
        {
            applyToCounterparts (file, [&entries] (const juce::File& candidate)
            {
                jam::JsonWriter::appendSeparator (entries);
                jam::JsonWriter::appendObject (entries, [&candidate] (std::string& object)
                {
                    jam::JsonWriter::appendKey (object, Id::path);
                    jam::JsonWriter::appendString (object, candidate.getFullPathName().toStdString());
                });
            }, index);
        }, index);
    });

    setResult (request, json);
}

void Query::inlayHint (juce::ValueTree& request, const Index& index) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToFile (request, [&index, &results] (const juce::File& file)
    {
        const auto pathIndex { index.paths.indexOf (file.getFullPathName(), false, 0) };

        if (const auto entry { index.documents.find (pathIndex) }; entry != index.documents.end())
        {
            const auto& [documentIndex, storedDocument] { *entry };
            const auto& document { *storedDocument };

            for (const auto* child : document)
                child->applyFunctionRecursively ([&document, &results, pathIndex] (const jam::Document::Element& element)
                {
                    if (document.hasRole (element, jam::LanguageRole::call))
                        results.add ({ pathIndex, &element });
                });
        }
    }, index);

    setResult (request, std::move (results), [this, &index] (std::string& json, int pathIndex, const jam::Document::Element& reference)
    {
        jam::JsonWriter::appendKey (json, Id::parameters);
        jam::JsonWriter::appendArray (json, [this, &index, pathIndex, &reference] (std::string& entries)
        {
            applyToFirstFunction (pathIndex, reference, [&entries] (const jam::LanguageDocument& document, const jam::Document::Element& declaration)
            {
                JsonWriter::appendParameters (entries, document, declaration);
            }, index);
        });
    }, index);
}

jam::Fuzzy::Result Query::getMatch (const jam::Fuzzy::Pattern& pattern, const jam::LanguageDocument& document, const jam::Document::Element& element) const
{
    jassert (hasName (element));

    return pattern.match (jam::Strings::fromView (document.getView (element.get<jam::Document::Tokens> (Id::name)->last())), slab);
}

Query::Matches Query::getMatches (const jam::Fuzzy::Pattern& pattern, const Index& index) const
{
    Matches scored;

    index.applyToElements ([this, &scored, &pattern] (int pathIndex, const jam::LanguageDocument& document, const jam::Document::Element& element)
    {
        if (document.hasRole (element, jam::LanguageRole::declaration) and hasName (element))
        {
            const auto match { getMatch (pattern, document, element) };

            if (match.isMatch())
                scored.add ({ match.score, pathIndex, &element });
        }
    });

    std::sort (scored.begin(), scored.end(), [this, &index] (const auto& left, const auto& right)
    {
        const auto& [leftScore, leftIndex, leftElement] { left };
        const auto& [rightScore, rightIndex, rightElement] { right };

        return leftScore != rightScore ? leftScore > rightScore : isBefore (leftIndex, *leftElement, rightIndex, *rightElement, index);
    });

    return scored;
}

Query::Matches Query::getCompletions (const jam::LanguageDocument& document, const jam::Document::Element& reference, const jam::Fuzzy::Pattern& pattern, const Index& index) const
{
    Matches completions;

    applyToCandidates (document, reference, [this, &completions, &pattern] (int pathIndex, const jam::LanguageDocument& declarationDocument, const jam::Document::Element& declaration)
    {
        const auto match { getMatch (pattern, declarationDocument, declaration) };

        if (match.isMatch())
            completions.add ({ match.score, pathIndex, &declaration });
    }, index);

    std::sort (completions.begin(), completions.end(), [this, &index] (const auto& left, const auto& right)
    {
        const auto& [leftScore, leftIndex, leftElement] { left };
        const auto& [rightScore, rightIndex, rightElement] { right };

        return leftScore != rightScore ? leftScore > rightScore : isBeforeByPath (leftIndex, *leftElement, rightIndex, *rightElement, index);
    });

    return completions;
}

void Query::workspaceSymbol (juce::ValueTree& request, const Index& index) const
{
    static constexpr int queryArgumentCount { 1 };

    const auto& arguments { *request[Id::arguments].getArray() };
    std::string json;

    jam::JsonWriter::appendArray (json, [this, &index, &arguments] (std::string& entries)
    {
        if (arguments.size() == queryArgumentCount)
        {
            const auto pattern { jam::Fuzzy::Pattern::parse (arguments.getFirst().toString(), jam::Fuzzy::CaseMode::smart, shouldNormalize, jam::Fuzzy::Scheme::standard) };

            for (const auto& [score, pathIndex, element] : getMatches (pattern, index))
                JsonWriter::appendElement (entries, index.paths.getView (pathIndex), *index.documents.at (pathIndex), *element);
        }
        else
        {
            index.model.setMessage (text::Diagnostics::failArguments);
        }
    });

    setResult (request, json);
}

void Query::completion (juce::ValueTree& request, const Index& index) const
{
    static constexpr int cursorOffset { -1 };

    std::string json;

    jam::JsonWriter::appendArray (json, [this, &index, &request] (std::string& entries)
    {
        applyToPosition (request, [this, &index, &entries] (int, const jam::LanguageDocument& document, const jam::Document::Element& element)
        {
            if (document.hasRole (element, jam::LanguageRole::reference))
            {
                const auto pattern { jam::Fuzzy::Pattern::parse (jam::Strings::fromView (document.getView (element.get<jam::Document::Tokens> (Id::name)->last())), jam::Fuzzy::CaseMode::smart, shouldNormalize, jam::Fuzzy::Scheme::standard) };

                applyToCompletions (document, element, pattern, [this, &index, &entries] (int pathIndex, const jam::LanguageDocument& declarationDocument, const jam::Document::Element& declaration, const jam::LanguageDocument& commentDocument, const jam::Document::Element& commentDeclaration)
                {
                    appendCompletion (entries, pathIndex, declarationDocument, declaration, commentDocument, commentDeclaration, index);
                }, index);
            }
        }, index, cursorOffset);
    });

    setResult (request, json);
}

void Query::goToImplementation (juce::ValueTree& request, const Index& index) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToPosition (request, [this, &index, &results] (int pathIndex, const jam::LanguageDocument&, const jam::Document::Element& element)
    {
        applyToDeclarations (element, [this, &index, &results] (int declarationIndex, const jam::LanguageDocument&, const jam::Document::Element& declaration)
        {
            applyToImplementations (declarationIndex, declaration, [&results] (int pathIndex, const jam::LanguageDocument&, const jam::Document::Element& implementation)
            {
                results.add ({ pathIndex, &implementation });
            }, index);
        }, index);
    }, index);

    setResult (request, std::move (results), index);
}

void Query::prepareCallHierarchy (juce::ValueTree& request, const Index& index) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToFunctions (request, [&results] (int declarationIndex, const jam::LanguageDocument&, const jam::Document::Element& declaration)
    {
        results.add ({ declarationIndex, &declaration });
    }, index);

    setResult (request, std::move (results), index);
}

void Query::incomingCalls (juce::ValueTree& request, const Index& index) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToFunctions (request, [this, &index, &results] (int declarationIndex, const jam::LanguageDocument&, const jam::Document::Element& declaration)
    {
        applyToReferences (declaration, [&results] (int referenceIndex, const jam::LanguageDocument& document, const jam::Document::Element& reference)
        {
            const auto* name { reference.parent->find<jam::Document::Tokens> (Id::name) };

            if (document.hasRole (reference, jam::LanguageRole::call) and name != nullptr and not name->isEmpty())
                results.add ({ referenceIndex, reference.parent });
        }, index);
    }, index);

    setResult (request, std::move (results), index);
}

void Query::outgoingCalls (juce::ValueTree& request, const Index& index) const
{
    jam::Array<std::pair<int, const jam::Document::Element*>> results;

    applyToFunctions (request, [this, &index, &results] (int declarationIndex, const jam::LanguageDocument& document, const jam::Document::Element& declaration)
    {
        declaration.applyFunctionRecursively ([this, &index, &results, &document, declarationIndex] (const jam::Document::Element& element)
        {
            if (document.hasRole (element, jam::LanguageRole::call))
                applyToDeclarations (element, [&results] (int pathIndex, const jam::LanguageDocument&, const jam::Document::Element& candidate)
                {
                    results.add ({ pathIndex, &candidate });
                }, index);
        });
    }, index);

    setResult (request, std::move (results), index);
}
