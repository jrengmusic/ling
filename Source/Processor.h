#pragma once
#include <JuceHeader.h>
#include "generated/Generated.h"
#include "LINGModel.h"
#include "Index.h"
#include "Query.h"
#include "Language.h"

/**
 * @class Processor
 * @brief Runs unit-discovery, parse, and index jobs on a thread pool, and applies results to the index.
 *
 * Registers built-in languages and loads dynamic language libraries into index.
 * UnitsJob discovers compilation units and emits the files tree. Each file row
 * runs a ParseJob that updates syntax hashes and may emit a SymbolDocument.
 *
 * Semantic indexing is demand-driven: requestUnit(), requestAllUnits(), or open
 * buffers queue IndexJobs for requested or stale units. Running jobs preserve
 * existing bindings until replacement jobs finish in handleAsyncUpdate(). Query
 * requests wait for scope readiness via isRequestReady() while
 * moveRequestJobsToFront() prioritizes blocking jobs.
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
     * @brief Registers the processor as a listener of @p newModel, populates event tables, and registers languages.
     *
     * @param newModel    The model whose state changes drive jobs and requests.
     * @param sharedPool  Optional shared thread pool; creates an owned pool when null.
     */
    explicit Processor (LINGModel& newModel, std::shared_ptr<juce::ThreadPool> sharedPool = {});

    /** @brief Removes the processor from model listeners and waits for all pool jobs to finish. */
    ~Processor() override;

    /**
     * @brief Returns the versioned cache directory containing unpacked built-in language archives.
     *
     * Unpacks archived language resources from BinaryData when absent and prunes
     * stale language cache folders.
     *
     * @return Directory containing the unpacked built-in language folders.
     */
    static juce::File getOrCreateLanguages();

    /**
     * @brief Returns thread pool options configured for language worker threads.
     *
     * Sets @p threadCount threads, an 8 MiB stack size, and normal priority.
     *
     * @param threadCount  Number of worker threads in the pool.
     * @return Configured thread pool options.
     */
    static juce::ThreadPool::Options getPoolOptions (int threadCount);

    /**
     * @brief Forwards document iteration to index.applyToDocuments().
     *
     * @tparam Function  Callable taking (std::string_view, const jam::Document&, const Index&).
     * @param function   Callable invoked for each stored document in path order.
     */
    template <typename Function>
    void applyToDocuments (const Function& function) const
    {
        index.applyToDocuments (function);
    }

    /** @brief Marks all translation units as requested and queues their index jobs. */
    void requestAllUnits();

