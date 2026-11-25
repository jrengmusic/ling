#include "Query.h"

void Query::setError (juce::ValueTree& request, int code, std::string_view messageText)
{
    juce::MemoryBlock json;

    jam::JsonWriter::appendObject (json, [&request, code, messageText] (juce::MemoryBlock& message)
    {
        appendJsonRpc (message);

        jam::JsonWriter::appendKey (message, lsp::Member::id);

        if (request.hasProperty (Id::id))
            jam::Format::appendBytes (message, jam::Strings::getView (request[Id::id].toString()));
        else
            jam::JsonWriter::appendNull (message);

        jam::JsonWriter::appendKey (message, lsp::Member::error);
        jam::JsonWriter::appendObject (message, [code, messageText] (juce::MemoryBlock& members)
        {
            jam::JsonWriter::appendKey (members, lsp::Member::code);
            jam::Format::appendDecimal (members, code);

            jam::JsonWriter::appendKey (members, lsp::Member::message);
            jam::JsonWriter::appendString (members, messageText, juce::JSON::Encoding::utf8);
        });
    });

    request.setProperty (Id::result, juce::var (std::move (json)), nullptr);
}

void Query::setError (juce::ValueTree& request, int code)
{
    const auto command { request[Id::command].toString() };

    setError (request, code, jam::Strings::getView (command));
}

juce::Identifier Query::getCommand (const juce::ValueTree& request)
{
    const auto word { request[Id::command].toString() };

    return word.isNotEmpty() ? juce::Identifier { word } : juce::Identifier {};
}

juce::Identifier Query::getArgumentShape (const juce::Identifier& operation)
{
    static const map::Parameters parameters;

    if (const auto entry { parameters.find (operation) }; entry != parameters.end())
    {
        const auto& [operationKey, shape] { *entry };

        return shape;
    }

    return {};
}

bool Query::isValidRequest (const juce::ValueTree& request)
{
    static constexpr int positionArgumentCount { 3 };
    static constexpr int rangeArgumentCount { 5 };
    static constexpr int fileArgumentCount { 1 };

    const auto& arguments { *request[Id::arguments].getArray() };
    const auto shape { getArgumentShape (getCommand (request)) };

    if (shape == Id::position)
        return arguments.size() == positionArgumentCount and static_cast<int> (arguments[lineIndex]) > 0 and static_cast<int> (arguments[columnIndex]) > 0;

    if (shape == Id::range)
        return arguments.size() == rangeArgumentCount and static_cast<int> (arguments[lineIndex]) > 0 and static_cast<int> (arguments[columnIndex]) > 0
               and static_cast<int> (arguments[endLineIndex]) > 0 and static_cast<int> (arguments[endColumnIndex]) > 0;

    return not shape.isNull() and arguments.size() == fileArgumentCount;
}

juce::File Query::getCounterpart (const juce::File& file, const Index& index)
{
    const auto fileRows { index.model.state.getChildWithName (jam::Format::toType (Id::files)) };
    juce::ValueTree fileRow;

    if (const auto entry { index.pathRows.find (file.getFullPathName()) }; entry != index.pathRows.end())
    {
        const auto& [rowPath, rowIndex] { *entry };

        fileRow = fileRows.getChild (rowIndex);
    }

    if (fileRow.isValid())
    {
        if (const auto entry { index.languages.find (static_cast<int> (fileRow[Id::family])) }; entry != index.languages.end())
        {
            const auto& [languageFamily, language] { *entry };
            const auto extensions { language->getCounterparts (file.getFileExtension().substring (1)) };

            for (const auto& extension : extensions)
            {
                const auto sameFolderCandidate { file.withFileExtension (extension) };

                if (index.paths.indexOf (sameFolderCandidate.getFullPathName(), false, 0) >= 0)
                    return sameFolderCandidate;
            }

            if (const auto stemEntry { index.stemRows.find (file.getFileNameWithoutExtension()) }; stemEntry != index.stemRows.end())
            {
                const auto& [stem, stemRowIndices] { *stemEntry };

                for (const auto rowIndex : stemRowIndices)
                {
                    const juce::File candidate { fileRows.getChild (rowIndex)[Id::path].toString() };

                    if (extensions.contains (candidate.getFileExtension().substring (1), false))
                        return candidate;
                }
            }
        }
    }

    return {};
}

juce::File Query::getFile (const juce::ValueTree& request)
{
    return juce::File { request[Id::directory].toString() }.getChildFile ((*request[Id::arguments].getArray())[fileIndex].toString());
}

int Query::getPathIndex (const juce::ValueTree& request, const Index& index) const
{
    return index.paths.indexOf (getFile (request).getFullPathName(), false, 0);
}

int Query::getLastTokenIndex (const jam::Document::Element& element)
{
    return element.get<jam::Document::Tokens> (Id::nameTokens)->size() - 1;
}

bool Query::isBefore (int pathIndex, const jam::Document::Element& element, int tokenIndex, int otherPathIndex, const jam::Document::Element& otherElement, int otherTokenIndex, const Index& index) const
{
    const auto order { index.paths.getView (pathIndex).compare (index.paths.getView (otherPathIndex)) };

    if (order == 0)
        return element.get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex).offset() < otherElement.get<jam::Document::Tokens> (Id::nameTokens)->at (otherTokenIndex).offset();

    return order < 0;
}

void Query::appendPosition (juce::MemoryBlock& json, const jam::Document& document, size_t offset, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [&document, offset, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::line);
        jam::Format::appendDecimal (members, static_cast<int> (document.getLineNumber (offset)) - 1);

        jam::JsonWriter::appendKey (members, lsp::Member::character);
        jam::Format::appendDecimal (members, Index::getCharacter (document, offset, encoding));
    });
}

void Query::appendRange (juce::MemoryBlock& json, const jam::Document& document, size_t startOffset, size_t endOffset, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [&document, startOffset, endOffset, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::start);
        appendPosition (members, document, startOffset, encoding);

        jam::JsonWriter::appendKey (members, lsp::Member::end);
        appendPosition (members, document, endOffset, encoding);
    });
}

void Query::appendUri (juce::MemoryBlock& json, const juce::File& file)
{
    jam::JsonWriter::appendString (json, jam::Strings::getView (juce::URL (file).toString (false)));
}

void Query::appendLocation (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Token& token, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [&path, &document, &token, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::uri);
        appendUri (members, juce::File (path));

        jam::JsonWriter::appendKey (members, lsp::Member::range);
        appendRange (members, document, token.span.getStart(), token.span.getEnd(), encoding);
    });
}

void Query::appendNameLocation (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, std::string_view encoding)
{
    appendLocation (json, path, document, element.get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex), encoding);
}

void Query::appendItem (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Index& index, std::string_view encoding)
{
    const auto& token { element.get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex) };

    jam::JsonWriter::appendObject (json, [&path, &document, &element, &token, &index, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::name);
        jam::JsonWriter::appendString (members, document.getView (token));

        jam::JsonWriter::appendKey (members, lsp::Member::type);
        jam::Format::appendDecimal (members, index.getSymbolType (document, element));

        jam::JsonWriter::appendKey (members, lsp::Member::uri);
        appendUri (members, juce::File (path));

        jam::JsonWriter::appendKey (members, lsp::Member::range);
        appendRange (members, document, token.span.getStart(), token.span.getEnd(), encoding);

        jam::JsonWriter::appendKey (members, lsp::Member::selectionRange);
        appendRange (members, document, token.span.getStart(), token.span.getEnd(), encoding);
    });
}

