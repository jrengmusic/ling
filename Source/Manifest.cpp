#include "Manifest.h"
#include "Validator.h"

Manifest::Manifest (LINGModel& newModel)
    : model (newModel)
{
    model.addListener (this);
}

Manifest::~Manifest()
{
    model.removeListener (this);
}

void Manifest::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == Id::manifest and tree == model.state)
        addJob();
}

static juce::ValueTree getRoot (const juce::File& folder, const juce::File& swapFolder)
{
    juce::ValueTree root { jam::Format::toType (Id::root) };

    root.setProperty (Id::path, folder.getFullPathName(), nullptr);
    root.setProperty (Id::swap, swapFolder.getFullPathName(), nullptr);

    return root;
}

static juce::ValueTree getProjectRoots (const juce::File& manifestFile)
{
    const auto project { LINGModel::getProjectDirectory (manifestFile) };
    juce::ValueTree roots { jam::Format::toType (Id::roots) };

    roots.appendChild (getRoot (project, project.getChildFile (files::lingCache)), nullptr);

    return roots;
}

static juce::File getFolder (const jam::MarkdownDocument& manifest, const juce::String& cell, const juce::File& project)
{
    const auto path { cell.startsWithChar (Chars::at) ? manifest.getTableValue (Id::index, Id::symbol, juce::Identifier { cell }) : cell };

    return project.getChildFile (jam::Format::replaceholder (path, files::sourceDirectoryVariable, project.getFullPathName(), juce::StringRef {}));
}

static juce::File getUserModules (const jam::MarkdownDocument& manifest, const juce::File& project)
{
    return getFolder (manifest, manifest.getTableValue (Id::index, Id::symbol, Id::userModule), project);
}

static juce::ValueTree getPatternRow (const juce::Identifier& command, const juce::String& wildcard)
{
    juce::ValueTree row { jam::Format::toType (Id::row) };

    row.setProperty (Id::command, command.toString(), nullptr);
    row.setProperty (Id::flag, wildcard, nullptr);

    return row;
}

static jam::Strings getPatchFiles (const juce::File& userModules)
{
    jam::Strings patchFiles;
    const auto infoFile { userModules.getChildFile (files::userModulesInfo) };
    const auto info { jam::MarkdownDocument::parse (infoFile.loadFileAsString(), infoFile.getFullPathName()) };

    if (info != nullptr)
    {
        for (const auto* row : info->getTableRows (Id::submodule))
        {
            const auto name { info->getTableValue (*row, Id::module) };

            if (name.compare (Id::patch.toString()) == 0)
                patchFiles.add (userModules.getChildFile (name).getChildFile (info->getTableValue (*row, Id::path)).getFullPathName());
        }
    }

    return patchFiles;
}

void Manifest::addJob()
{
    static constexpr int noWait { 0 };

    auto newJob { std::make_unique<ManifestJob> (*this, model.state[Id::manifest].toString(), model.state[Id::toolchainArgument].toString()) };

    if (job != nullptr)
    {
        if (pool.removeJob (job.get(), true, noWait))
        {
            nextJob.reset();
            job = std::move (newJob);
            pool.addJob (job.get(), false);
        }
        else
        {
            nextJob = std::move (newJob);
        }
    }
    else
    {
        job = std::move (newJob);
        pool.addJob (job.get(), false);
    }
}

Manifest::ManifestJob::ManifestJob (Manifest& ownerToUse, const juce::String& manifestPathToUse, const juce::String& toolchainArgumentToUse)
    : juce::ThreadPoolJob (manifestPathToUse),
      owner (ownerToUse),
      manifestPath (manifestPathToUse),
      toolchainArgument (toolchainArgumentToUse)
{
}

juce::ThreadPoolJob::JobStatus Manifest::ManifestJob::runJob()
{
    const juce::File manifestFile { manifestPath };

    if (manifestFile.isDirectory())
    {
        timeout = LINGModel::defaultTimeoutMs;
        threads = LINGModel::defaultThreads;
        toolchain = juce::ValueTree { jam::Format::toType (Id::toolchain) };
        roots = getProjectRoots (manifestFile);
    }
    else
    {
        const auto text { manifestFile.loadFileAsString() };
        const auto manifest { jam::MarkdownDocument::parse (text, manifestFile.getFullPathName()) };
        const auto result { manifest != nullptr ? Validator {}.isValid (*manifest) : jam::MarkdownDocument::isValid (jam::Strings::getView (text), manifestFile.getFullPathName()) };

        timeout = result.wasOk() ? getDaemonValue (*manifest, Id::timeout, LINGModel::defaultTimeoutMs) : LINGModel::defaultTimeoutMs;
        threads = result.wasOk() ? getDaemonValue (*manifest, Id::threads, LINGModel::defaultThreads) : LINGModel::defaultThreads;
        toolchain = result.wasOk() ? getToolchain (*manifest, toolchainArgument, manifestFile)
                                   : juce::ValueTree { jam::Format::toType (Id::toolchain) };
        roots = result.wasOk() ? getRoots (*manifest, manifestFile) : getProjectRoots (manifestFile);

        if (not result.wasOk())
            toolchain.setProperty (Id::message, result.getErrorMessage(), nullptr);
    }

    isFinished.store (true, std::memory_order_release);
    owner.triggerAsyncUpdate();

    return jobHasFinished;
}

