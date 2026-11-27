# RFC: Demand-Driven Indexing, State Retention, and Efficiency Confinement

- **Project:** `ling` (Language Index Navigation Graph)
- **Baseline:** v0.0.1 (post-Sprint 19)
- **Governing Contracts:** `MANIFESTO.md` (BLESSED), `CODING.md`, `NAMES.md`, `Source/HELP.md`
- **Handoff Target:** COUNSELOR (for owner verification and `PLAN.md` decomposition)

---

## 1. Objective and Non-Negotiable Boundaries

`ling` already exceeds `clangd` in both query accuracy and speed on modular C++/JUCE codebases. The remaining defect is **resource efficiency** during active development and multi-agent sessions:

1. **Thermal and CPU gluttony:** Indexing drives laptop fans to maximum RPM by escaping background thread pools onto OS performance cores.
2. **Parallel-agent thrashing:** When three concurrent MCP agents read and edit files on disk, `ling` enters a non-stop loop of speculative unit indexing, mid-parse aborts, and full-workspace state wipes.
3. **Compilation-exceeding index cost:** Warm edits and manifest reloads perform redundant work across unchanged units, transit data copies, and unguarded AST walks.

### Architectural Invariants

- **Universal Engine Boundary (`Source/`):** `ling` is a language-agnostic, compiler-agnostic, and build-system-agnostic engine. Zero `clang`, `libclang`, `ninja`, or C++ specifics may enter `Source/`. All compiler and AST behavior lives behind the polymorphic `Language` interface (`Source/Language.h`) and the dynamic `jam::LanguageLibrary` ABI (`language/cLang/`).
- **Zero Accuracy or Determinism Regression (`MANIFESTO.md` D):** Every request must continue to wait until its exact `map::Readiness` scope is indexed (`Source/HELP.md:169-173`). Stale symbol reads, probabilistic heuristics, and timer-based race windows are forbidden.
- **Zero Timer State in `Processor` (`MANIFESTO.md` S):** `LINGModel` is the single state machine. `Processor` remains a deterministic orchestrator over `LINGModel` state and `pool` workers.

---

## 2. Ground-of-Truth Diagnosis and Contract Violations

### 2.1 Universal Engine Violations (`Source/`)

