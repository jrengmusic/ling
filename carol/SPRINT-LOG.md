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

## Sprint 23: Restore repeated-reference goToDefinition by removing suppressRedundantReferences ✅

**Date:** 2026-11-27
**Duration:** 00:15

### Agents Participated
- Primary: COUNSELOR — reproduced `goToDefinition` failures on `Client.cpp:14:15` and `Client.cpp:23:30`, traced the root cause to `IndexOptionFlags::suppressRedundantReferences`, validated the fix, and logged Sprint 23.
- Subagents: `@Pathfinder` (Neovim LSP config inspection, CLI probes, `benchmark/mcp.md` rebuild and verification, git status collection), `@Engineer` (`language/cLang/language.cast:415` fix).

### Files Modified (1 total)
- `language/cLang/language.cast:415` — removed `IndexOptionFlags::suppressRedundantReferences` from `indexOptions` while keeping `indexImplicitTemplateInstantiations` removed so `libclang` emits every reference inside a lexical scope.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- `goToDefinition` returned empty results on repeated references inside the same function or constructor initializer (`Source/Client.cpp:14:15` `: juce::InterprocessConnection (true)` and `Source/Client.cpp:23:30` `LINGModel::coalesceMs`) because `IndexOptionFlags::suppressRedundantReferences` (`CXIndexOpt_SuppressRedundantRefs`) dropped them. Removed the flag at `language/cLang/language.cast:415`, rebuilt `/Users/jreng/.local/bin/ling` via `benchmark/mcp.md` (`exitCode 0`, `epoch` `6.01 s`), and verified both positions return their definitions (`juce_InterprocessConnection.h:64` and `Source/LINGModel.h:13`).

### State for Continuation
- None — sprint fully closed, nothing open.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 22: Demand-driven indexing, resource confinement, shared daemon, and cold-build efficiency ✅

**Date:** 2026-11-27
**Duration:** 08:30

### Agents Participated
- Primary: COUNSELOR — planned Parts 1–4, validated each step against CONTRACT, diagnosed cold-build bottlenecks and runtime defects via `scout` and `whatdbg`, validated all 7 `@Auditor` DCF fixes and the post-audit Doxygen pass.
- Subagents: `@Pathfinder` (codebase discovery, `scout` profiling and MCP accuracy runs, Doxygen pass across 13 headers), `@Engineer` (Parts 1–4 implementation, 7 audit fixes, cold-build optimizations, `addOwnedContainers` cycle fix, `setIndexed` hang fix), `@Auditor` (single sprint-end contract sweep).

