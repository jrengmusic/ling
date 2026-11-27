# PLAN: Demand-Driven Indexing, Resource Confinement, and Efficiency

**RFC:** `rfc_demand_driven_indexing_and_efficiency.md` (verified and revised against codebase, `jam_core`/`juce_core` inventory, and `libclang` runtime probes)
**Date:** 2026-11-26
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE / JAM (`~/.carol/LANGUAGE.md`, `~/.carol/CODING.md`)

---

## Overview

This plan removes CPU thermal spikes, multi-agent re-index thrashing, multi-gigabyte RAM bloat, and 31.7 GB duplicate disk caches in `ling` without reducing query speed or accuracy. It enforces MANIFESTO **E** (Complete at Creation, 0 rules in transit, Materialisation on demand), **B** (Bound thread confinement and root-scoped watchers), **S** (SSOT swap sharing and lifetime-stable identity), and **L** (pruned AST/macro/outline walks and bounded document residency) across four incremental parts. Every verified RFC claim maps to a step; three RFC proposals disproved by direct codebase and `libclang` probes (E5 `jam::Strings::indexOf`, C1 `CXGlobalOpt_ThreadBackgroundPriorityForAll`, C3 cross-platform `-fno-validate-pch`) are explicitly corrected or descoped below.

---

## Language / Framework Constraints

- **One object, one translation unit (`LANGUAGE.md`, `CODING.md`):** Templates and `constexpr` stay in headers; every non-template member body stays in the object's single `.cpp`. Never split one class across multiple `.cpp` files.
- **Lean 300/30/3 (`MANIFESTO.md` L, `LANGUAGE.md`):** 300 lines/file is a smell detector for responsibility count; 30 lines/function and 3 branches max apply unchanged. Replace chains of more than 3 branches with lookup tables (`jam::HashMap`, `jam::LookupTable`, `jam::Function::Map`).
- **0 rules on data in transit (`MANIFESTO.md` E):** Zero allocation, zero copy, zero mutation, zero temporary container, zero fake-carrier struct, zero shadow state between creation and query use.
- **Framework-first (`CODING.md` CRITICAL RULES):** Use `jam_core` APIs first, then `juce_core` as fallback. Hand-rolling is allowed only for proven absences in Layer 5 below.
- **No bail-out guards (`CODING.md` CRITICAL RULES):** Use `jassert`/`assert` for preconditions and positive nesting for conditions; result returns are permitted when the return carries the function's computed value.
- **Alternative tokens & style (`CODING.md`):** Use `not`, `and`, `or` in C++ code (keep `!`, `&&`, `||` in `#if`/`#elif`); brace initialization except same-container copies (`auto x (getX())`); `.at()` for indexed access; structured bindings with descriptive names (never `.first`/`.second`, `_`, `it`); no manual cleanup in destructors; no `==`/`!=` on strings; no comments or Doxygen blocks until the dedicated post-audit pass.
- **Universal Engine Boundary (`Source/`):** Zero `clang`, `libclang`, `ninja`, or C++ specifics in `Source/`. All compiler and AST logic stays inside `language/cLang/`.

---

## Framework Inventory

### 1. API (`jam_core` first, then `juce_core`)

| Need | Module | API | Source | Step |
| :--- | :--- | :--- | :--- | :--- |
| O(1) interned string lookup & zero-copy view | `jam_core` | `jam::Strings::indexOf (StringRef, false, 0)`, `contains (StringRef, false)`, `getView (int)`, `add (std::string_view)` | `jam_core/text/jam_Strings.h:224, 329-348, 578` | 1.3, 1.4 |
| Dense & segmented hash maps / sets | `jam_core` | `jam::HashMap<K, V>`, `jam::HashSet<K>`, `jam::SegmentedHashMap<K, V>`, `jam::SegmentedHashSet<K>` (`addOrReplace`, `try_emplace`, `find`, `contains`, `erase`, `at`, `reserve`; transparent lookup when both `HashFn::is_transparent` and `KeyEqual::is_transparent` exist) | `jam_core/utils/jam_HashMap.h:858-867, 2692-2795, 3542-3589, 3736-3753, 3895-3950` | 1.2, 1.3, 1.4, 2.4, 4.1 |
| Owning heterogeneous / homogeneous containers | `jam_core` | `jam::Owner<T>` (optional O(1) dedup), `jam::Array<T>` (`ensureStorageAllocated`, `insertArray`, `removeRange`, `clear`, `sort`) | `jam_core/utilities/jam_Owner.h:44-500`, `jam_core/utils/jam_Array.h:184-222, 490-624` | 1.1, 1.2, 4.1 |
| Process-wide shared singletons / instance chains | `jam_core` | `jam::SharedInstance<T>` (type-keyed `weak_ptr` join-or-create), `jam::Instance<T>` (LIFO atomic chain) | `jam_core/utils/jam_SharedInstance.h:17-86`, `jam_core/utils/jam_Instance.h:46-126` | 4.1, 4.2 |
| Event dispatch & coalesced async updates | `jam_core` | `jam::Function::Map<K, Sig>`, `jam::AsyncUpdater`, `jam::Mailbox<T>` | `jam_core/function_map/jam_Function.h:96-332`, `jam_core/concurrency/jam_AsyncUpdater.h:21-275`, `jam_core/concurrency/jam_Mailbox.h:24-63` | 2.3, 4.2 |
| Wyhash & avalanching hash functors | `jam_core` | `jam::Wyhash::hashBytes`, `jam::Wyhash::fold`, `jam::Hash<T>` (`std::string_view` supported; `juce::String` lacks `is_transparent`) | `jam_core/utils/jam_HashMap.h:81-293, 305-665`, `jam_core/utilities/jam_Hash.h:41-83` | 1.4, 2.2, 2.4, 4.1 |
| File system watcher & OS file primitives | `jam_core` | `jam::File::Watcher` (`addFolder`, `coalesceEvents`, `addListener`), `jam::File` (`replaceFileIn`, `createDirectory`, `deleteFile`) | `jam_core/file/jam_FileWatcher.h:25-162`, `jam_core/file/jam_File.h:9-354` | 2.1, 2.3, 4.1 |
| Binary codec | `jam_core` | `jam::BinaryCodec` (`writeUint16/32/64`, `readUint16/32/64`, `writeString`, `readString`) | `jam_core/binary_codec/jam_BinaryCodec.h:16-61` | 1.1 |
| Language library ABI callbacks | `jam_syntax_tree` | `jam::LanguageCallbacks` (`isForeign`, `shouldExit`, `addSymbol`, `addHover`, `addOutline`, `addCompletion`, `addUnit`, `addBuildFile`) | `jam_syntax_tree/document/jam_LanguageInterface.h:66-87` | 1.4, 2.1, 3.2 |
| Zero-copy file mapping (full & partial range) | `juce_core` | `juce::MemoryMappedFile (file, readOnly)` and `juce::MemoryMappedFile (file, Range<int64>, readOnly)` | `JUCE/modules/juce_core/files/juce_MemoryMappedFile.h:44-125` | 1.1, 2.4 |
| Root containment & atomic swap file write | `juce_core` | `juce::File::isAChildOf`, `getRelativePathFrom`, `getFullPathName`, `juce::TemporaryFile::overwriteTargetFileWithTemporary` | `JUCE/modules/juce_core/files/juce_File.h:251-305`, `juce_TemporaryFile.h:76-198` | 2.1, 2.2 |
| Thread pool with explicit stack size & background QoS | `juce_core` | `juce::ThreadPool::Options::withNumberOfThreads`, `withThreadStackSizeBytes (size_t)`, `withDesiredThreadPriority (Thread::Priority::background)` | `JUCE/modules/juce_core/threads/juce_ThreadPool.h:166-198`, `juce_Threads_mac.mm:47-59, 131-135`, `juce_SharedCode_posix.h:864-868` | 3.1, 4.2 |
| Inter-process daemon lock & TCP server | `juce_core` / `juce_events` | `juce::InterProcessLock`, `juce::InterprocessConnectionServer`, `juce::InterprocessConnection` | `Source/Main.cpp:421`, `Source/Server.h:19`, `Source/Client.cpp:3-52` | 4.1, 4.2 |
| Project identifier vocabulary | `ling` | `Id::isIndexed`, `Id::isRequested`, `Id::prerequisites`, `Id::dependencies`, `Id::roots`, `Id::buffers`, `Id::cancel`, `Id::request`, `Id::message`, `Id::timeout`, `Id::threads`, new `Id::coalesce` | `cast/identifiers.md:26, 62, 64, 86, 94, 119-120` | 2.1, 2.3, 4.1 |

