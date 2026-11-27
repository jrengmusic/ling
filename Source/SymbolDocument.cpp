#include "SymbolDocument.h"

static_assert (std::is_trivially_copyable_v<SymbolDocument::Header>);
static_assert (std::is_trivially_copyable_v<SymbolDocument::FileEntry>);
static_assert (std::is_trivially_copyable_v<SymbolDocument::SymbolRecord>);
static_assert (std::is_trivially_copyable_v<SymbolDocument::CompletionRecord>);
static_assert (std::is_trivially_copyable_v<SymbolDocument::HoverEntry>);
static_assert (std::is_trivially_copyable_v<SymbolDocument::InlayHintEntry>);
static_assert (std::is_trivially_copyable_v<SymbolDocument::DiagnosticEntry>);
static_assert (std::is_trivially_copyable_v<SymbolDocument::OutlineEntry>);
static_assert (std::is_trivially_copyable_v<SymbolDocument::FixItEntry>);

static constexpr uint64_t alignOffset (uint64_t offset, uint64_t alignment) noexcept
{
    return (offset + alignment - 1) & ~ (alignment - 1);
}

static bool isSectionValid (uint64_t offset, uint32_t count, size_t elementSize, size_t alignment, uint64_t totalBytes) noexcept
{
    const auto bytes { static_cast<uint64_t> (count) * static_cast<uint64_t> (elementSize) };

    return offset <= totalBytes and bytes <= totalBytes - offset and (count == 0 or (offset % alignment) == 0);
}

static bool isRangeValid (uint32_t start, uint32_t count, uint64_t limit) noexcept
{
    return static_cast<uint64_t> (start) + static_cast<uint64_t> (count) <= limit;
}

template <typename Entry, typename GetKey>
static void removeDuplicateEntries (jam::Array<Entry>& entries, const GetKey& getKey)
{
    std::sort (entries.begin(), entries.end(), [&getKey] (const Entry& left, const Entry& right)
    {
        return getKey (left) < getKey (right);
    });

    const auto duplicates { std::unique (entries.begin(), entries.end(), [&getKey] (const Entry& left, const Entry& right)
    {
        return getKey (left) == getKey (right);
    }) };

    entries.removeRange (static_cast<int> (std::distance (entries.begin(), duplicates)),
                         static_cast<int> (std::distance (duplicates, entries.end())));
}

static jam::HashMap<int, jam::Array<int>> getOutlineChildren (const jam::Array<SymbolDocument::OutlineEntry>& outlines)
{
    jam::HashMap<int, jam::Array<int>> children;

    for (int outlineIndex { 0 }; outlineIndex < outlines.size(); ++outlineIndex)
    {
        const auto [childrenEntry, isChildrenAdded] { children.try_emplace (outlines.at (outlineIndex).parent) };
        auto& [parentIndex, parentChildren] { *childrenEntry };

        parentChildren.add (outlineIndex);
    }

    return children;
}

static void sortOutlineEntries (jam::Array<SymbolDocument::OutlineEntry>& outlines)
{
    const auto children (getOutlineChildren (outlines));
    jam::Array<SymbolDocument::OutlineEntry> sorted;
    jam::Array<std::pair<int, int>> pending;
    const auto addPending { [&children, &pending] (int parentIndex, int sortedParentIndex)
    {
        if (const auto entry { children.find (parentIndex) }; entry != children.end())
        {
            const auto& [entryParentIndex, parentChildren] { *entry };

            for (auto child { parentChildren.end() }; child != parentChildren.begin();)
                pending.add ({ *--child, sortedParentIndex });
        }
    } };

    sorted.ensureStorageAllocated (outlines.size());
    addPending (SymbolDocument::noParent, SymbolDocument::noParent);

    while (not pending.isEmpty())
    {
        const auto [outlineIndex, sortedParentIndex] { pending.last() };
        auto outline { outlines.at (outlineIndex) };

        pending.remove (pending.size() - 1);
        outline.parent = sortedParentIndex;
        sorted.add (outline);
        addPending (outlineIndex, sorted.size() - 1);
    }

    jassert (sorted.size() == outlines.size());
    outlines = std::move (sorted);
}