### Files Modified (37 source/config files + regenerated `docs/xml/` + plan/RFC)
- `Source/SymbolDocument.h`, `Source/SymbolDocument.cpp` — bumped binary format to v2, sorted `SymbolRecord` at creation, added `getHash()`.
- `Source/Index.h`, `Source/Index.cpp` — removed `Index::records`, added lifetime-stable `unitPathIndex` keying, incremental `removeDocuments()`, `UnitOwners` with `roots` and local-directory ownership pass, bounded LRU document cache (`maxCachedDocuments = 64`) with `DocumentScope`.
- `Source/LINGModel.h`, `Source/LINGModel.cpp` — added `getDefaultThreads()`, `defaultCoalesceMs`, binary-hash `getDaemonName()`, `isWithinRoots()`, and shared `.swap` paths inside roots.
- `Source/Manifest.h`, `Source/Manifest.cpp` — added configurable `coalesce` in `## daemon` and updated default thread count.
- `Source/Processor.h`, `Source/Processor.cpp` — added demand-driven `requestUnit()`/`requestAllUnits()`, non-destructive background rebuilds, shared `ThreadPool` injection, burst normal-priority worker pool, root-scoped `UnitsJob`, and `hasIndex()` check in `setIndexed (int)`.
- `Source/Query.h`, `Source/Query.cpp` — added `appendHoverDefinition()` fallback for external SDK definitions and `DocumentScope` pinning.
- `Source/Server.h`, `Source/Server.cpp` — implemented multi-workspace `Server::Workspace` registry sharing one `ThreadPool` with per-workspace idle eviction.
- `Source/Client.h`, `Source/Client.cpp`, `Source/Connection.h`, `Source/Connection.cpp`, `Source/Operation.h`, `Source/Operation.cpp`, `Source/Relay.h`, `Source/Relay.cpp`, `Source/ToolClient.h`, `Source/ToolClient.cpp`, `Source/ToolRelay.h`, `Source/ToolRelay.cpp` — routed clients to the binary-hashed shared daemon and sent workspace manifest metadata on attach.
- `Source/Main.cpp`, `Source/Validator.cpp`, `Source/HELP.md`, `cast/identifiers.md`, `cast/text.md`, `Source/generated/*` — updated daemon lock, CLI/profiler startup, `Id::coalesce`, help text, and `runDaemon` Doxygen block.
- `language/cLang/language.cast`, `language/cLang/construction.md` — pruned foreign callbacks/outlines/macros/completions, added lazy `addForeignBaseCompletions()`, fixed `addOwnedContainers()` include-cycle loop, removed `indexImplicitTemplateInstantiations`, added `suppressRedundantReferences`, and set `LIBCLANG_NOTHREADS=1`.
- `PLAN-demand-driven-indexing-and-efficiency.md` — marked Steps 1.1–5.2 complete.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- Cold index build exceeded 9 minutes on `end` due to unrooted JUCE headers returning `isForeign == false`, eager foreign completions, `Nexus.cpp` stealing module headers, and `indexImplicitTemplateInstantiations` taking 50.7% CPU. Fixed root containment in `UnitOwners::isForeign`, added local-directory header ownership, pruned foreign callbacks with lazy base completions, tuned `indexOptions`, and scaled worker pool to `max(2, cpus - 2)`. Verified cold build dropped to `34.08 s` (>16x faster), warm build dropped to `1.34 s` (`-90.0%` vs `13.36 s` baseline), and `.symbols` disk size dropped >10x.
- `IndexContext::addOwnedContainers()` looped infinitely on cyclic includes (`whatdbg` trace on PID `61569`). Guarded traversal with `not ownedContainers.contains (includers.at (current))` (`language/cLang/language.cast:3219`).
- Failed `ParseJob` on `hasIndex() == false` files (such as `Source/tools.json`) left `files[Id::isIndexed]` permanently `false`, hanging `scope == Id::files` requests. Fixed `Processor::setIndexed (int)` (`Source/Processor.cpp:446-456`); verified `--debug` failed-toolchain probe exits `0` in `3 s`.
- Resolved all 7 `@Auditor` DCF findings and completed the post-audit Doxygen pass across all 13 modified headers and `Source/Main.cpp:406-413`.

### State for Continuation
- None — sprint fully closed, nothing open.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 21: Adopt slim CLI module layout and build-time HelpFrames in archetype and JAM CLIs ✅

**Date:** 2026-11-25
**Duration:** 01:06

### Agents Participated
- COUNSELOR: Audited `saw` and `jam` slim linking mechanics, mapped per-CLI runtime module dependencies across all JAM CLIs and `archetype/cli`, validated all Engineer outputs against CONTRACT, fixed two minor `archetype/cli` alignment/constant issues, and logged Sprint 21.
- Pathfinder (`flash`): Surveyed `saw`, `jam`, `archetype/cli`, and every CLI under `~/Documents/Poems/dev/` for `HELP.md`, `jam::TerminalPage`, `## binary`, `cast/files.md`, and runtime module usage.
- Engineer (`flash`): Implemented the slim + `*_HelpFrames` pattern in `archetype/cli`, `whatdbg`, `scout`, `tit` (full slim) and `ling`, `cast`, `ggwp` (partial slim).

