#include "CompilerManifest.h"

juce::ValueTree CompilerManifest::getFiles (LINGModel& model, const jam::ConfigDocument& manifest, const juce::File& manifestFile, const juce::String& toolchainArgument)
{
    static const jam::SharedInstance<map::Family> family { std::in_place };

    const auto& filesConstruction { getFilesConstruction() };
    const auto directory { manifestFile.getParentDirectory() };
    juce::ValueTree files { jam::Format::toType (Id::files) };
    const auto tables (manifest.getTables (Id::toolchain));
    const auto isToolchainRow { [&manifest, &toolchainArgument] (const jam::ConfigDocument::Element* row) { return manifest.getTableValue (*row, Id::argument).compare (toolchainArgument) == 0; } };

    for (const auto* table : tables)
    {
        for (const auto* row : manifest.getTableRows (*table))
        {
            if (isToolchainRow (row))
            {
                for (const auto& [familyKey, familyName] : map::Family::getInstance()->get())
                    if (filesConstruction.contains (familyKey))
                        filesConstruction.get (familyKey, model, files, manifest, *row, directory);
            }
        }
    }

    if (toolchainArgument.isNotEmpty() and std::none_of (tables.begin(), tables.end(), [&manifest, &isToolchainRow] (const jam::ConfigDocument::Element* table)
                                                          {
                                                              const auto rows (manifest.getTableRows (*table));
                                                              return std::any_of (rows.begin(), rows.end(), isToolchainRow);
                                                          }))
        model.setMessage (toolchainArgument + Id::diagnosticSeparator + text::Diagnostics::failToolchainArgument);

    return files;
}

const jam::Function::Map<int, jam::Function::pointerCapacity, void (LINGModel&, juce::ValueTree&, const jam::ConfigDocument&, const jam::ConfigDocument::Element&, const juce::File&)>& CompilerManifest::getFilesConstruction()
{
    static const auto filesConstruction (
        []
        {
            jam::Function::Map<int, jam::Function::pointerCapacity, void (LINGModel&, juce::ValueTree&, const jam::ConfigDocument&, const jam::ConfigDocument::Element&, const juce::File&)> filesConstruction;

            filesConstruction.add (map::Family::cLang, addCLanguageFiles);

            return filesConstruction;
        }()
    );

    return filesConstruction;
}

void CompilerManifest::addCLanguageFiles (const int&, LINGModel& model, juce::ValueTree& files, const jam::ConfigDocument& manifest, const jam::ConfigDocument::Element& row, const juce::File& directory)
{
    const auto command { manifest.getTableValue (row, Id::command) };

    if (command.compare (text::CLanguage::compilerManifestCommand) == 0)
        addCLanguageUnits (model, files, manifest, row, directory);
    else if (command.compare (text::CLanguage::dependencyCommand) == 0)
        addCLanguageDependencies (model, files, manifest, row, directory);
}

void CompilerManifest::addCLanguageUnits (LINGModel& model, juce::ValueTree& files, const jam::ConfigDocument& manifest, const jam::ConfigDocument::Element& row, const juce::File& directory)
{
    const auto flagArguments (jam::ChildProcess::getArguments (jam::Strings::getView (manifest.getTableValue (row, Id::flag))));
    const auto buildDirectory { getBuildDirectory (flagArguments, text::CLanguage::flagBuildDirectory) };

    if (buildDirectory.isNotEmpty())
    {
        const auto compilerManifest { directory.getChildFile (buildDirectory).getChildFile (text::CLanguage::compilerManifest) };

        if (compilerManifest.existsAsFile())
        {
            const auto entries { juce::JSON::parse (compilerManifest) };

            if (const auto* units { entries.getArray() })
            {
                for (const auto& unit : *units)
                    addCLanguageUnit (model, files, unit);
            }
            else
            {
                model.setMessage (compilerManifest.getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failCompilerManifestInvalid);
            }
        }
        else
        {
            model.setMessage (compilerManifest.getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failCompilerManifestNotFound);
        }
    }
}