template <typename Entry>
static void copyArray (uint8_t* destination, uint64_t& offset, const jam::Array<Entry>& entries)
{
    if (not entries.isEmpty())
    {
        const auto bytes { static_cast<size_t> (entries.size()) * sizeof (Entry) };

        std::memcpy (destination + offset, entries.data(), bytes);
        offset += bytes;
    }
}

int SymbolDocument::Builder::getOrCreateFile (std::string_view path)
{
    std::string key { path };

    if (const auto entry { pathIndices.find (key) }; entry != pathIndices.end())
    {
        const auto& [entryPath, fileIndex] { *entry };

        return fileIndex;
    }

    const auto fileIndex { files.size() };
    const auto [pathOffset, pathLength] { addString (path) };
    FileBuilder file;

    file.pathOffset = pathOffset;
    file.pathLength = pathLength;
    files.add (std::move (file));
    pathIndices.try_emplace (std::move (key), fileIndex);

    return fileIndex;
}

void SymbolDocument::Builder::addSymbol (int fileIndex, int64_t symbol, int32_t line, int32_t column, int32_t nameLength, int32_t role, int32_t type, int32_t hover, const unsigned long long* newTypeSymbols, int typeCount)
{
    const auto typeSymbolsStart { static_cast<uint32_t> (typeSymbols.size()) };

    for (int typeIndex { 0 }; typeIndex < typeCount; ++typeIndex)
        typeSymbols.add (static_cast<int64_t> (newTypeSymbols[typeIndex]));

    SymbolRecord record {};

    record.symbol = symbol;
    record.line = line;
    record.column = column;
    record.nameLength = nameLength;
    record.role = role;
    record.type = type;
    record.hover = hover;
    record.typeSymbolsStart = typeSymbolsStart;
    record.typeSymbolsCount = static_cast<uint32_t> (typeCount);

    files.at (fileIndex).symbols.add (record);
}

int SymbolDocument::Builder::addHover (int fileIndex, std::string_view text, std::string_view definitionPath, int32_t line, int32_t column, int32_t endLine, int32_t endColumn)
{
    auto& hovers { files.at (fileIndex).hovers };
    const auto hoverIndex { hovers.size() };
    const auto [hoverOffset, hoverLength] { addString (text) };
    const auto [pathOffset, pathLength] { addString (definitionPath) };
    HoverEntry entry {};

    entry.hoverOffset = hoverOffset;
    entry.hoverLength = hoverLength;
    entry.pathOffset = pathOffset;
    entry.pathLength = pathLength;
    entry.line = line;
    entry.column = column;
    entry.endLine = endLine;
    entry.endColumn = endColumn;
    hovers.add (entry);

    return hoverIndex;
}

void SymbolDocument::Builder::addInlayHint (int fileIndex, int32_t line, int32_t column, std::string_view label, int32_t type, bool paddingLeft, bool paddingRight)
{
    const auto [labelOffset, labelLength] { addString (label) };
    InlayHintEntry entry {};

    entry.line = line;
    entry.column = column;
    entry.type = type;
    entry.labelOffset = labelOffset;
    entry.labelLength = labelLength;
    entry.paddingLeft = paddingLeft ? 1 : 0;
    entry.paddingRight = paddingRight ? 1 : 0;

    files.at (fileIndex).inlayHints.add (entry);
}

void SymbolDocument::Builder::addDiagnostic (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, int32_t severity, std::string_view message)
{
    const auto [messageOffset, messageLength] { addString (message) };
    DiagnosticEntry entry {};

    entry.line = line;
    entry.column = column;
    entry.endLine = endLine;
    entry.endColumn = endColumn;
    entry.severity = severity;
    entry.messageOffset = messageOffset;
    entry.messageLength = messageLength;

    files.at (fileIndex).diagnostics.add (entry);
}

