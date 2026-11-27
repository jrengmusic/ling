#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "SymbolDocument.h"
#include "Language.h"
#include "SwapValidator.h"

/**
 * @class Index
 * @brief Holds cached source documents, SymbolDocument bindings, symbol tables, and loaded languages.
 *
 * Each source path gets a stable index in paths. Source hashes stay in
 * sourceHashes while getOrCreateDocument() loads and caches documents on demand
 * up to maxCachedDocuments. DocumentScope pins cached documents during queries.
 * addBindings() and removeDocuments() maintain symbolDocuments, symbolFiles,
 * completions, declarations, and references. Processor writes the index, and
 * Query reads it through a const reference.
 */
class Index
{
public:
    /**
     * @struct UnitOwners
     * @brief Maps file paths to owning translation unit row indices and holds project roots.
     */
    struct UnitOwners
    {
        /** @brief Sentinel row index returned when a path has no owning unit. */
        static constexpr int noRow { -1 };

        /**
         * @brief Records @p unitRow as the owner of @p path when @p path is absent.
         *
         * @param path     File path to register.
         * @param unitRow  Owning unit row index.
         */
        void addIfAbsent (const juce::String& path, int unitRow);
        /**
         * @brief Returns the owning unit row index for @p path, or @c noRow when absent.
         *
         * @param path  File path to look up.
         * @return Owning unit row index, or @c noRow.
         */
        int getRow (juce::StringRef path) const;
        /**
         * @brief Tells whether @p recordPath belongs to another unit or lies outside @c roots.
         *
         * @param recordPath  File path of the symbol record.
         * @param unitRow     Unit row index performing the check.
         * @return @c true when another unit owns @p recordPath or when unowned outside @c roots.
         */
        bool isForeign (const char* recordPath, int unitRow) const;

        /** @brief Registered source and dependency file paths parallel to @c rows. */
        jam::Strings paths;
        /** @brief Owning unit row indices parallel to @c paths. */
        jam::Array<int> rows;
        /** @brief Project root directories limiting unowned symbol records in isForeign(). */
        jam::Array<juce::File> roots;
    };

    /**
     * @struct DocumentScope
     * @brief RAII guard that suspends LRU document cache eviction while queries read @c Index.
     */
    struct DocumentScope
    {
        /**
         * @brief Increments the eviction lock depth on @p indexToUse.
         *
         * @param indexToUse  Index whose cached documents stay pinned during this scope.
         */
        explicit DocumentScope (const Index& indexToUse);
        /** @brief Decrements the eviction lock depth and evicts excess cached documents when zero. */
        ~DocumentScope();
        /** @brief Index whose document eviction lock depth this guard manages. */
        const Index& index;
        JUCE_DECLARE_NON_COPYABLE (DocumentScope)
    };

    /**
     * @brief Creates an empty index bound to @p modelToUse.
     *
     * @param modelToUse  The model whose files child and swap paths the index reads.
     */
    explicit Index (LINGModel& modelToUse);

    /**
     * @brief Calls @p function for each stored document in ascending path byte order.
     *
     * Iterates over pathOrder, which tracks path indices sorted by path bytes.
     * The string_view passed to @p function points into paths and remains valid
     * during the call.
     *
     * @tparam Function  Callable taking (std::string_view, const jam::Document&, const Index&).
     * @param function   Callable invoked for each stored document.
     */
    template<typename Function>
    void applyToDocuments (const Function& function) const
    {
        for (const auto pathIndex : pathOrder)
            if (sourceHashes.contains (pathIndex))
                if (const auto* document { getOrCreateDocument (pathIndex) }; document != nullptr)
                    function (paths.getView (pathIndex), *document, *this);
    }

    /**
     * @brief Erases documents absent from @p files and unbinds SymbolDocuments not retained by isUnitRetained().
     *
     * Drops removed paths from sourceHashes, documents, and cachedDocuments.
     * Unbinds non-retained units via removeBindings() and erases them from symbolDocuments.
     *
     * @param files  The current files child tree to retain entries against.
     */
    void removeDocuments (const juce::ValueTree& files);

