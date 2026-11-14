#include "Processor.h"
#include "SwapValidator.h"

Processor::Processor (LINGModel& newModel)
    : model (newModel),
      index (model)
{
    model.addListener (this);
    registerEvents();
}

Processor::~Processor()
{
    model.removeListener (this);
}

bool Processor::hasDependency (const juce::ValueTree& unit, int unitIndex, int rowIndex)
{
    const auto* dependencies { unit[Id::dependencies].getArray() };

    return unit.hasProperty (Id::arguments)
       and (unitIndex == rowIndex
            or (dependencies != nullptr and std::any_of (dependencies->begin(), dependencies->end(), [rowIndex] (const juce::var& dependency) { return static_cast<int> (dependency) == rowIndex; })));
}

juce::ValueTree Processor::getUnit (int pathIndex) const
{
    if (pathIndex < 0) return {};

    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto row { files.getChildWithProperty (Id::path, index.paths.at (pathIndex)) };

    if (row.hasProperty (Id::arguments)) return row;

    const auto rowIndex { files.indexOf (row) };

    for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
        if (const auto unit { files.getChild (unitIndex) }; hasDependency (unit, unitIndex, rowIndex))
            return unit;

    return {};
}

juce::ValueTree Processor::getHeaderEntry (const juce::ValueTree& unit) const
{
    const auto files { unit.getParent() };
    const juce::File directory { unit[Id::directory].toString() };

    for (const auto& argument : *unit[Id::arguments].getArray())
    {
        const auto headerEntry { files.getChildWithProperty (Id::output, directory.getChildFile (argument.toString()).getFullPathName()) };

        if (headerEntry.isValid() and headerEntry != unit) return headerEntry;
    }

    return {};
}

jam::Array<juce::ValueTree> Processor::getHeaderEntries (const juce::ValueTree& files) const
{
    jam::Array<juce::ValueTree> units;
    jam::Array<juce::ValueTree> headerEntries;

    for (const auto& row : files)
        if (row.hasProperty (Id::arguments))
            units.add (row);

    for (const auto& unit : units)
    {
        const juce::File directory { unit[Id::directory].toString() };

        for (const auto& argument : *unit[Id::arguments].getArray())
        {
            const auto fullPath { directory.getChildFile (argument.toString()).getFullPathName() };

            for (const auto& headerEntry : units)
                if (headerEntry != unit and headerEntry[Id::output].toString().compare (fullPath) == 0)
                    headerEntries.addIfNotAlreadyThere (headerEntry);
        }
    }

    return headerEntries;
}

jam::Strings Processor::getArguments (const juce::ValueTree& unit, const juce::ValueTree& headerEntry) const
{
    const juce::File directory { unit[Id::directory].toString() };
    const auto output { headerEntry[Id::output].toString() };
    jam::Strings arguments;

    for (const auto& argument : *unit[Id::arguments].getArray())
    {
        if (headerEntry.isValid() and directory.getChildFile (argument.toString()).getFullPathName().compare (output) == 0)
            arguments.add (getPrecompiledHeader (headerEntry).getFullPathName());
        else
            arguments.add (argument.toString());
    }

    return arguments;
}

jam::Strings Processor::getCheckArguments (const juce::ValueTree& unit) const
{
    auto arguments { getArguments (unit, getHeaderEntry (unit)) };
    const auto checkFlags { model.state.getChildWithProperty (Id::family, unit[Id::family])[Id::checkFlags] };

    for (const auto& flag : *checkFlags.getArray())
        arguments.add (flag.toString());

    return arguments;
}

juce::File Processor::getPrecompiledHeader (const juce::ValueTree& headerEntry) const
{
    const auto output { headerEntry[Id::output].toString() };
    const auto manifest { model.state[Id::manifest].toString() };
    const auto toolchainArgument { model.state[Id::toolchainArgument].toString() };
    const auto bytes { jam::Strings::getView (output) };
    std::string name;

    jam::Format::appendHex (name, jam::Wyhash::hashBytes (bytes.data(), bytes.size()));

    return LINGModel::getCacheDirectory (juce::File { manifest }, jam::Strings::getView (toolchainArgument)).getChildFile (jam::Format::toFileName (name.c_str(), juce::File { output }.getFileExtension()));
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
    events.add (Id::diagnostics, [this] (const juce::Identifier&, juce::ValueTree& request) { diagnostics (request); });
    events.add (Id::switchSourceHeader, [this] (const juce::Identifier&, juce::ValueTree& request) { query.switchSourceHeader (request, index); });
    events.add (Id::inlayHint, [this] (const juce::Identifier&, juce::ValueTree& request) { query.inlayHint (request, index); });
}