### 2. Pattern usage

| Pattern | Exemplar | Use sites | Step |
| :--- | :--- | :--- | :--- |
| Path-sorted occurrence range removal via binary search | `Index::getPathRange` (`Source/Index.cpp:420-433`) + `Index::removeOccurrences` (`Source/Index.cpp:344-358`) | `Index::removeSymbols` (`Index.cpp:336-337`), `Index::addOccurrences` (`Index.cpp:404`) | 1.2 |
| Deduplicated set before unbinding per file | `jam::HashSet<juce::int64> removedSymbols` (`Source/Index.cpp:328-339`) | `Index::removeSymbols` (`Index.cpp:328`) | 1.2 |
| Binary-search over sorted per-file `SymbolRecord` slice | `Index::applyToRecords` (`Source/Index.h:239-265`) | `Query.h:379, 384, 463, 472, 481, 721, 730, 782`, `Query.cpp:789` | 1.1 |
| `HoverEntry` definition span read | `Query::hover` (`Source/Query.cpp:449-465`) | `Query::goToDefinition` (`Source/Query.cpp:338-351`) | 3.2 |
| `## daemon` manifest config rows | `Manifest::getDaemonValue` (`Source/Manifest.cpp:134-135, 172-173, 181-188`) | `Id::timeout`, `Id::threads`, new `Id::coalesce` | 2.3 |
| Model-driven event dispatch via `jam::Function::Map` | `Processor::registerEvents` (`Source/Processor.cpp:114-210`) | `events`, `bufferEvents`, `propertyEvents`, `childEvents` (`Processor.cpp:116-209`) | 2.3 |
| Request readiness scope table | `map::Readiness` + `Processor::getScope` (`Source/Processor.cpp:212-227`) + `Processor::isRequestReady` (`Source/Processor.cpp:292-317`) | `Source/Main.cpp:302`, `Source/Processor.cpp:214, 232, 307` | 2.3 |
| Root longest-prefix matching for swap paths | `LINGModel::getSwapFile` (`Source/LINGModel.cpp:159-187`) | `Processor::addJob` (`Processor.cpp:1076`), `Processor::addIndexJob` (`Processor.cpp:1119`) | 2.1, 2.2 |
| `language.cast` table-driven generation | `language/cLang/construction.md:403-517`, `language/cLang/spell.md:59-63` | `LibClang` (`language.cast:61-69, 440-464`), `platformMacros` (`language.cast:5925`) | 3.1, 3.2, 3.3 |

### 3. Names

| Family | Members | Intent | Step |
| :--- | :--- | :--- | :--- |
| `Index` binding lifecycle | `addBindings`, `removeBindings`, `addSymbols`, `removeSymbols`, `addCompletions`, `removeCompletions`, `removeDocuments` (`Source/Index.h:59, 88, 104, 438-449`) | Mutate `Index` binding tables consistently per unit or path. | 1.1, 1.2, 1.3 |
| `Index` read/traversal | `applyToDocuments`, `applyToRecords`, `applyToOccurrences`, `getSymbolDocument`, `getElement`, `getPathRange` (`Source/Index.h:40, 132, 141, 170, 178, 239, 276`) | Read-only callable traversal (`apply*`) or direct retrieval (`get*`). | 1.1, 1.3, 2.4 |
| `Processor` job scheduling | `addJob`, `addJobs`, `addIndexJob`, `addIndexJobs`, `addUnitsJob`, `setIndexed`, `setUnitIndexed`, `moveRequestJobsToFront` (`Source/Processor.cpp:319, 406, 424, 447, 620, 1073, 1088, 1124, 1173, 1297`) | Enqueue work (`add*`), write state (`set*`), prioritize (`move*`). | 2.3, 3.1 |
| `LINGModel` daemon defaults | `defaultTimeoutMs`, `defaultThreads`, `coalesceMs` (`Source/LINGModel.h:40-46`), `Id::timeout`, `Id::threads`, `Id::coalesce` | Default values and manifest keys for `## daemon` configuration. | 2.3, 3.1 |
| `SymbolDocument` format & builder | `currentFormatVersion`, `sortAndDeduplicate`, `writeSwapFile`, `open`, `getSymbols`, `getCompletions`, `getHovers` (`Source/SymbolDocument.h:360, 372, 417, 428, 444, 452, 499`) | Build once, sort at creation, map read-only, slice in place. | 1.1, 3.2 |
| `cLang` visitor & context filters | `isForeign`, `isKept`, `getKeptPosition`, `isPreprocessed`, `addPreprocessedFile`, `onCursor`, `onMacro`, `reportOutline`, `addSkippedDirectives` (`language/cLang/language.cast:3091-3183, 3821, 4930, 5514, 5903`) | Guard and filter AST/preprocessor callbacks before libclang allocations. | 3.2, 3.3 |
| `Server` / `Client` daemon identity | `LINGModel::getDaemonName`, `LINGModel::getProjectName`, `Server::getPortFile`, `Client::connectToDaemon` (`Source/LINGModel.cpp:29, 41`, `Source/Server.cpp:63`, `Source/Client.cpp:41`) | Locate and attach to a running daemon or spawn one. | 4.1, 4.2 |

### 4. Synthesis

