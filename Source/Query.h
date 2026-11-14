#pragma once
#include "Index.h"
#include "JsonWriter.h"

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
     * @brief Writes the declarations at the position of a request, each with its scope.
     *
     * The scope is the path of the declaration without its last segment. Each
     * JSON element holds the scope as an array of names.
     *
     * @param request  The request row. Its arguments give the file and the position.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void hover (juce::ValueTree& request, const Index& index) const;

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
     * @brief Stores the results of a request as a JSON array, with no extra members.
     *
     * @param request  The request row that receives the JSON.
     * @param results  The pairs of a path index and an element.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void setResult (juce::ValueTree& request, jam::Array<std::pair<int, const jam::Document::Element*>> results, const Index& index) const;

    /**
     * @brief Stores a JSON text as the result of a request.
     *
     * The connection of the row reacts to the change of the result. It sends the
     * text to the client, and it removes the request row.
     *
     * @param request  The request row that receives the result.
     * @param json     The JSON text. The property holds a copy of it.
     */
    void setResult (juce::ValueTree& request, std::string_view json) const;

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

private:
    /** @brief The index of the file argument of a request. The line and the column follow it. */
    static constexpr int fileIndex { 0 };

    /**
     * @brief Tells whether the name of an element has at least one token.
     *
     * The language parse gives Id::name to every element that it adds, with zero
     * or more tokens. The lookups read the name of each element that they test,
     * so the element must hold Id::name.
     *
     * @param element  The element to test. It must hold Id::name.
     * @return @c true when the element has at least one name token.
     */
    static bool hasName (const jam::Document::Element& element);

    /** @brief Whether the fuzzy patterns that the lookups parse normalize diacritics. */
    static constexpr bool shouldNormalize { true };

    /**
     * @brief Returns the names of an element and of its ancestors, from the root down.
     *
     * @param document  The document that holds the element. Its tokens give the names.
     * @param element   The element whose path is returned.
     * @return The names of the enclosing declarations, then the name of the element.
     */
    static jam::Array<std::string_view> getPath (const jam::LanguageDocument& document, const jam::Document::Element& element);

    /**
     * @brief Tells whether two paths hold the same segments.
     *
     * @param path       The first path.
     * @param otherPath  The second path.
     * @return @c true when both paths have the same count of segments, and each pair of segments has equal text.
     */
    static bool isSamePath (const jam::Array<std::string_view>& path, const jam::Array<std::string_view>& otherPath);

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
    void applyToDeclarations (const jam::Document::Element& element, const Function& function, const Index& index) const
    {
        if (const auto* symbol { element.find<juce::String> (Id::symbol) }; symbol != nullptr)
        {
            if (const auto entry { index.declarations.find (*symbol) }; entry != index.declarations.end())
            {
                const auto& [declarationSymbol, symbolDeclarations] { *entry };

                for (const auto& [declarationIndex, declaration] : symbolDeclarations)
                    function (declarationIndex, *index.documents.at (declarationIndex), *declaration);
            }
        }
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
    void applyToReferences (const jam::Document::Element& declaration, const Function& function, const Index& index) const
    {
        if (const auto* symbol { declaration.find<juce::String> (Id::symbol) }; symbol != nullptr)
        {
            if (const auto entry { index.references.find (*symbol) }; entry != index.references.end())
            {
                const auto& [referenceSymbol, symbolReferences] { *entry };

                for (const auto& [referenceIndex, reference] : symbolReferences)
                    function (referenceIndex, *index.documents.at (referenceIndex), *reference);
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
    void applyToPosition (juce::ValueTree& request, const Function& function, const Index& index, int columnOffset = 0) const
    {
        static constexpr int positionArgumentCount { 3 };
        static constexpr int lineIndex { 1 };
        static constexpr int columnIndex { 2 };

        const auto& arguments { *request[Id::arguments].getArray() };
        const int line { arguments[lineIndex] };
        const int column { arguments[columnIndex] };

        if (arguments.size() == positionArgumentCount and line > 0 and column > 0)
        {
            const auto pathIndex { getPathIndex (request, index) };

            if (const auto entry { index.documents.find (pathIndex) }; entry != index.documents.end() and column + columnOffset > 0)
            {
                const auto& [documentIndex, document] { *entry };

                if (const auto* element { getElement (*document, line, column + columnOffset) }; element != nullptr)
                    function (pathIndex, *document, *element);
            }
        }
        else
        {
            index.model.setMessage (text::Diagnostics::failArguments);
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
    void applyToFile (const juce::ValueTree& request, const Function& function, const Index& index) const
    {
        static constexpr int fileArgumentCount { 1 };

        if (request[Id::arguments].getArray()->size() == fileArgumentCount)
            function (getFile (request));
        else
            index.model.setMessage (text::Diagnostics::failArguments);
    }

    /**
     * @brief Calls @p function for each file row that pairs with a file under the counterpart table of its language.
     *
     * The language is the family of the file. A file of a family with no loaded
     * library has no counterparts. A counterpart has the same name without its
     * extension, and an extension that the table pairs with the extension of the file.
     *
     * @tparam Function  A callable type that takes (const juce::File&).
     * @param file       The file whose counterparts are visited.
     * @param function   The callable. It receives each counterpart file.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToCounterparts (const juce::File& file, const Function& function, const Index& index) const
    {
        const auto family { jam::SyntaxTree::getFamily (file) };

        if (const auto language { index.languages.find (family) }; language != index.languages.end())
        {
            const auto& [languageFamily, library] { *language };
            const auto extension { file.getFileExtension().substring (1) };

            if (const auto pairing { library->counterparts.find (extension) }; pairing != library->counterparts.end())
            {
                const auto& [pairedExtension, extensions] { *pairing };

                for (const auto& row : index.model.state.getChildWithName (jam::Format::toType (Id::files)))
                {
                    const juce::File candidate { row[Id::path].toString() };

                    if (candidate.getFileNameWithoutExtension().compare (file.getFileNameWithoutExtension()) == 0
                        and extensions.contains (candidate.getFileExtension().substring (1)))
                        function (candidate);
                }
            }
        }
    }

    /**
     * @brief Calls @p function once for the first function declaration that a reference resolves to.
     *
     * The first declaration is the one that comes first in the order of isBefore.
     * A reference with no function declaration calls nothing.
     *
     * @tparam Function  A callable type that takes (const jam::LanguageDocument&, const jam::Document::Element&).
     * @param pathIndex  The index of the document that holds the reference.
     * @param reference  The reference whose declarations are visited.
     * @param function   The callable. It receives the document and the first function declaration.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToFirstFunction (int pathIndex, const jam::Document::Element& reference, const Function& function, const Index& index) const
    {
        std::pair<int, const jam::Document::Element*> first { -1, nullptr };

        applyToDeclarations (reference, [this, &first, &index] (int declarationIndex, const jam::LanguageDocument& declarationDocument, const jam::Document::Element& declaration)
        {
            const auto& [firstIndex, firstDeclaration] { first };

            if (declarationDocument.hasRole (declaration, jam::LanguageRole::function)
                and (firstDeclaration == nullptr or isBefore (declarationIndex, declaration, firstIndex, *firstDeclaration, index)))
                first = { declarationIndex, &declaration };
        }, index);

        if (const auto& [firstIndex, firstDeclaration] { first }; firstDeclaration != nullptr)
            function (*index.documents.at (firstIndex), *firstDeclaration);
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
    void applyToFunctions (juce::ValueTree& request, const Function& function, const Index& index) const
    {
        applyToPosition (request, [this, &function, &index] (int pathIndex, const jam::LanguageDocument&, const jam::Document::Element& element)
        {
            applyToDeclarations (element, [&function] (int declarationIndex, const jam::LanguageDocument& declarationDocument, const jam::Document::Element& declaration)
            {
                if (declarationDocument.hasRole (declaration, jam::LanguageRole::function))
                    function (declarationIndex, declarationDocument, declaration);
            }, index);
        }, index);
    }

    /**
     * @brief Calls @p function for each declaration that a reference can complete to.
     *
     * A member reference gives each member declaration of every stored document.
     * Any other reference gives the named declarations below each scope that
     * applyToScopes visits. The qualifier is the name of the reference without its
     * last segment.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param document   The document that holds the reference.
     * @param reference  The reference element whose name is completed.
     * @param function   The callable. It receives the path index, the document, and each candidate.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToCandidates (const jam::LanguageDocument& document, const jam::Document::Element& reference, const Function& function, const Index& index) const
    {
        if (reference.contains (Id::member))
        {
            index.applyToElements ([&function] (int pathIndex, const jam::LanguageDocument& declarationDocument, const jam::Document::Element& declaration)
            {
                if (declarationDocument.hasRole (declaration, jam::LanguageRole::member) and hasName (declaration))
                    function (pathIndex, declarationDocument, declaration);
            });
        }
        else
        {
            const auto name (getName (document, reference));

            applyToScopes (getPath (document, *reference.parent), getPath (name, name.size() - 1, {}), [&function] (int pathIndex, const jam::LanguageDocument& declarationDocument, const jam::Document::Element& declaration)
            {
                for (const auto* child : declaration)
                {
                    if (declarationDocument.hasRole (*child, jam::LanguageRole::declaration) and hasName (*child))
                        function (pathIndex, declarationDocument, *child);
                }
            }, index);
        }
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

        if (document.hasRole (declaration, jam::LanguageRole::classToken))
        {
            if (const auto* symbol { declaration.find<juce::String> (Id::symbol) }; symbol != nullptr)
                applyToDerived (*symbol, function, index);
        }
        else if (document.hasRole (*declaration.parent, jam::LanguageRole::classToken))
        {
            if (const auto* parentSymbol { declaration.parent->find<juce::String> (Id::symbol) }; parentSymbol != nullptr)
            {
                const auto memberName { document.getView (declaration.get<jam::Document::Tokens> (Id::name)->last()) };

                applyToDerived (*parentSymbol, [this, &function, &index, memberName] (int derivedIndex, const jam::LanguageDocument& derivedDocument, const jam::Document::Element& derived)
                {
                    for (const auto* child : derived)
                    {
                        if (derivedDocument.hasRole (*child, jam::LanguageRole::declaration) and hasName (*child)
                            and derivedDocument.getView (child->get<jam::Document::Tokens> (Id::name)->last()).compare (memberName) == 0)
                            applyToDeclarations (*child, function, index);
                    }
                }, index);
            }
        }
    }

    /**
     * @brief Returns the first element whose last name token covers a position.
     *
     * The line and the column are one-based, as a request sends them. The tree
     * walk stops at the first element that covers the position.
     *
     * @param document  The document to search.
     * @param line      The one-based line of the position.
     * @param column    The one-based column of the position.
     * @return The element that covers the position, or nullptr when no element covers it.
     */
    const jam::Document::Element* getElement (const jam::LanguageDocument& document, int line, int column) const;

    /**
     * @brief Calls @p function for each declaration whose path equals @p path.
     *
     * The path is matched in each stored document.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param path       The declaration path to match.
     * @param function   The callable. It receives the path index, the document, and each matching declaration.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToDeclarations (const jam::Array<std::string_view>& path, const Function& function, const Index& index) const
    {
        for (const auto& [pathIndex, document] : index.documents)
            applyToElements (*document, *document->getRoot(), path, 0, [&function, pathIndex = pathIndex, document = document.get()] (const jam::Document::Element& declaration)
            {
                function (pathIndex, *document, declaration);
            });
    }

    /**
     * @brief Calls @p function for each element that derives from a class, directly or through others.
     *
     * The walk runs over a list of symbols that starts with the symbol of the base
     * class. A reference whose role holds the base bit names a derived class. A
     * jam::HashSet keeps each symbol that the walk has seen, so each derived class
     * is reported once. The walk copies each symbol before it runs, because the
     * walk adds to the list, and the add can move the storage of the list.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param symbol     The symbol of the base class.
     * @param function   The callable. It receives the path index, the document, and each derived element.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToDerived (const juce::String& symbol, const Function& function, const Index& index) const
    {
        jam::Array<juce::String> symbols { symbol };
        jam::HashSet<juce::String> knownSymbols;

        knownSymbols.insert (symbol);

        for (int position { 0 }; position < symbols.size(); ++position)
        {
            const auto current (symbols.at (position));

            if (const auto entry { index.references.find (current) }; entry != index.references.end())
            {
                const auto& [referenceSymbol, symbolReferences] { *entry };

                for (const auto& [referenceIndex, reference] : symbolReferences)
                {
                    if ((*reference->find<int> (Id::role) & jam::LanguageRole::base) != 0)
                    {
                        if (const auto* derivedSymbol { reference->parent->find<juce::String> (Id::symbol) }; derivedSymbol != nullptr)
                        {
                            auto [knownEntry, isAdded] { knownSymbols.insert (*derivedSymbol) };

                            if (isAdded)
                            {
                                function (referenceIndex, *index.documents.at (referenceIndex), *reference->parent);
                                symbols.add (*derivedSymbol);
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * @brief Calls @p function for each declaration that a name can see from a scope.
     *
     * The scope paths are the first segments of scope, each followed by qualifier,
     * for each count from the full scope down to zero. The walk does not stop at
     * the first scope that has a match. An empty path gives the root of each
     * stored document. Each base class of a visited class adds its paths to the
     * walk once, so a class that names itself as its base ends the walk.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param scope      The path of the scope that holds the reference.
     * @param qualifier  The segments of a qualified name, without its last segment.
     * @param function   The callable. It receives the path index, the document, and each scope element.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToScopes (const jam::Array<std::string_view>& scope, const jam::Array<std::string_view>& qualifier, const Function& function, const Index& index) const
    {
        jam::Array<jam::Array<std::string_view>> scopePaths;

        for (int size { scope.size() }; size >= 0; --size)
            scopePaths.add (getPath (scope, size, qualifier));

        for (int position { 0 }; position < scopePaths.size(); ++position)
        {
            const auto path (getPath (scopePaths.at (position), scopePaths.at (position).size(), {}));

            if (path.isEmpty())
            {
                for (const auto& [pathIndex, document] : index.documents)
                    function (pathIndex, *document, *document->getRoot());
            }
            else
            {
                applyToDeclarations (path, function, index);
                applyToBases (path, [&index, &scopePaths] (const jam::LanguageDocument&, const jam::Document::Element& base)
                {
                    if (const auto* symbol { base.find<juce::String> (Id::symbol) }; symbol != nullptr)
                    {
                        if (const auto entry { index.declarations.find (*symbol) }; entry != index.declarations.end())
                        {
                            const auto& [declarationSymbol, symbolDeclarations] { *entry };

                            for (const auto& [declarationIndex, declaration] : symbolDeclarations)
                            {
                                auto basePath (getPath (*index.documents.at (declarationIndex), *declaration));

                                if (not hasPath (scopePaths, basePath))
                                    scopePaths.add (std::move (basePath));
                            }
                        }
                    }
                }, index);
            }
        }
    }

    /**
     * @brief Calls @p function for each declaration below a parent whose name matches a path.
     *
     * A declaration name can hold more than one token, such as a namespace path.
     * Each name consumes as many path segments as it has tokens. A declaration
     * without a name passes the segments to its children. A declaration whose
     * name ends the path is a match.
     *
     * @tparam Function  A callable type that takes (const jam::Document::Element&).
     * @param document   The document that holds the parent. Its source gives the token text.
     * @param parent     The element whose children are searched.
     * @param path       The path segments to match.
     * @param start      The index of the first path segment that the name must match.
     * @param function   The callable. It receives each matching declaration.
     */
    template <typename Function>
    void applyToElements (const jam::LanguageDocument& document, const jam::Document::Element& parent, const jam::Array<std::string_view>& path, int start, const Function& function) const
    {
        for (const auto* child : parent)
        {
            if (document.hasRole (*child, jam::LanguageRole::declaration))
            {
                const auto& name { *child->get<jam::Document::Tokens> (Id::name) };
                const auto count { name.size() };

                if (start + count <= path.size()
                    and std::equal (name.begin(), name.end(), path.begin() + start, [&document] (const auto& token, std::string_view segment)
                    {
                        return document.getView (token).compare (segment) == 0;
                    }))
                {
                    if (count > 0 and start + count == path.size())
                        function (*child);
                    else
                        applyToElements (document, *child, path, start + count, function);
                }
            }
        }
    }

    /**
     * @brief Calls @p function for each base clause of the class at a path.
     *
     * A path can have more than one declaration. Each declaration with the class
     * role gives its base clauses, and any other declaration gives none.
     *
     * @tparam Function  A callable type that takes (const jam::LanguageDocument&, const jam::Document::Element&).
     * @param classPath  The path of the class.
     * @param function   The callable. It receives the document and each base clause.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToBases (const jam::Array<std::string_view>& classPath, const Function& function, const Index& index) const
    {
        applyToDeclarations (classPath, [&function] (int, const jam::LanguageDocument& document, const jam::Document::Element& declaration)
        {
            if (document.hasRole (declaration, jam::LanguageRole::classToken))
                for (const auto* child : declaration)
                    if (document.hasRole (*child, jam::LanguageRole::base))
                        function (document, *child);
        }, index);
    }

    /**
     * @brief Returns the text of each name token of an element.
     *
     * @param document  The document that holds the element.
     * @param element   The element whose name is returned.
     * @return The name segments in order. The list is empty when the element has no name.
     */
    static jam::Array<std::string_view> getName (const jam::LanguageDocument& document, const jam::Document::Element& element);

    /**
     * @brief Returns the first size segments of a scope, followed by a name.
     *
     * A lookup tries each shorter scope in turn. Each try copies the scope with a
     * smaller size, and then it appends the name.
     *
     * @param scope  The path whose first segments are copied. Its size is at least @p size.
     * @param size   The count of segments of the scope to copy.
     * @param name   The segments that are appended after the copied segments.
     * @return The copied segments, then the name segments.
     */
    static jam::Array<std::string_view> getPath (const jam::Array<std::string_view>& scope, int size, const jam::Array<std::string_view>& name);

    /**
     * @brief Tells whether a list of paths holds a path.
     *
     * @param knownPaths  The paths to search.
     * @param path        The path to find.
     * @return @c true when a known path has the same segments as the path.
     */
    static bool hasPath (const jam::Array<jam::Array<std::string_view>>& knownPaths, const jam::Array<std::string_view>& path);

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
    bool isBefore (int pathIndex, const jam::Document::Element& element, int otherPathIndex, const jam::Document::Element& otherElement, const Index& index) const;

    /** @brief The scored declarations of one lookup: the score, the index of the path, and the element. */
    using Matches = jam::Array<std::tuple<int, int, const jam::Document::Element*>>;

    /**
     * @brief Stores the results of a request as a JSON array in its result.
     *
     * The function sorts a copy of the results by isBefore, and it removes each
     * pair that equals the pair before it. The sort gives the JSON the same order
     * for each run, whatever the order of the hash map.
     *
     * @tparam Function      A callable type that takes (std::string&, int, const jam::Document::Element&).
     * @param request        The request row that receives the JSON.
     * @param results        The pairs of a path index and an element.
     * @param appendMembers  The callable that appends the extra members of each element.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void setResult (juce::ValueTree& request, jam::Array<std::pair<int, const jam::Document::Element*>> results, const Function& appendMembers, const Index& index) const
    {
        std::sort (results.begin(), results.end(), [this, &index] (const auto& left, const auto& right)
        {
            const auto& [leftIndex, leftElement] { left };
            const auto& [rightIndex, rightElement] { right };
            return isBefore (leftIndex, *leftElement, rightIndex, *rightElement, index);
        });

        const auto tail { std::unique (results.begin(), results.end()) };
        std::string json;

        results.removeRange (static_cast<int> (tail - results.begin()), static_cast<int> (results.end() - tail));

        jam::JsonWriter::appendArray (json, [&index, &results, &appendMembers] (std::string& entries)
        {
            for (const auto& [pathIndex, element] : results)
                JsonWriter::appendElement (entries, index.paths.getView (pathIndex), *index.documents.at (pathIndex), *element, [&appendMembers, pathIndex = pathIndex, element = element] (std::string& members)
                {
                    appendMembers (members, pathIndex, *element);
                });
        });

        setResult (request, json);
    }

    /**
     * @brief Tells whether one element comes before another in the order by path.
     *
     * The path segments give the first order, compared from the first segment. Two
     * elements with the same path are ordered by isBefore.
     *
     * @param pathIndex       The path index of the first element.
     * @param element         The first element.
     * @param otherPathIndex  The path index of the second element.
     * @param otherElement    The second element.
     * @return @c true when the first element comes before the second.
     * @param index  The index that holds the documents and the symbol maps.
     */
    bool isBeforeByPath (int pathIndex, const jam::Document::Element& element, int otherPathIndex, const jam::Document::Element& otherElement, const Index& index) const;

    /**
     * @brief Appends the declarations below a parent as a tree of JSON elements.
     *
     * A declaration without a name has no position. Its declarations become
     * entries of its parent, so the walk appends them to the same JSON text.
     *
     * @param json       The JSON text that receives the entries.
     * @param pathIndex  The index of the document that holds the parent.
     * @param parent     The element whose declarations are appended.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void appendDocumentSymbols (std::string& json, int pathIndex, const jam::Document::Element& parent, const Index& index) const;

    /**
     * @brief Appends one completion entry: the element, its comment, and the snippet of a function.
     *
     * The comment comes from the comment declaration, which can be another element
     * of the same symbol. The snippet is written only for the function types.
     *
     * @param json                The JSON text that receives the entry.
     * @param pathIndex           The index of the document that holds the declaration.
     * @param declarationDocument The document that holds the declaration.
     * @param declaration         The declaration of the entry.
     * @param commentDocument     The document that holds the comment declaration.
     * @param commentDeclaration  The element whose Id::comment is written.
     * @param index  The index that holds the documents and the symbol maps.
     */
    void appendCompletion (std::string& json, int pathIndex, const jam::LanguageDocument& declarationDocument, const jam::Document::Element& declaration, const jam::LanguageDocument& commentDocument, const jam::Document::Element& commentDeclaration, const Index& index) const;

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
    jam::Fuzzy::Result getMatch (const jam::Fuzzy::Pattern& pattern, const jam::LanguageDocument& document, const jam::Document::Element& element) const;

    /**
     * @brief Returns the declarations that a reference can complete to, with their scores, in completion order.
     *
     * Each candidate of applyToCandidates whose name matches the pattern is kept.
     * The list is sorted by descending score, then by the path segments of each
     * declaration, then by its file and its position. Entries with the same path are
     * adjacent, because they have the same name, so they have the same score.
     *
     * @param document   The document that holds the reference.
     * @param reference  The reference element whose name is completed.
     * @param pattern    The parsed name of the reference.
     * @return Tuples of the score, the path index, and the element.
     * @param index  The index that holds the documents and the symbol maps.
     */
    Matches getCompletions (const jam::LanguageDocument& document, const jam::Document::Element& reference, const jam::Fuzzy::Pattern& pattern, const Index& index) const;

    /**
     * @brief Calls @p function once for each symbol that a reference can complete to.
     *
     * Entries with the same path are one symbol. The first entry of a symbol, in
     * the order of getCompletions, is the declaration. The comment is the Id::comment
     * of the first entry of the symbol that has one. When no entry has one, the
     * comment element is the first entry, and it holds no Id::comment.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&, const jam::LanguageDocument&, const jam::Document::Element&).
     * @param document   The document that holds the reference.
     * @param reference  The reference element whose name is completed.
     * @param pattern    The parsed name of the reference.
     * @param function   The callable. It receives the path index, the document, and the declaration, then the document and the element that holds the comment.
     * @param index  The index that holds the documents and the symbol maps.
     */
    template <typename Function>
    void applyToCompletions (const jam::LanguageDocument& document, const jam::Document::Element& reference, const jam::Fuzzy::Pattern& pattern, const Function& function, const Index& index) const
    {
        const auto completions (getCompletions (document, reference, pattern, index));

        for (int position { 0 }; position < completions.size();)
        {
            const auto& [score, pathIndex, declaration] { completions.at (position) };
            const auto declarationPath (getPath (*index.documents.at (pathIndex), *declaration));
            const auto runEnd { std::find_if (completions.begin() + position, completions.end(), [&index, &declarationPath] (const auto& entry)
            {
                const auto& [otherScore, otherIndex, otherDeclaration] { entry };

                return not isSamePath (declarationPath, getPath (*index.documents.at (otherIndex), *otherDeclaration));
            }) };
            const auto commentEntry { std::find_if (completions.begin() + position, runEnd, [] (const auto& entry)
            {
                const auto& [otherScore, otherIndex, otherDeclaration] { entry };

                return otherDeclaration->contains (Id::comment);
            }) };
            const auto& [commentScore, commentIndex, commentDeclaration] { commentEntry == runEnd ? completions.at (position) : *commentEntry };

            function (pathIndex, *index.documents.at (pathIndex), *declaration, *index.documents.at (commentIndex), *commentDeclaration);

            position = static_cast<int> (runEnd - completions.begin());
        }
    }

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