| ID | Location | Factual Mechanism | BLESSED / CODING Violation |
| :--- | :--- | :--- | :--- |
| **E1** | `Processor.cpp:799-808`, `Processor.cpp:678-684` | On every disk file edit (`not job.buffer.has_value()`), `Processor::addDocument` immediately queues an `IndexJob` for **every** unit that lists the edited file in `Id::dependencies`. Subsequent edits abort running `IndexJob` instances via `pool->removeJob (..., true, noWait)`, discarding partial parses in a continuous loop. | **E — Materialisation** (*"only when it needs the capability"*) and **L — YAGNI**: materialises `SymbolDocument` objects that no active request or open editor buffer asked for. |
| **E2** | `Processor.cpp:441, 851-858`, `Index.cpp:7-27` | Any change to `Id::manifest` or `Id::buildFiles` (`LINGModel.cpp:221`) runs `UnitsJob` and replaces `Id::files`. `Processor::addJobs` calls `Index::removeDocuments`, which unbinds and clears **every** `SymbolDocument` in `symbolDocuments` because `symbolDocuments` (`Index.h:358`) and `symbolFiles` (`Index.h:361`) are keyed by transient `files` child `rowIndex` rather than `Index`'s lifetime-stable `pathIndex`. `addJobs` then spawns `ParseJob` and `IndexJob` runs across all project files. | **E — Complete at Creation** and **S — SSOT**: destroys valid, immutable `SymbolDocument` instances whose inputs did not change. |
| **E3** | `Index.h:364`, `Index.cpp:323-339`, `SymbolDocument.cpp:225-242` | `SymbolDocument::Builder::sortAndDeduplicate` sorts outlines, inlay hints, and fix-its on the worker thread, but leaves `file.symbols` unsorted. Consequently, `Index::addRecords` allocates a duplicate `jam::Array<const SymbolRecord*>` per file and runs `std::stable_sort` on the **message thread** during every `addBindings` call. | **E — Complete at Creation** (*"Data in transit has 0 allocation, 0 copy, 0 temporary container"*): mutates data shape in transit instead of sorting once at creation. |
| **E4** | `Index.cpp:246-267` | `Index::removeCompletions` loops over every completion in an unbound file without deduplicating `completion.scope` (unlike `removedSymbols` in `Index::removeSymbols`, `Index.cpp:217-227`), running a linear `std::remove_if` over all global completions (`scope == 0`) once per record in `$O(M \cdot C)$` time on the message thread. | **L — Lean** and **NAMES.md Rule 5** (inconsistent sibling pattern against `declarations` and `references`, which use path-ordered `std::equal_range`). |
| **E5** | `Index.cpp:89`, `Processor.cpp:50, 641, 694-696, 916` | `Index::getOrCreatePathIndex`, `Processor::getUnitHash`, and `UnitsJob::getOrCreatePathIndex` call `jam::Strings::indexOf` (`$O(N)$` linear string scan). `Processor::addIndexJob` copies every non-owned path from `index.unitRows` into each `IndexJob::foreignPaths`, and `IndexJob::isForeign` normalizes paths via `juce::File` and scans `foreignPaths` linearly (`$O(N)$`) on every included header. | **L — Lean** and **E — Complete at Creation**: copies ownership state per worker and performs `$O(U \cdot D \cdot P)$` string scans. |
| **E6** | `LINGModel.cpp:177-178`, `Manifest.cpp:173-199` | `LINGModel::getSwapFile` nests swap files under `getProjectName (manifest, toolchainArgument)` even inside shared `Id::roots` (`../jam/.cache/ling`, `../../JUCE/.cache/ling/<patchHash>`). Syntax `.ling` swap files (`ParseJob`) depend solely on `(sourceHash, languageHash, family)` and contain zero project or toolchain state, yet every project and `--debug`/Release variant duplicates and re-parses them. | **S — SSOT**: fragments identical syntax cache entries by unrelated manifest/toolchain keys. |
| **E7** | `Processor.cpp:10-13`, `LINGModel.h:43` | `getPoolOptions` sets `juce::Thread::Priority::background` with `defaultThreads { 4 }` and default OS secondary thread stack size (512 KiB on macOS), forcing language libraries with deep recursive parsers to spawn their own threads with larger stacks. | **B — Bound**: worker pool does not size thread stacks for recursive AST parsers. |

### 2.2 `cLang` Language Library Violations (`language/cLang/language.cast`)

