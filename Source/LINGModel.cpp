#include "LINGModel.h"

LINGModel::LINGModel()
    : jam::Model (jam::Format::toType (Id::ling))
{
    static constexpr int maximumMessageLength { 4096 };

    getOrCreateChildWithName (jam::Format::toType (Id::files));
    getOrCreateChildWithName (jam::Format::toType (Id::roots));
    getOrCreateChildWithName (jam::Format::toType (Id::connections));
    getOrCreateChildWithName (jam::Format::toType (Id::buffers));
    auto overlayRow { getOrCreateChildWithName (jam::Format::toType (Id::overlay)) };
    createAndAddParameter<jam::ParameterText> (overlayRow, Id::message, juce::String {}, maximumMessageLength);
    createAndAddParameter<jam::Parameter<int>> (overlayRow, Id::jobCount, 0);
    createAndAddParameter<jam::Parameter<int>> (overlayRow, Id::pendingJobCount, 0);
    state.setProperty (Id::timeout, defaultTimeoutMs, nullptr);

    watcher.coalesceEvents (coalesceMs);
    watcher.addListener (this);
}

void LINGModel::setMessage (const juce::String& text)
{
    auto* param { getParameter<jam::ParameterText> (jam::Format::toType (Id::overlay), Id::message) };

    param->setValue (text);
}

std::string LINGModel::getProjectName (const juce::File& manifest, std::string_view toolchainArgument)
{
    const auto manifestPath { manifest.getFullPathName() };
    std::string key { jam::Strings::getView (manifestPath) };
    std::string name;

    jam::Format::appendUtf8 (key, Chars::newline);
    key.append (toolchainArgument);
    jam::Format::appendHex (name, jam::Wyhash::hashBytes (key.data(), key.size()));
    return name;
}

std::string LINGModel::getDaemonName (const juce::File& manifest, std::string_view toolchainArgument)
{
    static const auto buildHash { []
    {
        const juce::MemoryMappedFile executable { juce::File::getSpecialLocation (juce::File::currentExecutableFile), juce::MemoryMappedFile::readOnly };

        jassert (executable.getData() != nullptr);
        return jam::Wyhash::hashBytes (executable.getData(), executable.getSize());
    }() };

    auto name (getProjectName (manifest, toolchainArgument));

    jam::Format::appendHex (name, buildHash);
    return name;
}

juce::File LINGModel::getCacheDirectory()
{
#if JUCE_WINDOWS
    return juce::File::getSpecialLocation (juce::File::windowsLocalAppData).getChildFile (files::lingLocalCache);
#else
    return juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (files::lingCache);
#endif
}

juce::File LINGModel::getConfigDirectory()
{
    return juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (files::lingConfig);
}

juce::Array<juce::File> LINGModel::getLanguageDirectories()
{
    auto folders (getConfigDirectory().getChildFile (files::language).findChildFiles (juce::File::findDirectories, false));

    folders.removeIf ([] (const juce::File& folder) { return not folder.getChildFile (files::languageConstruction).existsAsFile(); });
    folders.sort();
    return folders;
}

juce::Array<juce::File> LINGModel::getOrCreateLanguageDirectories()
{
    if (getLanguageDirectories().isEmpty())
    {
        const auto folder { getConfigDirectory().getChildFile (files::language).getChildFile (files::defaultLanguage) };
        [[maybe_unused]] const auto isCreated { folder.createDirectory() };
        jassert (isCreated.wasOk());

        for (const auto& fileName : { files::languageSpell, files::languageTemplate, files::languageConstruction })
        {
            const BinaryData::Raw resource { fileName };
            [[maybe_unused]] const auto isWritten { folder.getChildFile (fileName).replaceWithData (resource.data, static_cast<size_t> (resource.size)) };
            jassert (isWritten);
        }
    }

    return getLanguageDirectories();
}

juce::File LINGModel::getLanguageLibrary (const juce::File& languageFolder)
{
#if JUCE_WINDOWS
    return languageFolder.getChildFile (files::languageLibraryWindows);
#else
    return languageFolder.getChildFile (files::languageLibrary);
#endif
}

juce::File LINGModel::getCacheDirectory (const juce::File& manifest, std::string_view toolchainArgument)
{
    return getCacheDirectory().getChildFile (jam::Strings::fromView (getProjectName (manifest, toolchainArgument)));
}

juce::File LINGModel::getProjectDirectory (const juce::File& manifest)
{
    return manifest.isDirectory() ? manifest : manifest.getParentDirectory();
}

