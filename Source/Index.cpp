#include "Index.h"

Index::Index (LINGModel& modelToUse)
    : model (modelToUse)
{
}

void Index::removeDocuments (const juce::ValueTree& files)
{
    declarations.clear();
    references.clear();

    for (auto entry { documents.begin() }; entry != documents.end();)
    {
        const auto& [pathIndex, document] { *entry };

        if (files.getChildWithProperty (Id::path, paths.at (pathIndex)).isValid())
            ++entry;
        else
            entry = documents.erase (entry);
    }
}

std::unique_ptr<jam::LanguageDocument> Index::readSwapFile (const juce::File& swapFile, std::string_view sourceText, const std::shared_ptr<const jam::LanguageLibrary>& language, juce::int64 hash, juce::int64 languageHash)
{
    const juce::MemoryMappedFile mapped { swapFile, juce::MemoryMappedFile::readOnly };
    std::unique_ptr<jam::LanguageDocument> document;

    if (mapped.getData() != nullptr)
    {
        juce::MemoryInputStream input { mapped.getData(), mapped.getSize(), false };
        auto stored { jam::LanguageDocument::read (sourceText, input, language) };
        const auto* storedHash { stored->getRoot()->find<juce::int64> (Id::hash) };
        const auto* storedLanguageHash { stored->getRoot()->find<juce::int64> (Id::language) };

        if (storedHash != nullptr and *storedHash == hash and storedLanguageHash != nullptr and *storedLanguageHash == languageHash)
            document = std::move (stored);
    }

    return document;
}

void Index::writeSwapFile (const juce::File& swapFile, const std::unique_ptr<jam::LanguageDocument>& document, juce::int64 hash, juce::int64 languageHash)
{
    if (document != nullptr)
    {
        addOrReplaceProperty<juce::String> (*document->getRoot(), Id::version, ProjectInfo::versionString);
        addOrReplaceProperty<juce::int64> (*document->getRoot(), Id::hash, hash);
        addOrReplaceProperty<juce::int64> (*document->getRoot(), Id::language, languageHash);

        juce::TemporaryFile temporary { swapFile };

        {
            juce::FileOutputStream stream { temporary.getFile() };
            document->write (stream);
        }

        temporary.overwriteTargetFileWithTemporary();
    }
}

void Index::addDocument (const juce::String& path, std::unique_ptr<jam::LanguageDocument> document)
{
    const auto pathIndex { getOrCreatePathIndex (path) };

    declarations.clear();
    references.clear();

    if (document != nullptr)
        documents.addOrReplace (pathIndex, std::move (document));
    else
        documents.erase (pathIndex);
}

int Index::getOrCreatePathIndex (const juce::String& path)
{
    if (const auto pathIndex { paths.indexOf (path, false, 0) }; pathIndex >= 0)
        return pathIndex;

    paths.add (path);
    return paths.size() - 1;
}

void Index::addBindings (int rowIndex, Bindings records)
{
    bindings.addOrReplace (rowIndex, std::move (records));
}

const Index::Positions& Index::getPositions (jam::HashMap<int, Positions>& positionTables, jam::LanguageDocument& document, int pathIndex)
{
    auto [positionEntry, isAdded] { positionTables.try_emplace (pathIndex) };
    auto& [positionKey, positions] { *positionEntry };

    if (isAdded)
    {
        std::function<void (jam::Document::Element&, Positions&)> addPositions;

        addPositions = [&document, &addPositions] (jam::Document::Element& parent, Positions& table)
        {
            for (auto* child : parent)
            {
                const auto& name { *child->get<jam::Document::Tokens> (Id::name) };

                if (name.size() > 0)
                    table[static_cast<int> (document.getLineNumber (name.last().offset()))].try_emplace (static_cast<int> (document.getColumnNumber (name.last().offset())), child);

                addPositions (*child, table);
            }
        };

        addPositions (*document.getRoot(), positions);
    }

    return positions;
}

jam::Array<int> Index::addSymbols()
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    jam::HashMap<int, Positions> positionTables;
    jam::Array<int> changedPathIndices;
    jam::HashSet<const jam::Document::Element*> boundElements;

    const auto addRecord { [&boundElements] (const Positions& positions, int line, int column, const std::pair<std::string, int>& record)
    {
        if (const auto lineEntry { positions.find (line) }; lineEntry != positions.end())
        {
            const auto& [positionLine, positionColumns] { *lineEntry };

            if (const auto columnEntry { positionColumns.find (column) }; columnEntry != positionColumns.end())
            {
                const auto& [positionColumn, element] { *columnEntry };
                const auto& [symbol, role] { record };

                if (auto [boundEntry, isAdded] { boundElements.emplace (element) }; isAdded)
                {
                    addOrReplaceProperty<juce::String> (*element, Id::symbol, jam::Strings::fromView (symbol));
                    addOrReplaceProperty<int> (*element, Id::role, role);
                }
            }
        }
    } };

    for (int rowIndex { 0 }; rowIndex < files.getNumChildren(); ++rowIndex)
    {
        if (const auto entry { bindings.find (rowIndex) }; entry != bindings.end())
        {
            const juce::File directory { files.getChild (rowIndex)[Id::directory].toString() };
            const auto& [unitIndex, unitBindings] { *entry };

            for (const auto& [rawPath, lines] : unitBindings)
            {
                const auto pathIndex { paths.indexOf (directory.getChildFile (jam::Strings::fromView (rawPath)).getFullPathName(), false, 0) };

                if (pathIndex >= 0 and documents.contains (pathIndex))
                {
                    const auto& positions { getPositions (positionTables, *documents.at (pathIndex), pathIndex) };
                    const auto boundCount { boundElements.size() };

                    for (const auto& [line, columns] : lines)
                        for (const auto& [column, record] : columns)
                            addRecord (positions, line, column, record);

                    if (boundElements.size() > boundCount)
                        changedPathIndices.addIfNotAlreadyThere (pathIndex);
                }
            }
        }
    }

    bindings.clear();
    return changedPathIndices;
}

void Index::writeSwapFiles (const jam::Array<int>& pathIndices)
{
    for (const auto pathIndex : pathIndices)
    {
        const auto& document { documents.at (pathIndex) };

        writeSwapFile (model.getSwapFile (juce::File { paths.at (pathIndex) }), document, *document->getRoot()->find<juce::int64> (Id::hash), *document->getRoot()->find<juce::int64> (Id::language));
    }
}

void Index::setDocuments()
{
    writeSwapFiles (addSymbols());

    declarations.clear();
    references.clear();

    applyToElements ([this] (int pathIndex, const jam::LanguageDocument&, const jam::Document::Element& element)
    {
        if (const auto* symbol { element.find<juce::String> (Id::symbol) }; symbol != nullptr)
        {
            if ((*element.find<int> (Id::role) & jam::LanguageRole::declaration) != 0)
                declarations[*symbol].add ({ pathIndex, &element });
            else
                references[*symbol].add ({ pathIndex, &element });
        }
    });

    const juce::var count { static_cast<int> (documents.size()) };

    if (model.state[Id::documents] == count)
        model.state.sendPropertyChangeMessage (Id::documents);
    else
        model.state.setProperty (Id::documents, count, nullptr);
}
