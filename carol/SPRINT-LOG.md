# SPRINT-LOG.md

**Project:** ling  
**Repository:** /Users/jreng/Documents/Poems/dev/ling  
**Started:** 2026-11-10

**Purpose:** Long-term context memory across sessions. Tracks completed work, debts paid, and debts deferred to the inter-sprint ledger (`DEBT.md`). Written by PRIMARY agents only when ARCHITECT explicitly requests.

---

## 📖 Notation Reference

**[N]** = Sprint Number (e.g., `1`, `2`, `3`...)

**Sprint:** A discrete unit of work completed by one or more agents, ending with ARCHITECT approval ("done", "good", "commit")

---

## ⚠️ CRITICAL RULES

**AGENTS BUILD CODE FOR ARCHITECT TO TEST**
- Agents build/modify code ONLY when ARCHITECT explicitly requests
- ARCHITECT tests and provides feedback

**AGENTS NEVER RUN GIT COMMANDS WITHOUT ARCHITECT'S EXPLICIT INSTRUCTION**
- Write code changes without running git commands
- Agent runs git ONLY when user explicitly requests
- Never autonomous git operations
- One exception: `git log` for a history question that bears on the task (CAROL.md Git)
- **When committing:** Always stage ALL changes with `git add -A` before commit
  - ❌ DON'T selectively stage files (agents forget/miss files)
  - ✅ DO `git add -A` to capture every modified file

**SPRINT-LOG WRITTEN BY PRIMARY AGENTS ONLY**
- **COUNSELOR** writes to SPRINT-LOG
- Only when user explicitly says: `"log sprint"`
- No intermediate summary files
- No automatic logging after every task
- Latest sprint at top, keep last 5 entries

**NAMING RULE (CODE VOCABULARY)**
- All identifiers must obey project-specific naming conventions (see NAMES.md)
- Variable names: semantic + precise (not `temp`, `data`, `x`)
- Function names: verb-noun pattern (initRepository, detectCanonBranch)
- Struct fields: domain-specific terminology (not generic `value`, `item`, `entry`)
- Type names: PascalCase, clear intent (CanonBranchConfig, not BranchData)

**BEFORE CODING: ALWAYS SEARCH EXISTING PATTERNS**
- ❌ NEVER invent new states, enums, or utility functions without checking if they exist
- ✅ Always grep/search the codebase first for existing patterns
- ✅ Check types, constants, and error handling patterns before creating new ones
- **Methodology:** Read → Understand → Find SSOT → Use existing pattern

**TRUST THE LIBRARY, DON'T REINVENT**
- ❌ NEVER create custom helpers for things the library/framework already does
- ✅ Trust the library/framework - it's battle-tested

**FAIL-FAST RULE (CRITICAL)**
- ❌ NEVER silently ignore errors (no error suppression)
- ❌ NEVER use fallback values that mask failures
- ❌ NEVER return empty strings/zero values when operations fail
- ❌ NEVER use early returns
- ✅ ALWAYS check error returns explicitly
- ✅ ALWAYS return errors to caller or log + fail fast

**⚠️ NEVER REMOVE THESE RULES**
- Rules at top of SPRINT-LOG.md are immutable
- If rules need update: ADD new rules, don't erase old ones

---

## Quick Reference

### For Agents

**When user says:** `"log sprint"`

1. **Check:** Did I (PRIMARY agent) complete work this session?
2. **If YES:** Write sprint block to SPRINT-LOG.md (latest first)
3. **Include:** Files modified, changes made, alignment check, debts paid, debts deferred
4. **Hygiene:** After writing the SPRINT-LOG entry, drain paid debts from project-root `DEBT.md` via `carol debt clear <id>` for each ID under *Debts Paid*. Receipt first, then clean the books.

### For User

**Activate PRIMARY:**
```
"@CAROL.md COUNSELOR: Rock 'n Roll"
```

**Log completed work:**
```
"log sprint"
```

**Invoke subagent:**
```
"@oracle analyze this"
"@engineer scaffold that"
"@auditor verify this"
```

**Available Agents:**
- **PRIMARY:** COUNSELOR (domain specific strategic analysis)
- **Subagents:** Pathfinder, Engineer, Auditor, Librarian

---

<!-- SPRINT HISTORY STARTS BELOW -->
<!-- Latest sprint at top, oldest at bottom -->
<!-- Keep last 5 sprints, rotate older to git history -->

## SPRINT HISTORY

<!-- The entry format is in /log (commands/log.md). It is the only source. -->
<!-- Sprint entries go here, written by PRIMARY agents -->

## Sprint 20: Align `languageCore` `JuceHeader` includes with linked `jam_core` modules ✅

**Date:** 2026-11-24
**Duration:** 00:21

### Agents Participated
- COUNSELOR: Traced the `languageBundle.cmake:58` macOS linker failure (`juce::Colour::Colour(unsigned int)`), verified `jam_core` and `jam_subprocess` dependencies, removed the two unlinked includes from `language/core/core.cast`, and logged Sprint 20.
- Pathfinder (`flash`): Surveyed workspace orientation, ran read-only `nm`/`lipo` symbol analysis on `language.cpp.o` and `liblanguageCore.a`, checked `jam_core` working-tree diff and history, and verified `juce_generate_juce_header` in `JUCEUtils.cmake`.