    /**
     * @brief Writes the current documents count to Id::documents on the model state.
     *
     * Processor calls this once all parse, index, and unit jobs finish. When
     * the count already equals the stored property, sends a property change
     * message so listeners still react.
     */
    void setDocuments();

    /**
     * @brief Stores or replaces the parsed document and @p sourceHash for @p path, or erases both when null.
     *
     * @param path        Source file path whose index in paths keys documents.
     * @param document    Parsed document to store, or nullptr to erase the entry.
     * @param sourceHash  Wyhash digest of the source bytes, or zero when erasing.
     */
    void addDocument (const juce::String& path, std::unique_ptr<jam::Document> document, juce::int64 sourceHash);

    /**
     * @brief Records @p sourceHash for @p path and drops any cached document so it reloads from swap on demand.
     *
     * @param path        Source file path.
     * @param sourceHash  Wyhash digest of the source bytes.
     */
    void addSourceHash (const juce::String& path, juce::int64 sourceHash);

    /**
     * @brief Returns the parsed document for @p pathIndex, loading and caching it from swap when absent.
     *
     * Moves cache hits to the back of @c cachedDocuments and evicts entries above
     * @c maxCachedDocuments when @c evictionLockDepth is zero.
     *
     * @param pathIndex  Stable path index in @c paths.
     * @return Cached document pointer, or @c nullptr when missing or invalid.
     */
    const jam::Document* getOrCreateDocument (int pathIndex) const;

    /**
     * @brief Replaces the symbol bindings of unit @p rowIndex with @p symbolDocument.
     *
     * Clears previous bindings for @p rowIndex, then registers each owned or
     * unclaimed file entry into symbolFiles, completions, declarations, and
     * references.
     *
     * @param rowIndex        Translation unit row index in the files child.
     * @param symbolDocument  Validated SymbolDocument produced for the unit.
     */
    void addBindings (int rowIndex, std::unique_ptr<SymbolDocument> symbolDocument);

    /**
     * @brief Erases the SymbolDocument of @p rowIndex, moves @p temporary over its target, and binds the target as the SymbolDocument of @p rowIndex.
     *
     * The bindings of @p rowIndex are removed first. The target is read with readSwapFile(), and
     * a valid result is bound through addBindings(int, std::unique_ptr<SymbolDocument>).
     *
     * @param rowIndex      Translation unit row index in the files child.
     * @param temporary     The temporary file that holds the serialized SymbolDocument. Its target file is read.
     * @param hash          Expected content hash of the unit inputs.
     * @param languageHash  Expected hash of the language library.
     * @param family        Expected language family identifier.
     * @return ok when the target reads as a valid SymbolDocument and is bound. Otherwise the text is
     *         text::Diagnostics::failSwapWrite and the full path of the target file.
     */
    juce::Result addBindings (int rowIndex, const juce::TemporaryFile& temporary, juce::int64 hash, juce::int64 languageHash, int family);

    /**
     * @brief Rebuilds pathRows and stemRows from the rows of @p files.
     *
     * @param files  The files child tree.
     */
    void addPathRows (const juce::ValueTree& files);

    /**
     * @brief Rebuilds @c unitOwners mapping each unit and its dependencies to an owning unit row index.
     *
     * Populates project roots and explicit unit rows first. Registers local
     * dependencies inside each unit's folder next, then runs three fallback
     * passes: standalone units, prerequisite units, and units with prerequisites.
     *
     * @param files  The files child tree.
     */
    void addUnitRows (const juce::ValueTree& files);

    /**
     * @brief Returns the element and name-token index covering 1-based (@p line, @p column) in @p document.
     *
     * @param document  Parsed document to query.
     * @param line      1-based line number.
     * @param column    1-based byte column.
     * @return Matching (Element*, tokenIndex), or { nullptr, -1 } when out of range or unmatched.
     */
    static std::pair<const jam::Document::Element*, int>
    getElement (const jam::Document& document, int line, int column);

