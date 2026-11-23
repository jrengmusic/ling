#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

/**
 * @class SymbolDocument
 * @brief Memory-mapped or owned binary index of symbols, completions, hovers, hints, diagnostics, outlines, and fix-its for one translation unit.
 *
 * Builder collects records on a pool thread and writes one aligned, little-endian
 * buffer with a fixed Header, typed section tables, and a trailing string pool.
 * SymbolDocument validates the buffer once in isValidBuffer() and exposes typed
 * Span views directly over the validated bytes.
 */
class SymbolDocument
{
public:
    /** @brief Sentinel symbol identifier for global or root scope. */
    static constexpr juce::int64 noSymbol { 0 };

    /** @brief Sentinel parent index for top-level outline entries. */
    static constexpr int noParent { -1 };

    /**
     * @struct Span
     * @brief Non-owning typed view over a contiguous slice of records inside a SymbolDocument buffer.
     *
     * @tparam Element  The record type in the backing table.
     */
    template <typename Element>
    struct Span
    {
        const Element* ptr { nullptr };///< First element in the slice, or nullptr when empty.
        uint32_t count { 0 };///< Number of elements in the slice.

        /** @return Pointer to the first element in the slice. */
        const Element* begin() const noexcept { return ptr; }

        /** @return Pointer one past the last element in the slice. */
        const Element* end() const noexcept { return ptr + count; }

        /** @return Number of elements in the slice. */
        uint32_t size() const noexcept { return count; }

        /** @return True when the slice holds zero elements. */
        bool empty() const noexcept { return count == 0; }

        /**
         * @brief Returns the element at @p index after asserting bounds.
         *
         * @param index  Zero-based element index in [0, count).
         * @return Reference to the element at @p index.
         */
        const Element& at (uint32_t index) const noexcept { jassert (index < count); return ptr[index]; }
    };

    /**
     * @struct Header
     * @brief Fixed binary header at offset zero of a serialized SymbolDocument buffer.
     *
     * Stores format identity, validation stamps, byte offsets and counts for each
     * section table, and the string pool range. Readers copy it via std::memcpy.
     */
    struct Header
    {
        uint32_t magic { 0 };///< Magic tag identifying a SymbolDocument buffer (fileMagic).
        uint32_t formatVersion { 0 };///< Binary layout version (currentFormatVersion).
        uint64_t totalBytes { 0 };///< Expected total byte size of the serialized buffer.
        int64_t hash { 0 };///< Content hash of the translation unit inputs.
        int64_t languageHash { 0 };///< Content hash of the language library that produced the index.
        int32_t family { 0 };///< Language family identifier of the translation unit.
        uint32_t versionOffset { 0 };///< Byte offset of the producer version string in the string pool.
        uint32_t versionLength { 0 };///< Byte length of the producer version string.
        uint32_t reserved { 0 };///< Alignment padding reserved for future header fields.
        uint64_t filesOffset { 0 };///< Byte offset of the FileEntry table.
        uint32_t filesCount { 0 };///< Total number of FileEntry records.
        uint32_t filesPadding { 0 };///< Alignment padding for filesCount.
        uint64_t symbolsOffset { 0 };///< Byte offset of the SymbolRecord table.
        uint32_t symbolsCount { 0 };///< Total number of SymbolRecord entries.
        uint32_t symbolsPadding { 0 };///< Alignment padding for symbolsCount.
        uint64_t typeSymbolsOffset { 0 };///< Byte offset of the int64_t type-symbol array.
        uint32_t typeSymbolsCount { 0 };///< Total number of int64_t type-symbol entries.
        uint32_t typeSymbolsPadding { 0 };///< Alignment padding for typeSymbolsCount.
        uint64_t completionsOffset { 0 };///< Byte offset of the CompletionRecord table.
        uint32_t completionsCount { 0 };///< Total number of CompletionRecord entries.
        uint32_t completionsPadding { 0 };///< Alignment padding for completionsCount.
        uint64_t hoversOffset { 0 };///< Byte offset of the HoverEntry table.
        uint32_t hoversCount { 0 };///< Total number of HoverEntry records.
        uint32_t hoversPadding { 0 };///< Alignment padding for hoversCount.
        uint64_t inlayHintsOffset { 0 };///< Byte offset of the InlayHintEntry table.
        uint32_t inlayHintsCount { 0 };///< Total number of InlayHintEntry records.
        uint32_t inlayHintsPadding { 0 };///< Alignment padding for inlayHintsCount.
        uint64_t diagnosticsOffset { 0 };///< Byte offset of the DiagnosticEntry table.
        uint32_t diagnosticsCount { 0 };///< Total number of DiagnosticEntry records.
        uint32_t diagnosticsPadding { 0 };///< Alignment padding for diagnosticsCount.
        uint64_t outlinesOffset { 0 };///< Byte offset of the OutlineEntry table.
        uint32_t outlinesCount { 0 };///< Total number of OutlineEntry records.
        uint32_t outlinesPadding { 0 };///< Alignment padding for outlinesCount.
        uint64_t fixItsOffset { 0 };///< Byte offset of the FixItEntry table.
        uint32_t fixItsCount { 0 };///< Total number of FixItEntry records.
        uint32_t fixItsPadding { 0 };///< Alignment padding for fixItsCount.
        uint64_t stringPoolOffset { 0 };///< Byte offset of the UTF-8 string pool.
        uint64_t stringPoolSize { 0 };///< Total byte size of the string pool.
    };

