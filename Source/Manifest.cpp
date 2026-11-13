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

void Manifest::addJob()
{
    pool.addJob ([this, manifestPath = model.state[Id::manifest].toString(), toolchainArgument = model.state[Id::toolchainArgument].toString()]
    {
        const juce::File manifestFile { manifestPath };
        const auto text { manifestFile.loadFileAsString() };
        const auto manifest { jam::ConfigDocument::parse (text, manifestFile.getFullPathName()) };
        const auto result { manifest != nullptr ? Validator {}.isValid (*manifest) : jam::ConfigDocument::isValid (text, manifestFile.getFullPathName()) };
        const auto timeout { result.wasOk() ? getTimeout (*manifest) : LINGModel::defaultTimeoutMs };
        const auto files (result.wasOk() ? CompilerManifest::getFiles (model, *manifest, manifestFile, toolchainArgument)
                                         : juce::ValueTree { jam::Format::toType (Id::files) });

        if (not result.wasOk())
            model.setMessage (result.getErrorMessage());

        juce::MessageManager::callAsync ([this, files, timeout] { setFiles (files, timeout); });
    });
}

void Manifest::setFiles (const juce::ValueTree& files, int timeout)
{
    model.state.setProperty (Id::timeout, timeout, nullptr);
    model.removeChild (model.state.getChildWithName (jam::Format::toType (Id::files)));
    model.appendChild (files);
}

int Manifest::getTimeout (const jam::ConfigDocument& manifest)
{
    for (const auto* table : manifest.getTables (Id::daemon))
        if (const auto* row { manifest.getTableRow (*table, Id::timeout) }; row != nullptr)
            return manifest.getTableValue (*row, Id::value).getIntValue();

    return LINGModel::defaultTimeoutMs;
}
