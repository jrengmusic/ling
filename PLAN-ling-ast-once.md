# PLAN: ling builds each unit's data once and reads it in place

**Date:** 2026-11-20
**BLESSED Compliance:** each step is checked against MANIFESTO E "Complete at Creation".
**Language Constraints:** C++17 / JUCE / JAM engine (`Source/`). cLang library: `language/cLang/language.cast` + `construction.md` + `spell.md` (cast). JAM: `jam_core/document`, `jam_syntax_tree/document`.

## Context

ARCHITECT, this session:
- *"complete AST must be build ONCE, as fast as it can, memoized, cached, whatever. validate, all consumer never mutate, state update is not mutation."*
- *"what we have now, it tool WHOLE RESOURCES to build AST, with UNACCEPTABLE time"*
- *"NO NEW STRUCT. WE DONT NEED ANY NEW STRUCT."*
- *"why hover text created on demand? it only referenced text alrady exist, formatted, whatever, nothing need to be rebuild"* / *"I AM RIGHT"*
- *"if Document and Model cant hold EVERYTHING for the DATA STRUCTURE, you already violate the ARCHITECTURE"*
- On the hover conflict below, ARCHITECT selected *"Keep clangd hover"*. Option text: "T12 and ruling 30 hold. Delete step 2. The hover stays built once at creation and read in place. Step 1 measures where the cold-index CPU goes, and the next step follows from that measurement."

MANIFESTO E, Complete at Creation: *"A parse operation gives a complete AST … Both are one Model, built once, read in place."* It also forbids *"A second copy of the same complete data … A copy per hop."*

The step 4 platform passes of `PLAN-ling-compiler-truth.md` are deleted. The `inactive` role bit is deleted. Both are verified at `language.cast:5133-5190` and `Index.cpp:386-391`.

## The data path today (read this session)

| Hop | What happens | Evidence |
|---|---|---|
| 1. Compile | libclang parses the unit. The translation unit is the complete AST. | `language.cast:5148` |
| 2. Walk | `onDeclaration` and `onReference` copy each occurrence into `IndexContext` rows: `recordRows`, `records`, `typeSymbols`, `hoverPositions`, `cursors`. | `language.cast:2756-2762`, `2791-2795` |
| 3. Hover text | `addHovers` builds one hover string for each cursor and file. `getOrCreateHoverText` keeps one string for each cursor (`hoverTexts`). The inputs are:<br>• `getHover`, `getHoverLines`, `getHoverValue` (`libclang.evaluate`);<br>• `getHoverProvider` (include-graph ranking);<br>• `passings` (a cursor walk of every call). | `language.cast:5035-5052`, `5014-5031`, `4398-4413`, `4305-4334` |
| 4. Choice | `addChosenSymbols` sorts each file's rows, then sends one record for each position. A hover crosses the boundary once for each record file and hover text (`hoverIndices`). | `language.cast:4513-4536`, `4476-4504` |
| 5. Callbacks | Each record, hover string, outline, completion, hint and diagnostic becomes an element or Token in the job's `SymbolDocument`. | `Processor.cpp:888-1075`, `911-933` |
| 6. Swap write | The `SymbolDocument` is written to `<file>.symbols`. | `Processor.cpp:860`, `Index.cpp:82-94` |
| 7. Swap read | On a hash match, `SwapReader` decodes the file into a new tree and copies every string. | `Index.cpp:57-72`, `jam_Document.cpp:1217-1223`, `1237-1266` |
| 8. Index maps | `addSymbols` inserts a pointer to each record into `declarations` and `references`. Each insert is a sorted insert. | `Index.cpp:357-379`, `393-403` |
| 9. Query | Hover reads the stored string in place, `Id::hovers[Id::hover]`. It also reads the definition text in place from the source Document, through the stored range. | `Query.cpp:483-497` |

Hops 2 to 8 are not measured. Their share of the CPU is unknown. Step 1 measures it.

## Framework Inventory

### API
| Need | API | Source | Step |
|---|---|---|---|
| Profile a run | `scout <manifest>` with `## sample` | profile skill | 1, 3 |
| Build | `cast cast/spell.md --no-sign` | ARCHITECT's ruling | 3 |

### Pattern usage
| Pattern | Exemplar | Step |
|---|---|---|
| A scout manifest with a `## sample` table and a `## toolchain` table | profile skill, manifest grammar | 1 |

### Names
No new name.

### Absence
| Need | Query | Result |
|---|---|---|
| A reader of `IndexContext::mainFile` | trace of every `IndexContext` member (this session) | none. It is written at `language.cast:5166` and never read |

## Validation Gate