    /**
     * @struct FileEntry
     * @brief Index slice for one source file inside a SymbolDocument.
     *
     * Stores the file path range in the string pool and the [start, start + count)
     * subrange of that file inside each section table.
     */
    struct FileEntry
    {
        uint32_t pathOffset { 0 };///< Byte offset of the file path in the string pool.
        uint32_t pathLength { 0 };///< Byte length of the file path.
        uint32_t symbolsStart { 0 };///< Start index in the SymbolRecord table.
        uint32_t symbolsCount { 0 };///< Number of SymbolRecord entries for this file.
        uint32_t completionsStart { 0 };///< Start index in the CompletionRecord table.
        uint32_t completionsCount { 0 };///< Number of CompletionRecord entries for this file.
        uint32_t hoversStart { 0 };///< Start index in the HoverEntry table.
        uint32_t hoversCount { 0 };///< Number of HoverEntry records for this file.
        uint32_t inlayHintsStart { 0 };///< Start index in the InlayHintEntry table.
        uint32_t inlayHintsCount { 0 };///< Number of InlayHintEntry records for this file.
        uint32_t diagnosticsStart { 0 };///< Start index in the DiagnosticEntry table.
        uint32_t diagnosticsCount { 0 };///< Number of DiagnosticEntry records for this file.
        uint32_t outlinesStart { 0 };///< Start index in the OutlineEntry table.
        uint32_t outlinesCount { 0 };///< Number of OutlineEntry records for this file.
        uint32_t fixItsStart { 0 };///< Start index in the FixItEntry table.
        uint32_t fixItsCount { 0 };///< Number of FixItEntry records for this file.
    };

    /**
     * @struct SymbolRecord
     * @brief One indexed symbol occurrence at a 1-based line and column.
     */
    struct SymbolRecord
    {
        int64_t symbol { 0 };///< Unique symbol identifier.
        int32_t line { 0 };///< 1-based source line of the occurrence.
        int32_t column { 0 };///< 1-based byte column of the occurrence.
        int32_t nameLength { 0 };///< Byte length of the symbol name at this location.
        int32_t role { 0 };///< Bitmask of jam::LanguageRole flags for this occurrence.
        int32_t type { 0 };///< Token type (map::SyntaxTokenType) of the symbol.
        int32_t hover { 0 };///< Index into the file's HoverEntry slice, or jam::LanguageCallbacks::noHover.
        uint32_t typeSymbolsStart { 0 };///< Start index in the type-symbols table.
        uint32_t typeSymbolsCount { 0 };///< Number of associated type symbol identifiers.
    };