void Query::appendHighlight (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Token& token, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [&document, &token, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::range);
        appendRange (members, document, token.span.getStart(), token.span.getEnd(), encoding);

        jam::JsonWriter::appendKey (members, lsp::Member::type);
        jam::Format::appendDecimal (members, lsp::DocumentHighlightType::Text);
    });
}

void Query::appendTextEdit (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Token& token, std::string_view newName, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [&document, &token, newName, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::range);
        appendRange (members, document, token.span.getStart(), token.span.getEnd(), encoding);

        jam::JsonWriter::appendKey (members, lsp::Member::newText);
        jam::JsonWriter::appendString (members, newName);
    });
}

const Query::Occurrences& Query::getOccurrences (const jam::HashMap<juce::int64, Occurrences>& map, juce::int64 symbol)
{
    static const Occurrences none;

    if (const auto entry { map.find (symbol) }; entry != map.end())
    {
        const auto& [entrySymbol, occurrences] { *entry };

        return occurrences;
    }

    return none;
}

int Query::getOccurrencePath (const Occurrence& occurrence)
{
    const auto& [occurrencePathIndex, occurrenceRecord] { occurrence };

    return occurrencePathIndex;
}

bool Query::isOccurrenceBefore (const Index& index, const Occurrence& left, const Occurrence& right)
{
    const auto& [leftPathIndex, leftRecord] { left };
    const auto& [rightPathIndex, rightRecord] { right };

    return index.isOccurrenceBefore (leftPathIndex, *leftRecord, rightPathIndex, *rightRecord);
}

void Query::appendOccurrenceEdit (juce::MemoryBlock& edits, const Index& index, const Occurrence& occurrence, std::string_view newName, std::string_view encoding)
{
    const auto& [occurrencePathIndex, occurrenceRecord] { occurrence };

    if (const auto stored { index.documents.find (occurrencePathIndex) }; stored != index.documents.end())
    {
        const auto& [documentIndex, document] { *stored };
        const auto [element, tokenIndex] { Index::getElement (*document, occurrenceRecord->line, occurrenceRecord->column) };

        if (element != nullptr)
        {
            jam::JsonWriter::appendSeparator (edits);
            appendTextEdit (edits, *document, element->get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex), newName, encoding);
        }
    }
}

void Query::appendWorkspaceEdit (juce::MemoryBlock& json, const Index& index, const Occurrences& declarations, const Occurrences& references, std::string_view newName, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [&index, &declarations, &references, newName, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::changes);
        jam::JsonWriter::appendObject (members, [&index, &declarations, &references, newName, encoding] (juce::MemoryBlock& changes)
        {
            appendChanges (changes, index, declarations, references, newName, encoding);
        });
    });
}

void Query::appendChanges (juce::MemoryBlock& json, const Index& index, const Occurrences& declarations, const Occurrences& references, std::string_view newName, std::string_view encoding)
{
    int declarationPosition { 0 };
    int referencePosition { 0 };

    while (declarationPosition < declarations.size() or referencePosition < references.size())
    {
        const auto takesDeclaration { declarationPosition < declarations.size() and (referencePosition == references.size() or isOccurrenceBefore (index, declarations.at (declarationPosition), references.at (referencePosition))) };
        const auto runPathIndex { getOccurrencePath (takesDeclaration ? declarations.at (declarationPosition) : references.at (referencePosition)) };

        jam::JsonWriter::appendSeparator (json);
        appendUri (json, juce::File (index.paths.at (runPathIndex)));
        jam::Format::appendUtf8 (json, Chars::colon);

        jam::JsonWriter::appendArray (json, [&index, &declarations, &references, &declarationPosition, &referencePosition, runPathIndex, newName, encoding] (juce::MemoryBlock& edits)
        {
            const auto isOnRun { [runPathIndex] (const Occurrences& occurrences, int position)
            {
                return position < occurrences.size() and getOccurrencePath (occurrences.at (position)) == runPathIndex;
            } };

            while (isOnRun (declarations, declarationPosition) or isOnRun (references, referencePosition))
            {
                if (isOnRun (declarations, declarationPosition) and (not isOnRun (references, referencePosition) or isOccurrenceBefore (index, declarations.at (declarationPosition), references.at (referencePosition))))
                {
                    appendOccurrenceEdit (edits, index, declarations.at (declarationPosition), newName, encoding);
                    ++declarationPosition;
                }
                else
                {
                    appendOccurrenceEdit (edits, index, references.at (referencePosition), newName, encoding);
                    ++referencePosition;
                }
            }
        });
    }
}

void Query::goToDefinition (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToPosition (request, [this, &index, &entries, encoding] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            applyToDeclarations (pathIndex, document, element, tokenIndex, [&index, &entries, encoding] (int declarationIndex, const jam::Document& declarationDocument, const jam::Document::Element& declaration, int declarationTokenIndex)
            {
                jam::JsonWriter::appendSeparator (entries);
                appendNameLocation (entries, index.paths.at (declarationIndex), declarationDocument, declaration, declarationTokenIndex, encoding);
            }, index);
        }, index, encoding);
    });
}

void Query::findReferences (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToPosition (request, [this, &index, &entries, encoding] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            applyToSymbols (pathIndex, document, element, tokenIndex, [this, &index, &entries, encoding] (const SymbolDocument::SymbolRecord& record)
            {
                applyToSymbolOccurrences (record.symbol, [&index, &entries, encoding] (int occurrenceIndex, const jam::Document& occurrenceDocument, const jam::Document::Element& occurrence, int occurrenceTokenIndex)
                {
                    jam::JsonWriter::appendSeparator (entries);
                    appendNameLocation (entries, index.paths.at (occurrenceIndex), occurrenceDocument, occurrence, occurrenceTokenIndex, encoding);
                }, index);
            }, index);
        }, index, encoding);
    });
}

void Query::documentHighlight (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToPosition (request, [this, &index, &entries, encoding] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            applyToSymbols (pathIndex, document, element, tokenIndex, [this, &index, &entries, encoding, pathIndex] (const SymbolDocument::SymbolRecord& record)
            {
                applyToSymbolOccurrences (record.symbol, pathIndex, [&entries, encoding] (int, const jam::Document& occurrenceDocument, const jam::Document::Element& occurrence, int occurrenceTokenIndex)
                {
                    jam::JsonWriter::appendSeparator (entries);
                    appendHighlight (entries, occurrenceDocument, occurrence.get<jam::Document::Tokens> (Id::nameTokens)->at (occurrenceTokenIndex), encoding);
                }, index);
            }, index);
        }, index, encoding);
    });
}