    /**
     * @brief Resolves a symbol occurrence pair to its element and name-token index via getOrCreateDocument().
     *
     * @param occurrence  Pair of (pathIndex, SymbolRecord*) from declarations or references.
     * @return Matching (Element*, tokenIndex), or { nullptr, -1 } when the document is absent.
     */
    std::pair<const jam::Document::Element*, int> getElement (const std::pair<int, const SymbolDocument::SymbolRecord*>& occurrence) const;

    /**
     * @brief Orders two symbol records by 1-based (line, column).
     *
     * @param record       Left symbol record.
     * @param otherRecord  Right symbol record.
     * @return True when @p record precedes @p otherRecord.
     */
    static bool isRecordBefore (const SymbolDocument::SymbolRecord& record, const SymbolDocument::SymbolRecord& otherRecord);

    /**
     * @brief Orders two occurrences by ascending path byte order, then by (line, column).
     *
     * @param leftPathIndex   Path index of @p left.
     * @param left            Left symbol record.
     * @param rightPathIndex  Path index of @p right.
     * @param right           Right symbol record.
     * @return True when the left occurrence precedes the right occurrence.
     */
    bool isOccurrenceBefore (int leftPathIndex, const SymbolDocument::SymbolRecord& left, int rightPathIndex, const SymbolDocument::SymbolRecord& right) const;

    /**
     * @brief Returns the index range in path-sorted @p occurrences belonging to @p pathIndex.
     *
     * @param occurrences  Occurrences array sorted by path byte order.
     * @param pathIndex    Target path index in paths.
     * @return Contiguous [start, end) range for @p pathIndex.
     */
    template<typename Record>
    juce::Range<int> getPathRange (const jam::Array<std::pair<int, const Record*>>& occurrences, int pathIndex) const
    {
        const auto isPathBefore { [this] (const std::pair<int, const Record*>& left, const std::pair<int, const Record*>& right)
        {
            const auto& [leftPathIndex, leftRecord] { left };
            const auto& [rightPathIndex, rightRecord] { right };

            return paths.getView (leftPathIndex).compare (paths.getView (rightPathIndex)) < 0;
        } };
        const std::pair<int, const Record*> probe { pathIndex, nullptr };
        const auto [first, last] { std::equal_range (occurrences.begin(), occurrences.end(), probe, isPathBefore) };

        return { static_cast<int> (std::distance (occurrences.begin(), first)), static_cast<int> (std::distance (occurrences.begin(), last)) };
    }

    /**
     * @brief Returns the owning SymbolDocument for the file at @p pathIndex.
     *
     * @param pathIndex  Path index present in symbolFiles.
     * @return Reference to the SymbolDocument owning that file entry.
     */
    const SymbolDocument& getSymbolDocument (int pathIndex) const;

    /**
     * @brief Converts byte @p offset in @p document to a 0-based character column under @p encoding.
     *
     * @param document  Document owning the source bytes.
     * @param offset    Byte offset in @p document.
     * @param encoding  LSP position encoding ("utf-8" or UTF-16).
     * @return 0-based column count on the offset's line.
     */
    static int getCharacter (const jam::Document& document,
                             size_t offset,
                             std::string_view encoding);

    /**
     * @brief Converts 1-based (@p line, @p column) under @p encoding to a 1-based byte column.
     *
     * @param document  Document owning the source bytes.
     * @param line      1-based line number.
     * @param column    1-based LSP column in @p encoding units.
     * @param encoding  LSP position encoding ("utf-8" or UTF-16).
     * @return 1-based byte column on @p line.
     */
    static int getByteColumn (const jam::Document& document,
                              int line,
                              int column,
                              std::string_view encoding);

    /**
     * @brief Tells whether @p element carries @p role in its document's language.
     *
     * @param document  Document owning @p element.
     * @param element   Element to inspect.
     * @param role      jam::LanguageRole bitmask to test.
     * @return True when the language assigns @p role to the element's token type.
     */
    bool hasRole (const jam::Document& document, const jam::Document::Element& element, int role) const;

    /**
     * @brief Returns the LSP symbol kind integer for @p element in its document's language.
     *
     * @param document  Document owning @p element.
     * @param element   Element to inspect.
     * @return LSP symbol kind integer from the family's Language.
     */
    int getSymbolType (const jam::Document& document, const jam::Document::Element& element) const;

