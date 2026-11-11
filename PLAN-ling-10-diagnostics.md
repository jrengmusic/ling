# PLAN: LING Part 10 — Diagnostics: the Compiler Manifest's Own Compile, Unchanged Lines

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 14, 19, 20, 21, 22, 29, 45, 48, 49, 50, 51, 72 to 76; RFC rulings 4 and 8.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

Ruling 49: *"ARCHITECT need diagnostics, especially with vim. that's exactly why we develop LING in the first place."* / *"we're on environement where we can build DETERMINISITICALLY. of course we know what to compile, any language have manifest, we can read the manifest"*.
This part adds the operation `diagnostics`. It takes one file. It runs the compile command that the compiler manifest gives for that file, with the check flags of its language family, and it returns the compiler's output unchanged.
The diagnostics are the compiler's own, with the build's own flags. LING parses none of them and converts none of them (MANIFESTO: "do no operation on data in transit"). nvim and agents read the same lines.
A unit that loads the build's precompiled header gets LING's own copy of that header, made from the compiler manifest's own entry. LING writes nothing into the build tree.

## What this part does not do

- No diagnostics from the tree of Parts 4 and 5. The tree has no type analysis and no macro expansion (PLAN-ling-7-queries.md, What this part does not do).
- No parse of the compiler output. The result is the bytes of the compiler.
- No compiler name and no flag text in `Processor` or `LINGModel` (rulings 22 and 74). The check flags of a family are rows of its table file in `ling/cast/`, and `CompilerManifest` gives them.
- No LSP server. The stdio wrapper is closed (tracker Risks item 16).
- No nvim Lua and no Claude Code hook. Ruling 21 gives the client to the user; the tracker gives the harness to MACHINIST. Step 6 item 7 proves that nvim reads the result with existing APIs. The agent hook is MACHINIST's work, and it uses the documented `PostToolUse` fields (section Facts).

## Language / Framework Constraints

- Ruling 49: LING runs the compiler manifest's compile command of a unit (ruling 19: *"cast only execute"*; RFC ruling 8: *"our tools only `compile` what compiler need"*).
- Ruling 49, on the build tree: *"why running manifests pch entry need to writes to build tree"*. LING writes into its own cache folder only.
- Rulings 22 and 74: no compiler name, no flag text, and no output form in `Processor` or `LINGModel`. The check flags are rows of `cast/c_language.md`, read through `CompilerManifest` (PLAN-ling-6-manifest.md, In LING: the compiler manifest). Ruling 72: no part adds a table to a `project-info.md`.
- Ruling 50: no `juce::String` and no `juce::StringArray`, except where a JUCE API returns one or takes one as an argument. Ruling 51: JAM first (`jam::Subprocess`, `jam::Strings`, `jam::Owner`, `jam::Wyhash`, `jam::Format`), JUCE where JAM has no counterpart (`juce::File`, `juce::ValueTree`, `juce::MessageManager::callAsync`).
- MANIFESTO **E**, Complete at Creation: the compiler output is complete when the child exits. LING moves it to `Id::result` in one piece and changes no byte of it.
- MANIFESTO **B**: "Threads are bound to their context". The `jam::Subprocess` callbacks fire "on the Worker thread" (`jam_subprocess/subprocess/jam_Subprocess.h:11-19`). They hand the capture to the message thread with `juce::MessageManager::callAsync`, as the header requires. Only the message thread touches the tree and the owners.
- MANIFESTO **S** (Stateless): the check holds no state between requests. LING's precompiled header is a file on disk with one owner, `Processor`, and one reaction that replaces it.
- CODING.md: no out-parameter; no fake-carrier struct; no `==` on strings; uniform signature in the type of the events map.
- CAROL.md Code Hygiene: no comment and no doxygen in this sprint. CAROL.md Profiling: measurement uses scout.
- `jam/.claude/CLAUDE.md:104`: "Agents never build. ARCHITECT builds."
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Line numbers are reads of 2026-11-10. Measurements of 2026-11-10 ran on END's real compiler manifest (`end/Builds/Debug/compile_commands.json`, 142 entries), Apple clang 17.0.0, and nvim 0.12.6.