void Processor::runRequest (juce::ValueTree& request)
{
    jassert (request[Id::arguments].isArray());

    const auto word { request[Id::command].toString() };
    const juce::Identifier key { word.isNotEmpty() ? juce::Identifier { word } : juce::Identifier {} };

    if (events.contains (key))
    {
        events.get (key, request);
    }
    else
    {
        model.setMessage (text::Diagnostics::failArguments);
        query.setResult (request, jam::Array<std::pair<int, const jam::Document::Element*>> {}, index);
    }
}

void Processor::diagnostics (juce::ValueTree& request)
{
    const auto unit { getUnit (query.getPathIndex (request, index)) };

    if (unit.isValid())
    {
        const auto headerEntry { getHeaderEntry (unit) };

        if (headerEntry.isValid() and not getPrecompiledHeader (headerEntry).existsAsFile())
            addPrecompile (headerEntry, [this, request, unit] () mutable { addCheck (request, unit); });
        else
            addCheck (request, unit);
    }
    else
    {
        model.setMessage (Query::getFile (request).getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failNoUnit);
        query.setResult (request, std::string_view {});
    }
}

void Processor::addCheck (juce::ValueTree& request, const juce::ValueTree& unit)
{
    auto* subprocess { checks.add (std::make_unique<jam::Subprocess>()).get() };

    subprocess->launch (getCheckArguments (unit), juce::File { unit[Id::directory].toString() }, [this, subprocess, request] (int, const std::string& capture) mutable
    {
        juce::MessageManager::callAsync ([this, subprocess, request, capture] () mutable
        {
            query.setResult (request, capture);
            checks.remove (checks.indexOf (subprocess));
        });
    });
}

void Processor::addPrecompile (const juce::ValueTree& headerEntry)
{
    addPrecompile (headerEntry, [] {});
}

void Processor::addPrecompile (const juce::ValueTree& headerEntry, std::function<void()> onComplete)
{
    const auto manifest { model.state[Id::manifest].toString() };
    const auto toolchainArgument { model.state[Id::toolchainArgument].toString() };

    LINGModel::getCacheDirectory (juce::File { manifest }, jam::Strings::getView (toolchainArgument)).createDirectory();

    auto* subprocess { precompiles.add (std::make_unique<jam::Subprocess>()).get() };

    subprocess->launch (getArguments (headerEntry, headerEntry), juce::File { headerEntry[Id::directory].toString() }, [this, subprocess, onComplete] (int exitCode, const std::string& capture)
    {
        juce::MessageManager::callAsync ([this, subprocess, onComplete, exitCode, capture]
        {
            if (exitCode != 0)
                model.setMessage (jam::Strings::fromView (capture));

            precompiles.remove (precompiles.indexOf (subprocess));
            onComplete();
            triggerAsyncUpdate();
        });
    });
}

void Processor::addBuild (const juce::ValueTree& languageRow)
{
    static constexpr int failedCopyExitCode { -1 };

    const auto family { static_cast<int> (languageRow[Id::family]) };

    if (not builds.contains (family))
    {
        const auto cache { LINGModel::getCacheDirectory().getChildFile (files::language).getChildFile (languageRow[Id::name].toString()) };
        const juce::File config { languageRow[Id::path].toString() };
        const auto startTime { juce::Time::getCurrentTime() };

        if (config.copyDirectoryTo (cache))
        {
            auto [buildEntry, inserted] { builds.try_emplace (family, std::make_unique<jam::Subprocess>()) };
            auto& [buildFamily, subprocess] { *buildEntry };

            subprocess->launch (jam::Strings { Id::cast.toString(), files::languageSpell }, cache, [this, languageRow, cache, startTime] (int exitCode, const std::string& capture)
            {
                juce::MessageManager::callAsync ([this, languageRow, cache, startTime, exitCode, capture]
                {
                    applyBuild (languageRow, cache, startTime, exitCode, capture);
                });
            });
        }
        else
        {
            applyBuild (languageRow, cache, startTime, failedCopyExitCode, cache.getFullPathName().toStdString());
        }
    }
}

