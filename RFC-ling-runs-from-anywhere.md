# RFC — ling runs from anywhere
Date: 2026-11-19
Status: Ready for COUNSELOR handoff
Author: MACHINIST
Targets: ling `Source/Main.cpp`, `Source/Relay.cpp`, `Source/Client.cpp`; CAROL plugin manifest `~/.carol/.claude-plugin/plugin.json`

## Problem Statement

ARCHITECT's words, verbatim: *"ling should be able to run from anywhere. while project-info.md as manifest still adhere to its contract."*

Today ling does not start unless its first argument names a manifest file that exists. The CAROL plugin starts ling as `ling project-info.md`. Claude Code starts the server in the session folder. A session in a folder without a `project-info.md` gets no LSP server.

Sessions that fail today: `~/.carol`, `~/.config`, JAM (`~/Documents/Poems/dev/jam`), CAKE, CAROLINE. These folders hold no `project-info.md`. MACHINIST works in `~/.carol` and `~/.config`. Both are not C++ projects. ARCHITECT asks that ling run in every folder, and that the manifest contract stays as it is.

## Research Summary

### R1. The failure, run first

- Command: `cd ~/.carol && ling project-info.md </dev/null >out 2>err`
- Observed: exit code 1. `err` holds `ling: project-info.md: not found`. `out` holds the banner and the help guide.
- Command: LSP tool `documentSymbol` on `~/Documents/Poems/dev/end/Source/Main.h`, from a session in `~/.carol`.
- Observed: `LSP server exceeded the header size limit without a message header — it is likely printing logs or other non-protocol output to stdout instead of stderr`.
- The Claude Code binary (`/Users/jreng/.local/share/claude/versions/2.1.328`) holds this text: `LSP server sent non-protocol output in the header block — its stdout is desynchronized from the base protocol (logs/banners on stdout instead of stderr?)`.
- `~/.carol/project-info.md` and `~/Documents/Poems/dev/jam/project-info.md` do not exist. Folders that hold one, found with `find ~/Documents/Poems -maxdepth 3 -name project-info.md`: `dev/ling`, `dev/ggwp`, `dev/cast`, `dev/scout`, `dev/whelmed`, `dev/end`, `dev/sandbox`, `dev/eve`, `dev/stamp`, `dev/whatdbg`, `dev/tit`, `kuassa/jreng-filter-strip`. Running ling daemons exist for END and jreng-filter-strip (`ps`).

### R2. Why ling prints the guide on stdout

- `Source/Main.cpp:154-163`, `getExistingManifestFile`: when the manifest file does not exist, the function calls `printBannerAndHelp()` and then `juce::ConsoleApplication::fail (...)`. `printBannerAndHelp()` writes to stdout (`Main.cpp:95-108`).
- The guide documents this (`ling --help`, Command Line table): `ling <missing>` — `Prints this guide, then ling: <missing>: not found on stderr.` — exit 1. The same guide says: `Each failure prints ling: <first argument>: <message> on stderr.`
- The behavior is by design for a terminal user. It breaks the protocol when Claude Code is the client, because Claude Code reads the first bytes of stdout as an LSP header.

### R3. How ling binds to a manifest

- `ling --help`, Server: `ling <manifest> [--<word>] starts the server. The server speaks LSP over stdio.` The server connects to the daemon of the manifest and the word.
- `ling --help`, Server: `The project name is a hash of the manifest path and the word. The daemon name is the project name and a hash of the ling build. Each daemon name runs one daemon.`
- `Source/Main.cpp:339-378`, `runManifest`: the default command builds `Relay (manifestFile, toolchainArgument)` and calls `relay->start()`. The manifest is known when the relay is constructed.
- `Source/Relay.cpp:76-77`: the `Relay` constructor passes the manifest to the `Client` base. `Source/Relay.cpp:112-118`: `Relay::start()` calls `Client::start()`, then opens stdin. The daemon connection starts before the client sends `initialize`.
- `Source/Relay.cpp:229-246`, `Relay::initialize`: the relay answers `initialize` itself. It reads the position encoding. It does not read `rootUri`, `rootPath`, or `workspaceFolders`.
- `Source/Relay.cpp:272`, `getRequest`: each request carries `Id::directory` set to `juce::File::getCurrentWorkingDirectory()`. ling already uses the working folder of the server process.

