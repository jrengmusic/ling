#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"

/**
 * @class Manifest
 * @brief Reads the manifest on a pool thread, and sets the toolchain, roots, timeout, and threads of the model.
 *
 * A change of the manifest path starts one reading. The reading parses and
 * validates the manifest, then builds the toolchain child from the toolchain rows
 * matching the toolchain argument and the pattern rows. A failed validation sets
 * the message, and the toolchain child is empty.
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
     * it is signalled and kept until it ends, and a newer reading waits in nextJob.
     * File I/O, parsing, and validation run on the pool so the message thread stays free.
     */
    void addJob();

    /**
     * @brief One reading of the manifest, run on the pool. It holds the results until the message thread applies them.
     *
     * The job parses and validates the manifest. A valid manifest gives the toolchain
     * child, the roots child, and the timeout and threads values from the daemon table.
     * A failed validation gives an empty toolchain child carrying the message property,
     * the project roots, and the default timeout and thread count.
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
        juce::ValueTree toolchain;///< The toolchain child that the job builds. It holds the message property when validation fails.
        juce::ValueTree roots;///< The roots child that the job builds.
        int timeout { 0 };///< The timeout in milliseconds, from the daemon table, or the default timeout.
        int threads { LINGModel::getDefaultThreads() };///< The worker thread count from the daemon table, or LINGModel::getDefaultThreads().
        /** @brief The watcher coalescing window in milliseconds from the daemon table, or LINGModel::defaultCoalesceMs. */
        int coalesce { LINGModel::defaultCoalesceMs };
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
     * @brief Sets the timeout, thread count, and watcher coalesce window of the model, and replaces its roots and toolchain children.
     *
     * @param toolchain  The toolchain child that replaces the current one.
     * @param roots      The roots child that replaces the current one.
     * @param timeout    The timeout, in milliseconds, that Id::timeout receives.
     * @param threads    The worker thread count that Id::threads receives.
     * @param coalesce   The watcher event coalescing window, in milliseconds, that Id::coalesce receives.
     */
    void setToolchain (const juce::ValueTree& toolchain, const juce::ValueTree& roots, int timeout, int threads, int coalesce);

    /**
     * @brief Returns an integer setting from the daemon table of @p manifest, or @p defaultValue when absent.
     *
     * @param manifest      Parsed manifest markdown document.
     * @param rowId         Row identifier in the daemon table (such as Id::timeout or Id::threads).
     * @param defaultValue  Fallback integer returned when no daemon table contains @p rowId.
     * @return The configured integer value, or @p defaultValue.
     */
    static int getDaemonValue (const jam::MarkdownDocument& manifest, const juce::Identifier& rowId, int defaultValue);

    /**
     * @brief Builds the roots child holding the project root and any user-module and patched-JUCE roots.
     *
     * @param manifest      Parsed manifest markdown document.
     * @param manifestFile  Manifest file used to resolve relative paths.
     * @return An Id::roots ValueTree containing one Id::root child per source tree and swap directory.
     */
    static juce::ValueTree getRoots (const jam::MarkdownDocument& manifest, const juce::File& manifestFile);

    /**
     * @brief Builds the toolchain child for @p toolchainArgument and appends pattern rows from @p manifest.
     *
     * Sets Id::message on the returned tree when a non-empty @p toolchainArgument matches no row.
     *
     * @param manifest           Parsed manifest markdown document.
     * @param toolchainArgument  Toolchain flag selecting matching toolchain rows, or empty.
     * @param manifestFile       Manifest file used to resolve pattern-row directories.
     * @return An Id::toolchain ValueTree containing matching command rows and pattern rows.
     */
    static juce::ValueTree getToolchain (const jam::MarkdownDocument& manifest, const juce::String& toolchainArgument, const juce::File& manifestFile);

    /**
     * @brief Appends sourceGlob, layoutGlob, and binary pattern rows from @p manifest onto @p toolchain.
     *
     * @param toolchain     Target Id::toolchain tree that receives the pattern rows.
     * @param manifest      Parsed manifest markdown document.
     * @param manifestFile  Manifest file used to resolve relative directory paths.
     */
    static void addPatternRows (juce::ValueTree& toolchain, const jam::MarkdownDocument& manifest, const juce::File& manifestFile);

    /** @brief The number of threads of the pool. One thread runs the readings one after another. */
    static constexpr int threadCount { 1 };

    /** @brief The model that the readings read and write. The application owns it. */
    LINGModel& model;

    /** @brief The reading that is in the pool. handleAsyncUpdate applies its results when it finishes. */
    std::unique_ptr<ManifestJob> job;

    /** @brief The newest reading, queued behind job. A newer reading replaces it. */
    std::unique_ptr<ManifestJob> nextJob;

    /** @brief The thread pool that runs the readings, with threadCount threads. */
    juce::ThreadPool pool { juce::ThreadPool::Options{}.withNumberOfThreads (threadCount) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Manifest)
};