| Rule | Established form | Exemplar |
| :--- | :--- | :--- |
| "No bail-out guards; positive nesting" | Nest positive conditions; return early only when the return value is the computed result. | `Index::getOrCreatePathIndex` (`Source/Index.cpp:102-117`), `Index::getElement` (`Source/Index.cpp:228-235`) |
| "Structured bindings for all pair/tuple results" | Chain `const auto& [k, v] { *entry };` with descriptive names; never `.first`/`.second`. | `Index::addBindings` (`Source/Index.cpp:138-145`), `Processor::getUnit` (`Source/Processor.cpp:77-82`) |
| "Use `.at()` for container access" | `container.at (index)` on `jam::Array`, `jam::HashMap`, `jam::Strings`, `std::string`. | `Index::addSymbols` (`Source/Index.cpp:468`), `Processor::addIndexJobs` (`Source/Processor.cpp:1159-1161`) |
| "No `==` / `!=` on strings" | Use `juce::Identifier` equality or `string.compare (other) == 0`. | `Processor::addDocument` (`Source/Processor.cpp:1242`), `LINGModel::fileChanged` (`Source/LINGModel.cpp:228`) |
| "0 rules on data in transit" | Sort once inside `Builder::sortAndDeduplicate`; query `Span<T>` directly over mmap without intermediate arrays; use transparent lookup or `jam::Strings` to avoid temporary string allocations. | `SymbolDocument::getOutlines` / `getInlayHints` (`Source/SymbolDocument.cpp:257-273`, `Source/SymbolDocument.h:459-476`), `jam::Strings::indexOf` (`jam_Strings.h:331-348`) |
| "Use the framework API fully" | Reuse `jam::Strings::indexOf(..., 0)` (O(1)), `jam::SegmentedHashMap` for large growing tables, and `juce::File::isAChildOf` for root checks. | `jam_core/text/jam_Strings.h:331-348`, `jam_core/utils/jam_HashMap.h:3895`, `juce_core/files/juce_File.h:304` |

### 5. Absence (Proven across `jam_core` first, then `juce_core`)

| Need | Query | Result |
| :--- | :--- | :--- |
| Bounded LRU / eviction cache with explicit invalidation | Read `jam_core/jam_core.h:62-138`, `jam_SharedResources.h`, `jam_Document.h:1663-1696`, `JUCE/docs/xml/index.xml`, `juce_core/detail/juce_LruCache.h:39-115` | Absent in `jam_core` (`SharedResources` and `SharedDocuments` are unbounded, append-only). `juce::LruCache` is internal `detail/` (`@cond`), fixed compile-time size, mutex-locked `std::map`+`std::list`, and lacks `erase`/`contains`/pinned-entry support. A small bounded cache on `Index` using `jam::HashMap` + `jam::Array` is justified for Step 2.4. |
| `removeFolder` on `jam::File::Watcher` | Read `jam_core/file/jam_FileWatcher.h:25-162` | Absent — only `addFolder` exists. Filtering external SDK paths *before* `Id::files` insertion (Step 2.1) is mandatory. |
| Binary search / custom comparator sort on `jam::Array` | Read `jam_core/utils/jam_Array.h:583-624` | Absent — `jam::Array::sort()` uses `operator<` only and has no `lower_bound`/`equal_range`. Using `std::stable_sort` and `std::lower_bound`/`std::equal_range` on `jam::Array` / `Span` iterators is justified. |
| Header-only `.swap` hash peek in `jam::Document::SwapReader` | Read `jam_core/document/jam_Document.h:1001-1106` | Absent — `SwapReader` deserializes the full tree. Storing `sourceHashes` separately from resident `jam::Document` ASTs (Step 2.4) is justified. |
| Transparent `std::string_view` lookup on `jam::Hash<juce::String>` | Read `jam_core/utilities/jam_Hash.h:41-83`, `jam_core/utils/jam_HashMap.h:858-867` | Absent — `jam::Hash<juce::String>` lacks `is_transparent`. Step 1.4 uses `jam::Strings` + `jam::Array<int>` (or `std::string` with transparent `std::string_view` hasher/equal) for zero-allocation `isForeign(const char*)`. |
| `remove`/`clear` or many-to-one support on `jam::Bimap` | Read `jam_core/utils/jam_Bimap.h:129-233` | Absent — `jam::Bimap` is append-only strict 1-to-1 bijection (`jassert` on duplicates, no `erase`/`clear`). Unsuitable for mutable/many-to-one `unitRows` or `pathRows`. |
| `PRIO_DARWIN_BG` propagation in JUCE `Thread` | Read `JUCE/modules/juce_core/native/juce_Threads_mac.mm:47-59, 131-189` | Absent — JUCE maps `Thread::Priority::background` to `QOS_CLASS_BACKGROUND` only. Setting `LIBCLANG_NOTHREADS=1` keeps all parsing strictly inside JUCE's 8 MiB `QOS_CLASS_BACKGROUND` pool (MANIFESTO **B — Bound**). |
| Cross-platform PCH reuse with `-fno-validate-pch` | Live compiled probe against Xcode `libclang.dylib` (`Apple clang 17.0.0`) + `ASTReader.cpp:816-991` | Disproved — `-Xclang -fno-validate-pch` loads baked macOS AST without re-evaluating `#ifdef JUCE_WINDOWS`. Descoped from Step 3.3. |
| `O(N)` scan in `jam::Strings::indexOf(..., 0)` | Read `jam_core/text/jam_Strings.h:331-348` | Disproved — `startIndex == 0` is already O(1) hash-indexed. Duplicate path `HashMap` descoped from Step 1.4. |

---

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against `MANIFESTO.md` (BLESSED), `NAMES.md`, `~/.carol/CODING.md`, and the locked PLAN decisions (no deviation, no scope drift). `@Auditor` runs ONCE, after the final step, covering the whole sprint — never per step.

---

## Tracker