### R4. The manifest contract

From `ling --help`, Manifest. The manifest is the file `project-info.md`. ling reads these tables:

- `## index` — alias to symbol. A cell that starts with `@` names a row. `${CMAKE_CURRENT_SOURCE_DIR}` stands for the folder of the manifest.
- `## toolchain` — columns `argument`, `command`, `flag`. A row with an empty `argument` belongs to the default flow. `--<word>` selects rows whose `argument` is `<word>`. The manifest fails validation when `command` or `flag` is missing.
- `## daemon` — rows `timeout` (default 60000) and `threads`.
- `## source glob` — columns `path`, `extension`. Each file is one unit.
- `## layout glob` — column `value`. Each file is one unit.
- `## binary` — column `value`. Each file is one unit.
- `## extensions` — the file extensions that a language handles.
- Built-in languages (md, mmd, json, css, xml, html) read the rows of the three glob tables whose extension they handle. They parse each file. They do not run the index pass.

The manifest is the single source of the units of a project and of its toolchain. A unit exists only when a glob row names it.

### R5. How Claude Code starts an LSP server

Evidence: the strings of `/Users/jreng/.local/share/claude/versions/2.1.328`, extracted to `claude.strings` in the MACHINIST scratchpad.

- The plugin manifest key is `lspServers`. The schema fields: `command`, `args`, `transport`, `env`, `initializationOptions`, `settings`, `workspaceFolder` (description: `Workspace folder path to use for the server`), `startupTimeout`, `shutdownTimeout`, `requestTimeout`, `restartOnCrash`, `maxRestarts`, `diagnostics`, `extensionToLanguage`.
- The start call in the binary: `Q.start(o.command, o.args||[], {env:o.env, cwd:ds()??o.workspaceFolder, ...})`. The `rootUri` sent to the server is built from `o.workspaceFolder||pe()`.
- The server environment holds `CLAUDE_PROJECT_DIR`. The binary expands `workspaceFolder` and the environment values with a placeholder step.
- The official plugins in `~/.claude/plugins/marketplaces/claude-plugins-official/plugins/*/.claude-plugin/plugin.json` use only `command`, `args`, `extensionToLanguage`, and `startupTimeout`.
- The plugin has no field that chooses a manifest for each session. One global plugin gives one fixed `args` list to every session.

### R6. The current CAROL wiring

- `~/.carol/.claude-plugin/plugin.json:15-18`: `"lspServers": { "ling": { "command": "ling", "args": ["project-info.md"], ...`.
- `~/.carol/skills/doxygen-protocol/SKILL.md:23`: `The LSP tool runs ling project-info.md from PATH, in the session folder.`
- `~/.carol/skills/doxygen-protocol/SKILL.md:44-46`: `ling answers for the project whose project-info.md is in the session folder. A framework file (JAM, KANJUT, CIUM) gets an answer when a unit of that project compiles it.`
- A framework folder (JAM, KANJUT, CIUM) holds no `project-info.md`. So a session in a framework folder gets no answer today.

### R7. A glob-only manifest is valid, run first

- Manifest: one table, `## source glob`, one row, `path` `/Users/jreng/.carol`, `extension` `md`. No `## toolchain`, no `## index`, no `## cmake`.
- Command: `cd <scratchpad>/globtest && ling documentSymbol project-info.md /Users/jreng/.carol/NAMES.md`
- Observed: exit 0. The answer is one JSON-RPC response whose `result` lists the headings of NAMES.md (`Philosophical Framework for Naming in Code`, `Preamble`, `Rule -1 — No Improvisation`, ...).
- The manifest contract holds without a toolchain table for a folder of built-in languages (R4: built-in languages read the glob rows and do not run the index pass).

