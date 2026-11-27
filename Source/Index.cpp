#include "Index.h"

Index::Index (LINGModel& modelToUse)
    : model (modelToUse)
{
}

bool Index::isUnitRetained (int unitPathIndex, const juce::ValueTree& files) const
{
    const auto unitPath { paths.at (unitPathIndex) };

    if (const auto rowEntry { pathRows.find (unitPath) }; rowEntry != pathRows.end())
    {
        const auto& [entryPath, unitRow] { *rowEntry };

        if (files.getChild (unitRow).hasProperty (Id::arguments))
        {
            return std::none_of (symbolFiles.begin(), symbolFiles.end(), [this, unitPathIndex, newUnitRow = unitRow] (const auto& entry)
            {
                const auto& [boundPathIndex, unitFile] { entry };
                const auto& [boundUnitPathIndex, file] { unitFile };
                const auto ownerRow { unitOwners->getRow (paths.at (boundPathIndex)) };

                return boundUnitPathIndex == unitPathIndex and ownerRow != UnitOwners::noRow and ownerRow != newUnitRow;
            });
        }
    }

    return false;
}

void Index::removeDocuments (const juce::ValueTree& files)
{
    for (auto entry { sourceHashes.begin() }; entry != sourceHashes.end();)
    {
        const auto& [pathIndex, sourceHash] { *entry };

        if (pathRows.contains (paths.at (pathIndex)))
        {
            ++entry;
        }
        else
        {
            documents.erase (pathIndex);
            removeCachedDocument (pathIndex);
            entry = sourceHashes.erase (entry);
        }
    }

    for (auto entry { symbolDocuments.begin() }; entry != symbolDocuments.end();)
    {
        const auto& [unitPathIndex, symbolDocument] { *entry };

        if (isUnitRetained (unitPathIndex, files))
        {
            ++entry;
        }
        else
        {
            removeBindings (unitPathIndex);
            entry = symbolDocuments.erase (entry);
        }
    }
}

std::unique_ptr<jam::Document> Index::readSwapFile (const juce::File& swapFile, std::string_view sourceText, const std::shared_ptr<const Language>& language, juce::int64 hash, juce::int64 languageHash, int family)
{
    const juce::MemoryMappedFile mapped { swapFile, juce::MemoryMappedFile::readOnly };
    std::unique_ptr<jam::Document> document;

    if (mapped.getData() != nullptr)
    {
        if (auto read { language->read (sourceText, std::string_view { static_cast<const char*> (mapped.getData()), mapped.getSize() }) }; read != nullptr)
        {
            if (auto stored { jam::Document::getValid (std::move (read), SwapValidator {}) }; stored != nullptr and *stored->getRoot()->get<juce::int64> (Id::hash) == hash and *stored->getRoot()->get<juce::int64> (Id::language) == languageHash and *stored->getRoot()->get<int> (Id::family) == family)
                document = std::move (stored);
        }
    }

    return document;
}

std::unique_ptr<SymbolDocument> Index::readSwapFile (const juce::File& swapFile, juce::int64 hash, juce::int64 languageHash, int family)
{
    return SymbolDocument::open (swapFile, hash, languageHash, family);
}

void Index::addStamp (jam::Document& document, juce::int64 hash, juce::int64 languageHash, int family)
{
    document.getRoot()->add<juce::int64> (Id::hash, hash);
    document.getRoot()->add<juce::int64> (Id::language, languageHash);
    document.getRoot()->add<int> (Id::family, family);
    document.getRoot()->add<juce::String> (Id::version, ProjectInfo::versionString);
}

juce::Result Index::writeSwapFile (const juce::File& swapFile, jam::Document& document, juce::int64 hash, juce::int64 languageHash, int family)
{
    addStamp (document, hash, languageHash, family);

    juce::TemporaryFile temporary { swapFile };
    auto result { juce::Result::ok() };

    {
        juce::FileOutputStream stream { temporary.getFile() };

        if (stream.openedOk())
        {
            document.write (stream);
            stream.flush();
        }

        result = stream.getStatus();
    }

    if (not result.wasOk())
        return juce::Result::fail (juce::String (text::Diagnostics::failSwapWrite) + Id::diagnosticSeparator + swapFile.getFullPathName() + Id::diagnosticSeparator + result.getErrorMessage());

    if (not temporary.overwriteTargetFileWithTemporary())
        return juce::Result::fail (juce::String (text::Diagnostics::failSwapWrite) + Id::diagnosticSeparator + swapFile.getFullPathName());

    return result;
}

