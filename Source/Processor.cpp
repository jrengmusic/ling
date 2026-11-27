#include "Processor.h"
#include "LibraryLanguage.h"
#include "languages/MarkdownLanguage.h"
#include "languages/MermaidLanguage.h"
#include "languages/JsonLanguage.h"
#include "languages/CssLanguage.h"
#include "languages/XmlLanguage.h"
#include "languages/HtmlLanguage.h"

juce::ThreadPool::Options Processor::getPoolOptions (int threadCount)
{
    static constexpr size_t workerStackSizeBytes { 8 << 20 };

    return juce::ThreadPool::Options {}.withNumberOfThreads (threadCount)
                                       .withThreadStackSizeBytes (workerStackSizeBytes)
                                       .withDesiredThreadPriority (juce::Thread::Priority::normal);
}

Processor::Processor (LINGModel& newModel, std::shared_ptr<juce::ThreadPool> sharedPool)
    : model (newModel),
      index (model),
      ownedPool (sharedPool == nullptr ? std::make_shared<juce::ThreadPool> (getPoolOptions (LINGModel::getDefaultThreads())) : nullptr),
      pool (sharedPool != nullptr ? std::move (sharedPool) : ownedPool)
{
    model.addListener (this);
    registerEvents();
    addLanguages();
}

Processor::~Processor()
{
    model.removeListener (this);

    const auto removeJobs { [this] (const auto& jobs)
    {
        for (const auto& [rowIndex, job] : jobs)
            pool->removeJob (job.get(), true, waitForJob);
    } };

    removeJobs (unitsJobs);
    removeJobs (parseJobs);
    removeJobs (indexJobs);
}

bool Processor::hasDependency (const juce::ValueTree& unit, int unitIndex, int rowIndex)
{
    const auto* dependencies { unit[Id::dependencies].getArray() };

    return unit.hasProperty (Id::arguments)
       and (unitIndex == rowIndex
            or (dependencies != nullptr and std::any_of (dependencies->begin(), dependencies->end(), [rowIndex] (const juce::var& dependency) { return static_cast<int> (dependency) == rowIndex; })));
}

bool Processor::hasParseJob (const juce::ValueTree& unit, int unitIndex, int rowIndex) const
{
    const auto* dependencies { unit[Id::dependencies].getArray() };
    const auto isParsing { [this, rowIndex] (int dependency) { return dependency != rowIndex and (parseJobs.contains (dependency) or nextParseJobs.contains (dependency)); } };

    return isParsing (unitIndex)
        or (dependencies != nullptr and std::any_of (dependencies->begin(), dependencies->end(), [&isParsing] (const juce::var& dependency) { return isParsing (static_cast<int> (dependency)); }));
}

bool Processor::hasOpenBuffer (const juce::ValueTree& unit) const
{
    jassert (unit.isValid());

    const auto buffers { model.state.getChildWithName (jam::Format::toType (Id::buffers)) };

    if (buffers.getChildWithProperty (Id::path, unit[Id::path]).isValid())
        return true;

    if (const auto* dependencies { unit[Id::dependencies].getArray() }; dependencies != nullptr)
    {
        const auto files { unit.getParent() };

        return std::any_of (dependencies->begin(), dependencies->end(), [&buffers, &files] (const juce::var& dependency)
        {
            return buffers.getChildWithProperty (Id::path, files.getChild (static_cast<int> (dependency))[Id::path]).isValid();
        });
    }

    return false;
}

juce::int64 Processor::getUnitHash (const juce::ValueTree& unit, int unitIndex, const jam::Array<char>& arguments) const
{
    const auto files { unit.getParent() };
    const auto* dependencies { unit[Id::dependencies].getArray() };
    const auto getSourceHash { [this, &files] (int rowIndex) -> std::uint64_t
    {
        const auto pathIndex { index.paths.indexOf (files.getChild (rowIndex)[Id::path].toString(), false, 0) };

        if (const auto stored { index.sourceHashes.find (pathIndex) }; stored != index.sourceHashes.end())
        {
            const auto& [storedIndex, storedHash] { *stored };

            return static_cast<std::uint64_t> (storedHash);
        }

        return 0;
    } };
    auto hash { jam::Wyhash::fold (jam::Wyhash::hashBytes (arguments.data(), static_cast<size_t> (arguments.size())), getSourceHash (unitIndex)) };

    if (dependencies != nullptr)
        for (const auto& dependency : *dependencies)
            hash = jam::Wyhash::fold (hash, getSourceHash (static_cast<int> (dependency)));

    return static_cast<juce::int64> (hash);
}

juce::int64 Processor::getUnitStamp (const juce::ValueTree& unit, int unitIndex, const jam::Array<char>& arguments) const
{
    jassert (unit.isValid());
    return static_cast<juce::int64> (jam::Wyhash::fold (static_cast<std::uint64_t> (getUnitHash (unit, unitIndex, arguments)), juce::File { unit[Id::output].toString() }.existsAsFile() ? 1 : 0));
}

juce::ValueTree Processor::getUnit (const juce::String& path) const
{
    if (const auto unitRow { index.unitOwners->getRow (path) }; unitRow != Index::UnitOwners::noRow)
        return model.state.getChildWithName (jam::Format::toType (Id::files)).getChild (unitRow);

    return {};
}

jam::Array<char> Processor::getArguments (const juce::ValueTree& unit) const
{
    jam::Array<char> arguments;

    for (const auto& argument : *unit[Id::arguments].getArray())
        addArgument (arguments, jam::Strings::getView (argument.toString()));

    return arguments;
}

void Processor::addArgument (jam::Array<char>& arguments, std::string_view argument)
{
    arguments.insertArray (arguments.size(), argument.data(), static_cast<int> (argument.size()));
    arguments.add (static_cast<char> (Chars::nullCharacter));
}

jam::Array<const char*> Processor::getArgumentPointers (const jam::Array<char>& arguments)
{
    jam::Array<const char*> pointers;

    for (int position { 0 }; position < arguments.size(); ++position)
        if (position == 0 or arguments.at (position - 1) == static_cast<char> (Chars::nullCharacter))
            pointers.add (&arguments.at (position));

    return pointers;
}