void Manifest::handleAsyncUpdate()
{
    static constexpr int waitForJob { -1 };

    if (job != nullptr and job->isFinished.load (std::memory_order_acquire))
    {
        if (nextJob == nullptr)
            setToolchain (job->toolchain, job->roots, job->timeout, job->threads);

        pool.removeJob (job.get(), true, waitForJob);
        job = std::move (nextJob);

        if (job != nullptr)
            pool.addJob (job.get(), false);
    }
}

void Manifest::setToolchain (const juce::ValueTree& toolchain, const juce::ValueTree& roots, int timeout, int threads)
{
    if (toolchain.hasProperty (Id::message))
        model.setMessage (toolchain[Id::message].toString());

    model.state.setProperty (Id::timeout, timeout, nullptr);
    model.state.setProperty (Id::threads, threads, nullptr);

    model.removeChild (model.state.getChildWithName (jam::Format::toType (Id::roots)));
    model.appendChild (roots);
    model.removeChild (model.state.getChildWithName (jam::Format::toType (Id::toolchain)));
    model.appendChild (toolchain);
}

int Manifest::getDaemonValue (const jam::MarkdownDocument& manifest, const juce::Identifier& rowId, int defaultValue)
{
    for (const auto* table : manifest.getTables (Id::daemon))
        if (const auto* row { manifest.getTableRow (*table, rowId) }; row != nullptr)
            return manifest.getTableValue (*row, Id::value).getIntValue();

    return defaultValue;
}

juce::ValueTree Manifest::getRoots (const jam::MarkdownDocument& manifest, const juce::File& manifestFile)
{
    const auto project { LINGModel::getProjectDirectory (manifestFile) };
    const auto userModuleCell { manifest.getTableValue (Id::index, Id::symbol, Id::userModule) };
    auto roots { getProjectRoots (manifestFile) };

    if (userModuleCell.isNotEmpty())
    {
        const auto userModules { getUserModules (manifest, project) };
        const auto version { manifest.getTableValue (Id::cmake, Id::value, Id::juceVersion) };
        const auto patchFiles { getPatchFiles (userModules) };

        roots.appendChild (getRoot (userModules, userModules.getChildFile (files::lingCache)), nullptr);

        if (version.isNotEmpty() and patchFiles.size() > 0)
        {
            const auto patched { LINGModel::getPatchedDirectory (jam::Strings::getView (version), patchFiles) };
            const auto juceFolder { getFolder (manifest, manifest.getTableValue (Id::cmake, Id::value, Id::jucePath), project) };

            if (patched.isDirectory())
                roots.appendChild (getRoot (patched, juceFolder.getChildFile (files::lingCache).getChildFile (patched.getFileName())), nullptr);
        }
    }

    return roots;
}

juce::ValueTree Manifest::getToolchain (const jam::MarkdownDocument& manifest, const juce::String& toolchainArgument, const juce::File& manifestFile)
{
    juce::ValueTree toolchain { jam::Format::toType (Id::toolchain) };
    const auto tables (manifest.getTables (Id::toolchain));
    const auto isToolchainRow { [&manifest, &toolchainArgument] (const jam::MarkdownDocument::Element* row) { return manifest.getTableValue (*row, Id::argument).compare (toolchainArgument) == 0; } };

    for (const auto* table : tables)
    {
        for (const auto* row : manifest.getTableRows (*table))
        {
            if (isToolchainRow (row))
            {
                juce::ValueTree toolchainRow { jam::Format::toType (Id::row) };

                toolchainRow.setProperty (Id::command, manifest.getTableValue (*row, Id::command), nullptr);
                toolchainRow.setProperty (Id::flag, manifest.getTableValue (*row, Id::flag), nullptr);
                toolchain.appendChild (toolchainRow, nullptr);
            }
        }
    }

    if (toolchainArgument.isNotEmpty() and std::none_of (tables.begin(), tables.end(), [&manifest, &isToolchainRow] (const jam::MarkdownDocument::Element* table)
                                                          {
                                                              const auto rows (manifest.getTableRows (*table));
                                                              return std::any_of (rows.begin(), rows.end(), isToolchainRow);
                                                          }))
        toolchain.setProperty (Id::message, toolchainArgument + Id::diagnosticSeparator + text::Diagnostics::failToolchainArgument, nullptr);

    addPatternRows (toolchain, manifest, manifestFile);

    return toolchain;
}

void Manifest::addPatternRows (juce::ValueTree& toolchain, const jam::MarkdownDocument& manifest, const juce::File& manifestFile)
{
    const auto project { LINGModel::getProjectDirectory (manifestFile) };
    const auto userModules { getUserModules (manifest, project) };

    for (const auto* table : manifest.getTables (Id::sourceGlob))
        for (const auto* row : manifest.getTableRows (*table))
            toolchain.appendChild (getPatternRow (Id::sourceGlob, getFolder (manifest, manifest.getTableValue (*row, Id::path), project).getChildFile (jam::Format::toFileExtension (manifest.getTableValue (*row, Id::extension))).getFullPathName()), nullptr);

    for (const auto* table : manifest.getTables (Id::layoutGlob))
        for (const auto* row : manifest.getTableRows (*table))
            toolchain.appendChild (getPatternRow (Id::layoutGlob, getFolder (manifest, manifest.getTableValue (*row, Id::value).replace (files::userModulePathVariable, userModules.getFullPathName()), project).getFullPathName()), nullptr);

    for (const auto* table : manifest.getTables (Id::binary))
        for (const auto* row : manifest.getTableRows (*table))
            toolchain.appendChild (getPatternRow (Id::binary, getFolder (manifest, manifest.getTableValue (*row, Id::value), project).getFullPathName()), nullptr);
}
