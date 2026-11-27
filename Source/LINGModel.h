#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

/**
 * @class LINGModel
 * @brief Shared state of one LING run: files, roots, connections, buffers, languages, overlay parameters, and directory watcher.
 *
 * Holds the state trees of one run. Changing Id::manifest watches the manifest
 * folder; adding Id::files watches row folders and Id::buildFiles folders; adding
 * Id::language watches the language folder. File changes notify the manifest,
 * language row, or file row via property change messages.
 */
class LINGModel
    : public jam::Model
    , private jam::File::Watcher::Listener
{
public:
    /**
     * @brief Initializes state children, overlay parameters, default timeout, and file watcher listener.
     *
     * Creates files, roots, connections, buffers, and overlay (with Id::message,
     * Id::jobCount, and Id::pendingJobCount), stores defaultTimeoutMs in
     * Id::timeout, and configures watcher coalescing with coalesceMs.
     */
    LINGModel();

    /** @brief Releases the model. Members release their own resources. */
    ~LINGModel() override = default;

    /**
     * @brief Writes @p text to the Id::message parameter of the overlay row.
     *
     * @param text  Diagnostic or status message text.
     */
    void setMessage (const juce::String& text);

    /** @brief Default idle daemon timeout in milliseconds when the manifest sets none or fails validation. */
    static constexpr int defaultTimeoutMs { 60000 };

    /** @brief Default worker thread count used at Processor startup and when the manifest sets none or fails validation. */
    static int getDefaultThreads();

    /** @brief Coalescing window in milliseconds for merging repeated watcher events on the same file. */
    static constexpr int coalesceMs { 300 };
    /** @brief Default file-watcher coalescing window in milliseconds when the manifest sets none or fails validation. */
    static constexpr int defaultCoalesceMs { 1000 };

    /**
     * @brief Returns the Wyhash hex identifier for @p manifest and @p toolchainArgument.
     *
     * Hashes the full path of @p manifest, a newline, and @p toolchainArgument.
     *
     * @param manifest           Manifest file identifying the project.
     * @param toolchainArgument  Toolchain argument, or empty when none is given.
     * @return Hexadecimal project identifier string.
     */
    static std::string getProjectName (const juce::File& manifest, std::string_view toolchainArgument);

    /**
     * @brief Returns the Wyhash hex digest of the current executable binary.
     *
     * Isolates the daemon port file per built executable so rebuilt binaries do not
     * collide with older daemons.
     *
     * @return Hexadecimal daemon identifier string.
     */
    static std::string getDaemonName();

    /**
     * @brief Returns the base LING cache folder.
     *
     * Uses local application data on Windows and the home folder elsewhere. Does
     * not create the folder on disk.
     *
     * @return Base cache folder.
     */
    static juce::File getCacheDirectory();

    /**
     * @brief Returns the LING config folder inside the user home directory.
     *
     * Does not create the folder on disk. Language subdirectories live under its
     * language child folder.
     *
     * @return Base config folder.
     */
    static juce::File getConfigDirectory();

    /**
     * @brief Returns sorted language subdirectories in config that contain a construction file.
     *
     * @return Sorted array of valid language folders.
     */
    static juce::Array<juce::File> getLanguageDirectories();

    /**
     * @brief Returns language directories, seeding the default language files from BinaryData if none exist.
     *
     * @return Sorted array of valid language folders.
     */
    static juce::Array<juce::File> getOrCreateLanguageDirectories();

    /**
     * @brief Returns the host platform dynamic library file path inside @p languageFolder.
     *
     * @param languageFolder  Language directory containing the built grammar library.
     * @return Expected dynamic library file for the host OS.
     */
    static juce::File getLanguageLibrary (const juce::File& languageFolder);

    /**
     * @brief Returns the per-project cache directory for @p manifest and @p toolchainArgument.
     *
     * @param manifest           Manifest file identifying the project.
     * @param toolchainArgument  Toolchain argument, or empty when none is given.
     * @return Project cache directory under getCacheDirectory().
     */
    static juce::File getCacheDirectory (const juce::File& manifest, std::string_view toolchainArgument);

    /**
     * @brief Returns the project root folder for @p manifest.
     *
     * @param manifest  Manifest file or project directory.
     * @return @p manifest if it is a directory, otherwise its parent directory.
     */
    static juce::File getProjectDirectory (const juce::File& manifest);

    /**
     * @brief Returns the file named fileName, searched from folder upward through the parent folders.
     *
     * The search stops at the first folder that holds the file, or at the root folder. A link is
     * resolved to its linked target.
     *
     * @param folder    The folder where the search starts.
     * @param fileName  The file name to find.
     * @return The file found, or an empty juce::File when no folder up to the root holds it.
     */
    static juce::File getManifestFile (const juce::File& folder, const juce::String& fileName);

    /**
     * @brief Returns the patched JUCE cache directory keyed by SHA-256 of @p version and @p patchFiles.
     *
     * Strips CR before LF from each patch file so line endings produce stable
     * hashes across platforms.
     *
     * @param version     JUCE version identifier string.
     * @param patchFiles  Ordered list of patch file paths applied to JUCE.
     * @return Cache folder for the hashed patch set.
     */
    static juce::File getPatchedDirectory (std::string_view version, const jam::Strings& patchFiles);

    /**
     * @brief Returns the swap file path for @p file with @p extension under the longest matching root or project cache.
     *
     * Under a matching root's swap directory, syntax swaps (files::lingSwap) share a
     * common tree while other extensions (such as files::lingSymbols) nest under the
     * per-project toolchain identifier.
     *
     * @param file       Source file whose swap path is requested.
     * @param extension  Swap file extension (such as files::lingSwap or files::lingSymbols).
     * @return Swap file path inside the matching root's swap tree or hashed under the project cache.
     */
    juce::File getSwapFile (const juce::File& file, juce::StringRef extension) const;
    /**
     * @brief Tells whether @p file equals or lies beneath any directory in @p roots.
     *
     * Returns @c true unconditionally when @p roots is empty.
     *
     * @param roots  Root directories restricting indexed dependencies.
     * @param file   Candidate file to test.
     * @return @c true when @p roots is empty or contains @p file.
     */
    static bool isWithinRoots (const jam::Array<juce::File>& roots, const juce::File& file);

private:
    /**
     * @brief Adds the manifest folder to @c watcher when Id::manifest changes on @c state.
     *
     * @param tree      Tree whose property changed.
     * @param property  Identifier of the changed property.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Adds watched folders when Id::files or Id::language children are added.
     *
     * For Id::files, watches existing parent directories of file rows and parent
     * directories of Id::buildFiles entries. For Id::language with Id::path,
     * watches the language directory.
     *
     * @param parentTree              Tree receiving the child.
     * @param childWhichHasBeenAdded  Added child tree.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Routes file-system changes to Id::manifest, an Id::language row, or an Id::file row.
     *
     * Manifest and Id::buildFiles changes emit Id::manifest; files or compiled
     * libraries inside a watched language folder emit Id::path on that language
     * row; matching source paths emit Id::path on their file row.
     *
     * @param file   Changed file reported by @c watcher.
     * @param event  Watcher event kind (unused).
     */
    void fileChanged (const juce::File& file, jam::File::Watcher::Event event) override;

    /** @brief Directory watcher reporting changes for manifest, build, language, and source files. */
    jam::File::Watcher watcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LINGModel)
};