void Processor::registerEvents()
{
    events.add (Id::goToDefinition, [this] (const juce::Identifier&, juce::ValueTree& request) { query.goToDefinition (request, index); });
    events.add (Id::findReferences, [this] (const juce::Identifier&, juce::ValueTree& request) { query.findReferences (request, index); });
    events.add (Id::hover, [this] (const juce::Identifier&, juce::ValueTree& request) { query.hover (request, index); });
    events.add (Id::documentSymbol, [this] (const juce::Identifier&, juce::ValueTree& request) { query.documentSymbol (request, index); });
    events.add (Id::workspaceSymbol, [this] (const juce::Identifier&, juce::ValueTree& request) { query.workspaceSymbol (request, index); });
    events.add (Id::completion, [this] (const juce::Identifier&, juce::ValueTree& request) { query.completion (request, index); });
    events.add (Id::goToImplementation, [this] (const juce::Identifier&, juce::ValueTree& request) { query.goToImplementation (request, index); });
    events.add (Id::prepareCallHierarchy, [this] (const juce::Identifier&, juce::ValueTree& request) { query.prepareCallHierarchy (request, index); });
    events.add (Id::incomingCalls, [this] (const juce::Identifier&, juce::ValueTree& request) { query.incomingCalls (request, index); });
    events.add (Id::outgoingCalls, [this] (const juce::Identifier&, juce::ValueTree& request) { query.outgoingCalls (request, index); });
    events.add (Id::diagnostics, [this] (const juce::Identifier&, juce::ValueTree& request) { query.diagnostics (request, index); });
    events.add (Id::publishDiagnostics, [this] (const juce::Identifier&, juce::ValueTree& request) { query.publishDiagnostics (request, index); });
    events.add (Id::switchSourceHeader, [this] (const juce::Identifier&, juce::ValueTree& request) { query.switchSourceHeader (request, index); });
    events.add (Id::inlayHint, [this] (const juce::Identifier&, juce::ValueTree& request) { query.inlayHint (request, index); });
    events.add (Id::semanticTokens, [this] (const juce::Identifier&, juce::ValueTree& request) { query.semanticTokens (request, index); });
    events.add (Id::semanticTokensRange, [this] (const juce::Identifier&, juce::ValueTree& request) { query.semanticTokensRange (request, index); });
    events.add (Id::selectionRange, [this] (const juce::Identifier&, juce::ValueTree& request) { query.selectionRange (request, index); });
    events.add (Id::textObject, [this] (const juce::Identifier&, juce::ValueTree& request) { query.textObject (request, index); });
    events.add (Id::documentHighlight, [this] (const juce::Identifier&, juce::ValueTree& request) { query.documentHighlight (request, index); });
    events.add (Id::rename, [this] (const juce::Identifier&, juce::ValueTree& request) { query.rename (request, index); });
    events.add (Id::typeDefinition, [this] (const juce::Identifier&, juce::ValueTree& request) { query.typeDefinition (request, index); });
    events.add (Id::codeAction, [this] (const juce::Identifier&, juce::ValueTree& request) { query.codeAction (request, index); });

    bufferEvents.add (Id::setBuffer, [this] (const juce::Identifier&, juce::ValueTree& request) { setBuffer (request); });
    bufferEvents.add (Id::removeBuffer, [this] (const juce::Identifier&, juce::ValueTree& request) { removeBuffer (request); });

    propertyEvents.add (Id::path, [this] (const juce::Identifier&, juce::ValueTree& tree)
    {
        if (tree.hasType (jam::Format::toType (Id::file)))
            setFile (tree);
        else if (tree.hasType (jam::Format::toType (Id::language)))
        {
            setLanguage (tree);
            addUnitsJob();
            triggerAsyncUpdate();
        }
    });
    propertyEvents.add (Id::hash, [this] (const juce::Identifier&, juce::ValueTree& tree)
    {
        if (tree.hasType (jam::Format::toType (Id::language)))
            setLanguageHash (tree);
    });
    propertyEvents.add (Id::isIndexed, [this] (const juce::Identifier&, juce::ValueTree& tree)
    {
        if (static_cast<bool> (tree[Id::isIndexed]))
        {
            if (tree.hasType (jam::Format::toType (Id::file)))
                addDiagnosticRequests (tree);

            runRequests();
        }
    });
    propertyEvents.add (Id::threads, [this] (const juce::Identifier&, juce::ValueTree&)
    {
        if (ownedPool != nullptr)
        {
            pool->removeAllJobs (true, waitForJob);
            parseJobs.clear();
            nextParseJobs.clear();
            indexJobs.clear();
            nextIndexJobs.clear();
            unitsJobs.clear();
            nextUnitsJobs.clear();

            ownedPool = std::make_shared<juce::ThreadPool> (getPoolOptions (static_cast<int> (model.state[Id::threads])));
            pool = ownedPool;

            addUnitsJob();

            if (const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) }; files.isValid())
                addJobs (files);
        }
    });

    childEvents.add (jam::Format::toType (Id::files), [this] (const juce::Identifier&, juce::ValueTree& files)
    {
        addJobs (files);
    });
    childEvents.add (jam::Format::toType (Id::request), [this] (const juce::Identifier&, juce::ValueTree& request)
    {
        if (events.contains (Query::getCommand (request)))
            addIndexJobs (request);

        if (isRequestReady (request))
            runRequest (request);
        else
            moveRequestJobsToFront (request);
    });
    childEvents.add (jam::Format::toType (Id::language), [this] (const juce::Identifier&, juce::ValueTree& language)
    {
        setLanguage (language);
        addUnitsJob();
        triggerAsyncUpdate();
    });
    childEvents.add (jam::Format::toType (Id::toolchain), [this] (const juce::Identifier&, juce::ValueTree&)
    {
        addUnitsJob();
    });
}

juce::Identifier Processor::getScope (const juce::ValueTree& request) const
{
    static const map::Readiness readiness;

    if (index.pathRows.contains (Query::getFile (request).getFullPathName()))
    {
        if (const auto entry { readiness.find (Query::getCommand (request)) }; entry != readiness.end())
        {
            const auto& [operation, scope] { *entry };

            return scope;
        }
    }

    return Id::files;
}

jam::Array<int> Processor::getUnitRows (const juce::ValueTree& request) const
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto scope { getScope (request) };
    jam::Array<int> rows;

    if (scope == Id::files)
    {
        for (int rowIndex { 0 }; rowIndex < files.getNumChildren(); ++rowIndex)
            if (files.getChild (rowIndex).hasProperty (Id::arguments))
                rows.add (rowIndex);
    }
    else if (const auto unitRow { index.unitOwners->getRow (Query::getFile (request).getFullPathName()) }; unitRow != Index::UnitOwners::noRow)
    {
        const auto* dependencies { files.getChild (unitRow)[Id::dependencies].getArray() };

        rows.add (unitRow);

        if (scope == Id::dependencies and dependencies != nullptr)
        {
            for (const auto& dependency : *dependencies)
            {
                if (const auto ownerRow { index.unitOwners->getRow (files.getChild (static_cast<int> (dependency))[Id::path].toString()) }; ownerRow != Index::UnitOwners::noRow)
                    rows.addIfNotAlreadyThere (ownerRow);
            }
        }
    }

    return rows;
}

jam::Array<int> Processor::getFailedRows (const juce::ValueTree& request) const
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    jam::Array<int> rows;

    for (const auto rowIndex : getUnitRows (request))
        if (files.getChild (rowIndex).hasProperty (Id::message))
            rows.add (rowIndex);

    return rows;
}

juce::String Processor::getMessage (const jam::Array<int>& rows) const
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    jam::Strings messages;

    for (const auto rowIndex : rows)
    {
        const auto row { files.getChild (rowIndex) };

        messages.add (row[Id::path].toString() + Id::diagnosticSeparator + row[Id::message].toString());
    }

    return messages.joinIntoString (juce::String::charToString (Chars::newline));
}

bool Processor::isRequestReady (const juce::ValueTree& request) const
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto hasIndexedDependencies { [this, &files] (const juce::String& path)
    {
        if (const auto unitRow { index.unitOwners->getRow (path) }; unitRow != Index::UnitOwners::noRow)
        {
            if (const auto* dependencies { files.getChild (unitRow)[Id::dependencies].getArray() }; dependencies != nullptr)
                return std::all_of (dependencies->begin(), dependencies->end(), [&files] (const juce::var& dependency) { return static_cast<bool> (files.getChild (static_cast<int> (dependency))[Id::isIndexed]); });
        }

        return true;
    } };
    const auto scope { getScope (request) };

    if (scope != Id::files)
    {
        const auto path { Query::getFile (request).getFullPathName() };

        return static_cast<bool> (files.getChild (index.pathRows.at (path))[Id::isIndexed]) and (scope == Id::file or hasIndexedDependencies (path));
    }

    return static_cast<bool> (files[Id::isIndexed]);
}

void Processor::moveRequestJobsToFront (const juce::ValueTree& request)
{
    const auto path { Query::getFile (request).getFullPathName() };
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    if (const auto unitRow { index.unitOwners->getRow (path) }; unitRow != Index::UnitOwners::noRow)
    {
        auto unit { files.getChild (unitRow) };
        const auto* dependencies { unit[Id::dependencies].getArray() };
        const auto moveParseJobToFront { [this] (int rowIndex)
        {
            if (const auto job { parseJobs.find (rowIndex) }; job != parseJobs.end())
            {
                auto& [jobRow, parseJob] { *job };
                pool->moveJobToFront (parseJob.get());
            }
        } };

        moveParseJobToFront (unitRow);

        if (dependencies != nullptr)
            for (const auto& dependency : *dependencies)
                moveParseJobToFront (static_cast<int> (dependency));

        if (const auto pathEntry { index.pathRows.find (path) }; pathEntry != index.pathRows.end())
        {
            const auto& [rowPath, rowIndex] { *pathEntry };
            moveParseJobToFront (rowIndex);
        }

        const auto moveIndexJobToFront { [this, &files] (int rowIndex)
        {
            files.getChild (rowIndex).setProperty (Id::isRequested, true, nullptr);

            if (const auto job { indexJobs.find (rowIndex) }; job != indexJobs.end())
            {
                auto& [jobRow, indexJob] { *job };
                pool->moveJobToFront (indexJob.get());
            }
        } };

        if (const auto* prerequisites { unit[Id::prerequisites].getArray() }; prerequisites != nullptr)
            for (const auto& prerequisite : *prerequisites)
                moveIndexJobToFront (static_cast<int> (prerequisite));

        moveIndexJobToFront (unitRow);
    }
}

