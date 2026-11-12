#include "LINGModel.h"
#include "Validator.h"
#include "CompilerManifest.h"

LINGModel::LINGModel()
    : jam::Model (jam::Format::toType (Id::ling))
{
    static constexpr int maximumMessageLength { 4096 };

    getOrCreateChildWithName (jam::Format::toType (Id::files));
    auto overlayRow { getOrCreateChildWithName (jam::Format::toType (Id::overlay)) };
    createAndAddParameter<jam::ParameterText> (overlayRow, Id::message, juce::String {}, maximumMessageLength);

    watcher.coalesceEvents (coalesceMs);
    watcher.addListener (this);
}

void LINGModel::setMessage (const juce::String& text)
{
    auto* param { getParameter<jam::ParameterText> (jam::Format::toType (Id::overlay), Id::message) };
    jassert (param != nullptr);

    if (param != nullptr)
        param->setValue (text);
}

void LINGModel::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    jam::Model::valueTreePropertyChanged (tree, property);

    if (tree == state and property == Id::manifest)
    {
        const juce::File manifestFile { state[Id::manifest].toString() };
        const auto manifest { jam::ConfigDocument::parse (manifestFile.loadFileAsString(), manifestFile.getFullPathName()) };
        const auto result { Validator {}.isValid (manifest) };
        const auto files (result.wasOk() ? CompilerManifest::getFiles (*this, manifest, manifestFile, state[Id::toolchainArgument].toString())
                                         : juce::ValueTree { jam::Format::toType (Id::files) });

        if (not result.wasOk())
            setMessage (result.getErrorMessage());

        removeChild (state.getChildWithName (jam::Format::toType (Id::files)));
        appendChild (files);
    }
}

void LINGModel::valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
{
    jam::Model::valueTreeChildAdded (parentTree, childWhichHasBeenAdded);

    if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::files)))
        for (const auto& row : childWhichHasBeenAdded)
        {
            const auto folder { juce::File { row[Id::path].toString() }.getParentDirectory() };

            if (folder.isDirectory())
                watcher.addFolder (folder);
        }
}

void LINGModel::fileChanged (const juce::File& file, jam::File::Watcher::Event)
{
    auto row { state.getChildWithName (jam::Format::toType (Id::files)).getChildWithProperty (Id::path, file.getFullPathName()) };

    if (row.isValid())
        row.sendPropertyChangeMessage (Id::path);
}