int SymbolDocument::Builder::addOutline (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, int32_t nameLength, int32_t qualifierLength, int32_t type, int32_t parent)
{
    auto& outlines { files.at (fileIndex).outlines };
    const auto outlineIndex { outlines.size() };
    OutlineEntry entry {};

    entry.line = line;
    entry.column = column;
    entry.endLine = endLine;
    entry.endColumn = endColumn;
    entry.nameLength = nameLength;
    entry.qualifierLength = qualifierLength;
    entry.type = type;
    entry.parent = parent;
    outlines.add (entry);

    return outlineIndex;
}

void SymbolDocument::Builder::addCompletion (int fileIndex, int64_t scopeSymbol, int64_t symbol, std::string_view label, std::string_view snippet, int32_t type)
{
    const auto [labelOffset, labelLength] { addString (label) };
    const auto [snippetOffset, snippetLength] { addString (snippet) };
    CompletionRecord record {};

    record.scope = scopeSymbol;
    record.symbol = symbol;
    record.labelOffset = labelOffset;
    record.labelLength = labelLength;
    record.snippetOffset = snippetOffset;
    record.snippetLength = snippetLength;
    record.type = type;

    files.at (fileIndex).completions.add (record);
}

void SymbolDocument::Builder::addFixIt (int fileIndex, int32_t line, int32_t column, int32_t endLine, int32_t endColumn, std::string_view text)
{
    const auto [textOffset, textLength] { addString (text) };
    FixItEntry entry {};

    entry.line = line;
    entry.column = column;
    entry.endLine = endLine;
    entry.endColumn = endColumn;
    entry.textOffset = textOffset;
    entry.textLength = textLength;

    files.at (fileIndex).fixIts.add (entry);
}

void SymbolDocument::Builder::sortAndDeduplicate()
{
    for (auto& file : files)
    {
        std::stable_sort (file.symbols.begin(), file.symbols.end(), [] (const SymbolRecord& left, const SymbolRecord& right)
        {
            return std::make_pair (left.line, left.column) < std::make_pair (right.line, right.column);
        });

        sortOutlineEntries (file.outlines);

        removeDuplicateEntries (file.inlayHints, [this] (const InlayHintEntry& entry)
        {
            return std::make_tuple (entry.line, entry.column, getString (entry.labelOffset, entry.labelLength));
        });

        removeDuplicateEntries (file.fixIts, [this] (const FixItEntry& entry)
        {
            return std::make_tuple (entry.line, entry.column, entry.endLine, entry.endColumn, getString (entry.textOffset, entry.textLength));
        });
    }
}

std::pair<uint32_t, uint32_t> SymbolDocument::Builder::addString (std::string_view text)
{
    const auto offset { static_cast<uint32_t> (stringPool.size()) };
    const auto length { static_cast<uint32_t> (text.size()) };

    stringPool.append (text.data(), text.size());

    return { offset, length };
}

std::string_view SymbolDocument::Builder::getString (uint32_t offset, uint32_t length) const noexcept
{
    return std::string_view { stringPool.data() + offset, length };
}