| ID | Block in `language.cast` | Factual Mechanism | BLESSED / CODING Violation |
| :--- | :--- | :--- | :--- |
| **C1** | `index`, `IndexSession::setLoaded` | `CLanguageDocument::index` calls `session.libclang->executeOnThread (..., defaultStackSize)` (`clang_executeOnThread`). This spawns a brand-new OS thread at **default normal priority** on Apple Silicon Performance (P) cores for every unit, bypassing `Processor::pool`'s `Priority::background`. `IndexSession::setLoaded` also omits `clang_CXIndex_setGlobalOptions` (`CXGlobalOpt_ThreadBackgroundPriorityForAll`), so `libclang`'s internal crash-recovery threads also run on P-cores. | **B — Bound** (*"Threads are bound to their context"*): escapes the daemon's background worker pool onto foreground OS scheduling classes. |
| **C2** | `onCursor`, `ppIncludedFile`, `onMacro`, `addMacroCompletion`, `reportOutline` | `ppIncludedFile` registers **every** included file (including OS SDKs, libc++, and foreign headers) in `preprocessedFiles`. In `onCursor` (`addCallPassings`), `if (not context.isForeign (location))` guards `addCursorRecord`, but `onCursor` then **unconditionally returns `ChildVisitResult::recurse`**, walking every AST node of the C++ STL, OS SDK, JUCE, and JAM on every unit. `onMacro` and `addMacroCompletion` lack `isInSystemHeader` and `isForeign` checks, allocating USRs and global completions for SDK internals. `reportOutline` calls `libclang.tokenize` and `getNormalPath` before checking `not context.isForeign (file)` and `context.keptRanges.isEmpty()`. | **L — Lean**: performs `$O(\text{Total AST})$` work on foreign and system subtrees whose outputs are discarded. |
| **C3** | `indexSource`, `addContextPasses`, `getContextCommandLine` | `addContextPasses` provides `ling`'s cross-platform and inactive-`#ifdef` accuracy. However, non-host platform passes (`platformWindows`, `platformLinux`) pass `keepPrecompiled = false`, stripping `-include-pch` and reparsing all of JUCE and JAM from raw source without target OS SDK headers whenever a unit contains a single `#ifdef _WIN32` (`Main.cpp:12`) or `#if JUCE_WINDOWS` (`LINGModel.cpp:57`). Furthermore, during `addContextPass`, `onCursor` traverses the entire translation unit instead of pruning cursors outside `context.keptRanges`. | **L — Lean**: reparses entire module closures without PCH to inspect small conditional ranges. |
| **C4** | `addUnits`, `UnitsJob::addUnit` | With CMake's `-MD` flag, `ninja -t deps` emits thousands of system SDK headers (`/Library/Developer/.../SDKs/MacOSX.sdk/...`). `UnitsJob::addUnit` interns all `.h` SDK headers as `Id::file` rows in `model.state`, causing `LINGModel` (`LINGModel.cpp:195-204`) to attach directory watchers to OS SDK folders and `Processor::addJobs` to run `ParseJob` and `.ling` swap writes on immutable OS SDK headers. | **B — Bound** and **L — Lean**: watches and syntax-parses external OS SDK installations outside `Id::roots`. |

---

## 3. Algorithmic Foundations

Three established computer science mechanisms resolve every violation above without heuristics or language coupling:

1. **Demand-Driven Incremental Computation (*Adapton*, Hammer et al., PLDI 2014):**
   Separate **dirty propagation** from **evaluation**. When a source file changes on disk, run only the sub-millisecond per-file syntax `ParseJob`, unbind any `SymbolDocument` whose transitive unit input hash (`Processor::getUnitHash`) no longer matches, and set `Id::isIndexed = false` on affected rows. Do **not** schedule `IndexJob` runs on disk edits. Materialise `IndexJob` runs strictly on demand when:
   - An `Id::request` arrives: schedule only the unindexed rows in `Processor::getUnitRows (request)` (and their `Id::prerequisites`), governed by `map::Readiness` (`Id::file`, `Id::dependencies`, or `Id::files`).
   - A file has an active editor buffer in `Id::buffers`: schedule only its single owning unit row so LSP `publishDiagnostics` remains live while editing.
2. **Task Graph Minimality (*Build Systems à la Carte*, Mokhov, Mitchell, Peyton Jones, JFP 2020):**
   Key unit artifacts by lifetime-stable `pathIndex` and transitive input hash (`getUnitHash`). Across manifest or compilation-database reloads (`UnitsJob`), retain every `jam::Document` and bound `SymbolDocument` whose `(unitPathIndex, hash, languageHash, family)` and `unitRows` file ownership remain invariant. A unit executes `language->index` at most once per unique transitive input hash.
3. **Complete at Creation (`MANIFESTO.md` E):**
   Sort per-file `SymbolRecord` slices by `(line, column)` on the worker thread inside `SymbolDocument::Builder::sortAndDeduplicate` before serializing `.lingSymbols`. Downstream consumers binary-search the memory-mapped `Span<SymbolRecord>` directly in place with zero transit allocations, zero pointer tables, and zero message-thread sorting.

---

## 4. Specification — Part I: Universal Engine (`Source/`)

### 4.1 Demand-Driven `IndexJob` Materialisation (`Source/Processor.h`, `Source/Processor.cpp`)