void Query::rename (juce::ValueTree& request, const Index& index) const
{
    const auto& encodingText { request[Id::positionEncoding].toString() };
    const auto encoding { jam::Strings::getView (encodingText) };
    const auto newName { request[Id::newName].toString() };
    const SymbolDocument::SymbolRecord* symbolRecord {};

    applyToPosition (request, [this, &index, &symbolRecord] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
    {
        applyToSymbols (pathIndex, document, element, tokenIndex, [&symbolRecord] (const SymbolDocument::SymbolRecord& record)
        {
            if (symbolRecord == nullptr)
                symbolRecord = &record;
        }, index);
    }, index, encoding);

    const auto symbol { symbolRecord != nullptr ? symbolRecord->symbol : SymbolDocument::noSymbol };
    const auto& declarations { getOccurrences (index.declarations, symbol) };
    const auto& references { getOccurrences (index.references, symbol) };

    setResult (request, [&index, &newName, encoding, &declarations, &references] (juce::MemoryBlock& message)
    {
        if (declarations.isEmpty() and references.isEmpty())
            jam::JsonWriter::appendNull (message);
        else
            appendWorkspaceEdit (message, index, declarations, references, jam::Strings::getView (newName), encoding);
    });
}

void Query::hover (juce::ValueTree& request, const Index& index) const
{
    const auto& encodingText { request[Id::positionEncoding].toString() };
    const auto encoding { jam::Strings::getView (encodingText) };
    int hoverPathIndex { 0 };
    const jam::Document::Element* hoverElement { nullptr };
    int hoverTokenIndex { 0 };
    const SymbolDocument::SymbolRecord* hoverRecord { nullptr };

    applyToPosition (request, [this, &index, &hoverPathIndex, &hoverElement, &hoverTokenIndex, &hoverRecord] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
    {
        applyToSymbols (pathIndex, document, element, tokenIndex, [&hoverRecord] (const SymbolDocument::SymbolRecord& record)
        {
            if (hoverRecord == nullptr and record.hover != jam::LanguageCallbacks::noHover)
                hoverRecord = &record;
        }, index);

        if (hoverRecord != nullptr)
        {
            hoverPathIndex = pathIndex;
            hoverElement = &element;
            hoverTokenIndex = tokenIndex;
        }
    }, index, encoding);

    setResult (request, [&index, hoverPathIndex, hoverElement, hoverTokenIndex, hoverRecord, encoding] (juce::MemoryBlock& message)
    {
        if (hoverRecord != nullptr)
        {
            const auto& hoverDocument { *index.documents.at (hoverPathIndex) };
            const auto& hoverToken { hoverElement->get<jam::Document::Tokens> (Id::nameTokens)->at (hoverTokenIndex) };
            const auto& [unitRow, file] { index.symbolFiles.at (hoverPathIndex) };
            const auto& symbolDocument { *index.symbolDocuments.at (unitRow) };
            const auto& hoverEntry { symbolDocument.getHovers (*file).at (static_cast<uint32_t> (hoverRecord->hover)) };
            const auto hover { symbolDocument.getString (hoverEntry.hoverOffset, hoverEntry.hoverLength) };
            const auto definitionPath { symbolDocument.getString (hoverEntry.pathOffset, hoverEntry.pathLength) };
            std::string_view definition {};

            if (not definitionPath.empty())
            {
                if (const auto stored { index.documents.find (index.paths.indexOf (jam::Strings::fromView (definitionPath), false, 0)) }; stored != index.documents.end())
                {
                    const auto& [definitionIndex, definitionDocument] { *stored };

                    definition = definitionDocument->getView (jam::Document::Token (definitionDocument->getByteOffset (static_cast<uint32_t> (hoverEntry.line), static_cast<uint32_t> (hoverEntry.column)),
                                                                                     definitionDocument->getByteOffset (static_cast<uint32_t> (hoverEntry.endLine), static_cast<uint32_t> (hoverEntry.endColumn))));
                }
            }

            jam::JsonWriter::appendObject (message, [&hover, &definition, &hoverDocument, &hoverToken, encoding] (juce::MemoryBlock& members)
            {
                jam::JsonWriter::appendKey (members, lsp::Member::contents);
                jam::JsonWriter::appendObject (members, [&hover, &definition] (juce::MemoryBlock& contents)
                {
                    jam::JsonWriter::appendKey (contents, lsp::Member::type);
                    jam::JsonWriter::appendString (contents, lsp::MarkupType::PlainText);

                    jam::JsonWriter::appendKey (contents, lsp::Member::value);
                    jam::Format::appendUtf8 (contents, Chars::doubleQuote);
                    jam::JsonWriter::appendStringBody (contents, hover);
                    jam::JsonWriter::appendStringBody (contents, definition);
                    jam::Format::appendUtf8 (contents, Chars::doubleQuote);
                });

                jam::JsonWriter::appendKey (members, lsp::Member::range);
                appendRange (members, hoverDocument, hoverToken.span.getStart(), hoverToken.span.getEnd(), encoding);
            });
        }
        else
        {
            jam::JsonWriter::appendNull (message);
        }
    });
}

void Query::semanticTokens (juce::ValueTree& request, const Index& index) const
{
    applyToDocument (request, index, [this, &index] (juce::MemoryBlock& message, int pathIndex, const jam::Document& document, std::string_view encoding)
    {
        appendSemanticTokens (message, pathIndex, index, encoding, 0, document.getSource().size());
    });
}

void Query::semanticTokensRange (juce::ValueTree& request, const Index& index) const
{
    const auto& arguments { *request[Id::arguments].getArray() };

    applyToDocument (request, index, [this, &index, &arguments] (juce::MemoryBlock& message, int pathIndex, const jam::Document& document, std::string_view encoding)
    {
        const auto startOffset { getOffset (document, static_cast<int> (arguments[lineIndex]), static_cast<int> (arguments[columnIndex]), encoding) };
        const auto endOffset { getOffset (document, static_cast<int> (arguments[endLineIndex]), static_cast<int> (arguments[endColumnIndex]), encoding) };

        appendSemanticTokens (message, pathIndex, index, encoding, startOffset, endOffset);
    });
}

void Query::selectionRange (juce::ValueTree& request, const Index& index) const
{
    const auto& arguments { *request[Id::arguments].getArray() };

    applyToDocument (request, index, [&arguments] (juce::MemoryBlock& message, int, const jam::Document& document, std::string_view encoding)
    {
        const auto offset { getOffset (document, static_cast<int> (arguments[lineIndex]), static_cast<int> (arguments[columnIndex]), encoding) };

        jam::JsonWriter::appendArray (message, [&document, offset, encoding] (juce::MemoryBlock& entries)
        {
            appendSelectionRanges (entries, document, offset, encoding);
        });
    });
}

void Query::textObject (juce::ValueTree& request, const Index& index) const
{
    const auto typeName { request[Id::type].toString() };
    const juce::Identifier scopeName { request[Id::scope].toString() };
    const auto& arguments { *request[Id::arguments].getArray() };

    applyToDocument (request, index, [&index, &arguments, typeName, scopeName] (juce::MemoryBlock& message, int, const jam::Document& document, std::string_view encoding)
    {
        const auto& language { *index.languages.at (*document.getRoot()->get<int> (Id::family)) };
        const auto range { getTextObjectRange (document, language, typeName, scopeName, getOffset (document, static_cast<int> (arguments[lineIndex]), static_cast<int> (arguments[columnIndex]), encoding)) };

        if (not range.isEmpty())
            appendRange (message, document, range.getStart(), range.getEnd(), encoding);
        else
            jam::JsonWriter::appendNull (message);
    });
}

void Query::switchSourceHeader (juce::ValueTree& request, const Index& index) const
{
    setResult (request, [&request, &index] (juce::MemoryBlock& message)
    {
        const auto counterpart { getCounterpart (getFile (request), index) };

        if (counterpart != juce::File {})
            appendUri (message, counterpart);
        else
            jam::JsonWriter::appendNull (message);
    });
}

