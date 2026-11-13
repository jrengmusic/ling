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
 * match the toolchain argument. A
 * failed validation sets the message, and the files child is empty.
 */
class Manifest : private juce::ValueTree::Listener
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
     * @brief Starts one reading of the manifest on the pool thread.
     *
     * The manifest path and the toolchain argument are copied from the model on
     * the calling thread. The job parses and validates the manifest. It takes the
     * timeout from the daemon table when the manifest is valid, and the default
     * timeout otherwise. A failed validation sets the message. The job then sets
     * the files child on the message thread. The dependency command of a toolchain row blocks until
     * its process ends, so the job does not run on the message thread.
     */
    void addJob();

    /**
     * @brief Sets the timeout of the model, and replaces its files child.
     *
     * @param files    The files child that replaces the current one.
     * @param timeout  The timeout, in milliseconds, that Id::timeout receives.
     */
    void setFiles (const juce::ValueTree& files, int timeout);

    /**
     * @brief Returns the timeout of the first daemon table row that holds one, or the default timeout when none does.
     *
     * @param manifest  The parsed manifest.
     * @return The timeout, in milliseconds.
     */
    static int getTimeout (const jam::MarkdownDocument& manifest);

    /** @brief The number of threads of the pool. One thread runs the readings one after another. */
    static constexpr int threadCount { 1 };

    /** @brief The model that the readings read and write. The application owns it. */
    LINGModel& model;

    /** @brief The thread pool that runs the readings, with threadCount threads. */
    juce::ThreadPool pool { juce::ThreadPool::Options{}.withNumberOfThreads (threadCount) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Manifest)
};