void Processor::runRequests()
{
    for (const auto& connection : model.state.getChildWithName (jam::Format::toType (Id::connections)))
    {
        juce::Array<juce::ValueTree> pending;

        for (const auto& request : connection)
            if (isRequestReady (request))
                pending.add (request);

        for (auto& request : pending)
            runRequest (request);
    }
}

void Processor::addDiagnosticRequests (const juce::ValueTree& file)
{
    const auto directory { juce::File { file[Id::path].toString() }.getParentDirectory().getFullPathName() };

    if (not getUnit (file[Id::path].toString()).hasProperty (Id::message))
    {
        for (auto connection : model.state.getChildWithName (jam::Format::toType (Id::connections)))
        {
            if (connection.hasProperty (Id::positionEncoding))
            {
                juce::ValueTree request { jam::Format::toType (Id::request) };

                request.setProperty (Id::command, Id::publishDiagnostics.toString(), nullptr);
                request.setProperty (Id::directory, directory, nullptr);
                request.setProperty (Id::arguments, juce::Array<juce::var> { file[Id::path] }, nullptr);
                request.setProperty (Id::positionEncoding, connection[Id::positionEncoding], nullptr);
                connection.appendChild (request, nullptr);
            }
        }
    }
}

void Processor::setIndexed (int rowIndex)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    auto row { files.getChild (rowIndex) };
    bool isIndexed { not parseJobs.contains (rowIndex) and not nextParseJobs.contains (rowIndex) };

    if (const auto unitRow { index.unitOwners->getRow (row[Id::path].toString()) }; unitRow != Index::UnitOwners::noRow)
    {
        const auto unitTree { files.getChild (unitRow) };
        const auto unitFamily { static_cast<int> (unitTree[Id::family]) };
        const auto unitPathIndex { index.paths.indexOf (unitTree[Id::path].toString(), false, 0) };
        const bool hasSymbols { index.symbolDocuments.contains (unitPathIndex) };
        const bool hasNoIndexPending { not indexJobs.contains (unitRow) and not nextIndexJobs.contains (unitRow) };
        const bool needsIndex { index.languages.contains (unitFamily) and index.languages.at (unitFamily)->hasIndex() };

        isIndexed = isIndexed and (hasSymbols or (hasNoIndexPending and (unitTree.hasProperty (Id::message) or not needsIndex)));
    }

    row.setProperty (Id::isIndexed, isIndexed, nullptr);
}

void Processor::setUnitIndexed (const juce::ValueTree& unit, int unitIndex)
{
    setIndexed (unitIndex);

    if (const auto* dependencies { unit[Id::dependencies].getArray() }; dependencies != nullptr)
    {
        const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

        for (const auto& dependency : *dependencies)
        {
            const auto rowIndex { static_cast<int> (dependency) };

            if (const auto unitRow { index.unitOwners->getRow (files.getChild (rowIndex)[Id::path].toString()) }; unitRow != Index::UnitOwners::noRow)
            {
                if (unitRow == unitIndex)
                    setIndexed (rowIndex);
            }
        }
    }
}

void Processor::setIndexed()
{
    auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto areAllRowsIndexed { [&files]
    {
        for (int rowIndex { 0 }; rowIndex < files.getNumChildren(); ++rowIndex)
            if (not static_cast<bool> (files.getChild (rowIndex)[Id::isIndexed]))
                return false;

        return true;
    }() };

    files.setProperty (Id::isIndexed,
                       model.state.getChildWithName (jam::Format::toType (Id::toolchain)).isValid()
                           and parseJobs.empty()
                           and nextParseJobs.empty()
                           and indexJobs.empty()
                           and nextIndexJobs.empty()
                           and unitsJobs.empty()
                           and nextUnitsJobs.empty()
                           and areAllRowsIndexed,
                       nullptr);

    const int pendingJobs { static_cast<int> (parseJobs.size() + nextParseJobs.size() + indexJobs.size() + nextIndexJobs.size() + unitsJobs.size() + nextUnitsJobs.size()) };
    auto* jobCountParameter { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::jobCount) };
    auto* pendingJobCountParameter { model.getParameter<jam::Parameter<int>> (jam::Format::toType (Id::overlay), Id::pendingJobCount) };
    jassert (jobCountParameter != nullptr);
    jassert (pendingJobCountParameter != nullptr);

    jobCountParameter->setValue (pendingJobs == 0 ? 0 : std::max (jobCountParameter->getValue(), pendingJobs));
    pendingJobCountParameter->setValue (pendingJobs);
}

void Processor::runRequest (juce::ValueTree& request)
{
    jassert (request[Id::arguments].isArray());

    if (const auto unitRow { index.unitOwners->getRow (Query::getFile (request).getFullPathName()) }; unitRow != Index::UnitOwners::noRow)
    {
        const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
        auto unit { files.getChild (unitRow) };

        if (const auto* prerequisites { unit[Id::prerequisites].getArray() }; prerequisites != nullptr)
            for (const auto& prerequisite : *prerequisites)
                files.getChild (static_cast<int> (prerequisite)).removeProperty (Id::isRequested, nullptr);

        unit.removeProperty (Id::isRequested, nullptr);
    }

    const auto key { Query::getCommand (request) };

    if ((events.contains (key) or bufferEvents.contains (key)) and Query::isValidRequest (request))
    {
        const auto path { Query::getFile (request).getFullPathName() };

        if (const auto unit { getUnit (path) }; unit.isValid())
        {
            if (const auto languageRow { model.state.getChildWithProperty (Id::family, unit[Id::family]) }; languageRow.hasType (jam::Format::toType (Id::language)) and languageRow.hasProperty (Id::message))
                model.setMessage (path + Id::diagnosticSeparator + languageRow[Id::message].toString());
        }

        if (bufferEvents.contains (key))
        {
            bufferEvents.get (key, request);
        }
        else if (const auto failedRows (getFailedRows (request)); failedRows.isEmpty())
        {
            const Index::DocumentScope documentScope { index };

            events.get (key, request);
        }
        else
        {
            const auto message { getMessage (failedRows) };

            Query::setError (request, lsp::ErrorCodes::RequestFailed, jam::Strings::getView (message));
        }
    }
    else
    {
        Query::setError (request, lsp::ErrorCodes::InvalidParams);
    }
}

juce::File Processor::getOrCreateLanguages()
{
    static const juce::String languagesHash { CAST_LANGUAGES_HASH };

    const auto cache { LINGModel::getCacheDirectory().getChildFile (files::language) };
    const auto languages { cache.getChildFile (languagesHash) };

    if (languages.isDirectory())
        return languages;

    [[maybe_unused]] const auto isCreated { cache.createDirectory() };
    jassert (isCreated.wasOk());

    juce::TemporaryFile temporary { languages };

    for (int resourceIndex { 0 }; resourceIndex < BinaryData::namedResourceListSize; ++resourceIndex)
    {
        const char* resourceName { BinaryData::namedResourceList[resourceIndex] };
        const auto resourceFile { juce::File::createFileWithoutCheckingPath (BinaryData::getNamedResourceOriginalFilename (resourceName)) };

        if (resourceFile.hasFileExtension (files::archiveExtension))
        {
            int resourceSize { 0 };
            const char* resourceData { BinaryData::getNamedResource (resourceName, resourceSize) };
            juce::MemoryInputStream stream { resourceData, static_cast<size_t> (resourceSize), false };
            juce::ZipFile zip { stream };
            [[maybe_unused]] const auto unpacked { zip.uncompressTo (temporary.getFile().getChildFile (resourceFile.getFileNameWithoutExtension()), true) };
            jassert (unpacked.wasOk());
        }
    }

    if (temporary.getFile().moveFileTo (languages))
    {
        const auto languageDirectories (LINGModel::getLanguageDirectories());

        for (const auto& child : cache.findChildFiles (juce::File::findDirectories, false))
        {
            const auto isKept { child == languages
                                or std::any_of (languageDirectories.begin(), languageDirectories.end(),
                                       [&child] (const juce::File& directory) { return directory.getFileName().compare (child.getFileName()) == 0; }) };

            if (not isKept)
            {
                [[maybe_unused]] const auto isDeleted { child.deleteRecursively() };
                jassert (isDeleted);
            }
        }
    }

    return languages;
}

