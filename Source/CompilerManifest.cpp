#include "CompilerManifest.h"

juce::ValueTree CompilerManifest::getFiles (const jam::MarkdownDocument& manifest, const juce::File& manifestFile, const juce::String& toolchainArgument)
{
    juce::ValueTree files { jam::Format::toType (Id::files) };
    const auto constructions (getOrCreateConstructions (files));
    const auto directory { manifestFile.getParentDirectory() };
    const auto tables (manifest.getTables (Id::toolchain));
    const auto isToolchainRow { [&manifest, &toolchainArgument] (const jam::MarkdownDocument::Element* row) { return manifest.getTableValue (*row, Id::argument).compare (toolchainArgument) == 0; } };

    if (constructions.isEmpty())
        files.setProperty (Id::message, LINGModel::getConfigDirectory().getChildFile (files::language).getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failConstructionNotFound, nullptr);

    for (const auto* table : tables)
    {
        for (const auto* row : manifest.getTableRows (*table))
        {
            if (isToolchainRow (row))
            {
                for (const auto& construction : constructions)
                    addFiles (files, manifest, *row, directory, *construction);
            }
        }
    }

    if (toolchainArgument.isNotEmpty() and std::none_of (tables.begin(), tables.end(), [&manifest, &isToolchainRow] (const jam::MarkdownDocument::Element* table)
                                                          {
                                                              const auto rows (manifest.getTableRows (*table));
                                                              return std::any_of (rows.begin(), rows.end(), isToolchainRow);
                                                          }))
        files.setProperty (Id::message, toolchainArgument + Id::diagnosticSeparator + text::Diagnostics::failToolchainArgument, nullptr);

    return files;
}

jam::Owner<jam::MarkdownDocument> CompilerManifest::getOrCreateConstructions (juce::ValueTree& files)
{
    jam::Owner<jam::MarkdownDocument> constructions;

    for (const auto& [familyKey, familyName] : map::Family::getInstance()->get())
    {
        const auto folder { LINGModel::getConfigDirectory().getChildFile (files::language).getChildFile (familyName) };
        const auto file { folder.getChildFile (files::languageConstruction) };

        applyDefaults (folder, familyName);

        if (auto construction { getConstruction (file) }; construction != nullptr)
        {
            if (not construction->getTables (Id::compilerManifest).isEmpty())
                constructions.add (std::move (construction));
        }
        else if (file.existsAsFile())
        {
            files.setProperty (Id::message, jam::MarkdownDocument::isValid (jam::Strings::getView (file.loadFileAsString()), file.getFullPathName()).getErrorMessage(), nullptr);
        }
    }

    return constructions;
}

jam::Strings CompilerManifest::getCheckFlags (LINGModel& model, int family)
{
#if JUCE_WINDOWS
    const auto& checkFlags { Id::checkFlagsWindows };
#else
    const auto& checkFlags { Id::checkFlagsMac };
#endif

    const auto familyName { map::Family::getInstance()->get (family) };
    const auto folder { LINGModel::getConfigDirectory().getChildFile (files::language).getChildFile (familyName) };
    const auto file { folder.getChildFile (files::languageConstruction) };
    jam::Strings flags;

    if (const auto construction { getConstruction (file) }; construction != nullptr)
    {
        for (const auto* row : construction->getTableRows (checkFlags))
            flags.add (construction->getTableValue (*row, Id::value));
    }
    else if (file.existsAsFile())
    {
        model.setMessage (jam::MarkdownDocument::isValid (jam::Strings::getView (file.loadFileAsString()), file.getFullPathName()).getErrorMessage());
    }

    return flags;
}

void CompilerManifest::applyDefaults (const juce::File& folder, const juce::String& name)
{
    if (juce::Identifier { name } == juce::Identifier { files::defaultLanguage })
    {
        folder.createDirectory();

        for (const auto& fileName : { files::languageSpell, files::languageTemplate, files::languageConstruction, files::syntaxTree, files::core, files::languageInterface })
        {
            const auto file { folder.getChildFile (fileName) };

            if (not file.exists())
            {
                const BinaryData::Raw binary { fileName };
                file.replaceWithData (binary.data, static_cast<size_t> (binary.size));
            }
        }
    }
}

std::unique_ptr<jam::MarkdownDocument> CompilerManifest::getConstruction (const juce::File& file)
{
    if (file.existsAsFile())
        return jam::MarkdownDocument::parse (file.loadFileAsString(), file.getFullPathName());

    return nullptr;
}

juce::String CompilerManifest::getWord (juce::ValueTree& files, const jam::MarkdownDocument& construction, const juce::Identifier& word)
{
    if (const auto* row { construction.getTableRow (Id::compilerManifest, word) }; row != nullptr)
        return construction.getTableValue (*row, Id::value);

    files.setProperty (Id::message, word.toString() + Id::diagnosticSeparator + text::Diagnostics::failNotFound, nullptr);
    return {};
}