void Query::inlayHint (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToFile (request, [&index, &entries, encoding] (const juce::File& file)
        {
            applyToSymbolFile (file, [&entries, encoding] (const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& symbolFile, const jam::Document& document)
            {
                for (const auto& hint : symbolDocument.getInlayHints (symbolFile))
                {
                    jam::JsonWriter::appendSeparator (entries);
                    jam::JsonWriter::appendObject (entries, [&symbolDocument, &hint, &document, encoding] (juce::MemoryBlock& object)
                    {
                        jam::JsonWriter::appendKey (object, lsp::Member::position);
                        appendPosition (object, document, document.getByteOffset (static_cast<uint32_t> (hint.line), static_cast<uint32_t> (hint.column)), encoding);

                        jam::JsonWriter::appendKey (object, lsp::Member::label);
                        jam::JsonWriter::appendString (object, symbolDocument.getString (hint.labelOffset, hint.labelLength));

                        jam::JsonWriter::appendKey (object, lsp::Member::type);
                        jam::Format::appendDecimal (object, hint.type);

                        jam::JsonWriter::appendKey (object, lsp::Member::paddingLeft);
                        jam::JsonWriter::appendBoolean (object, hint.paddingLeft != 0);

                        jam::JsonWriter::appendKey (object, lsp::Member::paddingRight);
                        jam::JsonWriter::appendBoolean (object, hint.paddingRight != 0);
                    });
                }
            }, index);
        });
    });
}

void Query::diagnostics (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToFile (request, [&index, &entries, encoding] (const juce::File& file)
        {
            appendDiagnostics (entries, file, index, encoding);
        });
    });
}

void Query::publishDiagnostics (juce::ValueTree& request, const Index& index) const
{
    const auto& encodingText { request[Id::positionEncoding].toString() };
    const auto encoding { jam::Strings::getView (encodingText) };

    setNotification (request, lsp::Method::publishDiagnostics, [this, &index, &request, encoding] (juce::MemoryBlock& message)
    {
        jam::JsonWriter::appendObject (message, [this, &index, &request, encoding] (juce::MemoryBlock& params)
        {
            applyToFile (request, [&index, &params, encoding] (const juce::File& file)
            {
                jam::JsonWriter::appendKey (params, lsp::Member::uri);
                appendUri (params, file);

                jam::JsonWriter::appendKey (params, lsp::Member::diagnostics);
                jam::JsonWriter::appendArray (params, [&index, &file, encoding] (juce::MemoryBlock& entries)
                {
                    appendDiagnostics (entries, file, index, encoding);
                });
            });
        });
    });
}

void Query::appendDiagnostics (juce::MemoryBlock& json, const juce::File& file, const Index& index, std::string_view encoding)
{
    applyToSymbolFile (file, [&json, encoding] (const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& symbolFile, const jam::Document& document)
    {
        for (const auto& diagnostic : symbolDocument.getDiagnostics (symbolFile))
        {
            jam::JsonWriter::appendSeparator (json);
            jam::JsonWriter::appendObject (json, [&symbolDocument, &diagnostic, &document, encoding] (juce::MemoryBlock& object)
            {
                jam::JsonWriter::appendKey (object, lsp::Member::range);
                appendRange (object, document, document.getByteOffset (static_cast<uint32_t> (diagnostic.line), static_cast<uint32_t> (diagnostic.column)), document.getByteOffset (static_cast<uint32_t> (diagnostic.endLine), static_cast<uint32_t> (diagnostic.endColumn)), encoding);

                jam::JsonWriter::appendKey (object, lsp::Member::severity);
                jam::Format::appendDecimal (object, diagnostic.severity);

                jam::JsonWriter::appendKey (object, lsp::Member::message);
                jam::JsonWriter::appendString (object, symbolDocument.getString (diagnostic.messageOffset, diagnostic.messageLength));
            });
        }
    }, index);
}

int Query::appendOutlines (juce::MemoryBlock& json, const jam::Document& document, SymbolDocument::Span<SymbolDocument::OutlineEntry> outlines, int parentIndex, int outlineIndex, std::string_view encoding)
{
    auto nextIndex { outlineIndex };

    while (nextIndex < static_cast<int> (outlines.size()) and outlines.at (static_cast<uint32_t> (nextIndex)).parent == parentIndex)
    {
        jam::JsonWriter::appendSeparator (json);
        jam::JsonWriter::appendObject (json, [&document, outlines, &nextIndex, encoding] (juce::MemoryBlock& members)
        {
            nextIndex = appendOutline (members, document, outlines, nextIndex, encoding);
        });
    }

    return nextIndex;
}

int Query::appendOutline (juce::MemoryBlock& json, const jam::Document& document, SymbolDocument::Span<SymbolDocument::OutlineEntry> outlines, int outlineIndex, std::string_view encoding)
{
    const auto& outline { outlines.at (static_cast<uint32_t> (outlineIndex)) };
    const auto line { outline.line };
    const auto column { outline.column };
    const auto nameStart { document.getByteOffset (static_cast<uint32_t> (line), static_cast<uint32_t> (column)) };
    const auto nameEnd { nameStart + static_cast<size_t> (outline.nameLength) };
    const auto start { nameStart - static_cast<size_t> (outline.qualifierLength) };
    const auto end { document.getByteOffset (static_cast<uint32_t> (outline.endLine), static_cast<uint32_t> (outline.endColumn)) };

    jam::JsonWriter::appendKey (json, lsp::Member::name);
    jam::JsonWriter::appendString (json, document.getView (jam::Document::Token (start, nameEnd)));

    jam::JsonWriter::appendKey (json, lsp::Member::type);
    jam::Format::appendDecimal (json, outline.type);

    jam::JsonWriter::appendKey (json, lsp::Member::range);
    appendRange (json, document, start, end, encoding);

    jam::JsonWriter::appendKey (json, lsp::Member::selectionRange);
    appendRange (json, document, nameStart, nameEnd, encoding);

    jam::JsonWriter::appendKey (json, lsp::Member::children);

    auto subtreeEnd { outlineIndex + 1 };

    jam::JsonWriter::appendArray (json, [&document, outlines, &subtreeEnd, outlineIndex, encoding] (juce::MemoryBlock& entries)
    {
        subtreeEnd = appendOutlines (entries, document, outlines, outlineIndex, outlineIndex + 1, encoding);
    });

    return subtreeEnd;
}

void Query::appendScopeCompletions (juce::MemoryBlock& json, juce::int64 scopeSymbol, const jam::Fuzzy::Pattern& pattern, const Language& language, const Index& index) const
{
    if (const auto entry { index.completions.find (scopeSymbol) }; entry != index.completions.end())
    {
        const auto& [entrySymbol, scopeCompletions] { *entry };

        for (const auto& [completionPathIndex, completion] : scopeCompletions)
        {
            const auto& symbolDocument { index.getSymbolDocument (completionPathIndex) };

            if (pattern.match (symbolDocument.getString (completion->labelOffset, completion->labelLength), slab).isMatch())
                appendCompletionItem (json, symbolDocument, *completion, language);
        }
    }
}