void Index::removeCachedDocument (int pathIndex) const
{
    if (const auto position { cachedDocuments.indexOf (pathIndex) }; position >= 0)
        cachedDocuments.remove (position);
}

Index::DocumentScope::DocumentScope (const Index& indexToUse)
    : index (indexToUse)
{
    ++index.evictionLockDepth;
}

Index::DocumentScope::~DocumentScope()
{
    jassert (index.evictionLockDepth > 0);
    --index.evictionLockDepth;
    index.evictCachedDocuments();
}

void Index::evictCachedDocuments() const
{
    if (evictionLockDepth == 0)
    {
        while (cachedDocuments.size() > maxCachedDocuments)
        {
            documents.erase (cachedDocuments.first());
            cachedDocuments.remove (0);
        }
    }
}

std::unique_ptr<jam::Document> Index::getSwapDocument (int pathIndex, juce::int64 storedHash) const
{
    const auto path { paths.at (pathIndex) };

    if (const auto rowEntry { pathRows.find (path) }; rowEntry != pathRows.end())
    {
        const auto& [rowPath, rowIndex] { *rowEntry };
        const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
        const auto row { files.getChild (rowIndex) };
        const auto family { static_cast<int> (row[Id::family]) };

        if (languages.contains (family))
        {
            const auto languageRow { model.state.getChildWithProperty (Id::family, family) };
            const auto languageHash { static_cast<juce::int64> (languageRow[Id::hash]) };
            const juce::File file { path };
            const auto swapFile { model.getSwapFile (file, files::lingSwap) };

            if (file.existsAsFile())
            {
                const auto diskText { file.loadFileAsString() };
                const auto bytes { jam::Strings::getView (diskText) };
                const auto decoded { bytes.find (static_cast<char> (Chars::escapeCharacter)) == std::string_view::npos ? std::string {} : jam::AnsiDocument::getText (bytes) };
                const auto view { decoded.empty() ? bytes : std::string_view { decoded } };

                return readSwapFile (swapFile, view, languages.at (family), storedHash, languageHash, family);
            }
        }
    }

    return nullptr;
}

const jam::Document* Index::getOrCreateDocument (int pathIndex) const
{
    if (const auto stored { documents.find (pathIndex) }; stored != documents.end())
    {
        const auto& [documentIndex, document] { *stored };

        if (const auto position { cachedDocuments.indexOf (pathIndex) }; position >= 0)
        {
            cachedDocuments.remove (position);
            cachedDocuments.add (pathIndex);
        }

        return document.get();
    }

    if (const auto hashEntry { sourceHashes.find (pathIndex) }; hashEntry != sourceHashes.end())
    {
        const auto& [hashedPathIndex, storedHash] { *hashEntry };

        if (auto loaded { getSwapDocument (pathIndex, storedHash) }; loaded != nullptr)
        {
            documents.addOrReplace (pathIndex, std::move (loaded));
            cachedDocuments.add (pathIndex);
            evictCachedDocuments();
            return documents.at (pathIndex).get();
        }
    }

    return nullptr;
}

void Index::addDocument (const juce::String& path, std::unique_ptr<jam::Document> document, juce::int64 sourceHash)
{
    const auto pathIndex { getOrCreatePathIndex (path) };

    removeCachedDocument (pathIndex);

    if (document != nullptr)
    {
        sourceHashes.addOrReplace (pathIndex, sourceHash);
        documents.addOrReplace (pathIndex, std::move (document));
    }
    else
    {
        sourceHashes.erase (pathIndex);
        documents.erase (pathIndex);
    }
}

void Index::addSourceHash (const juce::String& path, juce::int64 sourceHash)
{
    const auto pathIndex { getOrCreatePathIndex (path) };

    sourceHashes.addOrReplace (pathIndex, sourceHash);
    documents.erase (pathIndex);
    removeCachedDocument (pathIndex);
}

