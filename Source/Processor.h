#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "JsonWriter.h"

/**
 * @class Processor
 * @brief Parses the source files of the model on a pool thread, and keeps the parsed documents.
 *
 * Each file row of the model starts one parse job. No ValueTree reaches the
 * pool thread. A job returns its result to the message thread, and the result
 * is kept only when the row still holds the same path and the file still has
 * the modification time that the job read. The documents are keyed by the index
 * of their path in paths.
 *
 * The processor also answers the request rows of the model. Each request row
 * runs the entry of events that its command names. The entry writes JSON to the
 * result of the row. The lookups read the documents in place and keep nothing.
 */
class Processor : public juce::ValueTree::Listener
{
public:
    /**
     * @brief Maps each request command to the operation that answers it.
     *
     * Each entry takes the request row, reads its arguments, and writes its
     * result. The key is the command word of the request.
     */
    using Events = jam::Function::Map<juce::Identifier, jam::Function::pointerCapacity, void (juce::ValueTree&)>;

    /**
     * @brief Registers the processor as a listener of the model.
     *
     * @param newModel  The model whose changes start the parse jobs.
     */
    explicit Processor (LINGModel& newModel);

    /** @brief Removes the processor from the listeners of the model. */
    ~Processor() override;

    /**
     * @brief Calls @p function for each stored document, in ascending order of the path bytes.
     *
     * The documents are held in a HashMap, which has no order, so the path
     * indices are sorted first. The view that @p function receives points into
     * paths, and it is valid during the call.
     *
     * @tparam Function  A callable type that takes (std::string_view, const jam::CodeDocument&).
     * @param function   The callable. It receives the path and the document of each stored entry.
     */
    template <typename Function>
    void applyToDocuments (const Function& function) const
    {
        jam::Array<int> pathIndices;

        for (const auto& [pathIndex, document] : documents)
            pathIndices.add (pathIndex);

        std::sort (pathIndices.begin(), pathIndices.end(), [this] (int left, int right)
        {
            return paths.getView (left).compare (paths.getView (right)) < 0;
        });

        for (const auto pathIndex : pathIndices)
            function (paths.getView (pathIndex), *documents.at (pathIndex));
    }

    /**
     * @brief Tells whether an element declares a name.
     *
     * The declaration types form one range of the type enumeration, from the
     * namespace token to the macro token. Every other type is a reference.
     *
     * @param element  The element to test.
     * @return @c true when the type of the element lies in the declaration range.
     */
    static bool isDeclaration (const jam::Document::Element& element);

private:
    /** @brief The scored declarations of one lookup: the score, the index of the path, and the element. */
    using Matches = jam::Array<std::tuple<int, int, const jam::Document::Element*>>;

    /**
     * @brief Tells whether the type of an element is one of the types in a table.
     *
     * @tparam Types     The table type, a std::array of map::SyntaxTokenType values.
     * @param types      The table of types.
     * @param element    The element to test. It must hold Id::type.
     * @return @c true when the type of the element is in the table.
     */
    template <typename Types>
    static bool hasType (const Types& types, const jam::Document::Element& element)
    {
        return std::find (types.begin(), types.end(), *element.get<int> (Id::type)) != types.end();
    }

    /**
     * @brief Tells whether the name of an element has at least one token.
     *
     * The C-family parse gives Id::name to every element that it adds. Every
     * element that a lookup tests is one of those elements.
     *
     * @param element  The element to test. It must hold Id::name.
     * @return @c true when the element has at least one name token.
     */
    static bool hasName (const jam::Document::Element& element);

    /** @brief The index of the file argument of a request. The line and the column follow it. */
    static constexpr int fileIndex { 0 };

    /**
     * @brief The types of a class member: a field, an instance method, a static method, and a static variable.
     *
     * The member lookups match only these types.
     */
    static constexpr std::array memberTypes { map::SyntaxTokenType::field, map::SyntaxTokenType::instanceMethod, map::SyntaxTokenType::staticMethod, map::SyntaxTokenType::staticVariable };

    /**
     * @brief The types of a callable declaration: a function, an instance method, a static method, a constructor, and a destructor.
     *
     * The call hierarchy keeps these types. A completion writes a snippet only for these types.
     */
    static constexpr std::array functionTypes { map::SyntaxTokenType::function, map::SyntaxTokenType::instanceMethod, map::SyntaxTokenType::staticMethod, map::SyntaxTokenType::constructor, map::SyntaxTokenType::destructor };

    /** @brief Whether the fuzzy patterns that the lookups parse normalize diacritics. */
    static constexpr bool shouldNormalize { true };