void Query::appendCompletionItem (juce::MemoryBlock& json, const SymbolDocument& symbolDocument, const SymbolDocument::CompletionRecord& completion, const Language& language)
{
    const auto snippet { symbolDocument.getString (completion.snippetOffset, completion.snippetLength) };

    jam::JsonWriter::appendSeparator (json);
    jam::JsonWriter::appendObject (json, [&symbolDocument, &completion, &language, snippet] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::label);
        jam::JsonWriter::appendString (members, symbolDocument.getString (completion.labelOffset, completion.labelLength));

        jam::JsonWriter::appendKey (members, lsp::Member::type);
        jam::Format::appendDecimal (members, language.getCompletionType (completion.type));

        jam::JsonWriter::appendKey (members, lsp::Member::insertText);
        jam::JsonWriter::appendString (members, snippet);

        jam::JsonWriter::appendKey (members, lsp::Member::insertTextFormat);
        jam::Format::appendDecimal (members, snippet.find (static_cast<char> (Chars::dollarSign)) == std::string_view::npos ? lsp::InsertTextFormat::PlainText : lsp::InsertTextFormat::Snippet);
    });
}

void Query::appendFixItActions (juce::MemoryBlock& json, const juce::File& file, const jam::Document& document, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& symbolFile, size_t rangeStart, size_t rangeEnd, std::string_view encoding)
{
    for (const auto& fixIt : symbolDocument.getFixIts (symbolFile))
    {
        const auto fixStart { document.getByteOffset (static_cast<uint32_t> (fixIt.line), static_cast<uint32_t> (fixIt.column)) };
        const auto fixEnd { document.getByteOffset (static_cast<uint32_t> (fixIt.endLine), static_cast<uint32_t> (fixIt.endColumn)) };

        if (fixStart <= rangeEnd and rangeStart <= fixEnd)
        {
            jam::JsonWriter::appendSeparator (json);
            appendCodeAction (json, file, document, jam::Document::Token (fixStart, fixEnd), symbolDocument.getString (fixIt.textOffset, fixIt.textLength), encoding);
        }
    }
}

void Query::appendCodeAction (juce::MemoryBlock& json, const juce::File& file, const jam::Document& document, const jam::Document::Token& range, std::string_view newText, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [&file, &document, &range, newText, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::title);
        jam::JsonWriter::appendString (members, text::CodeAction::fixItTitle);

        jam::JsonWriter::appendKey (members, lsp::Member::type);
        jam::JsonWriter::appendString (members, lsp::CodeActionKind::QuickFix);

        jam::JsonWriter::appendKey (members, lsp::Member::edit);
        jam::JsonWriter::appendObject (members, [&file, &document, &range, newText, encoding] (juce::MemoryBlock& edit)
        {
            jam::JsonWriter::appendKey (edit, lsp::Member::changes);
            jam::JsonWriter::appendObject (edit, [&file, &document, &range, newText, encoding] (juce::MemoryBlock& changes)
            {
                jam::JsonWriter::appendSeparator (changes);
                appendUri (changes, file);
                jam::Format::appendUtf8 (changes, Chars::colon);

                jam::JsonWriter::appendArray (changes, [&document, &range, newText, encoding] (juce::MemoryBlock& textEdits)
                {
                    jam::JsonWriter::appendSeparator (textEdits);
                    appendTextEdit (textEdits, document, range, newText, encoding);
                });
            });
        });
    });
}

void Query::typeDefinition (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToPosition (request, [&index, &entries, encoding] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            index.applyToRecords (pathIndex, document, element, tokenIndex, [&index, &entries, pathIndex, encoding] (const SymbolDocument::SymbolRecord& record)
            {
                for (const auto typeSymbol : index.getSymbolDocument (pathIndex).getTypeSymbols (record))
                {
                    if (const auto entry { index.declarations.find (typeSymbol) }; entry != index.declarations.end())
                    {
                        const auto& [declarationSymbol, occurrences] { *entry };
                        int definitionPathIndex { 0 };
                        const jam::Document* definitionDocument { nullptr };
                        const jam::Document::Element* definitionElement { nullptr };
                        int definitionTokenIndex { 0 };
                        int declarationPathIndex { 0 };
                        const jam::Document* declarationDocument { nullptr };
                        const jam::Document::Element* declarationElement { nullptr };
                        int declarationTokenIndex { 0 };

                        index.applyToOccurrences (occurrences, [&definitionPathIndex, &definitionDocument, &definitionElement, &definitionTokenIndex, &declarationPathIndex, &declarationDocument, &declarationElement, &declarationTokenIndex] (int occurrencePathIndex, const jam::Document& occurrenceDocument, const jam::Document::Element& occurrenceElement, int occurrenceTokenIndex, const SymbolDocument::SymbolRecord& occurrenceRecord)
                        {
                            if (declarationElement == nullptr)
                            {
                                declarationPathIndex = occurrencePathIndex;
                                declarationDocument = &occurrenceDocument;
                                declarationElement = &occurrenceElement;
                                declarationTokenIndex = occurrenceTokenIndex;
                            }

                            if (definitionElement == nullptr and (occurrenceRecord.role & jam::LanguageRole::definition) != 0)
                            {
                                definitionPathIndex = occurrencePathIndex;
                                definitionDocument = &occurrenceDocument;
                                definitionElement = &occurrenceElement;
                                definitionTokenIndex = occurrenceTokenIndex;
                            }
                        });

                        if (definitionElement != nullptr)
                        {
                            jam::JsonWriter::appendSeparator (entries);
                            appendNameLocation (entries, index.paths.at (definitionPathIndex), *definitionDocument, *definitionElement, definitionTokenIndex, encoding);
                        }
                        else if (declarationElement != nullptr)
                        {
                            jam::JsonWriter::appendSeparator (entries);
                            appendNameLocation (entries, index.paths.at (declarationPathIndex), *declarationDocument, *declarationElement, declarationTokenIndex, encoding);
                        }
                    }
                }
            });
        }, index, encoding);
    });
}

void Query::appendFileCodeActions (juce::MemoryBlock& json, const juce::File& file, const Index& index, const juce::Array<juce::var>& arguments, std::string_view encoding)
{
    applyToSymbolFile (file, [&json, &file, &arguments, encoding] (const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& symbolFile, const jam::Document& document)
    {
        const auto rangeStart { getOffset (document, static_cast<int> (arguments[lineIndex]), static_cast<int> (arguments[columnIndex]), encoding) };
        const auto rangeEnd { getOffset (document, static_cast<int> (arguments[endLineIndex]), static_cast<int> (arguments[endColumnIndex]), encoding) };

        appendFixItActions (json, file, document, symbolDocument, symbolFile, rangeStart, rangeEnd, encoding);
    }, index);
}

void Query::codeAction (juce::ValueTree& request, const Index& index) const
{
    const auto& arguments { *request[Id::arguments].getArray() };

    applyToArrayResult (request, [this, &index, &request, &arguments] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToFile (request, [&index, &entries, &arguments, encoding] (const juce::File& file)
        {
            appendFileCodeActions (entries, file, index, arguments, encoding);
        });
    });
}

void Query::documentSymbol (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToFile (request, [&index, &entries, encoding] (const juce::File& file)
        {
            applyToSymbolFile (file, [&entries, encoding] (const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& symbolFile, const jam::Document& document)
            {
                appendOutlines (entries, document, symbolDocument.getOutlines (symbolFile), SymbolDocument::noParent, 0, encoding);
            }, index);
        });
    });
}