### R8. A C++ unit exists only through a compile command, run first

- `~/Documents/Poems/dev/ling/language/cLang/language.cast:2152-2176`, `addUnitCommands`: when a toolchain row has `command` `cmake` and a `-B <dir>` flag, the language reads `<manifest folder>/<dir>/compile_commands.json`. It calls `addUnitCommand` for each compile command (`:2168-2169`). A missing file gives the message `compiler manifest not found` (`:2175`). ling reads the file. It does not run cmake here.
- `Source/Manifest.cpp:251-253`: a `## source glob` row becomes a file pattern. It gives no compile arguments.
- `Source/Manifest.cpp` reads the tables `index`, `cmake` (`jucePath`, `juceVersion`), `daemon`, `toolchain`, `source glob`, `layout glob`, `binary`. It does not read `## include` or `## define`.
- Command: scratch manifest with `## source glob` rows for `h` and `cpp` over `~/Documents/Poems/dev/jam`; `ling documentSymbol project-info.md .../jam_core/jam_core.h` and `ling diagnostics project-info.md .../jam_core/jam_core.h`. Observed: both return `{"jsonrpc":"2.0","id":null,"result":[]}` with no error.
- So R7 (a glob-only manifest) holds for the built-in languages only. A C++ file needs compile arguments.

### R9. A JUCE module declares its own requirements

- `~/Documents/Poems/dev/jam/jam_core/jam_core.h:1-19` holds `BEGIN_JUCE_MODULE_DECLARATION` with `ID: jam_core`, `dependencies: juce_core juce_data_structures juce_events juce_graphics`, `OSXFrameworks: CoreServices`, `minimumCppStandard`, and `searchpaths`. The JAM module headers that I counted with `grep -c` each hold one declaration (jam_clap, jam_audio_devices, jam_animation, jam_core).
- `~/Documents/Poems/dev/end/Builds/Debug/compile_commands.json` has 143 `"file"` entries. 48 lines name `/dev/jam/`, for example `jam_core.mm`, `jam_debug.cpp`, `jam_graphics.mm`. The command of `jam_core.mm` holds `-DJUCE_MODULE_AVAILABLE_<id>=1` for each module, `-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1`, and `-DJUCE_DONT_DECLARE_PROJECTINFO=1`.
- The JUCE root of a project is the `jucePath` row of the `## cmake` table (`end/project-info.md:64`): `${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE`. `~/Documents/Poems/JUCE/modules` exists. `env | grep ^CAST_` prints nothing in this session.
- `~/.config/ling/` holds one folder, `language/`.

## Principles and Rationale

- **Single Source of Truth.** The manifest holds the units and the toolchain of a project (R4). Any design that makes a second list of units, or a second toolchain source, makes shadow state. Every option below keeps the manifest file as the only source of units and toolchain for C++.
- **Explicit.** A server that cannot find a manifest must say so on stderr, and must write no byte that is not LSP to stdout (R1, R2). Explicit fail-fast is already the contract of the guide: failures go to stderr.
- **Deterministic.** The same working folder and the same manifest give the same project name, so the same daemon (R3). A discovery step must give one manifest for one folder, or it breaks this.
- **Bound.** One daemon owns one manifest and one word (R3). A change of manifest means a different daemon, not a re-bound daemon.
- **Considered and rejected:**
  - *An absolute manifest path in `plugin.json` `args`.* It fixes one project for every session. A global plugin cannot follow the session (R5). It also writes a project path into a framework file.
  - *A copy of the END or ling `project-info.md` in `~/.carol`, JAM, CAKE, CAROLINE.* Those manifests describe CMake projects. `~/.carol` is not one. A framework has no units of its own (R6). A glob-only manifest is a different case: see R7 and D7.
  - *Make the plugin pick the manifest.* The plugin format has no field for it (R5).

## Scaffold

Working structure from the session. This is the present call chain that every option changes at one point. No option adds code here.