juce::MemoryBlock SymbolDocument::Builder::serialize (juce::int64 hash, juce::int64 languageHash, int family) const
{
    Header header {};
    const std::string_view versionView { ProjectInfo::versionString };

    header.magic = fileMagic;
    header.formatVersion = currentFormatVersion;
    header.hash = hash;
    header.languageHash = languageHash;
    header.family = family;
    header.filesCount = static_cast<uint32_t> (files.size());
    header.typeSymbolsCount = static_cast<uint32_t> (typeSymbols.size());

    for (const auto& file : files)
    {
        header.symbolsCount += static_cast<uint32_t> (file.symbols.size());
        header.completionsCount += static_cast<uint32_t> (file.completions.size());
        header.hoversCount += static_cast<uint32_t> (file.hovers.size());
        header.inlayHintsCount += static_cast<uint32_t> (file.inlayHints.size());
        header.diagnosticsCount += static_cast<uint32_t> (file.diagnostics.size());
        header.outlinesCount += static_cast<uint32_t> (file.outlines.size());
        header.fixItsCount += static_cast<uint32_t> (file.fixIts.size());
    }

    uint64_t cursor { alignOffset (sizeof (Header), sectionAlignment) };

    header.filesOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.filesCount) * sizeof (FileEntry), sectionAlignment);

    header.symbolsOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.symbolsCount) * sizeof (SymbolRecord), sectionAlignment);

    header.typeSymbolsOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.typeSymbolsCount) * sizeof (int64_t), sectionAlignment);

    header.completionsOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.completionsCount) * sizeof (CompletionRecord), sectionAlignment);

    header.hoversOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.hoversCount) * sizeof (HoverEntry), sectionAlignment);

    header.inlayHintsOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.inlayHintsCount) * sizeof (InlayHintEntry), sectionAlignment);

    header.diagnosticsOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.diagnosticsCount) * sizeof (DiagnosticEntry), sectionAlignment);

    header.outlinesOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.outlinesCount) * sizeof (OutlineEntry), sectionAlignment);

    header.fixItsOffset = cursor;
    cursor = alignOffset (cursor + static_cast<uint64_t> (header.fixItsCount) * sizeof (FixItEntry), sectionAlignment);

    header.stringPoolOffset = cursor;
    header.versionOffset = static_cast<uint32_t> (stringPool.size());
    header.versionLength = static_cast<uint32_t> (versionView.size());
    header.stringPoolSize = static_cast<uint64_t> (stringPool.size() + versionView.size());
    header.totalBytes = header.stringPoolOffset + header.stringPoolSize;

    juce::MemoryBlock bytes { static_cast<size_t> (header.totalBytes), true };
    auto* destination { static_cast<uint8_t*> (bytes.getData()) };

    std::memcpy (destination, &header, sizeof (Header));

    uint64_t fileWriteOffset { header.filesOffset };
    uint64_t symbolWriteOffset { header.symbolsOffset };
    uint64_t completionWriteOffset { header.completionsOffset };
    uint64_t hoverWriteOffset { header.hoversOffset };
    uint64_t inlayHintWriteOffset { header.inlayHintsOffset };
    uint64_t diagnosticWriteOffset { header.diagnosticsOffset };
    uint64_t outlineWriteOffset { header.outlinesOffset };
    uint64_t fixItWriteOffset { header.fixItsOffset };

    uint32_t symbolsCursor { 0 };
    uint32_t completionsCursor { 0 };
    uint32_t hoversCursor { 0 };
    uint32_t inlayHintsCursor { 0 };
    uint32_t diagnosticsCursor { 0 };
    uint32_t outlinesCursor { 0 };
    uint32_t fixItsCursor { 0 };

    for (const auto& file : files)
    {
        FileEntry entry {};

        entry.pathOffset = file.pathOffset;
        entry.pathLength = file.pathLength;
        entry.symbolsStart = symbolsCursor;
        entry.symbolsCount = static_cast<uint32_t> (file.symbols.size());
        entry.completionsStart = completionsCursor;
        entry.completionsCount = static_cast<uint32_t> (file.completions.size());
        entry.hoversStart = hoversCursor;
        entry.hoversCount = static_cast<uint32_t> (file.hovers.size());
        entry.inlayHintsStart = inlayHintsCursor;
        entry.inlayHintsCount = static_cast<uint32_t> (file.inlayHints.size());
        entry.diagnosticsStart = diagnosticsCursor;
        entry.diagnosticsCount = static_cast<uint32_t> (file.diagnostics.size());
        entry.outlinesStart = outlinesCursor;
        entry.outlinesCount = static_cast<uint32_t> (file.outlines.size());
        entry.fixItsStart = fixItsCursor;
        entry.fixItsCount = static_cast<uint32_t> (file.fixIts.size());

        std::memcpy (destination + fileWriteOffset, &entry, sizeof (FileEntry));
        fileWriteOffset += sizeof (FileEntry);

        copyArray (destination, symbolWriteOffset, file.symbols);
        copyArray (destination, completionWriteOffset, file.completions);
        copyArray (destination, hoverWriteOffset, file.hovers);
        copyArray (destination, inlayHintWriteOffset, file.inlayHints);
        copyArray (destination, diagnosticWriteOffset, file.diagnostics);
        copyArray (destination, outlineWriteOffset, file.outlines);
        copyArray (destination, fixItWriteOffset, file.fixIts);

        symbolsCursor += entry.symbolsCount;
        completionsCursor += entry.completionsCount;
        hoversCursor += entry.hoversCount;
        inlayHintsCursor += entry.inlayHintsCount;
        diagnosticsCursor += entry.diagnosticsCount;
        outlinesCursor += entry.outlinesCount;
        fixItsCursor += entry.fixItsCount;
    }

    uint64_t typeSymbolWriteOffset { header.typeSymbolsOffset };
    copyArray (destination, typeSymbolWriteOffset, typeSymbols);

    if (not stringPool.empty())
        std::memcpy (destination + header.stringPoolOffset, stringPool.data(), stringPool.size());

    if (not versionView.empty())
        std::memcpy (destination + header.stringPoolOffset + header.versionOffset, versionView.data(), versionView.size());

    return bytes;
}