### Files Modified (1 total)
- `language/core/core.cast:823-830` — removed `#include <juce_data_structures/juce_data_structures.h>` and `#include <juce_graphics/juce_graphics.h>` from `juce-header` so `Builds/languages/core/JuceHeader.h` matches the modules linked by `languageCore` (`juce_core`, `juce_events`, `jam_core`, `jam_subprocess`).

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- **`liblanguage.dylib` macOS link error (`juce::Colour::Colour(unsigned int)`):** Uncommitted `jam_core/jam_core.h:10, 52-60` changes removed `juce_data_structures`, `juce_events`, and `juce_graphics` from `jam_core` dependencies and guarded their includes. `jam_Raw.cpp:1` still included `<JuceHeader.h>`, and `language/core/core.cast:823-832` generated an unconditional `<juce_graphics/juce_graphics.h>` include without linking `juce_graphics`. Removing the two unlinked module headers from `core.cast:823-830` eliminates the missing symbol.

### State for Continuation
- Build `ling` (`cast cast/spell.md`) to regenerate `Builds/languages/core/JuceHeader.h`, rebuild `liblanguageCore.a` and `liblanguage.dylib`, and verify the full macOS build passes.
- Working tree also carries regenerated `CMakeLists.txt:138-139` (`juce-posix-file-hook.patch`, `juce-sha256-hook.patch`) from prior `cast` run.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 19: Loud failed units, swap write results, manifest of the requested file, open-buffer index fix on `clang-cl`, and one libclang owner ✅

**Date:** 2026-11-24
**Duration:** 01:40

### Agents Participated
- COUNSELOR: Consumed `RFC-mcp-empty-responses-and-manifest-cwd.md`, wrote and executed `PLAN-failed-unit-index-and-manifest-routing.md` (approved through ExitPlanMode), validated each Engineer return at file:line, filtered 11 Auditor findings (9 accepted in full, 1 in part, 1 rejected), found the cause of RFC Open Question 4, and logged Sprint 19. One violation: the plan file was written and one Engineer was dispatched before plan mode was off. ARCHITECT reported it, and it was corrected at once.
- Pathfinder (`haiku`): Framework Inventory, two builds through cast, runtime tests (scratch projects, ling, END), whatdbg reproduction of the daemon end, proof of the open-buffer failure, doxygen pass.
- Engineer (`haiku`): Steps 1 to 6, the libclang owner, `Query::getCommand`, the accepted audit findings, `Source/HELP.md`.
- Auditor (`opus`): One audit after the runtime test (`NEEDS_WORK`, 11 findings).

### Files Modified (32 total: 31 in `ling`, 1 in `jam`)
- `language/cLang/language.cast:442-519` — `LibClang::path`, `getOrCreateLibClang`: one owner of each loaded libclang. `IndexSession` shares it.
- `language/cLang/language.cast:678-686, 733-760, 764-801` — failure constants, `index` sends the cause through `addMessage`, `addUnits` uses `getOrCreateLibClang`.
- `language/cLang/language.cast` — blocks `getInvocationEnd`, `addDeclaration`, `getIndexFailures`, `applyToTranslationUnit` (null unit), `addContextPass`, `indexSource` (`saveFailure`).
- `language/cLang/construction.md:526-537, 603-614` — `## indexFailures`. `argumentRemovals` row `--`.
- `language/cLang/spell.md:248-250, 344-346` — entries of the two new blocks.
- `cast/text.md:20` — `failSwapWrite`. `cast/readiness.md:17-23, 33` — `findReferences`, `goToImplementation`, `incomingCalls`, `rename` need `Id::files`; `prepareCallHierarchy`, `outgoingCalls` need `Id::dependencies`.
- `Source/generated/HashMaps.h`, `Source/generated/Text.h`, `CMakeLists.txt` — written by cast.
- `Source/SymbolDocument.{h,cpp}` — `Builder::writeSwapFile` replaces `buildToSwap` and returns `juce::Result`.
- `Source/Index.{h,cpp}` — `writeSwapFile` returns `juce::Result`; `addBindings (rowIndex, temporary, ...)` releases the mapping before the replace; `removeDocuments` releases each symbol document; `loadedLanguages`.
- `Source/Query.{h,cpp}` — `setError` with a message text, `getCommand`.
- `Source/Processor.{h,cpp}` — `IndexJob::result` as `juce::Result`, `IndexJob::addMessage`, `temporary`, `getScope`, `getUnitRows`, `getFailedRows`, `getMessage`, `addIndexJobs (request)`, `bufferEvents`, `RequestFailed` answer, diagnostics before the waiting requests.
- `Source/LINGModel.{h,cpp}`, `Source/Main.cpp` — `LINGModel::getManifestFile`.
- `Source/Client.{h,cpp}` — `state`, `requests`, `connectionMade`, `sendRequest`, `setMessage`. `Source/Relay.{h,cpp}`, `Source/Operation.{h,cpp}` — use them.
- `Source/ToolClient.{h,cpp}` (new) — one daemon connection of the MCP server. `Source/ToolRelay.{h,cpp}` — routes each request to the client of its manifest.
- `Source/HELP.md` — manifest routing, failed units, messages, readiness, `addMessage`.
- `CLAUDE.md` — Sprint 19 state.
- `../jam/jam_syntax_tree/document/jam_LanguageInterface.h:59-63, 80, 147` — `addMessage` also reports an index failure.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- **A failed unit stayed failed and gave an empty answer (RFC 1, 2, 3):** the row `message` is the one failure record. The next request that needs the unit runs its index again. When it fails again, the request gets error `-32803` with one `path: message` line for each unit. MCP gets `isError: true`, and the CLI exits with 1.
- **Cause of the 10 failed units at 21:44 (RFC Open Question 4):** with an open editor buffer, the index goes through `parseTranslationUnit2FullArgv`, and libclang appends its flags after the last argument. The `clang-cl` compile database ends with `-- <source>`, so each flag became an input file, and libclang answered `CXError_ASTReadError`. Proven with two scratch projects that differ only in `--`. The row `--` in `argumentRemovals` removes it. The condition and target flags of `getContextCommandLine` were after `--` too.
- **Swap write failures without a signal:** stream and replace results are read. The old symbols file is unmapped before the replace.
- **The daemon ended after an index with a second libclang path:** whatdbg showed an access violation in an unloaded `libclang.dll` from `LdrShutdownThread`. `getOrCreateLibClang` keeps each loaded library for the life of the language, and `loadedLanguages` keeps each language until the pool ends. Six rounds of the same test kept one PID.
- **`workspaceSymbol HWNDComponentPeer` empty (RFC Open Question 5):** `addDeclaration` read a macro invocation with no semicolon together with the next `namespace juce {`. `getInvocationEnd` ends the invocation.
- **MCP in another folder had no index (RFC 4):** each request with a file goes to the daemon of the manifest above that file.
- **Audit:** findings 1, 2, 4, 5, 6, 8, 9, 10, 11 resolved, finding 7 resolved except the ignored `bool` (`juce_Files_windows.cpp:457-473` keeps each failure in the status that is read). Finding 3 (reconnect after a lost daemon) rejected: `LINGModel.cpp:43-49` keeps the build hash for the process, so a client that reconnects after a binary replacement waits for a port file that no daemon writes.