void CompilerManifest::addFiles (juce::ValueTree& files, const jam::MarkdownDocument& manifest, const jam::MarkdownDocument::Element& row, const juce::File& directory, const jam::MarkdownDocument& construction)
{
    const auto command { manifest.getTableValue (row, Id::command) };

    if (command.compare (getWord (files, construction, Id::compilerManifestCommand)) == 0)
        addUnits (files, manifest, row, directory, construction);
    else if (command.compare (getWord (files, construction, Id::dependencyCommand)) == 0)
        addDependencies (files, manifest, row, directory, construction);
}

void CompilerManifest::addUnits (juce::ValueTree& files, const jam::MarkdownDocument& manifest, const jam::MarkdownDocument::Element& row, const juce::File& directory, const jam::MarkdownDocument& construction)
{
    const auto flagArguments (jam::ChildProcess::getArguments (jam::Strings::getView (manifest.getTableValue (row, Id::flag))));
    const auto buildDirectory { getBuildDirectory (flagArguments, getWord (files, construction, Id::flagBuildDirectory)) };

    if (buildDirectory.isNotEmpty())
    {
        const auto compilerManifest { directory.getChildFile (buildDirectory).getChildFile (getWord (files, construction, Id::compilerManifest)) };

        if (compilerManifest.existsAsFile())
        {
            auto compilerManifests (files[Id::compilerManifests]);
            compilerManifests.append (compilerManifest.getFullPathName());
            files.setProperty (Id::compilerManifests, compilerManifests, nullptr);

            if (const auto document { jam::JsonDocument::parse (compilerManifest) }; document != nullptr)
            {
                for (const auto* unit : *document->getChildByToken (*document->getRoot(), document->getValue()))
                    addUnit (files, *document, *unit);
            }
            else
            {
                files.setProperty (Id::message, compilerManifest.getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failCompilerManifestInvalid + Id::diagnosticSeparator + jam::JsonDocument::isValid (jam::Strings::getView (compilerManifest.loadFileAsString())).getErrorMessage(), nullptr);
            }
        }
        else
        {
            files.setProperty (Id::message, compilerManifest.getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failCompilerManifestNotFound, nullptr);
        }
    }
}

void CompilerManifest::addUnit (juce::ValueTree& files, const jam::JsonDocument& document, const jam::Document::Element& unit)
{
    const auto getText { [&document, &unit] (const juce::Identifier& key)
    {
        const auto* value { document.find (unit, jam::Strings::getView (key.toString())) };

        return value != nullptr ? jam::Strings::fromView (document.getString (*value)) : juce::String {};
    } };

    const juce::File directory { getText (Id::directory) };
    const auto file { directory.getChildFile (getText (Id::file)) };

    if (file.existsAsFile())
    {
        if (not files.getChildWithProperty (Id::path, file.getFullPathName()).isValid())
        {
            juce::ValueTree row { jam::Format::toType (Id::file) };
            row.setProperty (Id::path, file.getFullPathName(), nullptr);
            row.setProperty (Id::family, jam::SyntaxTree::getFamily (file), nullptr);
            juce::Array<juce::var> arguments;

            for (const auto& argument : getCommandArguments (document, unit))
                arguments.add (argument);

            row.setProperty (Id::output, directory.getChildFile (getText (Id::output)).getFullPathName(), nullptr);
            row.setProperty (Id::directory, directory.getFullPathName(), nullptr);
            row.setProperty (Id::arguments, arguments, nullptr);
            files.appendChild (row, nullptr);
        }
    }
    else
    {
        files.setProperty (Id::message, file.getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failUnitNotFound, nullptr);
    }
}

jam::Strings CompilerManifest::getCommandArguments (const jam::JsonDocument& document, const jam::Document::Element& unit)
{
    if (const auto* arguments { document.find (unit, jam::Strings::getView (Id::arguments.toString())) }; arguments != nullptr)
    {
        jam::Strings commandArguments;

        for (const auto& argument : *document.getChildByToken (unit, *arguments)->get<jam::Document::Tokens> (Id::tokens))
            commandArguments.add (jam::Strings::fromView (document.getString (argument)));

        return commandArguments;
    }

    const auto* command { document.find (unit, jam::Strings::getView (Id::command.toString())) };
    const auto commandText { command != nullptr ? document.getString (*command) : std::string {} };

    return jam::ChildProcess::getArguments (commandText);
}

