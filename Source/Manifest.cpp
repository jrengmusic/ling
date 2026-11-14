#include "Manifest.h"
#include "Validator.h"
#include "CompilerManifest.h"

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
    const auto project { manifestFile.getParentDirectory() };
    juce::ValueTree roots { jam::Format::toType (Id::roots) };

    roots.appendChild (getRoot (project, project.getChildFile (files::lingCache)), nullptr);

    return roots;
}

static juce::File getFolder (const jam::MarkdownDocument& manifest, const juce::String& cell, const juce::File& project)
{
    const auto path { cell.startsWithChar (Chars::at) ? manifest.getTableValue (Id::index, Id::symbol, juce::Identifier { cell }) : cell };

    return project.getChildFile (jam::Format::replaceholder (path, files::sourceDirectoryVariable, project.getFullPathName(), juce::StringRef {}));
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
    const auto text { manifestFile.loadFileAsString() };
    const auto manifest { jam::MarkdownDocument::parse (text, manifestFile.getFullPathName()) };
    const auto result { manifest != nullptr ? Validator {}.isValid (*manifest) : jam::MarkdownDocument::isValid (jam::Strings::getView (text), manifestFile.getFullPathName()) };

    timeout = result.wasOk() ? getTimeout (*manifest) : LINGModel::defaultTimeoutMs;
    files = result.wasOk() ? CompilerManifest::getFiles (*manifest, manifestFile, toolchainArgument)
                           : juce::ValueTree { jam::Format::toType (Id::files) };
    roots = result.wasOk() ? getRoots (*manifest, manifestFile) : getProjectRoots (manifestFile);

    if (not result.wasOk())
        files.setProperty (Id::message, result.getErrorMessage(), nullptr);

    isFinished.store (true, std::memory_order_release);
    owner.triggerAsyncUpdate();

    return jobHasFinished;
}

void Manifest::handleAsyncUpdate()
{
    if (job != nullptr and job->isFinished.load (std::memory_order_acquire))
    {
        if (nextJob == nullptr)
            setFiles (job->files, job->roots, job->timeout);

        pool.removeJob (job.get(), true, waitForJob);
        job = std::move (nextJob);

        if (job != nullptr)
            pool.addJob (job.get(), false);
    }
}

void Manifest::setFiles (const juce::ValueTree& files, const juce::ValueTree& roots, int timeout)
{
    if (files.hasProperty (Id::message))
        model.setMessage (files[Id::message].toString());

    model.state.setProperty (Id::timeout, timeout, nullptr);
    model.removeChild (model.state.getChildWithName (jam::Format::toType (Id::roots)));
    model.appendChild (roots);
    model.removeChild (model.state.getChildWithName (jam::Format::toType (Id::files)));
    model.appendChild (files);
}

int Manifest::getTimeout (const jam::MarkdownDocument& manifest)
{
    for (const auto* table : manifest.getTables (Id::daemon))
        if (const auto* row { manifest.getTableRow (*table, Id::timeout) }; row != nullptr)
            return manifest.getTableValue (*row, Id::value).getIntValue();

    return LINGModel::defaultTimeoutMs;
}

juce::ValueTree Manifest::getRoots (const jam::MarkdownDocument& manifest, const juce::File& manifestFile)
{
    const auto project { manifestFile.getParentDirectory() };
    const auto userModuleCell { manifest.getTableValue (Id::index, Id::symbol, Id::userModule) };
    auto roots { getProjectRoots (manifestFile) };

    if (userModuleCell.isNotEmpty())
    {
        const auto userModules { getFolder (manifest, userModuleCell, project) };
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