COUNSELOR validates each step against MANIFESTO, CODING, NAMES and this PLAN before the next one starts. ARCHITECT: *"dont abuse build for code validation."* The only builds are the one in step 3 and the profile build in step 1. Audit and doxygen stay on hold: *"hold audit and doxygen. until i explicitly said so."*

## Steps

### Step 1: measure where the cold index spends its CPU
- **Action:** write one scout manifest for the KANJUT cold index, with a `## sample` table, on the current build. The current build has no platform passes.
- **Output:** the stack share of each of these:
  - libclang parse;
  - `indexTranslationUnit` callbacks;
  - `addHovers` and its callees;
  - `addChosenSymbols`;
  - `IndexJob` callbacks;
  - swap write;
  - `Index::addSymbols`.
- **Validation:** each later step cites the share that this run shows. No change is made before the measurement. A step that the measurement selects is added to this plan with its citation.

- **Result (2026-11-20, `benchmark/kanjut-cold-sample-report.md`, Release, 1 epoch, exit 0):**
  - Zones: `run parse job` 1.167e11 ns over 5561 calls; `run index job` 2.393e12 ns over 247 calls; `add symbols` 1.906e9 ns over 253 calls. `user` 1.153e12 ns, `system` 1.888e11 ns. The sampler load is in these rows.
  - Stack samples inside the index job: `add unit records` 103779; `clang parse translation unit2 full argv` callback 13177; `clang index translation unit` callback 5488.
  - Inside `add unit records`: `add chosen symbols` lambda 90898, `add position symbols` 90714, **`get definition range` 82338**; `add hovers` 11166 (`get hover provider` 7596, `get ranked headers` 7517, `add physical headers` 6858, `get hover` 2375).
  - Inside `get definition range`: `clang tokenize` 39099 (`get tokens` 36980); `clang get token extent` 35478 (`measure token length` 29471).
  - `Index::addSymbols`, swap write and swap read are small: `add bindings` 83, `write swap file` 757, `read swap file` 576.
- **Selected by the measurement:** `getDefinitionRange` is 79% of `addUnitRecords` (82338 / 103779).
  1. It runs once for each record file and hover text (`hoverKey`, `language.cast:4482-4488`). The definition range belongs to the declaration cursor, so the same range is derived again in each file that refers to it. MANIFESTO E: *"no downstream consumer … re-derives what that operation already produced"*.
  2. For a body, a scope declaration, or a long initializer, it tokenizes the whole extent, body included (`:4118-4123`). It then calls `getTokenExtent` on each token until the body start (`:4157-4159`). The cut lies before the body (`bodyStart`, `:4109-4112`).

### Step 1b: one definition range for each declaration (selected by step 1)
ARCHITECT: *"/go"* after the step 1 result.
- **Scope:** `language/cLang/language.cast`, `language/cLang/spell.md`.
- **Action:**
  1. `getOrCreateDefinition (IndexContext&, Cursor)` returns the row of the declaration's definition in `IndexContext::definitions`. It derives the row once for each cursor: `getDefinitionRange`, the two spelling locations, `hasDefinitionText`, the path text. The cache is `IndexContext::definitionIndices`, keyed by `hashCursor` and checked with `equalCursors`. Pattern: `getOrCreateHoverText` (`language.cast:5014-5031`).
  2. `addPositionSymbols` reads the row in place of its own derivation (`:4487-4501`).
  3. `getDefinitionRange` tokenizes only `[extent start, bodyStart]` when the cursor has a body. The lexer stops at the range end (`CIndex.cpp:7770-7812`), so the `{` at `bodyStart` is lexed only when whitespace precedes it. The cut is the token at `bodyStart` when it is lexed, else the end of the last token.
- **Names:** `getOrCreateDefinition` joins `getOrCreateHoverText`, `getOrCreateProvider`. `definitions` and `definitionIndices` join `hoverPositions`, `hoverIndices`. `definitionStride` joins `hoverPositionStride` (`:607`).
- **Validation:** the hover and methods harnesses on END and KANJUT give the same results as before. The step 1 manifest runs again, and `get definition range` falls.
- **Result (re-profile, `benchmark/kanjut-cold-sample-report.md`):** `get definition range` has 89332 samples against 82338 at step 1. The cache removed no calls. The cost of each branch inside the function is not yet measured.

