# PLAN: Binding from the build toolchain, built once, looked up in O(1)

**RFC:** none. The objective comes from ARCHITECT's discussion of 2026-11-14.
**Date:** 2026-11-14
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8 / JAM. cast generates and builds. The language library is plain C++17, with no JAM and no JUCE.
**Status:** Locked 2026-11-14 by ARCHITECT's `/go`.

## Overview

LING binds each name use to its declaration with the build toolchain's own clang front end (libclang). It runs once per unit, with the unit's exact compile command. Each record is stored once, kept in the swap, and rebuilt only when a file of the unit changes. A query looks up the symbol in O(1). Every compiler word and every toolchain rule is runtime data in the language folder, where disk wins. The engine names no compiler and no library.

## ARCHITECT's words that govern this plan

- *"GENERAL UNIVERSAL ISOMORPHIC ABSTRACTION LANGUAGE AGNOSTIC. this is deterministic pattern. not harcoded any lib, any compiler into LING's ENGINE."*
- *"why compiled in rows? i told you this must data-driven runtime, user configurable, user extensible"*
- *"built AST once, memoized, cached - update as needed - O(1) to O(log n)"*
- *"we dont need to build parallel shadow state of what compiler built to BUILD THE BINARY. what actually can we use deterministically?"*
- *"how can we use project chosen compiler to build index deterministically, FASTEST, robust and reliable"*
- *"just implement correctly, we will prove there"*, and: never raise Windows as an issue. The Windows code is written from the same facts, and ARCHITECT proves it.
- `/go` on the design that COUNSELOR stated: the build toolchain's libclang, run once per unit with its exact command, records keyed by symbol, swap, dependency record, O(1).

## Evidence (this session)

- Probe `scratchpad/libclang/session-cc1.log`: Xcode toolchain `libclang.dylib`, END's exact commands, 0 diagnostics.
  - `Session.cpp:40:33` and `:141:22` give `c:@S@PluginState@F@setRows#&1$@N@juce@S@ValueTree#`.
  - `ENDView.cpp:360:36` gives `c:@S@TabView@F@loadPlugin#`.
- Probe `units-w10.log`: all 55 END units in 24.57 s wall time, peak 818,429,952 bytes.
- The three toolchain rules, each shown by a failing run and then a passing run:
  1. **The library sits beside the build compiler.** On macOS it is `<InstalledDir>/../lib/libclang.dylib`. On Windows it is `libclang.dll` in the folder of `clang-cl.exe` (ARCHITECT).
  2. **`argv[0]` is the real driver.** `/usr/bin/c++` failed with `stdarg.h` not found. The rule keeps each unit's own driver: `cc` becomes `<InstalledDir>/clang` and `c++` becomes `<InstalledDir>/clang++` (`compile_commands.json:142`, `:148`, `:154`). A C unit compiled as C++ gave 20 errors (`juce_graphics_Sheenbidi.c`).
  3. **macOS needs the SDK that the shim supplies.** Without it the parse failed with `TargetConditionals.h` not found. `xcrun --show-sdk-path` gives the SDK that the dependency record lists.
- libclang C API, declarations read from `Index.h` (LLVM 23.1.5, CINDEX 0.64):
  - `clang_indexSourceFileFullArgv` (`:6693-6698`);
  - `IndexerCallbacks` (`:6498-6544`);
  - `CXIdxEntityRefInfo` (`:6461-6492`);
  - `CXIdxDeclInfo` (`:6341-6365`);
  - `CXIdxEntityInfo.USR` (`:6322`);
  - `clang_indexLoc_getFileLocation` (`:6720-6732`), which also gives the byte offset;
  - `CXIndexOpt_SkipParsedBodiesInSession` (`:6649-6653`).

## Language / Framework Constraints

- **Tracker contract (`PLAN-ling-0-tracker.md:40-52`):**
  - JAM first, then JUCE.
  - Text is `std::string_view`, `jam::Format` and `jam::Strings`.
  - Builds run through cast.
- **The language library includes nothing from JAM or JUCE.** Its boundary is C functions (`jam_LanguageInterface.h`). It may hold the platform loader calls, as its export block already holds `#if defined (_WIN32)` (`language.cast:1184-1188`).
- **CODING.md:**
  - One object, one translation unit.
  - No comments or doxygen before the audit.
  - No `==` on strings.
  - Lookup tables in place of branch chains.
- **Refactor-Rewrite Discipline:** delete first. Old code never stays beside new code.

## Dependency & API Inventory

