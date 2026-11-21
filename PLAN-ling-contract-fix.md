# PLAN: ling contract fix — one parse, direct lookup, no rework

**RFC:** none — objective from ARCHITECT prompt. (`RFC-ling-runs-from-anywhere.md` has another objective.)
**Date:** 2026-11-20
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE / JAM engine (`Source/`). The cLang library is a cast project (`language/cLang/language.cast`, `construction.md`) on Apple libclang (`construction.md:510`).

After approval, this file is copied to `/Users/jreng/Documents/Poems/dev/ling/PLAN-ling-contract-fix.md`.

## Overview

ARCHITECT, this session:
- *"fix the violations first THOROUGHLY, fix all code smells against contract. then continue with outstandings."*
- *"dont abuse building and profiling … make it work per contract. then find the bottleneck at last stage after the working state meet expectation"*
- *"no redundancy no copy no rebuild no reparse 0 rules"*; *"ling still language agnostic, we should never hardcode anything to its engine"*.
- On the backend: LibTooling needs the clang front end as libraries (`clang/lib/Tooling/CMakeLists.txt:38-50`, `clang/lib/Frontend/CMakeLists.txt:51-60`). ARCHITECT rejected building or downloading them: *"FUCK NO. why we need the whole LLVM? this is clearly wrong approach"*. The backend stays Apple libclang from the build toolchain.

The plan has five phases: engine violations (steps 1-8), cLang one-pass work (9-10), contract smells (11-14), one build and the acceptance harness (15), and the profile last (16).

**Corrections to the earlier violation list** (with the reads that close them):
- Item 3 (full re-parse on edit) is a state update: *"A complete state replaces a complete state"* (MANIFESTO:256). The edited text has not been parsed before.
- Item 5 (dependents re-indexed on a header edit): a header edit changes each dependent unit's AST (`Processor.cpp:60-64`). No step.
- Item 7 (buffer texts and unit paths copied per job) is permitted copy 2. The job runs on a pool thread while the message thread changes the buffers (MANIFESTO:276; `Processor.cpp:1111-1119`).
- Item 8 (path text in `Index.paths`): `Index.paths` is the identity registry (MANIFESTO:252). Record `Id::path` is relative (`Index.cpp:140`). No step.
- Item 9 (path copy per location): `juce::File` takes a `juce::String` (`Query.cpp:216`). No step.
- Two items already match the code: the JuceHeader PCH is parsed once by ling (`language.cast:2192`, `:2507-2543`, `:5212-5213`), and `PLAN-ling-ast-once.md` step 1d is present (`language.cast:4159`).

## Language / Framework Constraints
- CODING.md "One object, one translation unit". Templates stay in the header (`Query.h`, `Index.h`).
- ARCHITECT: *"dont abuse build for code validation."* Step 15 is the only build. Step 16 is the only profile.
- Audit and doxygen are held: *"hold audit and doxygen. until i explicitly said so."* Doc/code contradictions found by the reads (`Index.h:14-16`, `:73`, `:80-86`, `:92`, `:193-195`, `:222-236`; `Processor.h:22-23`, `:137-138`, `:159-171`, `:470-497`) wait for the doxygen pass.

## Framework Inventory

### API
| Need | Module | API | Source | Step |
|---|---|---|---|---|
| Sorted pointer arrays, block insert, range remove | jam_core | `jam::Array::insertArray`, `removeRange`, `first`, `begin`/`end` | `jam_Array.h:490`, `:541`, `:278`, `:229-250` | 1, 2, 6 |
| Keyed maps | jam_core | `jam::HashMap::try_emplace`, `find`, `at`, `erase`, `contains` | `jam_HashMap.h:2932`, `:3524`, `:3361`, `:3235`, `:3573` | 1, 2, 6, 7 |
| Name token of a position | jam_core | `Document::getNameToken`, `getLineNumber`, `getColumnNumber`, `getByteOffset` | `jam_Document.cpp:315-332`, `:297-302` | 1 |
| Library callbacks | jam_syntax_tree | `jam::LanguageCallbacks` (`addSymbol` … `addFixIt`, `shouldExit` as the predicate member) | `jam_LanguageInterface.h:59-79`, `:75` | 9 |
| PCH declarations not enumerated | libclang | `clang_createIndex (excludeDeclarationsFromPCH, …)` | `Index.h:236-240`, `:268-269`, `:271-272` | 10 |
| File stem | JUCE | `juce::File::getFileNameWithoutExtension` | `Query.cpp:148` (use) | 7 |