1. **Disk-edit invalidation without speculative indexing (`Processor::addDocument`):**
   - When `job.buffer.has_value()` is `true`: retain the existing single-owner `addIndexJob (unit, unitRow)` path (`Processor.cpp:789-798`) so open LSP buffers receive live `publishDiagnostics`.
   - When `job.buffer.has_value()` is `false` (disk change or initial `ParseJob`):
     - Do **not** loop over all units calling `addIndexJob` (`Processor.cpp:799-808`), unless a unit already carries `Id::isRequested == true` (meaning a pending client request in `Id::connections` is actively waiting on that unit) or has an open buffer in `Id::buffers`.
     - For any dependent unit whose bound `SymbolDocument` in `index.symbolDocuments` no longer matches `getUnitHash`, unbind that stale `SymbolDocument` (so zero stale symbols remain in `Index`) and call `setUnitIndexed (unit, unitIndex)` so its owned rows transition to `Id::isIndexed = false`.
2. **Request-driven scheduling (`Processor::registerEvents`, `Processor::addIndexJobs`):**
   - Extend `addIndexJobs (const juce::ValueTree& request)` (currently limited to `getFailedRows (request)`, `Processor.cpp:728-748`) or pair it with request scheduling in `childEvents[Id::request]` (`Processor.cpp:184-193`) so that for every `unitRow` in `getUnitRows (request)` (and transitive `Id::prerequisites` of `unitRow`):
     - If `unitRow` is unindexed (`not static_cast<bool> (unit[Id::isIndexed])`) or failed (`unit.hasProperty (Id::message)`):
       - Mark `unit.setProperty (Id::isRequested, true, nullptr)`.
       - If `not hasParseJob (unit, unitRow, -1)` and `index.languages.at (family)->hasIndex()`, queue `addIndexJob (unit, unitRow)` when not already running/queued for the current hash, then dispatch via `addIndexJobs()` and `moveRequestJobsToFront (request)`.
       - If `hasParseJob` is `true`, `moveRequestJobsToFront (request)` prioritizes the blocking `ParseJob`s, and `addDocument` sees `unit[Id::isRequested] == true` and schedules `addIndexJob` the instant the final dependency `ParseJob` finishes.
3. **Exact global readiness invariant (`Processor::setIndexed ()`):**
   - Update `Processor::setIndexed ()` (`Processor.cpp:380-391`) so `files[Id::isIndexed]` is `true` if and only if `toolchain` is valid, `unitsJobs` and `nextUnitsJobs` are empty, and **every child row of `files` has `static_cast<bool> (row[Id::isIndexed]) == true`**.
   - Keep `jobCount` and `pendingJobCount` (`Processor.cpp:384-391`) counting only active/queued jobs (`parseJobs`, `nextParseJobs`, `indexJobs`, `nextIndexJobs`, `unitsJobs`, `nextUnitsJobs`), so LSP `$/progress` (`Server.cpp:31-41`) cleanly emits `Begin -> Report -> End` for each demanded batch and never hangs while unrequested units sit idle.
4. **Profiler build compatibility (`Source/Main.cpp`):**
   - Under `#elif JAM_USING_PROFILER` (`Main.cpp:323-332`), `Main` runs headless without client connections and waits on `Id::documents` (`Main.cpp:367`) to benchmark full indexing. Ensure profiler runs request workspace-wide indexing (for example by appending an `Id::files`-scope request or setting `Id::isRequested` on discovered units in profiler mode) so `JAM_USING_PROFILER` continues to profile the full `ParseJob` + `IndexJob` pipeline deterministically.

### 4.2 Lifetime-Stable `Index` Identity and Incremental `Id::files` Retention (`Source/Index.h`, `Source/Index.cpp`, `Source/Processor.cpp`)

1. **Key `symbolDocuments` and `symbolFiles` by stable `unitPathIndex`:**
   - `Index::paths` never removes entries (`Index.h:395`), making `pathIndex` a permanent integer identity for the lifetime of `Index`.
   - Re-key `Index::symbolDocuments` (`Index.h:358`) from `rowIndex` to `unitPathIndex`, and store `(unitPathIndex, const SymbolDocument::FileEntry*)` in `Index::symbolFiles` (`Index.h:361`).