| Need | API | Evidence |
|---|---|---|
| Library entry by name | `LanguageEntry`, `ling_get_language` | `jam_LanguageInterface.h` (`LanguageEntry`); `language.cast:1190-1208` |
| Library load | `LanguageLibrary::getLanguageLibrary`, fields read once | `jam_LanguageLibrary.cpp:3-23`, `:75-88` |
| Callback boundary | `LanguageCallbacks` (context plus `add*` functions) | `jam_LanguageInterface.h` (`LanguageCallbacks`) |
| Symbol identifier | `Id::symbol` exists | `ling/cast/identifiers.md:65` |
| Entry identifier | `index` exists | `jam/cast/core.md:99` |
| Swap write and read of every value type, `juce::String` included | `Document::write` / `read` | `jam_Document.cpp:998-1048`; `writePayload` for `juce::String` at `:792` |
| Parse job, swap, path change | `Processor::addJob`, `readSwapFile`, `writeSwapFile`, `addDocument`, `valueTreePropertyChanged` | `Processor.cpp:1034-1140`, `:998-1032` |
| Unit rows with exact arguments, and dependency lists | `Id::arguments`, `Id::directory`, `Id::dependencies` | `CompilerManifest.cpp:146-157`, `:271-272` |
| Language folder: copied when absent, disk wins | `Processor::saveToPath` | `Processor.cpp:888-905` |
| Language rebuild on a folder change | `addBuild` / `applyBuild` / `setLanguage` | `Processor.cpp:830-936` |
| Runtime table read | `jam::MarkdownDocument::parse`, `getTables`, `getTableRows`, `getTableValue` | `CompilerManifest.cpp:10-15`, `Manifest.cpp:75` |
| O(1) map | `jam::HashMap` | tracker:50 |

## Validation Gate

COUNSELOR validates each step before the next one. The checks are against MANIFESTO.md (BLESSED), NAMES.md, CODING.md and this PLAN, with no deviation and no scope drift. @Auditor runs ONCE, after the final step. One cast build runs only where a step says "Build".

## Steps

### Step 1: Compiler words become runtime data in the language folder
**Scope:**
- delete `ling/cast/c_language.md`, its `@cLanguage` index row (`cast/spell.md:25`), and the generated `text::CLanguage` and `map::cLanguageCheckFlags`;
- `language/cLang/construction.md`;
- `Source/CompilerManifest.h/.cpp`;
- `Source/Processor.cpp` (`getCheckArguments`, `saveToPath`).

**Action:**
1. Move the rows of `c_language.md` verbatim into new sections of `language/cLang/construction.md`: `## compilerManifest`, `## checkFlags mac` and `## checkFlags windows`. ARCHITECT, on audit finding #11: "Keep new names".
2. Make `saveToPath` a static function that `CompilerManifest::getFiles` calls before it reads the tables, so that the default files exist on disk first.
3. `CompilerManifest` reads those sections at run time from `LINGModel::getConfigDirectory()/language/<family name>/construction.md`, with `jam::MarkdownDocument`. That replaces each `text::CLanguage::*` and `map::cLanguageCheckFlags` use.
4. The check flags are stored on the language row for `getCheckArguments`. They are not compiled in.

**Validation:**
- No `CLanguage` word and no C-family literal remains in `Source/`.
- Editing a row in `~/.config/ling/language/cLang/construction.md` changes the behaviour with no rebuild of LING.

### Step 2: The index entry in the boundary header and the library loader
**Scope:** `jam_LanguageInterface.h`, `jam_LanguageLibrary.h/.cpp`.
**Action:**
1. Add `LanguageEntry::index { "index" }`.
2. Add `using LanguageIndex = int (const char* const* arguments, int argumentCount, const char* directory, const LanguageCallbacks& callbacks);` in the form of `LanguageBuild`.
3. Append `void (*addSymbol) (void* context, const char* path, int offset, const char* symbol, int role) { nullptr };` to `LanguageCallbacks`. The role is `LanguageRole` bits.
4. `LanguageLibrary` reads `index` beside `build` and adds it to the `entryNames` assert.

**Validation:** the names come from the families `LanguageEntry::build`, `LanguageBuild` and `add*` (NAMES Rule 5).

### Step 3: The `cLang` library binds with the toolchain's libclang
**Scope:** `language/cLang/language.cast`, `language/cLang/construction.md`, `language/cLang/spell.md`.
**Action:**
1. Write an `index` function. It loads libclang once (`dlopen` / `LoadLibraryA`) from the path that the rule tables give.
2. Declare the C API types from the `Index.h` lines cited above.
3. Run `clang_indexSourceFileFullArgv` with `SkipParsedBodiesInSession | IndexFunctionLocalSymbols` on one index action for each process.
4. For each declaration and each reference, call `addSymbol` with the file path, the byte offset (`clang_indexLoc_getFileLocation`), the USR, and the role (declaration, reference, call, base).
5. The three toolchain rules are tables in `construction.md`, keyed by platform: the library path from the compiler, the `argv[0]` driver mapping, and the SDK command. The template reads them as lookups, not as branches.

**Validation:**
- The library includes nothing from JAM or JUCE.
- Every literal is a row or a named constant.

