#include "Index.h"

Index::Index (LINGModel& modelToUse)
    : model (modelToUse)
{
}

void Index::removeDocuments (const juce::ValueTree& files)
{
    for (auto entry { documents.begin() }; entry != documents.end();)
    {
        const auto& [pathIndex, document] { *entry };

        if (files.getChildWithProperty (Id::path, paths.at (pathIndex)).isValid())
        {
            ++entry;
        }
        else
        {
            entry = documents.erase (entry);
        }
    }

    for (auto entry { symbolDocuments.begin() }; entry != symbolDocuments.end();)
    {
        const auto& [rowIndex, symbolDocument] { *entry };

        if (rowIndex < files.getNumChildren() and files.getChild (rowIndex).hasProperty (Id::arguments))
        {
            ++entry;
        }
        else
        {
            removeBindings (rowIndex);
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
    const juce::MemoryMappedFile mapped { swapFile, juce::MemoryMappedFile::readOnly };
    std::unique_ptr<SymbolDocument> document;

    if (mapped.getData() != nullptr)
    {
        if (auto read { SymbolDocument::read (std::string_view { static_cast<const char*> (mapped.getData()), mapped.getSize() }) }; read != nullptr)
        {
            if (auto stored { jam::Document::getValid (std::move (read), SwapValidator {}) }; stored != nullptr and *stored->getRoot()->get<juce::int64> (Id::hash) == hash and *stored->getRoot()->get<juce::int64> (Id::language) == languageHash and *stored->getRoot()->get<int> (Id::family) == family)
                document = std::move (stored);
        }
    }

    return document;
}

void Index::addStamp (jam::Document& document, juce::int64 hash, juce::int64 languageHash, int family)
{
    document.getRoot()->add<juce::int64> (Id::hash, hash);
    document.getRoot()->add<juce::int64> (Id::language, languageHash);
    document.getRoot()->add<int> (Id::family, family);
    document.getRoot()->add<juce::String> (Id::version, ProjectInfo::versionString);
}

void Index::writeSwapFile (const juce::File& swapFile, jam::Document& document, juce::int64 hash, juce::int64 languageHash, int family)
{
    addStamp (document, hash, languageHash, family);

    juce::TemporaryFile temporary { swapFile };

    {
        juce::FileOutputStream stream { temporary.getFile() };
        document.write (stream);
    }

    temporary.overwriteTargetFileWithTemporary();
}

void Index::addDocument (const juce::String& path, std::unique_ptr<jam::Document> document)
{
    const auto pathIndex { getOrCreatePathIndex (path) };

    if (document != nullptr)
    {
        documents.addOrReplace (pathIndex, std::move (document));
    }
    else
    {
        documents.erase (pathIndex);
    }
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

    removeBindings (rowIndex);

    const auto row { model.state.getChildWithName (jam::Format::toType (Id::files)).getChild (rowIndex) };
    const juce::File directory { row[Id::directory].toString() };

    for (const auto* file : *symbolDocument)
    {
        const auto path { directory.getChildFile (*file->get<juce::String> (Id::path)).getFullPathName() };
        const auto pathIndex { getOrCreatePathIndex (path) };
        const bool isOwner { unitRows.contains (path) ? unitRows.at (path) == rowIndex : not symbolFiles.contains (pathIndex) };

        if (isOwner)
        {
            if (symbolFiles.contains (pathIndex))
                removeSymbols (pathIndex);
            symbolFiles.addOrReplace (pathIndex, std::make_pair (rowIndex, file));
            addSymbols (pathIndex, *file);
        }
    }

    symbolDocuments.addOrReplace (rowIndex, std::move (symbolDocument));
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
    unitRows.clear();

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (const auto unit { files.getChild (unitIndex) }; unit.hasProperty (Id::arguments))
            unitRows.try_emplace (unit[Id::path].toString(), unitIndex);

    jam::Array<int> prerequisiteRows;

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
    {
        const auto* prerequisites { files.getChild (unitIndex)[Id::prerequisites].getArray() };

        if (prerequisites != nullptr)
            for (const auto& prerequisite : *prerequisites)
                prerequisiteRows.addIfNotAlreadyThere (static_cast<int> (prerequisite));
    }

    const auto addDependencyRows { [this, &files] (int unitIndex)
    {
        const auto* dependencies { files.getChild (unitIndex)[Id::dependencies].getArray() };

        if (dependencies != nullptr)
            for (const auto& dependency : *dependencies)
                unitRows.try_emplace (files.getChild (static_cast<int> (dependency))[Id::path].toString(), unitIndex);
    } };

    const auto hasPrerequisites { [&files] (int unitIndex)
    {
        const auto* prerequisites { files.getChild (unitIndex)[Id::prerequisites].getArray() };

        return prerequisites != nullptr and not prerequisites->isEmpty();
    } };

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (files.getChild (unitIndex).hasProperty (Id::arguments) and not prerequisiteRows.contains (unitIndex) and not hasPrerequisites (unitIndex))
            addDependencyRows (unitIndex);

    for (const auto prerequisite : prerequisiteRows)
        addDependencyRows (prerequisite);

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (files.getChild (unitIndex).hasProperty (Id::arguments) and hasPrerequisites (unitIndex))
            addDependencyRows (unitIndex);
}

std::pair<const jam::Document::Element*, int> Index::getElement (const jam::Document& document, int line, int column)
{
    const auto lineCount { static_cast<int> (document.getLineNumber (static_cast<uint32_t> (document.getSource().size()))) };

    if (line <= lineCount) return document.getNameToken (document.getByteOffset (static_cast<uint32_t> (line), static_cast<uint32_t> (column)));

    return { nullptr, -1 };
}

std::pair<const jam::Document::Element*, int> Index::getElement (const std::pair<int, const jam::Document::Element*>& occurrence) const
{
    const auto& [pathIndex, record] { occurrence };

    if (const auto stored { documents.find (pathIndex) }; stored != documents.end())
    {
        const auto& [documentIndex, document] { *stored };

        return getElement (*document, *record->get<int> (Id::line), *record->get<int> (Id::column));
    }

    return { nullptr, -1 };
}

int Index::getCharacter (const jam::Document& document, uint32_t offset, std::string_view encoding)
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
    const auto lineCount { static_cast<int> (document.getLineNumber (static_cast<uint32_t> (document.getSource().size()))) };
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

void Index::removeBindings (int rowIndex)
{
    for (auto entry { symbolFiles.begin() }; entry != symbolFiles.end();)
    {
        const auto& [pathIndex, unitFile] { *entry };
        const auto& [unitRow, file] { unitFile };

        if (unitRow == rowIndex)
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
    const auto& [unitRow, file] { symbolFiles.at (pathIndex) };

    removeCompletions (pathIndex, *file);

    jam::HashSet<juce::int64> removedSymbols;

    for (const auto* record : records.at (pathIndex))
    {
        const auto symbol { *record->get<juce::int64> (Id::symbol) };

        if (const auto [symbolEntry, isInserted] { removedSymbols.insert (symbol) }; isInserted)
        {
            removeOccurrences (declarations, symbol, pathIndex);
            removeOccurrences (references, symbol, pathIndex);
        }
    }

    records.erase (pathIndex);
}

void Index::removeOccurrences (jam::HashMap<juce::int64, jam::Array<std::pair<int, const jam::Document::Element*>>>& symbols,
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

void Index::removeCompletions (int pathIndex, const jam::Document::Element& file)
{
    SymbolDocument::applyToCompletions (file, [this, pathIndex] (const jam::Document::Element& completion)
    {
        const auto scopeSymbol { *completion.get<juce::int64> (Id::scope) };

        if (const auto entry { completions.find (scopeSymbol) }; entry != completions.end())
        {
            auto& [entrySymbol, scopeCompletions] { *entry };
            const auto tail { std::remove_if (scopeCompletions.begin(), scopeCompletions.end(), [pathIndex] (const std::pair<int, const jam::Document::Element*>& scopeCompletion)
            {
                const auto& [completionPathIndex, completionElement] { scopeCompletion };

                return completionPathIndex == pathIndex;
            }) };

            scopeCompletions.removeRange (static_cast<int> (std::distance (scopeCompletions.begin(), tail)), static_cast<int> (std::distance (tail, scopeCompletions.end())));

            if (scopeCompletions.isEmpty())
                completions.erase (scopeSymbol);
        }
    });
}

void Index::addCompletions (int pathIndex, const jam::Document::Element& file)
{
    SymbolDocument::applyToCompletions (file, [this, pathIndex] (const jam::Document::Element& completion)
    {
        const auto [scopeEntry, isScopeAdded] { completions.try_emplace (*completion.get<juce::int64> (Id::scope)) };
        auto& [scopeSymbol, scopeCompletions] { *scopeEntry };

        scopeCompletions.add ({ pathIndex, &completion });
    });
}

void Index::addOccurrences (jam::HashMap<juce::int64, jam::Array<std::pair<int, const jam::Document::Element*>>>& symbols,
                            int pathIndex,
                            const jam::HashMap<juce::int64, jam::Array<std::pair<int, const jam::Document::Element*>>>& blocks)
{
    for (const auto& [symbol, block] : blocks)
    {
        const auto [occurrencesEntry, isOccurrencesAdded] { symbols.try_emplace (symbol) };
        auto& [occurrencesSymbol, occurrences] { *occurrencesEntry };

        occurrences.insertArray (getPathRange (occurrences, pathIndex).getStart(), block.begin(), block.size());
    }
}

bool Index::isRecordBefore (const jam::Document::Element& record, const jam::Document::Element& otherRecord)
{
    return std::make_pair (*record.get<int> (Id::line), *record.get<int> (Id::column)) < std::make_pair (*otherRecord.get<int> (Id::line), *otherRecord.get<int> (Id::column));
}

bool Index::isOccurrenceBefore (int leftPathIndex, const jam::Document::Element& left, int rightPathIndex, const jam::Document::Element& right) const
{
    const auto order { paths.getView (leftPathIndex).compare (paths.getView (rightPathIndex)) };

    return order != 0 ? order < 0 : isRecordBefore (left, right);
}

juce::Range<int> Index::getPathRange (const jam::Array<std::pair<int, const jam::Document::Element*>>& occurrences, int pathIndex) const
{
    const auto isPathBefore { [this] (const std::pair<int, const jam::Document::Element*>& left, const std::pair<int, const jam::Document::Element*>& right)
    {
        const auto& [leftPathIndex, leftRecord] { left };
        const auto& [rightPathIndex, rightRecord] { right };

        return paths.getView (leftPathIndex).compare (paths.getView (rightPathIndex)) < 0;
    } };
    const std::pair<int, const jam::Document::Element*> probe { pathIndex, nullptr };
    const auto [first, last] { std::equal_range (occurrences.begin(), occurrences.end(), probe, isPathBefore) };

    return { static_cast<int> (std::distance (occurrences.begin(), first)), static_cast<int> (std::distance (occurrences.begin(), last)) };
}

void Index::addRecords (int pathIndex, const jam::Document::Element& file)
{
    jassert (not records.contains (pathIndex));
    records.try_emplace (pathIndex);
    auto& sortedRecords { records.at (pathIndex) };

    SymbolDocument::applyToRecords (file, [&sortedRecords] (const jam::Document::Element& record)
    {
        sortedRecords.add (&record);
    });

    std::stable_sort (sortedRecords.begin(), sortedRecords.end(), [] (const jam::Document::Element* left, const jam::Document::Element* right)
    {
        return isRecordBefore (*left, *right);
    });
}

void Index::addSymbols (int pathIndex, const jam::Document::Element& file)
{
    addCompletions (pathIndex, file);
    addRecords (pathIndex, file);

    jam::HashMap<juce::int64, jam::Array<std::pair<int, const jam::Document::Element*>>> declarationBlocks;
    jam::HashMap<juce::int64, jam::Array<std::pair<int, const jam::Document::Element*>>> referenceBlocks;

    for (const auto* record : records.at (pathIndex))
    {
        const auto symbol { *record->get<juce::int64> (Id::symbol) };
        auto& blocks { (*record->get<int> (Id::role) & jam::LanguageRole::declaration) != 0 ? declarationBlocks : referenceBlocks };
        const auto [blockEntry, isBlockAdded] { blocks.try_emplace (symbol) };
        auto& [blockSymbol, block] { *blockEntry };

        if (block.isEmpty())
        {
            block.add ({ pathIndex, record });
        }
        else
        {
            const auto& [lastPathIndex, lastRecord] { block.last() };

            if (isRecordBefore (*lastRecord, *record))
                block.add ({ pathIndex, record });
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

    const juce::var count { static_cast<int> (documents.size()) };

    if (model.state[Id::documents] == count)
        model.state.sendPropertyChangeMessage (Id::documents);
    else
        model.state.setProperty (Id::documents, count, nullptr);
}