2. **Retain valid `jam::Document` and `SymbolDocument` state across `addJobs (files)`:**
   - In `Index::removeDocuments (const juce::ValueTree& files)` (`Index.cpp:7-27`), unbind and erase only `symbolDocuments` whose unit path was removed from `files` or whose owned file set in `unitRows` changed after `addUnitRows (files)`.
   - In `Processor::addJobs (const juce::ValueTree& files)` (`Processor.cpp:439-461`), skip spawning a `ParseJob` for any row whose path has no open buffer, already holds a valid `jam::Document` in `index.documents`, and whose file on disk matches the stored document state (or let `ParseJob` validate swap hashes while preserving bound `SymbolDocument` entries in `index.symbolDocuments` whenever `symbolDocument->getHeader().hash == getUnitHash (...)`).

### 4.3 Zero-Copy Sorted `SymbolDocument` and `$O(1)$` Lookups (`Source/SymbolDocument.h`, `Source/SymbolDocument.cpp`, `Source/Index.h`, `Source/Index.cpp`)

1. **Sort `FileBuilder::symbols` at creation (`SymbolDocument` format v2):**
   - Increment `SymbolDocument::currentFormatVersion` from `1` to `2` (`SymbolDocument.h:425`) so existing unsorted swap files fail `isValidBuffer` cleanly and regenerate once.
   - In `SymbolDocument::Builder::sortAndDeduplicate ()` (`SymbolDocument.cpp:225-242`), `std::stable_sort` each `file.symbols` array by `(line, column)` (`Index::isRecordBefore` order).
   - Remove `Index::records` (`Index.h:364`) and `Index::addRecords` (`Index.cpp:323-339`) after verifying all readers in `Query.cpp`. Update `Index::applyToRecords` (`Index.h:220-248`) to binary-search `symbolDocument.getSymbols (*file)` directly over the memory-mapped `Span<SymbolRecord>`.
2. **Eliminate quadratic `$O(M \cdot C)$` completion unbinding (`Index.cpp:246-267`):**
   - Deduplicate `scopeSymbol` via a `jam::HashSet<juce::int64>` inside `Index::removeCompletions` (matching `removedSymbols` in `Index::removeSymbols`), and maintain `completions` slices ordered by `path` byte order (matching `declarations` and `references`) so unbinding removes the contiguous `pathIndex` range via binary search (`std::equal_range` + `removeRange`).
3. **`$O(1)$` path indexing and shared foreign-unit lookup:**
   - Add a `jam::HashMap<juce::String, int>` path-to-index map inside `Index` (`Index::getOrCreatePathIndex`, `Index.cpp:87`) and inside `Processor::UnitsJob` (`Processor.cpp:912`), eliminating `$O(N)$` `paths.indexOf` scans in `UnitsJob`, `Index::addBindings`, and `Processor::getUnitHash`.
   - Replace per-job `jam::Strings foreignPaths` copies (`Processor.cpp:694-696`) and `$O(N)$` `juce::File` normalizations in `IndexJob::isForeign` (`Processor.cpp:639-642`) with a shared immutable lookup table built once in `Index::addUnitRows` (`std::shared_ptr<const jam::HashMap<juce::String, int>>`) mapping normalized path strings to owning unit indices.
4. **Filter external system paths out of `Id::files` (`Processor::UnitsJob`):**
   - Ensure dependency paths outside configured `Id::roots` (such as OS SDK headers emitted by `-MD` build graphs) are not added as watched `Id::file` rows in `model.state` (`LINGModel.cpp:195-204`) or scheduled for `ParseJob` swap generation (`Processor.cpp:451`).

### 4.4 Cross-Project Syntax Swap Sharing and Worker Pool Sizing (`Source/LINGModel.h`, `Source/LINGModel.cpp`, `Source/Processor.cpp`, `Source/HELP.md`)

