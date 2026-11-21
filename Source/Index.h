#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "SymbolDocument.h"
#include "Language.h"
#include "SwapValidator.h"

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
    template<typename Function>
    void applyToDocuments (const Function& function) const
    {
        for (const auto pathIndex : pathOrder)
            if (const auto stored { documents.find (pathIndex) }; stored != documents.end())
            {
                const auto& [documentIndex, document] { *stored };

                function (paths.getView (pathIndex), *document, *this);
            }
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
    void addDocument (const juce::String& path, std::unique_ptr<jam::Document> document);

    void addBindings (int rowIndex, std::unique_ptr<SymbolDocument> symbolDocument);

    void addPathRows (const juce::ValueTree& files);

    void addUnitRows (const juce::ValueTree& files);

    static std::pair<const jam::Document::Element*, int>
    getElement (const jam::Document& document, int line, int column);

    std::pair<const jam::Document::Element*, int> getElement (const std::pair<int, const SymbolDocument::SymbolRecord*>& occurrence) const;

    static bool isRecordBefore (const SymbolDocument::SymbolRecord& record, const SymbolDocument::SymbolRecord& otherRecord);

    bool isOccurrenceBefore (int leftPathIndex, const SymbolDocument::SymbolRecord& left, int rightPathIndex, const SymbolDocument::SymbolRecord& right) const;

    juce::Range<int> getPathRange (const jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>& occurrences, int pathIndex) const;

    const SymbolDocument& getSymbolDocument (int pathIndex) const
    {
        return *symbolDocuments.at (symbolFiles.at (pathIndex).first);
    }

    static int getCharacter (const jam::Document& document,
                             uint32_t offset,
                             std::string_view encoding);

    static int getByteColumn (const jam::Document& document,
                              int line,
                              int column,
                              std::string_view encoding);

    bool hasRole (const jam::Document& document, const jam::Document::Element& element, int role) const;
    int getSymbolType (const jam::Document& document, const jam::Document::Element& element) const;

    template<typename Function>
    void applyToRecords (int pathIndex,
                         const jam::Document& document,
                         const jam::Document::Element& element,
                         int tokenIndex,
                         const Function& function) const
    {
        if (const auto stored { records.find (pathIndex) }; stored != records.end())
        {
            const auto& [recordPathIndex, sortedRecords] { *stored };
            const auto& token { element.get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex) };
            const auto line { static_cast<int> (document.getLineNumber (token.offset())) };
            const auto column { static_cast<int> (document.getColumnNumber (token.offset())) };
            const auto endColumn { column + static_cast<int> (document.getView (token).size()) };
            const auto recordPosition { [] (const SymbolDocument::SymbolRecord* record)
            {
                return std::make_pair (record->line, record->column);
            } };
            const auto firstRecord { std::lower_bound (sortedRecords.begin(), sortedRecords.end(), std::make_pair (line, column),
                [&recordPosition] (const SymbolDocument::SymbolRecord* record, const std::pair<int, int>& position)
                {
                    return recordPosition (record) < position;
                }) };

            for (auto record { firstRecord }; record != sortedRecords.end() and recordPosition (*record) < std::make_pair (line, endColumn); ++record)
                function (**record);
        }
    }

    template<typename Function>
    void applyToOccurrences (const jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>& occurrences,
                             juce::Range<int> range,
                             const Function& function) const
    {
        for (auto occurrence { occurrences.begin() + range.getStart() }; occurrence != occurrences.begin() + range.getEnd(); ++occurrence)
        {
            const auto& [pathIndex, record] { *occurrence };

            if (const auto [element, tokenIndex] { getElement (*occurrence) }; element != nullptr)
                function (pathIndex, *documents.at (pathIndex), *element, tokenIndex, *record);
        }
    }

    template<typename Function>
    void applyToOccurrences (const jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>& occurrences,
                             const Function& function) const
    {
        applyToOccurrences (occurrences, juce::Range<int> { 0, occurrences.size() }, function);
    }

    template<typename Function>
    void applyToOccurrences (const jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>& occurrences,
                             int pathIndex,
                             const Function& function) const
    {
        applyToOccurrences (occurrences, getPathRange (occurrences, pathIndex), function);
    }

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
    static std::unique_ptr<jam::Document>
    readSwapFile (const juce::File& swapFile,
                  std::string_view sourceText,
                  const std::shared_ptr<const Language>& language,
                  juce::int64 hash,
                  juce::int64 languageHash,
                  int family);

    static std::unique_ptr<SymbolDocument>
    readSwapFile (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family);

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
    static void writeSwapFile (const juce::File& swapFile,
                               jam::Document& document,
                               juce::int64 hash,
                               juce::int64 languageHash,
                               int family);

    static void addStamp (jam::Document& document, juce::int64 hash, juce::int64 languageHash, int family);

    /** @brief The model. The index reads its files child, and it asks the model for the swap file of each path. */
    LINGModel& model;

    /** @brief The path texts. The index of a path in paths is the key of its document. */
    jam::Strings paths;

    jam::Array<int> pathOrder;

    /** @brief The parsed documents, keyed by the index of their path in paths. */
    jam::HashMap<int, std::unique_ptr<jam::Document>> documents;

    /** @brief The loaded library of each family, keyed by family. A family without an entry gets no parse job. */
    jam::HashMap<int, std::shared_ptr<const Language>> languages;

    /**
     * @brief The declarations of each symbol, as pairs of a path index and an element.
     *
     * The elements belong to documents. setDocuments rebuilds this map, and
     * addDocument and removeDocuments clear it.
     */
    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>> declarations;

    /**
     * @brief The references to each symbol, as pairs of a path index and an element.
     *
     * The elements belong to documents. An element whose role lacks the declaration
     * bit is a reference. setDocuments rebuilds this map, and addDocument and
     * removeDocuments clear it.
     */
    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>> references;

    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::CompletionRecord*>>> completions;

    jam::HashMap<int, std::unique_ptr<SymbolDocument>> symbolDocuments;

    jam::HashMap<int, std::pair<int, const SymbolDocument::FileEntry*>> symbolFiles;

    jam::HashMap<int, jam::Array<const SymbolDocument::SymbolRecord*>> records;

    jam::HashMap<juce::String, int> unitRows;

    jam::HashMap<juce::String, int> pathRows;

    jam::HashMap<juce::String, jam::Array<int>> stemRows;

private:
    void removeBindings (int rowIndex);

    void removeSymbols (int pathIndex);

    void removeOccurrences (jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& symbols,
                            juce::int64 symbol,
                            int pathIndex);

    void removeCompletions (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file);

    void addCompletions (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file);

    void addRecords (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file);

    void addSymbols (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file);

    void addOccurrences (jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& symbols,
                         int pathIndex,
                         const jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& blocks);

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

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Index)
};