### Facts, measured and read

| Fact | Evidence |
|---|---|
| An entry holds `command`, `directory`, `file`, `output` | END compiler manifest; clang JSONCompilationDatabase.html: `command` is "a single shell-escaped string"; `output` "The name of the output created by this compilation step" |
| CMake writes `command`, not `arguments` | `cmGlobalNinjaGenerator.cxx:1256-1267`; CMAKE_EXPORT_COMPILE_COMMANDS: "implemented only by Makefile Generators and Ninja Generators" |
| The unit's command plus `-fsyntax-only -fdiagnostics-format=msvc` | `juce_gui_basics_2.cpp`: exit 0, no output, its object file unchanged |
| A unit loads the build's precompiled header | 16 of 142 entries hold `-Xclang -include-pch -Xclang <build>/CMakeFiles/END.dir/cmake_pch.hxx.pch`; all are END `Source/` files |
| The build's header can be older than its sources | `SessionView.cpp` with the check flag: `fatal error: file '.../jam_TerminalGlyph.h' has been modified since the precompiled header '.../cmake_pch.hxx.pch' was built`; object file unchanged |
| The compiler manifest holds the header's own entry | `cmake_pch.hxx.cxx` with `-Xclang -emit-pch`, `output` `.../cmake_pch.hxx.pch` |
| A unit's header argument is the header entry's `output` | 16 entries hold one argument that resolves, against `directory`, to the full path of another entry's `output`; that output is the one `.pch` output |
| Each entry's own output is in its argv | 142 of 142 entries hold one argument that resolves to their own `output`; 0 hold it as the same text (the key is absolute, the `-o` token relative) |
| LING's own header, no build tree write | the header entry with its own-output argument set to a scratch path: exit 0; then `SessionView.cpp` with that path in place of the build's: exit 0, no output; the build's `.pch` time unchanged |
| One observed run each (not a measurement) | header entry 2.88 s; unit with LING's header 1.0 s; unit with no header (the `-include-pch` pair removed) 2.88 s |
| A header error is reported by the unit that includes it, under the header's own path | scratch: `./h.h(1,17): error: expected ';' at end of declaration list` |
| One line per diagnostic, absolute path | with `-fno-caret-diagnostics -fdiagnostics-absolute-paths -fno-color-diagnostics`: `/abs/h.h(1,17): error: ...`, `/abs/u.cpp(2,18): error: ...`, `/abs/h.h(2,5): note: ...`; one more line `In file included from /abs/u.cpp:1:` |
| `-fsyntax-only` with `-c` and `-o` writes no file | clang help: "Run the preprocessor, parser and semantic analysis stages"; Apple clang 17 and Homebrew clang 23 runs; GCC 16 run |
| clang's text form of MSVC | `-fdiagnostics-format=` takes "clang (default), msvc, vi, and sarif"; SARIF "is currently unstable" (clang UsersManual) |
| MSVC check | `/Zs`: "no output files are created, and error messages are written to standard output"; `/diagnostics:column` "also includes the column" (learn.microsoft.com). clang-cl lists `/Zs` and `/diagnostics:column`. Not run: no Windows tools on this machine. |
| nvim reads the lines with no other step | nvim 0.12.6: `vim.fn.getqflist ({ lines = ..., efm = ... })`, then `vim.diagnostic.fromqflist`: buffer, line, column, severity, message for each of the 3 diagnostics. The row `%f(%l\,%c): %trror: %m` is already in `~/.config/nvim/lua/core/build.lua:43`; the display is `core/diagnostics.lua:6-31`. |
| Agents can get the lines after an edit | Claude Code `PostToolUse`: "Runs immediately after a tool completes successfully"; matcher `Edit|Write`; `tool_input.file_path` "always absolute"; exit 2 "Shows stderr to Claude" (code.claude.com hooks reference) |

### Existing pieces