### Pattern usage
| Pattern | Exemplar | Use sites | Step |
|---|---|---|---|
| Index map built once at bind, removed at unbind | `addOccurrence` into `declarations`/`references` | `Index.cpp:357-379`, `:393-403`; `:299-327` | 1, 2, 6 |
| Order of records by position | `Index::isRecordBefore` | `Index.cpp:381-384` | 1, 2 |
| Order of occurrences by path, then position | `Index::isOccurrenceBefore` | `Index.cpp:386-391` | 2, 3 |
| Row maps rebuilt at a files state update | `pathRows`, `unitRows` in `addUnitRows` | `Index.cpp:156-166` | 7 |
| Creation-time token cleanup after the library returns | `sortFileTokens` | `Processor.cpp:816-828`, `:855-858` | 5 |
| Engine predicate callback with a job context | `shouldExit` lambda | `Processor.cpp:851` | 9 |
| Foreign path test in the job | `IndexJob::isForeign` | `Processor.cpp:864-867`; callers `:892`, `:915`, `:939`, `:958`, `:977`, `:1040`, `:1063` | 9 |

### Names
| Family | Members | Intent | Step |
|---|---|---|---|
| Index symbol maps | `declarations`, `references`, `completions` (`Index.h:228`, `:237`, `:239`) | keyed index over pointers into SymbolDocuments | 1: `records` |
| Index row maps | `unitRows`, `pathRows` (`Index.h:245`, `:247`) | key → row index | 7: `stemRows` (std::filesystem `stem`) |
| `apply*` with a callable | `applyToRecords`, `applyToOccurrences` (`Index.h:127`, `:148`) | call the function on each match | 1, 3 (overload with a path index) |
| Library predicate members | `shouldExit` (`jam_LanguageInterface.h:75`) | returns nonzero for yes | 9: `isForeign` (joins `IndexJob::isForeign`, `Processor.cpp:864`) |
| `Type` for `kind` (NAMES Rule 6) | `getCompletionKind` (`Language.h:20`), `progressKind` (`Server.h:90`), `completionKinds` (`language.cast:502`), `definitionKinds` (`:2854`), `getOutlineKind` (`:3304`), `unsigned (int kind)` (`construction.md:471`) | ARCHITECT ruling 41: *"including fixing `kind`"* | 12: `getCompletionType`, `progressType`, `completionTypes`, `definitionTypes`, `getOutlineType`, `unsigned (int type)` |
| Banned `resolve` (NAMES Rule 6) | `resolved` (`language.cast:3202`, `:3207`) | the file that an include directive names | 12: `includedFile` |

### Synthesis
| Rule | Established form | Exemplar |
|---|---|---|
| "No bail-out guards" | positive nesting; result returns | `Index.cpp:112-113` |
| "Use `.at()` …" | `HashMap::at` for a present key | `Query.cpp:1004` |
| "Structured bindings for all pair/tuple results" | `const auto& [a, b] { *entry };` | `Index.cpp:284-285` |
| "No out-parameters" | return the value | `Index::getElement` (`Index.cpp:207-216`) |
| "Consume the event payload" | listener reacts to the delivered row | `Processor.cpp:625-629` |

### Absence
| Need | Query | Result |
|---|---|---|
| A libclang hook that skips indexing a file | read of `ppIncludedFile` storage | `CXIndexDataConsumer.cpp:473-490`: the return value is stored and never tested |
| An ABI version constant for `LanguageCallbacks` | LSP `workspaceSymbol` "languageVersion" | "No symbols found in workspace." |
| A position index of records in Index | read of `Index.h:209-247` | no member keyed by path index holds records by position |

## Validation Gate
Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step. No step builds. Step 15 builds once.

## Steps

### Phase 1 — engine violations

### Step 1: records by position, looked up in O(log n)
**Scope:** `Source/Index.h`, `Source/Index.cpp`.
**Action:**
1. Add the member `jam::HashMap<int, jam::Array<const jam::Document::Element*>> records;`: path index → that file's symbol records, sorted by `isRecordBefore`.
2. `addSymbols` (`Index.cpp:393-403`) fills `records[pathIndex]` from `SymbolDocument::applyToRecords`, then sorts it once.
3. `removeSymbols` (`:299-309`) erases `records` at the path index.
4. `applyToRecords` (`Index.h:127-145`) computes the name token's line, column and end column once. It `lower_bound`s the sorted records, then calls the function for each record until a record's position reaches the token end.
5. Delete `isRecordInName` (`Index.cpp:256-264`) and its declaration.
**Inventory:** Pattern "index map built once at bind", "order of records"; Names `records`.
**Validation:** no loop over all records of a file remains in `applyToRecords`. The order comes from `isRecordBefore` only.