juce::Result SymbolDocument::Builder::writeSwapFile (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family) const
{
    const auto bytes { serialize (hash, languageHash, family) };
    juce::FileOutputStream stream { swapFile };

    if (stream.openedOk())
    {
        stream.write (bytes.getData(), bytes.getSize());
        stream.flush();
    }

    const auto status { stream.getStatus() };

    return status.wasOk() ? status
                          : juce::Result::fail (juce::String (text::Diagnostics::failSwapWrite) + Id::diagnosticSeparator + swapFile.getFullPathName() + Id::diagnosticSeparator + status.getErrorMessage());
}

std::unique_ptr<SymbolDocument> SymbolDocument::Builder::buildInMemory (juce::int64 hash, juce::int64 languageHash, int family) const
{
    return std::make_unique<SymbolDocument> (Access {}, serialize (hash, languageHash, family));
}

SymbolDocument::SymbolDocument (Access, std::unique_ptr<juce::MemoryMappedFile> mappedFile)
    : mapped (std::move (mappedFile)),
      base (static_cast<const uint8_t*> (mapped->getData())),
      byteSize (mapped->getSize())
{
}

SymbolDocument::SymbolDocument (Access, juce::MemoryBlock ownedBlock)
    : owned (std::move (ownedBlock)),
      base (static_cast<const uint8_t*> (owned.getData())),
      byteSize (owned.getSize())
{
}