| Need | Existing piece | Where |
|---|---|---|
| Run a child with merged output in one capture | `jam::Subprocess::launch (command, workingDir, onComplete, onChunk = nullptr)`; `Completion (int exitCode, const std::string& capture)`; capture-only mode when `onChunk` is `nullptr`; "stdout and stderr are merged into one capture, capped at `byteCap` bytes" | `jam_subprocess/subprocess/jam_Subprocess.h:21-24`, `:59-98`, `:108-111` |
| A run that blocks nothing | `Completion` "fires on the Worker thread"; "marshal via `juce::MessageManager::callAsync`" | `jam_Subprocess.h:11-19` |
| One `Subprocess` for each run | "`launch()` asserts that no `Worker` exists" | `jam_Subprocess.h:30` |
| The form of a run in the codebase | `Toolchain::runProcess` | `scout/Source/Toolchain.cpp:15-41` |
| An owner of objects | `jam::Owner<ObjectClass>`: `add`, `remove`, `indexOf`, `size` | `jam_core/utilities/jam_Owner.h:94`, `:219`, `:279`, `:449` |
| The argv of a `command` | `jam::ChildProcess::getArguments` | PLAN-ling-6-manifest.md, In JAM: The argument splitter |
| A full path from a token | `juce::File::getChildFile`: "If the string is actually an absolute path, it will be treated as such"; `..` resolved | JUCE `juce_core/files/juce_File.h:261-275` |
| The folder of LING on the machine | `LINGModel::getCacheDirectory()`: `~/.cache/ling`, or `%LOCALAPPDATA%/ling` on Windows (ruling 58) | PLAN-ling-8-daemon.md, In LING: `Server` and `Connection` |
| The daemon name of a manifest and a toolchain argument | `LINGModel::getDaemonName` | PLAN-ling-8-daemon.md, In LING: `Server` and `Connection` |
| A generated list of text values for each platform | the tables `## textEditorKeys mac` and `## textEditorKeys windows`, one `value` column, row zero `""`; the fence `platform-value-lookup-table`; the type row `@textEditorKey`, `jam::LookupTable<int, const char*, 33>` | `jam/cast/terminal.md:1909-1923`, `:1990-1998`; `jam/cast/code.cast:323-334`; `jam/cast/spell.md:134`, `:961-968` |
| The list type | `jam::LookupTable`: the values constructor, "the first value is the table's default, and list position `i` lands in slot `i`"; `begin()`, `end()` | `jam_core/utils/jam_LookupTable.h:128-149`, `:162-164` |
| The table file of the C family and its `@cLanguage` index row | `ling/cast/c_language.md` | PLAN-ling-6-manifest.md Step 4 items 2 and 3 |
| A table keyed by family in `CompilerManifest` | `filesConstruction` behind `getFilesConstruction()` | PLAN-ling-6-manifest.md, In LING: the compiler manifest |
| A table by family with a read for one family | `vocabularyByFamily` behind `getVocabularyByFamily()`; `getVocabularyForFamily (int family)`; the entries `getCLangVocabulary`, `getPythonVocabulary` | `jam_CodeDocument.cpp:196-224` |
| The rows of the files, the paths, the documents | `FILE` rows; `Processor::paths`, `Processor::documents` | PLAN-ling-6-manifest.md, In LING: the state tree; In LING: the documents |
| The files of a unit | `Id::dependencies` of a unit row: the child indices, in the `FILES` row, of the files that the compiler read for it, from the dependency record of the build | PLAN-ling-6-manifest.md, In LING: the compiler manifest, The files |
| The child index of a row | `int indexOf (const ValueTree& child) const noexcept`: "Returns the index of a child item in this parent." | JUCE `juce_data_structures/values/juce_ValueTree.h:382-385` |
| A request waits for pending jobs | "When a parse is pending, it runs the entry when `Id::documents` is written" | PLAN-ling-7-queries.md, The nine operations |
| The change message of a file row | `sendPropertyChangeMessage (Id::path)` for any watcher event | PLAN-ling-6-manifest.md, item 4 of `LINGModel` |
| The request and the result in the daemon | a `Connection` adds the `REQUEST` row; the result goes back as text | PLAN-ling-8-daemon.md |
| A 64-bit hash of bytes | `jam::Wyhash::hashBytes (const void* key, std::size_t len)` | `jam_core/utils/jam_HashMap.h:221` |
| The text of a hash | `jam::Format::appendHex` | PLAN-ling-8-daemon.md Step 1 item 8 |