### Step 2: one block insert and one block remove per symbol
**Scope:** `Source/Index.cpp`, `Source/Index.h`.
**Action:**
1. In `addSymbols`, walk `records[pathIndex]` (sorted), and group the record pointers per symbol in a local work map. For each symbol, insert its block once with `insertArray` at the `partition_point` of the path (`isOccurrenceBefore`).
2. `removeSymbols` erases each symbol's block for the path once: `equal_range` by path, then `removeRange`. Each symbol is handled once, through a local `jam::HashSet<juce::int64>`.
3. `removeRecord` (`:311-327`) and `addOccurrence` (`:357-379`) are deleted, together with their declarations.
4. Dropped. `removeDocuments` calls `removeBindings` after the row is gone (`Index.cpp:28-34`), so the row cannot give the paths. The scan of `symbolFiles` is a state update: *"Creation and state updates have no limit"* (MANIFESTO:284). `removeBindings` stays as it is.
**Inventory:** API `insertArray`, `removeRange`; Pattern "order of occurrences".
**Validation:** no `remove_if` over all of a symbol's occurrences once per record. No sorted insert per record. MANIFESTO:254 (creation work buffers) covers the local map and set.

### Step 3: occurrences of one file by range
**Scope:** `Source/Index.h`, `Source/Query.h`, `Source/Query.cpp`.
**Action:**
1. Add an `Index::applyToOccurrences (occurrences, pathIndex, function)` overload. It finds the path's range with `partition_point` (path order, `paths.getView` compare) and visits that range only.
2. Add a `Query::applyToSymbolOccurrences (symbol, pathIndex, function, index)` overload that uses it.
3. `documentHighlight` (`Query.cpp:416-441`) uses the overload and drops its `occurrenceIndex == pathIndex` filter (`:431`).
**Inventory:** Names `applyToOccurrences` family.
**Validation:** the work is bounded by the occurrences in the queried file.

### Step 4: one declaration per name token in semantic tokens
**Scope:** `Source/Query.cpp`.
**Action:** `getRecordType` (`:981-1000`) takes the first record that `applyToSymbols` gives. It reads `index.declarations.find (symbol)`, and takes `first()` of that symbol's occurrences. It gets the element through `Index::getElement` and returns its `Id::type`. Without a declaration it returns its own type.
**Validation:** no walk over all of a symbol's declarations for each token (`:990-994` is gone).

### Step 5: outlines in pre-order, emitted in one pass
**Scope:** `Source/Processor.cpp`, `Source/Processor.h`, `Source/Query.cpp`, `Source/Query.h`, `Source/BuiltInLanguage.h`.
**Action:**
1. `sortFileTokens` (`Processor.cpp:816-828`) also rewrites the file's `Id::outline` Tokens into pre-order with contiguous subtrees, and remaps each `Id::parent`. It uses creation work buffers (child lists by parent).
2. `BuiltInLanguage::addOutlines` (`BuiltInLanguage.h:213-217`) builds a local element-to-index map once. `getParentIndex` (`:238-247`) reads that map instead of `std::find` over all declarations.
3. `Query::appendOutline` returns the index after its subtree. `appendOutlines` (`Query.cpp:718-731`) consumes the children of a node in one forward pass, as long as the next token's parent is that node.
**Validation:** `documentSymbol` does one pass over the outlines (`:720-722` is gone). BuiltInLanguage outlines are already pre-order (`applyFunctionRecursively` calls the function before the children, `jam_Document.h:389-393`).

### Step 6: typeDefinition in one pass, incomingCalls resolves each reference once
**Scope:** `Source/Query.cpp`, `Source/Query.h`.
**Action:**
1. `typeDefinition` (`:840-884`) makes one `applyToOccurrences` pass per type symbol. It keeps the first definition and the first declaration, then writes the definition, else the declaration. This replaces the two passes at `:860` and `:869`.
2. `appendIncomingRun` (`:1379-1404`) takes the run's first call element. It walks the references forward while `getCaller` is the run's caller, writes each range as it resolves the reference, and returns the run end. `appendIncomingRuns` (`:1360-1377`) continues from that return value.
**Validation:** each reference resolves once, plus once per run head.

### Step 7: counterpart by stem
**Scope:** `Source/Index.h`, `Source/Index.cpp`, `Source/Query.cpp`.
**Action:**
1. Add the member `jam::HashMap<juce::String, jam::Array<int>> stemRows;`: file name without extension → row indices. `addUnitRows` (`Index.cpp:156-166`) fills it beside `pathRows`.
2. `getCounterpart` (`Query.cpp:119-155`) replaces the whole-model loop (`:144-151`) with `stemRows.find`. It then tests the extension of each row in that entry.
**Validation:** no loop over every file row.

### Step 8: workspaceSymbol over the declaration map
**Scope:** `Source/Query.cpp`, `Source/Index.h`.
**Action:**
1. `getMatches` (`Query.cpp:1233-1257`) walks `index.declarations` and gets each element through `Index::getElement` on the record position. It stops walking every element of every document (`Index.h:57-68`).
2. Delete `Index::applyToElements`. Its only caller is `getMatches` (Pathfinder, LSP).
**Validation:** the walk covers declared names only. A fuzzy match has no lookup structure in this codebase. This step removes the whole-tree walk and keeps the fuzzy match.

