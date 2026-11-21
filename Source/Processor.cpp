#include "Processor.h"
#include "LibraryLanguage.h"
#include "languages/MarkdownLanguage.h"
#include "languages/MermaidLanguage.h"
#include "languages/JsonLanguage.h"
#include "languages/CssLanguage.h"
#include "languages/XmlLanguage.h"
#include "languages/HtmlLanguage.h"

static juce::ThreadPool::Options getPoolOptions (int threadCount)
{
    return juce::ThreadPool::Options {}.withNumberOfThreads (threadCount).withDesiredThreadPriority (juce::Thread::Priority::low);
}

Processor::Processor (LINGModel& newModel)
    : model (newModel),
      index (model),
      pool (std::make_unique<juce::ThreadPool> (getPoolOptions (LINGModel::defaultThreads)))
{
    model.addListener (this);
    registerEvents();
    addLanguages();
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

bool Processor::hasParseJob (const juce::ValueTree& unit, int unitIndex, int rowIndex) const
{
    const auto* dependencies { unit[Id::dependencies].getArray() };
    const auto isParsing { [this, rowIndex] (int dependency) { return dependency != rowIndex and (parseJobs.contains (dependency) or nextParseJobs.contains (dependency)); } };

    return isParsing (unitIndex)
        or (dependencies != nullptr and std::any_of (dependencies->begin(), dependencies->end(), [&isParsing] (const juce::var& dependency) { return isParsing (static_cast<int> (dependency)); }));
}

juce::int64 Processor::getUnitHash (const juce::ValueTree& unit, int unitIndex, const jam::Array<char>& arguments) const
{
    const auto files { unit.getParent() };
    const auto* dependencies { unit[Id::dependencies].getArray() };
    const auto getSourceHash { [this, &files] (int rowIndex) -> std::uint64_t
    {
        const auto pathIndex { index.paths.indexOf (files.getChild (rowIndex)[Id::path].toString(), false, 0) };

        if (const auto stored { index.documents.find (pathIndex) }; stored != index.documents.end())
        {
            const auto& [storedIndex, document] { *stored };

            return static_cast<std::uint64_t> (*document->getRoot()->get<juce::int64> (Id::hash));
        }

        return 0;
    } };
    auto hash { jam::Wyhash::fold (jam::Wyhash::hashBytes (arguments.data(), static_cast<size_t> (arguments.size())), getSourceHash (unitIndex)) };

    if (dependencies != nullptr)
        for (const auto& dependency : *dependencies)
            hash = jam::Wyhash::fold (hash, getSourceHash (static_cast<int> (dependency)));

    return static_cast<juce::int64> (hash);
}

juce::ValueTree Processor::getUnit (const juce::String& path) const
{
    if (const auto entry { index.unitRows.find (path) }; entry != index.unitRows.end())
    {
        const auto& [unitPath, unitRow] { *entry };

        return model.state.getChildWithName (jam::Format::toType (Id::files)).getChild (unitRow);
    }

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
    events.add (Id::hover, [this] (const juce::Identifier&, juce::ValueTree& request) { runHover (request); });
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

    events.add (Id::didOpen, [this] (const juce::Identifier&, juce::ValueTree& request) { setBuffer (request); });
    events.add (Id::didChange, [this] (const juce::Identifier&, juce::ValueTree& request) { setBuffer (request); });
    events.add (Id::didClose, [this] (const juce::Identifier&, juce::ValueTree& request) { removeBuffer (request); });

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
        jam::debug::Log::write ("isIndexedEvent", tree[Id::path].toString(), static_cast<bool> (tree[Id::isIndexed]), tree.hasType (jam::Format::toType (Id::file)));

        if (static_cast<bool> (tree[Id::isIndexed]))
        {
            runRequests();

            if (tree.hasType (jam::Format::toType (Id::file)))
                addDiagnosticRequests (tree);
        }
    });
    propertyEvents.add (Id::threads, [this] (const juce::Identifier&, juce::ValueTree&)
    {
        pool->removeAllJobs (true, waitForJob);
        parseJobs.clear();
        nextParseJobs.clear();
        indexJobs.clear();
        nextIndexJobs.clear();
        unitsJobs.clear();
        nextUnitsJobs.clear();

        pool = std::make_unique<juce::ThreadPool> (getPoolOptions (static_cast<int> (model.state[Id::threads])));

        addUnitsJob();

        if (const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) }; files.isValid())
            addJobs (files);
    });

    childEvents.add (jam::Format::toType (Id::files), [this] (const juce::Identifier&, juce::ValueTree& files)
    {
        addJobs (files);
    });
    childEvents.add (jam::Format::toType (Id::request), [this] (const juce::Identifier&, juce::ValueTree& request)
    {
        jam::debug::Log::write ("request", request[Id::command].toString(), Query::getFile (request).getFullPathName(), isRequestReady (request));

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

bool Processor::isRequestReady (const juce::ValueTree& request) const
{
    static const map::Readiness readiness;

    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto word { request[Id::command].toString() };
    const auto hasIndexedDependencies { [this, &files] (const juce::String& path)
    {
        if (const auto unit { index.unitRows.find (path) }; unit != index.unitRows.end())
        {
            const auto& [unitPath, unitRow] { *unit };

            if (const auto* dependencies { files.getChild (unitRow)[Id::dependencies].getArray() }; dependencies != nullptr)
                return std::all_of (dependencies->begin(), dependencies->end(), [&files] (const juce::var& dependency) { return static_cast<bool> (files.getChild (static_cast<int> (dependency))[Id::isIndexed]); });
        }

        return true;
    } };

    if (word.isNotEmpty())
    {
        if (const auto entry { readiness.find (juce::Identifier { word }) }; entry != readiness.end())
        {
            const auto& [operation, scope] { *entry };
            const auto path { Query::getFile (request).getFullPathName() };

            if (const auto row { index.pathRows.find (path) }; scope != Id::files and row != index.pathRows.end())
            {
                const auto& [rowPath, rowIndex] { *row };

                return static_cast<bool> (files.getChild (rowIndex)[Id::isIndexed]) and (scope == Id::file or hasIndexedDependencies (path));
            }
        }
    }

    return static_cast<bool> (files[Id::isIndexed]);
}

void Processor::moveRequestJobsToFront (const juce::ValueTree& request)
{
    const auto path { Query::getFile (request).getFullPathName() };
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    if (const auto entry { index.unitRows.find (path) }; entry != index.unitRows.end())
    {
        const auto& [unitPath, unitRow] { *entry };
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
    jam::debug::Log::write ("addDiagnosticRequests", file[Id::path].toString(), model.state.getChildWithName (jam::Format::toType (Id::connections)).getNumChildren());

    const auto directory { juce::File { file[Id::path].toString() }.getParentDirectory().getFullPathName() };

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

void Processor::setIndexed (int rowIndex)
{
    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    auto row { files.getChild (rowIndex) };
    bool isIndexed { not parseJobs.contains (rowIndex) and not nextParseJobs.contains (rowIndex) };

    if (const auto unit { index.unitRows.find (row[Id::path].toString()) }; unit != index.unitRows.end())
    {
        const auto& [unitPath, unitRow] { *unit };
        const auto unitTree { files.getChild (unitRow) };

        isIndexed = isIndexed and not indexJobs.contains (unitRow) and not nextIndexJobs.contains (unitRow)
                and (index.symbolDocuments.contains (unitRow) or unitTree.hasProperty (Id::message) or not index.languages.contains (static_cast<int> (unitTree[Id::family])));
    }

    jam::debug::Log::write ("setIndexed", row[Id::path].toString(), isIndexed, parseJobs.contains (rowIndex), nextParseJobs.contains (rowIndex));
    row.setProperty (Id::isIndexed, isIndexed, nullptr);
}

void Processor::setUnitIndexed (const juce::ValueTree& unit, int unitIndex)
{
    setIndexed (unitIndex);

    if (const auto* dependencies { unit[Id::dependencies].getArray() }; dependencies != nullptr)
        for (const auto& dependency : *dependencies)
            setIndexed (static_cast<int> (dependency));
}

void Processor::setIndexed()
{
    auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    files.setProperty (Id::isIndexed, model.state.getChildWithName (jam::Format::toType (Id::toolchain)).isValid() and parseJobs.empty() and nextParseJobs.empty() and indexJobs.empty() and nextIndexJobs.empty() and unitsJobs.empty() and nextUnitsJobs.empty(), nullptr);

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
    jam::debug::Log::write ("runRequest", request[Id::command].toString(), Query::getFile (request).getFullPathName());

    if (const auto entry { index.unitRows.find (Query::getFile (request).getFullPathName()) }; entry != index.unitRows.end())
    {
        const auto& [unitPath, unitRow] { *entry };
        const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
        auto unit { files.getChild (unitRow) };

        if (const auto* prerequisites { unit[Id::prerequisites].getArray() }; prerequisites != nullptr)
            for (const auto& prerequisite : *prerequisites)
                files.getChild (static_cast<int> (prerequisite)).removeProperty (Id::isRequested, nullptr);

        unit.removeProperty (Id::isRequested, nullptr);
    }

    const auto word { request[Id::command].toString() };
    const juce::Identifier key { word.isNotEmpty() ? juce::Identifier { word } : juce::Identifier {} };

    if (events.contains (key) and Query::isValidRequest (request))
    {
        const auto path { Query::getFile (request).getFullPathName() };
        const auto unit { getUnit (path) };

        if (unit.hasProperty (Id::message))
            model.setMessage (path + Id::diagnosticSeparator + unit[Id::message].toString());

        if (unit.isValid())
        {
            if (const auto languageRow { model.state.getChildWithProperty (Id::family, unit[Id::family]) }; languageRow.hasType (jam::Format::toType (Id::language)) and languageRow.hasProperty (Id::message))
                model.setMessage (path + Id::diagnosticSeparator + languageRow[Id::message].toString());
        }

        events.get (key, request);
    }
    else
    {
        Query::setError (request, lsp::ErrorCodes::InvalidParams);
    }
}

void Processor::runHover (juce::ValueTree& request)
{
    const auto answer { [this, &request] (int pathIndex, const jam::Document&, const jam::Document::Element& element, int tokenIndex, const jam::Document::Element& record)
    {
        query.hover (request, index, pathIndex, &element, tokenIndex, &record);
    } };
    const auto isFound { query.applyToHover (request, index, answer) };

    if (not isFound)
        query.hover (request, index, 0, nullptr, 0, nullptr);
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

void Processor::addJobs (const juce::ValueTree& files)
{
    index.removeDocuments (files);
    index.addPathRows (files);
    index.addUnitRows (files);
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

    setIndexed();
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
    juce::StringArray extensions;

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
    jam::debug::Log::write ("setBufferChange", Query::getFile (request).getFullPathName(), index.pathRows.contains (Query::getFile (request).getFullPathName()));

    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };

    if (const auto entry { index.pathRows.find (Query::getFile (request).getFullPathName()) }; entry != index.pathRows.end())
    {
        const auto& [rowPath, rowIndex] { *entry };

        files.getChild (rowIndex).sendPropertyChangeMessage (Id::path);
    }

    request.getParent().removeChild (request, nullptr);
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

    const juce::File file { path };

    modificationTime = file.getLastModificationTime();

    if (file.existsAsFile())
    {
        const auto diskText { file.loadFileAsString() };
        const auto diskBytes { jam::Strings::getView (diskText) };
        const auto hash { static_cast<juce::int64> (jam::Wyhash::hashBytes (diskBytes.data(), diskBytes.size())) };
        const juce::String& text { buffer.has_value() ? *buffer : diskText };
        const auto bytes { jam::Strings::getView (text) };
        const auto decoded { bytes.find (static_cast<char> (Chars::escapeCharacter)) == std::string_view::npos ? std::string {} : jam::AnsiDocument::getText (bytes) };
        const auto view { decoded.empty() ? bytes : std::string_view { decoded } };

        document = buffer.has_value() ? nullptr : Index::readSwapFile (swapFile, view, language, hash, languageHash, family);

        if (document == nullptr)
        {
            auto [parsedDocument, parsedSymbols] { language->parse (view, path) };

            document = std::move (parsedDocument);
            symbols = std::move (parsedSymbols);
            result = document == nullptr ? language->isValid (view) : juce::Result::ok();

            if (document != nullptr)
            {
                if (buffer.has_value())
                    Index::addStamp (*document, static_cast<juce::int64> (jam::Wyhash::hashBytes (bytes.data(), bytes.size())), languageHash, family);
                else
                    Index::writeSwapFile (swapFile, *document, hash, languageHash, family);
            }
        }
        else
        {
            symbols = language->getSymbols (*document, path);
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

Processor::IndexJob::IndexJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::String& directoryToUse, jam::Array<char> argumentsToUse, juce::int64 hashToUse, const juce::File& symbolSwapFileToUse, juce::int64 languageHashToUse, std::shared_ptr<const Language> languageToUse, int familyToUse, juce::StringArray bufferPathsToUse, juce::StringArray bufferTextsToUse, jam::Strings foreignPathsToUse)
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
      foreignPaths (std::move (foreignPathsToUse))
{
}

juce::ThreadPoolJob::JobStatus Processor::IndexJob::runJob()
{
#if JAM_USING_PROFILER
    const jam::Profiler::Zone zone { Id::runIndexJob };
#endif

    symbols = Index::readSwapFile (symbolSwapFile, hash, languageHash, family);

    if (symbols == nullptr)
    {
        symbols = std::make_unique<SymbolDocument>();
        addSymbols();
    }

    isFinished.store (true, std::memory_order_release);
    owner.triggerAsyncUpdate();

    return jobHasFinished;
}

template <typename GetKey>
static void removeDuplicateTokens (jam::Document::Tokens& tokens, const GetKey& getKey)
{
    std::sort (tokens.begin(), tokens.end(), [&getKey] (const auto& left, const auto& right) { return getKey (left) < getKey (right); });

    const auto duplicates { std::unique (tokens.begin(), tokens.end(), [&getKey] (const auto& left, const auto& right) { return getKey (left) == getKey (right); }) };

    tokens.removeRange (static_cast<int> (std::distance (tokens.begin(), duplicates)), static_cast<int> (std::distance (duplicates, tokens.end())));
}

static jam::HashMap<int, jam::Array<int>> getOutlineChildren (const jam::Document::Tokens& outlines)
{
    jam::HashMap<int, jam::Array<int>> children;

    for (int outlineIndex { 0 }; outlineIndex < outlines.size(); ++outlineIndex)
    {
        const auto [childrenEntry, isChildrenAdded] { children.try_emplace (*outlines.at (outlineIndex).get<int> (Id::parent)) };
        auto& [parentIndex, parentChildren] { *childrenEntry };

        parentChildren.add (outlineIndex);
    }

    return children;
}

static void sortOutlineTokens (jam::Document::Tokens& outlines)
{
    const auto children (getOutlineChildren (outlines));
    jam::Document::Tokens sorted;
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
        auto& outline { outlines.at (outlineIndex) };

        pending.remove (pending.size() - 1);
        *outline.get<int> (Id::parent) = sortedParentIndex;
        sorted.add (std::move (outline));
        addPending (outlineIndex, sorted.size() - 1);
    }

    jassert (sorted.size() == outlines.size());
    outlines = std::move (sorted);
}

static void sortFileTokens (jam::Document::Element& fileElement)
{
    sortOutlineTokens (*fileElement.get<jam::Document::Tokens> (Id::outline));

    removeDuplicateTokens (*fileElement.get<jam::Document::Tokens> (Id::inlayHints), [] (const jam::Document::Token& token)
    {
        return std::make_tuple (*token.get<int> (Id::line), *token.get<int> (Id::column), jam::Strings::getView (*token.get<juce::String> (Id::label)));
    });

    removeDuplicateTokens (*fileElement.get<jam::Document::Tokens> (Id::fixIts), [] (const jam::Document::Token& token)
    {
        return std::make_tuple (*token.get<int> (Id::line), *token.get<int> (Id::column), *token.get<int> (Id::endLine), *token.get<int> (Id::endColumn),
                                jam::Strings::getView (*token.get<juce::String> (Id::text)));
    });
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
    callbacks.shouldExit = [] (void* context) { return static_cast<IndexJob*> (context)->shouldExit() ? 1 : 0; };
    callbacks.isForeign = [] (void* context, const char* path) { return static_cast<IndexJob*> (context)->isForeign (path) ? 1 : 0; };

    result = language->index (pointers.data(), pointers.size(), directory.toRawUTF8(), bufferPathPointers.data(), bufferTextPointers.data(), bufferPathPointers.size(), callbacks);

    if (result == 0)
    {
        for (auto* fileElement : fileElements)
            sortFileTokens (*fileElement);

        Index::writeSwapFile (symbolSwapFile, *symbols, hash, languageHash, family);
    }
}

bool Processor::IndexJob::isForeign (const char* recordPath) const
{
    return foreignPaths.indexOf (juce::StringRef { juce::CharPointer_UTF8 { recordPath } }, false, 0) >= 0;
}

int Processor::IndexJob::getOrCreatePathIndex (const char* recordPath)
{
    if (const auto pathIndex { paths.indexOf (juce::StringRef { juce::CharPointer_UTF8 { recordPath } }, false, 0) }; pathIndex >= 0)
        return pathIndex;

    auto* element { symbols->addChild (*symbols->getRoot(), Id::file) };

    element->add<juce::String> (Id::path, juce::String::fromUTF8 (recordPath));
    element->add<jam::Document::Tokens> (Id::inlayHints);
    element->add<jam::Document::Tokens> (Id::diagnostics);
    element->add<jam::Document::Tokens> (Id::outline);
    element->add<jam::Document::Tokens> (Id::fixIts);
    element->add<jam::Document::Tokens> (Id::hovers);
    fileElements.add (element);
    completionContainers.add (symbols->addChild (*element, Id::completions));
    paths.add (std::string_view { recordPath });
    return paths.size() - 1;
}

void Processor::IndexJob::addSymbol (void* context, const char* recordPath, int line, int column, unsigned long long symbol, int nameLength, int role, int type, int hover, const unsigned long long* typeSymbols, int typeCount)
{
    auto& job { *static_cast<IndexJob*> (context) };

    jassert (line > 0 and column > 0);

    const auto pathIndex { job.getOrCreatePathIndex (recordPath) };
    auto* record { job.symbols->addChild (*job.fileElements.at (pathIndex), Id::symbol) };

    record->add<int> (Id::line, line);
    record->add<int> (Id::column, column);
    record->add<int> (Id::nameLength, nameLength);
    record->add<juce::int64> (Id::symbol, static_cast<juce::int64> (symbol));
    record->add<int> (Id::role, role);
    record->add<int> (Id::type, type);
    record->add<int> (Id::hover, hover);

    auto* types { record->add<jam::Document::Integers> (Id::typeSymbols, typeCount) };

    std::for_each (typeSymbols, typeSymbols + typeCount, [types] (unsigned long long typeSymbol) { types->add (static_cast<juce::int64> (typeSymbol)); });
}

int Processor::IndexJob::addHover (void* context, const char* recordPath, const char* text, const char* definitionPath, int line, int column, int endLine, int endColumn)
{
    auto& job { *static_cast<IndexJob*> (context) };

    auto& tokens { *job.fileElements.at (job.getOrCreatePathIndex (recordPath))->get<jam::Document::Tokens> (Id::hovers) };
    jam::Document::Token token;
    const auto hoverIndex { tokens.size() };

    token.add<juce::String> (Id::hover, juce::String::fromUTF8 (text));
    token.add<juce::String> (Id::path, definitionPath != nullptr ? juce::String::fromUTF8 (definitionPath) : juce::String {});
    token.add<int> (Id::line, line);
    token.add<int> (Id::column, column);
    token.add<int> (Id::endLine, endLine);
    token.add<int> (Id::endColumn, endColumn);
    tokens.add (std::move (token));

    return hoverIndex;
}

void Processor::IndexJob::addInlayHint (void* context, const char* recordPath, int line, int column, const char* label, int type, int paddingLeft, int paddingRight)
{
    auto& job { *static_cast<IndexJob*> (context) };

    auto& tokens { *job.fileElements.at (job.getOrCreatePathIndex (recordPath))->get<jam::Document::Tokens> (Id::inlayHints) };
    jam::Document::Token token;

    token.add<int> (Id::line, line);
    token.add<int> (Id::column, column);
    token.add<juce::String> (Id::label, juce::String::fromUTF8 (label));
    token.add<int> (Id::type, type);
    token.add<bool> (Id::paddingLeft, paddingLeft != 0);
    token.add<bool> (Id::paddingRight, paddingRight != 0);
    tokens.add (std::move (token));
}

void Processor::IndexJob::addDiagnostic (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, int severity, const char* message)
{
    auto& job { *static_cast<IndexJob*> (context) };

    auto& tokens { *job.fileElements.at (job.getOrCreatePathIndex (recordPath))->get<jam::Document::Tokens> (Id::diagnostics) };
    jam::Document::Token token;

    token.add<int> (Id::line, line);
    token.add<int> (Id::column, column);
    token.add<int> (Id::endLine, endLine);
    token.add<int> (Id::endColumn, endColumn);
    token.add<int> (Id::severity, severity);
    token.add<juce::String> (Id::message, juce::String::fromUTF8 (message));
    tokens.add (std::move (token));
}

int Processor::IndexJob::addOutline (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, int nameLength, int qualifierLength, int type, int parent)
{
    auto& job { *static_cast<IndexJob*> (context) };

    const auto pathIndex { job.getOrCreatePathIndex (recordPath) };
    const std::array<int, outlineFieldCount> keyFields { pathIndex, line, column, endLine, endColumn, nameLength, qualifierLength, type };
    std::uint64_t keyHash { 0 };

    for (const auto keyField : keyFields)
        keyHash = jam::Wyhash::fold (keyHash, static_cast<std::uint64_t> (keyField));

    if (const auto known { job.outlineKeys.find (static_cast<juce::int64> (keyHash)) }; known != job.outlineKeys.end())
    {
        const auto& [knownKey, knownTokenIndex] { *known };

        job.outlineSlots.add ({ pathIndex, knownTokenIndex });
    }
    else
    {
        auto& tokens { *job.fileElements.at (pathIndex)->get<jam::Document::Tokens> (Id::outline) };
        const auto tokenIndex { tokens.size() };

        addOutlineToken (tokens, { line, column, endLine, endColumn, nameLength, qualifierLength, type, job.getOutlineParent (parent, pathIndex) });
        job.outlineKeys.addOrReplace (static_cast<juce::int64> (keyHash), tokenIndex);
        job.outlineSlots.add ({ pathIndex, tokenIndex });
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

void Processor::IndexJob::addOutlineToken (jam::Document::Tokens& tokens, const std::array<int, outlineFieldCount>& fields)
{
    jam::Document::Token token;

    token.add<int> (Id::line, fields.at (0));
    token.add<int> (Id::column, fields.at (1));
    token.add<int> (Id::endLine, fields.at (2));
    token.add<int> (Id::endColumn, fields.at (3));
    token.add<int> (Id::nameLength, fields.at (4));
    token.add<int> (Id::qualifierLength, fields.at (5));
    token.add<int> (Id::type, fields.at (6));
    token.add<int> (Id::parent, fields.at (7));
    tokens.add (std::move (token));
}

void Processor::IndexJob::addCompletion (void* context, const char* recordPath, unsigned long long scopeSymbol, unsigned long long symbol, const char* label, const char* insertText, int type)
{
    auto& job { *static_cast<IndexJob*> (context) };

    const auto key { static_cast<juce::int64> (jam::Wyhash::fold (static_cast<std::uint64_t> (scopeSymbol), static_cast<std::uint64_t> (symbol))) };
    auto [keyEntry, isInserted] { job.completionKeys.insert (key) };

    if (isInserted)
    {
        auto* container { job.completionContainers.at (job.getOrCreatePathIndex (recordPath)) };
        auto* completion { job.symbols->addChild (*container, Id::completion) };

        completion->add<juce::int64> (Id::scope, static_cast<juce::int64> (scopeSymbol));
        completion->add<juce::int64> (Id::symbol, static_cast<juce::int64> (symbol));
        completion->add<juce::String> (Id::label, juce::String::fromUTF8 (label));
        completion->add<juce::String> (Id::snippet, juce::String::fromUTF8 (insertText));
        completion->add<int> (Id::type, type);
    }
}

void Processor::IndexJob::addFixIt (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, const char* text)
{
    auto& job { *static_cast<IndexJob*> (context) };

    auto& tokens { *job.fileElements.at (job.getOrCreatePathIndex (recordPath))->get<jam::Document::Tokens> (Id::fixIts) };
    jam::Document::Token token;

    token.add<int> (Id::line, line);
    token.add<int> (Id::column, column);
    token.add<int> (Id::endLine, endLine);
    token.add<int> (Id::endColumn, endColumn);
    token.add<juce::String> (Id::text, juce::String::fromUTF8 (text));
    tokens.add (std::move (token));
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

void Processor::addIndexJob (const juce::ValueTree& row, int rowIndex)
{
    jam::debug::Log::write ("addIndexJob", row[Id::path].toString(), rowIndex);

    auto arguments (getArguments (row));
    juce::StringArray bufferPaths;
    juce::StringArray bufferTexts;
    jam::Strings foreignPaths;

    if (const auto running { indexJobs.find (rowIndex) }; running != indexJobs.end())
    {
        const auto& [runningRow, runningJob] { *running };

        if (pool->removeJob (runningJob.get(), true, noWait))
            indexJobs.erase (rowIndex);
    }

    const auto family { static_cast<int> (row[Id::family]) };
    const auto hash { static_cast<juce::int64> (jam::Wyhash::fold (static_cast<std::uint64_t> (getUnitHash (row, rowIndex, arguments)), juce::File { row[Id::output].toString() }.existsAsFile() ? 1 : 0)) };
    const auto buffers { model.state.getChildWithName (jam::Format::toType (Id::buffers)) };

    for (const auto& buffer : buffers)
    {
        bufferPaths.add (buffer[Id::path].toString());
        bufferTexts.add (buffer[Id::text].toString());
    }

    for (const auto& [unitPath, unitRow] : index.unitRows)
        if (unitRow != rowIndex)
            foreignPaths.add (unitPath);

    nextIndexJobs.addOrReplace (rowIndex, std::make_unique<IndexJob> (*this, rowIndex, row[Id::path].toString(), row[Id::directory].toString(), std::move (arguments), hash, model.getSwapFile (juce::File { row[Id::path].toString() }, files::lingSymbols), static_cast<juce::int64> (model.state.getChildWithProperty (Id::family, family)[Id::hash]), index.languages.at (family), family, std::move (bufferPaths), std::move (bufferTexts), std::move (foreignPaths)));
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

        return files.getChild (first)[Id::dependencies].size() > files.getChild (second)[Id::dependencies].size();
    });

    for (const auto rowIndex : rows)
    {
        indexJobs.addOrReplace (rowIndex, std::move (nextIndexJobs.at (rowIndex)));
        nextIndexJobs.erase (rowIndex);
        pool->addJob (indexJobs.at (rowIndex).get(), false);

        if (static_cast<bool> (files.getChild (rowIndex)[Id::isRequested]))
            pool->moveJobToFront (indexJobs.at (rowIndex).get());
    }
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
    jam::debug::Log::write ("addDocument", job.path, job.buffer.has_value(), row[Id::path].toString().compare (job.path) == 0, juce::File { job.path }.getLastModificationTime() == job.modificationTime);
    const auto isIndexReady { [this, &job] (const juce::ValueTree& unit, int unitIndex)
    {
        const auto family { static_cast<int> (unit[Id::family]) };

        return not hasParseJob (unit, unitIndex, job.rowIndex) and index.languages.contains (family) and index.languages.at (family)->hasIndex();
    } };

    if (row[Id::path].toString().compare (job.path) == 0 and juce::File { job.path }.getLastModificationTime() == job.modificationTime
        and job.language == index.languages.at (static_cast<int> (row[Id::family])))
    {
        index.addDocument (job.path, std::move (job.document));

        if (job.symbols != nullptr)
            index.addBindings (job.rowIndex, std::move (job.symbols));

        if (not job.result.wasOk())
            model.setMessage (job.path + Id::diagnosticSeparator + job.result.getErrorMessage());

        if (job.buffer.has_value())
        {
            if (const auto entry { index.unitRows.find (job.path) }; entry != index.unitRows.end())
            {
                const auto& [unitPath, unitRow] { *entry };
                const auto unit { files.getChild (unitRow) };

                if (isIndexReady (unit, unitRow))
                    addIndexJob (unit, unitRow);
            }
        }
        else
        {
            for (int unitIndex { 0 }; unitIndex < files.getNumChildren(); ++unitIndex)
            {
                const auto unit { files.getChild (unitIndex) };

                if (hasDependency (unit, unitIndex, job.rowIndex) and isIndexReady (unit, unitIndex))
                    addIndexJob (unit, unitIndex);
            }
        }
    }
}

void Processor::addBindings (IndexJob& job)
{
    jam::debug::Log::write ("addBindings", job.path, job.result);

    const auto files { model.state.getChildWithName (jam::Format::toType (Id::files)) };
    auto row { files.getChild (job.rowIndex) };

    if (row[Id::path].toString().compare (job.path) == 0)
    {
        if (job.result == 0)
        {
            index.addBindings (job.rowIndex, std::move (job.symbols));
            row.removeProperty (Id::message, nullptr);
        }
        else
        {
            model.setMessage (job.path + Id::diagnosticSeparator + text::Diagnostics::failUnitIndex);
            row.setProperty (Id::message, text::Diagnostics::failUnitIndex, nullptr);
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

    return std::make_unique<UnitsJob> (*this, std::move (arguments), std::move (argumentCounts), LINGModel::getProjectDirectory (juce::File { manifest }).getFullPathName(),
                                       LINGModel::getCacheDirectory (juce::File { manifest }, jam::Strings::getView (toolchainArgument)).getFullPathName(),
                                       index.languages, std::move (families));
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

Processor::UnitsJob::UnitsJob (Processor& ownerToUse, jam::Array<char> argumentsToUse, jam::Array<int> argumentCountsToUse, const juce::String& directoryToUse, const juce::String& cacheDirectoryToUse, jam::HashMap<int, std::shared_ptr<const Language>> languagesToUse, jam::HashMap<juce::String, int> familiesToUse)
    : juce::ThreadPoolJob (directoryToUse),
      owner (ownerToUse),
      arguments (std::move (argumentsToUse)),
      argumentCounts (std::move (argumentCountsToUse)),
      directory (directoryToUse),
      cacheDirectory (cacheDirectoryToUse),
      languages (std::move (languagesToUse)),
      families (std::move (familiesToUse))
{
}

juce::ThreadPoolJob::JobStatus Processor::UnitsJob::runJob()
{
#if JAM_USING_PROFILER
    const jam::Profiler::Zone zone { Id::getFiles };
#endif

    const auto pointers (getArgumentPointers (arguments));
    jam::LanguageCallbacks callbacks {};

    callbacks.context = this;
    callbacks.addUnit = &UnitsJob::addUnit;
    callbacks.addBuildFile = &UnitsJob::addBuildFile;
    callbacks.addMessage = &UnitsJob::addMessage;

    for (const auto& [family, language] : languages)
        language->units (pointers.data(), argumentCounts.data(), argumentCounts.size(), directory.toRawUTF8(), cacheDirectory.toRawUTF8(), callbacks);

    files.setProperty (Id::buildFiles, buildFiles, nullptr);

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
    row.setProperty (Id::dependencies, job.getOrCreatePathIndices (dependencies, dependencyCount, static_cast<int> (row[Id::family])), nullptr);
    row.setProperty (Id::prerequisites, job.getOrCreatePathIndices (prerequisites, prerequisiteCount, static_cast<int> (row[Id::family])), nullptr);
}

void Processor::UnitsJob::addBuildFile (void* context, const char* path)
{
    auto& job { *static_cast<UnitsJob*> (context) };

    job.buildFiles.add (juce::String::fromUTF8 (path));
}

void Processor::UnitsJob::addMessage (void* context, const char* text)
{
    auto& job { *static_cast<UnitsJob*> (context) };

    job.files.setProperty (Id::message, juce::String::fromUTF8 (text), nullptr);
}

int Processor::UnitsJob::getOrCreatePathIndex (const juce::String& path, int family)
{
    if (const auto pathIndex { paths.indexOf (path, false, 0) }; pathIndex >= 0)
        return pathIndex;

    juce::ValueTree row { jam::Format::toType (Id::file) };

    row.setProperty (Id::path, path, nullptr);
    row.setProperty (Id::family, family, nullptr);
    files.appendChild (row, nullptr);
    paths.add (path);
    return paths.size() - 1;
}

juce::Array<juce::var> Processor::UnitsJob::getOrCreatePathIndices (const char* const* filePaths, int filePathCount, int family)
{
    juce::Array<juce::var> pathIndices;

    for (int argumentIndex { 0 }; argumentIndex < filePathCount; ++argumentIndex)
        pathIndices.add (getOrCreatePathIndex (juce::String::fromUTF8 (filePaths[argumentIndex]), family));

    return pathIndices;
}
