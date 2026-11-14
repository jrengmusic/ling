#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"

/**
 * @class Manifest
 * @brief Reads the manifest on a pool thread, and sets the files child and the timeout of the model.
 *
 * A change of the manifest path starts one reading. The reading parses and
 * validates the manifest, then builds the files child from the toolchain rows that
 * match the toolchain argument. A failed validation sets the message, and the files
 * child is empty.
 */
class Manifest : private juce::ValueTree::Listener,
                 private juce::AsyncUpdater
{
public:
    /**
     * @brief Listens to the model, so that a change of the manifest path starts a reading.
     *
     * @param newModel  The model whose manifest path starts the readings.
     */
    explicit Manifest (LINGModel& newModel);

    /** @brief Removes the manifest from the listeners of the model. */
    ~Manifest() override;

private:
    /**
     * @brief Starts a reading when the manifest path of the model changes.
     *
     * Only a change of Id::manifest on the state tree of the model starts a reading.
     *
     * @param tree      The tree whose property changed.
     * @param property  The changed property. Only Id::manifest starts a reading.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Starts one reading of the manifest, through the pool.
     *
     * The manifest path and the toolchain argument are copied from the model on
     * the calling thread. A running reading is removed when it has not started. Otherwise
     * it is signalled and kept until it ends, and a newer reading waits in nextJob. The
     * dependency command of a toolchain row blocks until its process ends, so the
     * reading does not run on the message thread.
     */
    void addJob();

    /**
     * @brief One reading of the manifest, run on the pool. It holds the results until the message thread applies them.
     *
     * The job parses and validates the manifest. A valid manifest gives the files child,
     * the roots child, and the timeout of the daemon table. A failed validation gives an
     * empty files child that holds the message, the project roots, and the default timeout.
     */
    struct ManifestJob : public juce::ThreadPoolJob
    {
        /**
         * @brief Creates a reading of one manifest.
         *
         * @param ownerToUse              The manifest that owns the reading.
         * @param manifestPathToUse       The path of the manifest.
         * @param toolchainArgumentToUse  The toolchain argument.
         */
        ManifestJob (Manifest& ownerToUse, const juce::String& manifestPathToUse, const juce::String& toolchainArgumentToUse);

        /**
         * @brief Parses, validates, and reads the manifest. Runs on a pool thread.
         *
         * @return jobHasFinished, which the pool reads as the end of the job.
         */
        JobStatus runJob() override;

        Manifest& owner;///< The manifest that owns the reading. The job signals it when it ends.
        juce::String manifestPath;///< The path of the manifest, copied from the model on the calling thread.
        juce::String toolchainArgument;///< The toolchain argument, copied from the model on the calling thread.
        juce::ValueTree files;///< The files child that the job builds. It holds the message property when validation fails.
        juce::ValueTree roots;///< The roots child that the job builds.
        int timeout { 0 };///< The timeout in milliseconds, from the daemon table, or the default timeout.
        std::atomic<bool> isFinished { false };///< Set to true when runJob ends. The message thread reads it with acquire order.
    };

    /**
     * @brief Applies a finished reading on the message thread, and starts the queued reading.
     *
     * The results are applied only when no newer reading waits. The finished reading
     * leaves the pool before the queued reading starts.
     */
    void handleAsyncUpdate() override;

    /**
     * @brief Sets the timeout of the model, and replaces its files child.
     *
     * @param files    The files child that replaces the current one.
     * @param roots    The roots child that replaces the current one.
     * @param timeout  The timeout, in milliseconds, that Id::timeout receives.
     */
    void setFiles (const juce::ValueTree& files, const juce::ValueTree& roots, int timeout);

    /**
     * @brief Returns the timeout of the first daemon table row that holds one, or the default timeout when none does.
     *
     * @param manifest  The parsed manifest.
     * @return The timeout, in milliseconds.
     */
    static int getTimeout (const jam::MarkdownDocument& manifest);

    static juce::ValueTree getRoots (const jam::MarkdownDocument& manifest, const juce::File& manifestFile);

    /** @brief The number of threads of the pool. One thread runs the readings one after another. */
    static constexpr int threadCount { 1 };

    /** @brief The model that the readings read and write. The application owns it. */
    LINGModel& model;

    /**
     * @brief The removeJob timeout that waits until the reading leaves the pool.
     *
     * A negative timeout never expires, so the call returns after the reading ends.
     */
    static constexpr int waitForJob { -1 };

    /**
     * @brief The removeJob timeout that does not wait.
     *
     * A running reading is signalled when interruptIfRunning is set, and it stays in the pool.
     */
    static constexpr int noWait { 0 };

    /** @brief The reading that is in the pool. handleAsyncUpdate applies its results when it finishes. */
    std::unique_ptr<ManifestJob> job;

    /** @brief The newest reading, queued behind job. A newer reading replaces it. */
    std::unique_ptr<ManifestJob> nextJob;

    /** @brief The thread pool that runs the readings, with threadCount threads. */
    juce::ThreadPool pool { juce::ThreadPool::Options{}.withNumberOfThreads (threadCount) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Manifest)
};