### Phase 2 — cLang does no work for a file that another unit owns

### Step 9: `isForeign` callback, tested before any row is built
**Scope:** `jam/jam_syntax_tree/document/jam_LanguageInterface.h`, `language/cLang/language.cast`, `Source/Processor.cpp`, `Source/Processor.h`.
**Action:**
1. Append the member `int (*isForeign) (void* context, const char* path) { nullptr };` to `LanguageCallbacks`, after `addFixIt` (`:78`), so the offsets of the earlier members stay the same.
2. `IndexJob::addSymbols` (`Processor.cpp:843-851`) sets `callbacks.isForeign` to a static wrapper over `IndexJob::isForeign` (`:864-867`).
3. Delete the seven `if (not job.isForeign (recordPath))` gates (`:892`, `:915`, `:939`, `:958`, `:977`, `:1040`, `:1063`). The library holds the test once (MANIFESTO D).
4. In cLang:
   - `IndexContext` caches the answer per file word.
   - The test sits at the output funnels, not at the entry of `onDeclaration`. `onDeclaration` also fills cross-file state that owned-file answers read: `declarationFiles` (`:3762-3763`) and the completion rows of inherited completions (`:3708-3725`).
   - Funnels: `IndexContext::addSymbol` (`:2730`; every record, from `:3760`, `:3775`, `:3798`, `:4435`, `:4939`, `:4968`), `IndexContext::addHover` (`:2778`; this also skips the hover text rendering at `:5070-5083`), `addHint` (`:2818`), `reportOutline` (`:3352`), `emitCompletion` (`:3696`, tested by the emitted file), `addDiagnostics` (`:5136`) and `addFixIts` (`:5104`).
   - `onCursor` (`:4977`) does its work only for a location whose file is not foreign, and still visits the children. Its `cursorRecords` table (`construction.md:726-741`) holds only hint and symbol writers.
**Inventory:** Names `isForeign`; Pattern "engine predicate callback", "foreign path test".
**Validation:** no record, hover, outline, completion, hint, diagnostic or fix-it is built for a foreign file. The engine has no foreign test left. Old config-folder libraries are deleted at step 15.

### Step 10: no redundant PCH declarations
**Scope:** `language/cLang/language.cast`.
**Action:** Dropped. The PCH declarations in a consumer unit are not redundant. They feed owned-file answers:
1. Inherited completions read the base rows from `completionsByScope` (`language.cast:3708-3725`).
2. Only `onDeclaration` → `addCompletionRecord` fills those rows (`:3656-3667`, `:3766`).
3. The engine has no base walk: `applyToScopeSymbols` (`Query.h:299-334`) → `appendScopeCompletions` (`Query.cpp:764-772`) reads one scope.
With `{ 1 }`, a class in an owned file that derives from a PCH class loses its inherited members. Step 9 already drops every output for a foreign file at the funnels. `excludeDeclarationsFromPCH` stays `{ 0 }`.
**Validation:** consumer units of the PCH unit (`language.cast:2462-2489`) no longer enumerate PCH declarations. The PCH unit indexes them once.

### Phase 3 — contract smells

### Step 11: dead code and duplicate guards
**Scope:** `Source/Query.h/.cpp`, `Source/Index.h/.cpp`, `Source/LINGModel.cpp`.
**Action:**
1. Delete `Query::isVisible`, `getSegmentCount` and `getSegment` (`Query.h:231-235`; `Query.cpp:67-117`). `isVisible` has no caller, and the other two are called only from it.
2. Dropped. `Index::applyToDocuments` has a caller: `Processor.h:55-57`, called from `Main.cpp:482`.
3. Delete the second assert of an invariant that its owner already checks. `LINGModel.cpp:25` asserts a parameter created at `:13`. The `getElement` assert (`Index.cpp:209`) also covers library record positions, which `Query.cpp:58-62` does not check. It moves to the boundary where those positions enter: `IndexJob::addSymbol` asserts `line > 0 and column > 0`.
4. Replace `operator[]` on the completion map. After steps 1-3 the sites are `Index.cpp:354` and `:373`:
   - `removeCompletions` (`:354`): the key can be absent. An earlier pass of the same file erases a scope when its array empties (`:353-354`). `.at()` would throw, so the site uses `find` with positive nesting.
   - `addCompletions` (`:373`): the site inserts, so it uses `try_emplace` with structured bindings.
   - `Processor.cpp` `addOutlineToken`: `fields[N]` → `.at (N)`.
**Validation:** CODING "Use `.at()`"; MANIFESTO D (one assert per invariant); E (no dead getters).