void CompilerManifest::addDependencies (juce::ValueTree& files, const jam::MarkdownDocument& manifest, const jam::MarkdownDocument::Element& row, const juce::File& directory, const jam::MarkdownDocument& construction)
{
    const auto command { manifest.getTableValue (row, Id::command) };
    const auto flag { manifest.getTableValue (row, Id::flag) };
    const auto flagArguments (jam::ChildProcess::getArguments (jam::Strings::getView (flag)));
    juce::WaitableEvent finished;
    int exitCode { EXIT_FAILURE };
    std::string record;
    jam::Subprocess subprocess;

    subprocess.launch (getDependencyArguments (command, flagArguments, getWord (files, construction, Id::dependencyFlag)),
                       directory,
                       [&finished, &exitCode] (int processExitCode, const std::string&)
                       {
                           exitCode = processExitCode;
                           finished.signal();
                       },
                       [&record] (std::string_view chunk, bool) { record.append (chunk); });

    finished.wait();

    if (exitCode == 0)
    {
        const auto buildDirectory { directory.getChildFile (getBuildDirectory (flagArguments, getWord (files, construction, Id::flagDependencyDirectory))) };
        auto dependencyRecords (files[Id::dependencyRecords]);

        addRecords (files, record, buildDirectory, getWord (files, construction, Id::dependencyHead));
        dependencyRecords.append (buildDirectory.getChildFile (getWord (files, construction, Id::dependencyRecord)).getFullPathName());
        files.setProperty (Id::dependencyRecords, dependencyRecords, nullptr);
    }
    else
    {
        files.setProperty (Id::message, command + Chars::space + flag + Id::diagnosticSeparator + text::Diagnostics::failToolchain, nullptr);
    }
}

jam::Strings CompilerManifest::getDependencyArguments (const juce::String& command, const jam::Strings& flagArguments, const juce::String& dependencyFlag)
{
    jam::Strings arguments { command };

    for (const auto& argument : flagArguments)
        arguments.add (argument);

    for (const auto& argument : jam::ChildProcess::getArguments (jam::Strings::getView (dependencyFlag)))
        arguments.add (argument);

    return arguments;
}

void CompilerManifest::addRecords (juce::ValueTree& files, std::string_view record, const juce::File& buildDirectory, const juce::String& dependencyHead)
{
    jam::Strings paths;

    for (const auto& file : files)
        paths.add (file[Id::path].toString());

    for (size_t position { 0 }; position < record.size();)
        position = addRecord (files, paths, record, position, buildDirectory, dependencyHead);

    for (const auto& file : files)
        if (file.hasProperty (Id::output) and not file.hasProperty (Id::dependencies))
            files.setProperty (Id::message, file[Id::path].toString() + Id::diagnosticSeparator + text::Diagnostics::failDependenciesNotFound, nullptr);
}

size_t CompilerManifest::addRecord (juce::ValueTree& files, jam::Strings& paths, std::string_view record, size_t position, const juce::File& buildDirectory, const juce::String& dependencyHead)
{
    const auto headEnd { std::min (record.find (static_cast<char> (Chars::newline), position), record.size()) };
    const auto head { record.substr (position, headEnd - position) };
    const auto outputEnd { head.find (jam::Strings::getView (dependencyHead)) };
    auto unit { outputEnd != std::string_view::npos
                    ? files.getChildWithProperty (Id::output, buildDirectory.getChildFile (jam::Strings::fromView (head.substr (0, outputEnd))).getFullPathName())
                    : juce::ValueTree {} };
    juce::Array<juce::var> dependencies;
    auto lineStart { headEnd + 1 };

    while (lineStart < record.size() and record.at (lineStart) == static_cast<char> (Chars::space))
    {
        const auto lineEnd { std::min (record.find (static_cast<char> (Chars::newline), lineStart), record.size()) };
        const auto line { record.substr (lineStart, lineEnd - lineStart) };
        const auto pathStart { line.find_first_not_of (static_cast<char> (Chars::space)) };

        if (unit.isValid())
            dependencies.add (getOrCreateFileIndex (files, paths, unit, buildDirectory.getChildFile (jam::Strings::fromView (line.substr (pathStart)))));

        lineStart = lineEnd + 1;
    }

    if (unit.isValid())
        unit.setProperty (Id::dependencies, dependencies, nullptr);

    return lineStart;
}

int CompilerManifest::getOrCreateFileIndex (juce::ValueTree& files, jam::Strings& paths, const juce::ValueTree& unit, const juce::File& file)
{
    const auto path { file.getFullPathName() };

    if (const auto pathIndex { paths.indexOf (path, false, 0) }; pathIndex >= 0)
        return pathIndex;

    juce::ValueTree row { jam::Format::toType (Id::file) };
    row.setProperty (Id::path, path, nullptr);
    row.setProperty (Id::family, unit[Id::family], nullptr);
    files.appendChild (row, nullptr);
    paths.add (path);
    return paths.size() - 1;
}

juce::String CompilerManifest::getBuildDirectory (const jam::Strings& flagArguments, juce::StringRef directoryFlag)
{
    const auto flagIndex { flagArguments.indexOf (directoryFlag, false, 0) };

    if (flagIndex >= 0 and flagIndex + 1 < flagArguments.size())
        return flagArguments.at (flagIndex + 1);

    return {};
}