    /**
     * @struct CompletionRecord
     * @brief One completion item associated with a scope symbol.
     */
    struct CompletionRecord
    {
        int64_t scope { 0 };///< Enclosing scope symbol identifier, or noSymbol for global scope.
        int64_t symbol { 0 };///< Candidate symbol identifier.
        uint32_t labelOffset { 0 };///< Byte offset of the display label in the string pool.
        uint32_t labelLength { 0 };///< Byte length of the display label.
        uint32_t snippetOffset { 0 };///< Byte offset of the insert text or snippet in the string pool.
        uint32_t snippetLength { 0 };///< Byte length of the insert text or snippet.
        int32_t type { 0 };///< Completion item kind index.
        uint32_t padding { 0 };///< Alignment padding to 8-byte record boundary.
    };

    /**
     * @struct HoverEntry
     * @brief Hover documentation text and definition range for a symbol.
     */
    struct HoverEntry
    {
        uint32_t hoverOffset { 0 };///< Byte offset of the hover text in the string pool.
        uint32_t hoverLength { 0 };///< Byte length of the hover text.
        uint32_t pathOffset { 0 };///< Byte offset of the definition path in the string pool.
        uint32_t pathLength { 0 };///< Byte length of the definition path.
        int32_t line { 0 };///< 1-based start line of the hovered or defined range.
        int32_t column { 0 };///< 1-based start column of the hovered or defined range.
        int32_t endLine { 0 };///< 1-based end line of the range.
        int32_t endColumn { 0 };///< 1-based end column of the range.
    };

    /**
     * @struct InlayHintEntry
     * @brief One inlay hint anchored at a 1-based line and column.
     */
    struct InlayHintEntry
    {
        int32_t line { 0 };///< 1-based anchor line.
        int32_t column { 0 };///< 1-based anchor column.
        int32_t type { 0 };///< LSP inlay hint kind.
        uint32_t labelOffset { 0 };///< Byte offset of the hint label in the string pool.
        uint32_t labelLength { 0 };///< Byte length of the hint label.
        uint8_t paddingLeft { 0 };///< Nonzero when LSP paddingLeft is requested.
        uint8_t paddingRight { 0 };///< Nonzero when LSP paddingRight is requested.
        uint16_t padding { 0 };///< Alignment padding to 4-byte record boundary.
    };

    /**
     * @struct DiagnosticEntry
     * @brief One compiler diagnostic span and message for a source file.
     */
    struct DiagnosticEntry
    {
        int32_t line { 0 };///< 1-based start line of the diagnostic span.
        int32_t column { 0 };///< 1-based start column of the diagnostic span.
        int32_t endLine { 0 };///< 1-based end line of the diagnostic span.
        int32_t endColumn { 0 };///< 1-based end column of the diagnostic span.
        int32_t severity { 0 };///< LSP diagnostic severity level.
        uint32_t messageOffset { 0 };///< Byte offset of the diagnostic text in the string pool.
        uint32_t messageLength { 0 };///< Byte length of the diagnostic text.
        uint32_t padding { 0 };///< Alignment padding to 4-byte record boundary.
    };

    /**
     * @struct OutlineEntry
     * @brief One hierarchical document-symbol entry stored in depth-first preorder.
     */
    struct OutlineEntry
    {
        int32_t line { 0 };///< 1-based start line of the symbol range.
        int32_t column { 0 };///< 1-based start column of the symbol range.
        int32_t endLine { 0 };///< 1-based end line of the symbol range.
        int32_t endColumn { 0 };///< 1-based end column of the symbol range.
        int32_t nameLength { 0 };///< Byte length of the symbol name.
        int32_t qualifierLength { 0 };///< Byte length of the leading scope qualifier.
        int32_t type { 0 };///< LSP symbol kind.
        int32_t parent { 0 };///< Index of the parent OutlineEntry in the file's slice, or noParent.
    };

    /**
     * @struct FixItEntry
     * @brief One text replacement edit associated with a file diagnostic.
     */
    struct FixItEntry
    {
        int32_t line { 0 };///< 1-based start line of the replaced range.
        int32_t column { 0 };///< 1-based start column of the replaced range.
        int32_t endLine { 0 };///< 1-based end line of the replaced range.
        int32_t endColumn { 0 };///< 1-based end column of the replaced range.
        uint32_t textOffset { 0 };///< Byte offset of the replacement text in the string pool.
        uint32_t textLength { 0 };///< Byte length of the replacement text.
    };

