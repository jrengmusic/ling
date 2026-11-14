#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"

/**
 * @struct CompilerManifest
 * @brief Builds the files child of the model from the toolchain rows of a manifest.
 *
 * A C-family toolchain row reads one of two sources. A compiler manifest is a
 * JSON file in the build folder of the row, and it lists the units. A
 * dependency row runs its command, and the record that the command prints lists
 * the dependencies of each unit.
 */
struct CompilerManifest
{
    /**
     * @brief Returns a new files child, built from each toolchain row whose argument matches the toolchain argument.
     *
     * A toolchain argument that matches no toolchain row sets the message.
     *
     * @param manifest           The parsed manifest.
     * @param manifestFile       The manifest file. Its folder is the base of the relative paths in the rows.
     * @param toolchainArgument  The toolchain argument from the command line. Empty when none is given.
     * @return The files child.
     */
    static juce::ValueTree getFiles (const jam::MarkdownDocument& manifest, const juce::File& manifestFile, const juce::String& toolchainArgument);

    static jam::Strings getCheckFlags (LINGModel& model, int family);

private:
    static void applyDefaults (const juce::File& folder, const juce::String& name);

    static std::unique_ptr<jam::MarkdownDocument> getConstruction (const juce::File& file);

    static juce::String getWord (juce::ValueTree& files, const jam::MarkdownDocument& construction, const juce::Identifier& word);

    /**
     * @brief Returns the construction of each family that holds a compiler manifest table, and writes the absent default files.
     *
     * The default files are written only into the folder of the default family, and
     * each file is written only when it does not exist. A construction without a
     * compiler manifest table is left out. A construction file that exists and does
     * not parse sets the message.
     *
     * @param files  The files child that receives the message.
     * @return The constructions of the families that hold a compiler manifest table.
     */
    static jam::Owner<jam::MarkdownDocument> getOrCreateConstructions (juce::ValueTree& files);

    /**
     * @brief Adds the files of a C-family toolchain row, by the command of the row.
     *
     * A compiler manifest command adds the units. A dependency command adds the
     * dependency records. Any other command adds nothing.
     *
     * @param files         The files child that receives the rows.
     * @param manifest      The parsed manifest.
     * @param row           The toolchain row.
     * @param directory     The folder of the manifest.
     * @param construction  The construction of the family. It holds the command words.
     */
    static void addFiles (juce::ValueTree& files, const jam::MarkdownDocument& manifest, const jam::MarkdownDocument::Element& row, const juce::File& directory, const jam::MarkdownDocument& construction);

    /**
     * @brief Adds a file row for each unit of the compiler manifest in the build folder of the row.
     *
     * A row whose flag names no build folder adds nothing. When the compiler
     * manifest exists, its path joins the compiler manifests of the files child
     * before the file is parsed. A missing compiler manifest sets the message. A
     * compiler manifest that does not parse sets the message.
     *
     * @param files         The files child that receives the rows.
     * @param manifest      The parsed manifest.
     * @param row           The toolchain row whose flag names the build folder.
     * @param directory     The folder of the manifest. The build folder is relative to it.
     * @param construction  The construction of the family. It holds the command words.
     */
    static void addUnits (juce::ValueTree& files, const jam::MarkdownDocument& manifest, const jam::MarkdownDocument::Element& row, const juce::File& directory, const jam::MarkdownDocument& construction);

    /**
     * @brief Adds a file row for one unit, when its file exists and no row has its path.
     *
     * The row gets the path, the family from the file extension, and the output
     * path. A unit whose file does not exist sets the message.
     *
     * @param files     The files child that receives the row.
     * @param document  The parsed compiler manifest that holds the unit and its text.
     * @param unit      The unit: its directory, file, output, and arguments.
     */
    static void addUnit (juce::ValueTree& files, const jam::JsonDocument& document, const jam::Document::Element& unit);