void Query::completion (juce::ValueTree& request, const Index& index) const
{
    static constexpr int cursorOffset { -1 };

    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToPosition (request, [this, &index, &entries] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            std::string prefixQuery (1, static_cast<char> (Chars::caret));
            prefixQuery.append (document.getView (element.get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex)));
            const auto pattern { jam::Fuzzy::Pattern::parse (prefixQuery, jam::Fuzzy::CaseMode::smart, shouldNormalize, jam::Fuzzy::Scheme::standard) };
            const auto& language { *index.languages.at (*document.getRoot()->get<int> (Id::family)) };

            applyToScopeSymbols (pathIndex, document, element, tokenIndex, [this, &index, &entries, &pattern, &language] (juce::int64 scopeSymbol)
            {
                appendScopeCompletions (entries, scopeSymbol, pattern, language, index);
            }, index);
        }, index, encoding, cursorOffset);
    });
}

int Query::getRecordType (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Index& index) const
{
    const auto ownType { *element.get<int> (Id::type) };

    if (index.hasRole (document, element, jam::LanguageRole::declaration))
        return ownType;

    const SymbolDocument::SymbolRecord* symbolRecord {};

    applyToSymbols (pathIndex, document, element, tokenIndex, [&symbolRecord] (const SymbolDocument::SymbolRecord& record)
    {
        if (symbolRecord == nullptr)
            symbolRecord = &record;
    }, index);

    if (symbolRecord != nullptr)
    {
        if (const auto entry { index.declarations.find (symbolRecord->symbol) }; entry != index.declarations.end())
        {
            const auto& [declarationSymbol, symbolDeclarations] { *entry };

            if (not symbolDeclarations.isEmpty())
                if (const auto [declaration, declarationTokenIndex] { index.getElement (symbolDeclarations.first()) }; declaration != nullptr)
                    return *declaration->get<int> (Id::type);
        }
    }

    return ownType;
}

void Query::appendSemanticTokens (juce::MemoryBlock& json, int pathIndex, const Index& index, std::string_view encoding, size_t startOffset, size_t endOffset) const
{
    const auto& document { *index.documents.at (pathIndex) };
    const auto& language { *index.languages.at (*document.getRoot()->get<int> (Id::family)) };
    const auto& stream { *document.getRoot()->get<jam::Document::Tokens> (Id::tokens) };
    const auto first { std::lower_bound (stream.begin(), stream.end(), startOffset, [] (const jam::Document::Token& token, size_t offset) { return token.offset() + token.length() <= offset; }) };
    size_t previousOffset { 0 };

    jam::JsonWriter::appendObject (json, [this, pathIndex, &index, &document, &language, &stream, first, startOffset, endOffset, encoding, &previousOffset] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::data);
        jam::JsonWriter::appendArray (members, [this, pathIndex, &index, &document, &language, &stream, first, startOffset, endOffset, encoding, &previousOffset] (juce::MemoryBlock& entries)
        {
            for (auto token { first }; token != stream.end() and token->offset() < endOffset; ++token)
            {
                const auto tokenEnd { token->offset() + token->length() };

                const auto [element, tokenIndex] { language.hasIndex() ? document.getNameToken (token->offset()) : std::pair<const jam::Document::Element*, int> { nullptr, -1 } };
                const auto tokenType { element != nullptr ? getRecordType (pathIndex, document, *element, tokenIndex, index) : token->type };

                previousOffset = appendTokenLines (entries, document, previousOffset, std::max (token->offset(), startOffset), std::min (tokenEnd, endOffset), language.getSemanticType (tokenType), language.getSemanticModifiers (tokenType), encoding);
            }
        });
    });
}

size_t Query::appendTokenLines (juce::MemoryBlock& json, const jam::Document& document, size_t previousOffset, size_t startOffset, size_t endOffset, int type, int modifiers, std::string_view encoding)
{
    const auto sourceSize { document.getSource().size() };
    const auto lastLine { document.getLineNumber (sourceSize) };
    auto previous { previousOffset };

    for (auto segmentStart { startOffset }; segmentStart < endOffset;)
    {
        const auto line { document.getLineNumber (segmentStart) };
        const auto lineEnd { line < lastLine ? document.getByteOffset (line + 1, 1) - 1 : sourceSize };
        const auto segmentEnd { std::min (endOffset, lineEnd) };

        if (segmentEnd > segmentStart)
        {
            appendSegment (json, document, previous, segmentStart, segmentEnd, type, modifiers, encoding);
            previous = segmentStart;
        }

        segmentStart = lineEnd + 1;
    }

    return previous;
}

void Query::appendSegment (juce::MemoryBlock& json, const jam::Document& document, size_t previousOffset, size_t startOffset, size_t endOffset, int type, int modifiers, std::string_view encoding)
{
    const auto line { static_cast<int> (document.getLineNumber (startOffset)) };
    const auto previousLine { static_cast<int> (document.getLineNumber (previousOffset)) };
    const auto character { Index::getCharacter (document, startOffset, encoding) };
    const auto deltaLine { line - previousLine };
    const auto deltaStart { deltaLine == 0 ? character - Index::getCharacter (document, previousOffset, encoding) : character };
    const auto length { Index::getCharacter (document, endOffset, encoding) - character };

    jam::JsonWriter::appendSeparator (json);
    jam::Format::appendDecimal (json, deltaLine);

    jam::JsonWriter::appendSeparator (json);
    jam::Format::appendDecimal (json, deltaStart);

    jam::JsonWriter::appendSeparator (json);
    jam::Format::appendDecimal (json, length);

    jam::JsonWriter::appendSeparator (json);
    jam::Format::appendDecimal (json, type);

    jam::JsonWriter::appendSeparator (json);
    jam::Format::appendDecimal (json, modifiers);
}

size_t Query::getOffset (const jam::Document& document, int line, int column, std::string_view encoding)
{
    const auto sourceSize { document.getSource().size() };
    const auto lineCount { document.getLineNumber (sourceSize) };

    if (static_cast<uint32_t> (line) <= lineCount)
        return document.getByteOffset (static_cast<uint32_t> (line), static_cast<uint32_t> (Index::getByteColumn (document, line, column, encoding)));

    return sourceSize;
}

juce::Range<size_t> Query::getElementSpan (const jam::Document::Element& element)
{
    return element.get<jam::Document::Tokens> (Id::range)->at (0).span;
}

const jam::Document::Element* Query::getEnclosingChild (const jam::Document::Element& element, size_t offset)
{
    for (const auto* child : element)
        if (const auto span { getElementSpan (*child) }; span.getStart() <= offset and offset < span.getEnd())
            return child;

    return nullptr;
}

const jam::Document::Element* Query::getInnermostElement (const jam::Document& document, size_t offset)
{
    const auto* element { document.getRoot() };

    for (const auto* child { getEnclosingChild (*element, offset) }; child != nullptr; child = getEnclosingChild (*element, offset))
        element = child;

    return element;
}

const jam::Document::Element* Query::getOuterLevel (const jam::Document::Element& element)
{
    const auto span { getElementSpan (element) };

    for (const auto* ancestor { element.parent }; ancestor != nullptr; ancestor = ancestor->parent)
        if (getElementSpan (*ancestor) != span)
            return ancestor;

    return nullptr;
}