    /**
     * @brief Returns the names of an element and of its ancestors, from the root down.
     *
     * @param document  The document that holds the element. Its tokens give the names.
     * @param element   The element whose path is returned.
     * @return The names of the enclosing declarations, then the name of the element.
     */
    static jam::Array<std::string_view> getPath (const jam::CodeDocument& document, const jam::Document::Element& element);

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
    const jam::Document::Element* getElement (const jam::CodeDocument& document, int line, int column) const;

    /**
     * @brief Calls @p function for each declaration that an element refers to.
     *
     * An element with a target list gives its declarations at once. A member
     * element looks up the members of its class. Any other element resolves its
     * declaration paths, and each path visits every stored document.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param pathIndex  The index of the document that holds the element.
     * @param element    The element whose declarations are visited.
     * @param function   The callable. It receives the path index, the document, and each declaration.
     */
    template <typename Function>
    void applyToDeclarations (int pathIndex, const jam::Document::Element& element, const Function& function) const
    {
        const auto& document { *documents.at (pathIndex) };

        if (const auto* targets { element.find<jam::Document::Elements> (Id::to) }; targets != nullptr)
        {
            for (const auto* target : *targets)
                function (pathIndex, document, *target);
        }
        else if (element.contains (Id::member))
        {
            applyToMembers (getName (document, element).last(), function);
        }
        else
        {
            for (const auto& path : getDeclarationPaths (document, element))
                applyToDeclarations (path, function);
        }
    }

    /**
     * @brief Calls @p function for each declaration whose path equals @p path.
     *
     * The path is matched in each stored document.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param path       The declaration path to match.
     * @param function   The callable. It receives the path index, the document, and each matching declaration.
     */
    template <typename Function>
    void applyToDeclarations (const jam::Array<std::string_view>& path, const Function& function) const
    {
        for (const auto& [pathIndex, document] : documents)
            applyToElements (*document, *document->getRoot(), path, 0, [&function, pathIndex = pathIndex, document = document.get()] (const jam::Document::Element& declaration)
            {
                function (pathIndex, *document, declaration);
            });
    }

    /**
     * @brief Calls @p function for each reference to a declaration.
     *
     * A declaration with a reference list gives its references at once. Any other
     * declaration finds its references by its path, in every stored document.
     *
     * @tparam Function   A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param pathIndex    The index of the document that holds the declaration.
     * @param declaration  The declaration whose references are visited.
     * @param function     The callable. It receives the path index, the document, and each reference.
     */
    template <typename Function>
    void applyToReferences (int pathIndex, const jam::Document::Element& declaration, const Function& function) const
    {
        const auto& document { *documents.at (pathIndex) };

        if (const auto* references { declaration.find<jam::Document::Elements> (Id::in) }; references != nullptr)
        {
            for (const auto* reference : *references)
                function (pathIndex, document, *reference);
        }
        else
        {
            const auto declarationPath (getPath (document, declaration));

            jassert (not declarationPath.isEmpty());

            applyToReferences (declarationPath, function);
        }
    }

    /**
     * @brief Calls @p function for each element that refers to a declaration path.
     *
     * @tparam Function        A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param declarationPath  The path of the declaration.
     * @param function         The callable. It receives the path index, the document, and each reference.
     */
    template <typename Function>
    void applyToReferences (const jam::Array<std::string_view>& declarationPath, const Function& function) const
    {
        applyToElements ([this, &function, &declarationPath] (int pathIndex, const jam::CodeDocument& document, const jam::Document::Element& reference)
        {
            if (isReferenceTo (pathIndex, reference, declarationPath))
                function (pathIndex, document, reference);
        });
    }

    /**
     * @brief Calls @p function for the element at the position of a request.
     *
     * The arguments hold the file, the line, and the column, in that order. The
     * function runs only when the element covers the position in a stored
     * document. When the arguments are not valid, the overlay message reports
     * the failure.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param request    The request row. Its arguments give the position.
     * @param function      The callable. It receives the path index, the document, and the element.
     * @param columnOffset  The count of columns that is added to the column before the lookup. completion passes -1, so the position is the last byte of the typed name.
     */
    template <typename Function>
    void applyToPosition (juce::ValueTree& request, const Function& function, int columnOffset = 0) const
    {
        static constexpr int positionArgumentCount { 3 };
        static constexpr int lineIndex { 1 };
        static constexpr int columnIndex { 2 };

        const auto& arguments { *request[Id::arguments].getArray() };
        const int line { arguments[lineIndex] };
        const int column { arguments[columnIndex] };

        if (arguments.size() == positionArgumentCount and line > 0 and column > 0)
        {
            const auto pathIndex { getPathIndex (request) };

            if (documents.contains (pathIndex) and column + columnOffset > 0)
                if (const auto* element { getElement (*documents.at (pathIndex), line, column + columnOffset) }; element != nullptr)
                    function (pathIndex, *documents.at (pathIndex), *element);
        }
        else
        {
            model.setMessage (text::Diagnostics::failArguments);
        }
    }