bool SymbolDocument::isValidBuffer (const uint8_t* data, size_t size, juce::int64 hash, juce::int64 languageHash, int family) noexcept
{
    if (data == nullptr or size < sizeof (Header))
        return false;

    Header header {};
    std::memcpy (&header, data, sizeof (Header));

    if (header.magic != fileMagic
        or header.formatVersion != currentFormatVersion
        or header.totalBytes != static_cast<uint64_t> (size)
        or header.hash != hash
        or header.languageHash != languageHash
        or header.family != family)
        return false;

    const auto totalBytes { header.totalBytes };

    if (not isSectionValid (header.filesOffset, header.filesCount, sizeof (FileEntry), alignof (FileEntry), totalBytes)
        or not isSectionValid (header.symbolsOffset, header.symbolsCount, sizeof (SymbolRecord), alignof (SymbolRecord), totalBytes)
        or not isSectionValid (header.typeSymbolsOffset, header.typeSymbolsCount, sizeof (int64_t), alignof (int64_t), totalBytes)
        or not isSectionValid (header.completionsOffset, header.completionsCount, sizeof (CompletionRecord), alignof (CompletionRecord), totalBytes)
        or not isSectionValid (header.hoversOffset, header.hoversCount, sizeof (HoverEntry), alignof (HoverEntry), totalBytes)
        or not isSectionValid (header.inlayHintsOffset, header.inlayHintsCount, sizeof (InlayHintEntry), alignof (InlayHintEntry), totalBytes)
        or not isSectionValid (header.diagnosticsOffset, header.diagnosticsCount, sizeof (DiagnosticEntry), alignof (DiagnosticEntry), totalBytes)
        or not isSectionValid (header.outlinesOffset, header.outlinesCount, sizeof (OutlineEntry), alignof (OutlineEntry), totalBytes)
        or not isSectionValid (header.fixItsOffset, header.fixItsCount, sizeof (FixItEntry), alignof (FixItEntry), totalBytes)
        or header.stringPoolOffset > totalBytes
        or header.stringPoolSize > totalBytes - header.stringPoolOffset)
        return false;

    if (not isRangeValid (header.versionOffset, header.versionLength, header.stringPoolSize))
        return false;

    const std::string_view storedVersion { reinterpret_cast<const char*> (data + header.stringPoolOffset + header.versionOffset), header.versionLength };

    if (storedVersion.compare (ProjectInfo::versionString) != 0)
        return false;

    const Span<FileEntry> files { reinterpret_cast<const FileEntry*> (data + header.filesOffset), header.filesCount };
    const Span<SymbolRecord> symbols { reinterpret_cast<const SymbolRecord*> (data + header.symbolsOffset), header.symbolsCount };
    const Span<CompletionRecord> completions { reinterpret_cast<const CompletionRecord*> (data + header.completionsOffset), header.completionsCount };
    const Span<HoverEntry> hovers { reinterpret_cast<const HoverEntry*> (data + header.hoversOffset), header.hoversCount };
    const Span<InlayHintEntry> inlayHints { reinterpret_cast<const InlayHintEntry*> (data + header.inlayHintsOffset), header.inlayHintsCount };
    const Span<DiagnosticEntry> diagnostics { reinterpret_cast<const DiagnosticEntry*> (data + header.diagnosticsOffset), header.diagnosticsCount };
    const Span<FixItEntry> fixIts { reinterpret_cast<const FixItEntry*> (data + header.fixItsOffset), header.fixItsCount };

    for (const auto& file : files)
    {
        if (not isRangeValid (file.pathOffset, file.pathLength, header.stringPoolSize)
            or not isRangeValid (file.symbolsStart, file.symbolsCount, header.symbolsCount)
            or not isRangeValid (file.completionsStart, file.completionsCount, header.completionsCount)
            or not isRangeValid (file.hoversStart, file.hoversCount, header.hoversCount)
            or not isRangeValid (file.inlayHintsStart, file.inlayHintsCount, header.inlayHintsCount)
            or not isRangeValid (file.diagnosticsStart, file.diagnosticsCount, header.diagnosticsCount)
            or not isRangeValid (file.outlinesStart, file.outlinesCount, header.outlinesCount)
            or not isRangeValid (file.fixItsStart, file.fixItsCount, header.fixItsCount))
            return false;

        for (uint32_t symbolOffset { 0 }; symbolOffset < file.symbolsCount; ++symbolOffset)
        {
            const auto& record { symbols.at (file.symbolsStart + symbolOffset) };

            if (not isRangeValid (record.typeSymbolsStart, record.typeSymbolsCount, header.typeSymbolsCount))
                return false;

            if (record.hover != jam::LanguageCallbacks::noHover and (record.hover < 0 or static_cast<uint32_t> (record.hover) >= file.hoversCount))
                return false;
        }
    }

    for (const auto& completion : completions)
        if (not isRangeValid (completion.labelOffset, completion.labelLength, header.stringPoolSize)
            or not isRangeValid (completion.snippetOffset, completion.snippetLength, header.stringPoolSize))
            return false;

    for (const auto& hover : hovers)
        if (not isRangeValid (hover.hoverOffset, hover.hoverLength, header.stringPoolSize)
            or not isRangeValid (hover.pathOffset, hover.pathLength, header.stringPoolSize))
            return false;

    for (const auto& inlayHint : inlayHints)
        if (not isRangeValid (inlayHint.labelOffset, inlayHint.labelLength, header.stringPoolSize))
            return false;

    for (const auto& diagnostic : diagnostics)
        if (not isRangeValid (diagnostic.messageOffset, diagnostic.messageLength, header.stringPoolSize))
            return false;

    for (const auto& fixIt : fixIts)
        if (not isRangeValid (fixIt.textOffset, fixIt.textLength, header.stringPoolSize))
            return false;

    return true;
}

