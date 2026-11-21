#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

class SymbolDocument
{
public:
    static constexpr juce::int64 noSymbol { 0 };
    static constexpr int noParent { -1 };

    template <typename Element>
    struct Span
    {
        const Element* ptr { nullptr };
        uint32_t count { 0 };

        const Element* begin() const noexcept { return ptr; }
        const Element* end() const noexcept { return ptr + count; }
        uint32_t size() const noexcept { return count; }
        bool empty() const noexcept { return count == 0; }
        const Element& at (uint32_t index) const noexcept { jassert (index < count); return ptr[index]; }
    };

    struct Header
    {
        uint32_t magic { 0 };
        uint32_t formatVersion { 0 };
        uint64_t totalBytes { 0 };
        int64_t hash { 0 };
        int64_t languageHash { 0 };
        int32_t family { 0 };
        uint32_t versionOffset { 0 };
        uint32_t versionLength { 0 };
        uint32_t reserved { 0 };
        uint64_t filesOffset { 0 };
        uint32_t filesCount { 0 };
        uint32_t filesPadding { 0 };
        uint64_t symbolsOffset { 0 };
        uint32_t symbolsCount { 0 };
        uint32_t symbolsPadding { 0 };
        uint64_t typeSymbolsOffset { 0 };
        uint32_t typeSymbolsCount { 0 };
        uint32_t typeSymbolsPadding { 0 };
        uint64_t completionsOffset { 0 };
        uint32_t completionsCount { 0 };
        uint32_t completionsPadding { 0 };
        uint64_t hoversOffset { 0 };
        uint32_t hoversCount { 0 };
        uint32_t hoversPadding { 0 };
        uint64_t inlayHintsOffset { 0 };
        uint32_t inlayHintsCount { 0 };
        uint32_t inlayHintsPadding { 0 };
        uint64_t diagnosticsOffset { 0 };
        uint32_t diagnosticsCount { 0 };
        uint32_t diagnosticsPadding { 0 };
        uint64_t outlinesOffset { 0 };
        uint32_t outlinesCount { 0 };
        uint32_t outlinesPadding { 0 };
        uint64_t fixItsOffset { 0 };
        uint32_t fixItsCount { 0 };
        uint32_t fixItsPadding { 0 };
        uint64_t stringPoolOffset { 0 };
        uint64_t stringPoolSize { 0 };
    };

    struct FileEntry
    {
        uint32_t pathOffset { 0 };
        uint32_t pathLength { 0 };
        uint32_t symbolsStart { 0 };
        uint32_t symbolsCount { 0 };
        uint32_t completionsStart { 0 };
        uint32_t completionsCount { 0 };
        uint32_t hoversStart { 0 };
        uint32_t hoversCount { 0 };
        uint32_t inlayHintsStart { 0 };
        uint32_t inlayHintsCount { 0 };
        uint32_t diagnosticsStart { 0 };
        uint32_t diagnosticsCount { 0 };
        uint32_t outlinesStart { 0 };
        uint32_t outlinesCount { 0 };
        uint32_t fixItsStart { 0 };
        uint32_t fixItsCount { 0 };
    };

    struct SymbolRecord
    {
        int64_t symbol { 0 };
        int32_t line { 0 };
        int32_t column { 0 };
        int32_t nameLength { 0 };
        int32_t role { 0 };
        int32_t type { 0 };
        int32_t hover { 0 };
        uint32_t typeSymbolsStart { 0 };
        uint32_t typeSymbolsCount { 0 };
    };

    struct CompletionRecord
    {
        int64_t scope { 0 };
        int64_t symbol { 0 };
        uint32_t labelOffset { 0 };
        uint32_t labelLength { 0 };
        uint32_t snippetOffset { 0 };
        uint32_t snippetLength { 0 };
        int32_t type { 0 };
        uint32_t padding { 0 };
    };

    struct HoverEntry
    {
        uint32_t hoverOffset { 0 };
        uint32_t hoverLength { 0 };
        uint32_t pathOffset { 0 };
        uint32_t pathLength { 0 };
        int32_t line { 0 };
        int32_t column { 0 };
        int32_t endLine { 0 };
        int32_t endColumn { 0 };
    };