```
Claude Code ── spawn: ling project-info.md (cwd = session folder)           [R5, R6]
  Main.cpp:339 runManifest
    Main.cpp:154 getExistingManifestFile ── missing file: guide on stdout, exit 1   [R2]
    Main.cpp:376 Relay (manifestFile, toolchainArgument)
      Relay.cpp:112 start() ── Client::start() connects the daemon, then stdin opens  [R3]
      Relay.cpp:229 initialize ── reads position encoding only                      [R3]
```

The manifest is chosen in `Main.cpp:348-349` and `Main.cpp:376`, before the first LSP message. Any option that reads `rootUri` must move the daemon connection after `initialize`, or must find the manifest from the working folder before the relay starts.

## BLESSED Compliance Checklist

- [ ] Bounds — one daemon per manifest and word stays true after the change
- [ ] Lean — no option adds a second manifest format
- [ ] Explicit — a missing manifest reports on stderr only
- [ ] SSOT — the manifest file stays the only source of units and toolchain
- [ ] Stateless — discovery holds no state between runs
- [ ] Encapsulation — the plugin passes no project knowledge
- [ ] Deterministic — one folder gives one manifest

## Open Questions

None. ARCHITECT's words: *"no open questions. resolve now"* and, on the JUCE root: *"why not just add them to language manifest?"*. MACHINIST resolved D1 to D8 by the cited evidence and BLESSED pillar.

### Resolutions

**D1. Manifest discovery: search upward from the working folder, by file name.**
- The first argument stays the manifest (`ling <manifest> [--<word>]`, `ling --help` Server). A manifest path that names an existing file works as today.
- When the first argument is a bare file name that does not exist in the working folder, ling searches the ancestors of the working folder, nearest first, for a file of that name. The first file found is the manifest.
- Options rejected, with evidence:
  - `rootUri` of `initialize`: the daemon connection starts before `initialize` (`Relay.cpp:112-118`, `Relay.cpp:229`). The option moves the connection after `initialize`. That changes the lifecycle, which breaks Bound (R3).
  - `CLAUDE_PROJECT_DIR`: Claude Code sets it (R5). Another client does not. It breaks "run from anywhere".
- Why the working folder: ling already uses it (`Relay.cpp:272`), and the manifest is chosen at `Main.cpp:348-349` before the relay starts, so the search adds no lifecycle step. Claude Code starts the server in the session folder (R1: the run in `~/.carol` failed on the relative name).
- Deterministic: one working folder gives one nearest ancestor, so one manifest and one daemon name (R3).
- The project name hash input stays the manifest path (R3).

**D2. No manifest found: the server runs and serves no units. It starts no daemon.**
- `initialize` and `shutdown` answer as today (`Relay.cpp:229-246`). Each operation request gets the empty result of its method.
- Why: SSOT. Units exist only through glob rows (R4). A folder with no manifest has no units. ling does not synthesize a scope (YAGNI, Lean). The exit option (c) fails ARCHITECT's words.
- A daemon with zero units would own nothing (Bound), so none starts.

**D3. The synthesized-folder rule: void.** D2 does not choose option (b), so no new field exists: no scope, depth, exclusion list, or extension list.

**D4. A failed start writes nothing to stdout.**
- The guide prints for no argument and for `--help` only. `getExistingManifestFile` (`Main.cpp:154-163`) stops calling `printBannerAndHelp()`. The failure line goes to stderr, as the guide already states: `Each failure prints ling: <first argument>: <message> on stderr`.
- Why: Explicit pillar, and R1/R2: Claude Code reads the guide on stdout as a broken header. The terminal-gated option (a) adds a branch on `isTerminalOutput` (Lean) for a behavior the guide does not need.
- `Source/HELP.md`: the Command Line row `ling <missing>` changes to `Prints ling: <missing>: not found on stderr.` Exit 1.
- D1 and D2 leave this failure for an argument with a path part that does not exist (for example an absolute path).

**D5. Daemon name for a folder with no manifest: void.** D2 starts no daemon.