void Processor::setLanguage (juce::ValueTree languageRow)
{
    if (languageRow.hasProperty (Id::path))
    {
        static constexpr const char* entryName { "ling_get_language" };

        const auto name { languageRow[Id::name].toString() };
        const auto build { LINGModel::getLanguageLibrary (juce::File { languageRow[Id::path].toString() }) };
        const auto isBuilt { build.existsAsFile() };
        const auto source { isBuilt ? build : getOrCreateLanguages().getChildFile (name).getChildFile (build.getFileName()) };

        juce::MemoryBlock bytes;

        source.loadFileAsData (bytes);

        const auto hash { static_cast<juce::int64> (jam::Wyhash::hashBytes (bytes.getData(), bytes.getSize())) };
        const auto cache { LINGModel::getCacheDirectory().getChildFile (files::language).getChildFile (name) };
        const auto copy { cache.getChildFile (juce::String (hash)).withFileExtension (build.getFileExtension()) };
        const auto library { isBuilt ? copy : source };

        if (isBuilt and not copy.existsAsFile())
        {
            [[maybe_unused]] const auto isCreated { cache.createDirectory() };
            [[maybe_unused]] const auto isWritten { copy.replaceWithData (bytes.getData(), bytes.getSize()) };
            jassert (isCreated.wasOk() and isWritten);

            for (const auto& file : cache.findChildFiles (juce::File::findFiles, false))
            {
                if (file != copy)
                {
                    [[maybe_unused]] const auto isDeleted { file.deleteFile() };
                    jassert (isDeleted);
                }
            }
        }

        if (auto language { jam::LanguageLibrary::getLanguageLibrary (library, entryName) }; language != nullptr)
        {
            index.languages.addOrReplace (static_cast<int> (languageRow[Id::family]), std::make_shared<const LibraryLanguage> (language));
            index.loadedLanguages.add (index.languages.at (static_cast<int> (languageRow[Id::family])));
            languageRow.setProperty (Id::hash, hash, nullptr);
        }
        else
        {
            model.setMessage (juce::String (text::Diagnostics::failLanguageLoad) + Id::diagnosticSeparator + library.getFullPathName());
        }
    }
}

void Processor::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& childWhichHasBeenAdded)
{
    if (childEvents.contains (childWhichHasBeenAdded.getType()))
        childEvents.get (childWhichHasBeenAdded.getType(), childWhichHasBeenAdded);
}

void Processor::valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree& childWhichHasBeenRemoved, int)
{
    if (childWhichHasBeenRemoved.hasType (jam::Format::toType (Id::request)) or childWhichHasBeenRemoved.hasType (jam::Format::toType (Id::connection)))
        clearUnusedRequests();
}

void Processor::clearUnusedRequests()
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    jam::HashSet<int> activeUnits;

    for (const auto& connection : model.state.getChildWithName (jam::Format::toType (Id::connections)))
    {
        for (const auto& request : connection)
        {
            for (const auto unitRow : getUnitRows (request))
            {
                activeUnits.insert (unitRow);

                if (const auto* prerequisites { files.getChild (unitRow)[Id::prerequisites].getArray() }; prerequisites != nullptr)
                    for (const auto& prerequisite : *prerequisites)
                        activeUnits.insert (static_cast<int> (prerequisite));
            }
        }
    }

    for (int rowIndex { 0 }; rowIndex < files.getNumChildren(); ++rowIndex)
    {
        auto row { files.getChild (rowIndex) };

        if (row.hasProperty (Id::isRequested) and not activeUnits.contains (rowIndex))
            row.removeProperty (Id::isRequested, nullptr);
    }
}

void Processor::addJobs (const juce::ValueTree& files)
{
    index.addPathRows (files);
    index.addUnitRows (files);
    index.removeDocuments (files);
    nextIndexJobs.clear();

    const auto manifest { model.state[Id::manifest].toString() };
    const auto toolchainArgument { model.state[Id::toolchainArgument].toString() };

    LINGModel::getCacheDirectory (juce::File { manifest }, jam::Strings::getView (toolchainArgument)).createDirectory();

    for (int rowIndex { 0 }; rowIndex < files.getNumChildren(); ++rowIndex)
    {
        const auto row { files.getChild (rowIndex) };

        if (index.languages.contains (static_cast<int> (row[Id::family])))
            addJob (row, rowIndex);
    }

    for (int rowIndex { 0 }; rowIndex < files.getNumChildren(); ++rowIndex)
        setIndexed (rowIndex);

    for (const auto& connection : model.state.getChildWithName (jam::Format::toType (Id::connections)))
    {
        for (const auto& request : connection)
        {
            if (events.contains (Query::getCommand (request)))
                addIndexJobs (request);

            if (not isRequestReady (request))
                moveRequestJobsToFront (request);
        }
    }

    setIndexed();
    runRequests();
    triggerAsyncUpdate();
}

template <typename Type>
static std::shared_ptr<const Language> getBuiltInLanguage (const juce::String& extension)
{
    return std::make_shared<const Type> (extension);
}

int Processor::addBuiltInLanguages()
{
    static const jam::HashMap<juce::String, std::shared_ptr<const Language> (*) (const juce::String&)> builtInLanguages {
        { "md", &getBuiltInLanguage<MarkdownLanguage> },
        { "mmd", &getBuiltInLanguage<MermaidLanguage> },
        { "json", &getBuiltInLanguage<JsonLanguage> },
        { "css", &getBuiltInLanguage<CssLanguage> },
        { "xml", &getBuiltInLanguage<XmlLanguage> },
        { "html", &getBuiltInLanguage<HtmlLanguage> }
    };

    const map::Languages languages;
    const juce::MemoryMappedFile executable { juce::File::getSpecialLocation (juce::File::currentExecutableFile), juce::MemoryMappedFile::readOnly };
    const auto hash { static_cast<juce::int64> (jam::Wyhash::hashBytes (executable.getData(), executable.getSize())) };
    jam::Strings extensions;

    for (const auto& [extension, name] : languages)
        extensions.add (extension);

    extensions.sort (false);

    int family { 0 };

    for (const auto& extension : extensions)
    {
        juce::Array<juce::var> languageExtensions;
        juce::ValueTree languageRow { jam::Format::toType (Id::language) };

        languageExtensions.add (extension);
        languageRow.setProperty (Id::name, languages.at (extension), nullptr);
        languageRow.setProperty (Id::family, family, nullptr);
        languageRow.setProperty (Id::extensions, languageExtensions, nullptr);
        languageRow.setProperty (Id::hash, hash, nullptr);
        index.languages.addOrReplace (family, builtInLanguages.at (extension) (extension));
        model.state.appendChild (languageRow, nullptr);
        ++family;
    }

    return family;
}

void Processor::addLanguages()
{
    const auto firstFamily { addBuiltInLanguages() };
    const auto folders (LINGModel::getOrCreateLanguageDirectories());

    if (folders.isEmpty())
        model.setMessage (LINGModel::getConfigDirectory().getChildFile (files::language).getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failConstructionNotFound);

    for (int folderIndex { 0 }; folderIndex < folders.size(); ++folderIndex)
    {
        const auto family { firstFamily + folderIndex };

        if (not model.state.getChildWithProperty (Id::family, family).isValid())
        {
            const auto& folder { folders.getReference (folderIndex) };
            const auto file { folder.getChildFile (files::languageConstruction) };
            const auto construction { jam::MarkdownDocument::parse (file.loadFileAsString(), file.getFullPathName()) };
            juce::Array<juce::var> extensions;
            juce::ValueTree languageRow { jam::Format::toType (Id::language) };

            if (construction != nullptr)
                for (const auto& extension : construction->getTableRowKeys (Id::extensions))
                    extensions.add (extension);

            languageRow.setProperty (Id::name, folder.getFileName(), nullptr);
            languageRow.setProperty (Id::family, family, nullptr);
            languageRow.setProperty (Id::path, folder.getFullPathName(), nullptr);
            languageRow.setProperty (Id::extensions, extensions, nullptr);
            model.state.appendChild (languageRow, nullptr);
        }
    }
}

