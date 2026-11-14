#include "LINGModel.h"

LINGModel::LINGModel()
    : jam::Model (jam::Format::toType (Id::ling))
{
    static constexpr int maximumMessageLength { 4096 };

    getOrCreateChildWithName (jam::Format::toType (Id::files));
    getOrCreateChildWithName (jam::Format::toType (Id::roots));
    getOrCreateChildWithName (jam::Format::toType (Id::connections));
    auto overlayRow { getOrCreateChildWithName (jam::Format::toType (Id::overlay)) };
    createAndAddParameter<jam::ParameterText> (overlayRow, Id::message, juce::String {}, maximumMessageLength);
    state.setProperty (Id::timeout, defaultTimeoutMs, nullptr);

    watcher.coalesceEvents (coalesceMs);
    watcher.addListener (this);
}

void LINGModel::setMessage (const juce::String& text)
{
    auto* param { getParameter<jam::ParameterText> (jam::Format::toType (Id::overlay), Id::message) };
    jassert (param != nullptr);

    param->setValue (text);
}

std::string LINGModel::getDaemonName (const juce::File& manifest, std::string_view toolchainArgument)
{
    const auto manifestPath { manifest.getFullPathName() };
    std::string key { jam::Strings::getView (manifestPath) };
    std::string name;

    jam::Format::appendUtf8 (key, Chars::newline);
    key.append (toolchainArgument);
    jam::Format::appendHex (name, jam::Wyhash::hashBytes (key.data(), key.size()));
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

juce::File LINGModel::getCacheDirectory (const juce::File& manifest, std::string_view toolchainArgument)
{
    return getCacheDirectory().getChildFile (jam::Strings::fromView (getDaemonName (manifest, toolchainArgument)));
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

juce::File LINGModel::getSwapFile (const juce::File& file) const
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

    if (row.isValid())
        return juce::File { row[Id::swap].toString() }.getChildFile (jam::Format::toFileName (file.getRelativePathFrom (juce::File { row[Id::path].toString() }), files::lingSwap));

    const auto path { file.getFullPathName() };
    const auto bytes { jam::Strings::getView (path) };
    std::string name;

    jam::Format::appendHex (name, jam::Wyhash::hashBytes (bytes.data(), bytes.size()));
    return getCacheDirectory().getChildFile (jam::Format::toFileName (jam::Strings::fromView (name), files::lingSwap));
}

void LINGModel::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    jam::Model::valueTreePropertyChanged (tree, property);

    if (tree == state and property == Id::manifest)
        watcher.addFolder (juce::File { state[Id::manifest].toString() }.getParentDirectory());
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

        for (const auto& property : { Id::compilerManifests, Id::dependencyRecords })
            if (const auto* paths { childWhichHasBeenAdded[property].getArray() }; paths != nullptr)
                for (const auto& path : *paths)
                    watcher.addFolder (juce::File { path.toString() }.getParentDirectory());
    }
    else if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::language)))
    {
        watcher.addFolder (juce::File { childWhichHasBeenAdded[Id::path].toString() });
    }
}

void LINGModel::fileChanged (const juce::File& file, jam::File::Watcher::Event)
{
    const auto path { file.getFullPathName() };
    const auto files { state.getChildWithName (jam::Format::toType (Id::files)) };
    auto languageRow { state.getChildWithProperty (Id::path, file.getParentDirectory().getFullPathName()) };
    const auto isPath { [&path] (const juce::var& entry) { return entry.toString().compare (path) == 0; } };
    const auto hasPath { [&files, &isPath] (const juce::Identifier& property)
    {
        const auto* paths { files[property].getArray() };
        return paths != nullptr and std::any_of (paths->begin(), paths->end(), isPath);
    } };

    if (isPath (state[Id::manifest]) or hasPath (Id::compilerManifests) or hasPath (Id::dependencyRecords))
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