int Index::getOrCreatePathIndex (const juce::String& path)
{
    if (const auto pathIndex { paths.indexOf (path, false, 0) }; pathIndex >= 0)
        return pathIndex;

    paths.add (path);

    const auto newPathIndex { paths.size() - 1 };
    const auto position { std::partition_point (pathOrder.begin(), pathOrder.end(), [this, newPathIndex] (int orderedPathIndex)
    {
        return paths.getView (orderedPathIndex).compare (paths.getView (newPathIndex)) < 0;
    }) };

    pathOrder.insert (static_cast<int> (std::distance (pathOrder.begin(), position)), newPathIndex);
    return newPathIndex;
}

void Index::addBindings (int rowIndex, std::unique_ptr<SymbolDocument> symbolDocument)
{
#if JAM_USING_PROFILER
    const jam::Profiler::Zone zone { Id::addSymbols };
#endif

    const auto row { model.state.getChildWithName (jam::Format::toType (Id::files)).getChild (rowIndex) };
    const auto unitPathIndex { getOrCreatePathIndex (row[Id::path].toString()) };

    removeBindings (unitPathIndex);

    const juce::File directory { row[Id::directory].toString() };

    for (const auto& file : symbolDocument->getFiles())
    {
        const auto path { directory.getChildFile (jam::Strings::fromView (symbolDocument->getString (file.pathOffset, file.pathLength))).getFullPathName() };
        const auto pathIndex { getOrCreatePathIndex (path) };
        const auto ownerRow { unitOwners->getRow (path) };
        const bool isOwner { ownerRow != UnitOwners::noRow ? ownerRow == rowIndex : not symbolFiles.contains (pathIndex) };

        if (isOwner)
        {
            if (symbolFiles.contains (pathIndex))
                removeSymbols (pathIndex);
            symbolFiles.addOrReplace (pathIndex, std::make_pair (unitPathIndex, &file));
            addSymbols (pathIndex, *symbolDocument, file);
        }
    }

    symbolDocuments.addOrReplace (unitPathIndex, std::move (symbolDocument));
}

juce::Result Index::addBindings (int rowIndex, const juce::TemporaryFile& temporary, juce::int64 hash, juce::int64 languageHash, int family)
{
    const auto row { model.state.getChildWithName (jam::Format::toType (Id::files)).getChild (rowIndex) };
    const auto unitPathIndex { getOrCreatePathIndex (row[Id::path].toString()) };

    removeBindings (unitPathIndex);
    symbolDocuments.erase (unitPathIndex);

    if (temporary.overwriteTargetFileWithTemporary())
    {
        if (auto symbolDocument { readSwapFile (temporary.getTargetFile(), hash, languageHash, family) }; symbolDocument != nullptr)
        {
            addBindings (rowIndex, std::move (symbolDocument));
            return juce::Result::ok();
        }
    }

    return juce::Result::fail (juce::String (text::Diagnostics::failSwapWrite) + Id::diagnosticSeparator + temporary.getTargetFile().getFullPathName());
}

void Index::addPathRows (const juce::ValueTree& files)
{
    pathRows.clear();
    stemRows.clear();

    for (int rowIndex { 0 }; rowIndex < files.getNumChildren(); ++rowIndex)
    {
        const auto rowPath { files.getChild (rowIndex)[Id::path].toString() };
        const auto [stemEntry, isStemAdded] { stemRows.try_emplace (juce::File { rowPath }.getFileNameWithoutExtension()) };
        auto& [stem, stemRowIndices] { *stemEntry };

        pathRows.try_emplace (rowPath, rowIndex);
        stemRowIndices.add (rowIndex);
    }
}