void Processor::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (propertyEvents.contains (property))
        propertyEvents.get (property, tree);
}

void Processor::setBuffer (juce::ValueTree& request)
{
    const auto path { Query::getFile (request).getFullPathName() };
    auto buffers { model.state.getChildWithName (jam::Format::toType (Id::buffers)) };
    auto buffer { buffers.getChildWithProperty (Id::path, path) };

    if (buffer.isValid())
    {
        buffer.setProperty (Id::text, request[Id::text], nullptr);
    }
    else
    {
        buffer = juce::ValueTree { jam::Format::toType (Id::buffer) };
        buffer.setProperty (Id::path, path, nullptr);
        buffer.setProperty (Id::text, request[Id::text], nullptr);
        buffers.appendChild (buffer, nullptr);
    }

    setBufferChange (request);
}

void Processor::removeBuffer (juce::ValueTree& request)
{
    const auto path { Query::getFile (request).getFullPathName() };
    auto buffers { model.state.getChildWithName (jam::Format::toType (Id::buffers)) };
    const auto buffer { buffers.getChildWithProperty (Id::path, path) };

    if (buffer.isValid())
        buffers.removeChild (buffer, nullptr);

    setBufferChange (request);
}

void Processor::setBufferChange (juce::ValueTree& request)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    if (const auto entry { index.pathRows.find (Query::getFile (request).getFullPathName()) }; entry != index.pathRows.end())
    {
        const auto& [rowPath, rowIndex] { *entry };

        files.getChild (rowIndex).sendPropertyChangeMessage (Id::path);
    }

    if (request.getParent().hasProperty (Id::positionEncoding))
        request.getParent().removeChild (request, nullptr);
    else
        Query::setResult (request, [] (juce::MemoryBlock& message) { jam::JsonWriter::appendNull (message); });
}

void Processor::setFile (const juce::ValueTree& file)
{
    const auto files { file.getParent() };
    const auto rowIndex { files.indexOf (file) };

    if (index.languages.contains (static_cast<int> (file[Id::family])))
        addJob (file, rowIndex);
}

void Processor::setLanguageHash (const juce::ValueTree& languageRow)
{
    const auto family { static_cast<int> (languageRow[Id::family]) };
    const auto rows { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    for (int rowIndex { 0 }; rowIndex < rows.getNumChildren(); ++rowIndex)
    {
        const auto row { rows.getChild (rowIndex) };

        if (static_cast<int> (row[Id::family]) == family)
            addJob (row, rowIndex);
    }
}

Processor::ParseJob::ParseJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::File& swapFileToUse, std::shared_ptr<const Language> languageToUse, juce::int64 languageHashToUse, int familyToUse, std::optional<juce::String> bufferToUse)
    : juce::ThreadPoolJob (pathToUse),
      owner (ownerToUse),
      rowIndex (rowIndexToUse),
      path (pathToUse),
      swapFile (swapFileToUse),
      language (std::move (languageToUse)),
      languageHash (languageHashToUse),
      family (familyToUse),
      buffer (std::move (bufferToUse))
{
}

juce::ThreadPoolJob::JobStatus Processor::ParseJob::runJob()
{
#if JAM_USING_PROFILER
    const jam::Profiler::Zone zone { Id::runParseJob };
#endif

    try
    {
        const juce::File file { path };

        modificationTime = file.getLastModificationTime();

        if (buffer.has_value())
        {
            const auto bytes { jam::Strings::getView (*buffer) };
            const auto decoded { bytes.find (static_cast<char> (Chars::escapeCharacter)) == std::string_view::npos ? std::string {} : jam::AnsiDocument::getText (bytes) };
            const auto view { decoded.empty() ? bytes : std::string_view { decoded } };
            auto [parsedDocument, parsedSymbols] { language->parse (view, path) };

            document = std::move (parsedDocument);
            symbols = std::move (parsedSymbols);
            result = document == nullptr ? language->isValid (view) : juce::Result::ok();

            if (document != nullptr)
            {
                sourceHash = static_cast<juce::int64> (jam::Wyhash::hashBytes (bytes.data(), bytes.size()));
                Index::addStamp (*document, sourceHash, languageHash, family);
            }
        }
        else if (file.existsAsFile())
        {
            const auto diskText { file.loadFileAsString() };
            const auto bytes { jam::Strings::getView (diskText) };
            const auto hash { static_cast<juce::int64> (jam::Wyhash::hashBytes (bytes.data(), bytes.size())) };
            const auto decoded { bytes.find (static_cast<char> (Chars::escapeCharacter)) == std::string_view::npos ? std::string {} : jam::AnsiDocument::getText (bytes) };
            const auto view { decoded.empty() ? bytes : std::string_view { decoded } };

            document = Index::readSwapFile (swapFile, view, language, hash, languageHash, family);

            if (document == nullptr)
            {
                auto [parsedDocument, parsedSymbols] { language->parse (view, path) };

                document = std::move (parsedDocument);
                symbols = std::move (parsedSymbols);
                result = document == nullptr ? language->isValid (view) : juce::Result::ok();

                if (document != nullptr)
                    result = Index::writeSwapFile (swapFile, *document, hash, languageHash, family);
            }
            else
            {
                symbols = language->getSymbols (*document, path);
            }

            if (document != nullptr and result.wasOk())
            {
                sourceHash = hash;
                document.reset();
            }
        }
        else
        {
            swapFile.deleteFile();
        }
    }
    catch (const std::exception& error)
    {
        result = juce::Result::fail (error.what());
    }

    isFinished.store (true, std::memory_order_release);
    owner.triggerAsyncUpdate();

    return jobHasFinished;
}

Processor::IndexJob::IndexJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::String& directoryToUse, jam::Array<char> argumentsToUse, juce::int64 hashToUse, const juce::File& symbolSwapFileToUse, juce::int64 languageHashToUse, std::shared_ptr<const Language> languageToUse, int familyToUse, juce::StringArray bufferPathsToUse, juce::StringArray bufferTextsToUse, std::shared_ptr<const Index::UnitOwners> unitOwnersToUse)
    : juce::ThreadPoolJob (pathToUse),
      owner (ownerToUse),
      rowIndex (rowIndexToUse),
      path (pathToUse),
      directory (directoryToUse),
      arguments (std::move (argumentsToUse)),
      hash (hashToUse),
      symbolSwapFile (symbolSwapFileToUse),
      languageHash (languageHashToUse),
      language (std::move (languageToUse)),
      family (familyToUse),
      bufferPaths (std::move (bufferPathsToUse)),
      bufferTexts (std::move (bufferTextsToUse)),
      unitOwners (std::move (unitOwnersToUse))
{
}

juce::ThreadPoolJob::JobStatus Processor::IndexJob::runJob()
{
#if JAM_USING_PROFILER
    const jam::Profiler::Zone zone { Id::runIndexJob };
#endif

    try
    {
        symbols = Index::readSwapFile (symbolSwapFile, hash, languageHash, family);

        if (symbols == nullptr)
            addSymbols();
    }
    catch (const std::exception& error)
    {
        result = juce::Result::fail (error.what());
    }

    isFinished.store (true, std::memory_order_release);
    owner.triggerAsyncUpdate();

    return jobHasFinished;
}