| Part | Step | Title | Target Files | Status |
| :--- | :--- | :--- | :--- | :--- |
| **Part 1** | **1.1** | Sort `SymbolRecord` at creation (`SymbolDocument` v2) and remove `Index::records` | `Source/SymbolDocument.h`, `Source/SymbolDocument.cpp`, `Source/Index.h`, `Source/Index.cpp` | `[x]` |
| **Part 1** | **1.2** | Deduplicate scopes and path-order `Index::completions` unbinding | `Source/Index.h`, `Source/Index.cpp` | `[x]` |
| **Part 1** | **1.3** | Lifetime-stable `unitPathIndex` keying and incremental `Index::removeDocuments` | `Source/Index.h`, `Source/Index.cpp`, `Source/Processor.cpp`, `Source/Query.h`, `Source/Query.cpp` | `[x]` |
| **Part 1** | **1.4** | Shared immutable zero-allocation foreign-unit ownership lookup | `Source/Index.h`, `Source/Index.cpp`, `Source/Processor.h`, `Source/Processor.cpp` | `[x]` |
| **Part 2** | **2.1** | Filter external paths outside `Id::roots` out of `Id::files` in `UnitsJob` | `Source/Processor.h`, `Source/Processor.cpp` | `[x]` |
| **Part 2** | **2.2** | Share `.swap` syntax caches across projects and toolchains inside `Id::roots` | `Source/LINGModel.cpp` | `[x]` |
| **Part 2** | **2.3** | Demand-driven `IndexJob` materialisation, configurable `## daemon` `coalesce`, `Id::isRequested` cleanup, and profiler compatibility | `cast/identifiers.md`, `Source/LINGModel.h`, `Source/LINGModel.cpp`, `Source/Manifest.h`, `Source/Manifest.cpp`, `Source/Processor.h`, `Source/Processor.cpp`, `Source/Connection.cpp`, `Source/Main.cpp`, `Source/HELP.md` | `[x]` |
| **Part 2** | **2.4** | On-demand bounded `Index::documents` loading and lightweight file-hash table | `Source/Index.h`, `Source/Index.cpp`, `Source/Processor.h`, `Source/Processor.cpp`, `Source/Query.h`, `Source/Query.cpp` | `[x]` |
| **Part 3** | **3.1** | Worker pool 8 MiB stack, `defaultThreads = 2`, and strict E-core `libclang` confinement (`LIBCLANG_NOTHREADS=1`) | `Source/Processor.cpp`, `Source/LINGModel.h`, `Source/HELP.md`, `language/cLang/language.cast` | `[x]` |
| **Part 3** | **3.2** | `cLang` AST, macro, outline, and `ppIncludedFile` pruning + `HoverEntry` fallback for OS SDK `goToDefinition` | `language/cLang/language.cast`, `Source/Query.cpp` | `[x]` |
| **Part 3** | **3.3** | Prune empty/include-only `#ifdef` platform passes and `keptRanges` cursor bounds | `language/cLang/language.cast` | `[x]` |
| **Part 4** | **4.1** | User-wide shared daemon identity and workspace registry ("Vulkan Instance" model) | `Source/LINGModel.h`, `Source/LINGModel.cpp`, `Source/Server.h`, `Source/Server.cpp`, `Source/Main.cpp` | `[x]` |
| **Part 4** | **4.2** | Shared worker pool across workspaces, client routing, and per-workspace idle eviction | `Source/Client.h`, `Source/Client.cpp`, `Source/Connection.h`, `Source/Connection.cpp`, `Source/ToolRelay.cpp`, `Source/Processor.h`, `Source/Processor.cpp` | `[x]` |
| **Verify** | **5.1** | Full build, `SchemaSuite` + `StdioClientProbe`, multi-agent stress, and `JAM_USING_PROFILER` verification | Build & test harness (ARCHITECT builds; Pathfinder verifies) | `[x]` |
| **Audit** | **5.2** | Single `@Auditor` sweep, Pathfinder Doxygen pass, and `/log` | All touched headers/sources | `[x]` |

---

## Steps

### Part 1 — Zero-Copy Complete-at-Creation `SymbolDocument` v2 & `Index` Efficiency

#### Step 1.1: Sort `SymbolRecord` at creation (`SymbolDocument` v2) and remove `Index::records`
- **Scope:** `Source/SymbolDocument.h`, `Source/SymbolDocument.cpp`, `Source/Index.h`, `Source/Index.cpp`
- **Action:**
  1. Increment `SymbolDocument::currentFormatVersion` from `1` to `2` (`Source/SymbolDocument.h:499`).
  2. In `SymbolDocument::Builder::sortAndDeduplicate()` (`Source/SymbolDocument.cpp:257-273`), `std::stable_sort` each `file.symbols` array by `std::make_pair (left.line, left.column) < std::make_pair (right.line, right.column)`.
  3. Delete `Index::records` (`Source/Index.h:425`) and `Index::addRecords` (`Source/Index.cpp:442-458`).
  4. Update `Index::applyToRecords` (`Source/Index.h:239-265`) to look up `symbolFiles.find (pathIndex)`, retrieve `symbolDocument.getSymbols (*file)`, and binary-search that `Span<SymbolRecord>` directly in place by `(line, column)`.
  5. Update `Index::addSymbols` (`Source/Index.cpp:460-490`) and `Index::removeSymbols` (`Source/Index.cpp:322-342`) to iterate `symbolDocument.getSymbols (file)` directly instead of `records.at (pathIndex)`.
- **Inventory:** `SymbolDocument::currentFormatVersion`, `SymbolDocument::Builder::sortAndDeduplicate`, `SymbolDocument::getSymbols`, `Index::applyToRecords`, `Index::addSymbols`, `Index::removeSymbols`. Absence: `jam::Array` has no custom-comparator sort or binary search. Synthesis: 0 rules on data in transit, structured bindings, positive nesting.
- **Validation:** Zero `Index::records` references remain; `applyToRecords` performs zero heap allocation and zero copy; `SymbolDocument::isValidBuffer` rejects v1 `.symbols` files cleanly; all `CODING.md` and `MANIFESTO.md` rules pass.

#### Step 1.2: Deduplicate scopes and path-order `Index::completions` unbinding
- **Scope:** `Source/Index.h`, `Source/Index.cpp`
- **Action:**
  1. Generalize `Index::getPathRange` (`Source/Index.h:170`, `Source/Index.cpp:420-433`) as a template over `jam::Array<std::pair<int, const Record*>>` so both `SymbolRecord` and `CompletionRecord` share one path-order binary search (DRY, MANIFESTO **S**).
  2. In `Index::addCompletions` (`Source/Index.cpp:384-393`), collect per-scope blocks and insert each block at `getPathRange (scopeCompletions, pathIndex).getStart()` (matching `Index::addOccurrences`, `Source/Index.cpp:395-406`).
  3. In `Index::removeCompletions` (`Source/Index.cpp:360-382`), deduplicate `completion.scope` using `jam::HashSet<juce::int64> removedScopes` (matching `removedSymbols` in `Index::removeSymbols`, `Source/Index.cpp:328-339`) and remove the contiguous slice via `getPathRange` + `jam::Array::removeRange` instead of `std::remove_if`.
- **Inventory:** `jam::HashSet`, `jam::Array::removeRange`, `jam::Array::insertArray`, `Index::getPathRange`, `Index::addCompletions`, `Index::removeCompletions`. Synthesis: structured bindings, DRY, nearest-sibling precedence (`NAMES.md` Rule 5).
- **Validation:** `removeCompletions` runs in `O(uniqueScopes * log C)` instead of `O(M * C)`; zero `std::remove_if` linear scans remain in `Index.cpp`; `Query::appendScopeCompletions` (`Query.cpp:703`) receives identical completion sets.

#### Step 1.3: Lifetime-stable `unitPathIndex` keying and incremental `Index::removeDocuments`
- **Scope:** `Source/Index.h`, `Source/Index.cpp`, `Source/Processor.cpp`, `Source/Query.h`, `Source/Query.cpp`
- **Action:**
  1. Re-key `Index::symbolDocuments` (`Source/Index.h:419`) from transient `rowIndex` to lifetime-stable `unitPathIndex` (`getOrCreatePathIndex (unitPath)`).
  2. Store `(unitPathIndex, const SymbolDocument::FileEntry*)` in `Index::symbolFiles` (`Source/Index.h:422`).
  3. Update `Index::addBindings`, `Index::removeBindings`, `Index::getSymbolDocument`, `Query::applyToSymbolFile` (`Query.h:565-574`), `Query::hover` (`Query.cpp:449`), and `Processor::setIndexed (int rowIndex)` (`Processor.cpp:418`) to key by `unitPathIndex`.
  4. In `Index::removeDocuments (const juce::ValueTree& files)` (`Source/Index.cpp:8-28`), preserve `symbolDocuments` entries whose unit path still exists in `files` as a compilation unit (with `Id::arguments`) and whose owned file set in `unitRows` matches the bound `symbolFiles`; unbind and erase only removed or ownership-changed units.
  5. Run `index.addPathRows (files)` and `index.addUnitRows (files)` before `index.removeDocuments (files)` in `Processor::addJobs` (`Source/Processor.cpp:620-625`) so ownership comparison uses the new `unitRows`.