    /**
     * @brief Calls @p function for every SymbolRecord overlapping the @p tokenIndex name token of @p element.
     *
     * Binary-searches the path's sorted records by (line, column) over the token's
     * byte-column span.
     *
     * @tparam Function   Callable taking (const SymbolDocument::SymbolRecord&).
     * @param pathIndex   Path index in paths.
     * @param document    Parsed document owning @p element.
     * @param element     Element holding Id::nameTokens.
     * @param tokenIndex  Index into @p element's Id::nameTokens array.
     * @param function    Callable invoked for each matching record.
     */
    template<typename Function>
    void applyToRecords (int pathIndex,
                         const jam::Document& document,
                         const jam::Document::Element& element,
                         int tokenIndex,
                         const Function& function) const
    {
        if (const auto stored { symbolFiles.find (pathIndex) }; stored != symbolFiles.end())
        {
            const auto& [storedPathIndex, unitFile] { *stored };
            const auto& [unitPathIndex, file] { unitFile };
            const auto fileSymbols { symbolDocuments.at (unitPathIndex)->getSymbols (*file) };
            const auto& token { element.get<jam::Document::Tokens> (Id::nameTokens)->at (tokenIndex) };
            const auto line { static_cast<int> (document.getLineNumber (token.offset())) };
            const auto column { static_cast<int> (document.getColumnNumber (token.offset())) };
            const auto endColumn { column + static_cast<int> (document.getView (token).size()) };
            const auto recordPosition { [] (const SymbolDocument::SymbolRecord& record)
            {
                return std::make_pair (record.line, record.column);
            } };
            const auto firstRecord { std::lower_bound (fileSymbols.begin(), fileSymbols.end(), std::make_pair (line, column),
                [&recordPosition] (const SymbolDocument::SymbolRecord& record, const std::pair<int, int>& position)
                {
                    return recordPosition (record) < position;
                }) };

            for (auto record { firstRecord }; record != fileSymbols.end() and recordPosition (*record) < std::make_pair (line, endColumn); ++record)
                function (*record);
        }
    }

    /**
     * @brief Resolves each occurrence in @p range of @p occurrences and calls @p function.
     *
     * @tparam Function    Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex, const SymbolDocument::SymbolRecord&).
     * @param occurrences  Occurrences array.
     * @param range        Index subrange within @p occurrences.
     * @param function     Callable invoked for each occurrence with a valid document element.
     */
    template<typename Function>
    void applyToOccurrences (const jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>& occurrences,
                             juce::Range<int> range,
                             const Function& function) const
    {
        for (auto occurrence { occurrences.begin() + range.getStart() }; occurrence != occurrences.begin() + range.getEnd(); ++occurrence)
        {
            const auto& [pathIndex, record] { *occurrence };

            if (const auto* document { getOrCreateDocument (pathIndex) }; document != nullptr)
                if (const auto [element, tokenIndex] { getElement (*document, record->line, record->column) }; element != nullptr)
                    function (pathIndex, *document, *element, tokenIndex, *record);
        }
    }

    /**
     * @brief Resolves every occurrence in @p occurrences and calls @p function.
     *
     * @tparam Function    Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex, const SymbolDocument::SymbolRecord&).
     * @param occurrences  Occurrences array.
     * @param function     Callable invoked for each resolved occurrence.
     */
    template<typename Function>
    void applyToOccurrences (const jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>& occurrences,
                             const Function& function) const
    {
        applyToOccurrences (occurrences, juce::Range<int> { 0, occurrences.size() }, function);
    }

    /**
     * @brief Resolves every occurrence for @p pathIndex in @p occurrences and calls @p function.
     *
     * @tparam Function    Callable taking (int pathIndex, const jam::Document&, const jam::Document::Element&, int tokenIndex, const SymbolDocument::SymbolRecord&).
     * @param occurrences  Occurrences array sorted by path byte order.
     * @param pathIndex    Path index to filter by.
     * @param function     Callable invoked for each resolved occurrence in that file.
     */
    template<typename Function>
    void applyToOccurrences (const jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>& occurrences,
                             int pathIndex,
                             const Function& function) const
    {
        applyToOccurrences (occurrences, getPathRange (occurrences, pathIndex), function);
    }