void Index::addUnitRows (const juce::ValueTree& files)
{
    auto owners { std::make_shared<UnitOwners>() };

    for (const auto& root : model.state.getChildWithName (jam::Format::toType (Id::roots)))
        owners->roots.add (juce::File { root[Id::path].toString() });

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (const auto unit { files.getChild (unitIndex) }; unit.hasProperty (Id::arguments))
            owners->addIfAbsent (unit[Id::path].toString(), unitIndex);

    jam::Array<int> prerequisiteRows;

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
    {
        const auto* prerequisites { files.getChild (unitIndex)[Id::prerequisites].getArray() };

        if (prerequisites != nullptr)
            for (const auto& prerequisite : *prerequisites)
                prerequisiteRows.addIfNotAlreadyThere (static_cast<int> (prerequisite));
    }

    const auto addLocalDependencyRows { [&owners, &files] (int unitIndex)
    {
        const auto unit { files.getChild (unitIndex) };
        const auto unitDirectory { juce::File { unit[Id::path].toString() }.getParentDirectory() };

        if (const auto* dependencies { unit[Id::dependencies].getArray() }; dependencies != nullptr)
            for (const auto& dependency : *dependencies)
                if (const auto dependencyPath { files.getChild (static_cast<int> (dependency))[Id::path].toString() }; juce::File { dependencyPath }.isAChildOf (unitDirectory))
                    owners->addIfAbsent (dependencyPath, unitIndex);
    } };

    const auto addDependencyRows { [&owners, &files] (int unitIndex)
    {
        const auto* dependencies { files.getChild (unitIndex)[Id::dependencies].getArray() };

        if (dependencies != nullptr)
            for (const auto& dependency : *dependencies)
                owners->addIfAbsent (files.getChild (static_cast<int> (dependency))[Id::path].toString(), unitIndex);
    } };

    const auto hasPrerequisites { [&files] (int unitIndex)
    {
        const auto* prerequisites { files.getChild (unitIndex)[Id::prerequisites].getArray() };

        return prerequisites != nullptr and not prerequisites->isEmpty();
    } };

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (files.getChild (unitIndex).hasProperty (Id::arguments))
            addLocalDependencyRows (unitIndex);

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (files.getChild (unitIndex).hasProperty (Id::arguments) and not prerequisiteRows.contains (unitIndex) and not hasPrerequisites (unitIndex))
            addDependencyRows (unitIndex);

    for (const auto prerequisite : prerequisiteRows)
        addDependencyRows (prerequisite);

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (files.getChild (unitIndex).hasProperty (Id::arguments) and hasPrerequisites (unitIndex))
            addDependencyRows (unitIndex);

    unitOwners = std::move (owners);
}

void Index::UnitOwners::addIfAbsent (const juce::String& path, int unitRow)
{
    if (paths.indexOf (path, false, 0) < 0)
    {
        paths.add (path);
        rows.add (unitRow);
    }
}

int Index::UnitOwners::getRow (juce::StringRef path) const
{
    const auto index { paths.indexOf (path, false, 0) };

    return index >= 0 ? rows.at (index) : noRow;
}

bool Index::UnitOwners::isForeign (const char* recordPath, int unitRow) const
{
#if JUCE_WINDOWS
    const juce::File recordFile { juce::String::fromUTF8 (recordPath) };
    const auto ownerRow { getRow (recordFile.getFullPathName()) };

    return ownerRow != noRow ? ownerRow != unitRow : not LINGModel::isWithinRoots (roots, recordFile);
#else
    const auto ownerRow { getRow (juce::StringRef (recordPath)) };

    return ownerRow != noRow ? ownerRow != unitRow : not LINGModel::isWithinRoots (roots, juce::File { juce::String::fromUTF8 (recordPath) });
#endif
}

std::pair<const jam::Document::Element*, int> Index::getElement (const jam::Document& document, int line, int column)
{
    const auto lineCount { static_cast<int> (document.getLineNumber (document.getSource().size())) };

    if (line <= lineCount) return document.getNameToken (document.getByteOffset (static_cast<uint32_t> (line), static_cast<uint32_t> (column)));

    return { nullptr, -1 };
}

std::pair<const jam::Document::Element*, int> Index::getElement (const std::pair<int, const SymbolDocument::SymbolRecord*>& occurrence) const
{
    const auto& [pathIndex, record] { occurrence };

    if (const auto* document { getOrCreateDocument (pathIndex) }; document != nullptr)
        return getElement (*document, record->line, record->column);

    return { nullptr, -1 };
}