juce::Range<size_t> Query::getTextObjectRange (const jam::Document& document, const Language& language, const juce::String& typeName, const juce::Identifier& scopeName, size_t offset)
{
    static constexpr std::string_view functionType { "function" };
    static constexpr std::string_view classType { "class" };
    static constexpr std::string_view loopType { "loop" };
    static constexpr std::string_view conditionalType { "conditional" };
    static constexpr int noDelimiters { 0 };
    static constexpr int classDelimiters { 1 };
    static const juce::Identifier innerScope { "inner" };
    static const jam::HashMap<std::string_view, jam::Union<int, int>> textObjects {
        { functionType,    jam::Union<int, int>::pack (jam::LanguageRole::function,    noDelimiters) },
        { classType,       jam::Union<int, int>::pack (jam::LanguageRole::classToken,  classDelimiters) },
        { loopType,        jam::Union<int, int>::pack (jam::LanguageRole::loop,        noDelimiters) },
        { conditionalType, jam::Union<int, int>::pack (jam::LanguageRole::conditional, noDelimiters) }
    };

    if (const auto entry { textObjects.find (jam::Strings::getView (typeName)) }; entry != textObjects.end())
    {
        const auto& [entryType, roleDelimiters] { *entry };
        const auto [role, delimiters] { roleDelimiters };

        if (const auto* match { getRoleElement (*document.getRoot(), offset, language, role) }; match != nullptr)
            return scopeName == innerScope ? getInnerRange (document, *match, delimiters) : getElementSpan (*match);
    }

    return {};
}

const jam::Document::Element* Query::getRoleElement (const jam::Document::Element& element, size_t offset, const Language& language, int role)
{
    if (const auto* child { getEnclosingChild (element, offset) }; child != nullptr)
        if (const auto* inner { getRoleElement (*child, offset, language, role) }; inner != nullptr)
            return inner;

    if (element.parent != nullptr and language.hasRole (element, role))
        return &element;

    return nullptr;
}

void Query::appendSelectionChain (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Element& element, std::string_view encoding)
{
    jam::JsonWriter::appendObject (json, [&document, &element, encoding] (juce::MemoryBlock& members)
    {
        const auto span { getElementSpan (element) };

        jam::JsonWriter::appendKey (members, lsp::Member::range);
        appendRange (members, document, span.getStart(), span.getEnd(), encoding);

        if (const auto* outer { getOuterLevel (element) }; outer != nullptr)
        {
            jam::JsonWriter::appendKey (members, lsp::Member::parent);
            appendSelectionChain (members, document, *outer, encoding);
        }
    });
}

void Query::appendSelectionRanges (juce::MemoryBlock& json, const jam::Document& document, size_t offset, std::string_view encoding)
{
    const auto& stream { *document.getRoot()->get<jam::Document::Tokens> (Id::tokens) };
    const auto after { std::upper_bound (stream.begin(), stream.end(), offset, [] (size_t position, const jam::Document::Token& token) { return position < token.offset(); }) };
    const auto& innermost { *getInnermostElement (document, offset) };
    const auto isTokenLevel { after != stream.begin() and std::prev (after)->offset() + std::prev (after)->length() > offset and std::prev (after)->span != getElementSpan (innermost) };

    if (isTokenLevel)
    {
        jam::JsonWriter::appendObject (json, [&document, after, &innermost, encoding] (juce::MemoryBlock& members)
        {
            const auto tokenSpan { std::prev (after)->span };

            jam::JsonWriter::appendKey (members, lsp::Member::range);
            appendRange (members, document, tokenSpan.getStart(), tokenSpan.getEnd(), encoding);

            jam::JsonWriter::appendKey (members, lsp::Member::parent);
            appendSelectionChain (members, document, innermost, encoding);
        });
    }
    else
    {
        appendSelectionChain (json, document, innermost, encoding);
    }
}

juce::Range<size_t> Query::getInnerRange (const jam::Document& document, const jam::Document::Element& element, int delimiters)
{
    const auto& body { element.get<jam::Document::Tokens> (Id::body)->at (0) };
    const auto delimiterCount { static_cast<size_t> (delimiters) };

    if (body.length() == 0)
        return {};

    if (delimiters > 0)
        return { body.offset() - delimiterCount, body.offset() + body.length() + delimiterCount };

    const auto view { document.getView (body) };
    const auto trimmed { jam::Format::trim (view) };
    const auto startOffset { trimmed.empty() ? size_t { 0 } : static_cast<size_t> (trimmed.data() - view.data()) };

    return { body.offset() + startOffset, body.offset() + startOffset + static_cast<size_t> (trimmed.size()) };
}

jam::Fuzzy::Result Query::getMatch (const jam::Fuzzy::Pattern& pattern, const jam::Document& document, const jam::Document::Element& element) const
{
    return pattern.match (document.getView (element.get<jam::Document::Tokens> (Id::nameTokens)->last()), slab);
}

Query::Matches Query::getMatches (const jam::Fuzzy::Pattern& pattern, const Index& index) const
{
    Matches scored;

    for (const auto& [symbol, symbolDeclarations] : index.declarations)
    {
        for (const auto& declarationOccurrence : symbolDeclarations)
        {
            const auto& [pathIndex, record] { declarationOccurrence };

            if (const auto [element, tokenIndex] { index.getElement (declarationOccurrence) }; element != nullptr)
            {
                const auto& document { *index.documents.at (pathIndex) };

                if (index.hasRole (document, *element, jam::LanguageRole::declaration))
                {
                    const auto match { getMatch (pattern, document, *element) };

                    if (match.isMatch())
                        scored.add ({ match.score, pathIndex, element });
                }
            }
        }
    }

    std::sort (scored.begin(), scored.end(), [this, &index] (const auto& left, const auto& right)
    {
        const auto& [leftScore, leftIndex, leftElement] { left };
        const auto& [rightScore, rightIndex, rightElement] { right };

        return leftScore != rightScore ? leftScore > rightScore : isBefore (leftIndex, *leftElement, getLastTokenIndex (*leftElement), rightIndex, *rightElement, getLastTokenIndex (*rightElement), index);
    });

    const auto duplicates { std::unique (scored.begin(), scored.end(), [] (const auto& left, const auto& right)
    {
        const auto& [leftScore, leftIndex, leftElement] { left };
        const auto& [rightScore, rightIndex, rightElement] { right };

        return leftElement == rightElement;
    }) };

    scored.removeRange (static_cast<int> (std::distance (scored.begin(), duplicates)), static_cast<int> (std::distance (duplicates, scored.end())));

    return scored;
}