    /**
     * @class Builder
     * @brief Accumulates per-file symbol tables on a worker thread and serializes them into a SymbolDocument.
     *
     * Collects records and string pool data, sorts outline trees into preorder,
     * removes duplicate inlay hints and fix-its, and emits an aligned binary image.
     */
    class Builder
    {
    public:
        /**
         * @brief Returns the file index for @p path, creating a new FileBuilder entry when absent.
         *
         * @param path  File path recorded in the document.
         * @return Zero-based file index in this builder.
         */
        int getOrCreateFile (std::string_view path);

        /**
         * @brief Appends one symbol occurrence and its associated type symbols to a file entry.
         *
         * @param fileIndex       File index returned by getOrCreateFile().
         * @param symbol          Unique symbol identifier.
         * @param line            1-based source line.
         * @param column          1-based byte column.
         * @param nameLength      Byte length of the symbol name.
         * @param role            Bitmask of jam::LanguageRole flags.
         * @param type            Token type of the symbol.
         * @param hover           Hover entry index in the file, or jam::LanguageCallbacks::noHover.
         * @param newTypeSymbols  Array of associated type symbol identifiers.
         * @param typeCount       Number of entries in @p newTypeSymbols.
         */
        void addSymbol (int fileIndex, int64_t symbol, int32_t line, int32_t column, int32_t nameLength, int32_t role, int32_t type, int32_t hover, const unsigned long long* newTypeSymbols, int typeCount);

        /**
         * @brief Appends one hover entry to a file and returns its per-file index.
         *
         * @param fileIndex       File index returned by getOrCreateFile().
         * @param text            Hover documentation text.
         * @param definitionPath  Definition file path, or empty.
         * @param line            1-based start line.
         * @param column          1-based start column.
         * @param endLine         1-based end line.
         * @param endColumn       1-based end column.
         * @return Zero-based index of the added HoverEntry within the file.
         */
        int addHover (int fileIndex, std::string_view text, std::string_view definitionPath, int32_t line, int32_t column, int32_t endLine, int32_t endColumn);

        /**
         * @brief Appends one inlay hint to a file entry.
         *
         * @param fileIndex     File index returned by getOrCreateFile().
         * @param line          1-based anchor line.
         * @param column        1-based anchor column.
         * @param label         Hint display text.
         * @param type          LSP inlay hint kind.
         * @param paddingLeft   True to request padding before the label.
         * @param paddingRight  True to request padding after the label.
         */
        void addInlayHint (int fileIndex, int32_t line, int32_t column, std::string_view label, int32_t type, bool paddingLeft, bool paddingRight);

        /**
         * @brief Appends one diagnostic entry to a file.
         *
         * @param fileIndex  File index returned by getOrCreateFile().
         * @param line       1-based start line.
         * @param column     1-based start column.
         * @param endLine    1-based end line.
         * @param endColumn  1-based end column.
         * @param severity   LSP diagnostic severity.
         * @param message    Diagnostic message text.
         */
        void addDiagnostic (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, int32_t severity, std::string_view message);

        /**
         * @brief Appends one outline entry to a file and returns its pre-sort index.
         *
         * @param fileIndex        File index returned by getOrCreateFile().
         * @param line             1-based start line.
         * @param column           1-based start column.
         * @param endLine          1-based end line.
         * @param endColumn        1-based end column.
         * @param nameLength       Byte length of the symbol name.
         * @param qualifierLength  Byte length of the scope qualifier.
         * @param type             LSP symbol kind.
         * @param parent           Parent index within the file, or noParent.
         * @return Zero-based index of the added OutlineEntry before sortAndDeduplicate().
         */
        int addOutline (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, int32_t nameLength, int32_t qualifierLength, int32_t type, int32_t parent);

        /**
         * @brief Appends one completion candidate to a file entry.
         *
         * @param fileIndex    File index returned by getOrCreateFile().
         * @param scopeSymbol  Enclosing scope symbol identifier, or noSymbol.
         * @param symbol       Candidate symbol identifier.
         * @param label        Display label for completion filtering.
         * @param snippet      Inserted text or LSP snippet.
         * @param type         Completion item kind index.
         */
        void addCompletion (int fileIndex, int64_t scopeSymbol, int64_t symbol, std::string_view label, std::string_view snippet, int32_t type);