void Processor::applyBuild (const juce::ValueTree& languageRow, const juce::File& cache, juce::Time startTime, int exitCode, const std::string& capture)
{
    builds.erase (static_cast<int> (languageRow[Id::family]));

    if (exitCode == 0)
        setLanguage (languageRow, cache);
    else
        model.setMessage (juce::String (text::Diagnostics::failLanguageBuild) + Id::diagnosticSeparator + jam::Strings::fromView (capture));

    if (isChangedSince (juce::File { languageRow[Id::path].toString() }, startTime))
        addBuild (languageRow);

    triggerAsyncUpdate();
}

bool Processor::isChangedSince (const juce::File& folder, juce::Time time)
{
    const auto files (folder.findChildFiles (juce::File::findFiles, false));

    return std::any_of (files.begin(), files.end(), [time] (const juce::File& file) { return file.getLastModificationTime() > time; });
}

void Processor::setLanguage (juce::ValueTree languageRow, const juce::File& cache)
{
    static constexpr const char* entryName { "ling_get_language" };

#if JUCE_WINDOWS
    const auto build { cache.getChildFile (files::languageLibraryWindows) };
#else
    const auto build { cache.getChildFile (files::languageLibrary) };
#endif

    juce::MemoryBlock bytes;

    build.loadFileAsData (bytes);

    const auto hash { static_cast<juce::int64> (jam::Wyhash::hashBytes (bytes.getData(), bytes.getSize())) };
    const auto library { cache.getChildFile (juce::String (hash)).withFileExtension (build.getFileExtension()) };

    if (not library.existsAsFile())
        library.replaceWithData (bytes.getData(), bytes.getSize());

    if (auto language { jam::LanguageLibrary::getLanguageLibrary (library, entryName) }; language != nullptr)
    {
        juce::Array<juce::var> checkFlags;

        index.languages.addOrReplace (static_cast<int> (languageRow[Id::family]), language);

        for (const auto& flag : CompilerManifest::getCheckFlags (model, static_cast<int> (languageRow[Id::family])))
            checkFlags.add (flag);

        languageRow.setProperty (Id::checkFlags, checkFlags, nullptr);
        languageRow.setProperty (Id::hash, hash, nullptr);
    }
    else
    {
        model.setMessage (juce::String (text::Diagnostics::failLanguageLoad) + Id::diagnosticSeparator + library.getFullPathName());
    }
}

void Processor::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& childWhichHasBeenAdded)
{
    if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::files)))
    {
        index.removeDocuments (childWhichHasBeenAdded);
        nextIndexJobs.clear();
        addLanguages (childWhichHasBeenAdded);

        for (int rowIndex { 0 }; rowIndex < childWhichHasBeenAdded.getNumChildren(); ++rowIndex)
        {
            const auto row { childWhichHasBeenAdded.getChild (rowIndex) };

            if (index.languages.contains (static_cast<int> (row[Id::family])))
                addJob (row, rowIndex);
        }

        for (const auto& headerEntry : getHeaderEntries (childWhichHasBeenAdded))
            addPrecompile (headerEntry);

        triggerAsyncUpdate();
    }
    else if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::request)))
    {
        runRequest (childWhichHasBeenAdded);
    }
    else if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::language)))
    {
        addBuild (childWhichHasBeenAdded);
    }
}

void Processor::addLanguages (const juce::ValueTree& files)
{
    for (const auto& row : files)
    {
        if (not model.state.getChildWithProperty (Id::family, row[Id::family]).isValid())
        {
            const auto name { map::Family::getInstance()->get (static_cast<int> (row[Id::family])) };
            const auto folder { LINGModel::getConfigDirectory().getChildFile (files::language).getChildFile (name) };
            juce::ValueTree languageRow { jam::Format::toType (Id::language) };

            folder.createDirectory();
            languageRow.setProperty (Id::name, name, nullptr);
            languageRow.setProperty (Id::family, row[Id::family], nullptr);
            languageRow.setProperty (Id::path, folder.getFullPathName(), nullptr);
            languageRow.setProperty (Id::checkFlags, juce::Array<juce::var> {}, nullptr);
            model.state.appendChild (languageRow, nullptr);
        }
    }
}

void Processor::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == Id::path and tree.hasType (jam::Format::toType (Id::file)))
        setFile (tree);
    else if (property == Id::path and tree.hasType (jam::Format::toType (Id::language)))
        addBuild (tree);
    else if (property == Id::hash and tree.hasType (jam::Format::toType (Id::language)))
        setLanguageHash (tree);
}

