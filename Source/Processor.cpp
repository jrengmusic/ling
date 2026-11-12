#include "Processor.h"

Processor::Processor (LINGModel& newModel)
    : model (newModel)
{
    model.addListener (this);
}

Processor::~Processor()
{
    model.removeListener (this);
}

void Processor::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& childWhichHasBeenAdded)
{
    if (childWhichHasBeenAdded.hasType (jam::Format::toType (Id::files)))
    {
        for (int rowIndex { 0 }; rowIndex < childWhichHasBeenAdded.getNumChildren(); ++rowIndex)
            addJob (childWhichHasBeenAdded.getChild (rowIndex), rowIndex);

        if (pendingJobs == 0)
            setDocuments();
    }
}

void Processor::valueTreeChildRemoved (juce::ValueTree&, juce::ValueTree& childWhichHasBeenRemoved, int)
{
    if (childWhichHasBeenRemoved.hasType (jam::Format::toType (Id::files)))
        for (const auto& row : childWhichHasBeenRemoved)
            documents.erase (paths.indexOf (row[Id::path].toString(), false, 0));
}

void Processor::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == Id::path and tree.hasType (jam::Format::toType (Id::file)))
        addJob (tree, tree.getParent().indexOf (tree));
}

void Processor::addJob (const juce::ValueTree& row, int rowIndex)
{
    ++pendingJobs;

    pool.addJob ([this, rowIndex, path = row[Id::path].toString(), family = static_cast<int> (row[Id::family])]
    {
        const juce::File file { path };
        const auto modificationTime { file.getLastModificationTime() };
        auto document { file.existsAsFile() ? jam::SyntaxTree::parse (jam::Strings::getView (file.loadFileAsString()), family) : nullptr };
        const auto result { document != nullptr ? jam::SyntaxTree::isValid (*document, family) : juce::Result::ok() };
        const auto parsed { std::make_shared<std::unique_ptr<jam::CodeDocument>> (std::move (document)) };

        juce::MessageManager::callAsync ([this, rowIndex, path, modificationTime, parsed, result]
        {
            addDocument (rowIndex, path, modificationTime, std::move (*parsed), result);
        });
    });
}

void Processor::addDocument (int rowIndex, const juce::String& path, juce::Time modificationTime, std::unique_ptr<jam::CodeDocument> document, const juce::Result& result)
{
    --pendingJobs;

    const auto row { model.state.getChildWithName (jam::Format::toType (Id::files)).getChild (rowIndex) };

    if (row[Id::path].toString().compare (path) == 0 and juce::File { path }.getLastModificationTime() == modificationTime)
    {
        const auto pathIndex { getOrCreatePathIndex (path) };

        if (document != nullptr)
            documents.addOrReplace (pathIndex, std::move (document));
        else
            documents.erase (pathIndex);

        if (not result.wasOk())
            model.setMessage (path + Id::diagnosticSeparator + result.getErrorMessage());
    }

    if (pendingJobs == 0)
        setDocuments();
}

int Processor::getOrCreatePathIndex (const juce::String& path)
{
    if (const auto pathIndex { paths.indexOf (path, false, 0) }; pathIndex >= 0)
        return pathIndex;

    paths.add (path);
    return paths.size() - 1;
}

void Processor::setDocuments()
{
    const juce::var count { static_cast<int> (documents.size()) };

    if (model.state[Id::documents] == count)
        model.state.sendPropertyChangeMessage (Id::documents);
    else
        model.state.setProperty (Id::documents, count, nullptr);
}