    /**
     * @brief Returns the arguments of a unit: its arguments array, or its command split into arguments.
     *
     * The command is split with jam::ChildProcess::getArguments when the unit has
     * no arguments array.
     *
     * @param document  The parsed compiler manifest that holds the unit and its text.
     * @param unit      The unit whose arguments are read.
     * @return The arguments of the unit.
     */
    static jam::Strings getCommandArguments (const jam::JsonDocument& document, const jam::Document::Element& unit);

    /**
     * @brief Runs the dependency command of a row, and adds the records that it prints.
     *
     * The command runs in the folder of the manifest, and its output is read as
     * the record. The call waits for the process to end. A zero exit adds the
     * records, and the path of the record file in the build folder joins the
     * dependency records of the files child. A nonzero exit sets the message, and
     * no record is added.
     *
     * @param files         The files child that receives the rows.
     * @param manifest      The parsed manifest.
     * @param row           The toolchain row whose command and flag run.
     * @param directory     The folder of the manifest, which is the working folder of the process.
     * @param construction  The construction of the family. It holds the command words.
     */
    static void addDependencies (juce::ValueTree& files, const jam::MarkdownDocument& manifest, const jam::MarkdownDocument::Element& row, const juce::File& directory, const jam::MarkdownDocument& construction);

    /**
     * @brief Returns the argument list of a dependency command: the command, the flag arguments, and the dependency flag.
     *
     * @param command        The command of the toolchain row.
     * @param flagArguments  The arguments of the flag of the row.
     * @param dependencyFlag The dependency flag of the construction. Its arguments follow the flag arguments.
     * @return The argument list of the process.
     */
    static jam::Strings getDependencyArguments (const juce::String& command, const jam::Strings& flagArguments, const juce::String& dependencyFlag);

    /**
     * @brief Adds the file rows and the dependency lists that a dependency record names.
     *
     * A file row with an output and no dependency list after the record sets the message.
     *
     * @param files           The files child that receives the rows.
     * @param record          The dependency record: the text that the command printed.
     * @param buildDirectory  The folder that the paths of the record are relative to.
     * @param dependencyHead  The text that marks the output in a head line of the record.
     */
    static void addRecords (juce::ValueTree& files, std::string_view record, const juce::File& buildDirectory, const juce::String& dependencyHead);

    /**
     * @brief Reads one record of a dependency record, and returns the position of the next record.
     *
     * A head line that names an output links the indented lines after it to
     * that unit as its dependencies. The unit is found by its output path.
     *
     * @param files           The files child that holds the units.
     * @param paths           The paths of the file rows, in row order. A new dependency path is added here.
     * @param record          The dependency record.
     * @param position        The index of the first byte of the record.
     * @param buildDirectory  The folder that the paths of the record are relative to.
     * @param dependencyHead  The text that marks the output in a head line of the record.
     * @return The index of the first byte after the record.
     */
    static size_t addRecord (juce::ValueTree& files, jam::Strings& paths, std::string_view record, size_t position, const juce::File& buildDirectory, const juce::String& dependencyHead);

    /**
     * @brief Returns the row index of a file, and appends a row when no row has its path.
     *
     * The paths hold the rows in row order, so an index of paths is the index of
     * its row. A new row takes the family of the unit.
     *
     * @param files  The files child that receives a new row.
     * @param paths  The paths of the rows, in row order. A new path is added here.
     * @param unit   The unit whose family a new row takes.
     * @param file   The dependency file.
     * @return The row index of the file.
     */
    static int getOrCreateFileIndex (juce::ValueTree& files, jam::Strings& paths, const juce::ValueTree& unit, const juce::File& file);

    /**
     * @brief Returns the argument that follows a directory flag.
     *
     * @param flagArguments  The arguments of the flag of a row.
     * @param directoryFlag  The flag that names the directory.
     * @return The argument after the first directoryFlag, or an empty string when none follows.
     */
    static juce::String getBuildDirectory (const jam::Strings& flagArguments, juce::StringRef directoryFlag);
};