### State for Continuation
- Build: `cast cast/spell.md --no-sign` exited 0 with no warning. Installed `ling 0.0.1 (41587de)`, 19850240 bytes, 2026-11-24 03:56. The version names HEAD of the build time, which is the commit before this work.
- Runtime check of that binary: open-buffer index passes; ling `workspaceSymbol` answers with no failed unit (321 s cold); END `workspaceSymbol` answers with no failed unit (352 s cold; 238 s with the first build of this sprint, where the context passes failed because of `--`); MCP from `~/.carol` answers for a ling file; LSP initialize, didOpen, hover, shutdown, exit code 0.
- Verify on macOS: build and index. No macOS run was made.
- A new ling binary ends each running daemon (`Server.cpp:139-143`), and the MCP and LSP processes of open sessions end with it (`Client.cpp:66-71`). Restart the MCP server of each open session.
- Check `jam::StdioServer` when its stdout is a regular file: two MCP answers written to a file started at the same offset. A pipe gives complete lines.
- Regenerate the doxygen XML of `ling` and `jam`.
- `D:\ling-mapped-experiment` is an empty folder from a cancelled experiment. The harness holds its removal for a permission prompt.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 18: Universal MCP `content` + `structuredContent` contract, explicit `noPort` error, Windows Job breakaway fallback, `goToImplementation` crash fix, and audit ✅

**Date:** 2026-11-22
**Duration:** 02:35

### Agents Participated
- COUNSELOR: Built Framework Inventory across `jam` and `ling`, authored and executed `PLAN-mcp-empty-responses-and-manifest-cwd.md`, diagnosed `Query::applyToImplementations` root-parent null dereference, validated all Engineer steps, confirmed and resolved all 3 Auditor findings, verified all Pathfinder Doxygen edits, and logged Sprint 18.
- Pathfinder (`flash`): Gathered `jam` and `ling` inventory excerpts, investigated Windows stdio vs MCP response framing, verified git history for `test/` and `benchmark/`, ran the 13/13 sequential raw MCP stdio verification, `noPort` error verification, `ling` and `end` Windows `clang-cl` accuracy checks, and executed the post-audit Doxygen pass.
- Engineer (`inherit`): Implemented `appendTextContent` and updated `ModelContextServer::sendResponse`/`sendToolError`, added `failNoDaemon` and `ToolRelay::sendNoDaemonError`, added `CREATE_BREAKAWAY_FROM_JOB` `ERROR_ACCESS_DENIED` fallback in `ChildProcess::startDetached`, and resolved all 3 Auditor findings (`isValidMeta` → `applyValidMeta`, `Source/HELP.md`, `CLAUDE.md`, `README.md`, and deletion of `PLAN-mcp-empty-responses-and-manifest-cwd.md`).
- Auditor (`inherit`): Audited all 10 sprint files and root docs against `MANIFESTO.md`, `CODING.md`, `NAMES.md`, `LANGUAGE.md`, and `PLAN-mcp-empty-responses-and-manifest-cwd.md` (`NEEDS_WORK`, 3 findings — all 3 confirmed and resolved).

