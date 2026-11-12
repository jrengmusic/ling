#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"

/**
 * @class LINGModel
 * @brief The state of one LING run: the files, the message, and the watcher of the file folders.
 *
 * The model reads the manifest path from its state, and it replaces the files
 * child with the rows of the manifest. Each folder of a file row is watched. A
 * change of a watched file sends a path change for its row.
 */
class LINGModel
    : public jam::Model
    , public jam::Instance<LINGModel>
    , private jam::File::Watcher::Listener
{
public:
    /**
     * @brief Creates the files child and the overlay row, then starts the watcher.
     *
     * The overlay row holds the message parameter. The application prints each
     * change of that parameter.
     */
    LINGModel();

    /** @brief Releases the model. The members release their own resources. */
    ~LINGModel() override = default;

    /**
     * @brief Writes text to the message parameter of the overlay row.
     *
     * The manifest reading, the compiler manifest reading, and the parse jobs all
     * report their failures through this function.
     *
     * @param text  The message text.
     */
    void setMessage (const juce::String& text);

private:
    /**
     * @brief Rebuilds the files child from the manifest when the manifest path changes.
     *
     * The toolchain argument is read from the state at the same time. A manifest
     * that fails validation sets the message, and its files child is empty.
     *
     * @param tree      The tree whose property changed.
     * @param property  The changed property. Only Id::manifest rebuilds the files child.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Adds the folder of each row of an added files child to the watcher.
     *
     * Only a folder that exists is watched.
     *
     * @param parentTree              The tree that received the child. Passed to the base class.
     * @param childWhichHasBeenAdded  The added child. Its rows give the folders when it is a files child.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Sends a path change for the row of a changed file.
     *
     * The processor parses the file again on that change.
     *
     * @param file   The changed file.
     * @param event  The kind of change. Not read.
     */
    void fileChanged (const juce::File& file, jam::File::Watcher::Event event) override;

    /** @brief The file watcher. It reports the changes of the files in the watched folders. */
    jam::File::Watcher watcher;

    /** @brief The window, in milliseconds, in which the watcher merges the events of one file into one event. */
    static constexpr int coalesceMs { 300 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LINGModel)
};