int Index::getCharacter (const jam::Document& document, size_t offset, std::string_view encoding)
{
    const auto lineStart { document.getByteOffset (document.getLineNumber (offset), 1) };

    if (encoding.compare (lsp::PositionEncodingType::UTF8) == 0) return static_cast<int> (offset - lineStart);

    juce::CharPointer_UTF8 cursor { document.getSource().data() + lineStart };
    const juce::CharPointer_UTF8 end { document.getSource().data() + offset };
    size_t unitCount { 0 };

    while (cursor < end)
        unitCount += juce::CharPointer_UTF16::getBytesRequiredFor (cursor.getAndAdvance()) / sizeof (juce::CharPointer_UTF16::CharType);

    return static_cast<int> (unitCount);
}

int Index::getByteColumn (const jam::Document& document, int line, int column, std::string_view encoding)
{
    const auto lineCount { static_cast<int> (document.getLineNumber (document.getSource().size())) };
    int byteColumn { column };

    if (encoding.compare (lsp::PositionEncodingType::UTF8) != 0 and line <= lineCount)
    {
        const auto* lineStart { document.getSource().data() + document.getByteOffset (static_cast<uint32_t> (line), 1) };
        juce::CharPointer_UTF8 cursor { lineStart };
        const juce::CharPointer_UTF8 end { document.getSource().data() + document.getSource().size() };
        const auto targetUnitCount { static_cast<size_t> (column - 1) };
        size_t unitCount { 0 };

        while (unitCount < targetUnitCount and cursor < end)
            unitCount += juce::CharPointer_UTF16::getBytesRequiredFor (cursor.getAndAdvance()) / sizeof (juce::CharPointer_UTF16::CharType);

        byteColumn = static_cast<int> (cursor.getAddress() - lineStart) + 1;
    }

    return byteColumn;
}

bool Index::hasRole (const jam::Document& document, const jam::Document::Element& element, int role) const
{
    const auto family { *document.getRoot()->get<int> (Id::family) };

    return languages.at (family)->hasRole (element, role);
}

int Index::getSymbolType (const jam::Document& document, const jam::Document::Element& element) const
{
    const auto family { *document.getRoot()->get<int> (Id::family) };

    return languages.at (family)->getSymbolType (element);
}

void Index::removeBindings (int unitPathIndex)
{
    for (auto entry { symbolFiles.begin() }; entry != symbolFiles.end();)
    {
        const auto& [pathIndex, unitFile] { *entry };
        const auto& [boundUnitPathIndex, file] { unitFile };

        if (boundUnitPathIndex == unitPathIndex)
        {
            removeSymbols (pathIndex);
            entry = symbolFiles.erase (entry);
        }
        else
        {
            ++entry;
        }
    }
}

void Index::removeSymbols (int pathIndex)
{
    const auto& [unitPathIndex, file] { symbolFiles.at (pathIndex) };

    removeCompletions (pathIndex, *symbolDocuments.at (unitPathIndex), *file);

    jam::HashSet<juce::int64> removedSymbols;

    for (const auto& record : symbolDocuments.at (unitPathIndex)->getSymbols (*file))
    {
        const auto symbol { record.symbol };

        if (const auto [symbolEntry, isInserted] { removedSymbols.insert (symbol) }; isInserted)
        {
            removeOccurrences (declarations, symbol, pathIndex);
            removeOccurrences (references, symbol, pathIndex);
        }
    }
}

void Index::removeOccurrences (jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& symbols,
                               juce::int64 symbol,
                               int pathIndex)
{
    if (auto stored { symbols.find (symbol) }; stored != symbols.end())
    {
        auto& [storedSymbol, occurrences] { *stored };
        const auto pathRange { getPathRange (occurrences, pathIndex) };

        occurrences.removeRange (pathRange.getStart(), pathRange.getLength());

        if (occurrences.isEmpty())
            symbols.erase (symbol);
    }
}