### Files Modified (13 total: 4 in `jam`, 9 in `ling` including 1 deleted)
- `../jam/jam_model_context_protocol/server/jam_ModelContextServer.h:44-53, 106-138` — updated `sendResponse` Doxygen, replaced `isValidMeta` with template `applyValidMeta` (NAMES.md Rule 1 Verb Contract), and added `applyValidMeta` Doxygen.
- `../jam/jam_model_context_protocol/server/jam_ModelContextServer.cpp:109-156, 288-386` — added file-local `static void appendTextContent`, updated `sendResponse` to emit `content` text block alongside `structuredContent` and `sendToolError` to reuse it, and migrated `discover`/`listTools`/`callTool` to `applyValidMeta`.
- `../jam/jam_subprocess/subprocess/jam_ChildProcess.h:46-61` — updated `startDetached` Doxygen for Windows breakaway fallback.
- `../jam/jam_subprocess/subprocess/native/jam_ChildProcess_windows.cpp:233-267` — added `baseCreationFlags` and retry without `CREATE_BREAKAWAY_FROM_JOB` when `CreateProcessW` returns `ERROR_ACCESS_DENIED`.
- `cast/text.md:24` — added `failNoDaemon` diagnostic text (`no daemon for manifest`).
- `Source/Client.h:64-78` — moved `const juce::File manifestFile` to `protected:` (preserving member order relative to `Client.cpp:5-6`) without adding a getter.
- `Source/ToolRelay.h:64-84, 107-115` — renamed `sendEmptyResult` to `sendNoDaemonError` and updated Doxygen blocks.
- `Source/ToolRelay.cpp:115-171` — replaced `sendEmptyResult` (`null`) with `sendNoDaemonError` calling `server.sendToolError` with `failNoDaemon` + `Id::diagnosticSeparator` + `manifestFile.getFullPathName()`.
- `Source/Query.h:681-705` — guarded `applyToImplementations` with `declaration.parent != nullptr and declaration.parent != document.getRoot()` to prevent null `Id::type` dereference on top-level out-of-line definitions, and updated Doxygen.
- `Source/HELP.md:32, 309` — documented `ling --mcp` tool error on `-1` port file and Windows `%LOCALAPPDATA%\ling` cache path.
- `CLAUDE.md:13, 20-28` — synced Sprint 18 summary, removed non-existent `test/` and `benchmark/` layout entries, and updated sprint log range.
- `README.md:46` — clarified pre-installation PATH collision statement.
- `PLAN-mcp-empty-responses-and-manifest-cwd.md` — deleted completed Sprint 18 plan.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- **Empty MCP tool output for standard `content`-reading clients:** `jam::ModelContextServer::sendResponse` emitted `"content": []` and relied on a single host's `structuredContent` fallback; now emits both `content: [{"type":"text","text":"..."}]` and `structuredContent` for all JAM MCP servers (`ling`, `whatdbg`, `scout`).
- **Silent `null` on `Server::noPort` in `ToolRelay`:** Replaced `sendEmptyResult` with `sendNoDaemonError`, returning an explicit MCP `isError: true` message naming the manifest path.
- **Windows restricted Job Object daemon spawn failure:** `jam::ChildProcess::startDetached` now retries `CreateProcessW` without `CREATE_BREAKAWAY_FROM_JOB` on `ERROR_ACCESS_DENIED`.
- **`goToImplementation` daemon crash on out-of-line member definitions:** `Query::applyToImplementations` called `index.hasRole` on `document.getRoot()` (no `Id::type`), crashing in `LibraryLanguage::hasRole`; fixed with `declaration.parent != nullptr and declaration.parent != document.getRoot()`, passing 13/13 MCP tools in one sequential stdio session (`EXIT_CODE=0`).
- **Audit clean sweep (3/3 resolved):** Replaced side-effecting `ModelContextServer::isValidMeta` with `applyValidMeta`, synced `Source/HELP.md`/`CLAUDE.md`/`README.md`, and deleted the completed plan.

### State for Continuation
- Build `ling` (`cast cast/spell.md --no-sign`) exited 0 and installed `ling 0.0.1 (a550b12)` to `C:\Users\jreng\.local\bin\ling.exe`.
- Rebuild `whatdbg` and `scout` when needed so they pick up the `jam::ModelContextServer` fix.
- Regenerate Doxygen XML in `jam` (`<leader>bd`) and `ling` when Doxygen config/target is available.
- Live `carol_ling` MCP host spawns `ling --mcp project-info.md` in `C:\Users\jreng\.gemini\config\plugins\carol`; verify host CWD configuration separately if needed.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 17: Windows build, `cLang` `clang-cl` indexing, path normalization, `addUnits` rename, and audit ✅

**Date:** 2026-11-22
**Duration:** 02:00

### Agents Participated
- COUNSELOR: Diagnosed Windows build, `cLang` `clang-cl` indexing, path separator mismatch, and `units()` contract smells; validated Engineer implementations and all 4 Auditor findings; verified Pathfinder post-audit Doxygen edits; logged Sprint 17.
- Pathfinder (`flash`): Inspected Windows build/bundle pipeline, verified LSP stdio and CLI behavior on Windows, probed `language.dll` `units()`/`index()` return codes, mapped the `LanguageEntry::units` blast radius across `jam` and `ling`, and executed the post-audit Doxygen pass.
- Engineer (`inherit`): Implemented `databaseDrivers`, `argumentPrefixRemovals`, `/Fo` output extraction, CRLF trimming, `clang-cl` `/clang:` argument ordering, `getManifestDriver`/`getInstallDirectory`, `units` → `addUnits` rename across `jam` and `ling`, canonical `juce::File::getFullPathName()` normalization in `Processor.cpp`, and resolved all 4 Auditor findings (`getArguments` return-by-value, `getManifestDriver` `"arguments"` path handling, `Source/HELP.md`, `CLAUDE.md`, `README.md`, and deletion of `PLAN-cli-notification-operations-hang.md`).
- Auditor (`inherit`): Audited all 13 modified files plus root docs against `MANIFESTO.md`, `CODING.md`, `NAMES.md`, and `LANGUAGE.md` (`NEEDS_WORK`, 4 findings — all 4 confirmed and resolved).