        /**
         * @brief Appends one fix-it text edit to a file entry.
         *
         * @param fileIndex  File index returned by getOrCreateFile().
         * @param line       1-based start line.
         * @param column     1-based start column.
         * @param endLine    1-based end line.
         * @param endColumn  1-based end column.
         * @param text       Replacement text.
         */
        void addFixIt (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, std::string_view text);

        /**
         * @brief Reorders each file's outlines into preorder and removes duplicate inlay hints and fix-its.
         */
        void sortAndDeduplicate();

        /**
         * @brief Serializes the built tables and writes them directly to swapFile.
         *
         * @param swapFile      The swap file to write.
         * @param hash          Content hash of the unit inputs.
         * @param languageHash  Hash of the producing language library.
         * @param family        Language family identifier.
         * @return ok when the write succeeds. Otherwise the text is text::Diagnostics::failSwapWrite,
         *         the full path of swapFile, and the system error text, separated by Id::diagnosticSeparator.
         */
        juce::Result writeSwapFile (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family) const;

        /**
         * @brief Serializes the built tables into an owned memory block and wraps it in a SymbolDocument.
         *
         * @param hash          Content hash of the unit inputs.
         * @param languageHash  Hash of the producing language library.
         * @param family        Language family identifier.
         * @return In-memory SymbolDocument owning the serialized bytes.
         */
        std::unique_ptr<SymbolDocument> buildInMemory (juce::int64 hash, juce::int64 languageHash, int family) const;

    private:
        struct FileBuilder
        {
            uint32_t pathOffset { 0 };
            uint32_t pathLength { 0 };
            jam::Array<SymbolRecord> symbols;
            jam::Array<CompletionRecord> completions;
            jam::Array<HoverEntry> hovers;
            jam::Array<InlayHintEntry> inlayHints;
            jam::Array<DiagnosticEntry> diagnostics;
            jam::Array<OutlineEntry> outlines;
            jam::Array<FixItEntry> fixIts;
        };

        std::pair<uint32_t, uint32_t> addString (std::string_view text);
        std::string_view getString (uint32_t offset, uint32_t length) const noexcept;
        juce::MemoryBlock serialize (juce::int64 hash, juce::int64 languageHash, int family) const;

        jam::Array<FileBuilder> files;
        jam::Array<int64_t> typeSymbols;
        std::string stringPool;
        jam::HashMap<std::string, int> pathIndices;
    };

    /**
     * @brief Maps @p swapFile read-only and returns a SymbolDocument when isValidBuffer() succeeds.
     *
     * @param swapFile      Swap file to map.
     * @param hash          Expected content hash of the unit inputs.
     * @param languageHash  Expected hash of the language library.
     * @param family        Expected language family identifier.
     * @return Validated SymbolDocument, or nullptr when mapping or validation fails.
     */
    static std::unique_ptr<SymbolDocument> open (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family);

    /** @return Span of all FileEntry records in the document. */
    Span<FileEntry> getFiles() const noexcept;

    /**
     * @brief Returns the SymbolRecord slice belonging to @p file.
     *
     * @param file  File entry within this document.
     * @return Span of symbol records for @p file.
     */
    Span<SymbolRecord> getSymbols (const FileEntry& file) const noexcept;

    /**
     * @brief Returns the type-symbol identifiers associated with @p record.
     *
     * @param record  Symbol record within this document.
     * @return Span of type symbol identifiers for @p record.
     */
    Span<int64_t> getTypeSymbols (const SymbolRecord& record) const noexcept;

    /**
     * @brief Returns the CompletionRecord slice belonging to @p file.
     *
     * @param file  File entry within this document.
     * @return Span of completion records for @p file.
     */
    Span<CompletionRecord> getCompletions (const FileEntry& file) const noexcept;

    /**
     * @brief Returns the HoverEntry slice belonging to @p file.
     *
     * @param file  File entry within this document.
     * @return Span of hover entries for @p file.
     */
    Span<HoverEntry> getHovers (const FileEntry& file) const noexcept;