### Names

| Name | Source |
|---|---|
| `diagnostics`, the operation and `Id::diagnostics` | ARCHITECT's word (ruling 49); the Claude Code `diagnostics` field of `lspServers` |
| `map::cLanguageCheckFlags`, type row `@cLanguageCheckFlag` | `map::textEditorKeys` and `@textEditorKey` (`jam/cast/spell.md:134`, `:961-968`); a plural noun of what the table holds (NAMES.md Rule 8) |
| `CompilerManifest::getCheckFlags`, `checkFlagsByFamily`, `getCheckFlagsByFamily`, `getCLanguageCheckFlags` | `getVocabularyForFamily`, `vocabularyByFamily`, `getVocabularyByFamily`, `getCLangVocabulary` (`jam_CodeDocument.cpp:196-224`); "check flags" is the word of this part; `CLanguage` from ruling 26 |
| `Id::output`, `Id::dependencies` | exist from Part 6 (PLAN-ling-6-manifest.md, In LING: the state tree) |
| `Id::directory`, `Id::arguments` | the keys of an entry of `compile_commands.json`; `Id::directory` exists in `jam/cast/core.md:71`; `Id::arguments` comes in Part 6 Step 4 |
| `getUnit` | the verb `get`; "unit" is the word of `failUnitNotFound` (Part 6) |
| `getCheckArguments` | `Toolchain::getToolchainArguments` (`scout/Source/Toolchain.cpp:3`) |
| `getPrecompiledHeader` | the verb `get`; the build's own word: `cmake_pch`, `-include-pch` |
| `LINGModel::getCacheDirectory (manifest, toolchainArgument)` | the overload of `LINGModel::getCacheDirectory()` (Part 8) for the folder of one daemon |
| `checks`, `precompiles` | plural nouns of what the owners hold: one check run, one precompile run |
| Text row `failNoUnit` | `failUnitNotFound` (Part 6) |

## The design

### The rows

`CompilerManifest::addCLanguageFiles` writes two more properties on each `FILE` row of a unit (PLAN-ling-6-manifest.md, In LING: the compiler manifest, The units, items 3 and 4). `Id::output` and `Id::dependencies` are there from Part 6.

| Property | Value |
|---|---|
| `Id::directory` | the `directory` of the entry |
| `Id::arguments` | the argv of the entry: the `arguments` array, or `jam::ChildProcess::getArguments` of `command`. A `juce::var` array, because a `juce::ValueTree` property takes a `juce::var` (ruling 50). |

A row of a file that only a dependency record names holds neither. So a row with `Id::arguments` is a unit.

The check flags of a family come from `CompilerManifest`. `static jam::Strings getCheckFlags (int family);` reads the table `checkFlagsByFamily`, keyed by `map::Family`, behind the private `getCheckFlagsByFamily()`. The table has the form of `filesConstruction` (Part 6), and its map type holds the signature `jam::Strings ()`. `contains` comes before `get`: a family with no entry gives an empty `jam::Strings`, and the check then runs the unit's own command. The table has one entry: `map::Family::cLang` gives `getCLanguageCheckFlags`, a private static function that takes the key and returns the values of `map::cLanguageCheckFlags` that are not empty, in slot order. `map::cLanguageCheckFlags` is a `jam::LookupTable<int, const char*, 6>` from the tables `## cLanguageCheckFlags mac` and `## cLanguageCheckFlags windows` of `cast/c_language.md` (Step 1). The tree holds no copy of the flags, and no `project-info.md` holds one (ruling 72).

### The unit of a file

`juce::ValueTree Processor::getUnit (int pathIndex) const;`

- The `FILE` row of the path has `Id::arguments`: that row.
- Otherwise: the first unit row, in the order of the `FILES` children, whose `Id::dependencies` holds the child index of the `FILE` row of the path. The order is the order of the compiler manifest, so the choice is fixed (MANIFESTO **D**).
- No `FILE` row has the path, or no unit holds its index: an invalid `juce::ValueTree`.

