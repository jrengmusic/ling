#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Index.h"
#include "Query.h"
#include "Language.h"

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

    static juce::File getOrCreateLanguages();

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

    bool hasParseJob (const juce::ValueTree& unit, int unitIndex, int rowIndex) const;

    /**
     * @brief Returns the unit row that compiles a file.
     *
     * A file row with arguments is its own unit. Any other file row gives the first
     * unit row that depends on it.
     *
     * @param pathIndex  The index of the file path in paths. A negative index gives no unit.
     * @return The unit row, or an invalid tree when no unit row depends on the file.
     */
    juce::ValueTree getUnit (const juce::String& path) const;

    jam::Array<char> getArguments (const juce::ValueTree& unit) const;

    static void addArgument (jam::Array<char>& arguments, std::string_view argument);

    static jam::Array<const char*> getArgumentPointers (const jam::Array<char>& arguments);

    juce::int64 getUnitHash (const juce::ValueTree& unit, int unitIndex, const jam::Array<char>& arguments) const;

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

    void runHover (juce::ValueTree& request);

    bool isRequestReady (const juce::ValueTree& request) const;

    void moveRequestJobsToFront (const juce::ValueTree& request);

    void runRequests();

    void addDiagnosticRequests (const juce::ValueTree& file);

    void setIndexed (int rowIndex);

    void setUnitIndexed (const juce::ValueTree& unit, int unitIndex);

    void setIndexed();

    /**
     * @brief Starts the jobs of an added files child, a language child, or a request child.
     *
     * A files child first erases the documents whose path has no row in the child,
     * and it clears the queued index jobs. It then adds the language rows that its
     * families need. It starts one parse job for each row whose family has a loaded
     * library, and one precompile for each header entry. It then requests an update.
     * A language child starts its build. A request child runs when its file is indexed, and otherwise waits in nextRequests until handleAsyncUpdate finds the file indexed. Any other child
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
    void addLanguages();

    int addBuiltInLanguages();

    void addJobs (const juce::ValueTree& files);

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
        ParseJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::File& swapFileToUse, std::shared_ptr<const Language> languageToUse, juce::int64 languageHashToUse, int familyToUse, std::optional<juce::String> bufferToUse);

        /**
         * @brief Reads or parses the file, and writes its swap file. Runs on a pool thread.
         *
         * @return jobHasFinished, which the pool reads as the end of the job.
         */
        JobStatus runJob() override;

        Processor& owner;///< The processor that receives the completion signal. The job signals it when it ends.
        int rowIndex { 0 };///< The index of the file row in the files child. The message thread checks the row against it.
        juce::String path;///< The path of the file, copied from the row on the calling thread.
        juce::File swapFile;///< The swap file of the file. The job reads it, or writes it after a parse. A missing file deletes it.
        std::shared_ptr<const Language> language;///< The library of the family. The shared pointer keeps the library loaded while the job runs.
        juce::int64 languageHash { 0 };///< The hash of the language row. A swap file must hold this hash to give the document.
        int family { 0 };
        std::optional<juce::String> buffer;
        std::unique_ptr<jam::Document> document;///< The document that the job read or parsed. The message thread moves it into the index.
        std::unique_ptr<SymbolDocument> symbols;
        juce::Result result { juce::Result::ok() };///< The swap validation result, or the parse result. A parse without a document gives the library diagnosis.
        juce::Time modificationTime;///< The modification time of the file, read before the file is loaded.
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
         * @param symbolSwapFileToUse  The symbol swap file of the unit.
         * @param languageHashToUse    The hash of the language row.
         * @param languageToUse   The library of the family.
         */
        IndexJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::String& directoryToUse, jam::Array<char> argumentsToUse, juce::int64 hashToUse, const juce::File& symbolSwapFileToUse, juce::int64 languageHashToUse, std::shared_ptr<const Language> languageToUse, int familyToUse, juce::StringArray bufferPathsToUse, juce::StringArray bufferTextsToUse, jam::Strings foreignPathsToUse);

        /**
         * @brief Runs the library index function on the source. Runs on a pool thread.
         *
         * @return jobHasFinished, which the pool reads as the end of the job.
         */
        JobStatus runJob() override;

        void addSymbols();

        /**
         * @brief Stores one symbol record of the index pass. Every record of a position stays.
         *
         * @param context     The index job that receives the record.
         * @param recordPath  The path of the file of the location.
         * @param line        The 1-based line of the location.
         * @param column      The 1-based column of the location.
         * @param symbol      The symbol text of the entity.
         * @param role        The role bits of the record.
         * @param type        The token type of the entity.
         * @param name        The entity name. A null name is stored as empty.
         */
        static void addSymbol (void* context, const char* recordPath, int line, int column, unsigned long long symbol, int nameLength, int role, int type, int hover, const unsigned long long* typeSymbols, int typeCount);

        static int addHover (void* context, const char* recordPath, const char* text, const char* definitionPath, int line, int column, int endLine, int endColumn);

        static void addInlayHint (void* context, const char* recordPath, int line, int column, const char* label, int type, int paddingLeft, int paddingRight);

        static void addDiagnostic (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, int severity, const char* message);

        static int addOutline (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, int nameLength, int qualifierLength, int type, int parent);

        static void addCompletion (void* context, const char* recordPath, unsigned long long scopeSymbol, unsigned long long symbol, const char* label, const char* insertText, int type);

        static void addFixIt (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, const char* text);

        static constexpr int outlineFieldCount { 8 };

        int getOutlineParent (int parent, int pathIndex) const;

        bool isForeign (const char* recordPath) const;

        Processor& owner;///< The processor that receives the completion signal. The job signals it when it ends.
        int rowIndex { 0 };///< The index of the unit row in the files child.
        juce::String path;///< The path of the source file of the unit.
        juce::String directory;///< The working folder of the compile.
        jam::Array<char> arguments;///< The compile arguments, each ended by a null character.
        juce::int64 hash { 0 };
        juce::File symbolSwapFile;
        juce::int64 languageHash { 0 };
        std::shared_ptr<const Language> language;///< The library of the family. The shared pointer keeps the library loaded while the job runs.
        int family { 0 };
        juce::StringArray bufferPaths;
        juce::StringArray bufferTexts;
        jam::Strings foreignPaths;
        SymbolDocument::Builder builder;
        std::unique_ptr<SymbolDocument> symbols;
        jam::Array<std::pair<int, int>> outlineSlots;
        jam::HashMap<juce::int64, int> outlineKeys;
        jam::HashSet<juce::int64> completionKeys;
        int result { 0 };///< The return value of the library index function. Zero means the pass succeeded.
        std::atomic<bool> isFinished { false };///< Set to true when runJob ends. The message thread reads it with acquire order.
    };

    struct UnitsJob : public juce::ThreadPoolJob
    {
        UnitsJob (Processor& ownerToUse, jam::Array<char> argumentsToUse, jam::Array<int> argumentCountsToUse, const juce::String& directoryToUse, const juce::String& cacheDirectoryToUse, jam::HashMap<int, std::shared_ptr<const Language>> languagesToUse, jam::HashMap<juce::String, int> familiesToUse);

        JobStatus runJob() override;

        static void addUnit (void* context, const char* path, const char* directory, const char* output, const char* const* arguments, int argumentCount, const char* const* dependencies, int dependencyCount, const char* const* prerequisites, int prerequisiteCount);

        static void addBuildFile (void* context, const char* path);

        static void addMessage (void* context, const char* text);

        int getOrCreatePathIndex (const juce::String& path, int family);

        juce::Array<juce::var> getOrCreatePathIndices (const char* const* filePaths, int filePathCount, int family);

        Processor& owner;
        jam::Array<char> arguments;
        jam::Array<int> argumentCounts;
        juce::String directory;
        juce::String cacheDirectory;
        jam::HashMap<int, std::shared_ptr<const Language>> languages;
        jam::HashMap<juce::String, int> families;
        juce::ValueTree files { jam::Format::toType (Id::files) };
        jam::Strings paths;
        juce::Array<juce::var> buildFiles;
        std::atomic<bool> isFinished { false };
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

            if (pool->removeJob (runningJob.get(), true, noWait))
            {
                nextJobs.erase (rowIndex);
                runningJob = std::move (newJob);
                pool->addJob (runningJob.get(), false);
            }
            else
            {
                nextJobs.addOrReplace (rowIndex, std::move (newJob));
            }
        }
        else
        {
            jobs.addOrReplace (rowIndex, std::move (newJob));
            pool->addJob (jobs.at (rowIndex).get(), false);
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
        pool->removeJob (jobs.at (rowIndex).get(), true, waitForJob);

        if (const auto queued { nextJobs.find (rowIndex) }; queued != nextJobs.end())
        {
            auto& [queuedRow, queuedJob] { *queued };

            jobs.addOrReplace (rowIndex, std::move (queuedJob));
            nextJobs.erase (queued);
            pool->addJob (jobs.at (rowIndex).get(), false);
        }
        else
        {
            jobs.erase (rowIndex);
        }
    }

    template <typename Job>
    static jam::Array<int> getFinishedRows (const jam::HashMap<int, std::unique_ptr<Job>>& jobs)
    {
        jam::Array<int> rows;

        for (const auto& [rowIndex, job] : jobs)
            if (job->isFinished.load (std::memory_order_acquire))
                rows.add (rowIndex);

        std::sort (rows.begin(), rows.end());
        return rows;
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

    void setBuffer (juce::ValueTree& request);

    void removeBuffer (juce::ValueTree& request);

    void setBufferChange (juce::ValueTree& request);

    /**
     * @brief Starts a parse job for each file row of the family of a language row whose hash changed.
     *
     * setLanguage loads the library before it sets the hash, so addJob finds the library.
     *
     * @param languageRow  The language row whose hash changed.
     */
    void setLanguageHash (const juce::ValueTree& languageRow);

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
    void setLanguage (juce::ValueTree languageRow);

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

    void addUnitsJob();

    std::unique_ptr<UnitsJob> getUnitsJob (const juce::ValueTree& toolchain);

    void setUnitsJobs();

    void setFiles (const juce::ValueTree& files);

    /** @brief The model that this processor listens to. The application owns the model. */
    LINGModel& model;

    /** @brief The documents, the bindings, and the libraries of the families. The processor writes it, and Query reads it through a const reference. */
    Index index;

    /** @brief Answers the request rows from the index. Its only state is the fuzzy finder slab. */
    Query query;

    jam::HashMap<int, std::unique_ptr<ParseJob>> parseJobs;

    jam::HashMap<int, std::unique_ptr<ParseJob>> nextParseJobs;

    jam::HashMap<int, std::unique_ptr<IndexJob>> indexJobs;

    jam::HashMap<int, std::unique_ptr<IndexJob>> nextIndexJobs;

    jam::HashMap<int, std::unique_ptr<UnitsJob>> unitsJobs;

    jam::HashMap<int, std::unique_ptr<UnitsJob>> nextUnitsJobs;

    /**
     * @brief The thread pool that runs the parse jobs and the index jobs.
     *
     * The pool is declared after the job maps. Its destructor signals each job to
     * exit and waits up to 5000 milliseconds for the running jobs, so the jobs stop
     * before the maps free them.
     */
    std::unique_ptr<juce::ThreadPool> pool;

    /** @brief The operations that answer the request rows, keyed by the command identifier. */
    Events events;

    Events propertyEvents;

    Events childEvents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};