### Step 12: `kind` and `resolve` names
**Scope:** `Source/Language.h`, `LibraryLanguage.h/.cpp`, `BuiltInLanguage.h`, `Query.cpp`, `Server.h/.cpp`, `language/cLang/language.cast`, `language/cLang/construction.md`.
**Action:** rename the Names-table rows to their targets at every use: LSP findReferences for C++, Grep for `.cast` and `.md`. LSP protocol literals (`lsp::WorkDoneProgressKind`, JSON keys) stay verbatim. libclang mirror names (`getCursorKind`) stay, because they name the C symbol.
**Validation:** no own name holds `kind` or `resolve`.

### Step 13: cLang control-flow and literal smells
**Scope:** `language/cLang/language.cast`, `language/core/core.cast`.
**Action:**
1. Bail-out at `language.cast:4990` → positive nesting. Done in Step 9, which rewrites the entry test of `onCursor`.
2. Lambda `=` init → brace init at `:2964`, `:3063`, `:3095`, `:3196`, `:3921`, `:3959`, `:4649`, `:4656`, `:4660`, `:4661`, `:4706`, `:4710`.
3. `core.cast:363`, `:368`, `:439`, `:516` `operator[]` → `try_emplace` with structured bindings.
4. `setArchitecture` (`:2436-2458`): its five text tests become one lookup of the flag word.
**Validation:** CODING CRITICAL RULES (bail-out, brace init, `.at()`, string tests over three are a table).

### Step 14: Lean ladder on each crossing
**Scope:** every function the three sweeps list over 30 lines or over 3 branches that still exists after steps 1-13: engine (Query 8, Index `addUnitRows`, Processor 10, Relay, Server, Main, Validator, SwapValidator, StdioClientProbe) and cLang (23 over 30 lines, 11 over 3 branches, the 6 or-chain tables).
**Action:** COUNSELOR walks each crossing through the auditor.md Lean questions in order (removal, lookup, contract correction, responsibility split). COUNSELOR delegates the fix that the ladder finds, with the exact names and lines in the prompt. No split serves only to move lines (MANIFESTO L).
**Validation:** each fix cites its ladder answer. Or-chains of equality tests (`isCompletionScope`, `isTypedDeclaration`, `isSelfTypeDeclaration`, …) become lookup tables (MANIFESTO L, 3 branches).

**Ladder result.** The inventory after steps 1-13 has 68 crossings: 60 long bodies (A), 1 branch chain (B), 7 or-chains (C). COUNSELOR read each one. Fixes:
- S1 `Index::addSymbols`, Q1: the declaration and reference insert loops were identical. One `addOccurrences (symbols, pathIndex, blocks)` serves both. Q6: the `records` build is its own job, with its own state, so it moves to `addRecords`.
- S2 `Index::addUnitRows`, Q6: two jobs with separate state. `addPathRows` fills `pathRows` and `stemRows` (path → row). `addUnitRows` fills `unitRows` (file → owning unit). `Processor::addJobs` calls both.
- S3 `Processor::addDocument`, Q1: the readiness test repeated at `:1250` and `:1260`. It becomes one local lambda.
- S4 `Query`, Q1: the symbol-file and document lookup had four copies (`inlayHint`, `appendDiagnostics`, `appendFileCodeActions`, `documentSymbol`). It becomes one `applyToSymbolFile` template.
- S5 `SwapValidator::addRules`, Q1: three root-field rules and four token-field rules repeated one shape. Each set becomes a field table, one loop, and `hasFields`. The sibling form is the function-local table at `Validator.cpp:30-31`.
- C1 cLang, Q4: seven or-chains become `ling::LookupTable` blocks in the `definitionTypes` shape (`language.cast:2884-2889`, `construction.md:800-810`, `spell.md:561-563`).
- C2 cLang, Q1: the child-visit idiom had seven copies (`:3433`, `:3539`, `:4096`, `:4141`, `:4327`, `:4620`, `:4894`). It becomes `applyToChildren` and `findChild`.
- C3 cLang `getRankedHeaders`: the same pointer test appeared twice (`:3177`, `:3184`). It becomes one if/else.
- C4 cLang, Q1: the case-insensitive text compare had two copies (`isPreferredHeader` `:3166`, `addParameterHints` `:4742-4745`). It becomes one `isSameText`.

The B crossing, `getHoverDesugaredType` (`:4277-4280`), ends at Q7. Each arm calls a different libclang entry point, and those load at run time. A table would need forwarding helpers, which CODING forbids (*"No … unnecessary helpers"*). Every other crossing ends at Q7: its body holds one job (MANIFESTO L: *"Some classes are legitimately large as a single unit"*).

