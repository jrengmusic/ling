#pragma once
#include "Index.h"

/**
 * @class Query
 * @brief Answers the request rows of the model from an Index, and writes the JSON result of each row.
 *
 * Each lookup reads the documents and the symbol maps of a const Index in place.
 * The lookup stores its JSON as the result of the row, and it keeps no result.
 * Query holds only the working memory of the fuzzy finder.
 */
class Query
{
public:
    /** @brief Creates a query. The only state that it holds is the fuzzy finder slab. */
    Query() = default;

    /**
     * @brief Writes the declarations of the element at the position of a request.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void goToDefinition (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes each declaration of the element at the position of a request, and the references to each declaration.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void findReferences (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes DocumentHighlight entries for all occurrences of the symbol at the request position within its file.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void documentHighlight (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes a WorkspaceEdit replacing every occurrence of the symbol at the request position with Id::newName.
     *
     * Writes JSON null when the symbol has neither declarations nor references.
     *
     * @param request  The request row. Its arguments give the file and position, and Id::newName gives the replacement text.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void rename (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes definition (or declaration) name locations for the type symbols of the element at the request position.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void typeDefinition (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes quick-fix CodeAction edits for compiler fix-its overlapping the request range.
     *
     * @param request  The request row. Its arguments give the file and start/end range coordinates.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void codeAction (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes an LSP Hover object for the first symbol record at the request position carrying a hover entry.
     *
     * Combines the SymbolDocument hover string with the definition span text read
     * from the definition document when present, paired with the name-token range.
     * Writes JSON null when no hover record matches.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void hover (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the symbols of the file in a request, as a hierarchical DocumentSymbol tree.
     *
     * Builds the tree from SymbolDocument::OutlineEntry records using each entry's
     * parent index.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void documentSymbol (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the URI string of the counterpart file for the file in a request, or JSON null when none exists.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void switchSourceHeader (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes all InlayHint entries recorded in SymbolDocument for the file in a request.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void inlayHint (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes delta-encoded LSP SemanticTokens for the full document in a request, or JSON null when not stored.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void semanticTokens (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes delta-encoded LSP SemanticTokens covering the byte range of a request, or JSON null when not stored.
     *
     * @param request  The request row. Its arguments give the file and start/end range coordinates.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void semanticTokensRange (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes an LSP SelectionRange parent chain from the token or innermost element at the request position.
     *
     * @param request  The request row. Its arguments give the file, line, and column.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void selectionRange (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the Range of the enclosing named text object (function, class, loop, or conditional) at the request position.
     *
     * Selects either the full element span or its inner body range according to Id::scope,
     * writing JSON null when no matching element encloses the position.
     *
     * @param request  The request row. Its arguments give the position, and Id::type and Id::scope select the target.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void textObject (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes an array of LSP Diagnostic objects recorded in SymbolDocument for the file in a request.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void diagnostics (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes an LSP textDocument/publishDiagnostics JSON-RPC notification for the file in a request.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void publishDiagnostics (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the declarations whose last name matches the fuzzy query of a request.
     *
     * @param request  The request row. Its single argument is the query.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void workspaceSymbol (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the completions of the name that ends before the position of a request, as a JSON array.
     *
     * The position is the cursor after the typed name, so the lookup reads the
     * byte before it. No stored document, no element, or no matching completion entry
     * gives an empty array. The lookup reads the stored documents and starts no parse.
     *
     * @param request  The request row. Its arguments give the file, the line, and the column.
     * @param index    The index that holds the documents and the symbol maps.
     */
    void completion (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the implementations of the declarations at the position of a request.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void goToImplementation (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the function declarations at the position of a request, as the start of a call hierarchy.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void prepareCallHierarchy (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the parents of the call elements that refer to the function at the position of a request.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void incomingCalls (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the declarations that the calls inside the function at the position of a request refer to.
     *
     * The walk visits each element under the function declaration. Each call
     * element gives the declarations with its symbol.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void outgoingCalls (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Returns the index in paths of the file that a request names.
     *
     * The file argument resolves against the folder of the client. The request
     * stores that folder in its directory, because this process can run in a
     * different folder.
     *
     * @param request  The request row. Its directory and its file argument give the path.
     * @return The index of the path in paths, or -1 when the path is not stored.
     * @param index  The index that holds the documents and the symbol maps.
     */
    int getPathIndex (const juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Returns the file that a request names, resolved against the folder that the request stores.
     *
     * This process can run in a different folder from the client, so the request
     * holds the folder of the client in its directory.
     *
     * @param request  The request row. Its directory and its file argument give the file.
     * @return The file named by the file argument, under the directory of the request.
     */
    static juce::File getFile (const juce::ValueTree& request);

    /**
     * @brief Appends the `"jsonrpc":"2.0"` member pair to a JSON object buffer.
     *
     * @tparam Buffer  String or MemoryBlock buffer accepted by jam::JsonWriter.
     * @param json     Target JSON object members buffer.
     */
    template <typename Buffer>
    static void appendJsonRpc (Buffer& json)
    {
        jam::JsonWriter::appendKey (json, lsp::Member::jsonrpc);
        jam::JsonWriter::appendString (json, lsp::Message::jsonrpc);
    }

    /**
     * @brief Writes a JSON-RPC error response onto @p request Id::result, with the command name of @p request as the message.
     *
     * The response id is the Id::id of @p request, or null when the request has none.
     *
     * @param request  The request row receiving Id::result.
     * @param code     JSON-RPC error code from lsp::ErrorCodes.
     */
    static void setError (juce::ValueTree& request, int code);

    /**
     * @brief Writes a JSON-RPC error response onto @p request Id::result, with @p messageText as the message.
     *
     * The response id is the Id::id of @p request, or null when the request has none.
     *
     * @param request      The request row receiving Id::result.
     * @param code         JSON-RPC error code from lsp::ErrorCodes.
     * @param messageText  The error message text.
     */
    static void setError (juce::ValueTree& request, int code, std::string_view messageText);

    /**
     * @brief Serializes a JSON-RPC response object with @p appendValue as `result` onto @p request Id::result.
     *
     * @tparam Function     Callable taking (juce::MemoryBlock& message).
     * @param request       The request row providing optional Id::id (null when absent) and receiving Id::result.
     * @param appendValue   Callable writing the JSON value of the `result` member.
     */
    template <typename Function>
    static void setResult (juce::ValueTree& request, const Function& appendValue)
    {
        juce::MemoryBlock json;

        jam::JsonWriter::appendObject (json, [&request, &appendValue] (juce::MemoryBlock& message)
        {
            appendJsonRpc (message);

            jam::JsonWriter::appendKey (message, lsp::Member::id);

            if (request.hasProperty (Id::id))
                jam::Format::appendBytes (message, jam::Strings::getView (request[Id::id].toString()));
            else
                jam::JsonWriter::appendNull (message);

            jam::JsonWriter::appendKey (message, lsp::Member::result);
            appendValue (message);
        });

        request.setProperty (Id::result, juce::var (std::move (json)), nullptr);
    }

    /**
     * @brief Validates that @p request has a known command shape and matching positive position/range arguments.
     *
     * @param request  The request row to validate before dispatch.
     * @return @c true when the command and its argument count and bounds are valid.
     */
    static bool isValidRequest (const juce::ValueTree& request);

    /**
     * @brief Returns the command word of @p request as an identifier.
     *
     * @param request  The request row that holds Id::command.
     * @return The command identifier, or an empty identifier when the request has no command.
     */
    static juce::Identifier getCommand (const juce::ValueTree& request);

    /**
     * @brief Returns the parameter shape identifier (position, range, file, or query) for @p operation.
     *
     * @param operation  Operation identifier looked up in map::Parameters.
     * @return Shape identifier, or null identifier when @p operation is unknown.
     */
    static juce::Identifier getArgumentShape (const juce::Identifier& operation);

private:
    /** @brief The index of the file argument of a request. The line and the column follow it. */
    static constexpr int fileIndex { 0 };
    static constexpr int lineIndex { 1 };///< Index of the 1-based start line argument in position/range requests.
    static constexpr int columnIndex { 2 };///< Index of the 1-based start column argument in position/range requests.
    static constexpr int endLineIndex { 3 };///< Index of the 1-based end line argument in range requests.
    static constexpr int endColumnIndex { 4 };///< Index of the 1-based end column argument in range requests.

    /**
     * @brief Returns the index of the last token in @p element's Id::nameTokens array.
     *
     * @param element  Document element carrying Id::nameTokens.
     * @return Last name-token index.
     */
    static int getLastTokenIndex (const jam::Document::Element& element);

    /** @brief Whether the fuzzy patterns that the lookups parse normalize diacritics. */
    static constexpr bool shouldNormalize { true };

    /**
     * @brief Resolves the first indexed counterpart file for @p file using its language's counterpart extensions.
     *
     * Checks the same directory first, then falls back to stem matches across @p index.
     *
     * @param file   Source or header file whose counterpart is sought.
     * @param index  The index that holds the language map, paths, and stem rows.
     * @return Matching counterpart file, or an empty juce::File when none is indexed.
     */
    static juce::File getCounterpart (const juce::File& file, const Index& index);

    /**
     * @brief Calls @p function for each SymbolRecord covering @p tokenIndex of @p element, preferring exact name matches.
     *
     * @tparam Function   Callable taking (const SymbolDocument::SymbolRecord&).
     * @param pathIndex   Path index of @p document in @p index.
     * @param document    Parsed document containing @p element.
     * @param element     Element whose token symbol records are visited.
     * @param tokenIndex  Index inside @p element's Id::nameTokens.
     * @param function    Callable invoked for each matching symbol record.
     * @param index       The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToSymbols (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Function& function, const Index& index) const
    {
        const auto text { document.getView (element.get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex)) };
        const auto isNamed { [&document, text] (const SymbolDocument::SymbolRecord& record)
        {
            const auto start { document.getByteOffset (static_cast<uint32_t> (record.line), static_cast<uint32_t> (record.column)) };
            const auto length { static_cast<size_t> (record.nameLength) };

            return document.getView (jam::Document::Token (start, start + length)).compare (text) == 0;
        } };
        bool hasNamedRecord { false };

        index.applyToRecords (pathIndex, document, element, tokenIndex, [&hasNamedRecord, &isNamed] (const SymbolDocument::SymbolRecord& record)
        {
            hasNamedRecord = hasNamedRecord or isNamed (record);
        });

        index.applyToRecords (pathIndex, document, element, tokenIndex, [&function, &isNamed, hasNamedRecord] (const SymbolDocument::SymbolRecord& record)
        {
            if (not hasNamedRecord or isNamed (record))
                function (record);
        });
    }

    /**
     * @brief Calls @p function for each declaration occurrence of @p symbol in index.declarations.
     *
     * @tparam Function  Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param symbol     64-bit symbol identifier.
     * @param function   Callable invoked for each declaration occurrence.
     * @param index      The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToSymbolDeclarations (juce::int64 symbol, const Function& function, const Index& index) const
    {
        if (const auto entry { index.declarations.find (symbol) }; entry != index.declarations.end())
        {
            const auto& [declarationSymbol, symbolDeclarations] { *entry };

            index.applyToOccurrences (symbolDeclarations, [&function] (int declarationIndex, const jam::Document& declarationDocument, const jam::Document::Element& declaration, int declarationTokenIndex, const SymbolDocument::SymbolRecord&)
            {
                function (declarationIndex, declarationDocument, declaration, declarationTokenIndex);
            });
        }
    }

    /**
     * @brief Resolves symbol records at (@p element, @p tokenIndex) and visits each matching declaration.
     *
     * @tparam Function   Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param pathIndex   Path index of @p document in @p index.
     * @param document    Parsed document containing @p element.
     * @param element     Element whose declarations are visited.
     * @param tokenIndex  Index inside @p element's Id::nameTokens.
     * @param function    Callable invoked for each declaration occurrence.
     * @param index       The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToDeclarations (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Function& function, const Index& index) const
    {
        applyToSymbols (pathIndex, document, element, tokenIndex, [this, &function, &index] (const SymbolDocument::SymbolRecord& record)
        {
            applyToSymbolDeclarations (record.symbol, function, index);
        }, index);
    }

    /**
     * @brief Calls @p function for each candidate completion scope symbol visible at (@p element, @p tokenIndex).
     *
     * Inspects a preceding qualifier token, the receiver before a member access, or
     * enclosing declaration ancestors plus the global root SymbolDocument::noSymbol.
     *
     * @tparam Function   Callable taking (juce::int64 scopeSymbol).
     * @param pathIndex   Path index of @p document in @p index.
     * @param document    Parsed document containing @p element.
     * @param element     Element at the completion position.
     * @param tokenIndex  Index inside @p element's Id::nameTokens.
     * @param function    Callable invoked with each scope symbol identifier.
     * @param index       The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToScopeSymbols (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Function& function, const Index& index) const
    {
        const auto applyToScope { [&function, &index, pathIndex] (const SymbolDocument::SymbolRecord& record)
        {
            const auto symbol { record.symbol };

            if (index.completions.contains (symbol))
                function (symbol);
            else
                for (const auto typeSymbol : index.getSymbolDocument (pathIndex).getTypeSymbols (record))
                    function (typeSymbol);
        } };

        if (tokenIndex > 0)
        {
            index.applyToRecords (pathIndex, document, element, tokenIndex - 1, applyToScope);
        }
        else
        {
            if (index.hasRole (document, element, jam::LanguageRole::member))
            {
                const auto [objectElement, objectTokenIndex] { document.getPreviousNameToken (element.get<jam::Document::Tokens> (Id::nameTokens)->at (0).offset()) };

                if (objectElement != nullptr)
                    index.applyToRecords (pathIndex, document, *objectElement, objectTokenIndex, applyToScope);
            }
            else
            {
                function (SymbolDocument::noSymbol);

                for (const auto* ancestor { element.parent }; ancestor != nullptr and ancestor->parent != nullptr; ancestor = ancestor->parent)
                    if (index.hasRole (document, *ancestor, jam::LanguageRole::declaration))
                        for (int nameTokenIndex { 0 }; nameTokenIndex < ancestor->get<jam::Document::Tokens> (Id::nameTokens)->size(); ++nameTokenIndex)
                            index.applyToRecords (pathIndex, document, *ancestor, nameTokenIndex, applyToScope);
            }
        }
    }

    /**
     * @brief Calls @p function for all declaration and reference occurrences of @p symbol across the index.
     *
     * @tparam Function  Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param symbol     64-bit symbol identifier.
     * @param function   Callable invoked for each occurrence.
     * @param index      The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToSymbolOccurrences (juce::int64 symbol, const Function& function, const Index& index) const
    {
        applyToSymbolDeclarations (symbol, function, index);
        applyToReferences (symbol, function, index);
    }

    /**
     * @brief Calls @p function for all declaration and reference occurrences of @p symbol within @p pathIndex.
     *
     * @tparam Function  Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param symbol     64-bit symbol identifier.
     * @param pathIndex  Path index restricting which file's occurrences are visited.
     * @param function   Callable invoked for each matching occurrence.
     * @param index      The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToSymbolOccurrences (juce::int64 symbol, int pathIndex, const Function& function, const Index& index) const
    {
        const auto applyToOccurrenceMap { [symbol, pathIndex, &function, &index] (const jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& occurrences)
        {
            if (const auto entry { occurrences.find (symbol) }; entry != occurrences.end())
            {
                const auto& [occurrenceSymbol, symbolOccurrences] { *entry };

                index.applyToOccurrences (symbolOccurrences, pathIndex, [&function] (int occurrenceIndex, const jam::Document& occurrenceDocument, const jam::Document::Element& occurrence, int occurrenceTokenIndex, const SymbolDocument::SymbolRecord&)
                {
                    function (occurrenceIndex, occurrenceDocument, occurrence, occurrenceTokenIndex);
                });
            }
        } };

        applyToOccurrenceMap (index.declarations);
        applyToOccurrenceMap (index.references);
    }

    /**
     * @brief Calls @p function for each reference occurrence of @p symbol in index.references.
     *
     * @tparam Function  Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param symbol     64-bit symbol identifier.
     * @param function   Callable invoked for each reference occurrence.
     * @param index      The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToReferences (juce::int64 symbol, const Function& function, const Index& index) const
    {
        if (const auto entry { index.references.find (symbol) }; entry != index.references.end())
        {
            const auto& [referenceSymbol, symbolReferences] { *entry };

            index.applyToOccurrences (symbolReferences, [&function] (int referenceIndex, const jam::Document& referenceDocument, const jam::Document::Element& reference, int referenceTokenIndex, const SymbolDocument::SymbolRecord&)
            {
                function (referenceIndex, referenceDocument, reference, referenceTokenIndex);
            });
        }
    }

    /**
     * @brief Resolves @p file to its SymbolDocument, FileEntry, and stored jam::Document and invokes @p function.
     *
     * @tparam Function  Callable taking (const SymbolDocument&, const SymbolDocument::FileEntry&, const jam::Document&).
     * @param file       Canonical file path to look up in @p index.
     * @param function   Callable invoked when both symbol file and parsed document exist.
     * @param index      The index that holds the documents and symbol bindings.
     */
    template <typename Function>
    static void applyToSymbolFile (const juce::File& file, const Function& function, const Index& index)
    {
        const auto pathIndex { index.paths.indexOf (file.getFullPathName(), false, 0) };

        if (const auto entry { index.symbolFiles.find (pathIndex) }; entry != index.symbolFiles.end())
        {
            const auto& [filePathIndex, unitFile] { *entry };
            const auto& [unitPathIndex, symbolFile] { unitFile };

            if (const auto* document { index.getOrCreateDocument (pathIndex) }; document != nullptr)
                function (*index.symbolDocuments.at (unitPathIndex), *symbolFile, *document);
        }
    }

    /**
     * @brief Calls @p function for the element covering the 1-based position of @p request.
     *
     * @tparam Function      Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param request        The request row whose arguments give file, line, and column.
     * @param function       Callable invoked when a stored document and matching element exist.
     * @param index          The index that holds the documents and the symbol maps.
     * @param encoding       Negotiated LSP position encoding (UTF-8 or UTF-16).
     * @param columnOffset   Byte-column adjustment added before element lookup (completion passes -1).
     */
    template <typename Function>
    void applyToPosition (juce::ValueTree& request, const Function& function, const Index& index, std::string_view encoding, int columnOffset = 0) const
    {
        const auto& arguments { *request[Id::arguments].getArray() };
        const int line { arguments[lineIndex] };
        const int column { arguments[columnIndex] };
        const auto pathIndex { getPathIndex (request, index) };

        if (const auto* document { index.getOrCreateDocument (pathIndex) }; document != nullptr)
        {
            const auto byteColumn { Index::getByteColumn (*document, line, column, encoding) + columnOffset };

            if (byteColumn > 0)
            {
                if (const auto [element, tokenIndex] { Index::getElement (*document, line, byteColumn) }; element != nullptr)
                    function (pathIndex, *document, *element, tokenIndex);
            }
        }
    }

    /**
     * @brief Resolves the file named by @p request and passes it to @p function.
     *
     * @tparam Function  Callable taking (const juce::File&).
     * @param request    The request row whose directory and file argument identify the file.
     * @param function   Callable invoked with the resolved file.
     */
    template <typename Function>
    void applyToFile (const juce::ValueTree& request, const Function& function) const
    {
        function (getFile (request));
    }

    /**
     * @brief Writes a document-scoped JSON-RPC result onto @p request, or JSON null when the document is absent.
     *
     * @tparam Function  Callable taking (juce::MemoryBlock& message, int pathIndex, const jam::Document&, std::string_view encoding).
     * @param request    The request row receiving Id::result.
     * @param index      The index that holds the documents and path table.
     * @param function   Callable writing the JSON result value when the document exists.
     */
    template <typename Function>
    void applyToDocument (juce::ValueTree& request, const Index& index, const Function& function) const
    {
        const auto& encodingText { request[Id::positionEncoding].toString() };
        const auto encoding { jam::Strings::getView (encodingText) };
        const auto pathIndex { getPathIndex (request, index) };

        setResult (request, [&index, pathIndex, encoding, &function] (juce::MemoryBlock& message)
        {
            if (const auto* document { index.getOrCreateDocument (pathIndex) }; document != nullptr)
                function (message, pathIndex, *document, encoding);
            else
                jam::JsonWriter::appendNull (message);
        });
    }

    /**
     * @brief Wraps @p function output in a JSON array and writes the JSON-RPC result onto @p request.
     *
     * Reads Id::positionEncoding from @p request and passes it along with the array
     * entries buffer to @p function, storing the serialized response via setResult().
     *
     * @tparam Function  Callable taking (juce::MemoryBlock& entries, std::string_view encoding).
     * @param request    The request row receiving Id::result.
     * @param function   Callable appending comma-separated JSON array elements into entries.
     */
    template <typename Function>
    void applyToArrayResult (juce::ValueTree& request, const Function& function) const
    {
        const auto& encodingText { request[Id::positionEncoding].toString() };
        const auto encoding { jam::Strings::getView (encodingText) };

        setResult (request, [encoding, &function] (juce::MemoryBlock& message)
        {
            jam::JsonWriter::appendArray (message, [encoding, &function] (juce::MemoryBlock& entries)
            {
                function (entries, encoding);
            });
        });
    }

    /**
     * @brief Calls @p function for each function declaration at the position of @p request.
     *
     * Filters declarations at the position to those carrying jam::LanguageRole::function.
     *
     * @tparam Function  Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param request    The request row whose arguments give the position.
     * @param function   Callable invoked for each function declaration.
     * @param index      The index that holds the documents and the symbol maps.
     * @param encoding   Negotiated LSP position encoding (UTF-8 or UTF-16).
     */
    template <typename Function>
    void applyToFunctions (juce::ValueTree& request, const Function& function, const Index& index, std::string_view encoding) const
    {
        applyToPosition (request, [this, &function, &index] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            applyToDeclarations (pathIndex, document, element, tokenIndex, [&function, &index] (int declarationIndex, const jam::Document& declarationDocument, const jam::Document::Element& declaration, int declarationTokenIndex)
            {
                if (index.hasRole (declarationDocument, declaration, jam::LanguageRole::function))
                    function (declarationIndex, declarationDocument, declaration, declarationTokenIndex);
            }, index);
        }, index, encoding);
    }

    /**
     * @brief Calls @p function for each derived-class or overridden-member implementation of @p declaration.
     *
     * A class declaration visits all derived classes; a member with a non-null,
     * non-root class parent visits matching-name declarations inside each derived class.
     *
     * @tparam Function    Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param pathIndex    Path index of the document containing @p declaration.
     * @param declaration  The declaration whose implementations are visited.
     * @param function     Callable invoked for each implementation declaration.
     * @param index        The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToImplementations (int pathIndex, const jam::Document::Element& declaration, const Function& function, const Index& index) const
    {
        if (const auto* document { index.getOrCreateDocument (pathIndex) }; document != nullptr)
        {
            if (index.hasRole (*document, declaration, jam::LanguageRole::classToken))
            {
                index.applyToRecords (pathIndex, *document, declaration, getLastTokenIndex (declaration), [this, &function, &index] (const SymbolDocument::SymbolRecord& record)
                {
                    applyToDerived (record.symbol, function, index);
                });
            }
            else if (declaration.parent != nullptr and declaration.parent != document->getRoot() and index.hasRole (*document, *declaration.parent, jam::LanguageRole::classToken))
            {
                const auto memberName { document->getView (declaration.get<jam::Document::Tokens> (Id::nameTokens)->last()) };

                index.applyToRecords (pathIndex, *document, *declaration.parent, getLastTokenIndex (*declaration.parent), [this, &function, &index, memberName] (const SymbolDocument::SymbolRecord& record)
                {
                    applyToDerived (record.symbol, [this, &function, &index, memberName] (int derivedIndex, const jam::Document& derivedDocument, const jam::Document::Element& derived, int)
                    {
                        for (const auto* child : derived)
                        {
                            if (index.hasRole (derivedDocument, *child, jam::LanguageRole::declaration)
                                and derivedDocument.getView (child->get<jam::Document::Tokens> (Id::nameTokens)->last()).compare (memberName) == 0)
                                applyToDeclarations (derivedIndex, derivedDocument, *child, getLastTokenIndex (*child), function, index);
                        }
                    }, index);
                });
            }
        }
    }

    /**
     * @brief Traverses the inheritance hierarchy rooted at class @p symbol, invoking @p function for each derived class.
     *
     * @tparam Function  Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param symbol     Base class symbol identifier.
     * @param function   Callable invoked once per unique derived class.
     * @param index      The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToDerived (juce::int64 symbol, const Function& function, const Index& index) const
    {
        jam::HashSet<juce::int64> knownSymbols;

        knownSymbols.insert (symbol);
        applyToDerivedFrom (symbol, function, knownSymbols, index);
    }

    /**
     * @brief Recursively visits classes referencing @p symbol with jam::LanguageRole::base, guarding cycles via @p knownSymbols.
     *
     * @tparam Function     Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex).
     * @param symbol        Current base class symbol identifier.
     * @param function      Callable invoked for each newly discovered derived class.
     * @param knownSymbols  Set of already visited class symbol identifiers.
     * @param index         The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToDerivedFrom (juce::int64 symbol, const Function& function, jam::HashSet<juce::int64>& knownSymbols, const Index& index) const
    {
        if (const auto entry { index.references.find (symbol) }; entry != index.references.end())
        {
            const auto& [referenceSymbol, symbolReferences] { *entry };

            index.applyToOccurrences (symbolReferences, [this, &function, &index, &knownSymbols] (int referenceIndex, const jam::Document& referenceDocument, const jam::Document::Element& reference, int, const SymbolDocument::SymbolRecord& record)
            {
                if ((record.role & jam::LanguageRole::base) != 0)
                {
                    index.applyToRecords (referenceIndex, referenceDocument, *reference.parent, getLastTokenIndex (*reference.parent), [this, &function, &index, &knownSymbols, referenceIndex, &referenceDocument, &reference] (const SymbolDocument::SymbolRecord& derivedRecord)
                    {
                        const auto derivedSymbol { derivedRecord.symbol };

                        if (auto [knownEntry, isAdded] { knownSymbols.insert (derivedSymbol) }; isAdded)
                        {
                            function (referenceIndex, referenceDocument, *reference.parent, getLastTokenIndex (*reference.parent));
                            applyToDerivedFrom (derivedSymbol, function, knownSymbols, index);
                        }
                    });
                }
            });
        }
    }

    /**
     * @brief Tells whether one element token comes before another in deterministic result order.
     *
     * Orders by path string first, then by the byte offset of the specified name token.
     *
     * @param pathIndex        The path index of @p element.
     * @param element          The first element.
     * @param tokenIndex       Name-token index in @p element.
     * @param otherPathIndex   The path index of @p otherElement.
     * @param otherElement     The second element.
     * @param otherTokenIndex  Name-token index in @p otherElement.
     * @param index            The index that holds the documents and path table.
     * @return @c true when the first element comes before the second.
     */
    bool isBefore (int pathIndex, const jam::Document::Element& element, int tokenIndex, int otherPathIndex, const jam::Document::Element& otherElement, int otherTokenIndex, const Index& index) const;

    /** @brief The scored declarations of one lookup: the score, the index of the path, and the element. */
    using Matches = jam::Array<std::tuple<int, int, const jam::Document::Element*>>;

    /** @brief A single indexed symbol occurrence pairing a path index with its SymbolRecord. */
    using Occurrence = std::pair<int, const SymbolDocument::SymbolRecord*>;

    /** @brief Ordered list of indexed symbol occurrences. */
    using Occurrences = jam::Array<Occurrence>;

    /**
     * @brief Returns the occurrence slice stored under @p symbol in @p map, or an empty slice when absent.
     *
     * @param map     Declarations or references occurrence map on Index.
     * @param symbol  64-bit symbol identifier.
     * @return Const reference to the matching Occurrences array or a shared empty array.
     */
    static const Occurrences& getOccurrences (const jam::HashMap<juce::int64, Occurrences>& map, juce::int64 symbol);

    /**
     * @brief Extracts the path index from an Occurrence pair.
     *
     * @param occurrence  Occurrence pair (pathIndex, SymbolRecord*).
     * @return Path index in Index::paths.
     */
    static int getOccurrencePath (const Occurrence& occurrence);

    /**
     * @brief Orders two occurrences by ascending path byte order, then by (line, column).
     *
     * @param index  The index providing path comparison and ordering.
     * @param left   Left occurrence pair.
     * @param right  Right occurrence pair.
     * @return @c true when @p left precedes @p right.
     */
    static bool isOccurrenceBefore (const Index& index, const Occurrence& left, const Occurrence& right);

    /**
     * @brief Appends a TextEdit JSON object replacing @p occurrence's name token with @p newName.
     *
     * @param edits       JSON array buffer receiving the edit object.
     * @param index       The index holding stored documents.
     * @param occurrence  Target occurrence to rename.
     * @param newName     Replacement identifier text.
     * @param encoding    Negotiated LSP position encoding.
     */
    static void appendOccurrenceEdit (juce::MemoryBlock& edits, const Index& index, const Occurrence& occurrence, std::string_view newName, std::string_view encoding);

    /**
     * @brief Resolves a call-site @p occurrence to its AST call element and name-token index.
     *
     * Requires the element to have jam::LanguageRole::call and an enclosing parent below root.
     *
     * @param index       The index holding documents and language roles.
     * @param occurrence  Reference occurrence to inspect.
     * @return Matching (callElement, tokenIndex), or `{ nullptr, -1 }` when not a valid call site.
     */
    static std::pair<const jam::Document::Element*, int> getCallReference (const Index& index, const Occurrence& occurrence);

    /**
     * @brief Returns the enclosing caller element for a call-site @p occurrence, or nullptr if invalid.
     *
     * @param index       The index holding documents and language roles.
     * @param occurrence  Reference occurrence to inspect.
     * @return Parent element enclosing the call, or nullptr.
     */
    static const jam::Document::Element* getCaller (const Index& index, const Occurrence& occurrence);

    /**
     * @brief Groups incoming call references to @p symbol by contiguous caller runs and appends CallHierarchyIncomingCall entries.
     *
     * @param entries   JSON array buffer receiving CallHierarchyIncomingCall objects.
     * @param symbol    Target function symbol identifier.
     * @param index     The index holding references and documents.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendIncomingRuns (juce::MemoryBlock& entries, juce::int64 symbol, const Index& index, std::string_view encoding);

    /**
     * @brief Appends one CallHierarchyIncomingCall object for a contiguous run of calls inside @p caller.
     *
     * @param entries     JSON array buffer receiving the object.
     * @param index       The index holding paths and documents.
     * @param references  Reference occurrences for the target symbol.
     * @param runStart    Starting index of the run in @p references.
     * @param caller      Enclosing caller element shared by the run.
     * @param encoding    Negotiated LSP position encoding.
     * @return Index in @p references immediately after the run.
     */
    static int appendIncomingRun (juce::MemoryBlock& entries, const Index& index, const Occurrences& references, int runStart, const jam::Document::Element& caller, std::string_view encoding);

    /**
     * @brief Appends LSP Range objects for consecutive call references belonging to @p caller starting at @p runStart.
     *
     * @param ranges      JSON array buffer receiving Range objects.
     * @param index       The index holding documents and language roles.
     * @param references  Reference occurrences for the target symbol.
     * @param runStart    Starting index of the run in @p references.
     * @param caller      Enclosing caller element bounding the run.
     * @param document    Parsed document owning @p caller.
     * @param encoding    Negotiated LSP position encoding.
     * @return First index after @p caller's run, or @p references.size().
     */
    static int appendCallRanges (juce::MemoryBlock& ranges, const Index& index, const Occurrences& references, int runStart, const jam::Document::Element& caller, const jam::Document& document, std::string_view encoding);

    /**
     * @brief Appends sibling DocumentSymbol entries whose outline parent equals @p parentIndex.
     *
     * @param json          JSON array buffer receiving DocumentSymbol objects.
     * @param document      Parsed document matching @p outlines.
     * @param outlines      Contiguous outline table for the file.
     * @param parentIndex   Outline slot index of the enclosing parent, or SymbolDocument::noParent.
     * @param outlineIndex  First candidate slot index in @p outlines.
     * @param encoding      Negotiated LSP position encoding.
     * @return Next outline slot index after all siblings and their subtrees.
     */
    static int appendOutlines (juce::MemoryBlock& json, const jam::Document& document, SymbolDocument::Span<SymbolDocument::OutlineEntry> outlines, int parentIndex, int outlineIndex, std::string_view encoding);

    /**
     * @brief Appends the DocumentSymbol fields and recursive children array for @p outlineIndex.
     *
     * @param json          JSON object members buffer.
     * @param document      Parsed document matching @p outlines.
     * @param outlines      Contiguous outline table for the file.
     * @param outlineIndex  Slot index of the outline entry to emit.
     * @param encoding      Negotiated LSP position encoding.
     * @return Next outline slot index after this entry's subtree.
     */
    static int appendOutline (juce::MemoryBlock& json, const jam::Document& document, SymbolDocument::Span<SymbolDocument::OutlineEntry> outlines, int outlineIndex, std::string_view encoding);

    /**
     * @brief Reads the request range arguments and appends matching fix-it CodeAction objects for @p file.
     *
     * @param json       JSON array buffer receiving CodeAction objects.
     * @param file       Target source file.
     * @param index      The index holding symbol files and documents.
     * @param arguments  Request arguments holding 1-based start and end line/column coordinates.
     * @param encoding   Negotiated LSP position encoding.
     */
    static void appendFileCodeActions (juce::MemoryBlock& json, const juce::File& file, const Index& index, const juce::Array<juce::var>& arguments, std::string_view encoding);

    /**
     * @brief Appends quick-fix CodeAction objects for fix-its overlapping the byte span [@p rangeStart, @p rangeEnd].
     *
     * @param json            JSON array buffer receiving CodeAction objects.
     * @param file            Source file owning the fix-its.
     * @param document        Parsed document matching @p symbolFile.
     * @param symbolDocument  SymbolDocument owning the fix-it entries and string pool.
     * @param symbolFile      FileEntry slice for @p file.
     * @param rangeStart      Start byte offset of the requested range.
     * @param rangeEnd        End byte offset of the requested range.
     * @param encoding        Negotiated LSP position encoding.
     */
    static void appendFixItActions (juce::MemoryBlock& json, const juce::File& file, const jam::Document& document, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& symbolFile, size_t rangeStart, size_t rangeEnd, std::string_view encoding);

    /**
     * @brief Appends one QuickFix CodeAction object replacing @p range in @p file with @p newText.
     *
     * @param json      JSON object buffer receiving the CodeAction.
     * @param file      Source file receiving the edit.
     * @param document  Parsed document used to convert byte offsets to LSP positions.
     * @param range     Byte token span replaced by the fix-it.
     * @param newText   Replacement text string.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendCodeAction (juce::MemoryBlock& json, const juce::File& file, const jam::Document& document, const jam::Document::Token& range, std::string_view newText, std::string_view encoding);

    /**
     * @brief Appends CompletionItem entries in @p scopeSymbol whose labels fuzzy-match @p pattern.
     *
     * @param json         JSON array buffer receiving CompletionItem objects.
     * @param scopeSymbol  Scope symbol identifier keying index.completions.
     * @param pattern      Parsed fuzzy prefix pattern.
     * @param language     Language backend mapping completion item kinds.
     * @param index        The index holding completion tables and SymbolDocuments.
     */
    void appendScopeCompletions (juce::MemoryBlock& json, juce::int64 scopeSymbol, const jam::Fuzzy::Pattern& pattern, const Language& language, const Index& index) const;

    /**
     * @brief Appends one LSP CompletionItem object for @p completion.
     *
     * @param json            JSON array buffer receiving the item.
     * @param symbolDocument  SymbolDocument owning @p completion's string pool slices.
     * @param completion      Completion record to serialize.
     * @param language        Language backend mapping @p completion.type to LSP CompletionItemKind.
     */
    static void appendCompletionItem (juce::MemoryBlock& json, const SymbolDocument& symbolDocument, const SymbolDocument::CompletionRecord& completion, const Language& language);

    /**
     * @brief Returns the declaration's token type for semantic highlighting of a reference, or @p element's own type.
     *
     * @param pathIndex   Path index of @p document.
     * @param document    Parsed document containing @p element.
     * @param element     Element being classified.
     * @param tokenIndex  Index inside @p element's Id::nameTokens.
     * @param index       The index holding symbol records and declarations.
     * @return Grammar token type integer.
     */
    int getRecordType (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Index& index) const;

    /**
     * @brief Appends the LSP SemanticTokens `data` array covering byte offsets [@p startOffset, @p endOffset).
     *
     * @param json         JSON object buffer receiving the `data` key and integer array.
     * @param pathIndex    Path index of the target document.
     * @param index        The index holding documents and language backends.
     * @param encoding     Negotiated LSP position encoding.
     * @param startOffset  Start byte offset (inclusive).
     * @param endOffset    End byte offset (exclusive).
     */
    void appendSemanticTokens (juce::MemoryBlock& json, int pathIndex, const Index& index, std::string_view encoding, size_t startOffset, size_t endOffset) const;

    /**
     * @brief Splits a multi-line token span [@p startOffset, @p endOffset) into single-line LSP semantic token tuples.
     *
     * @param json            JSON array buffer receiving 5-integer semantic token tuples.
     * @param document        Parsed document providing line numbers and character offsets.
     * @param previousOffset  Byte offset of the previously emitted semantic token segment.
     * @param startOffset     Start byte offset of the token slice.
     * @param endOffset       End byte offset of the token slice.
     * @param type            Semantic token type index in Legend::getTypes().
     * @param modifiers       Semantic token modifier bitmask in Legend::getModifiers().
     * @param encoding        Negotiated LSP position encoding.
     * @return Byte offset of the last emitted segment, becoming the next @p previousOffset.
     */
    static size_t appendTokenLines (juce::MemoryBlock& json, const jam::Document& document, size_t previousOffset, size_t startOffset, size_t endOffset, int type, int modifiers, std::string_view encoding);

    /**
     * @brief Appends one 5-integer delta-encoded LSP semantic token tuple (`deltaLine`, `deltaStart`, `length`, `type`, `modifiers`).
     *
     * @param json            JSON array buffer receiving the five integers.
     * @param document        Parsed document providing line and character mappings.
     * @param previousOffset  Byte offset of the previous semantic token segment.
     * @param startOffset     Start byte offset of this single-line segment.
     * @param endOffset       End byte offset of this single-line segment.
     * @param type            Semantic token type index in Legend::getTypes().
     * @param modifiers       Semantic token modifier bitmask in Legend::getModifiers().
     * @param encoding        Negotiated LSP position encoding.
     */
    static void appendSegment (juce::MemoryBlock& json, const jam::Document& document, size_t previousOffset, size_t startOffset, size_t endOffset, int type, int modifiers, std::string_view encoding);

    /**
     * @brief Converts a 1-based (@p line, @p column) position in @p encoding to a byte offset clamped at source size.
     *
     * @param document  Parsed document.
     * @param line      1-based line number.
     * @param column    1-based character column in @p encoding.
     * @param encoding  Negotiated LSP position encoding.
     * @return Byte offset within @p document's source.
     */
    static size_t getOffset (const jam::Document& document, int line, int column, std::string_view encoding);

    /**
     * @brief Computes the inner byte range of @p element's Id::body token, trimming whitespace or delimiter bytes.
     *
     * @param document    Parsed document owning @p element.
     * @param element     Element carrying Id::body.
     * @param delimiters  Number of surrounding delimiter bytes to expand outward, or 0 to trim whitespace.
     * @return Inner byte range, or empty when body length is zero.
     */
    static juce::Range<size_t> getInnerRange (const jam::Document& document, const jam::Document::Element& element, int delimiters);

    /**
     * @brief Returns the full source byte span [start, end) from @p element's Id::range token.
     *
     * @param element  Document element carrying Id::range.
     * @return Byte range of @p element.
     */
    static juce::Range<size_t> getElementSpan (const jam::Document::Element& element);

    /**
     * @brief Finds the direct child of @p element whose span contains byte @p offset.
     *
     * @param element  Parent element to search.
     * @param offset   Target byte offset.
     * @return Enclosing child element, or nullptr when none contains @p offset.
     */
    static const jam::Document::Element* getEnclosingChild (const jam::Document::Element& element, size_t offset);

    /**
     * @brief Descends from the root of @p document to the deepest element enclosing @p offset.
     *
     * @param document  Parsed document to search.
     * @param offset    Target byte offset.
     * @return Deepest enclosing element (or the root element).
     */
    static const jam::Document::Element* getInnermostElement (const jam::Document& document, size_t offset);

    /**
     * @brief Walks upward from @p element to the nearest ancestor having a strictly wider byte span.
     *
     * @param element  Starting element.
     * @return First ancestor with a different span, or nullptr at root.
     */
    static const jam::Document::Element* getOuterLevel (const jam::Document::Element& element);

    /**
     * @brief Finds the deepest non-root element enclosing @p offset that carries @p role in @p language.
     *
     * @param element   Subtree root to search.
     * @param offset    Target byte offset.
     * @param language  Language backend providing hasRole().
     * @param role      Bitmask of jam::LanguageRole flags to match.
     * @return Deepest matching element, or nullptr when none matches.
     */
    static const jam::Document::Element* getRoleElement (const jam::Document::Element& element, size_t offset,const Language& language, int role);

    /**
     * @brief Appends an LSP SelectionRange object for @p element and recursively links its outer parent chain.
     *
     * @param json      JSON object buffer receiving `range` and optional `parent`.
     * @param document  Parsed document owning @p element.
     * @param element   Current selection level element.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendSelectionChain (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Element& element, std::string_view encoding);

    /**
     * @brief Appends a SelectionRange entry at @p offset, starting at token level when narrower than the innermost element.
     *
     * @param json      JSON array buffer receiving the SelectionRange object.
     * @param document  Parsed document to inspect.
     * @param offset    Target byte offset.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendSelectionRanges (juce::MemoryBlock& json, const jam::Document& document, size_t offset, std::string_view encoding);

    /**
     * @brief Resolves the inner or outer byte range for text object @p typeName (`function`, `class`, `loop`, `conditional`) at @p offset.
     *
     * @param document   Parsed document to inspect.
     * @param language   Language backend providing element roles.
     * @param typeName   Text object kind name (`function`, `class`, `loop`, or `conditional`).
     * @param scopeName  Scope selector (`inner` for body range, any other value for full span).
     * @param offset     Target byte offset.
     * @return Resolved byte range, or empty when @p typeName is unknown or unmatched.
     */
    static juce::Range<size_t> getTextObjectRange (const jam::Document& document, const Language& language, const juce::String& typeName, const juce::Identifier& scopeName, size_t offset);

    /**
     * @brief Serializes a JSON-RPC notification object for @p method onto @p request Id::result.
     *
     * @tparam Function      Callable taking (juce::MemoryBlock& message).
     * @param request        The request row receiving Id::result.
     * @param method         LSP notification method name.
     * @param appendParams   Callable writing the JSON value of the `params` member.
     */
    template <typename Function>
    void setNotification (juce::ValueTree& request, std::string_view method, const Function& appendParams) const
    {
        juce::MemoryBlock json;

        jam::JsonWriter::appendObject (json, [method, &appendParams] (juce::MemoryBlock& message)
        {
            appendJsonRpc (message);

            jam::JsonWriter::appendKey (message, lsp::Member::method);
            jam::JsonWriter::appendString (message, method);

            jam::JsonWriter::appendKey (message, lsp::Member::params);
            appendParams (message);
        });

        request.setProperty (Id::result, juce::var (std::move (json)), nullptr);
    }

    /**
     * @brief Appends an LSP Position object (`line`, `character`, 0-based) for byte @p offset.
     *
     * @param json      JSON object buffer receiving the Position.
     * @param document  Parsed document providing line and character conversions.
     * @param offset    Byte offset in @p document.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendPosition (juce::MemoryBlock& json, const jam::Document& document, size_t offset, std::string_view encoding);

    /**
     * @brief Appends an LSP Position object (`line`, `character`) from 0-based coordinates.
     *
     * @param json       JSON object buffer receiving the Position.
     * @param line       0-based line number.
     * @param character  0-based character offset in the negotiated encoding.
     */
    static void appendPosition (juce::MemoryBlock& json, int line, int character);

    /**
     * @brief Appends an LSP Range object (`start`, `end`) for byte span [@p startOffset, @p endOffset).
     *
     * @param json         JSON object buffer receiving the Range.
     * @param document     Parsed document providing position conversions.
     * @param startOffset  Start byte offset.
     * @param endOffset    End byte offset.
     * @param encoding     Negotiated LSP position encoding.
     */
    static void appendRange (juce::MemoryBlock& json, const jam::Document& document, size_t startOffset, size_t endOffset, std::string_view encoding);

    /**
     * @brief Appends a quoted `file://` URI string for @p file.
     *
     * @param json  JSON buffer receiving the URI string.
     * @param file  Local file whose URI is written.
     */
    static void appendUri (juce::MemoryBlock& json, const juce::File& file);

    /**
     * @brief Appends LSP Diagnostic objects from SymbolDocument for @p file into @p json.
     *
     * @param json      JSON array buffer receiving Diagnostic objects.
     * @param file      Source file whose diagnostics are emitted.
     * @param index     The index holding symbol files and documents.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendDiagnostics (juce::MemoryBlock& json, const juce::File& file, const Index& index, std::string_view encoding);

    /**
     * @brief Appends an LSP Location object (`uri`, `range`) for @p token in @p path.
     *
     * @param json      JSON object buffer receiving the Location.
     * @param path      Canonical source file path.
     * @param document  Parsed document owning @p token.
     * @param token     Source token span.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendLocation (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Token& token, std::string_view encoding);

    /**
     * @brief Appends an LSP Location for the @p tokenIndex name token of @p element.
     *
     * @param json        JSON object buffer receiving the Location.
     * @param path        Canonical source file path.
     * @param document    Parsed document owning @p element.
     * @param element     Element carrying Id::nameTokens.
     * @param tokenIndex  Index inside @p element's Id::nameTokens.
     * @param encoding    Negotiated LSP position encoding.
     */
    static void appendNameLocation (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, std::string_view encoding);

    /**
     * @brief Appends a fallback LSP Location from the symbol's HoverEntry definition span when no indexed declaration exists.
     *
     * @param entries     JSON array buffer receiving the Location entry.
     * @param pathIndex   Path index of the queried file in @c index.paths.
     * @param document    Parsed document owning @p element.
     * @param element     Element carrying Id::nameTokens.
     * @param tokenIndex  Index inside @p element's Id::nameTokens.
     * @param index       Index holding symbol files and SymbolDocument hover entries.
     */
    void appendHoverDefinition (juce::MemoryBlock& entries, int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Index& index) const;

    /**
     * @brief Appends an LSP CallHierarchyItem object (`name`, `type`, `uri`, `range`, `selectionRange`).
     *
     * @param json        JSON object buffer receiving the CallHierarchyItem.
     * @param path        Canonical source file path.
     * @param document    Parsed document owning @p element.
     * @param element     Element carrying Id::nameTokens.
     * @param tokenIndex  Index inside @p element's Id::nameTokens.
     * @param index       The index providing symbol type classification.
     * @param encoding    Negotiated LSP position encoding.
     */
    static void appendItem (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Index& index, std::string_view encoding);

    /**
     * @brief Appends an LSP DocumentHighlight object (`range`, `type`) for @p token.
     *
     * @param json      JSON object buffer receiving the DocumentHighlight.
     * @param document  Parsed document owning @p token.
     * @param token     Highlighted name token.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendHighlight (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Token& token, std::string_view encoding);

    /**
     * @brief Appends an LSP TextEdit object (`range`, `newText`) replacing @p token.
     *
     * @param json      JSON object buffer receiving the TextEdit.
     * @param document  Parsed document owning @p token.
     * @param token     Replaced name token.
     * @param newName   Replacement text.
     * @param encoding  Negotiated LSP position encoding.
     */
    static void appendTextEdit (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Token& token, std::string_view newName, std::string_view encoding);

    /**
     * @brief Appends an LSP WorkspaceEdit object wrapping per-file edit changes.
     *
     * @param json          JSON object buffer receiving the WorkspaceEdit.
     * @param index         The index holding paths and documents.
     * @param declarations  Path-ordered declaration occurrences to rename.
     * @param references    Path-ordered reference occurrences to rename.
     * @param newName       Replacement identifier text.
     * @param encoding      Negotiated LSP position encoding.
     */
    static void appendWorkspaceEdit (juce::MemoryBlock& json, const Index& index, const Occurrences& declarations, const Occurrences& references, std::string_view newName, std::string_view encoding);

    /**
     * @brief Merges sorted @p declarations and @p references by file URI into WorkspaceEdit `changes` entries.
     *
     * @param json          JSON object members buffer mapping each file URI to its TextEdit array.
     * @param index         The index holding paths and documents.
     * @param declarations  Path-ordered declaration occurrences to rename.
     * @param references    Path-ordered reference occurrences to rename.
     * @param newName       Replacement identifier text.
     * @param encoding      Negotiated LSP position encoding.
     */
    static void appendChanges (juce::MemoryBlock& json, const Index& index, const Occurrences& declarations, const Occurrences& references, std::string_view newName, std::string_view encoding);

    /**
     * @brief Returns the declarations whose last name matches a fuzzy pattern, with their scores.
     *
     * Each declaration with a name is scored by the fuzzy finder, with the slab as
     * its working memory. The list is sorted by descending score, then by the
     * result order.
     *
     * @param pattern  The parsed query.
     * @return Tuples of the score, the path index, and the element.
     * @param index  The index that holds the documents and the symbol maps.
     */
    Matches getMatches (const jam::Fuzzy::Pattern& pattern, const Index& index) const;

    /**
     * @brief Scores the last name of an element against a pattern.
     *
     * The match uses the slab as its working memory.
     *
     * @param pattern   The parsed query.
     * @param document  The document that holds the element.
     * @param element   The element to score. It must hold a name.
     * @return The result of the match, with its isMatch() and score.
     */
    jam::Fuzzy::Result getMatch (const jam::Fuzzy::Pattern& pattern, const jam::Document& document, const jam::Document::Element& element) const;

    /**
     * @brief The working memory of the fuzzy finder.
     *
     * The working memory of the fuzzy finder. The lookups that score names are
     * const, so the slab is mutable. Each lookup runs inline in the listener that
     * receives its request row.
     */
    mutable jam::Fuzzy::Slab slab;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Query)
};