- **Inventory:** `jam::Strings::indexOf(..., 0)`, `Index::paths`, `Index::getOrCreatePathIndex`, `Index::symbolDocuments`, `Index::symbolFiles`, `Index::removeDocuments`, `Index::addUnitRows`. Synthesis: SSOT, Complete at Creation, structured bindings.
- **Validation:** Reloading `Id::files` with unchanged units wipes zero valid `SymbolDocument` bindings; removed or re-owned units are cleanly unbound with zero dangling `FileEntry*` or `SymbolRecord*` pointers.

#### Step 1.4: Shared immutable zero-allocation foreign-unit ownership lookup
- **Scope:** `Source/Index.h`, `Source/Index.cpp`, `Source/Processor.h`, `Source/Processor.cpp`
- **Action:**
  1. In `Index::addUnitRows (const juce::ValueTree& files)` (`Source/Index.cpp:181-226`), publish an immutable shared ownership lookup (`std::shared_ptr<const ...>`) built once per `files` update — using `jam::Strings` (for zero-allocation O(1) `indexOf (juce::StringRef, false, 0)`) paired with a parallel `jam::Array<int>` of owning `unitRow` indices, OR `jam::HashMap<std::string, int, TransparentHash, std::equal_to<>>` so `IndexJob::isForeign (const char* recordPath)` performs zero heap allocation on lookup.
  2. Replace `IndexJob::foreignPaths` (`jam::Strings`, `Source/Processor.cpp:887, 1113-1115`) with the shared pointer + `rowIndex` (zero copy per job — MANIFESTO **E** Published data is immutable).
  3. Update `IndexJob::isForeign (const char* recordPath)` (`Source/Processor.cpp:964-967`) to look up `recordPath` directly without constructing `juce::File` on POSIX when already normalized (preserving Windows `\` normalization from Sprint 17, `SPRINT-LOG.md:330`).
  4. Do **not** add a parallel `HashMap` beside `jam::Strings` (`Index::paths`, `UnitsJob::paths`) — `jam::Strings::indexOf(..., 0)` is already O(1) (`jam_Strings.h:331-348`).
- **Inventory:** `jam::Strings::indexOf`, `jam::Array`, `jam::HashMap` transparent lookup (`jam_HashMap.h:858-867`), `Index::addUnitRows`, `Processor::IndexJob::isForeign`, `Processor::addIndexJob`. Absence: `jam::Hash<juce::String>` lacks `is_transparent`. Synthesis: 0 rules on data in transit, Published data is immutable, Use the framework API fully.
- **Validation:** Zero per-job `foreignPaths` copies; `isForeign` performs one O(1) zero-allocation lookup on macOS/Linux; Windows path normalization matches `unitRows` keys identically.

---

### Part 2 — Demand-Driven Indexing, Lazy Document Loading & Swap Sharing

#### Step 2.1: Filter external paths outside `Id::roots` out of `Id::files` in `UnitsJob`
- **Scope:** `Source/Processor.h`, `Source/Processor.cpp`
- **Action:**
  1. Pass the configured `Id::roots` paths (from `model.state.getChildWithName (Id::roots)`) into `Processor::UnitsJob` (`Source/Processor.cpp:1308-1335, 1357-1367`).
  2. In `Processor::UnitsJob::getOrCreatePathIndices` (`Source/Processor.cpp:1456-1464`) when adding `dependencies`, skip any dependency path where `juce::File::isAChildOf` is false for every configured root in `roots` (so OS SDK headers under `/Library/Developer/...` or Windows SDK folders are never inserted as `Id::file` rows, never watched by `LINGModel::valueTreeChildAdded` at `LINGModel.cpp:201-214`, and never scheduled for `ParseJob` at `Processor.cpp:632-638`).
  3. Keep explicit compilation unit paths and `Id::prerequisites` intact.
- **Inventory:** `Id::roots`, `Id::files`, `Id::dependencies`, `juce::File::isAChildOf` (`juce_File.h:304`), `Processor::UnitsJob`, `LINGModel::valueTreeChildAdded`. Absence: `jam::File::Watcher` has no `removeFolder`. Synthesis: Bound (watchers stay within owned roots), Lean (zero SDK `.swap` churn).
- **Validation:** `LINGModel` attaches zero watchers to OS SDK folders; `Id::files` contains zero OS SDK headers; `~/.cache/ling` stops accumulating thousands of OS SDK `.swap` files.

#### Step 2.2: Share `.swap` syntax caches across projects and toolchains inside `Id::roots`
- **Scope:** `Source/LINGModel.cpp`
- **Action:**
  1. In `LINGModel::getSwapFile` (`Source/LINGModel.cpp:159-187`), when `row.isValid()` (file is inside an `Id::root`) and `juce::String (extension).compare (files::lingSwap) == 0`, omit `getProjectName (manifest, toolchainArgument)` from the returned path: return `juce::File { row[Id::swap].toString() }.getChildFile (jam::Format::toFileName (relativePath, extension))`.
  2. Keep `getProjectName (manifest, toolchainArgument)` when `extension` is `files::lingSymbols` (since `.symbols` depends on per-project/toolchain compile arguments).
- **Inventory:** `LINGModel::getSwapFile`, `files::lingSwap`, `files::lingSymbols`, `Index::readSwapFile` (`Index.cpp:39`). Synthesis: SSOT, no `==` on strings (`compare == 0`).
- **Validation:** Projects sharing `../jam` and `../../JUCE` read and reuse one `.swap` file per source file without re-parsing or duplicating caches; `.symbols` files remain isolated per `(manifest, toolchainArgument)`.

#### Step 2.3: Demand-driven `IndexJob` materialisation, configurable `## daemon` `coalesce`, `Id::isRequested` cleanup, and profiler compatibility
- **Scope:** `cast/identifiers.md`, `Source/LINGModel.h`, `Source/LINGModel.cpp`, `Source/Manifest.h`, `Source/Manifest.cpp`, `Source/Processor.h`, `Source/Processor.cpp`, `Source/Connection.cpp`, `Source/Main.cpp`, `Source/HELP.md`
- **Action:**
  1. **Configurable `## daemon` `coalesce` (`cast/identifiers.md`, `Source/LINGModel.*`, `Source/Manifest.*`, `Source/HELP.md`):**
     - Add `coalesce` to `cast/identifiers.md` (alphabetical, between `cmake` and `codeAction`).
     - Rename/expose `LINGModel::defaultCoalesceMs` (default `1000` ms) in `LINGModel.h:46`, initialize `state.setProperty (Id::coalesce, defaultCoalesceMs, nullptr)` in `LINGModel::LINGModel`, and apply `watcher.coalesceEvents (static_cast<int> (state[Id::coalesce]))` when `Id::coalesce` changes in `LINGModel::valueTreePropertyChanged`.
     - Read `coalesce` via `Manifest::getDaemonValue (*manifest, Id::coalesce, LINGModel::defaultCoalesceMs)` in `Manifest::ManifestJob::runJob` (`Manifest.cpp:134-135`), apply in `Manifest::setToolchain` (`Manifest.cpp:172-173`), and document `coalesce` in `Source/HELP.md:195-203`.
  2. **Disk-edit invalidation (`Processor::addDocument`, `Source/Processor.cpp:1253-1274`):**
     - When `job.buffer.has_value()` is `true`, retain the single-owner `addIndexJob (unit, unitRow)` path so open LSP buffers keep live `publishDiagnostics`.
     - When `job.buffer.has_value()` is `false`: for each unit where `hasDependency (unit, unitIndex, job.rowIndex)` is true, compare the unit's current `getUnitHash` against its bound `SymbolDocument` header hash. When stale, unbind the stale `SymbolDocument` (`index.removeBindings (unitPathIndex)` + erase) and call `setUnitIndexed (unit, unitIndex)` so affected rows become `Id::isIndexed = false`.
     - Queue `addIndexJob (unit, unitIndex)` on disk edit **only** when `isIndexReady (unit, unitIndex)` is true AND (`static_cast<bool> (unit[Id::isRequested])` is true OR the unit/its owned files have an open buffer in `Id::buffers`). Do **not** speculatively queue `IndexJob` for unrequested units.
  3. **Request-driven scheduling (`Processor::addIndexJobs (const juce::ValueTree& request)`, `Source/Processor.cpp:1173-1195`):**
     - Iterate every `unitRow` in `getUnitRows (request)` and its transitive `Id::prerequisites`.
     - For each row where `not static_cast<bool> (row[Id::isIndexed])` or `row.hasProperty (Id::message)`: mark `row.setProperty (Id::isRequested, true, nullptr)`. If `not hasParseJob (row, unitRow, -1)` and `index.languages.at (family)->hasIndex()` and no `indexJob`/`nextIndexJob` already matches the current hash, queue `addIndexJob (row, unitRow)` and trigger dispatch (`addIndexJobs()` + `moveRequestJobsToFront (request)`).
  4. **Exact global readiness (`Processor::setIndexed()`, `Source/Processor.cpp:447-461`):**
     - Set `files[Id::isIndexed]` true iff `toolchain` is valid, `unitsJobs` and `nextUnitsJobs` are empty, and **every child row of `files` has `static_cast<bool> (row[Id::isIndexed]) == true`**.
     - Keep `jobCount` and `pendingJobCount` counting only active/queued jobs so `Server` `$/progress` (`Server.cpp:31-41`) emits `Begin -> Report -> End` cleanly per demanded batch.
  5. **Clear `Id::isRequested` on cancel and disconnect (`Source/Connection.cpp:22-26, 36-40`, `Source/Processor.cpp`):**
     - React to `Id::request` / `Id::connection` removal (`valueTreeChildRemoved` — MANIFESTO **E** Consume the event payload) to clear `Id::isRequested` on any unit no longer needed by any remaining pending request across all connections.
  6. **Profiler compatibility (`Source/Main.cpp:384-394`):**
     - Under `#elif JAM_USING_PROFILER`, once `Id::files` is populated, schedule workspace-wide indexing so `JAM_USING_PROFILER` benchmarks the full `ParseJob` + `IndexJob` pipeline deterministically.
- **Inventory:** `Id::coalesce`, `Manifest::getDaemonValue`, `jam::File::Watcher::coalesceEvents`, `Processor::addDocument`, `Processor::addIndexJobs`, `Processor::getUnitRows`, `Processor::setIndexed`, `Processor::setUnitIndexed`, `Processor::moveRequestJobsToFront`, `Id::isRequested`, `Id::isIndexed`, `Connection::connectionLost`, `Connection::messageReceived`. Synthesis: Materialisation on demand, Consume the event payload, nearest-sibling precedence (`timeout`/`threads`/`coalesce`), positive nesting.
- **Validation:** `## daemon` `coalesce` updates `LINGModel::watcher` cleanly; disk edits with zero active requests and zero open buffers trigger `ParseJob` only and zero `IndexJob` runs; stale symbols are immediately unbound (zero stale reads); `Id::file` query indexes only 1 unit + its prerequisites; `Id::files` query (`findReferences`, `workspaceSymbol`) indexes all unindexed units; cancelling/disconnecting clears `Id::isRequested`.

#### Step 2.4: On-demand bounded `Index::documents` loading and lightweight file-hash table
- **Scope:** `Source/Index.h`, `Source/Index.cpp`, `Source/Processor.h`, `Source/Processor.cpp`, `Source/Query.h`, `Source/Query.cpp`
- **Action:**
  1. Separate lightweight file hash tracking (`jam::HashMap<int, juce::int64> sourceHashes`, keyed by `pathIndex`) from resident `jam::Document` AST storage (`Index::documents`).
  2. Update `Processor::getUnitHash` (`Source/Processor.cpp:49-73`) to read `index.sourceHashes` instead of `index.documents`.
  3. On `ParseJob` completion for disk files without an open buffer (`not job.buffer.has_value()`), store the validated `sourceHash` in `index.sourceHashes`, write/verify the `.swap` file on disk, and release the heap `jam::Document` instead of pinning all 5,000+ ASTs in RAM (keep open buffers in `Id::buffers`, built-in non-indexed languages, and a bounded working-set cache of `maxCachedDocuments { 64 }` most-recently-queried documents).
  4. Provide `Index::getOrLoadDocument (int pathIndex)` that returns the resident `jam::Document*` if cached/buffered, or deserializes it on demand from its `.swap` file + source file into the bounded cache (evicting the least-recently-used non-buffered document when exceeding `maxCachedDocuments`).
  5. Update `Index::getElement` (`Source/Index.cpp:237-249`), `Index::applyToOccurrences` (`Source/Index.h:276-287`), and `Query` callers to read through the on-demand document accessor. Keep `applyToDocuments` (`JAM_USING_PROFILER`) working deterministically.
  6. Ensure generated `BinaryData*.cpp` units already ignored by `cLang` (`ignoredUnits`) are never scheduled for `ParseJob`, preventing 968 MB `BinaryData7.cpp.swap` files.
- **Inventory:** `Index::documents`, `Index::readSwapFile`, `Index::getElement`, `Index::applyToOccurrences`, `Processor::getUnitHash`, `Processor::ParseJob`, `jam::HashMap`, `jam::Array`. Absence: no bounded LRU cache with explicit invalidation in `jam_core` or public `juce_core`; `jam::Document::SwapReader` has no header-only hash peek. Synthesis: Bound memory, named constant for cache limit.
- **Validation:** Idle daemon RSS drops from 1.0–1.4 GB to <150 MB; zero `BinaryData*.cpp.swap` files parsed; `Query` results (`goToDefinition`, `findReferences`, `hover`, `rename`, `semanticTokens`, etc.) are 100% bit-identical to resident mode.

---

### Part 3 — `cLang` Efficiency-Core Confinement & AST/Macro Pruning

#### Step 3.1: Worker pool 8 MiB stack, `defaultThreads = 2`, and strict E-core `libclang` confinement
- **Scope:** `Source/Processor.cpp`, `Source/LINGModel.h`, `Source/HELP.md`, `language/cLang/language.cast`
- **Action:**
  1. In `Source/Processor.cpp:10-13`, set `static constexpr size_t workerStackSizeBytes { 8 << 20 };` and configure `getPoolOptions` with `.withThreadStackSizeBytes (workerStackSizeBytes)` alongside `.withDesiredThreadPriority (juce::Thread::Priority::background)`.
  2. Lower `LINGModel::defaultThreads` (`Source/LINGModel.h:43`) from `4` to `2` and update `Source/HELP.md:202` (`## daemon` `threads` default = `2`).
  3. Remove `session.libclang->executeOnThread (..., defaultStackSize)` from `language/cLang/language.cast:752` and invoke `indexSource` directly on the calling pool thread.
  4. In `IndexSession::setLoaded` (`language/cLang/language.cast:487-507`), set `LIBCLANG_NOTHREADS=1` (`setenv ("LIBCLANG_NOTHREADS", "1", 0)` on POSIX, `_putenv_s ("LIBCLANG_NOTHREADS", "1")` on Windows) before `createIndex` so `libclang` runs directly on JUCE's 8 MiB `QOS_CLASS_BACKGROUND` worker thread without spawning unowned child threads (MANIFESTO **B — Bound**). Do **not** call `clang_CXIndex_setGlobalOptions (CXGlobalOpt_ThreadBackgroundPriorityForAll)` (proved to upgrade `QOS_CLASS_BACKGROUND` to `QOS_CLASS_UTILITY`).
- **Inventory:** `juce::ThreadPool::Options::withThreadStackSizeBytes`, `juce::Thread::Priority::background`, `LINGModel::defaultThreads`, `language/cLang/language.cast:487-507, 732-759`. Absence: `PRIO_DARWIN_BG` absent in JUCE. Synthesis: Bound (threads stay strictly on E-cores), no magic numbers.
- **Validation:** Zero `clang_executeOnThread` calls and zero extra `libclang` threads spawned; macOS `qos_class_self()` verified at `QOS_CLASS_BACKGROUND (0x9)` during parsing; no stack overflows on deep JUCE/JAM templates.

#### Step 3.2: `cLang` AST, macro, outline, and `ppIncludedFile` pruning + `HoverEntry` fallback for OS SDK `goToDefinition`
- **Scope:** `language/cLang/language.cast`, `Source/Query.cpp`
- **Action:**
  1. **`reportOutline` (`language.cast:3821-3855`):** Check `context.callbacks.addOutline != nullptr and not context.isForeign (file) and context.keptRanges.isEmpty()` **before** calling `addPreprocessedFile`, `getFileContents`, `getOutlineQualifierStart`, `getOutlineNameLength` (`libclang.tokenize`), `getFileName`, and `getNormalPath`.
  2. **`onMacro` & `addMacroCompletion` (`language.cast:4930-4949, 4116-4127`):** Extract `location` and `file` once; guard with `file != nullptr and libclang.isInSystemHeader (location) == 0 and not context.isForeign (file) and context.isKept (file, offset)` **before** calling `getCursorUSR` and `getCursorSpelling`. Pass the already-computed `usr` and `spelling` into `addMacroCompletion` (eliminating duplicate libclang calls and skipping non-`macroDefinition` calls).
  3. **`onCursor` (`language.cast:5514-5546`):**
     - If `libclang.isInSystemHeader (location) != 0` and `cursor.type != CursorType::cursorNamespace` and `cursor.type != CursorType::linkageSpecification`, return `ChildVisitResult::visitContinue`.
     - If `context.isForeign (location)` and `cursor.type != CursorType::cursorNamespace` and `cursor.type != CursorType::linkageSpecification`, return `ChildVisitResult::visitContinue` (preserving unity-build `namespace juce { #include ... }` and `extern "C" { #include ... }`).
     - When `not context.keptRanges.isEmpty()` and `cursor.type != CursorType::cursorNamespace` and `cursor.type != CursorType::linkageSpecification`, check whether `getCursorExtent` overlaps any kept range of its file; if not, return `ChildVisitResult::visitContinue`.
  4. **`ppIncludedFile` (`language.cast:5719-5723`):** Do **not** call `isInSystemHeader (includedFile->hashLoc)` (proved unreliable — points to includer). Skip `addPreprocessedFile` when `context.isForeign (includedFile->file)` is true; defer `addPreprocessedFile` for non-foreign files until needed.
  5. **`onDeclaration` / `onReference` / `addDiagnostics` + `Query::goToDefinition` fallback (`Source/Query.cpp:338-351`):** Guard `onDeclaration`, `onReference`, and `addDiagnostics` with `isInSystemHeader == 0` so OS SDK records no longer bloat `.symbols`. In `Query::goToDefinition`, when `index.declarations` yields no owned declaration for a symbol record whose `hover != SymbolDocument::noHover`, return the definition location already stored in `HoverEntry` (`definitionPath`, `line`, `column`, `endLine`, `endColumn`), preserving 100% `goToDefinition` runtime behavior into OS SDK headers with zero SDK watchers and zero SDK `.swap` files.
- **Inventory:** `language/cLang/language.cast:3091-3183, 3821-3855, 4116-4127, 4231-4281, 4930-4949, 5514-5546, 5719-5723`, `Query::goToDefinition` (`Source/Query.cpp:338-351`), `Query::hover` (`Source/Query.cpp:449-465`). Synthesis: Lean, 0 new structs, reuses already-materialized `HoverEntry`.
- **Validation:** `.symbols` file sizes drop substantially on JUCE/JAM units; `SchemaSuite`, unity-build tests, and `goToDefinition` (both project/JAM/JUCE and OS SDK headers) pass 100%.

#### Step 3.3: Prune empty and include-only `#ifdef` platform passes
- **Scope:** `language/cLang/language.cast`
- **Action:**
  1. Do **not** pass `-Xclang -fno-validate-pch` on cross-platform passes (proved broken by live `libclang` probe — loads baked macOS AST and fails on Windows declarations). Keep `keepPrecompiled = false` for genuine cross-platform code passes.
  2. In `addSkippedDirectives` (`language.cast:5902-5935`), inspect the skipped range body text before setting `platformMask` bits: ignore empty ranges and ranges whose non-comment, non-whitespace lines are exclusively `#include` directives (whose target OS SDK headers do not exist on the host machine, e.g. `#ifdef _WIN32 #include <windows.h> #endif` at `Main.cpp:12-14`).
  3. Rely on Step 3.2's `keptRanges` extent pruning inside `onCursor` so active cross-platform passes inspect only the kept conditional ranges instead of walking the full translation unit.
- **Inventory:** `addSkippedDirectives` (`language.cast:5902-5935`), `addContextPasses` (`language.cast:6056-6080`), `getContextCommandLine` (`language.cast:5789-5821`). Absence: cross-platform PCH reuse disproved.
- **Validation:** Units with `#ifdef _WIN32` `#include`-only guards run zero unnecessary non-PCH reparses; units with real cross-platform code (`JUCE_WINDOWS` / `JUCE_LINUX`) retain 100% cross-`#ifdef` symbol accuracy (`workspaceSymbol HWNDComponentPeer` passes).

---

### Part 4 — User-Wide Shared Daemon ("Vulkan Instance" Model)

#### Step 4.1: User-wide daemon identity and workspace registry
- **Scope:** `Source/LINGModel.h`, `Source/LINGModel.cpp`, `Source/Server.h`, `Source/Server.cpp`, `Source/Main.cpp`
- **Action:**
  1. Key the daemon `juce::InterProcessLock` and port file solely on `buildHash` (`LINGModel::getDaemonName()`), stored at `LINGModel::getCacheDirectory()` (`~/.cache/ling/<buildHash>.port`) so one daemon instance serves all projects for the running `ling` binary.
  2. Introduce a `Workspace` owner (one `LINGModel`, `Manifest`, `Index`, and per-workspace idle timer per `(manifestPath, toolchainArgument)` key) managed by a top-level `Daemon` / `Server` registry using `jam::HashMap<juce::String, std::unique_ptr<Workspace>>`.
  3. Keep `Server`'s executable watcher so a rebuilt `ling` binary still cleanly terminates the old daemon.
- **Inventory:** `LINGModel::getDaemonName` (`LINGModel.cpp:41-55`), `Server::getPortFile` (`Server.cpp:63-66`), `Application::runDaemon` (`Main.cpp:416-436`), `jam::HashMap`, `juce::InterProcessLock`. Synthesis: Bound (clear ownership of each `Workspace`), SSOT (one daemon per binary build).
- **Validation:** Opening `ling`, `saw`, and Neovim simultaneously spawns exactly **1** `ling daemon` process instead of N separate daemons.

#### Step 4.2: Shared worker pool across workspaces, client routing, and idle eviction
- **Scope:** `Source/Client.h`, `Source/Client.cpp`, `Source/Connection.h`, `Source/Connection.cpp`, `Source/ToolRelay.cpp`, `Source/Processor.h`, `Source/Processor.cpp`
- **Action:**
  1. Own a single shared `juce::ThreadPool` (`defaultThreads = 2`, 8 MiB stack, background QoS) at the daemon level via `jam::SharedInstance<juce::ThreadPool>` (or passed by reference from the daemon owner) into each `Workspace`'s `Processor`, capping total concurrent `libclang` parses across ALL open projects to 2 threads.
  2. Include `(manifestPath, toolchainArgument)` in the `Id::connection` handshake message (`Client::connectionMade` -> `Connection::messageReceived`) so `Server` routes each client connection to its `Workspace` (creating the `Workspace` on first attach).
  3. Evict a `Workspace` (releasing its `Index`, `SymbolDocument` mappings, and `Processor` state) when its last client disconnects and its `Id::timeout` expires; exit the daemon when zero `Workspace`s remain.
- **Inventory:** `jam::SharedInstance` (`jam_SharedInstance.h:17-86`), `Client::start` (`Client.cpp:23-39`), `Connection::messageReceived` (`Connection.cpp:28-45`), `ToolRelay::getOrCreateClient` (`ToolRelay.cpp:91-103`), `Processor::pool` (`Processor.cpp:18`). Synthesis: Bound (zero leaked workspaces), Encapsulation (unidirectional routing).
- **Validation:** Concurrent agents across `ling` and `saw` share 1 daemon and at most 2 background threads; closing one project evicts its workspace RAM after timeout while the other project stays live.

---

### Verification & Audit

#### Step 5.1: Build, determinism suite, multi-agent stress, and profiler verification
- **Scope:** Build & verification (ARCHITECT builds via `cast cast/spell.md`; Pathfinder runs read-only probes/tests)
- **Action:**
  1. Run `LING_USING_SCHEMA` (`SchemaSuite` + `StdioClientProbe`) — 100% pass rate, including failed-unit retry (`-32803`), `publishDiagnostics` on open buffers, and cross-`#ifdef` queries.
  2. Run multi-agent stress test: 3 concurrent MCP clients editing files on disk interleaved with `hover`, `diagnostics`, `goToDefinition`, and `findReferences` — verify zero speculative `IndexJob`s on disk edits, zero unchanged `SymbolDocument` wipes on `.ninja_deps` reload, and single shared daemon RSS/CPU.
  3. Run `JAM_USING_PROFILER` (`profile-info.md`) — verify E-core background QoS, wall-clock/CPU reduction in `runIndexJob` and `addSymbols`, and idle/peak RSS reduction.
- **Inventory:** `Main.cpp:373-394`, `profile-info.md:1-19`.
- **Validation:** 100% test pass rate; zero regressions in query results; measured CPU, RAM, and disk cache reductions recorded.

#### Step 5.2: Single `@Auditor` sweep, Pathfinder Doxygen pass, and `/log`
- **Scope:** All files modified across Parts 1–4.
- **Action:**
  1. Run `@Auditor` once across the entire sprint against `MANIFESTO.md`, `CODING.md`, `NAMES.md`, `LANGUAGE.md`, and this locked PLAN.
  2. Resolve every validated DCF finding via `@Engineer` and verify at file:line.
  3. Delegate comprehensive Doxygen pass to `@Pathfinder` on all modified headers, verify every block against code, and regenerate Doxygen XML.
  4. On ARCHITECT's `/log` trigger, log sprint and write commit message in chat.
- **Inventory:** `CODING.md` COMMENTS, `doxygen-writer` skill.
- **Validation:** Zero unresolved Auditor findings; zero Doxygen warnings; every changed symbol documented accurately.

---

## BLESSED Alignment

- **B — Bound:** Worker threads run with explicit 8 MiB stack, `QOS_CLASS_BACKGROUND`, and `LIBCLANG_NOTHREADS=1` (zero unowned child threads); OS SDK paths are excluded from watchers; each `Workspace` has one clear owner with RAII eviction.
- **L — Lean:** Eliminates quadratic `removeCompletions`, per-job `foreignPaths` copies, unpruned AST/macro/outline walks, include-only cross-platform reparses, and unbounded heap `jam::Document` residency.
- **E — Explicit:** Every constant (`workerStackSizeBytes`, `maxCachedDocuments`, `defaultCoalesceMs`, `currentFormatVersion`) is named; no bail-out guards; no magic values; failure paths stay loud (`-32803`, `juce::Result`).
- **S — Single Source of Truth:** Shared `.swap` files across projects under `Id::roots`; `jam::Strings::indexOf(..., 0)` reused without duplicate hash maps; one user-wide daemon per binary build; lifetime-stable `unitPathIndex`.
- **S — Stateless:** `Processor` remains a deterministic orchestrator over `LINGModel` state with zero timer heuristics; `Index::unitRows` published as immutable `shared_ptr<const ...>`.
- **E — Encapsulation:** Complete at Creation (`SymbolDocument` v2 sorted on worker, 0 transit allocations/copies); Materialisation on demand (`IndexJob` and `jam::Document` loaded only when needed); strict `Source/` vs `language/cLang/` boundary preserved.
- **D — Deterministic:** Exact `map::Readiness` guarantees preserved; stale symbols unbound immediately on hash mismatch; identical query outputs (including OS SDK `goToDefinition` via `HoverEntry`) verified by `SchemaSuite`.

---

## Risks / Open Questions

None. All design choices are resolved by `MANIFESTO.md`, `CODING.md`, verified codebase/runtime probes, and ARCHITECT's rulings. Ready for approval (`Proceed`).