    /**
     * @brief Returns the InlayHintEntry slice belonging to @p file.
     *
     * @param file  File entry within this document.
     * @return Span of inlay hint entries for @p file.
     */
    Span<InlayHintEntry> getInlayHints (const FileEntry& file) const noexcept;

    /**
     * @brief Returns the DiagnosticEntry slice belonging to @p file.
     *
     * @param file  File entry within this document.
     * @return Span of diagnostic entries for @p file.
     */
    Span<DiagnosticEntry> getDiagnostics (const FileEntry& file) const noexcept;

    /**
     * @brief Returns the OutlineEntry slice belonging to @p file.
     *
     * @param file  File entry within this document.
     * @return Span of outline entries in preorder for @p file.
     */
    Span<OutlineEntry> getOutlines (const FileEntry& file) const noexcept;

    /**
     * @brief Returns the FixItEntry slice belonging to @p file.
     *
     * @param file  File entry within this document.
     * @return Span of fix-it entries for @p file.
     */
    Span<FixItEntry> getFixIts (const FileEntry& file) const noexcept;

    /**
     * @brief Returns a string-pool slice at [@p offset, @p offset + @p length).
     *
     * @param offset  Byte offset inside the string pool.
     * @param length  Byte length of the slice.
     * @return View into the validated string pool.
     */
    std::string_view getString (uint32_t offset, uint32_t length) const noexcept;

private:
    struct Access {};

    static constexpr uint32_t fileMagic { 0x4c494e47 };
    static constexpr uint32_t currentFormatVersion { 1 };
    static constexpr uint64_t sectionAlignment { 8 };

public:
    /**
     * @brief Constructs a SymbolDocument backed by a validated memory-mapped file.
     *
     * The private Access passkey restricts construction to open() and Builder.
     *
     * @param mappedFile  Validated read-only mapped file.
     */
    SymbolDocument (Access, std::unique_ptr<juce::MemoryMappedFile> mappedFile);

    /**
     * @brief Constructs a SymbolDocument backed by an owned serialized memory block.
     *
     * The private Access passkey restricts construction to open() and Builder.
     *
     * @param ownedBlock  Serialized buffer produced by Builder::serialize().
     */
    SymbolDocument (Access, juce::MemoryBlock ownedBlock);

private:
    /**
     * @brief Validates header stamps, section alignment and bounds, version string, and all per-record indices and string-pool slices.
     *
     * Ensures no section overflows @p size, every non-empty section offset satisfies
     * alignof(Record), and every FileEntry, SymbolRecord, HoverEntry, CompletionRecord,
     * InlayHintEntry, DiagnosticEntry, and FixItEntry index or string slice is in bounds.
     *
     * @param data          Start of the candidate buffer.
     * @param size          Byte size of the buffer.
     * @param hash          Expected content hash.
     * @param languageHash  Expected language library hash.
     * @param family        Expected language family identifier.
     * @return True when the entire binary layout passes validation.
     */
    static bool isValidBuffer (const uint8_t* data, size_t size, juce::int64 hash, juce::int64 languageHash, int family) noexcept;

    /**
     * @brief Copies the Header from @p base by value via std::memcpy.
     *
     * @return Copy of the buffer Header without unaligned or type-punned access.
     */
    Header getHeader() const noexcept;

    /**
     * @brief Returns a typed pointer to a validated section table at @p offset.
     *
     * @tparam Record  Trivially copyable record type of the target section.
     * @param offset   Section byte offset from the Header.
     * @return Pointer to the first Record in the section.
     */
    template <typename Record>
    const Record* getTable (uint64_t offset) const noexcept
    {
        // Builder::serialize aligns every section to sectionAlignment (8 bytes) and
        // isValidBuffer verifies (offset % alignof(Record)) == 0 and byte bounds when count > 0.
        return reinterpret_cast<const Record*> (base + offset);
    }

    std::unique_ptr<juce::MemoryMappedFile> mapped;
    juce::MemoryBlock owned;
    const uint8_t* base { nullptr };
    size_t byteSize { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SymbolDocument)
};