void Index::removeCompletions (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file)
{
    jam::HashSet<juce::int64> removedScopes;

    for (const auto& completion : symbolDocument.getCompletions (file))
    {
        const auto scopeSymbol { completion.scope };

        if (const auto [scopeEntry, isInserted] { removedScopes.insert (scopeSymbol) }; isInserted)
        {
            if (const auto entry { completions.find (scopeSymbol) }; entry != completions.end())
            {
                auto& [entrySymbol, scopeCompletions] { *entry };
                const auto pathRange { getPathRange (scopeCompletions, pathIndex) };

                scopeCompletions.removeRange (pathRange.getStart(), pathRange.getLength());

                if (scopeCompletions.isEmpty())
                    completions.erase (scopeSymbol);
            }
        }
    }
}

void Index::addCompletions (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file)
{
    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::CompletionRecord*>>> completionBlocks;

    for (const auto& completion : symbolDocument.getCompletions (file))
    {
        const auto [blockEntry, isBlockAdded] { completionBlocks.try_emplace (completion.scope) };
        auto& [blockScope, block] { *blockEntry };

        block.add ({ pathIndex, &completion });
    }

    for (const auto& [scopeSymbol, block] : completionBlocks)
    {
        const auto [scopeEntry, isScopeAdded] { completions.try_emplace (scopeSymbol) };
        auto& [entryScope, scopeCompletions] { *scopeEntry };

        scopeCompletions.insertArray (getPathRange (scopeCompletions, pathIndex).getStart(), block.begin(), block.size());
    }
}

void Index::addOccurrences (jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& symbols,
                            int pathIndex,
                            const jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>>& blocks)
{
    for (const auto& [symbol, block] : blocks)
    {
        const auto [occurrencesEntry, isOccurrencesAdded] { symbols.try_emplace (symbol) };
        auto& [occurrencesSymbol, occurrences] { *occurrencesEntry };

        occurrences.insertArray (getPathRange (occurrences, pathIndex).getStart(), block.begin(), block.size());
    }
}

bool Index::isRecordBefore (const SymbolDocument::SymbolRecord& record, const SymbolDocument::SymbolRecord& otherRecord)
{
    return std::make_pair (record.line, record.column) < std::make_pair (otherRecord.line, otherRecord.column);
}

bool Index::isOccurrenceBefore (int leftPathIndex, const SymbolDocument::SymbolRecord& left, int rightPathIndex, const SymbolDocument::SymbolRecord& right) const
{
    const auto order { paths.getView (leftPathIndex).compare (paths.getView (rightPathIndex)) };

    return order != 0 ? order < 0 : isRecordBefore (left, right);
}

const SymbolDocument& Index::getSymbolDocument (int pathIndex) const
{
    const auto& [unitPathIndex, file] { symbolFiles.at (pathIndex) };

    return *symbolDocuments.at (unitPathIndex);
}

void Index::addSymbols (int pathIndex, const SymbolDocument& symbolDocument, const SymbolDocument::FileEntry& file)
{
    addCompletions (pathIndex, symbolDocument, file);

    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>> declarationBlocks;
    jam::HashMap<juce::int64, jam::Array<std::pair<int, const SymbolDocument::SymbolRecord*>>> referenceBlocks;

    for (const auto& record : symbolDocument.getSymbols (file))
    {
        const auto symbol { record.symbol };
        auto& blocks { (record.role & jam::LanguageRole::declaration) != 0 ? declarationBlocks : referenceBlocks };
        const auto [blockEntry, isBlockAdded] { blocks.try_emplace (symbol) };
        auto& [blockSymbol, block] { *blockEntry };

        if (block.isEmpty())
        {
            block.add ({ pathIndex, &record });
        }
        else
        {
            const auto& [lastPathIndex, lastRecord] { block.last() };

            if (isRecordBefore (*lastRecord, record))
                block.add ({ pathIndex, &record });
        }
    }

    addOccurrences (declarations, pathIndex, declarationBlocks);
    addOccurrences (references, pathIndex, referenceBlocks);
}

void Index::setDocuments()
{
#if JAM_USING_PROFILER
    const jam::Profiler::Zone zone { Id::setDocuments };
#endif

    const juce::var count { static_cast<int> (sourceHashes.size()) };

    if (model.state[Id::documents] == count)
        model.state.sendPropertyChangeMessage (Id::documents);
    else
        model.state.setProperty (Id::documents, count, nullptr);
}