    struct InlayHintEntry
    {
        int32_t line { 0 };
        int32_t column { 0 };
        int32_t type { 0 };
        uint32_t labelOffset { 0 };
        uint32_t labelLength { 0 };
        uint8_t paddingLeft { 0 };
        uint8_t paddingRight { 0 };
        uint16_t padding { 0 };
    };

    struct DiagnosticEntry
    {
        int32_t line { 0 };
        int32_t column { 0 };
        int32_t endLine { 0 };
        int32_t endColumn { 0 };
        int32_t severity { 0 };
        uint32_t messageOffset { 0 };
        uint32_t messageLength { 0 };
        uint32_t padding { 0 };
    };

    struct OutlineEntry
    {
        int32_t line { 0 };
        int32_t column { 0 };
        int32_t endLine { 0 };
        int32_t endColumn { 0 };
        int32_t nameLength { 0 };
        int32_t qualifierLength { 0 };
        int32_t type { 0 };
        int32_t parent { 0 };
    };

    struct FixItEntry
    {
        int32_t line { 0 };
        int32_t column { 0 };
        int32_t endLine { 0 };
        int32_t endColumn { 0 };
        uint32_t textOffset { 0 };
        uint32_t textLength { 0 };
    };

    class Builder
    {
    public:
        int getOrCreateFile (std::string_view path);

        void addSymbol (int fileIndex, int64_t symbol, int32_t line, int32_t column, int32_t nameLength, int32_t role, int32_t type, int32_t hover, const unsigned long long* newTypeSymbols, int typeCount);

        int addHover (int fileIndex, std::string_view text, std::string_view definitionPath, int32_t line, int32_t column, int32_t endLine, int32_t endColumn);

        void addInlayHint (int fileIndex, int32_t line, int32_t column, std::string_view label, int32_t type, bool paddingLeft, bool paddingRight);

        void addDiagnostic (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, int32_t severity, std::string_view message);

        int addOutline (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, int32_t nameLength, int32_t qualifierLength, int32_t type, int32_t parent);

        void addCompletion (int fileIndex, int64_t scopeSymbol, int64_t symbol, std::string_view label, std::string_view snippet, int32_t type);

        void addFixIt (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, std::string_view text);

        void sortAndDeduplicate();

        std::unique_ptr<SymbolDocument> buildToSwap (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family) const;

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

        std::vector<FileBuilder> files;
        jam::Array<int64_t> typeSymbols;
        std::string stringPool;
        jam::HashMap<std::string, int> pathIndices;
    };

    static std::unique_ptr<SymbolDocument> open (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family);

    Span<FileEntry> getFiles() const noexcept;
    Span<SymbolRecord> getSymbols (const FileEntry& file) const noexcept;
    Span<int64_t> getTypeSymbols (const SymbolRecord& record) const noexcept;
    Span<CompletionRecord> getCompletions (const FileEntry& file) const noexcept;
    Span<HoverEntry> getHovers (const FileEntry& file) const noexcept;
    Span<InlayHintEntry> getInlayHints (const FileEntry& file) const noexcept;
    Span<DiagnosticEntry> getDiagnostics (const FileEntry& file) const noexcept;
    Span<OutlineEntry> getOutlines (const FileEntry& file) const noexcept;
    Span<FixItEntry> getFixIts (const FileEntry& file) const noexcept;
    std::string_view getString (uint32_t offset, uint32_t length) const noexcept;

private:
    struct Access {};

    static constexpr uint32_t fileMagic { 0x4c494e47 };
    static constexpr uint32_t currentFormatVersion { 1 };
    static constexpr uint64_t sectionAlignment { 8 };

public:
    SymbolDocument (Access, std::unique_ptr<juce::MemoryMappedFile> mappedFile);
    SymbolDocument (Access, juce::MemoryBlock ownedBlock);

private:
    static bool isValidBuffer (const uint8_t* data, size_t size, juce::int64 hash, juce::int64 languageHash, int family) noexcept;

    const Header& getHeader() const noexcept;

    template <typename Record>
    const Record* getTable (uint64_t offset) const noexcept
    {
        return reinterpret_cast<const Record*> (base + offset);
    }

    std::unique_ptr<juce::MemoryMappedFile> mapped;
    juce::MemoryBlock owned;
    const uint8_t* base { nullptr };
    size_t byteSize { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SymbolDocument)
};
