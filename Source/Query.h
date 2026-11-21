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

    void documentHighlight (juce::ValueTree& request, const Index& index) const;

    void rename (juce::ValueTree& request, const Index& index) const;

    void typeDefinition (juce::ValueTree& request, const Index& index) const;

    void codeAction (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the declarations at the position of a request, each with its scope.
     *
     * The scope is the path of the declaration without its last segment. Each
     * JSON element holds the scope as an array of names.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void hover (juce::ValueTree& request, const Index& index, int pathIndex, const jam::Document::Element* element, int tokenIndex, const jam::Document::Element* record) const;

    template <typename Function>
    bool applyToHover (juce::ValueTree& request, const Index& index, const Function& function) const
    {
        const auto& encodingText { request[Id::positionEncoding].toString() };
        const auto encoding { jam::Strings::getView (encodingText) };
        const jam::Document::Element* hoverRecord { nullptr };

        applyToPosition (request, [this, &index, &function, &hoverRecord] (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex)
        {
            applyToSymbols (pathIndex, document, element, tokenIndex, [&hoverRecord] (const jam::Document::Element& record)
            {
                if (hoverRecord == nullptr and *record.get<int> (Id::hover) != jam::LanguageCallbacks::noHover)
                    hoverRecord = &record;
            }, index);

            if (hoverRecord != nullptr)
                function (pathIndex, document, element, tokenIndex, *hoverRecord);
        }, index, encoding);

        return hoverRecord != nullptr;
    }

    /**
     * @brief Writes the symbols of the file in a request, as a tree.
     *
     * The request must hold one argument, the file. Each symbol holds its named
     * declarations as children. Unnamed declarations give their children to their
     * parent.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void documentSymbol (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the counterpart files of the file in a request, each as an object that holds only its path.
     *
     * A request that holds other than one argument writes an empty array, and sets
     * the overlay message.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void switchSourceHeader (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes each call reference of the file in a request, with the parameter texts of its first function declaration.
     *
     * The entries are in result order: path, then position. A call with no function
     * declaration gets an empty parameters array. A request that holds other than one
     * argument writes an empty array, and sets the overlay message.
     *
     * @param request  The request row. Its single argument is the file.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void inlayHint (juce::ValueTree& request, const Index& index) const;

    void semanticTokens (juce::ValueTree& request, const Index& index) const;

    void semanticTokensRange (juce::ValueTree& request, const Index& index) const;

    void selectionRange (juce::ValueTree& request, const Index& index) const;

    void textObject (juce::ValueTree& request, const Index& index) const;

    void diagnostics (juce::ValueTree& request, const Index& index) const;

    void publishDiagnostics (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the declarations whose last name matches the fuzzy query of a request.
     *
     * The request must hold one argument, the query. Other argument counts set the
     * overlay message.
     *
     * @param request  The request row. Its single argument is the query.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void workspaceSymbol (juce::ValueTree& request, const Index& index) const;

    /**
     * @brief Writes the completions of the name that ends before the position of a request, as a JSON array.
     *
     * The position is the cursor after the typed name, so the lookup reads the
     * byte before it. No stored document, no element, or an element of another
     * type gives an empty array. The lookup reads the stored documents and starts
     * no parse.
     *
     * @param request  The request row. Its arguments give the file, the line, and the column.
     * @param index  The index that holds the documents and the symbol maps.
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

    template <typename Buffer>
    static void appendJsonRpc (Buffer& json)
    {
        jam::JsonWriter::appendKey (json, lsp::Member::jsonrpc);
        jam::JsonWriter::appendString (json, lsp::Message::jsonrpc);
    }

    static void setError (juce::ValueTree& request, int code);

    static bool isValidRequest (const juce::ValueTree& request);

    static juce::Identifier getArgumentShape (const juce::Identifier& operation);

private:
    /** @brief The index of the file argument of a request. The line and the column follow it. */
    static constexpr int fileIndex { 0 };
    static constexpr int lineIndex { 1 };
    static constexpr int columnIndex { 2 };
    static constexpr int endLineIndex { 3 };
    static constexpr int endColumnIndex { 4 };

    static int getLastTokenIndex (const jam::Document::Element& element);

    /** @brief Whether the fuzzy patterns that the lookups parse normalize diacritics. */
    static constexpr bool shouldNormalize { true };

    static juce::File getCounterpart (const juce::File& file, const Index& index);

    /**
     * @brief Calls @p function for each declaration that has the symbol of an element.
     *
     * The lookup reads the declarations that share the symbol of the element. An
     * element without Id::symbol calls nothing.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param element    The element whose declarations are visited.
     * @param function   The callable. It receives the path index, the document, and each declaration.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToSymbols (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Function& function, const Index& index) const
    {
        const auto text { document.getView (element.get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex)) };
        const auto isNamed { [&document, text] (const jam::Document::Element& record)
        {
            const auto start { document.getByteOffset (static_cast<uint32_t> (*record.get<int> (Id::line)), static_cast<uint32_t> (*record.get<int> (Id::column))) };
            const auto length { static_cast<uint32_t> (*record.get<int> (Id::nameLength)) };

            return document.getView (jam::Document::Token (start, start + length)).compare (text) == 0;
        } };
        bool hasNamedRecord { false };

        index.applyToRecords (pathIndex, document, element, tokenIndex, [&hasNamedRecord, &isNamed] (const jam::Document::Element& record)
        {
            hasNamedRecord = hasNamedRecord or isNamed (record);
        });

        index.applyToRecords (pathIndex, document, element, tokenIndex, [&function, &isNamed, hasNamedRecord] (const jam::Document::Element& record)
        {
            if (not hasNamedRecord or isNamed (record))
                function (record);
        });
    }

    template <typename Function>
    void applyToSymbolDeclarations (juce::int64 symbol, const Function& function, const Index& index) const
    {
        if (const auto entry { index.declarations.find (symbol) }; entry != index.declarations.end())
        {
            const auto& [declarationSymbol, symbolDeclarations] { *entry };

            index.applyToOccurrences (symbolDeclarations, [&function] (int declarationIndex, const jam::Document& declarationDocument, const jam::Document::Element& declaration, int declarationTokenIndex, const jam::Document::Element&)
            {
                function (declarationIndex, declarationDocument, declaration, declarationTokenIndex);
            });
        }
    }

    template <typename Function>
    void applyToDeclarations (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Function& function, const Index& index) const
    {
        applyToSymbols (pathIndex, document, element, tokenIndex, [this, &function, &index] (const jam::Document::Element& record)
        {
            applyToSymbolDeclarations (*record.get<juce::int64> (Id::symbol), function, index);
        }, index);
    }

    template <typename Function>
    void applyToScopeSymbols (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Function& function, const Index& index) const
    {
        const auto applyToScope { [&function, &index] (const jam::Document::Element& record)
        {
            const auto symbol { *record.get<juce::int64> (Id::symbol) };

            if (index.completions.contains (symbol))
                function (symbol);
            else
                for (const auto typeSymbol : *record.get<jam::Document::Integers> (Id::typeSymbols))
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

    template <typename Function>
    void applyToSymbolOccurrences (juce::int64 symbol, const Function& function, const Index& index) const
    {
        applyToSymbolDeclarations (symbol, function, index);
        applyToReferences (symbol, function, index);
    }

    template <typename Function>
    void applyToSymbolOccurrences (juce::int64 symbol, int pathIndex, const Function& function, const Index& index) const
    {
        const auto applyToOccurrenceMap { [symbol, pathIndex, &function, &index] (const jam::HashMap<juce::int64, jam::Array<std::pair<int, const jam::Document::Element*>>>& occurrences)
        {
            if (const auto entry { occurrences.find (symbol) }; entry != occurrences.end())
            {
                const auto& [occurrenceSymbol, symbolOccurrences] { *entry };

                index.applyToOccurrences (symbolOccurrences, pathIndex, [&function] (int occurrenceIndex, const jam::Document& occurrenceDocument, const jam::Document::Element& occurrence, int occurrenceTokenIndex, const jam::Document::Element&)
                {
                    function (occurrenceIndex, occurrenceDocument, occurrence, occurrenceTokenIndex);
                });
            }
        } };

        applyToOccurrenceMap (index.declarations);
        applyToOccurrenceMap (index.references);
    }

    /**
     * @brief Calls @p function for each reference that has the symbol of a declaration.
     *
     * The lookup reads the references that share the symbol of the declaration. A
     * declaration without Id::symbol calls nothing.
     *
     * @tparam Function   A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param declaration  The declaration whose references are visited.
     * @param function     The callable. It receives the path index, the document, and each reference.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToReferences (juce::int64 symbol, const Function& function, const Index& index) const
    {
        if (const auto entry { index.references.find (symbol) }; entry != index.references.end())
        {
            const auto& [referenceSymbol, symbolReferences] { *entry };

            index.applyToOccurrences (symbolReferences, [&function] (int referenceIndex, const jam::Document& referenceDocument, const jam::Document::Element& reference, int referenceTokenIndex, const jam::Document::Element&)
            {
                function (referenceIndex, referenceDocument, reference, referenceTokenIndex);
            });
        }
    }

    template <typename Function>
    static void applyToSymbolFile (const juce::File& file, const Function& function, const Index& index)
    {
        const auto pathIndex { index.paths.indexOf (file.getFullPathName(), false, 0) };

        if (const auto entry { index.symbolFiles.find (pathIndex) }; entry != index.symbolFiles.end())
        {
            const auto& [filePathIndex, unitFile] { *entry };
            const auto& [unitRow, symbolFile] { unitFile };

            if (const auto stored { index.documents.find (pathIndex) }; stored != index.documents.end())
            {
                const auto& [documentIndex, document] { *stored };

                function (*symbolFile, *document);
            }
        }
    }

    /**
     * @brief Calls @p function for the element at the position of a request.
     *
     * The arguments hold the file, the line, and the column, in that order. The
     * function runs only when the element covers the position in a stored
     * document. When the arguments are not valid, the overlay message reports
     * the failure.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param request    The request row. Its arguments give the position.
     * @param function      The callable. It receives the path index, the document, and the element.
     * @param columnOffset  The count of columns that is added to the column before the lookup. completion passes -1, so the position is the last byte of the typed name.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToPosition (juce::ValueTree& request, const Function& function, const Index& index, std::string_view encoding, int columnOffset = 0) const
    {
        const auto& arguments { *request[Id::arguments].getArray() };
        const int line { arguments[lineIndex] };
        const int column { arguments[columnIndex] };
        const auto pathIndex { getPathIndex (request, index) };

        if (const auto entry { index.documents.find (pathIndex) }; entry != index.documents.end())
        {
            const auto& [documentIndex, document] { *entry };
            const auto byteColumn { Index::getByteColumn (*document, line, column, encoding) + columnOffset };

            if (byteColumn > 0)
            {
                if (const auto [element, tokenIndex] { Index::getElement (*document, line, byteColumn) }; element != nullptr)
                    function (pathIndex, *document, *element, tokenIndex);
            }
        }
    }

    /**
     * @brief Calls @p function with the file that a request names, when the request holds one argument.
     *
     * Any other argument count sets the overlay message, and @p function is not called.
     *
     * @tparam Function  A callable type that takes (const juce::File&).
     * @param request    The request row. Its single argument is the file.
     * @param function   The callable. It receives the file.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToFile (const juce::ValueTree& request, const Function& function) const
    {
        function (getFile (request));
    }

    template <typename Function>
    void applyToDocument (juce::ValueTree& request, const Index& index, const Function& function) const
    {
        const auto& encodingText { request[Id::positionEncoding].toString() };
        const auto encoding { jam::Strings::getView (encodingText) };
        const auto pathIndex { getPathIndex (request, index) };

        setResult (request, [&index, pathIndex, encoding, &function] (juce::MemoryBlock& message)
        {
            if (const auto entry { index.documents.find (pathIndex) }; entry != index.documents.end())
            {
                const auto& [documentIndex, document] { *entry };

                function (message, pathIndex, *document, encoding);
            }
            else
            {
                jam::JsonWriter::appendNull (message);
            }
        });
    }

    /**
     * @brief Calls @p function for each function declaration at the position of a request.
     *
     * A call hierarchy starts at a callable declaration. The filter keeps each
     * declaration that has the function role of its language, and drops every other
     * declaration at the position.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param request    The request row. Its arguments give the position.
     * @param function   The callable. It receives the path index, the document, and each function declaration.
     * @param index  The index that holds the documents and the symbol maps.
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
     * @brief Calls @p function for each implementation of a declaration.
     *
     * A declaration with the class role has its derived classes as implementations.
     * A declaration whose parent has the class role, which is a member of a class,
     * has the member of the same name in each derived class. Any other declaration
     * has no implementation.
     *
     * @tparam Function    A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param pathIndex    The index of the document that holds the declaration.
     * @param declaration  The declaration whose implementations are visited.
     * @param function     The callable. It receives the path index, the document, and each implementation.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToImplementations (int pathIndex, const jam::Document::Element& declaration, const Function& function, const Index& index) const
    {
        const auto& document { *index.documents.at (pathIndex) };

        if (index.hasRole (document, declaration, jam::LanguageRole::classToken))
        {
            index.applyToRecords (pathIndex, document, declaration, getLastTokenIndex (declaration), [this, &function, &index] (const jam::Document::Element& record)
            {
                applyToDerived (*record.get<juce::int64> (Id::symbol), function, index);
            });
        }
        else if (index.hasRole (document, *declaration.parent, jam::LanguageRole::classToken))
        {
            const auto memberName { document.getView (declaration.get<jam::Document::Tokens> (Id::nameTokens)->last()) };

            index.applyToRecords (pathIndex, document, *declaration.parent, getLastTokenIndex (*declaration.parent), [this, &function, &index, memberName] (const jam::Document::Element& record)
            {
                applyToDerived (*record.get<juce::int64> (Id::symbol), [this, &function, &index, memberName] (int derivedIndex, const jam::Document& derivedDocument, const jam::Document::Element& derived, int)
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

    template <typename Function>
    void applyToDerived (juce::int64 symbol, const Function& function, const Index& index) const
    {
        jam::HashSet<juce::int64> knownSymbols;

        knownSymbols.insert (symbol);
        applyToDerivedFrom (symbol, function, knownSymbols, index);
    }

    template <typename Function>
    void applyToDerivedFrom (juce::int64 symbol, const Function& function, jam::HashSet<juce::int64>& knownSymbols, const Index& index) const
    {
        if (const auto entry { index.references.find (symbol) }; entry != index.references.end())
        {
            const auto& [referenceSymbol, symbolReferences] { *entry };

            index.applyToOccurrences (symbolReferences, [this, &function, &index, &knownSymbols] (int referenceIndex, const jam::Document& referenceDocument, const jam::Document::Element& reference, int, const jam::Document::Element& record)
            {
                if ((*record.get<int> (Id::role) & jam::LanguageRole::base) != 0)
                {
                    index.applyToRecords (referenceIndex, referenceDocument, *reference.parent, getLastTokenIndex (*reference.parent), [this, &function, &index, &knownSymbols, referenceIndex, &referenceDocument, &reference] (const jam::Document::Element& derivedRecord)
                    {
                        const auto derivedSymbol { *derivedRecord.get<juce::int64> (Id::symbol) };

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
     * @brief Tells whether one result comes before another in the result order.
     *
     * The path text gives the first order. For two elements of one path, the byte
     * offset of the last name token gives the order.
     *
     * @param pathIndex       The path index of the first element.
     * @param element         The first element.
     * @param otherPathIndex  The path index of the second element.
     * @param otherElement    The second element.
     * @return @c true when the first element comes before the second.
     * @param index  The index that holds the documents and the symbol maps.
     */
    bool isBefore (int pathIndex, const jam::Document::Element& element, int tokenIndex, int otherPathIndex, const jam::Document::Element& otherElement, int otherTokenIndex, const Index& index) const;

    /** @brief The scored declarations of one lookup: the score, the index of the path, and the element. */
    using Matches = jam::Array<std::tuple<int, int, const jam::Document::Element*>>;

    using Occurrence = std::pair<int, const jam::Document::Element*>;

    using Occurrences = jam::Array<Occurrence>;

    static const Occurrences& getOccurrences (const jam::HashMap<juce::int64, Occurrences>& map, juce::int64 symbol);

    static int getOccurrencePath (const Occurrence& occurrence);

    static bool isOccurrenceBefore (const Index& index, const Occurrence& left, const Occurrence& right);

    static void appendOccurrenceEdit (juce::MemoryBlock& edits, const Index& index, const Occurrence& occurrence, std::string_view newName, std::string_view encoding);

    static std::pair<const jam::Document::Element*, int> getCallReference (const Index& index, const Occurrence& occurrence);

    static const jam::Document::Element* getCaller (const Index& index, const Occurrence& occurrence);

    static void appendIncomingRuns (juce::MemoryBlock& entries, juce::int64 symbol, const Index& index, std::string_view encoding);

    static int appendIncomingRun (juce::MemoryBlock& entries, const Index& index, const Occurrences& references, int runStart, const jam::Document::Element& caller, std::string_view encoding);

    static int appendCallRanges (juce::MemoryBlock& ranges, const Index& index, const Occurrences& references, int runStart, const jam::Document::Element& caller, const jam::Document& document, std::string_view encoding);

    static int appendOutlines (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Tokens& outlines, int parentIndex, int outlineIndex, std::string_view encoding);

    static int appendOutline (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Tokens& outlines, int outlineIndex, std::string_view encoding);

    static void appendFileCodeActions (juce::MemoryBlock& json, const juce::File& file, const Index& index, const juce::Array<juce::var>& arguments, std::string_view encoding);

    static void appendFixItActions (juce::MemoryBlock& json, const juce::File& file, const jam::Document& document, const jam::Document::Element& symbolFile, uint32_t rangeStart, uint32_t rangeEnd, std::string_view encoding);

    static void appendCodeAction (juce::MemoryBlock& json, const juce::File& file, const jam::Document& document, const jam::Document::Token& range, std::string_view newText, std::string_view encoding);

    void appendScopeCompletions (juce::MemoryBlock& json, juce::int64 scopeSymbol, const jam::Fuzzy::Pattern& pattern, const Language& language, const Index& index) const;

    static void appendCompletionItem (juce::MemoryBlock& json, const jam::Document::Element& completion, const Language& language);

    int getRecordType (int pathIndex, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Index& index) const;

    void appendSemanticTokens (juce::MemoryBlock& json, int pathIndex, const Index& index, std::string_view encoding, uint32_t startOffset, uint32_t endOffset) const;

    static uint32_t appendTokenLines (juce::MemoryBlock& json, const jam::Document& document, uint32_t previousOffset, uint32_t startOffset, uint32_t endOffset, int type, int modifiers, std::string_view encoding);

    static void appendSegment (juce::MemoryBlock& json, const jam::Document& document, uint32_t previousOffset, uint32_t startOffset, uint32_t endOffset, int type, int modifiers, std::string_view encoding);

    static uint32_t getOffset (const jam::Document& document, int line, int column, std::string_view encoding);

    static juce::Range<uint32_t> getInnerRange (const jam::Document& document, const jam::Document::Element& element, int delimiters);

    static juce::Range<uint32_t> getElementSpan (const jam::Document::Element& element);

    static const jam::Document::Element* getEnclosingChild (const jam::Document::Element& element, uint32_t offset);

    static const jam::Document::Element* getInnermostElement (const jam::Document& document, uint32_t offset);

    static const jam::Document::Element* getOuterLevel (const jam::Document::Element& element);

    static const jam::Document::Element* getRoleElement (const jam::Document::Element& element, uint32_t offset, const Language& language, int role);

    static void appendSelectionChain (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Element& element, std::string_view encoding);

    static void appendSelectionRanges (juce::MemoryBlock& json, const jam::Document& document, uint32_t offset, std::string_view encoding);

    static juce::Range<uint32_t> getTextObjectRange (const jam::Document& document, const Language& language, const juce::String& typeName, const juce::Identifier& scopeName, uint32_t offset);

    template <typename Function>
    void setResult (juce::ValueTree& request, const Function& appendValue) const
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

    static void appendPosition (juce::MemoryBlock& json, const jam::Document& document, uint32_t offset, std::string_view encoding);

    static void appendRange (juce::MemoryBlock& json, const jam::Document& document, uint32_t startOffset, uint32_t endOffset, std::string_view encoding);

    static void appendUri (juce::MemoryBlock& json, const juce::File& file);

    static void appendDiagnostics (juce::MemoryBlock& json, const juce::File& file, const Index& index, std::string_view encoding);

    static void appendLocation (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Token& token, std::string_view encoding);

    static void appendNameLocation (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, std::string_view encoding);

    static void appendItem (juce::MemoryBlock& json, const juce::String& path, const jam::Document& document, const jam::Document::Element& element, int tokenIndex, const Index& index, std::string_view encoding);

    static void appendHighlight (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Token& token, std::string_view encoding);

    static void appendTextEdit (juce::MemoryBlock& json, const jam::Document& document, const jam::Document::Token& token, std::string_view newName, std::string_view encoding);

    static void appendWorkspaceEdit (juce::MemoryBlock& json, const Index& index, const Occurrences& declarations, const Occurrences& references, std::string_view newName, std::string_view encoding);

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