    /**
     * @brief Reads a serialized document swap file when its source hash, language hash, and family match.
     *
     * Maps @p swapFile read-only, parses it against @p sourceText, and validates it
     * with SwapValidator before checking the stamp fields.
     *
     * @param swapFile      Swap file to map.
     * @param sourceText    Source text referenced by the stored tokens.
     * @param language      Language used to deserialize the document.
     * @param hash          Expected source content hash.
     * @param languageHash  Expected language library hash.
     * @param family        Expected language family identifier.
     * @return Validated document, or nullptr when missing or mismatched.
     */
    static std::unique_ptr<jam::Document>
    readSwapFile (const juce::File& swapFile,
                  std::string_view sourceText,
                  const std::shared_ptr<const Language>& language,
                  juce::int64 hash,
                  juce::int64 languageHash,
                  int family);

    /**
     * @brief Opens and validates a binary SymbolDocument swap file.
     *
     * @param swapFile      Symbol swap file to map.
     * @param hash          Expected translation unit content hash.
     * @param languageHash  Expected language library hash.
     * @param family        Expected language family identifier.
     * @return Validated SymbolDocument, or nullptr when missing or mismatched.
     */
    static std::unique_ptr<SymbolDocument>
    readSwapFile (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family);

    /**
     * @brief Stamps @p document, writes it to a temporary file, and overwrites @p swapFile with that file.
     *
     * @param swapFile      Destination swap file path.
     * @param document      Document to stamp and serialize.
     * @param hash          Source content hash.
     * @param languageHash  Language library hash.
     * @param family        Language family identifier.
     * @return ok when the write and the overwrite succeed. When the stream fails, the text is
     *         text::Diagnostics::failSwapWrite, the full path of swapFile, and the system error text.
     *         When the overwrite fails, the text is text::Diagnostics::failSwapWrite and the full path of swapFile.
     */
    static juce::Result writeSwapFile (const juce::File& swapFile,
                                       jam::Document& document,
                                       juce::int64 hash,
                                       juce::int64 languageHash,
                                       int family);

    /**
     * @brief Attaches source hash, language hash, family, and version stamps to @p document's root.
     *
     * @param document      Document whose root receives the validation properties.
     * @param hash          Source content hash.
     * @param languageHash  Language library hash.
     * @param family        Language family identifier.
     */
    static void addStamp (jam::Document& document, juce::int64 hash, juce::int64 languageHash, int family);

    /** @brief The model. The index reads its files child and requests swap paths from it. */
    LINGModel& model;

    /** @brief Interned source paths. A path's index in paths is its stable pathIndex key. */
    jam::Strings paths;

    /** @brief Indices into paths maintained in ascending path byte order. */
    jam::Array<int> pathOrder;

    /** @brief Wyhash source content digests keyed by @c pathIndex for indexed files. */
    jam::HashMap<int, juce::int64> sourceHashes;

    /** @brief Loaded Language instances keyed by family identifier. */
    jam::HashMap<int, std::shared_ptr<const Language>> languages;

    /** @brief Holds a shared reference to each language that Processor::setLanguage loads. Each successful load adds an entry, so the library of a replaced language stays loaded for the threads that ran it. */
    jam::Array<std::shared_ptr<const Language>> loadedLanguages;

    /**
     * @brief Declaration occurrences per symbol identifier, sorted by path byte order.
     *
     * Each entry pairs pathIndex with a SymbolRecord* owned by a SymbolDocument in
     * symbolDocuments. Maintained by addBindings() and removeDocuments().
     */
    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>> declarations;

    /**
     * @brief Non-declaration reference occurrences per symbol identifier, sorted by path byte order.
     *
     * Each entry pairs pathIndex with a SymbolRecord* whose role lacks
     * jam::LanguageRole::declaration. Maintained by addBindings() and removeDocuments().
     */
    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>> references;

