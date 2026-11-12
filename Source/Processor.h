#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"

/**
 * @class Processor
 * @brief Parses the source files of the model on a pool thread, and keeps the parsed documents.
 *
 * Each file row of the model starts one parse job. No ValueTree reaches the
 * pool thread. A job returns its result to the message thread, and the result
 * is kept only when the row still holds the same path and the file still has
 * the modification time that the job read. The documents are keyed by the index
 * of their path in paths.
 */
class Processor : public juce::ValueTree::Listener
{
public:
    /**
     * @brief Registers the processor as a listener of the model.
     *
     * @param newModel  The model whose changes start the parse jobs.
     */
    explicit Processor (LINGModel& newModel);

    /** @brief Removes the processor from the listeners of the model. */
    ~Processor() override;

    /**
     * @brief Calls @p function for each stored document, in ascending order of the path bytes.
     *
     * The documents are held in a HashMap, which has no order, so the path
     * indices are sorted first. The view that @p function receives points into
     * paths, and it is valid during the call.
     *
     * @tparam Function  A callable type that takes (std::string_view, const jam::CodeDocument&).
     * @param function   The callable. It receives the path and the document of each stored entry.
     */
    template <typename Function>
    void applyToDocuments (const Function& function) const
    {
        jam::Array<int> pathIndices;

        for (const auto& [pathIndex, document] : documents)
            pathIndices.add (pathIndex);

        std::sort (pathIndices.begin(), pathIndices.end(), [this] (int left, int right)
        {
            return paths.getView (left).compare (paths.getView (right)) < 0;
        });

        for (const auto pathIndex : pathIndices)
            function (paths.getView (pathIndex), *documents.at (pathIndex));
    }

private:
    /**
     * @brief Starts a parse job for each row of an added files child.
     *
     * When no job is pending after the rows start, the documents count is written
     * at once. Any other added child is ignored.
     *
     * @param parentTree              The tree that received the child. Not read.
     * @param childWhichHasBeenAdded  The added child. Its rows start the parse jobs when it is a files child.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Drops the stored documents of the rows of a removed files child.
     *
     * The entries of paths stay. Each document is keyed by the index of its path,
     * so removing an entry would move the indices of the other documents.
     *
     * @param parentTree                     The tree that lost the child. Not read.
     * @param childWhichHasBeenRemoved       The removed child. Its rows give the documents to drop when it is a files child.
     * @param indexFromWhichChildWasRemoved  The index of the removed child in its parent. Not read.
     */
    void valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;

    /**
     * @brief Starts a parse job for a file row whose path changes.
     *
     * The model sends a path change for a row when the watcher reports that its
     * file changed. Only the path of a file row starts a job.
     *
     * @param tree      The tree whose property changed. A file row starts a job.
     * @param property  The changed property. Only Id::path starts a job.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Starts a parse job for one file row, on the thread pool.
     *
     * The job gets the path and the family as values, read on the calling
     * thread. It reads the file, parses it, and validates it. Then it returns its
     * result to the message thread through juce::MessageManager::callAsync.
     * The pending count rises here, and addDocument lowers it.
     *
     * @param row       The file row. Its path and family are copied for the job.
     * @param rowIndex  The index of the row in the files child. addDocument checks the result against this row.
     */
    void addJob (const juce::ValueTree& row, int rowIndex);

    /**
     * @brief Ends a parse job on the message thread, and keeps its document when the row still matches.
     *
     * The result is kept only when the row at rowIndex still has the path, and the
     * file still has the modification time that the job read. A result that does
     * not match is dropped with no message. For a match, a document replaces the
     * stored document of the path, and no document erases it. A failed validation
     * sets the overlay message. When the last pending job ends, the count of
     * documents is written.
     *
     * @param rowIndex          The index of the row that started the job.
     * @param path              The path that the job read.
     * @param modificationTime  The modification time that the job read.
     * @param document          The parsed document, or nullptr when the file is missing or its family has no parser.
     * @param result            The validation result. An ok result when there is no document.
     */
    void addDocument (int rowIndex, const juce::String& path, juce::Time modificationTime, std::unique_ptr<jam::CodeDocument> document, const juce::Result& result);

    /**
     * @brief Returns the index of a path in paths, and adds the path when it is absent.
     *
     * The index is the key of the path's document. Nothing removes an entry from
     * paths, so an index stays the same for the life of the processor.
     *
     * @param path  The path text.
     * @return The index of the path in paths.
     */
    int getOrCreatePathIndex (const juce::String& path);

    /**
     * @brief Writes the number of stored documents to Id::documents.
     *
     * The callers call this only when no job is pending. The application reacts
     * to each change of Id::documents. When the count is unchanged, the change
     * message is sent by hand.
     */
    void setDocuments();

    /** @brief The model that this processor listens to. The application owns the model. */
    LINGModel& model;

    /** @brief The path texts. The index of a path in paths is the key of its document. */
    jam::Strings paths;

    /** @brief The parsed documents, keyed by the index of their path in paths. */
    jam::HashMap<int, std::unique_ptr<jam::CodeDocument>> documents;

    /** @brief The parse jobs that started and have not yet ended on the message thread. */
    int pendingJobs { 0 };

    /** @brief The thread pool that runs the parse jobs. */
    juce::ThreadPool pool;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};