    /**
     * @brief Calls @p function for each function declaration at the position of a request.
     *
     * A call hierarchy starts at a callable declaration. The filter keeps the
     * function, instance method, static method, constructor, and destructor types,
     * and drops every other declaration at the position.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param request    The request row. Its arguments give the position.
     * @param function   The callable. It receives the path index, the document, and each function declaration.
     */
    template <typename Function>
    void applyToFunctions (juce::ValueTree& request, const Function& function) const
    {
        applyToPosition (request, [this, &function] (int pathIndex, const jam::CodeDocument&, const jam::Document::Element& element)
        {
            applyToDeclarations (pathIndex, element, [&function] (int declarationIndex, const jam::CodeDocument& declarationDocument, const jam::Document::Element& declaration)
            {
                if (hasType (functionTypes, declaration))
                    function (declarationIndex, declarationDocument, declaration);
            });
        });
    }

    /**
     * @brief Calls @p function for each class or struct that derives from a class, directly or through others.
     *
     * The walk runs over a list of class paths, and each derived class is
     * reported once. The walk copies each path before it runs, because the
     * callback adds to the list, and the add can move the storage of the list.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param classPath  The path of the base class.
     * @param function   The callable. It receives the path index, the document, and each derived class.
     */
    template <typename Function>
    void applyToDerived (const jam::Array<std::string_view>& classPath, const Function& function) const
    {
        jam::Array<jam::Array<std::string_view>> classPaths;

        classPaths.add (getPath (classPath, classPath.size(), {}));

        for (int index { 0 }; index < classPaths.size(); ++index)
        {
            const auto path (getPath (classPaths.at (index), classPaths.at (index).size(), {}));

            applyToReferences (path, [&function, &classPaths] (int pathIndex, const jam::CodeDocument& document, const jam::Document::Element& reference)
            {
                if (*reference.get<int> (Id::type) == map::SyntaxTokenType::base)
                {
                    auto derivedPath (getPath (document, *reference.parent));

                    if (not hasPath (classPaths, derivedPath))
                    {
                        function (pathIndex, document, *reference.parent);
                        classPaths.add (std::move (derivedPath));
                    }
                }
            });
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
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param scope      The path of the scope that holds the reference.
     * @param qualifier  The segments of a qualified name, without its last segment.
     * @param function   The callable. It receives the path index, the document, and each scope element.
     */
    template <typename Function>
    void applyToScopes (const jam::Array<std::string_view>& scope, const jam::Array<std::string_view>& qualifier, const Function& function) const
    {
        jam::Array<jam::Array<std::string_view>> scopePaths;

        for (int size { scope.size() }; size >= 0; --size)
            scopePaths.add (getPath (scope, size, qualifier));

        for (int index { 0 }; index < scopePaths.size(); ++index)
        {
            const auto path (getPath (scopePaths.at (index), scopePaths.at (index).size(), {}));

            if (path.isEmpty())
            {
                for (const auto& [pathIndex, document] : documents)
                    function (pathIndex, *document, *document->getRoot());
            }
            else
            {
                applyToDeclarations (path, function);
                applyToBases (path, [this, &path, &scopePaths] (const jam::CodeDocument& baseDocument, const jam::Document::Element& base)
                {
                    for (auto& basePath : getDeclarationPaths (path, getName (baseDocument, base), {}))
                        if (not hasPath (scopePaths, basePath))
                            scopePaths.add (std::move (basePath));
                });
            }
        }
    }

    /**
     * @brief Calls @p function for each declaration that a reference can complete to.
     *
     * A member reference gives each member declaration of every stored document.
     * Any other reference gives the named declarations below each scope that
     * applyToScopes visits. The qualifier is the name of the reference without its
     * last segment.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param document   The document that holds the reference.
     * @param reference  The reference element whose name is completed.
     * @param function   The callable. It receives the path index, the document, and each candidate.
     */
    template <typename Function>
    void applyToCandidates (const jam::CodeDocument& document, const jam::Document::Element& reference, const Function& function) const
    {
        if (reference.contains (Id::member))
        {
            applyToElements ([&function] (int pathIndex, const jam::CodeDocument& declarationDocument, const jam::Document::Element& declaration)
            {
                if (hasType (memberTypes, declaration) and hasName (declaration))
                    function (pathIndex, declarationDocument, declaration);
            });
        }
        else
        {
            const auto name (getName (document, reference));

            applyToScopes (getPath (document, *reference.parent), getPath (name, name.size() - 1, {}), [&function] (int pathIndex, const jam::CodeDocument& declarationDocument, const jam::Document::Element& declaration)
            {
                for (const auto* child : declaration)
                {
                    if (isDeclaration (*child) and hasName (*child))
                        function (pathIndex, declarationDocument, *child);
                }
            });
        }
    }

    /**
     * @brief Calls @p function for each implementation of a declaration.
     *
     * A class or a struct has its derived classes as implementations. A member
     * of a class has the member of the same name in each derived class. Any
     * other declaration has no implementation.
     *
     * @tparam Function    A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param pathIndex    The index of the document that holds the declaration.
     * @param declaration  The declaration whose implementations are visited.
     * @param function     The callable. It receives the path index, the document, and each implementation.
     */
    template <typename Function>
    void applyToImplementations (int pathIndex, const jam::Document::Element& declaration, const Function& function) const
    {
        const auto& document { *documents.at (pathIndex) };
        const auto type { *declaration.get<int> (Id::type) };
        const auto* parentType { declaration.parent->find<int> (Id::type) };

        if (type == map::SyntaxTokenType::classToken or type == map::SyntaxTokenType::structToken)
        {
            applyToDerived (getPath (document, declaration), function);
        }
        else if (parentType != nullptr and (*parentType == map::SyntaxTokenType::classToken or *parentType == map::SyntaxTokenType::structToken))
        {
            jam::Array<std::string_view> memberName;

            memberName.add (getName (document, declaration).last());

            applyToDerived (getPath (document, *declaration.parent), [this, &function, &memberName] (int, const jam::CodeDocument& derivedDocument, const jam::Document::Element& derived)
            {
                const auto derivedPath (getPath (derivedDocument, derived));

                applyToDeclarations (getPath (derivedPath, derivedPath.size(), memberName), function);
            });
        }
    }

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
     */
    template <typename Function>
    void setResult (juce::ValueTree& request, jam::Array<std::pair<int, const jam::Document::Element*>> results, const Function& appendMembers) const
    {
        std::sort (results.begin(), results.end(), [this] (const auto& left, const auto& right)
        {
            const auto& [leftIndex, leftElement] { left };
            const auto& [rightIndex, rightElement] { right };
            return isBefore (leftIndex, *leftElement, rightIndex, *rightElement);
        });

        const auto tail { std::unique (results.begin(), results.end()) };
        std::string json;

        results.removeRange (static_cast<int> (tail - results.begin()), static_cast<int> (results.end() - tail));

        jam::JsonWriter::appendArray (json, [this, &results, &appendMembers] (std::string& entries)
        {
            for (const auto& [pathIndex, element] : results)
                JsonWriter::appendElement (entries, paths.getView (pathIndex), *documents.at (pathIndex), *element, [&appendMembers, pathIndex = pathIndex, element = element] (std::string& members)
                {
                    appendMembers (members, pathIndex, *element);
                });
        });

        setResult (request, json);
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
    static void applyToElements (const jam::CodeDocument& document, const jam::Document::Element& parent, const jam::Array<std::string_view>& path, int start, const Function& function)
    {
        for (const auto* child : parent)
        {
            if (isDeclaration (*child))
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
     * @brief Calls @p function for each element of each stored document, at every depth.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param function   The callable. It receives the path index, the document, and each element.
     */
    template <typename Function>
    void applyToElements (const Function& function) const
    {
        for (const auto& [pathIndex, document] : documents)
            for (const auto* child : *document)
                child->applyFunctionRecursively ([&function, pathIndex = pathIndex, document = document.get()] (const jam::Document::Element& element)
                {
                    function (pathIndex, *document, element);
                });
    }

    /**
     * @brief Calls @p function for each base clause of the class or struct at a path.
     *
     * A path can have more than one declaration. Each class or struct declaration
     * gives its base clauses, and any other declaration gives none.
     *
     * @tparam Function  A callable type that takes (const jam::CodeDocument&, const jam::Document::Element&).
     * @param classPath  The path of the class or struct.
     * @param function   The callable. It receives the document and each base clause.
     */
    template <typename Function>
    void applyToBases (const jam::Array<std::string_view>& classPath, const Function& function) const
    {
        applyToDeclarations (classPath, [&function] (int, const jam::CodeDocument& document, const jam::Document::Element& declaration)
        {
            const auto type { *declaration.get<int> (Id::type) };

            if (type == map::SyntaxTokenType::classToken or type == map::SyntaxTokenType::structToken)
                for (const auto* child : declaration)
                    if (*child->get<int> (Id::type) == map::SyntaxTokenType::base)
                        function (document, *child);
        });
    }

    /**
     * @brief Calls @p function for each member named @p name in any stored document.
     *
     * A member is a field, an instance method, a static method, or a static
     * variable. The last name token of the member must equal the name.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&).
     * @param name       The member name, without its class path.
     * @param function   The callable. It receives the path index, the document, and each member.
     */
    template <typename Function>
    void applyToMembers (std::string_view name, const Function& function) const
    {
        applyToElements ([&function, name] (int pathIndex, const jam::CodeDocument& document, const jam::Document::Element& declaration)
        {
            const auto& tokens { *declaration.get<jam::Document::Tokens> (Id::name) };

            if (hasType (memberTypes, declaration) and not tokens.isEmpty()
                and document.getView (tokens.last()).compare (name) == 0)
                function (pathIndex, document, declaration);
        });
    }

    /**
     * @brief Returns the text of each name token of an element.
     *
     * @param document  The document that holds the element.
     * @param element   The element whose name is returned.
     * @return The name segments in order. The list is empty when the element has no name.
     */
    static jam::Array<std::string_view> getName (const jam::CodeDocument& document, const jam::Document::Element& element);

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
     * @brief Tells whether two paths hold the same segments.
     *
     * @param path       The first path.
     * @param otherPath  The second path.
     * @return @c true when both paths have the same count of segments, and each pair of segments has equal text.
     */
    static bool isSamePath (const jam::Array<std::string_view>& path, const jam::Array<std::string_view>& otherPath);

    /**
     * @brief Tells whether a list of paths holds a path.
     *
     * @param knownPaths  The paths to search.
     * @param path        The path to find.
     * @return @c true when a known path has the same segments as the path.
     */
    static bool hasPath (const jam::Array<jam::Array<std::string_view>>& knownPaths, const jam::Array<std::string_view>& path);

    /**
     * @brief Returns the index in paths of the file that a request names.
     *
     * The file argument resolves against the folder of the client. The request
     * stores that folder in its directory, because this process can run in a
     * different folder.
     *
     * @param request  The request row. Its directory and its file argument give the path.
     * @return The index of the path in paths, or -1 when the path is not stored.
     */
    int getPathIndex (const juce::ValueTree& request) const;

    /**
     * @brief Tells whether a path has a declaration other than a constructor or a destructor.
     *
     * A constructor or a destructor carries the name of its class. A lookup that
     * finds only those declarations reads the name as the class itself, so it
     * does not accept the name as a member. It searches the bases of the class.
     *
     * @param path  The path to test.
     * @return @c true when at least one declaration at the path is not a constructor or a destructor.
     */
    bool hasDeclarations (const jam::Array<std::string_view>& path) const;

    /**
     * @brief Tells whether a reference element refers to a declaration path.
     *
     * The reference must not be a declaration, and its last name token must equal
     * the last segment of the path. The declarations that the reference resolves
     * to must then include the path.
     *
     * @param referenceIndex   The index of the document that holds the reference.
     * @param reference        The element to test.
     * @param declarationPath  The path of the declaration.
     * @return @c true when the reference resolves to the declaration path.
     */
    bool isReferenceTo (int referenceIndex, const jam::Document::Element& reference, const jam::Array<std::string_view>& declarationPath) const;

    /**
     * @brief Returns the declaration paths that an element names.
     *
     * A declaration gives its own path. Any other element resolves its name in
     * the scope of its parent.
     *
     * @param document  The document that holds the element.
     * @param element   The element to resolve.
     * @return The declaration paths. The list is empty when no declaration matches.
     */
    jam::Array<jam::Array<std::string_view>> getDeclarationPaths (const jam::CodeDocument& document, const jam::Document::Element& element) const;

    /**
     * @brief Returns the paths of the member named @p name, searched from the full scope outward.
     *
     * The full scope is tried first. When no member is found, each shorter scope
     * is tried in turn, so an inner name hides an outer name.
     *
     * @param scope  The scope path. Its innermost segments come last.
     * @param name   The name segments to find.
     * @param tried  The class paths that the enclosing walks already visited. It stops a base cycle.
     * @return The member paths of the first scope that has a match, or an empty list.
     */
    jam::Array<jam::Array<std::string_view>> getDeclarationPaths (const jam::Array<std::string_view>& scope, const jam::Array<std::string_view>& name, const jam::Array<const jam::Array<std::string_view>*>& tried) const;

    /**
     * @brief Returns the paths of the member named @p name in a class, and in each of its bases.
     *
     * The guard @p tried stops the walk when a class is its own base, directly or
     * through other classes. The result is the union over every base. The documents
     * are held in a hash map with no order, so a first found base would be a guess.
     * Each path appears once in the result.
     *
     * @param classPath  The path of the class.
     * @param name       The member name segments.
     * @param tried      The class paths that the walk already visited.
     * @return The member paths, or an empty list when the class and its bases have no such member.
     */
    jam::Array<jam::Array<std::string_view>> getMemberPaths (const jam::Array<std::string_view>& classPath, const jam::Array<std::string_view>& name, const jam::Array<const jam::Array<std::string_view>*>& tried) const;

    /**
     * @brief Stores the results of a request as a JSON array, with no extra members.
     *
     * @param request  The request row that receives the JSON.
     * @param results  The pairs of a path index and an element.
     */
    void setResult (juce::ValueTree& request, jam::Array<std::pair<int, const jam::Document::Element*>> results) const;

    /**
     * @brief Stores a JSON text as the result of a request.
     *
     * The application reacts to the change of the result, prints the text, and
     * removes the request row.
     *
     * @param request  The request row that receives the result.
     * @param json     The JSON text. The property holds a copy of it.
     */
    void setResult (juce::ValueTree& request, std::string_view json) const;

    /**
     * @brief Tells whether one result comes before another in the result order.
     *
     * The path text gives the first order. In one document, the byte offsets of
     * the last name tokens give the order of the line, then the column.
     *
     * @param pathIndex       The path index of the first element.
     * @param element         The first element.
     * @param otherPathIndex  The path index of the second element.
     * @param otherElement    The second element.
     * @return @c true when the first element comes before the second.
     */
    bool isBefore (int pathIndex, const jam::Document::Element& element, int otherPathIndex, const jam::Document::Element& otherElement) const;

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
     */
    bool isBeforeByPath (int pathIndex, const jam::Document::Element& element, int otherPathIndex, const jam::Document::Element& otherElement) const;

    /**
     * @brief Appends the declarations below a parent as a tree of JSON elements.
     *
     * A declaration without a name has no position. Its declarations become
     * entries of its parent, so the walk appends them to the same JSON text.
     *
     * @param json       The JSON text that receives the entries.
     * @param pathIndex  The index of the document that holds the parent.
     * @param parent     The element whose declarations are appended.
     */
    void appendDocumentSymbols (std::string& json, int pathIndex, const jam::Document::Element& parent) const;

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
     */
    void appendCompletion (std::string& json, int pathIndex, const jam::CodeDocument& declarationDocument, const jam::Document::Element& declaration, const jam::CodeDocument& commentDocument, const jam::Document::Element& commentDeclaration) const;

    /**
     * @brief Fills events with the operation of each request command.
     *
     * The constructor calls this once. The table holds one entry for each command,
     * so runRequest finds an operation with one lookup.
     */
    void registerEvents();

    /**
     * @brief Runs the operation that the command of a request names.
     *
     * An empty command word maps to the empty identifier, which the table does
     * not hold. A juce::Identifier made from an empty string fails an assertion
     * (juce_Identifier.cpp), so the empty word is tested before the identifier is
     * made. An unknown command sets the overlay message and writes an empty result.
     *
     * @param request  The request row. Its command selects the operation, and the operation writes the result.
     */
    void runRequest (juce::ValueTree& request);

    /**
     * @brief Writes the declarations of the element at the position of a request.
     *
     * @param request  The request row. Its arguments give the file and the position.
     */
    void goToDefinition (juce::ValueTree& request) const;

    /**
     * @brief Writes the references to the declarations of the element at the position of a request.
     *
     * @param request  The request row. Its arguments give the file and the position.
     */
    void findReferences (juce::ValueTree& request) const;

    /**
     * @brief Writes the declarations at the position of a request, each with its scope.
     *
     * The scope is the path of the declaration without its last segment. Each
     * JSON element holds the scope as an array of names.
     *
     * @param request  The request row. Its arguments give the file and the position.
     */
    void hover (juce::ValueTree& request) const;

    /**
     * @brief Writes the symbols of the file in a request, as a tree.
     *
     * The request must hold one argument, the file. Each symbol holds its named
     * declarations as children. Unnamed declarations give their children to their
     * parent.
     *
     * @param request  The request row. Its single argument is the file.
     */
    void documentSymbol (juce::ValueTree& request) const;

    /**
     * @brief Writes the declarations whose last name matches the fuzzy query of a request.
     *
     * The request must hold one argument, the query. Other argument counts set the
     * overlay message.
     *
     * @param request  The request row. Its single argument is the query.
     */
    void workspaceSymbol (juce::ValueTree& request) const;

    /**
     * @brief Writes the completions of the name that ends before the position of a request, as a JSON array.
     *
     * The position is the cursor after the typed name, so the lookup reads the
     * byte before it. No stored document, no element, or an element of another
     * type gives an empty array. The lookup reads the stored documents and starts
     * no parse.
     *
     * @param request  The request row. Its arguments give the file, the line, and the column.
     */
    void completion (juce::ValueTree& request) const;

    /**
     * @brief Returns the declarations whose last name matches a fuzzy pattern, with their scores.
     *
     * Each declaration with a name is scored by the fuzzy finder, with the slab as
     * its working memory. The list is sorted by descending score, then by the
     * result order.
     *
     * @param pattern  The parsed query.
     * @return Tuples of the score, the path index, and the element.
     */
    Matches getMatches (const jam::Fuzzy::Pattern& pattern) const;

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
    jam::Fuzzy::Result getMatch (const jam::Fuzzy::Pattern& pattern, const jam::CodeDocument& document, const jam::Document::Element& element) const;

    /**
     * @brief Returns the declarations that a reference can complete to, with their scores, in completion order.
     *
     * Each candidate of applyToCandidates whose name matches the pattern is kept.
     * The list is sorted by descending score, then by path, then by file, line, and
     * column. Entries with the same path are adjacent, because they have the same
     * name, so they have the same score.
     *
     * @param document   The document that holds the reference.
     * @param reference  The reference element whose name is completed.
     * @param pattern    The parsed name of the reference.
     * @return Tuples of the score, the path index, and the element.
     */
    Matches getCompletions (const jam::CodeDocument& document, const jam::Document::Element& reference, const jam::Fuzzy::Pattern& pattern) const;

    /**
     * @brief Calls @p function once for each symbol that a reference can complete to.
     *
     * Entries with the same path are one symbol. The first entry of a symbol, in
     * the order of getCompletions, is the declaration. The comment is the Id::comment
     * of the first entry of the symbol that has one. When no entry has one, the
     * comment is absent.
     *
     * @tparam Function  A callable type that takes (int, const jam::CodeDocument&, const jam::Document::Element&, const jam::CodeDocument&, const jam::Document::Element&).
     * @param document   The document that holds the reference.
     * @param reference  The reference element whose name is completed.
     * @param pattern    The parsed name of the reference.
     * @param function   The callable. It receives the path index, the document, and the declaration, then the document and the element that holds the comment.
     */
    template <typename Function>
    void applyToCompletions (const jam::CodeDocument& document, const jam::Document::Element& reference, const jam::Fuzzy::Pattern& pattern, const Function& function) const
    {
        const auto completions (getCompletions (document, reference, pattern));

        for (int index { 0 }; index < completions.size();)
        {
            const auto& [score, pathIndex, declaration] { completions.at (index) };
            const auto declarationPath (getPath (*documents.at (pathIndex), *declaration));
            const auto runEnd { std::find_if (completions.begin() + index, completions.end(), [this, &declarationPath] (const auto& entry)
            {
                const auto& [otherScore, otherIndex, otherDeclaration] { entry };

                return not isSamePath (declarationPath, getPath (*documents.at (otherIndex), *otherDeclaration));
            }) };
            const auto commentEntry { std::find_if (completions.begin() + index, runEnd, [] (const auto& entry)
            {
                const auto& [otherScore, otherIndex, otherDeclaration] { entry };

                return otherDeclaration->contains (Id::comment);
            }) };
            const auto& [commentScore, commentIndex, commentDeclaration] { commentEntry == runEnd ? completions.at (index) : *commentEntry };

            function (pathIndex, *documents.at (pathIndex), *declaration, *documents.at (commentIndex), *commentDeclaration);

            index = static_cast<int> (runEnd - completions.begin());
        }
    }

    /**
     * @brief Writes the implementations of the declarations at the position of a request.
     *
     * @param request  The request row. Its arguments give the file and the position.
     */
    void goToImplementation (juce::ValueTree& request) const;

    /**
     * @brief Writes the function declarations at the position of a request, as the start of a call hierarchy.
     *
     * @param request  The request row. Its arguments give the file and the position.
     */
    void prepareCallHierarchy (juce::ValueTree& request) const;

    /**
     * @brief Writes the parents of the call elements that refer to the function at the position of a request.
     *
     * @param request  The request row. Its arguments give the file and the position.
     */
    void incomingCalls (juce::ValueTree& request) const;

    /**
     * @brief Writes the declarations that the calls inside the function at the position of a request refer to.
     *
     * The walk visits each element under the function declaration. Each call
     * element gives the declarations that its name resolves to.
     *
     * @param request  The request row. Its arguments give the file and the position.
     */
    void outgoingCalls (juce::ValueTree& request) const;

    /**
     * @brief Starts the parse jobs of an added files child, or runs an added request.
     *
     * A files child first erases each stored document whose path has no row in the
     * child. It then starts one parse job for each row. When no job is pending, it
     * writes the documents count. A request child runs at once. Any other child is
     * ignored.
     *
     * @param parentTree              The tree that received the child. Not read.
     * @param childWhichHasBeenAdded  The added child. A files child starts the parse jobs, and a request child runs.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Starts a parse job for a file row whose path changes.
     *
     * The model sends a path change for a row when the watcher reports that its
     * file changed. Only the path of a file row starts a job.
     *
     * @param tree      The tree whose property changed. A file row starts a job.
     * @param property  The changed property. Only Id::path starts a job.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Starts a parse job for one file row, on the thread pool.
     *
     * The job gets the path and the family as values, read on the calling
     * thread. It reads the file, parses it, and validates it. Then it returns its
     * result to the message thread through juce::MessageManager::callAsync.
     * The pending count rises here, and addDocument lowers it.
     *
     * @param row       The file row. Its path and family are copied for the job.
     * @param rowIndex  The index of the row in the files child. addDocument checks the result against this row.
     */
    void addJob (const juce::ValueTree& row, int rowIndex);

    /**
     * @brief Ends a parse job on the message thread, and keeps its document when the row still matches.
     *
     * The result is kept only when the row at rowIndex still has the path, and the
     * file still has the modification time that the job read. A result that does
     * not match is dropped with no message. For a match, a document replaces the
     * stored document of the path, and no document erases it. A failed validation
     * sets the overlay message. When the last pending job ends, the count of
     * documents is written.
     *
     * @param rowIndex          The index of the row that started the job.
     * @param path              The path that the job read.
     * @param modificationTime  The modification time that the job read.
     * @param document          The parsed document, or nullptr when the file is missing or its family has no parser.
     * @param result            The validation result. An ok result when there is no document.
     */
    void addDocument (int rowIndex, const juce::String& path, juce::Time modificationTime, std::unique_ptr<jam::CodeDocument> document, const juce::Result& result);

    /**
     * @brief Returns the index of a path in paths, and adds the path when it is absent.
     *
     * The index is the key of the path's document. Nothing removes an entry from
     * paths, so an index stays the same for the life of the processor.
     *
     * @param path  The path text.
     * @return The index of the path in paths.
     */
    int getOrCreatePathIndex (const juce::String& path);

    /**
     * @brief Writes the number of stored documents to Id::documents.
     *
     * The callers call this only when no job is pending. When the count equals the
     * stored value, the function sends the change message by hand, so that the
     * listeners of Id::documents still react. Otherwise it sets the property.
     */
    void setDocuments();

    /** @brief The model that this processor listens to. The application owns the model. */
    LINGModel& model;

    /** @brief The path texts. The index of a path in paths is the key of its document. */
    jam::Strings paths;

    /** @brief The parsed documents, keyed by the index of their path in paths. */
    jam::HashMap<int, std::unique_ptr<jam::CodeDocument>> documents;

    /** @brief The parse jobs that started and have not yet ended on the message thread. */
    int pendingJobs { 0 };

    /** @brief The thread pool that runs the parse jobs. */
    juce::ThreadPool pool;

    /** @brief The operations that answer the request rows, keyed by the command identifier. */
    Events events;

    /**
     * @brief The working memory of the fuzzy finder.
     *
     * The finder takes one slab per thread. Each entry of events runs on the
     * message thread, so one slab serves every entry. The lookups that score names
     * are const, so the slab is mutable.
     */
    mutable jam::Fuzzy::Slab slab;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};