### Step 14b (mid-sprint, ARCHITECT-surfaced): the index pool does not take every core
**Evidence:** MACHINIST's run on END, reported by ARCHITECT on 2026-11-21: *"during the cold index the daemon ran at 837% CPU and used 7:48 of CPU time"*. ARCHITECT's process list: `ling daemon /` at 98.0% CPU and 4.9G.
**Cause:**
- With no `## daemon` row, `threads` stays `LINGModel::defaultThreads { 0 }` (`LINGModel.h:47`).
- The pool then takes the JUCE defaults: `numberOfThreads { SystemStats::getNumCpus() }` and `Thread::Priority::normal` (`juce_ThreadPool.h:195`, `:197`), at `Processor.cpp:13` and `:176`.
**Ruling:** ARCHITECT: *"never exhaust machine resources"*. ARCHITECT's clangd config runs half the cores (`~/.config/nvim/lua/lsp/clangd.lua:27`) at low priority, with the reason given there: *"keeps the indexer yielding to interactive work"* (`:32-38`).
**Action:**
1. `LINGModel::defaultThreads` (the owner of the default) becomes half the CPU count, at least 1.
2. Both pool sites build the pool with that count and `juce::Thread::Priority::low` (`juce_ThreadPool.h:177`, `:189`).
3. `HELP.md:183` states the new default.

### Phase 4 — working state

### Step 15: one build, then the acceptance harness
**Action:**
1. Run `cast cast/spell.md --no-sign` once, then delete `~/.config/ling/language/cLang/`.
2. Run T1-T29 on END and KANJUT over stdio (`PLAN-ling-lsp-0-tracker.md:318-336`).
3. Trace each failure to its cause with a read and a trace. A C-family cause is fixed in `language/cLang`. This closes the outstanding open items of tracker group A (`:384-491`) that the run reproduces.
4. Repeat until every answer test agrees with the code read.
**Validation:** tracker Step 1 validation (`:336`), except T9, T10 and T21.

### Step 15a (mid-sprint, ARCHITECT): code the build skips gets its answer from a compile context
**Ruling:** ARCHITECT, 2026-11-21: *"ling must deterministically answer test agrees with the code read"*, then `/go` on the outline of this step.
**Evidence:** the END T4 run after the harness fix has 19 positions with `ling=0`. Each one is in a branch that the `debug` lane skips:
- `jam_Listener.cpp` `#ifdef JUCE_LINUX` (`:116-273`) and `#ifdef JUCE_WINDOWS` (`:276-391`);
- `ConfigModel.cpp` `#if JAM_USING_PROFILER` (`:73`, `:82`, `:101`);
- `juce_Component.cpp` `#if JUCE_UNIT_TESTS` (`:3357-4301`).
No unit in `Builds/Debug/compile_commands.json` defines `JUCE_UNIT_TESTS`. The name fallback is deleted (`PLAN-ling-compiler-truth.md:13`, ruling 35), and nothing else answers a skipped branch.
**Base design:** `PLAN-ling-compiler-truth.md` Step 4 (`:108-127`) and its closed precondition (`:154-161`). `PLAN-ling-ast-once.md:19` deleted that step, and it records no ruling for the deletion. This step restores it and extends it to configuration conditions.
**Scope:** `language/cLang/language.cast`, `language/cLang/construction.md`, `language/cLang/spell.md`, `test/expected/end-definition.json`, `test/expected/end-methods.json`.
**Action:**
1. Platform keys and table:
   - `language.cast:70-72` gets `platformLinux`.
   - `construction.md` gets `## platformTargets` after `## moduleExtensions` (`:546-551`). It holds one row for each platform key, valued by its flag: `--target=x86_64-unknown-linux-gnu`, `--target=x86_64-pc-windows-msvc`, `--target=arm64-apple-macosx` (`PLAN-ling-compiler-truth.md:155-160`).
   - The spell row and the getter fence take the shape of `moduleExtensions` / `getModuleExtensions` (`spell.md:358-360`).
2. libclang rows under `## libclangSymbols` (`construction.md:393`), in the shape of `:397-403`: `getAllSkippedRanges` (`clang_getAllSkippedRanges`, `Index.h:560-561`) and `disposeSourceRangeList` (`clang_disposeSourceRangeList`, `CXSourceLocation.h:288`). `SourceRangeList` mirrors `CXSourceRangeList` (`CXSourceLocation.h:276-283`).
3. A context pass parses with `TranslationUnitFlags` `keepGoing = 0x200` (`Index.h:781-787`), because the other platform's system headers are missing on this machine.
4. `IndexContext` (`language.cast:2667-2881`) holds the ranges that this pass keeps, keyed by path. `isKept` holds for every location when the pass keeps no ranges (the main pass). Otherwise it holds only inside a kept range. `addSymbol` (`:2770`) and `addHover` (`:2820`) test it beside `isForeign`.
5. `indexSource` (`:5194-5253`), after the main pass:
   1. Read the main unit's skipped ranges. A unit with no skipped range gets no second parse.
   2. For each other platform row, parse and index once with its target flag. The pass keeps only ranges that are still kept. It sends records and hovers only (`addHovers`, `addChosenSymbols`). A range that this pass does not skip is removed from the kept set, so each range has one producer (MANIFESTO B).
   3. For the ranges still kept, read the directive that opens each range (precondition read below). One configuration pass defines each macro that an `#if`, `#ifdef` or `#elif` names, and undefines each macro that an `#ifndef` names. A range opened by `#else` gets the macros of its opening directive set to `0` in a second configuration pass.
   4. Every value is in a table or in the source. No macro name is in code.