### Files Modified (17 total: 3 in `jam`, 14 in `ling` including 1 deleted)
- `../jam/jam_syntax_tree/document/jam_LanguageInterface.h:62, 100, 162` — renamed `LanguageEntry::units` → `addUnits` and `LanguageUnits` → `LanguageAddUnits`; updated Doxygen references.
- `../jam/jam_syntax_tree/document/jam_LanguageLibrary.h:84` — renamed `LanguageLibrary::units` → `addUnits` and updated Doxygen comment.
- `../jam/jam_syntax_tree/document/jam_LanguageLibrary.cpp:13, 103` — updated `LanguageEntry::addUnits` and `LanguageAddUnits` lookup.
- `language/cLang/construction.md:360-364, 524-529, 581-586, 601-609` — replaced scalar `databaseDriver` with platform-keyed `databaseDrivers` (`c++` / `clang-cl`), removed `clang-cl.exe` from `drivers`, added `outputPrefix`, `clangDriverPrefix`, `clangClDriver`, `commandKey`, `argumentsKey`, and `argumentPrefixRemovals` (`/Fo`, `/Fd`, `/Yc`, `/Yu`, `/Fp`).
- `language/cLang/spell.md:109, 341-344, 381-384, 440-451, 489` — wired `addUnits`, `getDatabaseDrivers`, `getArgumentPrefixRemovals`, `getPrefixPosition`, `getManifestDriver`, `getInstallDirectory`, and `addUnitRows`.
- `language/cLang/language.cast:715-765, 2000-2109, 2209-2416, 2610-2880, 6074, 6095-6123` — set Windows `CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded"` and `_CRT_SECURE_NO_WARNINGS`; stripped `\r` in `getCommandOutput`, `getVersionInstallDirectory`, and `addRecord`; added `/Fo` extraction via `getPrefixPosition`; returned `ling::Texts` from `getArguments` with `/clang:` flags placed before `-- <source>`; added `compile_commands.json`-first `getManifestDriver`/`getInstallDirectory`; renamed `units` → `addUnits` and internal helper to `addUnitRows`.
- `cast/cmake.cast:217-220` — moved Windows architecture suffix on `CAST_PACK_platform` after `file(GLOB CAST_BINARY_FILES ...)` so BinaryData finds `Source/languages/Windows/*.zip`.
- `.gitignore:36` — narrowed `Source/languages/` to `Source/languages/*/` so built-in language `.h`/`.cpp` files stay tracked while per-machine platform archives stay untracked.
- `Source/Language.h:80`, `Source/BuiltInLanguage.h:69`, `Source/LibraryLanguage.h:78`, `Source/LibraryLanguage.cpp:34-36` — renamed `units` → `addUnits`.
- `Source/Processor.h:452-556`, `Source/Processor.cpp:880-883, 1269, 1309-1337` — renamed `language->addUnits`, normalized `cLang` paths through `juce::File::getFullPathName()` in `UnitsJob::getOrCreatePathIndex`, `UnitsJob::addBuildFile`, and `IndexJob::isForeign`, and updated Doxygen blocks.
- `Source/HELP.md:248, 281, 301-305` — synced `addUnits` entry/callbacks, `definition` role (`1024`), and `isForeign` callback.
- `CLAUDE.md:13, 18, 30`, `README.md:7` — synced sprint state, `ToolRelay`/`tools.json`, and removed deleted tracker links.
- `PLAN-cli-notification-operations-hang.md` — deleted completed Sprint 16 plan.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- **Windows `cLang` DLL link mismatch and warning:** `language.cast` omitted `CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded"` (conflicting with `languageCore.lib`) and `_CRT_SECURE_NO_WARNINGS`.
- **Windows BinaryData pack path and `.gitignore`:** Narrowed `.gitignore` to `Source/languages/*/` and deferred the Windows `x64` suffix on `CAST_PACK_platform` until after the BinaryData glob.
- **Windows `clang-cl` indexing produced zero `.symbols` files:** `getArguments` placed `/clang:` flags after `-- <source>` (`CXError_InvalidArguments`), `databaseDriver` (`c++`) hit MSYS2 GCC, `drivers` rewrote Visual Studio `clang-cl.exe` to MSYS2 (`'windows.h' file not found`), CRLF `\r` broke paths, and `/Fo<path>` was not parsed for `ninja -t deps`.
- **Windows path separator mismatch (`/` vs `\`):** Normalized incoming `cLang` paths through `juce::File::getFullPathName()` at the `UnitsJob`/`IndexJob` boundary so `index.paths`, `pathRows`, `unitRows`, and `Query::getPathIndex` share one canonical representation.
- **`units()` contract smells:** Renamed `units` → `addUnits` (`NAMES.md` Rule 1, Rule 5) across `jam` and `ling`, and resolved `libclang` from `compile_commands.json`'s compiler path first instead of `PATH`.
- **Audit clean sweep (4/4 resolved):** Removed out-parameter from `getArguments`, fixed space-splitting for `"arguments"` entries in `getManifestDriver`, synced `Source/HELP.md`/`CLAUDE.md`/`README.md`, and deleted completed Sprint 16 plan.

### State for Continuation
- Rebuild `ling` (`cast cast/spell.md`) after deleting `C:\Users\jreng\.config\ling\language\cLang`, then verify `.symbols` generation and Neovim `gd` (`textDocument/definition`) on Windows.
- Regenerate Doxygen XML (`doxygen Doxyfile` in `../jam` and `ninja -C Builds/Release doxygen` in `ling`) after build.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 16: CLI buffer notification hang fix and `setBuffer`/`removeBuffer` naming ✅

**Date:** 2026-11-22
**Duration:** 02:03

### Agents Participated
- COUNSELOR: Consumed `RFC-cli-notification-operations-hang.md`, built Framework Inventory, settled Option A with ARCHITECT, authored `PLAN-cli-notification-operations-hang.md`, validated Steps 1–5 against CONTRACT, verified Auditor PASS and Pathfinder Doxygen edits, and logged Sprint 16.
- Pathfinder (`flash`): Discovered CLI/LSP notification pipeline, `did*` blast radius, `cast/spell.md` codegen mappings, and completed the post-audit Doxygen pass on `Source/Query.h`, `Source/Processor.h`, and `Source/Relay.h`.
- Engineer (`inherit`): Replaced `didOpen`/`didChange`/`didClose` with `setBuffer`/`removeBuffer` in `cast/` tables, exposed `Query::setResult` as `public static`, updated `Processor::registerEvents` and `Processor::setBufferChange`, unified `Relay::setBuffer`/`Relay::removeBuffer`, and updated `Source/HELP.md`.
- Auditor (`inherit`): Audited Steps 1–5 against `MANIFESTO.md`, `CODING.md`, `NAMES.md`, `LANGUAGE.md`, and `PLAN-cli-notification-operations-hang.md` (`PASS`, 0 findings).

### Files Modified (10 total)
- `cast/identifiers.md:92, 105` — removed `didChange`, `didClose`, `didOpen`; added `removeBuffer` and `setBuffer` in alphabetical order.
- `cast/readiness.md:24-25` — replaced `Id::didOpen`/`Id::didChange`/`Id::didClose` with `Id::setBuffer` and `Id::removeBuffer`.
- `cast/parameters.md:32-33` — replaced `Id::didOpen`/`Id::didChange`/`Id::didClose` with `Id::setBuffer` and `Id::removeBuffer`.
- `Source/Query.h:258-286` — exposed `Query::setResult` as `public static` beside `Query::setError` and updated its Doxygen block.
- `Source/Processor.cpp:139-140, 682-697` — bound `Id::setBuffer`/`Id::removeBuffer`; `setBufferChange` removes `request` silently when parent has `Id::positionEncoding` (`Relay`) and writes `result: null` via `Query::setResult` otherwise (`Operation`).
- `Source/Processor.h:705-728` — updated Doxygen blocks for `setBuffer`, `removeBuffer`, and `setBufferChange`.
- `Source/Relay.h:182-198`, `Source/Relay.cpp:84-86, 323-357` — replaced `Relay::didOpen`/`didChange`/`didClose` with `Relay::setBuffer` and `Relay::removeBuffer` (preserving external `lsp::Method::did*` wire constants) and added Doxygen blocks.
- `Source/HELP.md:97-99, 156-157` — updated `Methods` and `Operation Words` tables to `setBuffer` and `removeBuffer` with CLI `result: null` contract.
- `PLAN-cli-notification-operations-hang.md` — wrote execution plan for this sprint.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- **CLI buffer notification infinite hang:** `Processor::setBufferChange` previously removed `request` without setting `Id::result`, leaving CLI `Operation` waiting forever. `setBufferChange` now checks `request.getParent().hasProperty (Id::positionEncoding)` so LSP `Relay` stays silent while CLI `Operation` receives `{"jsonrpc":"2.0","id":null,"result":null}` via `Query::setResult` and exits `0` after `Id::isIndexed` clears synchronously.
- **Foreign `did*` semantics (`NAMES.md` Rule -1, Rule 1, Rule 5):** Collapsed duplicate `Id::didOpen`/`Id::didChange` and `Relay::didOpen`/`Relay::didChange` into `setBuffer`, and renamed `didClose` to `removeBuffer`, matching `Processor::setBuffer`/`Processor::removeBuffer` 1:1 while keeping external `lsp::Method::did*` wire strings intact (`NAMES.md` Rule 2).

### State for Continuation
- Build with `cast cast/spell.md` to regenerate `Source/generated/*` and verify `ling setBuffer`/`ling removeBuffer` output and LSP `test/buffer.lua`.
- Run `ninja doxygen` after build to regenerate `docs/xml/`.
- Ephemeral `RFC-cli-notification-operations-hang.md` and `PLAN-cli-notification-operations-hang.md` remain at project root until ARCHITECT deletes or archives them.

### Debts Paid
- None

### Debts Deferred
- None

---

**End of SPRINT-LOG.md Template**

Copy this template to your project root as `SPRINT-LOG.md` and customize:
- Project name
- Repository URL/path
- Starting date
- Add project-specific rules to CRITICAL RULES section

Rock 'n Roll!  
**JRENG!**


## Handoff to COUNSELOR: ling BLESSED Cold-Index Performance & Zero-Copy Mapped SymbolDocument

**From:** COUNSELOR
**Date:** 2026-11-21
**Status:** In Progress

### Read First
- `/Users/jreng/Documents/Poems/dev/ling/PLAN-ling-lsp-0-tracker.md` — master tracker, canon rulings, and updated 2026-11-21 state.
- `/Users/jreng/Documents/Poems/dev/ling/PLAN-ling-contract-fix.md` — remaining acceptance harness (Step 15) and `scout` profile vs clangd (Step 16).
- `/Users/jreng/Documents/Poems/dev/ling/DEBT.md` — open debt `DEBT-20261121T005809` (cold-index CPU/RAM load, now verified improved; drain at `/log`).

### Context
ARCHITECT directed fixing cold-index CPU/RAM load and thermal noise in adherence to `MANIFESTO.md`, `CODING.md`, and `NAMES.md`: ignored generated/vendored TUs, single-pass `clang_indexSourceFileFullArgv` + `CXIndexOpt_SkipParsedBodiesInSession`, background-priority thread pool capped at 4 threads, clean-room single-header `jam::simd` byte scanner in `jam_core`, per-worker `IndexSession`, direct `getParameterLabel` source slicing, priority `.pch` queueing, and zero-copy `juce::MemoryMappedFile` `SymbolDocument`. All changes are built and runtime-verified on `ling` and `END`.

### Completed
- **Ignored units table:** Added `## ignoredUnits` (7 patterns: `juce_binarydata_`, `Harfbuzz`, `Sheenbidi`, `ara`, `lv2_libs`, `jam_freetype.mm`, `CompilationTime`) in `language/cLang/construction.md:622-632`, wired in `language/cLang/spell.md:382-385, 437-439` and `language/cLang/language.cast:2075-2085, 2461-2510`. Verified 0 ignored TUs indexed on both `ling` and `END`.
- **Per-worker `IndexSession` + libclang crash fixes:** Added RAII `thread_local IndexSession` (`LibClang` + `CXIndex` + `CXIndexAction`, mutex-serialized `clang_createIndex`) in `language/cLang/language.cast:464-503, 703-722`. Non-PCH disk TUs (`bufferCount == 0 and not usesPrecompiled`) run `clang_indexSourceFileFullArgv` with `CXIndexOpt_SkipParsedBodiesInSession`; PCH-consuming and live-buffer TUs run `parseTranslationUnit2FullArgv` + `indexTranslationUnit` (`language.cast:5715-5848`), eliminating both the `unsaved_files` double-free (`Indexing.cpp:521-533` + `ASTUnit.cpp:244-248`) and the PCH `IndexingConsumer::HandleTopLevelDecl` dangling-stack `SIGSEGV` (`ling-2026-11-21-171351.ips`).
- **Cached `getFileContents` + direct `getParameterLabel`:** Cached `libclang.getFileContents` per `SourceFile` in `IndexContext::preprocessedFiles` (`language.cast:2868-2931, 3537-3538`), eliminating quadratic `clang::SourceManager::translateFile`. Replaced `libclang.tokenize` in `getParameterLabel` (`language.cast:4878-4946`) with a direct `std::string_view` slice from cached `getFileContents`. Added `shouldExit` check in `onCursor` (`language.cast:5170-5171`) and 1-arg guard in `addParameterHints` (`language.cast:4904-4906`).
- **Thread pool & PCH scheduling:** Set `LINGModel::defaultThreads { 4 }` (`Source/LINGModel.h:47`), `juce::Thread::Priority::background` (`Source/Processor.cpp:10-13`), `pool->removeAllJobs (true, waitForJob)` in `Processor::~Processor()` (`Source/Processor.cpp:25-29`), prerequisite-producer (`Id::output`) priority in `Processor::addIndexJobs` (`Source/Processor.cpp:1198-1245`), and updated `Source/HELP.md:183`.
- **Zero-copy mapped `SymbolDocument`:** Replaced `SymbolDocument`'s heap `jam::Document::Element` tree with an 8-byte aligned, pointer-free binary `.symbols` format mapped read-only in place via `juce::MemoryMappedFile` (`Source/SymbolDocument.h:1-259`, `Source/SymbolDocument.cpp:1-640`), updated `Source/BuiltInLanguage.h`, `Source/Index.h/.cpp`, `Source/Processor.h/.cpp`, `Source/Query.h/.cpp`, and `Source/SwapValidator.cpp`.
- **Single-header `jam::simd` byte scanner:** Implemented `jam::simd::findByte` and `jam::simd::findNonPlainByte` (NEON `vshrn_n_u16` + `vqtbl1q_u8`, SSE2 range masks, scalar fallback) inline in `jam/jam_core/jam_Simd.h:18-351` with canonical `jam_Css.h` namespace enclosure, wired into `jam/jam_core/document/jam_Document.cpp:279-287, 461-521`.
- **Runtime validation (build `/Users/jreng/.local/bin/ling`, `Nov 21 19:47`):**
  - `ling` cold index: `161.68 s`, peak CPU `416.7%` (was `500%`), peak RSS `3357.5 MB` (was `8.0 GB`), zero crashes.
  - `END` cold index: `171.21 s`, peak CPU `422.5%` (was `490%`), peak RSS `3958.3 MB` (was `4.57 GB`), settled idle RSS `1375.8 MB` (was `3.7 GB`), zero crashes.
  - `compare end-definition`: `10.41 s`, median `0.3325 ms` (p95 `5.2409 ms`), `agree = 171`, `disagree = 10`, `stale = 77`, `lingError = 0` (was `162/19/77`), `jam_FileWatcher.cpp` `14/14` agree, `ENDView.cpp` opened cleanly (`0` `unit index failed`, `0` `daemon connection lost`).
  - Zero new `ling-*.ips` crash reports.
- **Plan hygiene:** Deleted 5 completed/superseded plans (`PLAN-ling-runs-from-anywhere.md`, `PLAN-ling-compiler-truth.md`, `PLAN-ling-ast-once.md`, `PLAN-ling-cold-index-load.md`, `PLAN-ling-blessed-index-perf.md`) and updated `PLAN-ling-lsp-0-tracker.md`.

### Remaining
- **COUNSELOR (`PLAN-ling-contract-fix.md` Step 15):** Trace and fix `juce_Component.cpp:3531:46` (`bounds`), `:4160:13` (`expectEquals`), and `:2364:28` (`getInstance` hover overrun).
- **COUNSELOR (`PLAN-ling-contract-fix.md` Step 15):** Update remaining stale `:jam` value arrays and 77 stale positions in `test/end-positions.json` and `test/expected/end-*.json` from code reads.
- **COUNSELOR (`PLAN-ling-contract-fix.md` Step 15):** Run remaining END suites (`end-methods`, `end-references`, `end-hover`, `end-inlay`, warm `end-definition` repeat, T1–T3/T6/T8/T13/T28) and all 5 KANJUT suites via `/private/tmp/claude-501/-Users-jreng-Documents-Poems-dev-ling/27ab850f-6578-4c2a-961e-c63fd0b6cfa1/scratchpad/run_test_long.sh`.
- **COUNSELOR (Diagnostic cleanup):** Remove temporary `debug::Log` / `logScope` lines (`Source/Main.cpp:419, 512`, `Source/Connection.cpp:51`, and `Source/Processor.cpp` diagnostic log writes).
- **COUNSELOR (`PLAN-ling-contract-fix.md` Step 16):** Run `scout` profile against clangd (T9, T10, T21) and optional `scout` A/B benchmark of `jam::simd` vs scalar.
- **COUNSELOR (On ARCHITECT trigger):** Run `/audit`, Pathfinder doxygen pass, and `/log` (drain `DEBT-20261121T005809`).

### Key Decisions
- *"you dont build. i will build"*
- *"always delete on disk, let the binary build and replace"* (`/Users/jreng/.config/ling/language/cLang`)
- *"hold audit and doxygen. until i explicitly said so."*
- *"jam::Format for string manipulation. dont use juce::String. only use juce::String to communicate with juce native API"*
- *"reminder: we never write byte-to-byte translation. we always CLEAN ROOM rewrite from scratch BLESSED compliance into our domain"*
- *"make it 4 thread max"*
- *"make it single header. also the canon pattern for namespace is with comment separator with its enclosure for example jam_Css"*

### Files Modified
- `ling/language/cLang/construction.md` — added `## ignoredUnits` and `indexSourceFileFullArgv`.
- `ling/language/cLang/spell.md` — wired `ignoredUnits` into `getIgnoredUnits` / `isIgnoredUnit`.
- `ling/language/cLang/language.cast` — ignored-units filter, `IndexSession`, PCH/unsaved routing, cached `getFileContents`, direct `getParameterLabel` slice, `onCursor` exit check, `addParameterHints` 1-arg guard, `reportOutline` cursor TU fallback.
- `ling/Source/LINGModel.h`, `ling/Source/LINGModel.cpp`, `ling/Source/HELP.md` — `defaultThreads = 4`, background priority docs.
- `ling/Source/SymbolDocument.h`, `ling/Source/SymbolDocument.cpp` — zero-copy `juce::MemoryMappedFile` binary `.symbols` format and `Builder`.
- `ling/Source/BuiltInLanguage.h`, `ling/Source/Index.h`, `ling/Source/Index.cpp`, `ling/Source/Processor.h`, `ling/Source/Processor.cpp`, `ling/Source/Query.h`, `ling/Source/Query.cpp`, `ling/Source/SwapValidator.cpp` — zero-copy `SymbolDocument` integration, `~Processor` job wait, priority `.pch` queue.
- `ling/PLAN-ling-lsp-0-tracker.md` — updated 2026-11-21 state and verified numbers.
- `ling/test/end-positions.json`, `ling/test/end-references-positions.json`, `ling/test/end-symbols.json`, `ling/test/expected/end-definition.json` — retargeted deleted `jam_Listener.cpp` to `jam_FileWatcher.cpp`.
- `jam/jam_core/jam_Simd.h`, `jam/jam_core/document/jam_Document.cpp` — single-header `jam::simd` byte scanner (`findByte`, `findNonPlainByte`) wired into `jam::Document`.

### Open Questions
None.

### Next Steps
- Resume `PLAN-ling-contract-fix.md` Step 15 (fix `bounds`/`expectEquals`/`getInstance` hover, refresh stale END expectations, run remaining END and KANJUT suites, remove temporary `debug::Log` lines), then Step 16 (`scout` vs clangd). Wait for ARCHITECT's explicit trigger before running `/audit`, doxygen pass, or `/log`.