juce::File LINGModel::getPatchedDirectory (std::string_view version, const jam::Strings& patchFiles)
{
    std::string concatenation { version };

    for (const auto& patchFile : patchFiles)
    {
        const auto text { juce::File { patchFile }.loadFileAsString() };
        const auto view { jam::Strings::getView (text) };

        for (std::size_t index { 0 }; index < view.size(); ++index)
        {
            const auto isLineEnd { index + 1 == view.size() or view.at (index + 1) == static_cast<char> (Chars::newline) };

            if (not (view.at (index) == static_cast<char> (Chars::carriageReturn) and isLineEnd))
                concatenation.push_back (view.at (index));
        }
    }

    const auto hash { juce::SHA256 (concatenation.data(), concatenation.size()).toHexString() };

#if JUCE_WINDOWS
    return juce::File::getSpecialLocation (juce::File::windowsLocalAppData).getChildFile (files::castJuceLocalCache).getChildFile (hash);
#else
    return juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (files::castJuceCache).getChildFile (hash);
#endif
}

juce::File LINGModel::getSwapFile (const juce::File& file, juce::StringRef extension) const
{
    juce::ValueTree row;
    int size { 0 };

    for (const auto& root : state.getChildWithName (jam::Format::toType (Id::roots)))
    {
        const auto path { root[Id::path].toString() };

        if (path.length() > size and file.isAChildOf (juce::File { path }))
        {
            row = root;
            size = path.length();
        }
    }

    const auto toolchainArgument { state[Id::toolchainArgument].toString() };
    const juce::File manifest { state[Id::manifest].toString() };

    if (row.isValid())
        return juce::File { row[Id::swap].toString() }.getChildFile (jam::Strings::fromView (getProjectName (manifest, jam::Strings::getView (toolchainArgument)))).getChildFile (jam::Format::toFileName (file.getRelativePathFrom (juce::File { row[Id::path].toString() }), extension));

    const auto path { file.getFullPathName() };
    const auto bytes { jam::Strings::getView (path) };
    std::string name;

    jam::Format::appendHex (name, jam::Wyhash::hashBytes (bytes.data(), bytes.size()));
    return getCacheDirectory (manifest, jam::Strings::getView (toolchainArgument)).getChildFile (jam::Format::toFileName (jam::Strings::fromView (name), extension));
}

void LINGModel::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    jam::Model::valueTreePropertyChanged (tree, property);

    if (tree == state and property == Id::manifest)
        watcher.addFolder (getProjectDirectory (juce::File { state[Id::manifest].toString() }));
}

void LINGModel::valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
{
    jam::Model::valueTreeChildAdded (parentTree, childWhichHasBeenAdded);

    if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::files)))
    {
        for (const auto& row : childWhichHasBeenAdded)
        {
            const auto folder { juce::File { row[Id::path].toString() }.getParentDirectory() };

            if (folder.isDirectory())
                watcher.addFolder (folder);
        }

        if (const auto* paths { childWhichHasBeenAdded[Id::buildFiles].getArray() }; paths != nullptr)
            for (const auto& path : *paths)
                watcher.addFolder (juce::File { path.toString() }.getParentDirectory());
    }
    else if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::language)) and childWhichHasBeenAdded.hasProperty (Id::path))
    {
        watcher.addFolder (juce::File { childWhichHasBeenAdded[Id::path].toString() });
    }
}

void LINGModel::fileChanged (const juce::File& file, jam::File::Watcher::Event)
{
    const auto path { file.getFullPathName() };
    const auto files { state.getChildWithName (jam::Format::toType (Id::files)) };
    const auto languageFolder { file.getParentDirectory().getParentDirectory() };
    const auto folder { getLanguageLibrary (languageFolder) == file ? languageFolder : file.getParentDirectory() };
    auto languageRow { state.getChildWithProperty (Id::path, folder.getFullPathName()) };
    const auto isPath { [&path] (const juce::var& entry) { return entry.toString().compare (path) == 0; } };
    const auto hasPath { [&files, &isPath] (const juce::Identifier& property)
    {
        const auto* paths { files[property].getArray() };
        return paths != nullptr and std::any_of (paths->begin(), paths->end(), isPath);
    } };

    if (isPath (state[Id::manifest]) or hasPath (Id::buildFiles))
    {
        state.sendPropertyChangeMessage (Id::manifest);
    }
    else if (languageRow.hasType (jam::Format::toType (Id::language)))
    {
        languageRow.sendPropertyChangeMessage (Id::path);
    }
    else
    {
        auto row { files.getChildWithProperty (Id::path, path) };

        if (row.isValid())
            row.sendPropertyChangeMessage (Id::path);
    }
}