void Processor::IndexJob::addSymbols()
{
    const auto pointers (getArgumentPointers (arguments));
    jam::Array<const char*> bufferPathPointers;
    jam::Array<const char*> bufferTextPointers;
    jam::LanguageCallbacks callbacks {};

    for (const auto& bufferPath : bufferPaths)
        bufferPathPointers.add (bufferPath.toRawUTF8());

    for (const auto& bufferText : bufferTexts)
        bufferTextPointers.add (bufferText.toRawUTF8());

    callbacks.context = this;
    callbacks.addSymbol = &IndexJob::addSymbol;
    callbacks.addHover = &IndexJob::addHover;
    callbacks.addInlayHint = &IndexJob::addInlayHint;
    callbacks.addDiagnostic = &IndexJob::addDiagnostic;
    callbacks.addOutline = &IndexJob::addOutline;
    callbacks.addCompletion = &IndexJob::addCompletion;
    callbacks.addFixIt = &IndexJob::addFixIt;
    callbacks.addMessage = &IndexJob::addMessage;
    callbacks.shouldExit = [] (void* context) { return static_cast<IndexJob*> (context)->shouldExit() ? 1 : 0; };
    callbacks.isForeign = [] (void* context, const char* path) { return static_cast<IndexJob*> (context)->isForeign (path) ? 1 : 0; };

    result = juce::Result::fail (text::Diagnostics::failUnitIndex);

    if (language->index (pointers.data(), pointers.size(), directory.toRawUTF8(), bufferPathPointers.data(), bufferTextPointers.data(), bufferPathPointers.size(), callbacks) == 0)
    {
        builder.sortAndDeduplicate();
        temporary = std::make_unique<juce::TemporaryFile> (symbolSwapFile);
        result = builder.writeSwapFile (temporary->getFile(), hash, languageHash, family);
    }
}

bool Processor::IndexJob::isForeign (const char* recordPath) const
{
    return unitOwners->isForeign (recordPath, rowIndex);
}

void Processor::IndexJob::addSymbol (void* context, const char* recordPath, int line, int column, unsigned long long symbol, int nameLength, int role, int type, int hover, const unsigned long long* typeSymbols, int typeCount)
{
    auto& job { *static_cast<IndexJob*> (context) };

    jassert (line > 0 and column > 0);

    const auto fileIndex { job.builder.getOrCreateFile (std::string_view { recordPath }) };

    job.builder.addSymbol (fileIndex, static_cast<int64_t> (symbol), line, column, nameLength, role, type, hover, typeSymbols, typeCount);
}

int Processor::IndexJob::addHover (void* context, const char* recordPath, const char* text, const char* definitionPath, int line, int column, int endLine, int endColumn)
{
    auto& job { *static_cast<IndexJob*> (context) };
    const auto fileIndex { job.builder.getOrCreateFile (std::string_view { recordPath }) };

    return job.builder.addHover (fileIndex, std::string_view { text }, definitionPath != nullptr ? std::string_view { definitionPath } : std::string_view {}, line, column, endLine, endColumn);
}

void Processor::IndexJob::addInlayHint (void* context, const char* recordPath, int line, int column, const char* label, int type, int paddingLeft, int paddingRight)
{
    auto& job { *static_cast<IndexJob*> (context) };
    const auto fileIndex { job.builder.getOrCreateFile (std::string_view { recordPath }) };

    job.builder.addInlayHint (fileIndex, line, column, std::string_view { label }, type, paddingLeft != 0, paddingRight != 0);
}

void Processor::IndexJob::addDiagnostic (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, int severity, const char* message)
{
    auto& job { *static_cast<IndexJob*> (context) };
    const auto fileIndex { job.builder.getOrCreateFile (std::string_view { recordPath }) };

    job.builder.addDiagnostic (fileIndex, line, column, endLine, endColumn, severity, std::string_view { message });
}

int Processor::IndexJob::addOutline (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, int nameLength, int qualifierLength, int type, int parent)
{
    auto& job { *static_cast<IndexJob*> (context) };
    const auto fileIndex { job.builder.getOrCreateFile (std::string_view { recordPath }) };
    const std::array<int, outlineFieldCount> keyFields { fileIndex, line, column, endLine, endColumn, nameLength, qualifierLength, type };
    std::uint64_t keyHash { 0 };

    for (const auto keyField : keyFields)
        keyHash = jam::Wyhash::fold (keyHash, static_cast<std::uint64_t> (keyField));

    if (const auto known { job.outlineKeys.find (static_cast<juce::int64> (keyHash)) }; known != job.outlineKeys.end())
    {
        const auto& [knownKey, knownOutlineIndex] { *known };

        job.outlineSlots.add ({ fileIndex, knownOutlineIndex });
    }
    else
    {
        const auto outlineIndex { job.builder.addOutline (fileIndex, line, column, endLine, endColumn, nameLength, qualifierLength, type, job.getOutlineParent (parent, fileIndex)) };

        job.outlineKeys.addOrReplace (static_cast<juce::int64> (keyHash), outlineIndex);
        job.outlineSlots.add ({ fileIndex, outlineIndex });
    }

    return job.outlineSlots.size() - 1;
}

int Processor::IndexJob::getOutlineParent (int parent, int pathIndex) const
{
    if (parent >= 0)
    {
        const auto& [parentPathIndex, parentTokenIndex] { outlineSlots.at (parent) };

        if (parentPathIndex == pathIndex)
            return parentTokenIndex;
    }

    return SymbolDocument::noParent;
}

void Processor::IndexJob::addCompletion (void* context, const char* recordPath, unsigned long long scopeSymbol, unsigned long long symbol, const char* label, const char* insertText, int type)
{
    auto& job { *static_cast<IndexJob*> (context) };
    const auto key { static_cast<juce::int64> (jam::Wyhash::fold (static_cast<std::uint64_t> (scopeSymbol), static_cast<std::uint64_t> (symbol))) };
    auto [keyEntry, isInserted] { job.completionKeys.insert (key) };

    if (isInserted)
    {
        const auto fileIndex { job.builder.getOrCreateFile (std::string_view { recordPath }) };

        job.builder.addCompletion (fileIndex, static_cast<int64_t> (scopeSymbol), static_cast<int64_t> (symbol), std::string_view { label }, std::string_view { insertText }, type);
    }
}

void Processor::IndexJob::addFixIt (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, const char* text)
{
    auto& job { *static_cast<IndexJob*> (context) };
    const auto fileIndex { job.builder.getOrCreateFile (std::string_view { recordPath }) };

    job.builder.addFixIt (fileIndex, line, column, endLine, endColumn, std::string_view { text });
}

void Processor::IndexJob::addMessage (void* context, const char* messageText)
{
    auto& job { *static_cast<IndexJob*> (context) };

    job.result = juce::Result::fail (juce::String (text::Diagnostics::failUnitIndex) + Id::diagnosticSeparator + juce::String::fromUTF8 (messageText));
}

void Processor::addJob (const juce::ValueTree& row, int rowIndex)
{
    const auto family { static_cast<int> (row[Id::family]) };
    const auto swapFile { model.getSwapFile (juce::File { row[Id::path].toString() }, files::lingSwap) };
    const auto buffers { model.state.getChildWithName (jam::Format::toType (Id::buffers)) };
    const auto buffer { buffers.getChildWithProperty (Id::path, row[Id::path].toString()) };
    const auto text { buffer.isValid() ? std::optional<juce::String> { buffer[Id::text].toString() } : std::nullopt };

    swapFile.getParentDirectory().createDirectory();

    setJob (parseJobs, nextParseJobs, rowIndex, std::make_unique<ParseJob> (*this, rowIndex, row[Id::path].toString(), swapFile, index.languages.at (family), static_cast<juce::int64> (model.state.getChildWithProperty (Id::family, family)[Id::hash]), family, text));
    setIndexed (rowIndex);
    setIndexed();
}

void Processor::addIndexJob (juce::ValueTree row, int rowIndex)
{
    auto arguments (getArguments (row));
    juce::StringArray bufferPaths;
    juce::StringArray bufferTexts;

    if (const auto running { indexJobs.find (rowIndex) }; running != indexJobs.end())
    {
        const auto& [runningRow, runningJob] { *running };

        if (pool->removeJob (runningJob.get(), true, noWait))
            indexJobs.erase (rowIndex);
    }

    const auto family { static_cast<int> (row[Id::family]) };
    const auto hash { getUnitStamp (row, rowIndex, arguments) };
    const auto buffers { model.state.getChildWithName (jam::Format::toType (Id::buffers)) };

    for (const auto& buffer : buffers)
    {
        bufferPaths.add (buffer[Id::path].toString());
        bufferTexts.add (buffer[Id::text].toString());
    }

    row.removeProperty (Id::message, nullptr);

    nextIndexJobs.addOrReplace (rowIndex, std::make_unique<IndexJob> (*this, rowIndex, row[Id::path].toString(), row[Id::directory].toString(), std::move (arguments), hash, model.getSwapFile (juce::File { row[Id::path].toString() }, files::lingSymbols), static_cast<juce::int64> (model.state.getChildWithProperty (Id::family, family)[Id::hash]), index.languages.at (family), family, std::move (bufferPaths), std::move (bufferTexts), index.unitOwners));
    setUnitIndexed (row, rowIndex);
    setIndexed();
}

