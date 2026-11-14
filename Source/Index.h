#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"

/**
 * @class Index
 * @brief Holds the parsed documents of the source files, the symbols that index them, and the library of each family.
 *
 * The paths give each source file an index, and that index keys its document.
 * A document stays until removeDocuments or a null addDocument erases it. A newer
 * library replaces the library of its family. The declarations and references
 * maps are the symbol tables of the documents, and setDocuments rebuilds them.
 * The processor writes the index. Query reads it through a const reference.
 */
class Index
{
public:
    /**
     * @brief The symbol records of one index pass, keyed by the raw path, the line, and the column.
     *
     * Each record holds the symbol text and the role bits of one declaration or
     * reference. The raw path is relative to the folder of the unit row.
     */
    using Bindings = jam::HashMap<std::string, jam::HashMap<int, jam::HashMap<int, std::pair<std::string, int>>>>;

    /**
     * @brief Creates an empty index that reads the files child and the swap files of a model.
     *
     * @param modelToUse  The model that the index reads. The index does not own it.
     */
    explicit Index (LINGModel& modelToUse);

    /**
     * @brief Calls @p function for each stored document, in ascending order of the path bytes.
     *
     * The documents are held in a HashMap, which has no order, so the path
     * indices are sorted first. The view that @p function receives points into
     * paths, and it is valid during the call.
     *
     * @tparam Function  A callable type that takes (std::string_view, const jam::LanguageDocument&).
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
     * @brief Calls @p function for each element of each stored document, at every depth.
     *
     * @tparam Function  A callable type that takes (int, const jam::LanguageDocument&, const jam::Document::Element&).
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
     * @brief Erases each stored document whose path has no row in the files child.
     *
     * The call also clears declarations and references, because they point into the documents.
     *
     * @param files  The files child whose rows keep their documents.
     */
    void removeDocuments (const juce::ValueTree& files);

    /**
     * @brief Adds the symbols of the pending bindings, rebuilds the symbol maps, and writes the documents count.
     *
     * The processor calls this only when no parse job, index job, build, or precompile runs.
     * The swap files of the documents that gained symbols are written first. When the count
     * equals the stored value, the function sends the change message by hand, so that the
     * listeners of Id::documents still react. Otherwise it sets the property.
     */
    void setDocuments();

    /**
     * @brief Stores the document of a path, or erases its entry when the document is null.
     *
     * The call clears declarations and references, because they point into the documents.
     *
     * @param path      The path text. Its index in paths keys the document.
     * @param document  The document to store, or nullptr to erase the entry.
     */
    void addDocument (const juce::String& path, std::unique_ptr<jam::LanguageDocument> document);

    /**
     * @brief Stores the symbol records of an index pass for a files row, until addSymbols binds them.
     *
     * A new call for a row replaces the records of that row.
     *
     * @param rowIndex  The index of the files row that the records belong to.
     * @param records   The records of the pass. The index takes them by value.
     */
    void addBindings (int rowIndex, Bindings records);

    /**
     * @brief Reads the document of a swap file, when the file matches the source and the library.
     *
     * The root of the stored document must hold the source hash and the language
     * hash. The document is read against the source text.
     *
     * @param swapFile      The swap file to read. A file that cannot be mapped gives nullptr.
     * @param sourceText    The source text that the document refers to.
     * @param language      The library that the document is read with.
     * @param hash          The hash of the source text.
     * @param languageHash  The hash of the library of the language row.
     * @return The stored document, or nullptr when the file does not match.
     */
    static std::unique_ptr<jam::LanguageDocument> readSwapFile (const juce::File& swapFile, std::string_view sourceText, const std::shared_ptr<const jam::LanguageLibrary>& language, juce::int64 hash, juce::int64 languageHash);

    /**
     * @brief Writes a document to a swap file, with its version, its source hash, and its language hash.
     *
     * The file is written to a temporary file, which then replaces the swap file.
     * A null document writes nothing.
     *
     * @param swapFile      The swap file to write.
     * @param document      The document to write, or nullptr.
     * @param hash          The hash of the source text.
     * @param languageHash  The hash of the library of the language row.
     */
    static void writeSwapFile (const juce::File& swapFile, const std::unique_ptr<jam::LanguageDocument>& document, juce::int64 hash, juce::int64 languageHash);

    /** @brief The model. The index reads its files child, and it asks the model for the swap file of each path. */
    LINGModel& model;

    /** @brief The path texts. The index of a path in paths is the key of its document. */
    jam::Strings paths;

    /** @brief The parsed documents, keyed by the index of their path in paths. */
    jam::HashMap<int, std::unique_ptr<jam::LanguageDocument>> documents;

    /** @brief The loaded library of each family, keyed by family. A family without an entry gets no parse job. */
    jam::HashMap<int, std::shared_ptr<const jam::LanguageLibrary>> languages;

    /**
     * @brief The declarations of each symbol, as pairs of a path index and an element.
     *
     * The elements belong to documents. setDocuments rebuilds this map, and
     * addDocument and removeDocuments clear it.
     */
    jam::HashMap<juce::String, jam::Array<std::pair<int, const jam::Document::Element*>>> declarations;

    /**
     * @brief The references to each symbol, as pairs of a path index and an element.
     *
     * The elements belong to documents. An element whose role lacks the declaration
     * bit is a reference. setDocuments rebuilds this map, and addDocument and
     * removeDocuments clear it.
     */
    jam::HashMap<juce::String, jam::Array<std::pair<int, const jam::Document::Element*>>> references;

private:
    /**
     * @brief The elements of one document, keyed by the line and then the column of the last token of their name.
     *
     * A position holds the first element that is stored at it.
     */
    using Positions = jam::HashMap<int, jam::HashMap<int, jam::Document::Element*>>;

    /**
     * @brief Returns the position table of a document, and builds the table on the first call for its path.
     *
     * Each element with a name is stored at the line and the column of the last token of its name.
     * The tables belong to one index pass, so each document is walked once in the pass.
     *
     * @param positionTables  The position tables of the pass, keyed by path index.
     * @param document        The document whose elements are stored.
     * @param pathIndex       The index of the path of the document in paths.
     * @return The position table of the document.
     */
    static const Positions& getPositions (jam::HashMap<int, Positions>& positionTables, jam::LanguageDocument& document, int pathIndex);

    template <typename ValueType>
    static void addOrReplaceProperty (jam::Document::Element& element, const juce::Identifier& key, const ValueType& value)
    {
        if (auto* stored { element.find<ValueType> (key) }; stored != nullptr)
            *stored = value;
        else
            element.add<ValueType> (key, value);
    }

    /**
     * @brief Binds the pending records to the elements of the documents, and returns the paths that gained a binding.
     *
     * The rows of the files child are visited in order, and the records of each row
     * are bound to the elements at their positions. An element takes the first record
     * that binds it in the pass, which the pass tracks in boundElements. The function
     * clears the pending bindings when it ends.
     *
     * @return The path indices whose documents gained a bound element, each listed once.
     */
    jam::Array<int> addSymbols();

    /**
     * @brief Writes the swap file of each document of the given paths.
     *
     * @param pathIndices  The path indices whose documents are written.
     */
    void writeSwapFiles (const jam::Array<int>& pathIndices);

    /**
     * @brief Returns the index of a path in paths, and adds the path when it is absent.
     *
     * The index is the key of the path's document. Nothing removes an entry from
     * paths, so an index stays the same for the life of the Index.
     *
     * @param path  The path text.
     * @return The index of the path in paths.
     */
    int getOrCreatePathIndex (const juce::String& path);

    jam::HashMap<int, Bindings> bindings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Index)
};