**D6. The CAROL plugin does not change.** `plugin.json:18` keeps `"args": ["project-info.md"]`. D1 makes the bare name work in every folder. After the ling change ships, MACHINIST updates `~/.carol/skills/doxygen-protocol/SKILL.md:23` and `:44-46` to say "the nearest `project-info.md` above the session folder".

**D7. A folder of JUCE modules, such as JAM, KANJUT, CIUM, needs no project and no manifest.**
- ARCHITECT's question: *"why JUCE modules need PROJECT?"* The evidence agrees: a module declares its own ID, its dependencies, and its frameworks in `BEGIN_JUCE_MODULE_DECLARATION` (R9). A project adds only the arguments, not the module facts.
- Design: for a C++ file under a module folder that has no entry in a compile database, ling builds the compile arguments from the module declaration of the folder and the declarations of its dependencies. The arguments are the include root (the parent of the module folder), `-DJUCE_MODULE_AVAILABLE_<id>=1` for the module and each dependency, `-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1`, and the C++ standard of the language folder. The declaration is the single source, so no manifest table and no second file duplicates it (SSOT).
- A folder of markdown, json, css, xml, or html still gets its index from a glob-only `project-info.md` (R7).
- A file of a module that a project compiles keeps the arguments of that project database (R8), because a project can define more.
- What D7 does not settle is the JUCE root for the dependencies `juce_*`: see Open Questions.

**D8. The JUCE root lives in the construction tables of the language folder.**
- The `construction.md` of `language/cLang/` already holds machine values with one key for each platform: `## libraryPaths` (`platformMac` `../lib/libclang.dylib`, `platformWindows` `libclang.dll`), `## sdkCommands`, `## sdkFlags` (`construction.md:356-373`). The JUCE `modules` folder is the same kind of value.
- Add one table to the `libraryPaths` family: key `platformMac` or `platformWindows`, value the JUCE `modules` folder. Name: `juceModulePaths`. It states the content, it is a plural noun like `libraryPaths`, and it joins the nearest family (NAMES.md Rule -1 checks 1 and 2).
- Why not the other options: a new setting in `~/.config/ling/` adds a second place for machine values beside `construction.md` (SSOT). The `searchpaths` field of a module declaration is empty in `jam_core.h` and belongs to the module, not the machine (R9).
- `construction.md` is an input of the cast spell: `language/cLang/spell.md` indexes `@construction construction.md`, and `language.cast:564-565` builds a `construction-entry` from it. A change needs a rebuild of the language library with `ling --build` (`Main.cpp:298-337`, `runBuild`).
- Cross-platform: the value differs by platform, so the table carries both keys, as `libraryPaths` does.
- Observed: `language/cLang/construction.md` in the repo and `~/.config/ling/language/cLang/construction.md` differ (`diff -q`). The edit goes to the file that ling reads, and the two copies must match afterward. COUNSELOR settles which copy is the source.

## Handoff Notes

- Implementation, in order: D8 (the `juceModulePaths` table, then `ling --build`), D7, D4 (`Main.cpp:154-163`), D1 (the search before `Main.cpp:348-349`, one function in the manifest reading), D2 (`Relay` with no manifest: no `Client::start()` connection, empty results). Update `Source/HELP.md` for D1, D2, D4.
- Source facts: manifest chosen at `Main.cpp:348-349` and `:376`; guide printed at `Main.cpp:161`; daemon connection starts at `Relay.cpp:112-118`; `initialize` handled at `Relay.cpp:229-246`; the working folder is read at `Relay.cpp:272`.
- The guide `Source/HELP.md` is the text of `ling --help`.
- ling source is COUNSELOR's. MACHINIST wrote this RFC on ARCHITECT's request and edited no ling source.
- After the ling change ships, MACHINIST does these edits: `~/.carol/skills/doxygen-protocol/SKILL.md:23` and `:44-46` (D6). Glob-only manifests for `~/.carol` and `~/.config` (R7) need ARCHITECT's word before MACHINIST writes them.
- Until the change ships, the LSP tool works only in a session started in a folder that holds `project-info.md`.