    /** @brief Completion records keyed by enclosing scope symbol (SymbolDocument::noSymbol for global scope). */
    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::CompletionRecord*>>> completions;

    /** @brief Validated SymbolDocument objects keyed by owning unit row index in files. */
    jam::HashMap<int, std::unique_ptr<SymbolDocument>> symbolDocuments;

    /** @brief Maps each indexed pathIndex to (owning unit row index, FileEntry* in that unit's SymbolDocument). */
    jam::HashMap<int, std::pair<int, const SymbolDocument::FileEntry*>> symbolFiles;

    /** @brief Snapshot of file-to-unit ownership and project roots shared with IndexJob instances. */
    std::shared_ptr<const UnitOwners> unitOwners { std::make_shared<const UnitOwners>() };

    /** @brief Maps each file path in files to its row index in files. */
    jam::HashMap<juce::String, int> pathRows;

    /** @brief Maps each filename stem in files to all row indices sharing that stem. */
    jam::HashMap<juce::String, jam::Array<int>> stemRows;

private:
    /**
     * @brief Tells whether bindings for @p unitPathIndex remain valid against @p files and @c unitOwners.
     *
     * @param unitPathIndex  Path index of the unit in @c paths.
     * @param files          Current files child tree.
     * @return @c true when the unit keeps its arguments and still owns every bound file.
     */
    bool isUnitRetained (int unitPathIndex, const juce::ValueTree& files) const;

    /** @brief Unregisters all symbolFiles and symbol tables contributed by unit @p unitPathIndex. */
    void removeBindings (int unitPathIndex);

    /** @brief Removes completions, declarations, and references registered for @p pathIndex. */
    void removeSymbols (int pathIndex);

    /** @brief Removes all occurrences for @p pathIndex from @p symbols under @p symbol. */
    void removeOccurrences (jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& symbols,
                            juce::int64 symbol,
                            int pathIndex);

    /** @brief Removes completion records contributed by @p file at @p pathIndex from completions. */
    void removeCompletions (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file);

    /** @brief Registers all completion records of @p file at @p pathIndex into completions. */
    void addCompletions (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file);

    /** @brief Populates completions, declarations, and references from @p file at @p pathIndex. */
    void addSymbols (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file);

    /** @brief Inserts per-symbol @p blocks for @p pathIndex into @p symbols in path byte order. */
    void addOccurrences (jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& symbols,
                         int pathIndex,
                         const jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& blocks);

    /**
     * @brief Returns the index of @p path in paths, inserting it into paths and pathOrder when absent.
     *
     * Nothing removes entries from paths, so each pathIndex stays valid for the
     * lifetime of the Index.
     *
     * @param path  Source path string.
     * @return Stable pathIndex in paths.
     */
    int getOrCreatePathIndex (const juce::String& path);

    /**
     * @brief Loads and validates the syntax swap document for @p pathIndex from disk.
     *
     * @param pathIndex   Path index in @c paths.
     * @param storedHash  Expected source content hash.
     * @return Validated document, or @c nullptr when missing or mismatched.
     */
    std::unique_ptr<jam::Document> getSwapDocument (int pathIndex, juce::int64 storedHash) const;
    /**
     * @brief Removes @p pathIndex from @c cachedDocuments when present.
     *
     * @param pathIndex  Path index in @c paths.
     */
    void removeCachedDocument (int pathIndex) const;
    /** @brief Evicts oldest cached documents above @c maxCachedDocuments when @c evictionLockDepth is zero. */
    void evictCachedDocuments() const;

    /** @brief Maximum number of disk-backed documents retained in @c cachedDocuments. */
    static constexpr int maxCachedDocuments { 64 };
    /** @brief Parsed syntax documents keyed by @c pathIndex, holding open buffers and LRU-cached files. */
    mutable jam::HashMap<int, std::unique_ptr<jam::Document>> documents;
    /** @brief LRU queue of disk-backed @c pathIndex keys ordered from oldest to most recently used. */
    mutable jam::Array<int> cachedDocuments;
    /** @brief Active DocumentScope nesting depth; nonzero suspends LRU cache eviction. */
    mutable int evictionLockDepth { 0 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Index)
};