std::unique_ptr<SymbolDocument> SymbolDocument::open (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family)
{
    auto mapped { std::make_unique<juce::MemoryMappedFile> (swapFile, juce::MemoryMappedFile::readOnly) };

    if (mapped->getData() != nullptr
        and isValidBuffer (static_cast<const uint8_t*> (mapped->getData()), mapped->getSize(), hash, languageHash, family))
        return std::make_unique<SymbolDocument> (Access {}, std::move (mapped));

    return nullptr;
}

SymbolDocument::Header SymbolDocument::getHeader() const noexcept
{
    Header header {};
    std::memcpy (&header, base, sizeof (Header));
    return header;
}

juce::int64 SymbolDocument::getHash() const noexcept
{
    return getHeader().hash;
}

SymbolDocument::Span<SymbolDocument::FileEntry> SymbolDocument::getFiles() const noexcept
{
    const auto header { getHeader() };

    return { getTable<FileEntry> (header.filesOffset), header.filesCount };
}

SymbolDocument::Span<SymbolDocument::SymbolRecord> SymbolDocument::getSymbols (const FileEntry& file) const noexcept
{
    const auto header { getHeader() };

    return { getTable<SymbolRecord> (header.symbolsOffset) + file.symbolsStart, file.symbolsCount };
}

SymbolDocument::Span<int64_t> SymbolDocument::getTypeSymbols (const SymbolRecord& record) const noexcept
{
    const auto header { getHeader() };

    return { getTable<int64_t> (header.typeSymbolsOffset) + record.typeSymbolsStart, record.typeSymbolsCount };
}

SymbolDocument::Span<SymbolDocument::CompletionRecord> SymbolDocument::getCompletions (const FileEntry& file) const noexcept
{
    const auto header { getHeader() };

    return { getTable<CompletionRecord> (header.completionsOffset) + file.completionsStart, file.completionsCount };
}

SymbolDocument::Span<SymbolDocument::HoverEntry> SymbolDocument::getHovers (const FileEntry& file) const noexcept
{
    const auto header { getHeader() };

    return { getTable<HoverEntry> (header.hoversOffset) + file.hoversStart, file.hoversCount };
}

SymbolDocument::Span<SymbolDocument::InlayHintEntry> SymbolDocument::getInlayHints (const FileEntry& file) const noexcept
{
    const auto header { getHeader() };

    return { getTable<InlayHintEntry> (header.inlayHintsOffset) + file.inlayHintsStart, file.inlayHintsCount };
}

SymbolDocument::Span<SymbolDocument::DiagnosticEntry> SymbolDocument::getDiagnostics (const FileEntry& file) const noexcept
{
    const auto header { getHeader() };

    return { getTable<DiagnosticEntry> (header.diagnosticsOffset) + file.diagnosticsStart, file.diagnosticsCount };
}

SymbolDocument::Span<SymbolDocument::OutlineEntry> SymbolDocument::getOutlines (const FileEntry& file) const noexcept
{
    const auto header { getHeader() };

    return { getTable<OutlineEntry> (header.outlinesOffset) + file.outlinesStart, file.outlinesCount };
}

SymbolDocument::Span<SymbolDocument::FixItEntry> SymbolDocument::getFixIts (const FileEntry& file) const noexcept
{
    const auto header { getHeader() };

    return { getTable<FixItEntry> (header.fixItsOffset) + file.fixItsStart, file.fixItsCount };
}

std::string_view SymbolDocument::getString (uint32_t offset, uint32_t length) const noexcept
{
    const auto header { getHeader() };

    return std::string_view { reinterpret_cast<const char*> (base + header.stringPoolOffset + offset), length };
}