The `FILE` row of a path is `getChildWithProperty (Id::path, ...)` of the `FILES` row, as `LINGModel::fileChanged` finds it (PLAN-ling-6-manifest.md, item 4 of `LINGModel`). Its child index is `indexOf` of the `FILES` row (`juce_ValueTree.h:382-385`). The `FILES` row is complete when it comes to the tree, and no object adds a child to it or removes one (Part 6). So a child index is valid for as long as the row is in the tree. `Id::dependencies` is read in place, as the `juce::var` array that the property holds (ruling 50). LING walks no `#include`: the record of the build is the one source of the files of a unit (rulings 75 and 76).

### The check

`jam::Strings Processor::getCheckArguments (const juce::ValueTree& unit) const;` returns, in order:

1. Each argument of `Id::arguments`. An argument whose full path (`juce::File (directory).getChildFile (argument)`) is the `Id::output` of another unit row is replaced by `getPrecompiledHeader` of that row.
2. Each element of `CompilerManifest::getCheckFlags` of the `Id::family` of the unit.

The check runs in a `jam::Subprocess` that `Processor` owns in `jam::Owner<jam::Subprocess> checks;`. `launch` gets the arguments, the `Id::directory` folder, a `Completion`, and no `Chunk` (capture-only mode). `Completion` hands the exit code and the capture to the message thread with `juce::MessageManager::callAsync`. It holds the `REQUEST` row by value: a `juce::ValueTree` is a handle to shared data. The capture crosses the thread boundary in one copy (MANIFESTO, Permitted copies, item 2). On the message thread `Processor` writes the capture to `Id::result` of the `REQUEST` row, as one `juce::String` (the `juce::ValueTree` property takes it; ruling 50). Then it removes the `Subprocess` from `checks`. The `Subprocess` is removed on the message thread, never on its own worker thread, because its destructor "joins the Worker thread" (`jam_Subprocess.h:26-30`).

The exit code is not a LING failure. A unit with errors exits with 1, and its lines are the result. An empty capture means a clean unit.

### LING's precompiled header

