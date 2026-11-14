#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

/**
 * @class LINGModel
 * @brief The state of one LING run: the files, the roots, the connections, the language rows, the message, and the watcher of the folders.
 *
 * The model holds the rows of one run. A change of the manifest path adds the
 * folder of the manifest to the watcher. A files child adds the folders of its
 * rows, and the folders of the compiler manifests and dependency records that it
 * names. A language child adds its language folder. A change of a watched file
 * sends a change to the manifest, to its file row, or to its language row.
 */
class LINGModel
    : public jam::Model
    , public jam::Instance<LINGModel>
    , private jam::File::Watcher::Listener
{
public:
    /**
     * @brief Creates the state rows of the run, and listens to the watcher.
     *
     * The rows are the files child, the roots child, the connections child, and the overlay row.
     * The overlay row holds the message parameter. The application prints each
     * change of that parameter. The constructor also writes defaultTimeoutMs to
     * Id::timeout, and it sets the coalescing window of the watcher.
     */
    LINGModel();

    /** @brief Releases the model. The members release their own resources. */
    ~LINGModel() override = default;

    /**
     * @brief Writes text to the message parameter of the overlay row.
     *
     * The manifest reading, the compiler manifest reading, the parse jobs, and the
     * language builds all report their failures through this function.
     *
     * @param text  The message text.
     */
    void setMessage (const juce::String& text);

    /** @brief The daemon timeout, in milliseconds, when the manifest sets none or fails validation. */
    static constexpr int defaultTimeoutMs { 60000 };

    /** @brief The window, in milliseconds, in which the watcher merges the events of one file into one event. */
    static constexpr int coalesceMs { 300 };

    /**
     * @brief Returns the name of the daemon of a manifest and a toolchain argument.
     *
     * The name is the hexadecimal text of a Wyhash hash. The hash covers the full
     * path of the manifest, a newline, and the toolchain argument. The same
     * manifest and toolchain argument give the same name.
     *
     * @param manifest           The manifest file.
     * @param toolchainArgument  The toolchain argument. Empty when none is given.
     * @return The name of the daemon.
     */
    static std::string getDaemonName (const juce::File& manifest, std::string_view toolchainArgument);

    /**
     * @brief Returns the cache folder of LING.
     *
     * The folder is in the local application data folder on Windows, and in the
     * home folder on the other platforms. The function creates no folder. Callers
     * create the folder before they write to it.
     *
     * @return The cache folder.
     */
    static juce::File getCacheDirectory();

    /**
     * @brief Returns the config folder of LING.
     *
     * The folder is in the home folder on every platform. The function creates no
     * folder. The language folders are in the language folder below it, one for each language name.
     *
     * @return The config folder.
     */
    static juce::File getConfigDirectory();

    static juce::File getCacheDirectory (const juce::File& manifest, std::string_view toolchainArgument);

    static juce::File getPatchedDirectory (std::string_view version, const jam::Strings& patchFiles);

    juce::File getSwapFile (const juce::File& file) const;

private:
    /**
     * @brief Adds the folder of the manifest to the watcher when the manifest path changes.
     *
     * The base class receives the change first.
     *
     * @param tree      The tree whose property changed. Only the state tree acts.
     * @param property  The changed property. Only Id::manifest acts.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Adds the folders of an added child to the watcher.
     *
     * The base class receives the child first. The folder of a file row is the
     * parent of its path, and the watcher receives it only when it exists. The
     * parent of each compiler manifest path and each dependency record path is
     * passed to the watcher without that test, and the watcher asserts that the
     * folder exists. A language row is the language folder. The processor creates
     * that folder before it appends the row.
     *
     * @param parentTree              The tree that received the child. Passed to the base class.
     * @param childWhichHasBeenAdded  The added child. A files child gives the folders of its rows, its compiler manifests, and its dependency records. A language child gives its language folder.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Sends a change for a changed file, to the manifest, to a language row, or to a file row.
     *
     * The manifest, a compiler manifest, and a dependency record send a change of
     * the manifest property, and the manifest reading then runs again. A file that
     * lies directly in the folder of a language row sends a change of the path
     * property to that row, and the processor starts a build of the language. Any other file
     * sends a change of the path property to its file row, when a row holds its
     * path, and the processor parses the file again.
     *
     * @param file   The changed file.
     * @param event  The kind of change. Not read.
     */
    void fileChanged (const juce::File& file, jam::File::Watcher::Event event) override;

    /** @brief The file watcher. It reports the changes of the files in the watched folders. */
    jam::File::Watcher watcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LINGModel)
};