void CompilerManifest::addCLanguageUnit (LINGModel& model, juce::ValueTree& files, const juce::var& unit)
{
    const juce::File directory { unit[Id::directory].toString() };
    const auto file { directory.getChildFile (unit[Id::file].toString()) };
    const auto commandArguments (getCommandArguments (unit));

    if (file.existsAsFile())
    {
        if (not files.getChildWithProperty (Id::path, file.getFullPathName()).isValid())
        {
            juce::ValueTree row { jam::Format::toType (Id::file) };
            row.setProperty (Id::path, file.getFullPathName(), nullptr);
            row.setProperty (Id::family, jam::SyntaxTree::getFamily (file), nullptr);
            row.setProperty (Id::output, directory.getChildFile (unit[Id::output].toString()).getFullPathName(), nullptr);
            files.appendChild (row, nullptr);
        }
    }
    else
    {
        model.setMessage (file.getFullPathName() + Id::diagnosticSeparator + text::Diagnostics::failUnitNotFound);
    }
}

jam::Strings CompilerManifest::getCommandArguments (const juce::var& unit)
{
    if (const auto* arguments { unit[Id::arguments].getArray() })
    {
        jam::Strings commandArguments;

        for (const auto& argument : *arguments)
            commandArguments.add (argument.toString());

        return commandArguments;
    }

    return jam::ChildProcess::getArguments (jam::Strings::getView (unit[Id::command].toString()));
}

void CompilerManifest::addCLanguageDependencies (LINGModel& model, juce::ValueTree& files, const jam::ConfigDocument& manifest, const jam::ConfigDocument::Element& row, const juce::File& directory)
{
    const auto command { manifest.getTableValue (row, Id::command) };
    const auto flag { manifest.getTableValue (row, Id::flag) };
    const auto flagArguments (jam::ChildProcess::getArguments (jam::Strings::getView (flag)));
    juce::WaitableEvent finished;
    int exitCode { EXIT_FAILURE };
    std::string record;
    jam::Subprocess subprocess;

    subprocess.launch (getDependencyArguments (command, flagArguments),
                       directory,
                       [&finished, &exitCode] (int processExitCode, const std::string&)
                       {
                           exitCode = processExitCode;
                           finished.signal();
                       },
                       [&record] (std::string_view chunk, bool) { record.append (chunk); });

    finished.wait();

    if (exitCode == 0)
        addCLanguageRecords (model, files, record, directory.getChildFile (getBuildDirectory (flagArguments, text::CLanguage::flagDependencyDirectory)));
    else
        model.setMessage (command + Chars::space + flag + Id::diagnosticSeparator + text::Diagnostics::failToolchain);
}

juce::StringArray CompilerManifest::getDependencyArguments (const juce::String& command, const jam::Strings& flagArguments)
{
    juce::StringArray arguments { command };

    for (const auto& argument : flagArguments)
        arguments.add (argument);

    for (const auto& argument : jam::ChildProcess::getArguments (text::CLanguage::dependencyFlag))
        arguments.add (argument);

    return arguments;
}

void CompilerManifest::addCLanguageRecords (LINGModel& model, juce::ValueTree& files, std::string_view record, const juce::File& buildDirectory)
{
    jam::Strings paths;

    for (const auto& file : files)
        paths.add (file[Id::path].toString());

    for (size_t position { 0 }; position < record.size();)
        position = addCLanguageRecord (files, paths, record, position, buildDirectory);

    for (const auto& file : files)
        if (file.hasProperty (Id::output) and not file.hasProperty (Id::dependencies))
            model.setMessage (file[Id::path].toString() + Id::diagnosticSeparator + text::Diagnostics::failDependenciesNotFound);
}

size_t CompilerManifest::addCLanguageRecord (juce::ValueTree& files, jam::Strings& paths, std::string_view record, size_t position, const juce::File& buildDirectory)
{
    const auto headEnd { std::min (record.find (static_cast<char> (Chars::newline), position), record.size()) };
    const auto head { record.substr (position, headEnd - position) };
    const auto outputEnd { head.find (text::CLanguage::dependencyHead) };
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