1. **Share `.ling` syntax swaps across projects and toolchains (`LINGModel::getSwapFile`):**
   - In `LINGModel::getSwapFile` (`LINGModel.cpp:177-178`), when a file resolves under an `Id::root` (`project`, `userModules`, or `patched` JUCE) and `extension` is `files::lingSwap`, omit `getProjectName (manifest, toolchainArgument)` from the swap path because `jam::Document` swap validation stamps `(sourceHash, languageHash, family)` (`Index.cpp:36`) and has zero dependency on manifest or compile flags. Keep `getProjectName` on `files::lingSymbols`, whose `getUnitHash` depends on per-project/toolchain compile arguments.
2. **Worker pool stack size and default thread count (`Processor.cpp:10-13`, `LINGModel.h:43`, `Source/HELP.md:277`):**
   - Configure `getPoolOptions` (`Processor.cpp:10-13`) with an explicit 8 MiB stack size (`.withThreadStackSizeBytes (8 << 20)`) alongside `juce::Thread::Priority::background`, enabling language libraries to execute deep recursive AST passes directly on `Processor::pool` threads.
   - Lower `LINGModel::defaultThreads` (`LINGModel.h:43`) from `4` to `2` (and update the `threads` default in `Source/HELP.md:277`), keeping `## daemon` `threads` overrides intact (`Manifest.cpp:129`).

---

## 5. Specification — Part II: `cLang` Language Library (`language/cLang/`)

All C++ and `libclang` changes reside strictly inside `language/cLang/` (`language.cast` and `construction.md`).

### 5.1 Efficiency-Core Thread Confinement (`index`, `IndexSession`, `LibClang`)

1. **Remove `libclang->executeOnThread` (`language.cast` `index`):**
   - With `Processor::pool` providing an 8 MiB stack at `juce::Thread::Priority::background`, invoke `indexSource` directly on the calling worker thread in `CLanguageDocument::index` instead of spawning an unthrottled default-priority OS thread via `executeOnThread`.
2. **Set `libclang` background global options (`language.cast` `LibClang`, `IndexSession::setLoaded`, `construction.md`):**
   - Bind `clang_CXIndex_setGlobalOptions` in `LibClang` (`construction.md` / `language.cast`) and invoke it on `indexHandle` in `IndexSession::setLoaded` with `CXGlobalOpt_ThreadBackgroundPriorityForAll` (`0x3`), confining any internal `libclang` crash-recovery threads to OS background QoS (`PRIO_DARWIN_BG` on macOS, background priority on Windows).

### 5.2 AST, Macro, and System-Header Pruning (`language.cast`)

1. **Exclude system headers from `preprocessedFiles` and AST cursor walks:**
   - Guard `ppIncludedFile` (`getIndexerCallbacks`), `onMacro`, `addMacroCompletion`, `onReference`, and `addDiagnostics` with `libclang.isInSystemHeader (...) == 0` (matching `addOutline` and `getSkippedRanges`), after verifying in `Source/Query.cpp` whether `goToDefinition` on system declarations requires `onDeclaration` records or resolves via `HoverEntry` definition spans.
2. **Fix unconditional recursion in `onCursor` (`addCallPassings`):**
   - In `onCursor`:
     - When `libclang.isInSystemHeader (location) != 0`, return `ChildVisitResult::visitContinue`.
     - When `context.isForeign (location)` is `true` and `cursor.type != CursorType::cursorNamespace` (preserving `namespace juce { #include ... }` unity wrappers), return `ChildVisitResult::visitContinue` instead of recursing into foreign classes, functions, templates, and statements.
     - When `not context.keptRanges.isEmpty()` (during `addContextPasses`), skip recursing into any non-namespace cursor whose `getCursorExtent` does not overlap a kept range of its file.
3. **Move cheap guards before `libclang` allocations and tokenization:**
   - In `reportOutline`: test `context.callbacks.addOutline != nullptr and not context.isForeign (file) and context.keptRanges.isEmpty()` **before** calling `addPreprocessedFile`, `getOutlineNameLength` (`libclang.tokenize`), and `libclang.getFileName` / `getNormalPath`.
   - In `onMacro` and `addMacroCompletion`: test `not context.isForeign (location)` and `context.keptRanges` before calling `libclang.getCursorUSR` and `libclang.getCursorSpelling`, and pass the already-computed USR/spelling from `onMacro` into `addMacroCompletion` instead of calling `getCursorUSR` and `getCursorSpelling` a second time.