void Processor::setFile (const juce::ValueTree& file)
{
    const auto files { file.getParent() };
    const auto rowIndex { files.indexOf (file) };

    if (index.languages.contains (static_cast<int> (file[Id::family])))
        addJob (file, rowIndex);

    for (const auto& headerEntry : getHeaderEntries (files))
    {
        if (hasDependency (headerEntry, files.indexOf (headerEntry), rowIndex))
        {
            getPrecompiledHeader (headerEntry).deleteFile();
            addPrecompile (headerEntry);
        }
    }
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

Processor::ParseJob::ParseJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::File& swapFileToUse, std::shared_ptr<const jam::LanguageLibrary> languageToUse, juce::int64 languageHashToUse)
    : juce::ThreadPoolJob (pathToUse),
      owner (ownerToUse),
      rowIndex (rowIndexToUse),
      path (pathToUse),
      swapFile (swapFileToUse),
      language (std::move (languageToUse)),
      languageHash (languageHashToUse)
{
}

juce::ThreadPoolJob::JobStatus Processor::ParseJob::runJob()
{
    const juce::File file { path };

    modificationTime = file.getLastModificationTime();

    if (file.existsAsFile())
    {
        const auto text { file.loadFileAsString() };
        const auto view { jam::Strings::getView (text) };
        const auto hash { static_cast<juce::int64> (jam::Wyhash::hashBytes (view.data(), view.size())) };

        document = Index::readSwapFile (swapFile, view, language, hash, languageHash);

        if (document != nullptr)
        {
            const auto swapResult { SwapValidator().isValid (*document) };

            result = swapResult.wasOk() ? swapResult : juce::Result::fail (swapFile.getFullPathName() + Id::diagnosticSeparator + swapResult.getErrorMessage());
        }
        else
        {
            document = jam::LanguageDocument::parse (view, language);
            result = document == nullptr ? jam::LanguageDocument::isValid (view, language) : juce::Result::ok();
            isParsed = true;
            Index::writeSwapFile (swapFile, document, hash, languageHash);
        }
    }
    else
    {
        swapFile.deleteFile();
    }

    isFinished.store (true, std::memory_order_release);
    owner.triggerAsyncUpdate();

    return jobHasFinished;
}

Processor::IndexJob::IndexJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::String& directoryToUse, jam::Array<char> argumentsToUse, std::shared_ptr<const jam::LanguageLibrary> languageToUse)
    : juce::ThreadPoolJob (pathToUse),
      owner (ownerToUse),
      rowIndex (rowIndexToUse),
      path (pathToUse),
      directory (directoryToUse),
      arguments (std::move (argumentsToUse)),
      language (std::move (languageToUse))
{
}

juce::ThreadPoolJob::JobStatus Processor::IndexJob::runJob()
{
    jam::Array<const char*> pointers;
    jam::LanguageCallbacks callbacks {};

    for (int position { 0 }; position < arguments.size(); ++position)
        if (position == 0 or arguments.at (position - 1) == static_cast<char> (Chars::nullCharacter))
            pointers.add (&arguments.at (position));

    callbacks.context = this;
    callbacks.addSymbol = &IndexJob::addSymbol;
    callbacks.shouldExit = [] (void* context) { return static_cast<IndexJob*> (context)->shouldExit() ? 1 : 0; };

    result = language->index (pointers.data(), pointers.size(), directory.toRawUTF8(), callbacks);
    isFinished.store (true, std::memory_order_release);
    owner.triggerAsyncUpdate();

    return jobHasFinished;
}

void Processor::IndexJob::addSymbol (void* context, const char* recordPath, int line, int column, const char* symbol, int role)
{
    auto& job { *static_cast<IndexJob*> (context) };
    auto [pathEntry, pathInserted] { job.records.try_emplace (std::string (recordPath)) };
    auto& [pathKey, lines] { *pathEntry };
    auto [lineEntry, lineInserted] { lines.try_emplace (line) };
    auto& [lineKey, columns] { *lineEntry };

    columns.try_emplace (column, symbol, role);
}

void Processor::addJob (const juce::ValueTree& row, int rowIndex)
{
    const auto family { static_cast<int> (row[Id::family]) };
    const auto swapFile { model.getSwapFile (juce::File { row[Id::path].toString() }) };

    swapFile.getParentDirectory().createDirectory();

    setJob (parseJobs, nextParseJobs, rowIndex, std::make_unique<ParseJob> (*this, rowIndex, row[Id::path].toString(), swapFile, index.languages.at (family), static_cast<juce::int64> (model.state.getChildWithProperty (Id::family, family)[Id::hash])));
}