A unit row whose `Id::output` is the full path of an argument of another unit is a header entry. This rule reads the rows only: `Processor` names no file form and no flag. Its own command makes the header (`-Xclang -emit-pch` in END's compiler manifest); LING does not read that flag.

`juce::File Processor::getPrecompiledHeader (const juce::ValueTree& headerEntry) const;` returns the file `<LINGModel::getCacheDirectory (manifest, toolchainArgument)>/<jam::Format::appendHex text of jam::Wyhash::hashBytes over the bytes of Id::output><the extension of Id::output>`.

`static juce::File LINGModel::getCacheDirectory (const juce::File& manifest, std::string_view toolchainArgument);` returns the folder `LINGModel::getDaemonName (manifest, toolchainArgument)` inside `LINGModel::getCacheDirectory()` (PLAN-ling-8-daemon.md, In LING: `Server` and `Connection`). `Processor` takes the two values from `Id::manifest` and `Id::toolchainArgument` on the root. It makes no folder (NAMES.md Rule 1, Verb Contract). `Processor` makes the folder with `createDirectory()` before it starts a precompile run (`jam_markdown/document/jam_ConfigDocument.cpp:747`). `Processor` thus names no `Server` (MANIFESTO **E**).

| Event | Reaction |
|---|---|
| the `FILES` row is added (`valueTreeChildAdded`; PLAN-ling-6-manifest.md, In LING: the documents) | start one precompile run for each header entry among its children. Every row of the compiler manifests exists at this event, so the test of a header entry does not depend on the order of the rows. The runs start with the parse jobs, in parallel. |
| a change message of a `FILE` row whose child index is in `Id::dependencies` of a header entry | delete LING's file of that header, then start one precompile run |

A precompile run is the header entry's own argv, with the one argument that resolves to its own `Id::output` replaced by `getPrecompiledHeader`. It runs in a `jam::Subprocess` that `Processor` owns in `jam::Owner<jam::Subprocess> precompiles;`. It is a pending job, as a parse is: `Processor` writes `Id::documents` only when no parse and no precompile run is pending (PLAN-ling-6-manifest.md, "When a job ends and no job is pending"). So a `diagnostics` request waits for the header by the rule of Part 7, with no new path. Its `Completion` goes to the message thread, as a check does, and the run is removed from `precompiles` there. A failed precompile run is a `setMessage` with its capture.

### The operation

| Operation | Arguments | Result |
|---|---|---|
| `diagnostics` | file | the compiler output of the check of its unit, unchanged; empty for a clean unit |

1. `Processor` gets the entry of `Id::diagnostics` from `events`, as for the other operations (Part 7). When a parse or a precompile run is pending, it runs when `Id::documents` is written.
2. `getUnit` of the file. An invalid row gives a `setMessage` with `failNoUnit` and the path, and the result is empty.
3. `getCheckArguments` of the unit, then the check.
4. `Id::result` is the capture. The client prints it to stdout (PLAN-ling-8-daemon.md, In LING: the client, item 6).

### The two clients

The result is the compiler's text. Each client reads it with what it already has:

- **nvim:** `vim.fn.getqflist ({ lines = vim.split (result, '\n'), efm = <the rows of build.lua:40-47> }).items`, then `vim.diagnostic.fromqflist`, then `vim.diagnostic.set` for each buffer. The user writes this Lua, on save of a buffer (ruling 21).
- **Agents:** the `ling diagnostics` output in a Bash result, or a `PostToolUse` hook on `Edit|Write` that runs `ling diagnostics <project-info.md> [--<word>] <tool_input.file_path>` and exits 2 with the output on stderr. MACHINIST writes the hook (tracker, Work outside the numbered parts).

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: The rows
**Scope:** `ling/cast/identifiers.md`; `ling/cast/files.md`; `ling/cast/text.md`; `ling/cast/c_language.md`; `ling/cast/spell.md`.
**Action:**
1. `cast/identifiers.md`: add `diagnostics`. `Id::output` exists from Part 6 Step 4 item 5.
2. The file rows `lingCache` and `lingLocalCache` exist (PLAN-ling-8-daemon.md Step 2).
3. `cast/text.md`: add `failNoUnit` `` `no unit compiles the file` ``.
4. `cast/c_language.md`: add `## cLanguageCheckFlags mac`, in the form of `## textEditorKeys mac` (`jam/cast/terminal.md:1909-1923`), and `## cLanguageCheckFlags windows`, in the form of `## textEditorKeys windows` (`terminal.md:1990-1998`). The brief of the first is `@brief The flags that make a C family compile command check the code and write no output file, per platform.` Each table has one `value` column. Row zero of each is `""`. The rows after it, in this order:

| `## cLanguageCheckFlags mac` | `## cLanguageCheckFlags windows` | Meaning |
|---|---|---|
| `-fsyntax-only` | `/Zs` | Check the code, write no output file |
| `-fdiagnostics-format=msvc` | | One line form: file(line,col): severity: message |
| | `/diagnostics:column` | The column in each line |
| `-fno-caret-diagnostics` | | No source line and no caret line |
| `-fdiagnostics-absolute-paths` | | Absolute path of each file |
| `-fno-color-diagnostics` | | No colour codes |

   The `mac` table thus has six rows, and the `windows` table has three. A blank cell of this list is no row.
5. `cast/spell.md`: add the type row `@cLanguageCheckFlag`, `jam::LookupTable<int, const char*, 6>` after the `@string` row of `## index`, in the form of `jam/cast/spell.md:134`. The capacity is the higher row count of the two tables. Add one `## output` row group after the `@HashMaps` group, in the form of `jam/cast/spell.md:961-968`: lists `> > - [list]: @cLanguage:cLanguageCheckFlags mac` and `> > - [list]: @cLanguage:cLanguageCheckFlags windows`; separator `- [list]: @code:linebreak`; structure `@code:namespace`, `- name: map`, then `@code:platform-value-lookup-table`, `- type: @cLanguageCheckFlag`, `- name: cLanguageCheckFlags`, and two lines `> > - [list]: @code:value-entry`; file `@HashMaps`. Run cast (tracker, The contract of every part, cast runs).

**Validation:** Each row has the cell shape of its table. The flag text is in `cast/c_language.md` only (rulings 22 and 74). No `project-info.md` changes (ruling 72). `Source/generated/HashMaps.h` holds `map::cLanguageCheckFlags`. The `windows` rows are the documented MSVC options; Step 7 runs them.

### Step 2: `CompilerManifest`
**Scope:** `ling/Source/CompilerManifest.h` and `.cpp`.
**Action:** In `addCLanguageFiles`, write `Id::directory` and `Id::arguments` on each unit row. Write `getCheckFlags`, `getCheckFlagsByFamily`, and `getCLanguageCheckFlags`. Section The rows gives them.
**Validation:** The rows are written when the compiler manifest is read; a second read replaces them with the rows (Part 6). `Id::arguments` is the one argv of a unit. No compiler name and no flag text in the code: the flags are the generated rows.

### Step 3: The unit of a file
**Scope:** `ling/Source/Processor.h` and `.cpp`.
**Action:** Write `getUnit` as section The unit of a file gives it.
**Validation:** It reads the rows in place and keeps nothing (MANIFESTO **S**, Stateless). The files of a unit have one source, the dependency record of the build (MANIFESTO **S**, SSOT). `Processor` walks no `#include`. The choice of a unit is a total order.

### Step 4: LING's precompiled header
**Scope:** `ling/Source/Processor.h` and `.cpp`; `ling/Source/LINGModel.h` and `.cpp`.
**Action:** Write `LINGModel::getCacheDirectory (manifest, toolchainArgument)`, `getPrecompiledHeader`, `precompiles`, and the two reactions of section LING's precompiled header. Count a precompile run as a pending job for `Id::documents`.
**Validation:** LING writes only in its cache folder. A `Subprocess` is removed on the message thread. The header is replaced by one reaction to the change message, the event that the watcher already sends (ruling 45). The precompile run reuses the pending rule of Parts 6 and 7; no second wait path exists.

### Step 5: The check and the operation
**Scope:** `ling/Source/Processor.h` and `.cpp`; `ling/Source/HELP.md`.
**Action:**
1. Write `getCheckArguments`, `checks`, and the entry `Id::diagnostics` of `events`, as sections The check and The operation give them.
2. Describe `diagnostics` in `HELP.md`: its argument, the check flags of the C family, the cache folder, and that the result is the compiler's text.

**Validation:** The capture goes to `Id::result` unchanged: no byte is parsed or changed. It crosses the thread boundary in one copy (MANIFESTO, Permitted copies, item 2), and the `juce::var` of `Id::result` holds it because a `juce::ValueTree` property takes one (ruling 50). The entry has the signature that the map type declares. `Subprocess::launch` takes a `jam::Strings` (ruling 52; PLAN-ling-8-daemon.md Step 1 item 6), so `getCheckArguments` gives its result to `launch` with no conversion.

### Step 6: Build and check, macOS
**Scope:** builds of LING and END; runs of `ling`; one scout manifest.
**Action:**
1. ARCHITECT builds END Debug and LING.
2. Run `ling diagnostics ~/Documents/Poems/dev/end/project-info.md --debug <jam>/jam_core/document/jam_Document.cpp`. The result is empty. The object file of its unit in `Builds/Debug` keeps its time.
3. Run it on `end/Source/ui/SessionView.cpp`, a unit that loads the header. The result is empty. LING's header exists in the cache folder. The build's `cmake_pch.hxx.pch` keeps its time.
4. Add the line `int x = ;` in a function body of `SessionView.cpp`, save, and run item 3 again. The result holds one `error` line with the absolute path of `SessionView.cpp`, its row, and its column. Remove the line and save.
5. Add the same line in a function body of `end/Source/config/ConfigModel.h`, save, and run `diagnostics` on `ConfigModel.h`. The result holds the error under the path of `ConfigModel.h`. Remove the line and save.
6. Change one byte of a comment in `jam/jam_graphics/fonts/jam_TerminalGlyph.h`, save, and run item 3 at once. The request waits for the precompile run; the result is empty; LING's header has a new time. Restore the byte.
7. In nvim, run `vim.diagnostic.fromqflist (vim.fn.getqflist ({ lines = <the result of item 4>, efm = <build.lua:40-47> }).items)`. One diagnostic with the row and the column of item 4.
8. Run item 4 two times, with the error line in place. Compare the two results byte for byte.
9. Measure with scout: one benchmark for item 2, one for item 3, and one for a precompile run. Record the medians.

**Validation:** Items 2 to 8 hold. No file in `Builds/` changes (ruling 49). Item 8: the results are equal (MANIFESTO **D**). ARCHITECT reads item 9 against RFC ruling 4, "ULTRA FAST".

### Step 7: Build and check, Windows
**Scope:** a Windows machine, MSYS2 zsh.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:**
1. Build LING. Build END Debug with MSVC or clang-cl.
2. Read one `command` of the Windows compiler manifest. Check that `getArguments` gives the argv that CMake escaped (`cmOutputConverter.cxx:613-635`), with whatdbg at its return.
3. Run items 2 to 5 and 8 of Step 6 on the Windows machine, with the rows of `## cLanguageCheckFlags windows`.
4. If a result line has no column or no absolute path, record the line, and add the row that gives it to `## cLanguageCheckFlags windows`, and run cast.

**Validation:** The results hold the same diagnostics as on macOS, in the form `file(line,col): error ...` (ruling 16). No file in the build folder changes.

## BLESSED Alignment

- **B — Bound.** `Processor` owns the check runs, the precompile runs, and LING's header files. A run is removed on the message thread, never on its own worker thread.
- **L — Lean.** One operation, two row properties, one generated list of flags. No parser of compiler output. No new JAM type.
- **E — Explicit.** The check flags are rows of the table file of the family; no code holds one. A file with no unit is a message. The exit code of a unit with errors is data, not a failure of LING.
- **S — SSOT.** The compiler manifest is the one source of each compile command (ruling 49). LING's header comes from the compiler manifest's own entry. The compiler's text is the one form of a diagnostic; nothing converts it.
- **S — Stateless.** A check keeps nothing after it answers.
- **E — Encapsulation.** The data flows one way: the compiler manifest, the rows, the check, the capture, the result, the client. The client reads the compiler's text with its own API (ruling 21).
- **D — Deterministic.** The unit of a header is the first in the order of the compiler manifest. The flags come in slot order. Step 6 item 8 proves equal output for equal input.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open. Ruling 52 closed tracker Risks item 17.

Facts for the review of the whole PLAN:

- **The capture has a limit.** `byteCap { 100'000 }` bytes; past it, `truncationNotice` is added once and reading stops (`jam_Subprocess.h:21-24`, `:108-111`).
- **A message has a length limit.** `setMessage` writes a `jam::ParameterText`, and a longer text is cut at its length (`jam_data_structures/model/jam_ParameterText.cpp:32-35`). `LINGModel` has the form of `ENDModel`, whose length is 4096 bytes (`end/Source/end/ENDModel.cpp:7-9`). The capture of a failed precompile run is cut there.
- **A precompile run holds every request.** It is a pending job, so each operation waits for it, also `goToDefinition`. A change of a file that the record of the header entry holds starts one. One observed run took 2.88 s.
- **The first check after a daemon start waits for the header.** The precompile run starts when the header entry row is added.
- **A header is checked through one unit only**: the first unit in the order of the compiler manifest whose record holds it. Another unit with other defines can give other diagnostics for the same header.
- **The result holds the lines of every file that the unit compiles**, and the `In file included from` lines. `vim.diagnostic.fromqflist` gives each line its own buffer; a line with no file and position is not a valid quickfix item.
- **Homebrew clang warns** `argument unused during compilation: '-c'` with `-fsyntax-only`; Apple clang does not.
- **A unit whose header entry is not in the compiler manifest** keeps the build's header path, and reads the build's file.
- **The check reads the files on disk.** The client saves first, as for `completion` (Part 9).
- **A file with no row has no unit.** A header that is new since the last build is in no record. A `diagnostics` request for it gives `failNoUnit` until the next build.
- **The unit of a header is found by a scan.** `getUnit` reads `Id::dependencies` of each unit row in order until one holds the index. END Debug has 142 units and at most 3339 indices in one record.