void Query::workspaceSymbol (juce::ValueTree& request, const Index& index) const
{
    const auto& arguments { *request[Id::arguments].getArray() };

    applyToArrayResult (request, [this, &index, &arguments] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        const auto pattern { jam::Fuzzy::Pattern::parse (jam::Strings::getView (arguments.getFirst().toString()), jam::Fuzzy::CaseMode::smart, shouldNormalize, jam::Fuzzy::Scheme::standard) };

        for (const auto& [score, pathIndex, element] : getMatches (pattern, index))
        {
            const auto& path { index.paths.at (pathIndex) };
            const auto& document { *index.documents.at (pathIndex) };
            const auto& token { element->get<jam::Document::Tokens> (Id::nameTokens)->at (getLastTokenIndex (*element)) };
            const auto symbolType { index.getSymbolType (document, *element) };

            jam::JsonWriter::appendSeparator (entries);
            jam::JsonWriter::appendObject (entries, [&path, &document, &token, symbolType, encoding] (juce::MemoryBlock& members)
            {
                jam::JsonWriter::appendKey (members, lsp::Member::name);
                jam::JsonWriter::appendString (members, document.getView (token));

                jam::JsonWriter::appendKey (members, lsp::Member::type);
                jam::Format::appendDecimal (members, symbolType);

                jam::JsonWriter::appendKey (members, lsp::Member::location);
                appendLocation (members, path, document, token, encoding);
            });
        }
    });
}

void Query::goToImplementation (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToPosition (request, [this, &index, &entries, encoding] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            applyToDeclarations (pathIndex, document, element, tokenIndex, [this, &index, &entries, encoding] (int declarationIndex, const jam::Document&, const jam::Document::Element& declaration, int)
            {
                applyToImplementations (declarationIndex, declaration, [&index, &entries, encoding] (int implementationIndex, const jam::Document& implementationDocument, const jam::Document::Element& implementation, int implementationTokenIndex)
                {
                    jam::JsonWriter::appendSeparator (entries);
                    appendNameLocation (entries, index.paths.at (implementationIndex), implementationDocument, implementation, implementationTokenIndex, encoding);
                }, index);
            }, index);
        }, index, encoding);
    });
}

void Query::prepareCallHierarchy (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToFunctions (request, [&index, &entries, encoding] (int declarationIndex, const jam::Document& declarationDocument, const jam::Document::Element& declaration, int declarationTokenIndex)
        {
            jam::JsonWriter::appendSeparator (entries);
            appendItem (entries, index.paths.at (declarationIndex), declarationDocument, declaration, declarationTokenIndex, index, encoding);
        }, index, encoding);
    });
}

std::pair<const jam::Document::Element*, int> Query::getCallReference (const Index& index, const Occurrence& occurrence)
{
    const auto& [referencePathIndex, record] { occurrence };
    const auto [element, tokenIndex] { index.getElement (occurrence) };

    if (element != nullptr and index.hasRole (*index.documents.at (referencePathIndex), *element, jam::LanguageRole::call) and element->parent->parent != nullptr)
        return { element, tokenIndex };

    return { nullptr, -1 };
}

const jam::Document::Element* Query::getCaller (const Index& index, const Occurrence& occurrence)
{
    const auto [callElement, callTokenIndex] { getCallReference (index, occurrence) };

    return callElement != nullptr ? callElement->parent : nullptr;
}

void Query::appendIncomingRuns (juce::MemoryBlock& entries, juce::int64 symbol, const Index& index, std::string_view encoding)
{
    const auto& references { getOccurrences (index.references, symbol) };

    for (int runStart { 0 }; runStart < references.size();)
    {
        const auto* caller { getCaller (index, references.at (runStart)) };

        runStart = caller != nullptr ? appendIncomingRun (entries, index, references, runStart, *caller, encoding) : runStart + 1;
    }
}

int Query::appendIncomingRun (juce::MemoryBlock& entries, const Index& index, const Occurrences& references, int runStart, const jam::Document::Element& caller, std::string_view encoding)
{
    const auto runPathIndex { getOccurrencePath (references.at (runStart)) };
    const auto& path { index.paths.at (runPathIndex) };
    const auto& document { *index.documents.at (runPathIndex) };

    auto runEnd { runStart + 1 };

    jam::JsonWriter::appendSeparator (entries);
    jam::JsonWriter::appendObject (entries, [&index, &references, &path, &document, &caller, &runEnd, runStart, encoding] (juce::MemoryBlock& members)
    {
        jam::JsonWriter::appendKey (members, lsp::Member::from);
        appendItem (members, path, document, caller, getLastTokenIndex (caller), index, encoding);

        jam::JsonWriter::appendKey (members, lsp::Member::fromRanges);
        jam::JsonWriter::appendArray (members, [&index, &references, &document, &caller, runStart, &runEnd, encoding] (juce::MemoryBlock& ranges)
        {
            runEnd = appendCallRanges (ranges, index, references, runStart, caller, document, encoding);
        });
    });

    return runEnd;
}

int Query::appendCallRanges (juce::MemoryBlock& ranges, const Index& index, const Occurrences& references, int runStart, const jam::Document::Element& caller, const jam::Document& document, std::string_view encoding)
{
    for (int position { runStart }; position < references.size(); ++position)
    {
        const auto [callElement, callTokenIndex] { getCallReference (index, references.at (position)) };

        if (callElement == nullptr or callElement->parent != &caller)
            return position;

        const auto& token { callElement->get<jam::Document::Tokens> (Id::nameTokens)->at (callTokenIndex) };

        jam::JsonWriter::appendSeparator (ranges);
        appendRange (ranges, document, token.span.getStart(), token.span.getEnd(), encoding);
    }

    return references.size();
}

void Query::incomingCalls (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToPosition (request, [this, &index, &entries, encoding] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            applyToSymbols (pathIndex, document, element, tokenIndex, [&index, &entries, encoding] (const SymbolDocument::SymbolRecord& record)
            {
                appendIncomingRuns (entries, record.symbol, index, encoding);
            }, index);
        }, index, encoding);
    });
}

void Query::outgoingCalls (juce::ValueTree& request, const Index& index) const
{
    applyToArrayResult (request, [this, &index, &request] (juce::MemoryBlock& entries, std::string_view encoding)
    {
        applyToFunctions (request, [this, &index, &entries, encoding] (int declarationIndex, const jam::Document& document, const jam::Document::Element& declaration, int)
        {
            declaration.applyFunctionRecursively ([this, &index, &entries, &document, declarationIndex, encoding] (const jam::Document::Element& element)
            {
                const auto callTokenIndex { getLastTokenIndex (element) };

                if (index.hasRole (document, element, jam::LanguageRole::call))
                    applyToDeclarations (declarationIndex, document, element, callTokenIndex, [&index, &entries, &document, &element, callTokenIndex, encoding] (int pathIndex, const jam::Document& candidateDocument, const jam::Document::Element& candidate, int candidateTokenIndex)
                    {
                        jam::JsonWriter::appendSeparator (entries);
                        jam::JsonWriter::appendObject (entries, [&index, &document, &element, &candidate, &candidateDocument, pathIndex, candidateTokenIndex, callTokenIndex, encoding] (juce::MemoryBlock& members)
                        {
                            jam::JsonWriter::appendKey (members, lsp::Member::to);
                            appendItem (members, index.paths.at (pathIndex), candidateDocument, candidate, candidateTokenIndex, index, encoding);

                            jam::JsonWriter::appendKey (members, lsp::Member::fromRanges);
                            jam::JsonWriter::appendArray (members, [&document, &element, callTokenIndex, encoding] (juce::MemoryBlock& ranges)
                            {
                                const auto& token { element.get<jam::Document::Tokens> (Id::nameTokens)->at (callTokenIndex) };

                                jam::JsonWriter::appendSeparator (ranges);
                                appendRange (ranges, document, token.span.getStart(), token.span.getEnd(), encoding);
                            });
                        });
                    }, index);
            });
        }, index, encoding);
    });
}