void Processor::addIndexJob (const juce::ValueTree& row, int rowIndex)
{
    jam::Array<char> arguments;

    for (const auto& argument : *row[Id::arguments].getArray())
    {
        const auto text { argument.toString() };
        const auto view { jam::Strings::getView (text) };

        arguments.insertArray (arguments.size(), view.data(), static_cast<int> (view.size()));
        arguments.add (static_cast<char> (Chars::nullCharacter));
    }

    if (const auto running { indexJobs.find (rowIndex) }; running != indexJobs.end())
    {
        const auto& [runningRow, runningJob] { *running };

        if (pool.removeJob (runningJob.get(), true, noWait))
            indexJobs.erase (rowIndex);
    }

    nextIndexJobs.addOrReplace (rowIndex, std::make_unique<IndexJob> (*this, rowIndex, row[Id::path].toString(), row[Id::directory].toString(), std::move (arguments), index.languages.at (static_cast<int> (row[Id::family]))));
}

void Processor::addIndexJobs()
{
    jam::Array<int> rows;

    for (const auto& [rowIndex, job] : nextIndexJobs)
        if (not indexJobs.contains (rowIndex))
            rows.add (rowIndex);

    for (const auto rowIndex : rows)
    {
        indexJobs.addOrReplace (rowIndex, std::move (nextIndexJobs.at (rowIndex)));
        nextIndexJobs.erase (rowIndex);
        pool.addJob (indexJobs.at (rowIndex).get(), false);
    }
}

void Processor::handleAsyncUpdate()
{
    const auto getFinishedRows { [] (const auto& jobs)
    {
        jam::Array<int> rows;

        for (const auto& [rowIndex, job] : jobs)
            if (job->isFinished.load (std::memory_order_acquire))
                rows.add (rowIndex);

        std::sort (rows.begin(), rows.end());
        return rows;
    } };

    for (const auto rowIndex : getFinishedRows (parseJobs))
    {
        if (not nextParseJobs.contains (rowIndex))
            addDocument (*parseJobs.at (rowIndex));

        removeJob (parseJobs, nextParseJobs, rowIndex);
    }

    for (const auto rowIndex : getFinishedRows (indexJobs))
    {
        if (not nextIndexJobs.contains (rowIndex))
            addBindings (*indexJobs.at (rowIndex));

        pool.removeJob (indexJobs.at (rowIndex).get(), true, waitForJob);
        indexJobs.erase (rowIndex);
    }

    if (parseJobs.empty())
        addIndexJobs();

    if (parseJobs.empty() and indexJobs.empty() and builds.empty() and precompiles.isEmpty())
        index.setDocuments();
}

void Processor::addDocument (ParseJob& job)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto row { files.getChild (job.rowIndex) };

    if (row[Id::path].toString().compare (job.path) == 0 and juce::File { job.path }.getLastModificationTime() == job.modificationTime
        and job.language == index.languages.at (static_cast<int> (row[Id::family])))
    {
        const auto isIndexNeeded { job.isParsed and job.document != nullptr };

        index.addDocument (job.path, std::move (job.document));

        if (not job.result.wasOk())
            model.setMessage (job.path + Id::diagnosticSeparator + job.result.getErrorMessage());

        if (isIndexNeeded)
        {
            for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
            {
                const auto unit { files.getChild (unitIndex) };

                if (not nextIndexJobs.contains (unitIndex) and hasDependency (unit, unitIndex, job.rowIndex) and index.languages.contains (static_cast<int> (unit[Id::family])))
                    addIndexJob (unit, unitIndex);
            }
        }
    }
}

void Processor::addBindings (IndexJob& job)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto row { files.getChild (job.rowIndex) };

    if (row[Id::path].toString().compare (job.path) == 0)
    {
        if (job.result == 0)
        {
            index.addBindings (job.rowIndex, std::move (job.records));
        }
        else
        {
            model.setMessage (job.path + Id::diagnosticSeparator + text::Diagnostics::failUnitIndex);
            model.getSwapFile (juce::File { job.path }).deleteFile();

            if (const auto* dependencies { row[Id::dependencies].getArray() }; dependencies != nullptr)
                for (const auto& dependency : *dependencies)
                    model.getSwapFile (juce::File { files.getChild (static_cast<int> (dependency))[Id::path].toString() }).deleteFile();
        }
    }
}