6. Seeds: `Listener.cpp:226:46:mask` holds `bitwise.c:31`, a name guess. `mask` is a member of `inotify_event` (`:218`), a type in a Linux system header that this machine does not have. The code read gives `[]`.
**Precondition read (Librarian, before the Engineer prompt):** the location where a skipped range starts and ends, relative to its directive (`PPDirectives.cpp` `SkipExcludedConditionalBlock`; `CIndex.cpp` `clang_getAllSkippedRanges`).
**Inventory:** API "Skipped ranges of a unit", "Preprocessing record", "Platform key" (`PLAN-ling-compiler-truth.md:37-39`); Pattern "Platform table read", "libclang export row", "One occurrence once" (`:46-48`); Names "construction platform tables", "platform keys", "libclang export rows" (`:53-55`).
**Validation:**
- T4 and T14 on END: the 19 positions agree with the code read.
- A unit with no skipped range parses once.
- The engine has no change (T22).
- Step 16 measures the cost of the extra parses.

### Step 15b (mid-sprint): one identity for one declared entity across context passes
**Ruling:** *"ling must deterministically answer test agrees with the code read"* (2026-11-21); Step 15 action 3.
**Evidence:** `folder` at `jam_Listener.cpp:24:60` (Mac `Impl` member) answered `:264` and `:385`, the members of the Linux and Windows `File::Watcher::Impl` classes. The three members share one USR. The symbol hash is the USR alone (`language.cast:2850`, `:2904`).
**Action (done, read and validated):** `IndexContext::getSymbolHash (symbol, entity)` mixes the kept range of the entity's canonical declaration into the USR hash, during a context pass only (`language.cast:2814-2826`). The record, the hover, the type relation and `declarationFiles` use it (`:2867`, `:2921`, `:3558`, `:3874`). Each `addSymbol` call passes the cursor that it gives to `addHover`.
**Validation:** T4 `folder` 24:60 answers its own member only, on a build that embeds this library (Step 15d).

### Step 15c (mid-sprint, ARCHITECT): real platform sysroots for the context passes
**Ruling:** ARCHITECT, 2026-11-21, on the conflict between ruling 35 and the code-read ruling for `numRead` (`jam_Listener.cpp:215` → `:210`), `folderHandle` (`:289` → `:387`) and `path` (`:370` → `:361`): selected *"Real platform sysroots"*.
**Evidence:** without the platform SDK, Sema marks each declaration invalid, and each use becomes a `RecoveryExpr` (`SemaExpr.cpp:3209-3214`, release/19.x). No reference reaches the index (`IndexBody.cpp:140-145`).
**Action:**
1. Librarian: the source of a Linux sysroot and of the Windows SDK headers on macOS, and the clang flags that point a `--target` parse at them.
2. The platform table gives each target its sysroot flags. The context pass adds them (`getContextCommandLine`).
3. Seeds: `mask` (`jam_Listener.cpp:226:46`) then answers the `inotify_event` member in the Linux sysroot header. Its seed follows that read.
**Validation:** T4 `numRead`, `folderHandle`, `path`, `mask` agree with the code read.
**Descoped (ARCHITECT, 2026-11-21):** *"File::Watcher being rewrite on parallel session on windows. i dont think those 4 significant enough to pull 200mb. i will pull/merge remote changes shortly"*. The 4 T4 keys are read again against `jam_Listener.cpp` after the merge.

### Step 15d (mid-sprint, ARCHITECT): one build embeds its own language library
**Ruling:** ARCHITECT, 2026-11-21: `cast/cmake.cast` *"Add to scope"*.
**Evidence:** `Source/languages/macOS/cLang.zip` holds `liblanguage.dylib` sha256 `e0d752f3…`. The daemon unpacks `3ef62a62…` from the binary (`Processor.cpp:436-467`). The binary data and `CAST_LANGUAGES_HASH` come from `cmake.cast:192-209`; the pack runs at `:456-462`.
**Scope:** `cast/cmake.cast` and the spell rows that place its fences.
**Action:** read the cmake order end to end. Correct it so the binary data of one build holds the library that the same build packs.
**Validation:** after one build, the unpacked library equals `Builds/languages/cLang/build/liblanguage.dylib`.
**Result (done, no change):** the evidence above was wrong. `3ef62a62…` was the library of an older build: daemon pid 95876 (started 2026-11-20 07:14) maps `~/.cache/ling/language/7d38fde1…/cLang/liblanguage.dylib` (lsof). A daemon of the current binary maps `cf4e3760…/cLang/liblanguage.dylib` (lsof at 5, 20, 45 s). The unpacked library, `Builds/languages/cLang/build/liblanguage.dylib` and the `cLang.zip` entry all have sha256 `e0d752f3…`. The cmake order is correct. `cast/cmake.cast` needs no change.