private:
    /**
     * @brief Tells whether a unit row depends on a file row, which is the row itself or one of its dependencies.
     *
     * Only a unit row holds arguments, so a row without arguments depends on nothing.
     *
     * @param unit       The row to test.
     * @param unitIndex  The index of the row to test in the files child.
     * @param rowIndex   The index of the file row in the files child.
     * @return @c true when the row holds arguments, and its index or one of its dependencies is @p rowIndex.
     */
    static bool hasDependency (const juce::ValueTree& unit, int unitIndex, int rowIndex);

    /**
     * @brief Tells whether @p unit or any of its dependencies (other than @p rowIndex) has a running or queued ParseJob.
     *
     * @param unit       The unit row to check.
     * @param unitIndex  Index of @p unit in the files child.
     * @param rowIndex   File row index excluded from the check because its ParseJob just finished.
     * @return @c true when another required file still has an active or queued ParseJob.
     */
    bool hasParseJob (const juce::ValueTree& unit, int unitIndex, int rowIndex) const;

    /**
     * @brief Tells whether @p unit or any of its dependencies has an open editor buffer.
     *
     * @param unit  Translation unit row to inspect.
     * @return @c true when the unit or a dependency appears in @c Id::buffers.
     */
    bool hasOpenBuffer (const juce::ValueTree& unit) const;

    /**
     * @brief Returns the unit row that owns @p path in @c index.unitOwners.
     *
     * @param path  Full file path to look up.
     * @return The owning unit row, or an invalid ValueTree when @p path has no unit.
     */
    juce::ValueTree getUnit (const juce::String& path) const;

    /**
     * @brief Packs the Id::arguments array of @p unit into a null-delimited byte buffer.
     *
     * @param unit  Translation unit row holding Id::arguments.
     * @return Contiguous null-delimited argument strings.
     */
    jam::Array<char> getArguments (const juce::ValueTree& unit) const;

    /**
     * @brief Appends @p argument and a trailing null byte to @p arguments.
     *
     * @param arguments  Destination null-delimited argument buffer.
     * @param argument   Argument string to append.
     */
    static void addArgument (jam::Array<char>& arguments, std::string_view argument);

    /**
     * @brief Builds C-string pointers into a null-delimited @p arguments buffer.
     *
     * @param arguments  Null-delimited argument buffer.
     * @return Pointers to the start of each argument string inside @p arguments.
     */
    static jam::Array<const char*> getArgumentPointers (const jam::Array<char>& arguments);

    /**
     * @brief Computes the Wyhash content stamp for @p unit from its arguments, unit file hash, and dependency hashes.
     *
     * @param unit       Translation unit row.
     * @param unitIndex  Index of @p unit in the files child.
     * @param arguments  Packed null-delimited compile arguments.
     * @return Combined 64-bit hash key for symbol swap validation.
     */
    juce::int64 getUnitHash (const juce::ValueTree& unit, int unitIndex, const jam::Array<char>& arguments) const;

    /**
     * @brief Computes the unit stamp by folding getUnitHash() with output file existence.
     *
     * @param unit       Translation unit row.
     * @param unitIndex  Index of @p unit in the files child.
     * @param arguments  Packed null-delimited compile arguments.
     * @return Combined 64-bit stamp key for staleness and swap validation.
     */
    juce::int64 getUnitStamp (const juce::ValueTree& unit, int unitIndex, const jam::Array<char>& arguments) const;

    /**
     * @brief Populates @c events, @c propertyEvents, and @c childEvents dispatch tables.
     *
     * Called once by the constructor so request, property, and child callbacks
     * resolve via lookup instead of branch chains.
     */
    void registerEvents();

    /**
     * @brief Executes the request handler for @p request, or writes an error reply.
     *
     * Clears Id::isRequested on the owning unit and its prerequisites. When the language row of the
     * unit holds Id::message, sends the file path and that message to @c model. A valid command runs
     * through @c bufferEvents or @c events. A request that has a failed unit row receives
     * lsp::ErrorCodes::RequestFailed, with the failed row messages, except a buffer request, which
     * runs without this check. A request without a valid command or valid arguments receives
     * lsp::ErrorCodes::InvalidParams.
     *
     * @param request  The request row whose command selects the operation and receives Id::result.
     */
    void runRequest (juce::ValueTree& request);

    /**
     * @brief Tells whether the rows that @p request needs have finished indexing.
     *
     * For the files scope, the files child must be indexed. For any other scope, the file row of the
     * request must be indexed. For a scope other than Id::file, every dependency row of the unit of
     * the request must be indexed too, when the file has a unit row.
     *
     * @param request  The request row to check against map::Readiness scope rules.
     * @return @c true when the request can run immediately.
     */
    bool isRequestReady (const juce::ValueTree& request) const;

    /**
     * @brief Returns the readiness scope of the command of @p request.
     *
     * Returns the scope that map::Readiness gives the command, when the file of the request is a
     * stored path row and the command has an entry. Otherwise returns Id::files.
     *
     * @param request  The request row whose command and file give the scope.
     * @return The scope identifier.
     */
    juce::Identifier getScope (const juce::ValueTree& request) const;

    /**
     * @brief Returns the unit rows that @p request waits for.
     *
     * For the files scope, every row that holds Id::arguments. For any other scope, the unit row that
     * owns the file of the request. For the dependencies scope, the unit rows that own the dependencies
     * of that unit are added too.
     *
     * @param request  The request row.
     * @return The row indices.
     */
    jam::Array<int> getUnitRows (const juce::ValueTree& request) const;

    /**
     * @brief Returns the rows of getUnitRows() that hold Id::message.
     *
     * @param request  The request row.
     * @return The row indices of the failed units.
     */
    jam::Array<int> getFailedRows (const juce::ValueTree& request) const;

    /**
     * @brief Joins the path and the Id::message of each row into one text, one line for each row.
     *
     * Each line is the path, the diagnostic separator, and the message of the row.
     *
     * @param rows  The row indices to report.
     * @return The joined text.
     */
    juce::String getMessage (const jam::Array<int>& rows) const;

    /**
     * @brief Marks the target unit requested and moves its blocking parse and index jobs to the front of @c pool.
     *
     * @param request  Waiting request row whose target file determines which jobs to prioritize.
     */
    void moveRequestJobsToFront (const juce::ValueTree& request);

    /**
     * @brief Scans all connections for queued requests that are now ready and executes them.
     */
    void runRequests();

    /**
     * @brief Appends a publishDiagnostics request to each connection that supports position encoding, when @p file finishes indexing.
     *
     * Skips the file when its unit row holds Id::message.
     *
     * @param file  Indexed file row whose path is published.
     */
    void addDiagnosticRequests (const juce::ValueTree& file);

    /**
     * @brief Updates Id::isIndexed on file row @p rowIndex from its parse and unit index job state.
     *
     * @param rowIndex  Index of the file row in the files child.
     */
    void setIndexed (int rowIndex);

    /**
     * @brief Updates Id::isIndexed on @p unit and on every dependency row owned by @p unitIndex.
     *
     * @param unit       Translation unit row.
     * @param unitIndex  Index of @p unit in the files child.
     */
    void setUnitIndexed (const juce::ValueTree& unit, int unitIndex);

    /**
     * @brief Updates global Id::isIndexed on the files tree and refreshes overlay jobCount and pendingJobCount parameters.
     */
    void setIndexed();

    /**
     * @brief Dispatches child additions on @c model.state via @c childEvents.
     *
     * Handles Id::files (starts parse jobs), Id::request (queues the failed units through
     * addIndexJobs() when the command has an event, then runs the request when it is ready,
     * or moves its blocking jobs to the front otherwise), Id::language (loads the language
     * library and starts unit discovery), and Id::toolchain (starts unit discovery).
     *
     * @param parentTree              Tree that received the child (unused).
     * @param childWhichHasBeenAdded  Added child tree whose type selects the action.
     */
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;

    /**
     * @brief Clears unneeded unit requests when a request or connection leaves the tree.
     *
     * @param parentTree                       Tree that lost a child (unused).
     * @param childWhichHasBeenRemoved         Removed child tree.
     * @param indexFromWhichChildWasRemoved    Previous child index (unused).
     */
    void valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;

    /** @brief Removes @c Id::isRequested from unit rows no longer needed by any pending request. */
    void clearUnusedRequests();

    /**
     * @brief Registers built-in languages and appends a language row for each language directory in config.
     */
    void addLanguages();

    /**
     * @brief Registers the six built-in document languages into @c index.languages and @c model.state.
     *
     * @return Next available language family identifier after the built-in languages.
     */
    int addBuiltInLanguages();

    /**
     * @brief Prunes stale documents, rebuilds path/unit lookup tables, and starts ParseJobs for @p files.
     *
     * @param files  The files child tree added to @c model.state.
     */
    void addJobs (const juce::ValueTree& files);

    /**
     * @brief Dispatches property changes on @c model.state via @c propertyEvents.
     *
     * Handles Id::path on file or language rows, Id::hash on language rows,
     * Id::isIndexed to drain waiting requests and publish diagnostics, and
     * Id::threads to rebuild @c pool with the updated thread count.
     *
     * @param tree      Tree whose property changed.
     * @param property  Identifier of the changed property.
     */
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;

    /**
     * @brief Starts or queues a ParseJob for file @p row at @p rowIndex, passing any open buffer text.
     *
     * @param row       File row whose path and family define the job.
     * @param rowIndex  Index of @p row in the files child.
     */
    void addJob (const juce::ValueTree& row, int rowIndex);

    /**
     * @brief One-shot parse of a single file row (or open buffer) run on the thread pool.
     *
     * Reads or parses the source, optionally obtains a SymbolDocument from
     * built-in languages, catches std::exception at the job boundary, and signals
     * handleAsyncUpdate() on completion.
     */
    struct ParseJob : public juce::ThreadPoolJob
    {
        /**
         * @brief Creates a parse job for one file row.
         *
         * @param ownerToUse         Processor notified when the job finishes.
         * @param rowIndexToUse      Index of the file row in the files child.
         * @param pathToUse          Source file path.
         * @param swapFileToUse      Syntax swap file path.
         * @param languageToUse      Language instance for the row's family.
         * @param languageHashToUse  Hash of the language row.
         * @param familyToUse        Language family identifier.
         * @param bufferToUse        Optional unsaved editor text overriding disk contents.
         */
        ParseJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::File& swapFileToUse, std::shared_ptr<const Language> languageToUse, juce::int64 languageHashToUse, int familyToUse, std::optional<juce::String> bufferToUse);

        /**
         * @brief Loads from swap or parses the source, catches boundary exceptions, and signals the processor.
         *
         * @return jobHasFinished when complete.
         */
        JobStatus runJob() override;

        Processor& owner;///< Processor notified via triggerAsyncUpdate() on completion.
        int rowIndex { 0 };///< Index of the file row in the files child.
        juce::String path;///< Path of the file copied on the calling thread.
        juce::File swapFile;///< Syntax swap file read or updated when no buffer override is present.
        std::shared_ptr<const Language> language;///< Language instance kept alive for the job's duration.
        juce::int64 languageHash { 0 };///< Language hash required for swap cache hits.
        int family { 0 };///< Language family identifier stored on swap documents.
        /** @brief Wyhash digest of the parsed source bytes, or zero on failure. */
        juce::int64 sourceHash { 0 };
        std::optional<juce::String> buffer;///< Unsaved buffer text overriding disk contents when present.
        std::unique_ptr<jam::Document> document;///< Parsed syntax document moved into @c index on the message thread.
        std::unique_ptr<SymbolDocument> symbols;///< Optional SymbolDocument produced directly by built-in languages.
        juce::Result result { juce::Result::ok() };///< Validation, parse, or exception error result.
        juce::Time modificationTime;///< File modification timestamp captured before reading from disk.
        std::atomic<bool> isFinished { false };///< Release-stored completion flag read by handleAsyncUpdate().
    };

    /**
     * @brief One-shot semantic index pass over one translation unit run on the thread pool.
     *
     * Checks the symbol swap cache first; on a miss, invokes language->index(),
     * builds a SymbolDocument via Builder, and catches std::exception at the
     * thread boundary.
     */
    struct IndexJob : public juce::ThreadPoolJob
    {
        /**
         * @brief Creates an index job for one translation unit row.
         *
         * @param ownerToUse           Processor notified when the job finishes.
         * @param rowIndexToUse        Index of the unit row in the files child.
         * @param pathToUse            Source file path of the unit.
         * @param directoryToUse       Working directory of the compile command.
         * @param argumentsToUse       Null-delimited compile arguments.
         * @param hashToUse            Unit content hash including dependencies and output existence.
         * @param symbolSwapFileToUse  Binary symbol swap file path.
         * @param languageHashToUse    Hash of the language row.
         * @param languageToUse        Language instance for the unit's family.
         * @param familyToUse          Language family identifier.
         * @param bufferPathsToUse     Open editor buffer paths passed as unsaved overlays.
         * @param bufferTextsToUse     Open editor buffer contents aligned with @p bufferPathsToUse.
         * @param unitOwnersToUse      Shared file-to-unit ownership and roots snapshot.
         */
        IndexJob (Processor& ownerToUse, int rowIndexToUse, const juce::String& pathToUse, const juce::String& directoryToUse, jam::Array<char> argumentsToUse, juce::int64 hashToUse, const juce::File& symbolSwapFileToUse, juce::int64 languageHashToUse, std::shared_ptr<const Language> languageToUse, int familyToUse, juce::StringArray bufferPathsToUse, juce::StringArray bufferTextsToUse, std::shared_ptr<const Index::UnitOwners> unitOwnersToUse);

        /**
         * @brief Opens the symbol swap cache or runs addSymbols(), catching std::exception at the thread boundary.
         *
         * @return jobHasFinished when complete.
         */
        JobStatus runJob() override;

        /**
         * @brief Invokes language->index() with the builder callbacks, and writes the SymbolDocument to temporary.
         *
         * When index returns zero, the builder is sorted and deduplicated, and its SymbolDocument is
         * written to temporary, a juce::TemporaryFile that targets symbolSwapFile. Otherwise result
         * keeps the failure set before the call, or the message that addMessage() recorded.
         */
        void addSymbols();

        /**
         * @brief Callback recording one symbol occurrence into the job's SymbolDocument::Builder.
         *
         * @param context      Pointer to the owning IndexJob.
         * @param recordPath   File path of the occurrence.
         * @param line         1-based source line.
         * @param column       1-based byte column.
         * @param symbol       Unique symbol identifier.
         * @param nameLength   Byte length of the symbol name.
         * @param role         Bitmask of jam::LanguageRole flags.
         * @param type         Token type of the symbol.
         * @param hover        Per-file HoverEntry index, or jam::LanguageCallbacks::noHover.
         * @param typeSymbols  Associated type symbol identifiers.
         * @param typeCount    Count of entries in @p typeSymbols.
         */
        static void addSymbol (void* context, const char* recordPath, int line, int column, unsigned long long symbol, int nameLength, int role, int type, int hover, const unsigned long long* typeSymbols, int typeCount);

        /**
         * @brief Callback appending one hover entry to the job's SymbolDocument::Builder.
         *
         * @param context         Pointer to the owning IndexJob.
         * @param recordPath      File path owning the hover entry.
         * @param text            Hover documentation string.
         * @param definitionPath  Definition file path, or nullptr when unavailable.
         * @param line            1-based start line.
         * @param column          1-based start column.
         * @param endLine         1-based end line.
         * @param endColumn       1-based end column.
         * @return Per-file hover index for SymbolRecord::hover.
         */
        static int addHover (void* context, const char* recordPath, const char* text, const char* definitionPath, int line, int column, int endLine, int endColumn);

        /**
         * @brief Callback appending one inlay hint to the job's SymbolDocument::Builder.
         *
         * @param context       Pointer to the owning IndexJob.
         * @param recordPath    File path of the hint.
         * @param line          1-based anchor line.
         * @param column        1-based anchor column.
         * @param label         Hint display label.
         * @param type          LSP inlay hint kind.
         * @param paddingLeft   Nonzero to request left padding.
         * @param paddingRight  Nonzero to request right padding.
         */
        static void addInlayHint (void* context, const char* recordPath, int line, int column, const char* label, int type, int paddingLeft, int paddingRight);

        /**
         * @brief Callback appending one diagnostic entry to the job's SymbolDocument::Builder.
         *
         * @param context     Pointer to the owning IndexJob.
         * @param recordPath  File path of the diagnostic.
         * @param line        1-based start line.
         * @param column      1-based start column.
         * @param endLine     1-based end line.
         * @param endColumn   1-based end column.
         * @param severity    LSP diagnostic severity.
         * @param message     Diagnostic message text.
         */
        static void addDiagnostic (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, int severity, const char* message);

        /**
         * @brief Callback deduplicating and appending one outline entry to the job's SymbolDocument::Builder.
         *
         * @param context          Pointer to the owning IndexJob.
         * @param recordPath       File path of the outline symbol.
         * @param line             1-based start line.
         * @param column           1-based start column.
         * @param endLine          1-based end line.
         * @param endColumn        1-based end column.
         * @param nameLength       Byte length of the symbol name.
         * @param qualifierLength  Byte length of the scope qualifier.
         * @param type             LSP symbol kind.
         * @param parent           Slot index returned by a prior addOutline() call, or negative.
         * @return Slot index for child outlines to reference as @p parent.
         */
        static int addOutline (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, int nameLength, int qualifierLength, int type, int parent);

        /**
         * @brief Callback deduplicating and appending one completion candidate to the job's SymbolDocument::Builder.
         *
         * @param context      Pointer to the owning IndexJob.
         * @param recordPath   File path associated with the completion.
         * @param scopeSymbol  Enclosing scope symbol identifier.
         * @param symbol       Candidate symbol identifier.
         * @param label        Display label.
         * @param insertText   Inserted text or snippet.
         * @param type         Completion item kind index.
         */
        static void addCompletion (void* context, const char* recordPath, unsigned long long scopeSymbol, unsigned long long symbol, const char* label, const char* insertText, int type);

        /**
         * @brief Callback appending one diagnostic fix-it edit to the job's SymbolDocument::Builder.
         *
         * @param context     Pointer to the owning IndexJob.
         * @param recordPath  File path of the edit.
         * @param line        1-based start line.
         * @param column      1-based start column.
         * @param endLine     1-based end line.
         * @param endColumn   1-based end column.
         * @param text        Replacement text.
         */
        static void addFixIt (void* context, const char* recordPath, int line, int column, int endLine, int endColumn, const char* text);

        /**
         * @brief Records the message text of a language failure as the result of the job, after failUnitIndex.
         *
         * @param context      Pointer to the owning IndexJob.
         * @param messageText  The message text from the language library.
         */
        static void addMessage (void* context, const char* messageText);

        /** @brief Number of integer fields hashed to deduplicate outline entries. */
        static constexpr int outlineFieldCount { 8 };

        /**
         * @brief Resolves a callback outline slot index to a per-file parent index when in the same file.
         *
         * @param parent     Slot index passed from language->index(), or negative.
         * @param pathIndex  File index of the child outline entry.
         * @return Per-file parent index, or SymbolDocument::noParent.
         */
        int getOutlineParent (int parent, int pathIndex) const;

        /**
         * @brief Tells whether @p recordPath belongs to another unit or lies outside project roots.
         *
         * Delegates to @c unitOwners->isForeign() with this job's @c rowIndex.
         *
         * @param recordPath  File path to test.
         * @return @c true when @p recordPath belongs to another unit or lies outside project roots.
         */
        bool isForeign (const char* recordPath) const;

        Processor& owner;///< Processor notified via triggerAsyncUpdate() on completion.
        int rowIndex { 0 };///< Index of the unit row in the files child.
        juce::String path;///< Source file path of the unit.
        juce::String directory;///< Working directory of the compile command.
        jam::Array<char> arguments;///< Null-delimited compile arguments.
        juce::int64 hash { 0 };///< Unit content hash key for symbol swap validation.
        juce::File symbolSwapFile;///< Binary SymbolDocument swap file path.
        juce::int64 languageHash { 0 };///< Language hash required for swap cache hits.
        std::shared_ptr<const Language> language;///< Language instance kept alive for the job's duration.
        int family { 0 };///< Language family identifier stored in the SymbolDocument header.
        juce::StringArray bufferPaths;///< Paths of open editor buffers passed to the indexer.
        juce::StringArray bufferTexts;///< Text contents of open editor buffers aligned with @c bufferPaths.
        /** @brief Shared unit ownership and project roots snapshot queried by isForeign(). */
        std::shared_ptr<const Index::UnitOwners> unitOwners;
        SymbolDocument::Builder builder;///< Accumulator populated by static indexer callbacks.
        std::unique_ptr<SymbolDocument> symbols;///< SymbolDocument read from a valid swap file by runJob(), or null. addBindings() moves it into index.
        jam::Array<std::pair<int, int>> outlineSlots;///< Maps callback slot indices to (fileIndex, per-file outlineIndex).
        jam::HashMap<juce::int64, int> outlineKeys;///< Hash set deduplicating identical outline entries.
        jam::HashSet<juce::int64> completionKeys;///< Hash set deduplicating (scopeSymbol, symbol) completions.
        juce::Result result { juce::Result::ok() };///< Job result. addSymbols() sets failUnitIndex before the index call. addMessage() replaces it with the language message. A zero index result replaces it with the swap write result. An exception replaces it with the exception text.
        std::unique_ptr<juce::TemporaryFile> temporary;///< Temporary file that holds the swap written by addSymbols(), targeting symbolSwapFile. It is null when addSymbols() does not write a swap.
        std::atomic<bool> isFinished { false };///< Release-stored completion flag read by handleAsyncUpdate().
    };

    /**
     * @brief One-shot unit discovery job over toolchain commands run on the thread pool.
     *
     * Queries each language's addUnits() callback, builds a fresh Id::files ValueTree
     * with buildFiles, catches std::exception at the job boundary, and signals
     * handleAsyncUpdate() on completion.
     */
    struct UnitsJob : public juce::ThreadPoolJob
    {
        /**
         * @brief Creates a unit discovery job for the active toolchain.
         *
         * @param ownerToUse           Processor notified when the job finishes.
         * @param argumentsToUse       Null-delimited command arguments across all toolchain rows.
         * @param argumentCountsToUse  Argument count for each toolchain row.
         * @param directoryToUse       Project root directory.
         * @param cacheDirectoryToUse  Project cache directory.
         * @param languagesToUse       Snapshot of loaded languages by family.
         * @param familiesToUse        Extension-to-family map built from registered languages.
         * @param rootsToUse           Project root directories filtering stored dependencies.
         */
        UnitsJob (Processor& ownerToUse, jam::Array<char> argumentsToUse, jam::Array<int> argumentCountsToUse, const juce::String& directoryToUse, const juce::String& cacheDirectoryToUse, jam::HashMap<int, std::shared_ptr<const Language>> languagesToUse, jam::HashMap<juce::String, int> familiesToUse, jam::Array<juce::File> rootsToUse);

        /**
         * @brief Invokes addUnits() for each language, stores buildFiles, catches boundary exceptions, and signals @c owner.
         *
         * @return jobHasFinished when complete.
         */
        JobStatus runJob() override;

        /**
         * @brief Callback recording one discovered translation unit and its dependencies into @c files.
         *
         * @param context            Pointer to the owning UnitsJob.
         * @param path               Source file path of the unit.
         * @param directory          Working directory of the compile command.
         * @param output             Output artifact path, or empty.
         * @param arguments          Array of compile argument strings.
         * @param argumentCount      Number of entries in @p arguments.
         * @param dependencies       Array of included/dependent file paths.
         * @param dependencyCount    Number of entries in @p dependencies.
         * @param prerequisites      Array of prerequisite unit paths.
         * @param prerequisiteCount  Number of entries in @p prerequisites.
         */
        static void addUnit (void* context, const char* path, const char* directory, const char* output, const char* const* arguments, int argumentCount, const char* const* dependencies, int dependencyCount, const char* const* prerequisites, int prerequisiteCount);

        /**
         * @brief Callback appending a build system file path watched for manifest reload.
         *
         * Normalizes @p path via juce::File::getFullPathName() before storing it in @c buildFiles.
         *
         * @param context  Pointer to the owning UnitsJob.
         * @param path     Build file path reported by the language.
         */
        static void addBuildFile (void* context, const char* path);

        /**
         * @brief Callback storing the message text on @c files, as its Id::message property.
         *
         * @param context  Pointer to the owning UnitsJob.
         * @param text     Message text from the language library.
         */
        static void addMessage (void* context, const char* text);

        /**
         * @brief Returns the row index for @p path in @c files, appending a new Id::file row when absent.
         *
         * Normalizes @p path via juce::File::getFullPathName() before deduplicating against @c paths and storing Id::path.
         *
         * @param path    File path to intern.
         * @param family  Language family identifier for newly added rows.
         * @return Zero-based row index in @c files.
         */
        int getOrCreatePathIndex (const juce::String& path, int family);

        /**
         * @brief Converts @p filePathCount C-strings in @p filePaths to an array of @c files row indices.
         *
         * @param filePaths      Array of file path strings.
         * @param filePathCount  Number of paths in @p filePaths.
         * @param family         Language family identifier used when adding missing rows.
         * @return Array of integer row indices as juce::var entries.
         */
        juce::Array<juce::var> getOrCreatePathIndices (const char* const* filePaths, int filePathCount, int family);
        /**
         * @brief Converts @p filePathCount dependency paths inside project roots to @c files row indices.
         *
         * @param filePaths      Array of dependency file path strings.
         * @param filePathCount  Number of paths in @p filePaths.
         * @param family         Language family identifier used when adding missing rows.
         * @return Array of integer row indices as juce::var entries.
         */
        juce::Array<juce::var> getOrCreateDependencyIndices (const char* const* filePaths, int filePathCount, int family);

        /**
         * @brief Tells whether @p file lies inside @c roots.
         *
         * @param file  Candidate dependency file.
         * @return @c true when @c roots is empty or contains @p file.
         */
        bool isWithinRoots (const juce::File& file) const;

        Processor& owner;///< Processor notified via triggerAsyncUpdate() on completion.
        jam::Array<char> arguments;///< Null-delimited command arguments across toolchain rows.
        jam::Array<int> argumentCounts;///< Number of arguments per toolchain row.
        juce::String directory;///< Project root directory.
        juce::String cacheDirectory;///< Project cache directory.
        jam::HashMap<int, std::shared_ptr<const Language>> languages;///< Snapshot of loaded languages.
        jam::HashMap<juce::String, int> families;///< Maps file extensions to language family identifiers.
        /** @brief Project root directories filtering indexed dependencies. */
        jam::Array<juce::File> roots;
        juce::ValueTree files { jam::Format::toType (Id::files) };///< Discovered files tree applied to @c model on completion.
        jam::Strings paths;///< Deduplicated file paths indexed in row order of @c files.
        juce::Array<juce::var> buildFiles;///< Build system files collected via addBuildFile().
        std::atomic<bool> isFinished { false };///< Release-stored completion flag read by handleAsyncUpdate().
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

    /** @brief Sentinel row index passed to hasParseJob() when no file row is excluded. */
    static constexpr int noRow { -1 };

    /**
     * @brief Starts a job for a row, or queues it behind the running job of that row.
     *
     * @tparam Job      The job type (ParseJob, IndexJob, or UnitsJob).
     * @param jobs      Running jobs keyed by row index.
     * @param nextJobs  Queued jobs keyed by row index.
     * @param rowIndex  Index of the row owning the job.
     * @param newJob    New job to run or queue.
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
     * @brief Removes the finished job of @p rowIndex and starts its queued replacement when present.
     *
     * @tparam Job      The job type (ParseJob, IndexJob, or UnitsJob).
     * @param jobs      Running jobs keyed by row index.
     * @param nextJobs  Queued jobs keyed by row index.
     * @param rowIndex  Index of the row whose job finished.
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

    /**
     * @brief Returns sorted row indices of jobs in @p jobs whose @c isFinished flag is set.
     *
     * @tparam Job  The job type (ParseJob, IndexJob, or UnitsJob).
     * @param jobs  Active jobs keyed by row index.
     * @return Ascending array of finished row indices.
     */
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
     * @brief Queues an IndexJob for unit @p row at @p rowIndex, and signals any running IndexJob of that row to stop.
     *
     * Removes Id::message from the row. The running job, when it leaves the pool without a wait, is
     * erased from indexJobs. Otherwise it keeps running, and the new job waits in nextIndexJobs.
     *
     * @param row       Translation unit row holding arguments, directory, path, output, and family.
     * @param rowIndex  Index of @p row in the files child.
     */
    void addIndexJob (juce::ValueTree row, int rowIndex);

    /**
     * @brief Starts queued IndexJobs whose prerequisite units are idle, prioritizing requested and producer units.
     */
    void addIndexJobs();

    /**
     * @brief Queues the IndexJobs of the failed unit rows of @p request, and of their failed prerequisites.
     *
     * For each row of getFailedRows(), each prerequisite that holds Id::message is queued first.
     * Then the row itself is queued through addIndexJob().
     *
     * @param request  The request row whose failed unit rows are queued again.
     */
    void addIndexJobs (const juce::ValueTree& request);

    /**
     * @brief Marks unit @p unitRow requested and queues an IndexJob when ready to index.
     *
     * @param unitRow  Unit row index in the files child.
     */
    void requestUnit (int unitRow);

    /**
     * @brief Applies the result of a finished IndexJob to @c index, when the row path matches the job path.
     *
     * With @c job.symbols set, the document is moved into index.addBindings(). Otherwise, when
     * @c job.result is ok, the temporary file of the job is bound through index.addBindings(). A failed
     * result sets the overlay message, and sets Id::message on the row.
     *
     * @param job  Finished IndexJob.
     */
    void addBindings (IndexJob& job);

    /**
     * @brief Applies finished UnitsJobs, ParseJobs, and IndexJobs on the message thread and notifies @c index when all jobs finish.
     */
    void handleAsyncUpdate() override;

    /**
     * @brief Starts a ParseJob for changed @p file when its family has a registered language.
     *
     * @param file  File row whose Id::path changed or was invalidated by a buffer update.
     */
    void setFile (const juce::ValueTree& file);

    /**
     * @brief Stores or updates an unsaved editor buffer for an Id::setBuffer request and triggers a re-parse.
     *
     * @param request  Request row providing the file path, Id::directory, and Id::text.
     */
    void setBuffer (juce::ValueTree& request);

    /**
     * @brief Removes an unsaved editor buffer for an Id::removeBuffer request and triggers a re-parse.
     *
     * @param request  Request row providing the file path and Id::directory.
     */
    void removeBuffer (juce::ValueTree& request);

    /**
     * @brief Notifies the matching file row to re-parse and completes @p request.
     *
     * Sends an Id::path property change on the matching file row in Id::files when indexed.
     * Removes @p request from its parent when the parent has Id::positionEncoding,
     * and writes a JSON-RPC @c null result via Query::setResult() otherwise.
     *
     * @param request  Buffer request row to notify and complete.
     */
    void setBufferChange (juce::ValueTree& request);

    /**
     * @brief Starts a ParseJob for every file row matching the family of @p languageRow.
     *
     * @param languageRow  Language row whose Id::hash changed after loading.
     */
    void setLanguageHash (const juce::ValueTree& languageRow);

    /**
     * @brief Loads the language library for @p languageRow, caches a content-hashed copy when built locally, and updates Id::hash.
     *
     * @param languageRow  Language row whose family receives the loaded LibraryLanguage.
     */
    void setLanguage (juce::ValueTree languageRow);

    /**
     * @brief Applies a finished ParseJob to @c index when its path, mtime, and language still match, and schedules ready unit IndexJobs.
     *
     * @param job  Finished ParseJob.
     */
    void addDocument (ParseJob& job);

    /**
     * @brief Tells whether @p unit lacks a current SymbolDocument matching getUnitStamp().
     *
     * @param unit       Translation unit row.
     * @param unitIndex  Index of @p unit in the files child.
     * @return @c true when the unit is unindexed or its stamp changed.
     */
    bool isUnitStale (const juce::ValueTree& unit, int unitIndex) const;
    /**
     * @brief Re-queues an IndexJob after a disk edit when the unit is stale and active.
     *
     * @param unit              Translation unit row.
     * @param unitIndex         Index of @p unit in the files child.
     * @param finishedRowIndex  Row index whose ParseJob just finished.
     */
    void updateUnitOnDiskEdit (const juce::ValueTree& unit, int unitIndex, int finishedRowIndex);

    /**
     * @brief Starts a UnitsJob for the current Id::toolchain child on @c model.state.
     */
    void addUnitsJob();

    /**
     * @brief Builds a UnitsJob configured from @p toolchain and the current registered languages.
     *
     * @param toolchain  Toolchain tree containing compiler command rows.
     * @return Newly constructed UnitsJob.
     */
    std::unique_ptr<UnitsJob> getUnitsJob (const juce::ValueTree& toolchain);

    /**
     * @brief Applies finished UnitsJobs by replacing @c model.state's files child and cycling queued UnitsJobs.
     */
    void setUnitsJobs();

    /**
     * @brief Reports any unit-discovery error message on @p files and replaces the files child on @c model.state.
     *
     * @param files  Discovered files tree from a finished UnitsJob.
     */
    void setFiles (const juce::ValueTree& files);

    /** @brief The model that this processor listens to. The application owns the model. */
    LINGModel& model;

    /** @brief The documents, the bindings, and the libraries of the families. The processor writes it, and Query reads it through a const reference. */
    Index index;

    /** @brief Answers the request rows from the index. Its only state is the fuzzy finder slab. */
    Query query;

    /** @brief Active ParseJob instances keyed by file row index. */
    jam::HashMap<int, std::unique_ptr<ParseJob>> parseJobs;

    /** @brief Queued replacement ParseJob instances waiting for active jobs on the same row index. */
    jam::HashMap<int, std::unique_ptr<ParseJob>> nextParseJobs;

    /** @brief Active IndexJob instances keyed by unit row index. */
    jam::HashMap<int, std::unique_ptr<IndexJob>> indexJobs;

    /** @brief Queued IndexJob instances waiting to run by unit row index. */
    jam::HashMap<int, std::unique_ptr<IndexJob>> nextIndexJobs;

    /** @brief Active UnitsJob instances keyed by toolchain child index. */
    jam::HashMap<int, std::unique_ptr<UnitsJob>> unitsJobs;

    /** @brief Queued replacement UnitsJob instances keyed by toolchain child index. */
    jam::HashMap<int, std::unique_ptr<UnitsJob>> nextUnitsJobs;

    /** @brief Thread pool owned by this processor when no shared pool is injected. */
    std::shared_ptr<juce::ThreadPool> ownedPool;
    /**
     * @brief Thread pool executing unit-discovery, parse, and index jobs.
     *
     * Declared after the job maps so ~Processor() removes all jobs via
     * pool->removeJob() before the maps free them.
     */
    std::shared_ptr<juce::ThreadPool> pool;

    /** @brief Request command handlers keyed by command identifier. */
    Events events;

    /** @brief The setBuffer and removeBuffer operations, keyed by command identifier. runRequest() runs them without a check of the failed units. */
    Events bufferEvents;

    /** @brief Model property-change handlers keyed by property identifier. */
    Events propertyEvents;

    /** @brief Model child-addition handlers keyed by child tree type. */
    Events childEvents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Processor)
};
