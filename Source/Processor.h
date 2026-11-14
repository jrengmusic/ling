#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "CompilerManifest.h"
#include "Index.h"
#include "Query.h"

/**
 * @class Processor
 * @brief Runs the parse and index jobs of the source files on a thread pool, and applies their results to the index.
 *
 * Each language row starts a build of its language on a subprocess. The library
 * that the build writes is loaded for the family of the row. Each file row whose
 * family has a loaded library gets one ParseJob. When a parse gives a document,
 * each unit row that depends on the file gets an IndexJob. A job is one-shot: the
 * pool runs it once, and the message thread removes it. A job works on values that
 * its constructor receives, and it does not read the model. A job signals the
 * message thread, which applies its result in handleAsyncUpdate. A parse result
 * reaches the index only when the row still holds the same path, the file still
 * has the modification time that the job read, and the family still has the library
 * that the job used. An index job starts only when no parse job remains. The
 * documents are keyed by the index of their path in paths.
 *
 * The processor also answers the request rows of the model. Each request row runs
 * the entry of events that its command names, in the listener that receives the row.
 * The entry writes JSON to the result of the row. The lookups read the index in
 * place, and they keep no state except the fuzzy finder slab of Query.
 */
class Processor : public juce::ValueTree::Listener,
                  private juce::AsyncUpdater
{
public:
    /**
     * @brief Maps each request command to the operation that answers it.
     *
     * Each entry takes the request row, reads its arguments, and writes its
     * result. The key is the command word of the request.
     */
    using Events = jam::Function::Map<juce::Identifier, jam::Function::pointerCapacity, void (juce::ValueTree&)>;

    /**
     * @brief Registers the processor as a listener of the model.
     *
     * @param newModel  The model whose changes start the parse jobs.
     */
    explicit Processor (LINGModel& newModel);

    /** @brief Removes the processor from the listeners of the model. */
    ~Processor() override;

    template <typename Function>
    void applyToDocuments (const Function& function) const
    {
        index.applyToDocuments (function);
    }

private:
    /**
     * @brief Tells whether a unit row depends on a file row, which is the row itself or one of its dependencies.
     *
     * Only a unit row holds arguments, so a row without arguments depends on nothing.
     *
     * @param unit       The row to test.
     * @param unitIndex  The index of the row to test in the files child.
     * @param rowIndex   The index of the file row in the files child.
     * @return @c true when the row holds arguments, and its index or one of its dependencies is rowIndex.
     */
    static bool hasDependency (const juce::ValueTree& unit, int unitIndex, int rowIndex);

    /**
     * @brief Returns the unit row that compiles a file.
     *
     * A file row with arguments is its own unit. Any other file row gives the first
     * unit row that depends on it.
     *
     * @param pathIndex  The index of the file path in paths. A negative index gives no unit.
     * @return The unit row, or an invalid tree when no unit row depends on the file.
     */
    juce::ValueTree getUnit (int pathIndex) const;

    juce::ValueTree getHeaderEntry (const juce::ValueTree& unit) const;

    jam::Array<juce::ValueTree> getHeaderEntries (const juce::ValueTree& files) const;

    jam::Strings getArguments (const juce::ValueTree& unit, const juce::ValueTree& headerEntry) const;

    jam::Strings getCheckArguments (const juce::ValueTree& unit) const;

    juce::File getPrecompiledHeader (const juce::ValueTree& headerEntry) const;

    /**
     * @brief Fills events with the operation of each request command.
     *
     * The constructor calls this once. The table holds one entry for each command,
     * so runRequest finds an operation with one lookup.
     */
    void registerEvents();

    /**
     * @brief Runs the operation that the command of a request names.
     *
     * An empty command word maps to the empty identifier, which the table does
     * not hold. A juce::Identifier made from an empty string fails an assertion
     * (juce_Identifier.cpp), so the empty word is tested before the identifier is
     * made. An unknown command sets the overlay message and writes an empty result.
     *
     * @param request  The request row. Its command selects the operation, and the operation writes the result.
     */
    void runRequest (juce::ValueTree& request);

    void diagnostics (juce::ValueTree& request);

    /**
     * @brief Starts the jobs of an added files child, a language child, or a request child.
     *
     * A files child first erases the documents whose path has no row in the child,
     * and it clears the queued index jobs. It then adds the language rows that its
     * families need. It starts one parse job for each row whose family has a loaded
     * library, and one precompile for each header entry. It then requests an update.
     * A language child starts its build. A request child runs at once. Any other child
     * is ignored.
     *
     * @param parentTree              The tree that received the child. Not read.
     * @param childWhichHasBeenAdded  The added child. A files child starts the parse jobs, a language child starts its build, and a request child runs.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Adds a language row for each file row whose family has no language row.
     *
     * The config folder of a new language is created before the row is appended to
     * the state. The append starts the build of the language.
     *
     * @param files  The files child whose rows name the families.
     */
    void addLanguages (const juce::ValueTree& files);

    /**
     * @brief Starts the jobs that a changed path or hash of a file or a language row requires.
     *
     * A path change of a file row starts a parse job, when its family has a loaded
     * library. It also deletes the precompiled header of each header entry that
     * depends on the row, and starts that precompile again. A path change of a
     * language row starts its build. A hash change of a language row starts a parse
     * job for each file row of its family. The model sends a path change for a row
     * when the watcher reports a change of its file.
     *
     * @param tree      The tree whose property changed. A file row or a language row starts a job.
     * @param property  The changed property. Only Id::path and Id::hash start a job.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Starts a parse job for one file row, through setJob.
     *
     * The path, the family, the swap file, the library of the family, and the hash
     * of the language row are read on the calling thread, and the job gets them as
     * values. The swap file folder is created before the job starts. The job reads
     * the file. A swap file that holds the same source hash and the same language
     * hash gives the document, and SwapValidator checks it. Otherwise the job parses
     * the text with the library, and it writes the swap file. A missing file deletes
     * the swap file. The job signals the message thread, which applies the result in
     * handleAsyncUpdate.
     *
     * @param row       The file row. Its path and family are read on the calling thread.
     * @param rowIndex  The index of the row in the files child. addDocument checks the result against this row.
     */
    void addJob (const juce::ValueTree& row, int rowIndex);

    /**
     * @brief A one-shot parse of one file row, run on the pool. It holds its inputs as values.
     *
     * The job reads the file on the pool thread. It reads the swap file when the source
     * hash and the language hash match, and it parses the text otherwise. The message
     * thread applies the result in handleAsyncUpdate.
     */
    struct ParseJob : public juce::ThreadPoolJob
    {
        /**
         * @brief Creates a parse job for one file row.
         *
         * @param ownerToUse         The processor that receives the completion signal.
         * @param rowIndexToUse      The index of the file row in the files child.
         * @param pathToUse          The path of the file.
         * @param swapFileToUse      The swap file of the file.
         * @param languageToUse      The library of the family.
         * @param languageHashToUse  The hash of the language row.
         */
        ParseJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::File& swapFileToUse, std::shared_ptr<const jam::LanguageLibrary> languageToUse, juce::int64 languageHashToUse);

        /**
         * @brief Reads or parses the file, and writes its swap file. Runs on a pool thread.
         *
         * @return jobHasFinished, which the pool reads as the end of the job.
         */
        JobStatus runJob() override;

        Processor& owner;///< The processor that receives the completion signal. The job signals it when it ends.
        int rowIndex;///< The index of the file row in the files child. The message thread checks the row against it.
        juce::String path;///< The path of the file, copied from the row on the calling thread.
        juce::File swapFile;///< The swap file of the file. The job reads it, or writes it after a parse. A missing file deletes it.
        std::shared_ptr<const jam::LanguageLibrary> language;///< The library of the family. The shared pointer keeps the library loaded while the job runs.
        juce::int64 languageHash;///< The hash of the language row. A swap file must hold this hash to give the document.
        std::unique_ptr<jam::LanguageDocument> document;///< The document that the job read or parsed. The message thread moves it into the index.
        juce::Result result { juce::Result::ok() };///< The swap validation result, or the parse result. A parse without a document gives the library diagnosis.
        juce::Time modificationTime;///< The modification time of the file, read before the file is loaded.
        bool isParsed { false };///< True when the job parsed the text. False when the document came from the swap file, or none was read.
        std::atomic<bool> isFinished { false };///< Set to true when runJob ends. The message thread reads it with acquire order.
    };

    /**
     * @brief A one-shot index pass over one unit row, run on the pool. It holds its inputs as values.
     *
     * The job calls the library index function, and the library reports each symbol
     * to addSymbol. The message thread applies the records in handleAsyncUpdate.
     */
    struct IndexJob : public juce::ThreadPoolJob
    {
        /**
         * @brief Creates an index job for one unit row.
         *
         * @param ownerToUse      The processor that receives the completion signal.
         * @param rowIndexToUse   The index of the unit row in the files child.
         * @param pathToUse       The path of the source file of the unit.
         * @param directoryToUse  The working folder of the compile.
         * @param argumentsToUse  The compile arguments, each ended by a null character.
         * @param languageToUse   The library of the family.
         */
        IndexJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::String& directoryToUse, jam::Array<char> argumentsToUse, std::shared_ptr<const jam::LanguageLibrary> languageToUse);

        /**
         * @brief Runs the library index function on the source. Runs on a pool thread.
         *
         * @return jobHasFinished, which the pool reads as the end of the job.
         */
        JobStatus runJob() override;

        /**
         * @brief Stores one symbol record of the index pass. The first record at a position stays.
         *
         * @param context     The index job that receives the record.
         * @param recordPath  The path of the file of the location.
         * @param line        The 1-based line of the location.
         * @param column      The 1-based column of the location.
         * @param symbol      The symbol text of the entity.
         * @param role        The role bits of the record.
         */
        static void addSymbol (void* context, const char* recordPath, int line, int column, const char* symbol, int role);

        Processor& owner;///< The processor that receives the completion signal. The job signals it when it ends.
        int rowIndex;///< The index of the unit row in the files child.
        juce::String path;///< The path of the source file of the unit.
        juce::String directory;///< The working folder of the compile.
        jam::Array<char> arguments;///< The compile arguments, each ended by a null character.
        std::shared_ptr<const jam::LanguageLibrary> language;///< The library of the family. The shared pointer keeps the library loaded while the job runs.
        Index::Bindings records;///< The symbol records of the pass, keyed by the raw path, the line, and the column.
        int result { 0 };///< The return value of the library index function. Zero means the pass succeeded.
        std::atomic<bool> isFinished { false };///< Set to true when runJob ends. The message thread reads it with acquire order.
    };

    /**
     * @brief The removeJob timeout that waits until the job leaves the pool.
     *
     * A negative timeout never expires, so the call returns after the job ends.
     */
    static constexpr int waitForJob { -1 };

    /**
     * @brief The removeJob timeout that does not wait.
     *
     * A running job is signalled when interruptIfRunning is set, and it stays in the pool.
     */
    static constexpr int noWait { 0 };

    /**
     * @brief Starts a job for a row, or queues it behind the job of the row that runs.
     *
     * A row without a job gets the new job, and the pool runs it. The pool drops a
     * job that has not started, so the new job replaces it. A queued job of the row
     * is dropped. A running job is signalled and kept, and the new job waits in
     * nextJobs until the running job ends.
     *
     * @tparam Job      The job type, ParseJob or IndexJob.
     * @param jobs      The running jobs, keyed by row index.
     * @param nextJobs  The queued jobs, keyed by row index.
     * @param rowIndex  The index of the row of the job.
     * @param newJob    The job to start or to queue. The function takes ownership.
     */
    template <typename Job>
    void setJob (jam::HashMap<int, std::unique_ptr<Job>>& jobs, jam::HashMap<int, std::unique_ptr<Job>>& nextJobs, int rowIndex, std::unique_ptr<Job> newJob)
    {
        if (const auto running { jobs.find (rowIndex) }; running != jobs.end())
        {
            auto& [runningRow, runningJob] { *running };

            if (pool.removeJob (runningJob.get(), true, noWait))
            {
                nextJobs.erase (rowIndex);
                runningJob = std::move (newJob);
                pool.addJob (runningJob.get(), false);
            }
            else
            {
                nextJobs.addOrReplace (rowIndex, std::move (newJob));
            }
        }
        else
        {
            jobs.addOrReplace (rowIndex, std::move (newJob));
            pool.addJob (jobs.at (rowIndex).get(), false);
        }
    }

    /**
     * @brief Removes the finished job of a row, and starts the queued job of the row when one waits.
     *
     * The call waits until the pool releases the finished job. A row without a
     * queued job loses its entry.
     *
     * @tparam Job      The job type, ParseJob or IndexJob.
     * @param jobs      The running jobs, keyed by row index.
     * @param nextJobs  The queued jobs, keyed by row index.
     * @param rowIndex  The index of the row whose job finished.
     */
    template <typename Job>
    void removeJob (jam::HashMap<int, std::unique_ptr<Job>>& jobs, jam::HashMap<int, std::unique_ptr<Job>>& nextJobs, int rowIndex)
    {
        pool.removeJob (jobs.at (rowIndex).get(), true, waitForJob);

        if (const auto queued { nextJobs.find (rowIndex) }; queued != nextJobs.end())
        {
            auto& [queuedRow, queuedJob] { *queued };

            jobs.addOrReplace (rowIndex, std::move (queuedJob));
            nextJobs.erase (queued);
            pool.addJob (jobs.at (rowIndex).get(), false);
        }
        else
        {
            jobs.erase (rowIndex);
        }
    }

    /**
     * @brief Queues an index job for a unit row, and signals the running index job of the row.
     *
     * The arguments of the row are joined, each ended by a null character. A running
     * index job is removed when it has not started. Otherwise it is signalled and kept
     * until it ends. The queued job replaces any earlier queued job of the row.
     *
     * @param row       The unit row. Its arguments, directory, path, and family give the job.
     * @param rowIndex  The index of the unit row in the files child.
     */
    void addIndexJob (const juce::ValueTree& row, int rowIndex);

    /**
     * @brief Starts each queued index job whose row has no running index job.
     *
     * A started job moves from the queue to the running map, and the pool runs it.
     */
    void addIndexJobs();

    /**
     * @brief Applies the symbol records of a finished index job to the index, or reports its failure.
     *
     * The records move into the index when the row still holds the path of the job
     * and the library returns zero. Any other result sets the message, and deletes the
     * swap files of the path and of each dependency row. A job whose row no longer
     * holds its path drops its records.
     *
     * @param job  The finished index job.
     */
    void addBindings (IndexJob& job);

    /**
     * @brief Applies the finished jobs on the message thread, starts the queued index jobs, and writes the symbols when no work remains.
     *
     * A finished parse job is applied only when no newer job waits for its row. A
     * finished index job is applied likewise. The index jobs start only when no parse
     * job runs. When no parse job, index job, build, or precompile remains, setDocuments runs.
     */
    void handleAsyncUpdate() override;

    /**
     * @brief Starts the parse job of a changed file row, and the precompile of each header entry that depends on it.
     *
     * A header entry that depends on the row loses its precompiled header file before
     * the precompile starts.
     *
     * @param file  The file row whose path changed.
     */
    void setFile (const juce::ValueTree& file);

    /**
     * @brief Starts a parse job for each file row of the family of a language row whose hash changed.
     *
     * setLanguage loads the library before it sets the hash, so addJob finds the library.
     *
     * @param languageRow  The language row whose hash changed.
     */
    void setLanguageHash (const juce::ValueTree& languageRow);

    /**
     * @brief Starts the build of a language row on a subprocess, unless a build of its family runs.
     *
     * The config folder of the language is copied to its cache folder. The
     * subprocess then runs cast with the spell file of the language in that folder.
     * A failed copy ends the build at once, and applyBuild reports it.
     *
     * @param languageRow  The language row. Its family selects the build, and its name and path give the folders.
     */
    void addBuild (const juce::ValueTree& languageRow);

    /**
     * @brief Ends a build on the message thread, and loads its library on success.
     *
     * The build of the family is removed first, so a new build can start. An exit
     * code of zero loads the library. Any other exit code sets the overlay message
     * with the captured output. When a file directly in the config folder changed
     * after the start, the build runs again. It then requests an update, which writes the documents count once no work remains.
     *
     * @param languageRow  The language row whose build ended.
     * @param cache        The cache folder of the build.
     * @param startTime    The time at which the build started.
     * @param exitCode     The exit code of the subprocess.
     * @param capture      The captured output of the subprocess.
     */
    void applyBuild (const juce::ValueTree& languageRow, const juce::File& cache, juce::Time startTime, int exitCode, const std::string& capture);

    /**
     * @brief Tells whether a file directly in a folder was modified after a time.
     *
     * Files in subfolders are not searched.
     *
     * @param folder  The folder whose files are tested.
     * @param time    The time that a modification must follow.
     * @return @c true when at least one file of the folder was modified after the time.
     */
    static bool isChangedSince (const juce::File& folder, juce::Time time);

    /**
     * @brief Loads the library that a build wrote, and makes it the library of its family.
     *
     * The library is copied to a file that is named by the hash of its bytes, when
     * that file is absent, and the copy is opened. The system loader returns the
     * image that it already holds for a path, so a build with new bytes needs a new
     * path. The loaded library replaces the entry of the family, and the row
     * receives the hash. A library that fails to load sets the overlay message only.
     *
     * @param languageRow  The language row. Its family selects the entry, and its hash receives the hash of the library.
     * @param cache        The cache folder of the build. It holds the build file.
     */
    void setLanguage (juce::ValueTree languageRow, const juce::File& cache);

    void addPrecompile (const juce::ValueTree& headerEntry);

    void addPrecompile (const juce::ValueTree& headerEntry, std::function<void()> onComplete);

    void addCheck (juce::ValueTree& request, const juce::ValueTree& unit);

    /**
     * @brief Applies a finished parse job to the index, when the job still matches its row.
     *
     * The job matches when the row holds the path of the job, the file keeps the
     * modification time of the job, and the family keeps the library of the job.
     * A matching job stores its document in the index, and it reports a failed result
     * to the message. When the job parsed a document, each unit row that depends on the
     * file, and whose family has a library, gets an index job. A unit row that already
     * has a queued index job keeps that job. A job that does not match keeps its document
     * until the message thread removes the job.
     *
     * @param job  The finished parse job.
     */
    void addDocument (ParseJob& job);

    /** @brief The model that this processor listens to. The application owns the model. */
    LINGModel& model;

    jam::Owner<jam::Subprocess> checks;

    jam::Owner<jam::Subprocess> precompiles;

    /** @brief The language builds that run, keyed by family. A family has at most one build, and applyBuild removes it when the build ends. */
    jam::HashMap<int, std::unique_ptr<jam::Subprocess>> builds;

    /** @brief The documents, the bindings, and the libraries of the families. The processor writes it, and Query reads it through a const reference. */
    Index index;

    /** @brief Answers the request rows from the index. Its only state is the fuzzy finder slab. */
    Query query;

    jam::HashMap<int, std::unique_ptr<ParseJob>> parseJobs;

    jam::HashMap<int, std::unique_ptr<ParseJob>> nextParseJobs;

    jam::HashMap<int, std::unique_ptr<IndexJob>> indexJobs;

    jam::HashMap<int, std::unique_ptr<IndexJob>> nextIndexJobs;

    /**
     * @brief The thread pool that runs the parse jobs and the index jobs.
     *
     * The pool is declared after the job maps. Its destructor signals each job to
     * exit and waits up to 5000 milliseconds for the running jobs, so the jobs stop
     * before the maps free them.
     */
    juce::ThreadPool pool;

    /** @brief The operations that answer the request rows, keyed by the command identifier. */
    Events events;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};