### Step 15e (mid-sprint): the daemon indexes C++ after it writes the default language folder
**Ruling:** Step 15 action 3; ARCHITECT's standing step *"always delete on disk, let the binary build and replace"*.
**Evidence:** daemons that wrote `~/.config/ling/language/cLang/` at start (05:25, 06:12) queued no C++ job; every pool thread waits (`scratchpad/sample-attach.result:437-496`). With the folder present (06:31), END indexed in 200 s.
**Action:** find the cause from a runtime read of `addLanguages` → `setLanguage` → the folder watcher (`Processor.cpp:616-647`, `LINGModel.cpp:84-101`, `:179-240`). Fix it at the owner.
**Validation:** delete the folder, start the daemon: C++ units index, T4 completes.
**Cause (read):**
1. Each live daemon watches `~/.config/ling/language/cLang` (`LINGModel.cpp:205-207`). A file event there sends `Id::path` on the language row (`:229-231`), so the daemon calls `setLanguage` and then `getOrCreateLanguages`.
2. When the folder of the daemon's own hash is absent, `getOrCreateLanguages` unpacks it. Then it deletes every other hash folder (`Processor.cpp:467-482`).
3. A daemon of an older build stays alive while a client holds a connection, because only the timeout ends it (`Server.h:76-81`, `Server.cpp:81-95`). At 07:12:09 the old daemon (pid 95876) unpacked `7d38…` again and deleted `cf4e…` (cache listing before the 07:14 probe).
4. The old daemon outlives its build. This breaks MANIFESTO B: *"No object outlives its owner"*.
**Fix:** the daemon quits through its quit path (`Server::timerCallback`) when its own executable file changes. `Server` watches the executable folder with the `Client` pattern (`Client.cpp:8-13`, `:50-60`). After the quit, the relay reports `failDaemonLost` (`Client.cpp:62-67`).

### Step 15f (mid-sprint): deterministic answers
**Ruling:** *"ling must deterministically answer"*; MANIFESTO D.
**Evidence:** `folder` 24:60 disagreed at 05:44 and agreed at 06:35 with the same library `3ef62a62…`.
**Action:** trace the data flow of that answer across two runs. Fix the BLESSE violation that the trace names.
**Validation:** two warm runs give identical `end-definition-out.json` answers.

### Step 15g (mid-sprint): hover doc of a definition whose own comment is not a doc
**Ruling:** *"Keep clangd hover"* (`PLAN-ling-ast-once.md:15`).
**Evidence:** `setTabName` (`jam_TabbedComponent.cpp:61:23`) has no doc. With `-fparse-all-comments`, the `//====` line attaches to the definition, and the redeclaration-chain cache returns it for the canonical cursor as well (`ASTContext.cpp:461-536`, release/17.x and 19.x). clangd takes the doc from its index (`Hover.cpp:334-355`).
**Action:** runtime read of the comment cache order in the unit, then the fix at the hover owner.
**Validation:** hover `setTabName` shows the `@brief` of `jam_TabbedComponent.h`.

### Phase 5 — bottleneck last

### Step 16: profile against clangd
**Action:** T9, T10 and T21 through the tracker's scout manifest form (`PLAN-ling-lsp-0-tracker.md:324-333`). Each miss goes to the cost site that the report names. `PLAN-ling-ast-once.md` step 3 (re-profile of the cold index) runs here.
**Validation:** T9 for each method at the median and at p95, T10 and T21 are all below clangd.

## BLESSED Alignment
- **B:** Index owns `records` and `stemRows`, built at bind or at the files update and erased at unbind. The library owns its foreign cache for one index pass.
- **L:** direct lookups replace each whole-file and whole-index scan (steps 1-8). Step 14 runs the ladder.
- **E:** `isForeign` is the one test. No silent drop is left in the engine (step 9).
- **S (SSOT):** the build's facts stay the source: `compile_commands.json`, `.ninja_deps`, the PCH unit. Each foreign test lives in one place.
- **S (Stateless):** the work maps and sets exist only during creation or a state update (MANIFESTO:254).
- **E (Encapsulation):** the engine still names no language (T22). `isForeign` is a generic predicate.
- **D:** the harness proves the same answers (step 15) before any profile (step 16).

## Risks / Open Questions
None.