### 5.3 Conditional `#ifdef` Context Pass Optimization (`addContextPasses`)

1. **Preserve full JUCE conditional accuracy while eliminating redundant non-PCH reparses:**
   - Profile `addContextPasses` under `JAM_USING_PROFILER` across `ling` and JUCE/JAM units.
   - Ensure `addSkippedDirectives` records `platformMask` bits only for skipped ranges whose body contains code/declarations (ignoring empty `#ifdef` guards or header-include-only guards whose target OS SDK headers do not exist on the host machine), and verify in `construction.md` (`platformMacros`, `platformTargets`) whether project-level platform macros (`JUCE_WINDOWS`, `JUCE_LINUX`, `JUCE_MAC`) can be evaluated under `keepPrecompiled = true` (`-fno-validate-pch`) without stripping `-include-pch`.

---

## 6. Mandatory Pre-Plan Reads for COUNSELOR

Per `CODING.md` (*"Read the owner first"* and *"No identifier latitude in delegation"*), COUNSELOR must read the following owners before ratifying method signatures, identifiers, and task prompts in `PLAN.md`:

1. `carol/SPRINT-LOG.md` — Sprint 1 through Sprint 19 decisions, especially Sprint 19's failed-unit retry, single-owner `libclang`, and swap write contracts.
2. `Source/Query.h` and `Source/Query.cpp` — Verify every field of `Index` read by `Query` (specifically `index.records`, `index.symbolDocuments`, `index.symbolFiles`, `index.declarations`, and system-header `goToDefinition` behavior).
3. `Source/Connection.h`, `Source/Connection.cpp`, `Source/Client.cpp`, and `Source/ToolClient.cpp` — Verify request lifecycle, `Id::cancel` handling, and `Id::isRequested` cleanup when a client cancels or disconnects.
4. `language/cLang/construction.md` and `language/cLang/spell.md` — Verify `LibClang` function table schema, `platformMacros`, `platformTargets`, and `ToolchainWord` entries.
5. `cast/identifiers.md` — Verify existing `Id::` vocabulary before using or adding any identifier.
6. `../jam/jam_syntax_tree/document/jam_LanguageInterface.h` and `../jam/jam_core` (`jam::Strings`, `jam::HashMap`, `jam::File::Watcher`, and JUCE's `juce_ThreadPool.h`) — Verify exact container methods and `juce::ThreadPool::Options::withThreadStackSizeBytes` signature.
7. `profile-info.md` — Verify the `JAM_USING_PROFILER` configuration and benchmark harness.

---

## 7. Acceptance Criteria and Verification Plan

1. **Schema & Determinism Suite (`LING_USING_SCHEMA`):**
   - 100% pass rate on `SchemaSuite` and `StdioClientProbe` (`Main.cpp:312-322`), including failed-unit retry (`-32803`), `publishDiagnostics` on open buffers, and cross-`#ifdef` symbol queries.
2. **Parallel Multi-Agent Stress Verification:**
   - Simulate 3 concurrent MCP clients editing shared headers/sources on disk interleaved with `hover`, `diagnostics`, `goToDefinition`, and `findReferences` calls:
     - Disk edits with zero queries trigger `ParseJob` only and **zero** `IndexJob` executions.
     - `Id::file` queries index **only** the 1 target unit (plus unindexed prerequisites).
     - `ninja` builds updating `.ninja_deps` wipe **zero** unchanged `SymbolDocument` bindings in RAM.
3. **Thermal & Profiler Verification (`JAM_USING_PROFILER`):**
   - Zero `clang_executeOnThread` calls; all worker and `libclang` threads verified running at background QoS (`PRIO_DARWIN_BG` on macOS).
   - Measurable wall-clock and CPU-time reduction in `runIndexJob` and `addSymbols` profiler zones on both cold and warm runs, with `SymbolDocument` v2 eliminating `Index::records` allocations and message-thread sorting.