### Step 1c: symbols lane for the carried language libraries (added 2026-11-20)
ARCHITECT selected *"Add symbols lane"*. Question text: "Do you widen the scope to those four files, or close step 1c at the measured hotspot?"
- **Why:** the cLang and core libraries build Release-only with no debug info (`language/cLang/spell.md:845-851`, `language/core/spell.md:27-33`). The samples inside `getDefinitionRange` show no lines.
- **Scope:** `language/cLang/spell.md`, `language/core/spell.md`, `cast/cmake.cast` (`language-bundle`, `:433-462`), `profile-info.md:6`.
- **Action:**
  1. Add `symbols` toolchain rows to each language `spell.md`. They keep `-DCMAKE_BUILD_TYPE=Release` and add `-g` (`CMAKE_CXX_FLAGS`; core also `CMAKE_C_FLAGS`), so the optimization under measurement does not change.
  2. In `language-bundle`, `CAST_LANE` becomes `CAST_LANGUAGE_LANE` through `list(TRANSFORM … PREPEND "--")`. Both `cast spell.md` calls pass it. An empty `CAST_LANE` adds no argument.
  3. `profile-info.md:6` runs `-DCAST_LANE=symbols -P languageBundle.cmake`.
- **Pattern:** END `plugin-bundle` (`end/cast/cmake.cast:562-564`) and `end/profile-info.md:6`; EVE symbols lanes (profile skill, Builds).
- **Validation:** the step 1 manifest runs again. The samples inside `getDefinitionRange` resolve to lines of the generated `language.cpp`.
- **Result (2026-11-20):**
  - scout cannot give lines. Its sampler keys a frame by symbol name only (`scout/Source/Symbolizer.h:100-101`). The whatdbg sample driver prints `file:line` per frame (`trace/driver.lua:134-141`).
  - Cold scout run (`kanjut-cold-sample-report.md:10-18`, against the step 1 baseline): epoch 1.70e11 ns against 2.76e11 ns; `run index job` -39.7%; `get definition range` 79864 samples (`:3706`).
  - whatdbg sample, attached to the profile workload (101 samples, 72 in `getDefinitionRange` on the index thread):
    - `language.cpp:7518` (`language.cast:4125`, `tokenize`): 43 samples, callee `clang_tokenize`.
    - `language.cpp:7553` (`language.cast:4159-4161`, the cut loop): 29 samples, of which 25 in `clang_getTokenExtent`. Only the `hasBody` arm calls `getTokenExtent` there.

### Step 1d: the body cut reads the last token only (selected by step 1c)
- **Why:** with a body, the token range ends at `bodyStart` (`language.cast:4125`). The lexer stops at the range end (`CIndex.cpp:7770-7812`). Thus only the last token can start at `bodyStart`. The loop at `:4159-4161` calls `getTokenExtent` on every token to find it.
- **Scope:** `language/cLang/language.cast`.
- **Action:** split the `else` at `:4157-4162`. With `hasBody`, `cutIndex` is `tokenCount - 1` when `tokenCount > 0` and the last token starts at `bodyStart`, else `tokenCount`. Without a body, the loop stays as it is, with the `isTokenByte` test only.
- **Validation:** `:4166` gives the same range as before, because it reads `tokens[cutIndex - 1]` in both cases. The hover and methods harnesses on KANJUT give the same results as before.

### Step 1e: split the `tokenize` cost by branch
- **Action:** a whatdbg trace on the current build, with a breakpoint after `tokenize` (`language.cpp:7520`). It records `cursor.Type`, `hasBody` and `tokenCount` at each stop.
- **Output:** the branch that holds the token count. A change follows only from that output.
- **Result (2026-11-20):** two attach traces, breakpoints at `language.cpp:7520` and `:7518`. Both were `verified=true`. Both ended `outcome=timeout stops=0`, and ling exited 0. The sampler records 43 frames at `:7518`, so the line runs. The breakpoint does not stop it in this Release `-g` library. The split by branch is not measured. Step 3 measures what remains after step 1d.

### Step 1a: defects that block the step 1 run (added 2026-11-20)
ARCHITECT answered *"/go"* to: "Do I fix all three in this sprint, then measure KANJUT?"
1. The profile lane had no `cmake -P languageBundle.cmake` row (`profile-info.md:6-10`). The siblings have it (`project-info.md:218`, `:228`, `:236`). Every profile build carried a stale `cLang.zip` (`CMakeLists.txt:394-398`). Fixed: the row is now `profile-info.md:6`.
2. `juce_audio_plugin_client_VST3.mm: dependencies not found` on the cold KANJUT run. Each overlay message sets `EXIT_FAILURE` (`Main.cpp:212-217`), so scout reports `command failed`.
3. `Source/layout/DefaultSettings.xml: file did not parse` on the same run.
4. `!! killing thread by force !!` at shutdown (`juce_ThreadPool.cpp:130-143`, `juce_Thread.cpp:269`). Tracker open item 6 (`PLAN-ling-lsp-0-tracker.md:390`).
- **Findings (2026-11-20):**
  - Item 2 is a true diagnostic. `ninja -C Builds/Debug -t deps` prints `ninja: warning: premature end of file; recovering` and `deps not found` for the Standalone VST3 and VST2 objects and for the VST3-target VST3 object. The KANJUT `Builds/Debug/.ninja_deps` is truncated. ARCHITECT: *"delegate Pathfinder to run it"*.
  - Item 3 is a true diagnostic. `DefaultSettings.xml:3` is `<@product@ version="1.0.0">`, a cast template, and `@` is not legal in an XML name.
  - Items 2 and 3 failed the run only through the exit code. ARCHITECT selected *"HELP holds"*. Option text: "A project diagnostic is printed on stderr but does not set the exit code of the daemon run or the profile workload. Only the operation and command-line runs fail on a message." Fixed: `Main.cpp:212-216` prints the message and sets no exit code. The other runs set their own code: `Operation.cpp:44`, `:66`; `Client.cpp:33`, `:65`; `Relay.cpp:349`.
  - Item 4: an index job checks `shouldExit` only inside the libclang index pass (`language.cast:5155-5160`). The parse (`:5148`) and `addUnitRecords` (`:5119-5128`) do not check it. Which thread JUCE kills is not yet read.