void Processor::addIndexJobs()
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto isWaiting { [this, &files] (int rowIndex)
    {
        const auto* prerequisites { files.getChild (rowIndex)[Id::prerequisites].getArray() };

        return prerequisites != nullptr
           and std::any_of (prerequisites->begin(), prerequisites->end(), [this] (const juce::var& prerequisite) { return indexJobs.contains (static_cast<int> (prerequisite)) or nextIndexJobs.contains (static_cast<int> (prerequisite)); });
    } };
    jam::Array<int> rows;

    for (const auto& [rowIndex, job] : nextIndexJobs)
        if (not indexJobs.contains (rowIndex) and not isWaiting (rowIndex))
            rows.add (rowIndex);

    std::sort (rows.begin(), rows.end(), [&files] (int first, int second)
    {
        const bool isFirstRequested { static_cast<bool> (files.getChild (first)[Id::isRequested]) };
        const bool isSecondRequested { static_cast<bool> (files.getChild (second)[Id::isRequested]) };

        if (isFirstRequested != isSecondRequested)
            return isFirstRequested;

        const bool isFirstProducer { files.getChild (first)[Id::output].toString().isNotEmpty() };
        const bool isSecondProducer { files.getChild (second)[Id::output].toString().isNotEmpty() };

        if (isFirstProducer != isSecondProducer)
            return isFirstProducer;

        return files.getChild (first)[Id::dependencies].size() > files.getChild (second)[Id::dependencies].size();
    });

    for (const auto rowIndex : rows)
    {
        indexJobs.addOrReplace (rowIndex, std::move (nextIndexJobs.at (rowIndex)));
        nextIndexJobs.erase (rowIndex);
        pool->addJob (indexJobs.at (rowIndex).get(), false);
    }

    for (int i { rows.size() - 1 }; i >= 0; --i)
    {
        const auto rowIndex { rows.at (i) };

        if (static_cast<bool> (files.getChild (rowIndex)[Id::isRequested]) or files.getChild (rowIndex)[Id::output].toString().isNotEmpty())
            pool->moveJobToFront (indexJobs.at (rowIndex).get());
    }
}

void Processor::addIndexJobs (const juce::ValueTree& request)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    for (const auto rowIndex : getUnitRows (request))
    {
        const auto row { files.getChild (rowIndex) };

        if (not static_cast<bool> (row[Id::isIndexed]) or row.hasProperty (Id::message) or isUnitStale (row, rowIndex))
        {
            if (const auto* prerequisites { row[Id::prerequisites].getArray() }; prerequisites != nullptr)
            {
                for (const auto& prerequisite : *prerequisites)
                {
                    const auto prerequisiteIndex { static_cast<int> (prerequisite) };
                    const auto producer { files.getChild (prerequisiteIndex) };

                    if (not static_cast<bool> (producer[Id::isIndexed]) or producer.hasProperty (Id::message) or isUnitStale (producer, prerequisiteIndex))
                        requestUnit (prerequisiteIndex);
                }
            }

            requestUnit (rowIndex);
            triggerAsyncUpdate();
        }
    }
}

void Processor::requestUnit (int unitRow)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    jassert (unitRow >= 0 and unitRow < files.getNumChildren());

    auto row { files.getChild (unitRow) };
    const auto family { static_cast<int> (row[Id::family]) };

    row.setProperty (Id::isRequested, true, nullptr);

    if (not hasParseJob (row, unitRow, noRow)
        and index.languages.contains (family)
        and index.languages.at (family)->hasIndex()
        and not indexJobs.contains (unitRow)
        and not nextIndexJobs.contains (unitRow))
    {
        addIndexJob (row, unitRow);
    }
}

void Processor::requestAllUnits()
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (files.getChild (unitIndex).hasProperty (Id::arguments))
            requestUnit (unitIndex);

    triggerAsyncUpdate();
}

void Processor::handleAsyncUpdate()
{
    setUnitsJobs();

    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    for (const auto rowIndex : getFinishedRows (parseJobs))
    {
        if (not nextParseJobs.contains (rowIndex))
            addDocument (*parseJobs.at (rowIndex));

        removeJob (parseJobs, nextParseJobs, rowIndex);
        setIndexed (rowIndex);
    }

    for (const auto rowIndex : getFinishedRows (indexJobs))
    {
        if (not nextIndexJobs.contains (rowIndex))
        {
            addBindings (*indexJobs.at (rowIndex));
        }

        pool->removeJob (indexJobs.at (rowIndex).get(), true, waitForJob);
        indexJobs.erase (rowIndex);
        setUnitIndexed (files.getChild (rowIndex), rowIndex);
    }

    addIndexJobs();
    setIndexed();

    if (static_cast<bool> (files[Id::isIndexed]))
        index.setDocuments();
}

void Processor::addDocument (ParseJob& job)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto row { files.getChild (job.rowIndex) };

    if (row[Id::path].toString().compare (job.path) == 0 and juce::File { job.path }.getLastModificationTime() == job.modificationTime
        and job.language == index.languages.at (static_cast<int> (row[Id::family])))
    {
        if (job.buffer.has_value() or job.document != nullptr)
            index.addDocument (job.path, std::move (job.document), job.sourceHash);
        else if (job.sourceHash != 0 and job.result.wasOk())
            index.addSourceHash (job.path, job.sourceHash);
        else
            index.addDocument (job.path, nullptr, 0);

        if (job.symbols != nullptr) index.addBindings (job.rowIndex, std::move (job.symbols));
        if (not job.result.wasOk()) model.setMessage (job.path + Id::diagnosticSeparator + job.result.getErrorMessage());

        if (job.buffer.has_value())
        {
            if (const auto unitRow { index.unitOwners->getRow (job.path) }; unitRow != Index::UnitOwners::noRow)
            {
                const auto unit { files.getChild (unitRow) };
                const auto family { static_cast<int> (unit[Id::family]) };

                if (not hasParseJob (unit, unitRow, job.rowIndex) and index.languages.contains (family) and index.languages.at (family)->hasIndex())
                    addIndexJob (unit, unitRow);
            }
        }
        else
            for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
                if (const auto unit { files.getChild (unitIndex) }; hasDependency (unit, unitIndex, job.rowIndex))
                    updateUnitOnDiskEdit (unit, unitIndex, job.rowIndex);
    }
}

bool Processor::isUnitStale (const juce::ValueTree& unit, int unitIndex) const
{
    jassert (unit.isValid());

    const auto unitPathIndex { index.paths.indexOf (unit[Id::path].toString(), false, 0) };

    if (const auto bound { index.symbolDocuments.find (unitPathIndex) }; bound != index.symbolDocuments.end())
    {
        const auto& [boundPathIndex, symbolDocument] { *bound };

        return symbolDocument->getHash() != getUnitStamp (unit, unitIndex, getArguments (unit));
    }

    return true;
}

void Processor::updateUnitOnDiskEdit (const juce::ValueTree& unit, int unitIndex, int finishedRowIndex)
{
    jassert (unit.isValid());

    const auto family { static_cast<int> (unit[Id::family]) };

    if (isUnitStale (unit, unitIndex)
        and not hasParseJob (unit, unitIndex, finishedRowIndex)
        and index.languages.contains (family)
        and index.languages.at (family)->hasIndex()
        and (static_cast<bool> (unit[Id::isRequested]) or hasOpenBuffer (unit)))
    {
        addIndexJob (unit, unitIndex);
    }
}

void Processor::addBindings (IndexJob& job)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    auto row { files.getChild (job.rowIndex) };

    if (row[Id::path].toString().compare (job.path) == 0)
    {
        if (job.symbols != nullptr)
            index.addBindings (job.rowIndex, std::move (job.symbols));
        else if (job.result.wasOk())
            job.result = index.addBindings (job.rowIndex, *job.temporary, job.hash, job.languageHash, job.family);

        if (not job.result.wasOk())
        {
            model.setMessage (job.path + Id::diagnosticSeparator + job.result.getErrorMessage());
            row.setProperty (Id::message, job.result.getErrorMessage(), nullptr);
        }
    }
}