### Step 4: Binding job per unit, records in the swap
**Scope:** `Source/Processor.h/.cpp`.
**Action:**
1. After the parse jobs of the FILES rows, each unit row (one with `Id::arguments`) gets a binding job on `pool`, through `language->index`.
2. The job collects `(path, offset, symbol, role)` records.
3. On the message thread, `Id::symbol` is added to the element whose name token begins at that offset in that path's document.
4. The offset-to-element table of a document is built once per document, one walk, in a `jam::HashMap`.
5. A record is kept once per (path, offset).
6. The documents are written back to their swaps. `Document::write` already writes `juce::String`.
7. A change of a file runs the binding jobs again only for the units whose `Id::dependencies` hold that file.

**Validation:**
- Event chain only, with no wait.
- Two runs give byte-equal swaps (MANIFESTO **D**).

### Step 5: O(1) lookups by symbol, and the heuristic code removed
**Scope:** `Source/Processor.h/.cpp`, `language/cLang/language.cast`, `jam_LanguageDocument.h/.cpp`, `jam_LanguageInterface.h`, `jam_LanguageValidator.cpp`.
**Action:**
1. Delete first:
   - the object links: `addObject`, `Id::object` use, `getObjectPosition`, `addObjects`, and the `addReferences` object link;
   - `getMemberPaths` (both), `getClassPaths`, `applyToMembers`;
   - the by-name fallback in `applyToDeclarations`;
   - the `getDeclarationPaths`, `applyToReferences (path)` and `isReferenceTo` sweeps.
2. At `setDocuments`, build two `jam::HashMap<juce::String, Array<pair<int, const Element*>>>`: declarations by symbol, and references by symbol.
3. `goToDefinition`, `findReferences`, `hover`, `goToImplementation`, `incomingCalls`, `outgoingCalls` and `prepareCallHierarchy` look up the symbol of the element at the position.

**Validation:**
- The compiler finds no caller of a deleted function.
- No query walks all documents.

### Step 6: Build and runtime check
**Action:**
1. Build: `cast cast/spell.md --debug` in `ling/`.
2. Remove the swap roots.
3. Run `ling end/project-info.md --debug`.
4. Run the MACHINIST cases:
   - `findReferences` on `PluginState.h:41` gives `Session.cpp:40` and `:141`, plus the declaration;
   - `findReferences` on `Session::setRows` does not give `Session.cpp:40`;
   - `goToDefinition` at `ENDView.cpp:360` gives `TabView::loadPlugin`.
5. Run twice and compare byte for byte.
6. Run the time of the full index.

**Validation:** the three MACHINIST cases are correct, and the outputs are byte-equal.

### Step 7: Audit, doxygen pass, log
**Action:**
1. @Auditor runs once.
2. The findings are resolved.
3. The Pathfinder doxygen pass runs on the changed headers.
4. The sprint is logged per `/log`.
5. The commit messages go in chat.

## BLESSED Alignment

- **B:** one owner each. `LanguageLibrary` owns libclang, through the library. `Processor` owns the jobs and the two maps.
- **L:** the toolchain rules are tables, not branches.
- **E:** a failed index sets the overlay message with the unit path.
- **S:** the toolchain's own clang is the one truth for binding. The records are stored once, per (path, offset).
- **S (Stateless):** the maps are Model state, rebuilt as a whole at `setDocuments`.
- **E (Encapsulation):** the engine reads records and roles. Compiler words live only in the language folder.
- **D:** byte-equal output across runs (Step 6).

## Risks / Open Questions

None. The names come from existing families: `LanguageEntry::index`, `LanguageIndex`, `LanguageCallbacks::addSymbol`, `Id::symbol`, and `construction.md` sections that keep their `c_language.md` names.

## Amendment A (approved 2026-11-15)

ARCHITECT approved the amendment. Its full text is in `/Users/jreng/.claude/plans/fuzzy-doodling-candle.md`. ARCHITECT, verbatim:
- *"hot path should always lock free, no lock, no wait. unless libclang being called at message thread, then it must be done in parallel, thread pool, ephemeral, one shot, clean up when done, all of it without manual thread management, nor manually handle the state machine, everything is fully event driven, by Model parameters."*
- *"EVE already show exactly the pattern."*
- Audit #11: "Keep new names". Audit #24: the split is "Index + Query".

The amendment changes Steps 3 and 4:
- **Step 3.3** is replaced. Each `index()` call owns its library handle, its CXIndex and its CXIndexAction, and releases them before it returns.
- **Step 4** uses the EVE job pattern:
  - one owned `juce::ThreadPoolJob` for each row;
  - its result returns through `triggerAsyncUpdate`, and the message thread removes the job;
  - no pending counter.
- **Records** carry line and column, not byte offsets. They also carry the libclang role.
- **Processor** splits into `Index` and `Query`.