- **Validation:** the step 1 scout run exits with 0 and writes its report.

### Step 2: delete the dead `mainFile` member, and indent the moved `IndexerCallbacks` initializer
- **Action:**
  - Delete `IndexContext::mainFile` (`language.cast:2683`) and its write (`:5166`).
  - Re-indent `:5155-5175` by four spaces.

### Step 3: build and measure
1. Run `cast cast/spell.md --no-sign` once, then delete `~/.config/ling/language/cLang/`.
2. Run the step 1 scout manifest again, and compare it with the step 1 baseline.
3. Run the methods and hover harnesses on END and KANJUT.

## BLESSED Alignment
- **E (Complete at Creation):** the hover string is created once for each cursor and file (`language.cast:5014-5031`). The query reads it in place (`Query.cpp:485`). Step 1 finds the hop that costs the CPU, before any change.
- **L:** step 2 removes one dead member.
- **D:** the measurement runs on a fixed manifest, so two runs compare.

## Risks / Open Questions
None.

## Closed by read
- **Hover content.** ARCHITECT selected *"Keep clangd hover"*. The facts behind the question:
  - Only the comment and the definition are source text. The query reads the definition in place (`Query.cpp:489-497`).
  - All other hover content is a compiler value that no source holds (`language.cast:4338-4413`): the hover word and name; the return type, parameters and type (`:4357-4359`); the enumerator value and the evaluated value with its hex form (`:4305-4334`, `:4360-4361`); the passing note (`:4362`) and the provider (`:4408`); the scope, access and `#define` block (`:4371-4394`).
  - T12 measures hover against clangd `textDocument/hover` (`PLAN-ling-lsp-0-tracker.md:276`). Ruling 30 adds more compiler-only hover lines, such as `Size:`, `Offset:` and alignment (`tracker:483`).
  - The hover string is built once for each cursor and file, at creation (`language.cast:5014-5031`). It crosses the boundary once for each record file (`:4482-4501`). MANIFESTO permits this: *"Creation can use multiple passes … work buffers, and memory allocation"* (:254).
- **System headers have a Document.** This corrects an earlier risk, which said that a declaration in a system header, for example `memcpy` in `_string.h`, has no Document. That premise was wrong.
  1. Each dependency path becomes a `file` row with no path filter (`Processor.cpp:1376-1377`, `1408-1416`; `language.cast:2581-2591`).
  2. The dependency paths come from `ninja -t deps` (`construction.md:347-350`).
  3. The ling, END and KANJUT build rules all compile with `-MD`, and none with `-MMD` (`ling Builds/Release/CMakeFiles/rules.ninja:27,37,56`; `end Builds/Debug/CMakeFiles/rules.ninja`, 5 rules; `jreng-filter-strip Builds/Debug/CMakeFiles/rules.ninja`, 9 rules).
  4. The compiler states: `-MD  Write a depfile containing user and system headers` (`/usr/bin/c++ --help`).
  5. A file outside the project roots gets a hashed swap in the home cache (`LINGModel.cpp:168-173`).
  6. A row has no Document only when its family is not loaded, the file is missing, the job is stale, or the parse returns null (`Processor.cpp:534`, `724-759`, `1172`, `1203-1204`; `Index.cpp:100-107`).
- **Swap read.** `SwapReader` decodes `.symbols` into a new tree (`jam_Document.cpp:1237-1266`). This is a creation from the OS boundary. MANIFESTO permits it as copy 1, *"`read` or `write` into the buffer of the owner"*, and creation may *"use multiple passes … work buffers, and memory allocation"* (:254). It is not a violation.
- **Index occurrence maps.** `declarations` and `references` hold pointers into the `SymbolDocument` records, so they do not copy the data (`Index.cpp:401`). Step 1 measures the sorted-insert cost (`Index.cpp:361-377`). It is not a question.