void Processor::addUnitsJob()
{
    const auto toolchain { model.state.getChildWithName (jam::Format::toType (Id::toolchain)) };

    if (toolchain.isValid())
    {
        setJob (unitsJobs, nextUnitsJobs, model.state.indexOf (toolchain), getUnitsJob (toolchain));
        setIndexed();
    }
}

std::unique_ptr<Processor::UnitsJob> Processor::getUnitsJob (const juce::ValueTree& toolchain)
{
    const auto manifest { model.state[Id::manifest].toString() };
    const auto toolchainArgument { model.state[Id::toolchainArgument].toString() };
    jam::Array<char> arguments;
    jam::Array<int> argumentCounts;
    jam::HashMap<juce::String, int> families;
    jam::Array<juce::File> roots;

    for (const auto& row : toolchain)
    {
        jam::Strings rowArguments (jam::ChildProcess::getArguments (jam::Strings::getView (row[Id::flag].toString())));

        rowArguments.insert (0, row[Id::command].toString());
        argumentCounts.add (rowArguments.size());

        for (int argumentIndex { 0 }; argumentIndex < rowArguments.size(); ++argumentIndex)
            addArgument (arguments, rowArguments.getView (argumentIndex));
    }

    for (const auto& language : model.state)
        if (language.hasType (jam::Format::toType (Id::language)))
            for (const auto& extension : *language[Id::extensions].getArray())
                families.addOrReplace (extension.toString(), static_cast<int> (language[Id::family]));

    for (const auto& root : model.state.getChildWithName (jam::Format::toType (Id::roots)))
        roots.add (juce::File { root[Id::path].toString() });

    return std::make_unique<UnitsJob> (*this, std::move (arguments), std::move (argumentCounts), LINGModel::getProjectDirectory (juce::File { manifest }).getFullPathName(),
                                       LINGModel::getCacheDirectory (juce::File { manifest }, jam::Strings::getView (toolchainArgument)).getFullPathName(),
                                       index.languages, std::move (families), std::move (roots));
}

void Processor::setUnitsJobs()
{
    for (const auto rowIndex : getFinishedRows (unitsJobs))
    {
        if (not nextUnitsJobs.contains (rowIndex))
            setFiles (unitsJobs.at (rowIndex)->files);

        removeJob (unitsJobs, nextUnitsJobs, rowIndex);
    }
}

void Processor::setFiles (const juce::ValueTree& files)
{
    if (files.hasProperty (Id::message))
        model.setMessage (files[Id::message].toString());

    model.removeChild (model.state.getChildWithName (jam::Format::toType (Id::files)));
    model.appendChild (files);
}

Processor::UnitsJob::UnitsJob (Processor& ownerToUse, jam::Array<char> argumentsToUse, jam::Array<int> argumentCountsToUse, const juce::String& directoryToUse, const juce::String& cacheDirectoryToUse, jam::HashMap<int, std::shared_ptr<const Language>> languagesToUse, jam::HashMap<juce::String, int> familiesToUse, jam::Array<juce::File> rootsToUse)
    : juce::ThreadPoolJob (directoryToUse),
      owner (ownerToUse),
      arguments (std::move (argumentsToUse)),
      argumentCounts (std::move (argumentCountsToUse)),
      directory (directoryToUse),
      cacheDirectory (cacheDirectoryToUse),
      languages (std::move (languagesToUse)),
      families (std::move (familiesToUse)),
      roots (std::move (rootsToUse))
{
}

juce::ThreadPoolJob::JobStatus Processor::UnitsJob::runJob()
{
#if JAM_USING_PROFILER
    const jam::Profiler::Zone zone { Id::getFiles };
#endif

    try
    {
        const auto pointers (getArgumentPointers (arguments));
        jam::LanguageCallbacks callbacks {};

        callbacks.context = this;
        callbacks.addUnit = &UnitsJob::addUnit;
        callbacks.addBuildFile = &UnitsJob::addBuildFile;
        callbacks.addMessage = &UnitsJob::addMessage;

        for (const auto& [family, language] : languages)
            language->addUnits (pointers.data(), argumentCounts.data(), argumentCounts.size(), directory.toRawUTF8(), cacheDirectory.toRawUTF8(), callbacks);

        files.setProperty (Id::buildFiles, buildFiles, nullptr);
    }
    catch (const std::exception& error)
    {
        files.setProperty (Id::message, juce::String (error.what()), nullptr);
    }

    isFinished.store (true, std::memory_order_release);
    owner.triggerAsyncUpdate();

    return jobHasFinished;
}

void Processor::UnitsJob::addUnit (void* context, const char* path, const char* directory, const char* output, const char* const* arguments, int argumentCount, const char* const* dependencies, int dependencyCount, const char* const* prerequisites, int prerequisiteCount)
{
    static constexpr int noFamily { -1 };

    auto& job { *static_cast<UnitsJob*> (context) };
    const juce::String unitPath { juce::String::fromUTF8 (path) };
    const auto extension { juce::File { unitPath }.getFileExtension().substring (1) };
    const auto family { job.families.contains (extension) ? job.families.at (extension) : noFamily };
    const juce::String unitOutput { juce::String::fromUTF8 (output) };
    auto row { job.files.getChild (job.getOrCreatePathIndex (unitPath, family)) };
    juce::Array<juce::var> unitArguments;

    for (int argumentIndex { 0 }; argumentIndex < argumentCount; ++argumentIndex)
        unitArguments.add (juce::String::fromUTF8 (arguments[argumentIndex]));

    row.setProperty (Id::directory, juce::String::fromUTF8 (directory), nullptr);

    if (unitOutput.isNotEmpty())
        row.setProperty (Id::output, unitOutput, nullptr);

    row.setProperty (Id::arguments, unitArguments, nullptr);
    row.setProperty (Id::dependencies, job.getOrCreateDependencyIndices (dependencies, dependencyCount, static_cast<int> (row[Id::family])), nullptr);
    row.setProperty (Id::prerequisites, job.getOrCreatePathIndices (prerequisites, prerequisiteCount, static_cast<int> (row[Id::family])), nullptr);
}

void Processor::UnitsJob::addBuildFile (void* context, const char* path)
{
    auto& job { *static_cast<UnitsJob*> (context) };

    job.buildFiles.add (juce::File { juce::String::fromUTF8 (path) }.getFullPathName());
}

void Processor::UnitsJob::addMessage (void* context, const char* text)
{
    auto& job { *static_cast<UnitsJob*> (context) };

    job.files.setProperty (Id::message, juce::String::fromUTF8 (text), nullptr);
}

int Processor::UnitsJob::getOrCreatePathIndex (const juce::String& path, int family)
{
    const auto fullPath { juce::File { path }.getFullPathName() };

    if (const auto pathIndex { paths.indexOf (fullPath, false, 0) }; pathIndex >= 0)
        return pathIndex;

    juce::ValueTree row { jam::Format::toType (Id::file) };

    row.setProperty (Id::path, fullPath, nullptr);
    row.setProperty (Id::family, family, nullptr);
    files.appendChild (row, nullptr);
    paths.add (fullPath);
    return paths.size() - 1;
}

juce::Array<juce::var> Processor::UnitsJob::getOrCreatePathIndices (const char* const* filePaths, int filePathCount, int family)
{
    juce::Array<juce::var> pathIndices;

    for (int argumentIndex { 0 }; argumentIndex < filePathCount; ++argumentIndex)
        pathIndices.add (getOrCreatePathIndex (juce::String::fromUTF8 (filePaths[argumentIndex]), family));

    return pathIndices;
}

juce::Array<juce::var> Processor::UnitsJob::getOrCreateDependencyIndices (const char* const* filePaths, int filePathCount, int family)
{
    juce::Array<juce::var> pathIndices;

    for (int argumentIndex { 0 }; argumentIndex < filePathCount; ++argumentIndex)
    {
        const juce::String filePath { juce::String::fromUTF8 (filePaths[argumentIndex]) };

        if (isWithinRoots (juce::File { filePath }))
            pathIndices.add (getOrCreatePathIndex (filePath, family));
    }

    return pathIndices;
}

bool Processor::UnitsJob::isWithinRoots (const juce::File& file) const
{
    return LINGModel::isWithinRoots (roots, file);
}