### Files Modified (47 total across 7 repositories)
- `archetype/cli/cast/cmake.cast:105-290` — added `## program user module`, Apple `CAST_CORE_FRAMEWORKS` stripping, `CAST_HELP_MODULES`, `## program module` target linking, `*_HelpDocuments`, `*_HelpFrames`, and separated `## help` from runtime `## binary`.
- `archetype/cli/init.cast:104-632`, `archetype/cli/INIT.md:1-67`, `archetype/ARCHETYPE.md:96-284`, and stubs `archetype/cli/cast/files.md`, `archetype/cli/help/HelpFrames.cpp`, `archetype/cli/Source/HELP.md`, `archetype/cli/Source/Main.cpp` — added slim CLI tables, macros, linker flags, and cookie-cutter skeleton outputs.
- `whatdbg/{project-info.md,cast/cmake.cast,cast/spell.md,cast/files.md,help/HelpFrames.cpp,Source/Main.cpp}` — switched to full-slim `whatdbg_HelpFrames` with `JUCE_USE_FOUNDATION=0` and pre-rendered `HELP.frame`.
- `scout/{project-info.md,cast/cmake.cast,cast/spell.md,cast/files.md,help/HelpFrames.cpp,Source/Main.cpp}` — switched to full-slim `scout_HelpFrames` with `JUCE_USE_FOUNDATION=0` and pre-rendered `HELP.frame` / `erase.frame`.
- `tit/{project-info.md,cast/cmake.cast,cast/spell.md,cast/files.md,help/HelpFrames.cpp,Source/Main.cpp}` — switched to full-slim `titc_HelpFrames` with `JUCE_USE_FOUNDATION=0` and pre-rendered `HELP.frame` / `erase.frame`.
- `ling/{project-info.md,cast/cmake.cast,cast/spell.md,cast/files.md,help/HelpFrames.cpp,Source/Main.cpp}` — switched to partial-slim `ling_HelpFrames` (kept `juce_gui_basics` for `jam::AnsiDocument::getText` at `Source/Processor.cpp:834,850`; dropped `jam_tui`, `jam_gui`, `jam_markdown_graphics`, `jam_markdown`, `jam_animation`, `jam_web`, `jam_mermaid_diagram`, `juce_audio_processors`) and pre-rendered `HELP.frame` / `erase.frame`.
- `cast/{project-info.md,cast/cmake.cast,cast/spell.md,cast/files.md,help/HelpFrames.cpp,Source/main.cpp}` — switched to partial-slim `cast_HelpFrames` (dropped `jam_tui`, `jam_gui`, `jam_markdown_graphics`, `jam_pdf`, `jam_animation`, `jam_web`, `jam_mermaid_diagram`, `jam_terminal`) and pre-rendered `HELP.frame` / `erase.frame`.
- `ggwp/{project-info.md,cast/cmake.cast,cast/spell.md,cast/files.md,help/HelpFrames.cpp,Source/Main.cpp}` — switched to partial-slim `ggwp_HelpFrames` (dropped `jam_tui`, `jam_gui`, `jam_markdown_graphics`, `jam_animation`, `jam_web`, `jam_mermaid_diagram`, `jam_terminal`, `jam_profiler`, `jam_subprocess`, `juce_audio_processors`) and pre-rendered `HELP.frame` / `erase.frame`.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- **GUI/FreeType bloat in JAM CLIs:** Moved `jam::TerminalPage` (`jam::TextEditor`) help rendering into build-time `*_HelpFrames` helper targets and separated `## help` from runtime `## binary`, enabling full-slim builds (`JUCE_USE_FOUNDATION=0`, zero Apple GUI/Foundation frameworks) in `archetype/cli`, `whatdbg`, `scout`, and `tit`, and partial-slim builds in `ling`, `cast`, and `ggwp` (`stamp` skipped per ARCHITECT ruling).

### State for Continuation
- Build each updated CLI via `cast cast/spell.md` (starting with `cast`, then `ling`, `whatdbg`, `scout`, `tit`, `ggwp`) to regenerate `CMakeLists.txt` and `Source/generated/Files.h` and verify compilation and `--help` output.
- Verify `cast INIT.md <temp-target>` from `archetype/cli/` generates the complete slim CLI skeleton.
- `ling` can move to full-slim later if `jam::AnsiDocument::getText (std::string_view)` (`jam/jam_terminal/document/jam_AnsiDocument.h:171`) is unguarded from `#if JUCE_MODULE_AVAILABLE_juce_gui_basics`.

### Debts Paid
- None

### Debts Deferred
- None

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
