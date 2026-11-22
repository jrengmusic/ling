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

## Sprint 15: MCP Part 6 (ling --mcp), help layout & doxygen hygiene, T28 built-ins, and END fixture refresh ✅

**Date:** 2026-11-22
**Duration:** 02:43

### Agents Participated
- COUNSELOR: Root-cause investigation of Neovim `--debug` mismatch, terminal markdown rendering analysis, `RFC-nvim-ling-configuration.md` authorship, `HELP.md` column-width tuning, `style.css` inline-code padding fix, MCP Part 6 orchestration, Design by Contract filtering of all 3 Auditor findings, Doxygen verification, `CLAUDE.md` refresh, MCP tracker update, and sprint log.
- Engineer (`inherit`): Implemented MCP Part 6 (`Source/tools.json`, `Source/ToolRelay.h/.cpp`, `--mcp` CLI wiring, `test/mcp.lua`, `benchmark/mcp.md`/`mcp.sh`), fixed `ToolRelay::messageReceived` container JSON span, resolved Auditor Finding 3, created self-contained T28 built-in fixtures (`test/builtin/`, `test/config/builtin.json`, `test/lsp.lua`), refreshed END fixtures (`end-positions.json`, `end-definition.json`, `end-methods.json`, `end-hover.json`), and deleted completed plan/RFC files.
- Auditor (`inherit`): Audited MCP Part 6 scope against CONTRACT and `PLAN-model-context-protocol-6-ling.md` (3 findings, all 3 resolved).
- Pathfinder (`flash`): Verified codebase/runtime state, ran END/KANJUT/T28 acceptance suites, ran `scout` verification of `benchmark/mcp.md`, fixed Sprint 14 leftover Doxygen warnings (`jam_UriReference.h`, `MarkdownLanguage.h`), and completed the post-audit Doxygen pass on `Source/ToolRelay.h`.

### Files Modified (32 total)
**LING**
- `Source/ToolRelay.h`, `Source/ToolRelay.cpp` — added MCP tool relay (`Client` + `jam::ModelContextServer`), zero-copy `JsonDocument::read` with `Id::range` container span, `Server::noPort` null drain, and post-audit Doxygen pass.
- `Source/tools.json` — added 13 read-only MCP tool schemas matching `Query.cpp` result shapes.
- `Source/Main.cpp:8, 304, 354-361, 525` — wired `--mcp` command and `runToolRelay`.
- `Source/HELP.md:1-14, 59` — tuned YAML `column-widths` (`Method`, `Operation`, `Arguments`, `Word`, `File`, `Entry`, `Kind`, `Callback`, `Called by`) and documented `ling --mcp`.
- `Source/languages/MarkdownLanguage.h:14-28` — documented `MarkdownLanguage::getSymbol` (`file`, `name`, `role`) to clear Doxygen warning.
- `project-info.md:195`, `cast/files.md:41`, `cast/identifiers.md:75` — embedded `Source/tools.json` as `files::lingTools` and added `Id::mcp`.
- `test/mcp.lua`, `test/baseline/mcp.txt`, `benchmark/mcp.md`, `benchmark/mcp.sh`, `benchmark/mcp-report.md` — added 8-scenario MCP smoke harness and `scout` manifest (`exit 0`, `scout: Fine.`).
- `test/builtin/project-info.md`, `sample.{md,css,json,xml,html,mmd}`, `test/config/builtin.json`, `test/lsp.lua:81-82` — added self-contained T28 built-in fixture project (`8/8 PASS`, `0 SKIP`) and nil-checked `project` in `lsp.lua`.
- `test/end-positions.json`, `test/expected/end-definition.json`, `test/expected/end-methods.json`, `test/expected/end-hover.json` — removed 2 deleted END positions (`exclusiveZoom`, `setRow`) and refreshed shifted target lines verified on disk (`0 disagree`, `0 stale`).
- `CLAUDE.md` — refreshed orientation facts via `synthesis-writer`.
- Deleted: `RFC-ling-runs-from-anywhere.md`, `MACHINIST-REPORT-nvim-ling-test.md`, `PLAN-ling-0-tracker.md`, `PLAN-ling-lsp-0-tracker.md`.

**JAM & `~/.config/`**
- `jam/jam_tui/page/style.css:152` — set `inlineCodePaddingHorizontal: 0em` to remove extra blank cells around inline code in terminal markdown rendering.
- `jam/jam_model_context_protocol/schema/jam_UriReference.h:46` — removed backticks around `.` and `..` to clear Doxygen `<tt>` mismatch warning.
- `jam/PLAN-model-context-protocol-0-tracker.md:60` — marked Part 6 (`ling --mcp`) done on macOS.
- `~/.config/RFC-nvim-ling-configuration.md` — authored MACHINIST RFC for deterministic Neovim `lingCmd` built-configuration selection.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- **MCP Part 6 (`ling --mcp`):** Fixed container result serialization in `ToolRelay::messageReceived` where `JsonDocument::get` returns a 1-byte opening token (`[`/`{`) by reading the full JSON span from `getChildByToken (root, resultToken)->get<Tokens> (Id::range)->at (0)` (zero copy). Resolved all 3 Auditor findings (`BinaryData::getString`, `completion` and `inlayHint` descriptions). Verified `scout benchmark/mcp.md` exits `0` (`scout: Fine.`).
- **Help rendering & Doxygen:** Fixed `HELP.md` column proportions via YAML `column-widths` and removed extra inline-code spaces by setting `inlineCodePaddingHorizontal: 0em` in `jam/jam_tui/page/style.css:152`. Cleared both Sprint 14 leftover Doxygen warnings.
- **Neovim `Client.h` go-to-definition (`No locations found`):** Traced root cause to `nvim/lua/lsp/servers.lua:19-31` passing `--debug` from `.project` when `Builds/Debug` does not exist; wrote `~/.config/RFC-nvim-ling-configuration.md` for MACHINIST.
- **T28 built-ins & END fixtures:** Self-contained `test/builtin/` passes all 6 built-in languages (`8/8 PASS`, `0 SKIP`); END `definition` (`256/0`), `methods` (`0 disagree`), and `hover` (`230/0`) pass with zero stale or disagreeing entries.

### State for Continuation
- `ling` is complete on macOS (LSP Parts 1–8 and MCP Part 6).
- Remaining work outside macOS `ling`: Windows verification (`stdio-client-windows.md`, `benchmark/mcp.md`), MCP Parts 7–8 (`whatdbg --mcp`, `scout --mcp`) and Close in `jam/PLAN-model-context-protocol-0-tracker.md`, `~/.config/RFC-nvim-ling-configuration.md` (MACHINIST), and scope ruling on Sprint 14 Auditor 2 Finding `#4` (`jam_MermaidDiagram`).

### Debts Paid
- None

### Debts Deferred
- None

---

## Sprint 14: LING Part 8 close — contract fixes, profiling vs clangd, full-plan ling + jam audit, and doxygen pass ✅

**Date:** 2026-11-22
**Duration:** Multi-session sprint (2026-11-21 to 2026-11-22).
**Scope:**
- `PLAN-ling-contract-fix.md` Steps 15–16 and `PLAN-ling-lsp-0-tracker.md` Part 8 Steps 1, 3, 4, 6, 7.
- ARCHITECT directives this session:
  - *"finish 2 first"* (T21 CPU benchmark vs `clangd`, refresh stale END/KANJUT fixtures)
  - *"1 until /log"* (Step 3 `/audit`, Step 4 Pathfinder doxygen pass, Step 6 plan hygiene, Step 7 `/log`)
  - *"include all both jam and ling"* (widen Step 3 audit and Step 4 doxygen to the full `PLAN-ling-lsp` scope across `ling` and `jam`)
  - *"can you update jam plan mcp tracker to write your understanding especially for scout with lazy mcp to reserve context tax?"* (recorded as R19 in `jam/PLAN-model-context-protocol-0-tracker.md`)

### Agents Participated
- COUNSELOR: Step 15–16 verification, `scout` profiling, T21 CPU benchmark, Design by Contract filtering across all 61 Auditor findings, Doxygen verification across 40 headers, plan hygiene, `jam` MCP tracker R19, and this log.
- Auditor (`pro`, 3 runs): Pass 1 (`ling` + recent `jam`, 27 findings), Pass 2 `ling` full-plan + Carry C (14 findings), Pass 2 `jam` full-plan + MCP + parsers + Carry C (20 findings).
- Engineer (`inherit`): Step 15 fixes, diagnostic cleanup, fixture refresh, all 58 resolved audit fixes across `ling` and `jam`, and `PLAN-ling-contract-fix.md` deletion.
- Pathfinder (`flash`, 3 runs): Post-audit comprehensive Doxygen pass across 40 headers (20 `ling`, 20 `jam`; 0 doxygen warnings).

### Files Modified
**LING**
- `Source/Validator.cpp`, `Manifest.h/.cpp`, `Query.h/.cpp`, `Processor.h/.cpp`, `SymbolDocument.h/.cpp`, `Index.h/.cpp`, `Server.h/.cpp`, `BuiltInLanguage.h`, `Language.h`, `LibraryLanguage.h/.cpp`, `languages/HtmlLanguage.h/.cpp`, `Main.cpp`, `Connection.h/.cpp`, `Relay.h/.cpp`, `Client.h/.cpp`, `Operation.h/.cpp`, `SchemaSuite.h/.cpp`, `StdioClientProbe.h/.cpp`, `SwapValidator.h`, `Legend.h`, `LINGModel.h`.
- `language/cLang/construction.md`, `language/cLang/spell.md`, `language/cLang/language.cast`, `language/core/core.cast`.
- `cast/text.md`, `cast/language.md`, `Source/generated/HashMaps.h`.
- `test/lsp.lua`, `test/buffer.lua`, `test/keymaps.lua`, `test/methods.lua`, `test/builtin.lua`, `test/end-positions.json`, `test/end-references-positions.json`, `test/expected/*`.
- `benchmark/end-hover-sample-report.md`, `PLAN-ling-lsp-0-tracker.md`, `PLAN-ling-contract-fix.md` (deleted), `carol/SPRINT-LOG.md`.

**JAM**
- `jam_core/jam_Simd.h`, `jam_core/document/jam_Document.h/.cpp`, `jam_core/document/jam_DocumentIndex.cpp`.
- `jam_core/json/jam_JsonDocument.h/.cpp`, `jam_core/json/jam_JsonWriter.h/.cpp`, `jam_core/json/jam_JsonValidator.h/.cpp`.
- `jam_core/xml/jam_Xml.h/.cpp`, `jam_web/html/jam_Html.h/.cpp`, `jam_web/css/jam_Css.h/.cpp`, `jam_terminal/document/jam_AnsiDocument.h/.cpp`.
- `jam_markdown/document/jam_MarkdownDocument.h/.cpp`, `jam_MarkdownBlockParser.cpp`, `jam_MarkdownInlineParser.cpp`.
- `jam_syntax_tree/document/jam_LanguageInterface.h`, `jam_LanguageLibrary.h/.cpp`, `jam_LanguageDocument.h/.cpp`, `jam_LanguageValidator.h/.cpp`.
- `jam_model_context_protocol/protocol/jam_ModelContextProtocol.h/.cpp`, `server/jam_ModelContextServer.h/.cpp`, `schema/jam_JsonSchema.h/.cpp`, `jam_JsonPointer.h/.cpp`, `jam_JsonNumber.h/.cpp`, `jam_UriReference.h/.cpp`.
- `cast/core.md`, `cast/markdown.md`, `generated/jam_Core.h/.cpp`, `generated/jam_Markdown.h/.cpp`, `PLAN-model-context-protocol-0-tracker.md` (R19).

### Alignment Check
- [x] BLESSED principles followed: zero-copy `SymbolDocument` mmap, `jam::JsonDocument::read` zero-copy reads, stateless `LanguageValidator`, no `Target&` out-parameters in `SwapReader`, uniform `Query::hover` signature, `jam::Strings` / `jam::Array` / `jam::Format` / `jam::bit_cast` used throughout.
- [x] NAMES.md adhered: banned `kind` removed from `test/buffer.lua` and `test/keymaps.lua`; `Type` -> `type` on cLang structs; `addPayload` -> `readPayload`.
- [x] Code Hygiene: comments and doxygen written exclusively in the dedicated post-audit Pathfinder pass (40 headers, 0 doxygen warnings).

### Problems Solved
- **Step 15 runtime fixes (verified on `2026-11-21 21:29:09` build):**
  - `getDefinitionRange` (`language/cLang/language.cast`): gated `hasBody` and `definitionRange` on `extentFile` (fixes `getInstance` hover overrun).
  - `ignoredConditionMacros` (`language/cLang/construction.md`, `spell.md`, `language.cast`): filters `_`-prefixed and function-like macros in `addConditionFlags` (fixes `bounds` and `expectEquals` in `juce_Component.cpp`).
  - Removed temporary `logScope` and 10 `debug::Log` lines from `Main.cpp`, `Connection.cpp`, `Processor.cpp`.
  - Verified: warm END `definition` `181/0` (`0.1852 ms` median), `hover` `156/0` (`0.2038 ms` median), `methods` `0 disagree` across 870 checks (`T12`, `T14`, `T15`, `T16`, `T17`, `T20`), `end-buffer` `21/21`, `end-builtin` `markdown`/`css` PASS, Step 15e auto-quit and config reseed PASS.
- **Step 16 profiling & T21 CPU:**
  - `scout` `benchmark/end-hover-sample.md`: exit `0`, hover `0.1233 ms` median / `0.5729 ms` p95 vs `clangd` `0.4327 ms` / `6.4326 ms`, `0` libclang samples.
  - T21 CPU (warm END): active session `ling` daemon `2.11 s` (combined `6.04 s`) vs `clangd` `39.88 s` (**18.9× less daemon CPU**); warm-start `8.34 s` vs `389.97 s`; 60 s idle `0.24 s` vs `0.00 s`.
- **Audit Pass 1 (27 findings: 26 resolved, 1 rejected):**
  - Resolved: `#1` `Query.cpp` merged `textObjectRoles`/`innerDelimiters` map with `find`; `#2` `Processor.cpp` boundary `try/catch` in `ParseJob`/`IndexJob`/`UnitsJob`; `#3` `Query.cpp` `fileRow.isValid()` guard; `#4` `language.cast` `addHint` `keptRanges.isEmpty()` gate; `#5` `construction.md` `platformTargets` `WordMap`; `#6` `language.cast` `applyToTranslationUnit`; `#7` `SymbolDocument` `getHeader()` `std::memcpy` + `Span<T>`; `#8` `Query::typeDefinition` single pass; `#9` `Query::getRecordType` `.first()`; `#10` `Index::getSymbolDocument` to `.cpp`; `#11` `Server` lambdas to `.cpp`; `#12` `Server::progressValues` `std::string_view`; `#14` `jam_Simd.h` `findOffset`; `#15` `jam_Simd.h` `or`; `#16` `jam_Document.cpp` `jam::simd::isPlainAsciiByte`; `#17` `jam_LanguageLibrary.cpp` structured bindings; `#18` `BuiltInLanguage.h` `jam::Array`/`jam::Strings`; `#19` `SymbolDocument` `jam::Array<FileBuilder>`; `#20` `core.cast`/`language.cast` `ling::Texts`; `#21` `IndexSession::setLoaded`/`isLoaded`; `#22` `namespace HeaderHint`; `#23` `Type` -> `type`; `#24` `HoverLabel::arrowOperator`; `#25` `Index.cpp` `try_emplace` binding; `#26` `Main.cpp` direct `jam::Strings` + removed nested `#if`; `#27` `Query.cpp` spacing.
  - Rejected: `#13` (`LINGModel.h:47` `defaultThreads { 4 }`, `Processor.cpp:12` background priority) — explicitly ruled by ARCHITECT (2026-11-21).
- **Audit Pass 2 — `ling` full-plan + Carry C (14 findings: 14 resolved):**
  - `#1` `Validator.cpp:15-22` null guard on `headerRow`; `#2` HTML `href="#id"` slot 2 (`cast/language.md`, `HashMaps.h`, `BuiltInLanguage.h`, `HtmlLanguage.*`); `#3` `Manifest.cpp` `jam::Format::toFileExtension`; `#4` `Manifest` `getDaemonValue`; `#5` `Query.cpp` `jam::Format::trim`; `#6` deleted `Processor::runHover`/`Query::applyToHover` + unified `Query::hover`; `#7` `Query::applyToArrayResult` across 14 array handlers; `#8` `ParseJob::runJob` buffered branch skips disk read/hash/swap; `#9` `SchemaSuite.cpp`/`Operation.cpp` `jam::JsonDocument::read`; `#10` `Relay::isSessionActive()` guard on notifications; `#11` moved single-function constants out of 5 headers (`StdioClientProbe`, `SchemaSuite`, `Client`, `Manifest`, `Relay`); `#12` deleted dead `failNoUnit` in `cast/text.md`; `#13` extracted shared Lua helpers to `test/lsp.lua`, removed dead `legendTypes`, renamed `kind` -> `type`/`objectType`; `#14a/b/c` `language.cast` `addCallRecords` in `cursorRecords`, reused `getClosingIndex`, `getUnbracedBodyEnd` `ling::AddressTable` + `getControlEnd`.
- **Audit Pass 2 — `jam` full-plan + MCP + parsers + Carry C (20 findings: 18 resolved, 1 rejected, 1 scope-held):**
  - Resolved: `#1` `ModelContextServer::listTools` single-line `JsonWriter::appendValue` (fixes `"["` bug); `#2` `ModelContextServer` client token-type/container validation + `jassert (object.id == Id::object)` in `JsonDocument`; `#3` `jassert (onInputClosed != nullptr)` + `if (onCancelled != nullptr)`; `#5` `Id::range` in `XmlDocument`/`HtmlDocument::getStructuralKeys()`; `#6` `CssDocument::addSection` skips `Id::range` + deleted `addWindow`; `#7` moved `Id::href` to `jam_Core` + `HtmlDocument::addAttributes` slot 2 + skip zero-length placeholder in `Document::addNameTokens`; `#9` `SwapReader::readValue`/`readPayload` value returns (removed `Target&` out-parameters); `#10` `jam::bit_cast` in `SwapReader::read<double>`; `#11` removed public `Document::getArena()`; `#12` `JsonValidator::getContainerFailures` uses `applyFunctionRecursively`; `#13` stateless `LanguageValidator::addRules`; `#14` `LanguageLibrary::counterparts` `jam::Strings` + coordinated `ling` callers; `#15` `HtmlDocument` `constexpr` lengths, dropped 5 unused params, deleted `getOrCreateAttributes`; `#16` `AnsiDocument` narrowest scope, dropped unused capture, deleted duplicate `private:`; `#17` `JsonNumber` uses `Document::decimalRadix`; `#18` `Document.cpp` `Chars::space`/`lowerZ` + structured bindings; `#19` `CssDocument` direct map population + brace init on `Document.cpp`/`MarkdownDocument.cpp`; `#20` `CODING.md:671` namespace enclosure banners across 29 files.
  - Rejected: `#8` (remove `JsonDocument::stream` by replacing container `Id::tokens` with `Id::index`) — rejected per Design by Contract Filter because `Relay.cpp:326`, `SchemaSuite.cpp:17, 43`, and external JAM consumers read `container.get<Document::Tokens> (Id::tokens)` directly.
  - Scope-held for ARCHITECT: `#4` (`jam_MermaidDiagram` 3,953-line class split across 30+ diagram-family `.cpp` files in `jam_mermaid_diagram/`, outside this sprint's file set).

### State for Continuation
- All 58 post-audit code fixes and 40 doxygen-updated headers across `ling` and `jam` are on disk and **awaiting ARCHITECT's build** to compile-verify (plus `cast doc` for JAM doxygen XML regen).
- Open items for next sprint(s):
  - ARCHITECT scope ruling on Auditor 2 Finding `#4` (`jam_MermaidDiagram` 30+ files).
  - T21 60 s idle CPU (`0.24 s` vs `0.00 s` clangd), T10 cold/restart first-answer wall time, `jam::simd` benchmark, re-running KANJUT suites and remaining T28 built-ins (`json`/`xml`/`html`/`mermaid`), and refreshing the 63 shifted target lines in `test/expected/end-definition.json`.
  - Next planned work per locked order: MCP Parts 6–8 (`ling --mcp`, `whatdbg --mcp`, `scout --mcp`) and `RFC-ling-runs-from-anywhere.md`.

### Debts Paid
- `DEBT-20261121T005809`: Cold-index CPU/memory spike fixed via `SymbolDocument` zero-copy mmap, `ignoredUnits`, `SkipParsedBodiesInSession`, `ignoredConditionMacros`, `jam::simd`, and 4 background worker threads (verified: `ling` peak RSS `8.0 GB -> 3.36 GB`, `END` idle RSS `3.7 GB -> 1.38 GB`, active session CPU `18.9×` below `clangd`, hover `3.5×` faster median / `11.2×` faster p95).

### Debts Deferred
- None

## Handoff to COUNSELOR: ling contract fix, Step 15 acceptance harness in progress

**From:** COUNSELOR
**Date:** 2026-11-21
**Status:** In Progress

### Read First
1. `PLAN-ling-contract-fix.md`: the locked plan. Steps 1-14b are done. Steps 15, 15b-15g and 16 hold the open work, with their evidence and results.
2. `PLAN-ling-lsp-0-tracker.md:318-336`: the T1-T29 acceptance harness and its validation.
3. `Source/Server.cpp:3-17`, `:101-105`, and `Source/Server.h:17-20`, `:84`, `:107`: the Step 15e fix.
4. `test/compare.lua:5-56` and `test/lsp.lua:129-141`, `:297-301`: how a T4 run opens a file and marks a position stale.
5. `/private/tmp/claude-501/-Users-jreng-Documents-Poems-dev-ling/27ab850f-6578-4c2a-961e-c63fd0b6cfa1/scratchpad/run_test_long.sh`: the run script (`<test> <config>`, 560 s limit). The scratchpad can be cleared by a reboot.

### Context
The sprint fixes the contract violations of ling, then makes each answer of the acceptance harness agree with the code read (Step 15), then profiles against clangd (Step 16). Step 15 runs T1-T29 on END and KANJUT and fixes each failure from a read and a trace.

### Completed
- **Step 15b (identity):** `IndexContext::getSymbolHash` mixes the kept range of the canonical declaration into the USR hash, during context passes only (`language/cLang/language.cast:2814-2826`). Used at `:2867`, `:2921`, `:3558`, `:3874`. Read and validated. Not runtime-verified: its validation target (`folder` at `jam_Listener.cpp:24:60`) no longer exists after the jam pull.
- **Step 15d (no change):** daemon pid 95876 (started 2026-11-20 07:14) mapped `~/.cache/ling/language/7d38fde1…/cLang/liblanguage.dylib` (lsof). A daemon of the 06:08 binary mapped `cf4e3760…` (lsof at 5, 20 and 45 s). The unpacked library, `Builds/languages/cLang/build/liblanguage.dylib` and the `cLang.zip` entry all had sha256 `e0d752f3…`. `cast/cmake.cast` needs no change (`PLAN-ling-contract-fix.md`, Step 15d Result).
- **Step 15e (cause and fix):**
  1. Each live daemon watches `~/.config/ling/language/cLang` (`LINGModel.cpp:205-207`, `:229-231`).
  2. On a file event there, `getOrCreateLanguages` unpacks its own hash folder when that folder is absent. Then it deletes every other hash folder (`Processor.cpp:436-486`, deletion at `:467-482`).
  3. Only the timeout ends a daemon (`Server.h:76-81`, `Server.cpp:91-99`), so a daemon of an older build outlives its install.
  4. Fix: `Server` watches the folder of its own executable with the `Client` pattern (`Client.cpp:8-13`, `:50-60`). It calls `timerCallback` when the executable file changes (`Server.cpp:101-105`).
  5. The Engineer output was read and matches the prompt. Not runtime-verified.
- **Step 15c:** descoped by ARCHITECT (Key Decisions). Recorded in `PLAN-ling-contract-fix.md`, Step 15c.
- **jam pull check:**
  - `dev` equals `origin/dev` at `aacd6dc62`, with no conflicts and no unmerged paths (git status, `git diff --name-only --diff-filter=U`).
  - The pull deleted `jam_core/file/jam_Listener.cpp` and `.h`. It added `jam_FileWatcher.cpp` and `.h`.
  - The dirty diff is only the Step 12 rename `completionKinds` → `completionTypes` (`jam_LanguageInterface.h:103`, `jam_LanguageLibrary.h:35`, `:45`, `jam_LanguageLibrary.cpp:14`, `:89`, `:111-113`).
  - `isForeign` is in HEAD (`jam_LanguageInterface.h:79`). The library exports `LanguageEntry::completionTypes` (`language.cast:5661`).
- **Build of 2026-11-21 07:52:41 (ARCHITECT):**
  - `~/.local/bin/ling` sha256 is `a82c64eb…`, and it equals `Builds/Release/ling_artefacts/Release/ling`.
  - The embedded language hash is `19d05483…` (`strings`). The zips are `cLang.zip` `d04202f5…` and `core.zip` `c90a1127…`, rebuilt at 07:49-07:50.
- **T4 END before the pull** (warm, after 15a and the seed updates): 192 agree, 5 disagree, 77 stale.
- **Scratch:** the sysroot download and the probes are deleted (225 MB).

### Remaining
1. **COUNSELOR via Pathfinder:** read the machine state. ARCHITECT stopped the last Engineer run, so it is unknown whether pid 63624 was stopped, whether the config folder was deleted, and whether the harness started. Commands:
   - `ps -axo pid=,lstart=,command= | grep '[l]ing daemon'`
   - `ps -axo pid=,command= | grep '[n]vim --headless'`
   - `test -e /Users/jreng/.config/ling/language/cLang/construction.md && echo PRESENT || echo ABSENT`
   - `find /Users/jreng/.cache/ling/language -maxdepth 2`
2. **Engineer:** stop each leftover headless nvim from the stopped run.
3. **Engineer:** stop each ling daemon that started before 07:52:41. Pid 63624 (ling project, started 07:20:35, maps `cf4e…`) runs the build from before the fix. A config event makes it unpack `cf4e…` again and delete `19d0…`.
4. **Engineer:** `rm -rf /Users/jreng/.config/ling/language/cLang`, then run `zsh /private/tmp/claude-501/-Users-jreng-Documents-Poems-dev-ling/27ab850f-6578-4c2a-961e-c63fd0b6cfa1/scratchpad/run_test_long.sh compare end-definition`. After the run, use `lsof -p <END daemon pid> | grep liblanguage` to confirm that it maps `19d05483…`.
5. **COUNSELOR:** re-target the END test entries that name `jam_Listener.cpp` to `jam_FileWatcher.cpp`. Each new seed comes from the code read.
   - Counts: `test/end-positions.json` 1, `test/end-references-positions.json` 1, `test/end-symbols.json` 4 (`:45`, `:153`, `:171`, `:207`), `test/expected/end-definition.json` 101, `end-hover.json` 17, `end-methods.json` 131, `end-references.json` 22.
   - Pick a new 15b validation target: a member name that the per-platform `Impl` classes share in `jam_FileWatcher.cpp`.
6. **Step 15e validation:**
   - (a) Delete the config folder, start the daemon, and check that C++ units index and T4 completes.
   - (b) Check that a daemon of the new build quits when `~/.local/bin/ling` changes at the next install.
7. **Step 15f:** trace why `folder` 24:60 gave different answers in two runs with one library. The target moves with item 5.
8. **Step 15g:** do a runtime read of the comment cache order for `setTabName` (`jam_TabbedComponent.cpp:61:23`), then fix it at the hover owner. `getHoverComment` (`language.cast:4442-4454`) does not take effect, because of the redeclaration-chain cache.
9. **Rest of Step 15:**
   - The `bounds` definition (`juce_Component.cpp:3531:46` → `:3519`) and the `expectEquals` definition (`:4160:13` → `juce_UnitTest.h:215`).
   - The `getInstance` hover overrun, and the other hover differences against clangd.
   - The 77 stale positions.
   - The T12 seeds (`ENDView`, `TabView`, `Session`), and the T15 and T16 leftovers.
   - Item 22 `retain`.
   - The KANJUT runs (definition, methods, references, hover, inlay).
   - The re-index storm (setIndexed reached about 238k in about 4 minutes).
   - A test of the identity fix.
10. **Remove diagnostic instrumentation in this sprint:**
    - `Main.cpp:419` (`logScope`) and its member.
    - About 9 `debug::Log` lines in `Processor.cpp`.
    - `Connection.cpp:51`.
    - The Processor and Connection lines come from an earlier read. Read them again before removal.
11. **Step 16:** profile against clangd. Then the Auditor runs once. Doxygen stays on hold. The sprint log waits for ARCHITECT.
12. **COUNSELOR, first after item 1:** trace the load of the new build on the ling project.
    - ARCHITECT: *"ling now eats memory and cpu like crazy again, my fan is hot"*.
    - Evidence (ps, two reads 4 s apart): pid 76765 `/Users/jreng/.local/bin/ling daemon /Users/jreng/Documents/Poems/dev/ling/project-info.md` started at 07:54:28, after the 07:52:41 install. CPU was 371.9% then 251.3%. RSS was 2477024 KB then 3710704 KB. Its relay was pid 76711 `ling project-info.md`, started at 07:54:27.
    - COUNSELOR stopped pid 76765 under *"if it hang kill it, why wasting time"*.
    - The cause is not read yet. Reproduce it by starting that daemon with a sample (trace skill, `sample` driver, scratchpad `sample-index.sh` form), then read the hot frames.

### Key Decisions
- *"ling must deterministically answer test agrees with the code read"*
- `cast/cmake.cast`: *"Add to scope"*
- On ruling 35 against the code-read ruling: *"Real platform sysroots"*
- Windows header source: *"no mingw. juce doesnt support mingw anymore"*
- *"discuss first, bit smells here, why we need those source code only to fix our couple of mising features left"*
- *"File::Watcher being rewrite on parallel session on windows. i dont think those 4 significant enough to pull 200mb. i will pull/merge remote changes shortly"*
- *"jam pulled, remote at HEAD but i kept dirty on top check"*
- *"dont guess. use whatdbg, debug::Log whatever"*
- *"if it hang kill it, why wasting time"*
- *"you dont build. i will build"*
- *"dont abuse build for code validation"*
- *"always delete on disk, let the binary build and replace"*
- *"hold audit and doxygen. until i explicitly said so."*
- *"Keep clangd hover"* (`PLAN-ling-ast-once.md:15`)
- *"we need to pause, before i hit session limit."*

### Files Modified
- `Source/Server.h`: adds the base `private jam::File::Watcher::Listener` (`:17-20`), `fileChanged` (`:84`) and `watcher` (`:107`).
- `Source/Server.cpp`: the constructor watches the executable folder (`:3-9`). The destructor removes the listener (`:13`). `fileChanged` calls `timerCallback` (`:101-105`).
- `PLAN-ling-contract-fix.md`: Steps 15b-15g added. Step 15c descoped. Step 15d result. Step 15e cause and fix.
- `language/cLang/language.cast`: the outline gate on `keptRanges.isEmpty()`, the 15a-2 context-pass fences, `getHoverComment` (`:4442-4454`), and the 15b identity (`:2795-2826`, `addSymbol` overloads `:2850`, `:2855`).
- `language/cLang/spell.md`: the rows for the 15a-2 fences (`:846-881`), `isDirective` (`:594-596`) and `getHoverComment` (`:562-564`).
- `test/expected/end-definition.json`: `mask` → `[]`, and 16 keys from the code read (274 keys).
- `test/expected/end-methods.json`: `mask` → `[]`, and 19 keys from the code read.
- jam working tree (not committed): `jam_syntax_tree/document/jam_LanguageInterface.h`, `jam_LanguageLibrary.h`, `jam_LanguageLibrary.cpp`, with the Step 12 rename.

### Open Questions
None.

### Next Steps
- Pick up at Remaining item 1, when ARCHITECT starts the next session.

## Handoff to COUNSELOR: ling replaces clangd (Part 8, Step 1 in progress)

**From:** COUNSELOR
**Date:** 2026-11-19
**Status:** In Progress

### Read First
- `PLAN-ling-lsp-8-close.md` — Context item 1 (the two ARCHITECT rulings on the clangd oracle) and Step 1 (the T1-T26 run).
- `test/compare.lua:5-29` — the T4 verdict. The clangd word check was edited and not run.
- `language/cLang/language.cast` — the `kind` to `Type` rename (Completed below).

### Context
ling replaces clangd as the LSP. Part 8 Step 1 runs every test to 100%. The ARCHITECT rulings say the codebase is the ground of truth and clangd is a baseline only. The jam MCP Parts 1-5 run before Step 1 resumes (`PLAN-ling-lsp-8-close.md` Context item 6).

### Completed
- Remote check. ling `main` equals `origin/main`, with no commits behind or ahead (task a985e62134fb851fd, Pathfinder read).
- jam pull. ARCHITECT pulled jam `dev`. HEAD is `bb3baee7a`, equal to `origin/dev` (task aa7ab449760fe5bd7, Pathfinder read). ARCHITECT reports the conflict resolved to their side; the tree still holds the local changes.
- Overlap by file. 17 paths are both pulled in the last 3 commits and dirty locally (task a57e2d272b36723ca). The paths are listed in the Remaining section.
- `kind` to `Type`. `language/cLang/language.cast`: 87 substitutions. `\bkind\b` count is 0 (Grep, this session). `getCursorKind` and `CursorKind` untouched (task a4b91757b8c329017).
- `language/cLang/language.cast` holds ARCHITECT's uncommitted ling work of about 3677 insertions and 526 deletions against HEAD (task a8876919be44cfa7c). The rename sits on top of that work.

### Remaining
- Collision check, line level, on the 17 overlapping jam paths. Owner: Pathfinder. Task a43207d466fb1e12b was running at freeze and its result is not read. Re-run with the prompt in this session: `git diff -U0 HEAD~3 HEAD -- <paths>`, `git diff -U0 --cached -- <paths>`, `git diff -U0 -- <paths>`.
- The 17 overlapping jam paths: `cast/core.md`, `cast/spell.md`, `generated/jam_Core.h`, `jam_core/document/jam_Document.cpp`, `jam_core/document/jam_Document.h`, `jam_core/text/jam_Format.cpp`, `jam_core/text/jam_Format.h`, `jam_gui/widgets/jam_TextEditor.cpp`, `jam_terminal/document/jam_AnsiDocument.cpp`, `jam_terminal/document/jam_AnsiDocument.h`, `jam_terminal/document/jam_TerminalHistory.cpp`, `jam_terminal/jam_terminal.cpp`, `jam_terminal/jam_terminal.h`, `jam_terminal/processor/jam_TerminalProcessor.cpp`, `jam_terminal/processor/jam_TerminalProcessor.h`, `jam_terminal/tty/native/jam_TerminalTTY_windows.cpp`, `user-modules-info.md`.
- Stash. Not done. ARCHITECT instructed `stash`. Pathfinder and Engineer were denied by the hook (CAROL.md Git, hook enforcement). The stash list is unread.
- Commit message for the uncommitted jam and ling changes. Not written. The diff content has not been read.
- T4 harness. The clangd word check in `test/compare.lua` is not run. The ARCHITECT ruling to leave clangd answers is not implemented.
- Step 1 test run (T1-T26) on END and KANJUT. Not run.
- Speed miss T9 (ling median 0.258 ms, clangd 0.103 ms) from an earlier run in this session. Not re-measured.
- `abs` record triplication at `language/cLang/language.cast` 90:61 (222 locations). Cause not confirmed by read.
- `cstdlib:134` `using ::abs` record missing. Fix path in the overloaded-declaration API (task-read, this session). Not written.
- `construction.md:461` keeps `unsigned (int kind)`. COUNSELOR left it because it names a libclang signature parameter. ARCHITECT has not ruled on it.
- Uncompiled-lambda locals (`juce_Component.cpp` 3698, 3969) and `expectEquals` (4160). Not investigated.
- MCP Part 2b `jam::RegExp` plan (`/Users/jreng/.claude/plans/compressed-dreaming-hartmanis.md`). Not ratified by ARCHITECT.

### Key Decisions
- "clangd is reference baseline only. ling ultimate objectives is CODEBASE ground of truth. that is exactly why we develop ling. we have most accurate, most reliable language agnostic LSP" (ARCHITECT, 2026-11-19; recorded in `PLAN-ling-lsp-8-close.md` Context item 1).
- "when in doubt: discrepancy between ling vs clangd. trust THE CODEBASE READ as the absolute ground of truth. ling MUST NOT GUESSED. but GET the accurate read" (ARCHITECT, 2026-11-19; recorded in the same section).
- "if ling already able to get clangd answers (correctly, agree with the code), at this point we should leave clangd answers entirely. we only want clangd parity feature. correctness always must agree with the CODE READ." (ARCHITECT, after the compaction).
- "after the pull then you continue to complete your plan to completion, including fixing `kind`" (ARCHITECT, after the compaction).
- "dont abuse build permission to validate each turn code implementation. only build to measure and debug runtime parity features" (ARCHITECT, after the compaction).

### Files Modified
- `language/cLang/language.cast` — whole-word `kind` to `Type`, 87 substitutions, in ling-owned declarations and uses. The file also holds uncommitted work from before this session.
- `carol/SPRINT-LOG.md` — this entry.

### Open Questions
- `construction.md:461` (`unsigned (int kind)`) — decider: ARCHITECT. Renaming it would change the libclang signature label.
- Whether `Type` is the field name ARCHITECT wants for the four struct fields. Decider: ARCHITECT. NAMES.md Rule 6 is the basis.

### Next Steps
- Read the collision table from Pathfinder, then decide per file with ARCHITECT.
- Stash and pull state: run `git stash list` in the terminal, then send the output.
- Start Step 1 once ARCHITECT says go on the collision result.

## Sprint 13: LING Part 13, binding by libclang symbols, owned one-shot jobs, and the Index and Query split ✅

**Date:** 2026-11-15
**Duration:** Not measured. The work took several sessions, with compactions between them.
**Scope:**
- `PLAN-ling-13-binding.md` Steps 1-7, and its Amendment A (approved 2026-11-15).
- ARCHITECT's rulings this sprint, verbatim:
  - *"respect the architecture, dont fucking hack and just create workaround. hot path should always lock free, no lock, no wait. unless libclang being called at message thread, then it must be done in parallel, thread pool, ephemeral, one shot, clean up when done, all of it without manual thread management, nor manually handle the state machine, everything is fully event driven, by Model parameters."*
  - *"EVE already show exactly the pattern. hot path always lock free, FULL THROTTLE no wait. but at seam where we have no control like native TTY, we sync our rhythm so we nothing ever waits. if building AST for index swap genuienely need to be done in parallel then dont call it in hot path, do it in message thread, transport to hot path"*
  - Audit #11: *"Keep new names"*.
  - Audit #24: split Processor into Index and Query.
  - *"proceed until /audit wholesale when all objectives achieved. then /log no gate"*.
- Added by ARCHITECT this session: *"dont forget to write RFC for machinist at ~/.config/ to wire my nvim with ling to replace clangd."*

### Agents Participated
- COUNSELOR: Amendment A, every brief, the read of each return, the course corrections, the runtime diagnosis of two failures, the audit filter, the doxygen targets and their check, the RFC, and this log.
- Engineer (sonnet): A1 and A2, the A4 split, the build fixes, the two job corrections, the two audit fix batches, and the verification runs.
- Auditor (opus): one audit of the sprint.
- Pathfinder (haiku): the nvim and runtime reads, the hang reproductions, the git and process checks, and the doxygen pass.

### Files Modified
**JAM**
- `jam_syntax_tree/document/jam_LanguageInterface.h` — `LanguageCallbacks::addSymbol` takes the path, the 1-based line and column, the symbol, and the role. New field `shouldExit`. Doxygen for both, and for `LanguageIndex`.

**LING**
- `language/cLang/language.cast`, `construction.md`, `spell.md`:
  - Each `index()` call owns its libclang library, its CXIndex and its CXIndexAction, and frees them in reverse order.
  - The `thread_local` handles, `call_once`, the static caches, `applySdk` and `setEnvironmentVariable` are gone. The SDK goes on the command line through the `sdkFlags` row (`-isysroot`).
  - A load failure, including an empty install folder, returns `indexFailure`.
  - The unused `<mutex>` include and `SkipParsedBodiesInSession` are removed. `count` plus `at` became one `find`.
- `Source/Processor.h`, `.cpp`:
  - `ParseJob` and `IndexJob` are owned one-shot pool jobs, in EVE's `PreviewComponent` pattern. Processor derives `juce::AsyncUpdater`, and `handleAsyncUpdate` applies the results. `pendingJobs` is gone.
  - Index jobs start only from a document parsed again. They wait in `nextIndexJobs`, and `addIndexJobs` starts them when no parse job remains.
  - A new event calls `pool.removeJob (job, true, noWait)`. It replaces a job that has not started, or signals a running one.
  - `diagnostics`, `getUnit` and `hasDependency (unit, unitIndex, rowIndex)` stay in Processor.
- `Source/Index.h`, `.cpp` (new):
  - Index holds the state: paths, documents, languages, declarations, references, and the pending bindings.
  - It runs the record pass, in row order. The first record wins within the pass (`boundElements`).
  - It does the swap I/O.
- `Source/Query.h`, `.cpp` (new): the 12 request operations, the request parsing, and the resolution walks. Each reads `const Index&`. Its only member is `slab`. The lookup is by `Id::symbol` only.
- `Source/Manifest.h`, `.cpp`: `ManifestJob`, applied in `handleAsyncUpdate`, replaces the `callAsync` return of the pool job.
- `Source/CompilerManifest.h`, `.cpp`:
  - The getFiles path sets `failConstructionNotFound` and writes `Id::message` on the files tree.
  - `getOrCreateConstructions` is the one writer of the defaults. `applyDefaults` is private.
- `Source/LINGModel.*`, `Source/Main.cpp` (`map::Family` is an Application member), `cast/identifiers.md` (`role`, `checkFlags`), `cast/text.md` (`failConstructionNotFound`), `PLAN-ling-13-binding.md` (Amendment A).
- `/Users/jreng/.config/RFC-ling-replace-clangd.md` (new): the RFC for MACHINIST, with 8 open questions.

### Alignment Check
- [x] BLESSED principles followed.
  - Jobs are owned and one-shot.
  - The pool is declared after the job maps, so it stops every job before they are freed.
  - The message thread applies every result.
  - No lock, no counter, and no manual thread exists.
- [x] NAMES.md adhered. New names come from existing families: `ParseJob` and `IndexJob` from EVE's `DecodeJob`; `ManifestJob`; `setJob`, `removeJob`, `addIndexJobs`; `waitForJob` and `noWait`. `getOrCreateConstructions` follows Rule 3.
- [x] MANIFESTO.md principles applied.
- [ ] MANIFESTO §L: `Processor.cpp` is about 600 lines. The Auditor ladder reached Q5. Each split it tried makes Processor ask whether builds or precompiles remain, so it fails "Tell, don't ask".

### Problems Solved
- **Index job churn.**
  - Symptom: the first cold run started 110299 index jobs for 142 units, and did not end within 15 minutes.
  - Cause: each reparsed document restarted every dependent unit. The index jobs also shared the pool with the parse jobs.
  - Fix: queue each unit once, and start the queue only when no parse job remains.
- **A hang at 0% CPU.**
  - Cause: `juce_ThreadPool.cpp:290-295` (JUCE9) drops a signalled job that has not started, and never runs it. So its row never left the job map, and `setDocuments` never ran.
  - Trigger: concurrent edits in `jam_terminal` started the restarts (`run7.err:149-222`).
  - Fix: the locked `removeJob (job, true, noWait)` (`juce_ThreadPool.h:194`).
- **Audit.** 5 blocking findings and 30 others.
  - Resolved:
    - B1: a stale symbol stayed after a dependency change. Replaced per pass.
    - B2: a lost index was never retried. A failed index now deletes the unit's swaps.
    - B4: the query logic moved from Index to Query.
    - B5: the diagnostics precompile path now triggers an update.
    - N1, N3, N4 (`Manifest.cpp`), N6, N7, N8, N9, N10, N11, N12, N18, N19 (`getOrCreateConstructions`), N20, N21, N22, N23, N24, N28 (`shared_ptr` by reference).
  - Rejected with a read:
    - B3: MANIFESTO, "Add and remove operations are state updates".
    - N2: JAM has no public transparent hash, `jam_Strings.h:690-711`.
    - N4 at the subprocess sites: `jam_Subprocess.h:16`.
    - N5: ARCHITECT, "do it in message thread".
    - N13: a conversion at the store.
    - N14, N15: `jam_Model.h:1187`.
    - N16, N17.
    - N19 for `setFile` and `setDocuments` (named in A2.11).
    - N19 for `getWord` (it writes its own files tree) and `getCheckFlags` (it alone holds the parse error).
    - N20 for `runRequest`: `jam::Function::Map` has no single lookup.
    - N25: named in A1.4.
    - N26: `HELP.md:44`.
    - N27: A2.5 limits the stored role to the maps.
    - N28 for the `juce::String` fields: a JUCE File boundary.
    - N29, N30: rulings 60, 61, 66 and 76(e).
  - #33 from the first audit:
    - The `## compilerManifest` `@brief` is identical to `c_language.md:4` at HEAD, so it moved.
    - The `## checkFlags mac` `@brief` was new before the audit. It stays as part of the doxygen pass.
- **Verification.**
  - Two cold runs of END give `5606 documents` (83.12 s and 83.97 s), with byte-equal swaps (2923 files, 0 differences).
  - A warm run takes 20.47 s and starts no index job.
  - `findReferences` on `PluginState.h:41` and `Session.h:85`, and `goToDefinition` on `ENDView.cpp:360`, equal the baselines.
  - Earlier runs gave `5608`. The other workstream's `jam_terminal` edits changed the file set.

### State for Continuation
- The doxygen pass left pre-existing members undocumented: `Index::addOrReplaceProperty`, `Index::bindings`, `Processor::diagnostics`, `addPrecompile`, `addCheck`, `Manifest::getRoots`, and `CompilerManifest::getWord`, `applyDefaults`, `getConstruction`, `getCheckFlags`, `getBuildDirectory`.
- Regenerate the JAM doxygen XML for `jam_LanguageInterface.h` (`<leader>bd`).
- Stale language config copies are in the scratchpad (`config-cLang-stale*`). Old hash-named libraries stay in `~/.cache/ling/language/cLang`.
- The first A1+A2 Engineer kept running after its report until it was stopped at about 03:12. A read of `Processor.*` shows only the intended structure. The `jam_terminal` and `TerminalEncoder` edits (mtime 03:02) belong to the parallel session.
- Windows checks are ARCHITECT's. The open questions of `RFC-ling-replace-clangd.md` and `RFC-ling-nvim.md` are MACHINIST's and ARCHITECT's.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 12: LING Part 12, the language layer as data, the parity operations, and member access by object type ✅

**Date:** 2026-11-14
**Duration:** Not measured. The work took several sessions, with compactions between them.
**Scope:** `PLAN-ling-12-language.md`, Steps 1–12, from ARCHITECT's discussion of 2026-11-14 (the PLAN quotes it). After the audit, ARCHITECT added two items:
- MACHINIST's report on `findReferences` for `PluginState::setRows`. ARCHITECT: *"if machinist report is valid, fix it dont leave it broken"*, then *"DESIGN BY CONTRACT"*.
- *"proceed until /audit wholesale when all objectives achieved. then /log no gate"*.

### Agents Participated
- COUNSELOR: the PLAN, every delegation brief, the read of each return, the baseline and runtime scripts, the whatdbg sample specs, the audit filter, the doxygen targets and their check, and this log.
- Engineer (sonnet): Steps 3–10, the audit fixes, the object links (library, JAM callback, Processor), the template boundary, the brace initializer, and the `applyToDerived` order.
- Auditor (opus): one wholesale audit as two parallel sweeps, JAM and LING.
- Pathfinder (haiku): the discovery reads, every runtime and baseline run, three whatdbg samples, and the doxygen passes for LING and JAM.

### Files Modified (about 60 total)
**JAM**
- `jam_syntax_tree/document/jam_LanguageInterface.h` (new) — the C boundary of a language library: `LanguageToken`, `LanguageRole`, `LanguageCallbacks` (with `addObject` as its last field, so the layout of the earlier fields stays), `LanguageEntry`, `LanguageBuild`. It has no includes and no JAM or JUCE dependency.
- `jam_syntax_tree/document/jam_LanguageLibrary.h`, `.cpp` (new) — `getLanguageLibrary` opens the library and asserts every entry. Private `getClasses`, `getKeywords`, `getRoles`, `getCounterparts`.
- `jam_syntax_tree/document/jam_LanguageDocument.h`, `.cpp` (new) — `parse`, `isValid`, `read`, `hasRole`; the callbacks `addElement`, `addComment`, `addParameters`, `addMember`, `addLinkEndpoints`, `addObject`. `addObject` appends to `Id::object` through `addLinkEndpoint`.
- `jam_syntax_tree/document/jam_LanguageValidator.h`, `.cpp` (new) — the five rules of the old C validator, with the type range from the library roles.
- `jam_syntax_tree/document/jam_SyntaxTree.h`, `.cpp` — `parse` and `isValid` are removed. `getFamily` remains. `jam_CodeDocument.h` — `getFamily` is removed, and `getKeywords` is virtual.
- `jam_CLanguageDocument.*`, `jam_CLanguageValidator.*`, `jam_Language.*` — deleted.
- `jam_core/document/jam_Document.h:1046` — `getValid` asserts that the document is not nullptr (doxygen-pass finding F1, CODING "Preconditions use assert").
- `jam_markdown/document/jam_MarkdownDocument.h`, `.cpp` — `isEnabled (key, isAbsentEnabled)`; `getColumnWidths` reads the `column-widths` list; `getColumnFractions` measures, then shares the remainder with `jam::Value::map` and `clipMin`. `cast/markdown.md` gains `columnWidths` and `showMetadata`. JAM is regenerated.

**LING**
- `project-info.md:30` — version 0.0.1 (`versionNumber` `0x1`).
- `language/cLang/` (new): `spell.md`, `construction.md` (the tables, the roles, `extensionPairs`), `language.cast` (the procedures of the old C document as plain C++17).
  - `addReferences` returns the references it adds, and links a member access to the reference before its operator (`getObjectPosition`).
  - `addObjects` links a declaration to the references that name its type, or to the first initializer reference when it has none. `Element.objects` records the links, as `Element.isMember` records `addMember`.
  - `getBoundary` ignores a `(`, `[` or `=` inside a template argument list. A `<` opens the list only after an identifier, so `operator<` and comparisons stay as they were.
- `Source/Processor.h`, `.cpp`:
  - `jam::LanguageDocument` replaces `jam::CodeDocument`. Roles replace the C types.
  - New operations: `switchSourceHeader`, `inlayHint`. The library build is one per family. A hash-named copy of each build is opened, because `dlopen` returns the same handle for the same path (`man dlopen`).
  - `applyToDeclarations`: a member element goes through `getMemberPaths (pathIndex, member)` and `getClassPaths`. When no class is found, it falls back to `applyToMembers` by name (`PLAN-ling-5-links.md:24`).
  - `findReferences` adds each declaration to its result.
  - `applyToDerived` tests the base role before `isReferenceTo`.
- `Source/JsonWriter.*`, `LINGModel.cpp`, `Main.cpp` — the audit fixes. `cast/identifiers.md` gains `cast`, `inlayHint`, `switchSourceHeader`. `cast/c_language.md` — the mac check flags give parseable fix-its.
- `Source/HELP.md` — frontmatter, the new operation rows, and the `findReferences` row: "Each declaration of the element at the position, and the references to it."
- `/Users/jreng/.config/RFC-ling-nvim.md` (new) — the RFC for MACHINIST.
- `cast/signing.md` — the entitlement row `com.apple.security.cs.disable-library-validation`, the same row as `end/cast/signing.md:14`. cast writes it to `entitlements.plist`, and the Release sign step passes that file to codesign (`CMakeLists.txt:434`).

### Alignment Check
- [x] BLESSED principles followed. The parse links each object at creation, and Processor reads the links in place (MANIFESTO **E**, Complete at Creation).
- [x] NAMES.md adhered. `LanguageLibrary` is ARCHITECT's name. The object links reuse `Id::object` (`jam/cast/core.md:121`) and the `add*` callback family.
- [x] MANIFESTO.md principles applied.
- [ ] MANIFESTO §L: `addJob` is 45 lines. A split needs an out-parameter or a fake carrier, so it stays.

### Problems Solved
- **MACHINIST's report was valid.** `findReferences` on `PluginState::setRows` gave 4 calls on other classes and no declaration. The by-name rule of `PLAN-ling-5-links.md:24` caused it. The result is now `PluginState.h:41`, `PluginState.cpp:158`, `Session.cpp:40`, which is the true set. clangd misses `Session.cpp:40`. LING keeps it, because the object `pluginStates.at (uuid)` has no resolvable type, and the by-name fallback applies.
- **Three causes, each read before the fix:**
  1. No object link. A member access recorded no object (`language.cast` `addReferences`).
  2. A brace initializer. `getPreludeEnd` stops at `{`, so `auto& session { getSession() }` gave no initializer to `addDeclarators`.
  3. The template boundary. `getBoundary` took the `(` inside `Map<..., int (...)>` (`jam_ReflowDocument.cpp:192`). `getStatementEnd` then chained every later block, and every declaration after line 191 was lost (`documentSymbol` showed none past line 182).
- **The goToImplementation regression (before the audit).** A sample put 67% in `HashMap::at`. `LanguageDocument::hasRole` now reads the document's own library: 576–611 s.
- **goToImplementation after the parse fix.** The run passed the 780 s limit. A sample put 100% of the message thread in `applyToDerived` → `isReferenceTo`. The base role is now tested first. Result: 25 s, against 611 s. The result differs from the last run only by line shifts from the doxygen pass (`jam_MarkdownDocument.h` 554→555, `jam_CodeDocument.h` 82→93) and one recovered implementation (`jam_ReflowDocument.cpp:616`).
- **Audit.** Two parallel sweeps. The fixes and the rejections are in the session record. The doxygen pass reported F1–F3. F1 is fixed (`jam_Document.h:1046`). F2 is rejected: the sole caller gives a named column a length of zero (`jam_MarkdownDocument.cpp:1102-1114`, `jam_MarkdownBlockParser.cpp:1794`). F3 is rejected: `jam::Array::last` asserts at its owner (`jam_Array.h:308`).
- **The signed Release build loaded no language.** ARCHITECT's run gave `language library did not load` and `0 documents`. The kernel log: *"AppleMobileFileIntegrity) Library Validation failed: Rejecting '…/-3063693980930219348.dylib' … reason: mapping process and mapped file (non-platform) have different Team IDs"*. The Release binary runs with the hardened runtime and Team ID `9BDSN9TDX3` (`flags=0x10000(runtime)`). The library that cast builds on the machine is ad-hoc signed. Every earlier check used the Debug build, which is ad-hoc signed too, so the check never ran there. With the entitlement, the installed binary gives `5625 documents` on END, and `findReferences` on `PluginState::setRows` gives the 3 true references in 5.2 s.
- **Verification.** Before the object-link work, each output was equal to the baseline under the filter, and the warm run was byte-equal. `switchSourceHeader`, `inlayHint`, the fix-its, the hot reload with the same daemon PID, and Part 8 Step 7 items 2–8 passed. After the object-link work, the baseline outputs differ only by the doxygen line shifts, the added declarations in `findReferences`, and the recovered declarations.

### State for Continuation
- **findReferences cost.** `Session::setRows`: 7.40 s before the object links, 25 s after (warm, `scratchpad/sample3`). A sample puts 73% in `getClassPaths`, inside the scope walk (`getDeclarationPaths` → `getMemberPaths` → `applyToBases`). Every unqualified type name walks the bases of each enclosing class. MANIFESTO permits a second structure only with a measurement. The measurement is the sample above.
- **Objective-C headers.** The template boundary changes the counts of some AppKit headers by one declaration (`NSApplicationScripting.h` 3→2). The C grammar does not model `@interface` or `@property`. The old counts were misparses too.
- **Unresolved objects fall back to the name.** `this->x`, a call result (`a.b().c`), and an object whose type is a template parameter give every member with the name.
- **The config copy wins.** `Processor::saveToPath` copies the language files only when absent (`Processor.cpp:838-846`). A machine with an older `~/.config/ling/language/cLang/` keeps the old template and the old interface header, so it gets no object links. Remove that folder to take the new files.
- Regenerate the doxygen XML of JAM and LING. `jam/docs/xml/index.xml` was 0 bytes at the last read.
- Old hash-named libraries stay in `~/.cache/ling/language/cLang`. Port files stay after SIGTERM.
- Windows checks are ARCHITECT's. The open questions of `RFC-ling-nvim.md` are MACHINIST's.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 11: LING Part 9, completion and snippets from the stored document ✅

**Date:** 2026-11-14
**Duration:** Not measured. The work took two sessions (plan-9), with a compaction between them.
**Scope:** `PLAN-ling-9-completion.md`, from ARCHITECT's `/go @PLAN-ling-9-completion.md` and `/go DESIGN BY CONTRACT`. Inside the sprint, ARCHITECT also ruled on these items:
- The parse: *"why need to parsed again? why not only parsed once, complete AST build, memoized, cachaed. that have always been the contract"*.
- The unnamed parameter: *"Fix in Part 9"* and *"Its source text"*.
- The manifest load: *"Fix in this sprint"*.
- The test word: *"what the fuck is getVocab"*, then *"fix it"*.
- The leading attribute: *"getBoundary skips [[...]]"*.

### Agents Participated
- COUNSELOR: the plan amendments, the delegation, the read of each return, the Step 6 positions, the edit and the removal of the test line, the trace of the assertion, and this log.
- Engineer (sonnet): the rows, the JAM parse, the validator rules, the LING lookup and operation, the JSON keys, the manifest fix, the audit fixes, the temporary diagnostics, and the `getBoundary` fix.
- Auditor (opus): one sweep, with 22 findings. COUNSELOR accepted and fixed 17 of them: 1, 2, 4, 5, 6, 8, 10–14, 16–20, and 22. COUNSELOR rejected 5 of them:
  - 3: a second member would be shadow state.
  - 7 and 9: these allocations are part of a creation, which MANIFESTO permits.
  - 15: the default argument shows in the signature.
  - 21: this finding belongs to the doxygen pass.
- Pathfinder (haiku): the reads, the doxygen pass (0 warnings), the two later doxygen corrections, and the Step 6 runs.

### Files Modified
- `jam/cast/core.md` `## identifiers` — gained the rows `comment` and `parameters`. `cast/cast/identifiers.md` — the `comment` row is removed, so the identifier is defined once (MANIFESTO **S**). The JAM generated files are regenerated (`jam/generated/jam_Core.h`, `.cpp`).
- `jam/jam_syntax_tree/document/jam_CLanguageDocument.h`, `.cpp`:
  - `build` keeps the comment tokens beside the code tokens.
  - `addComment` attaches the comment tokens directly before each declaration as `Id::comment`.
  - `getStatementStart` walks back over `template <...>`.
  - `isNamedParameter` and `addParameter` give `Id::parameters`: the name token of a named part, or the span of an unnamed part.
  - `addParameters` builds the list.
  - `getDeclarationType` gives a constructor or a destructor only for a function shape.
  - `getBoundary` skips a leading `[[...]]` group (`.cpp:590-591`).
  - `addParameter` gives the whole part when the boundary is its first token (`.cpp:782-785`).
- `jam/jam_syntax_tree/document/jam_CLanguageValidator.h`, `.cpp` — two new rules: `comment` and `parameters`.
- `Source/Processor.h`, `.cpp`:
  - New members: `applyToScopes`, `applyToCandidates`, `getMatches`, `getCompletions`, `applyToCompletions`, and `completion`. `completion` is registered after `workspaceSymbol`.
  - The answer comes from the stored document, with no second parse (ARCHITECT's ruling above).
  - `Processor::getView` is removed. Each call site uses `document.getView`.
- `Source/JsonWriter.h`, `.cpp` — `appendComment` writes one source view. `appendSnippet` writes `name(${n:text}, …)`.
- `Source/Manifest.h`, `.cpp`, `Source/CompilerManifest.h`, `.cpp`, and `Source/Validator.cpp` — the manifest is read as `jam::MarkdownDocument` (`Manifest.cpp:28-29`). Since Sprint 10, `ConfigDocument::parse` had applied the rules of END's config format, so `ling project-info.md` printed `0 documents`.
- `Source/HELP.md` — gained the `completion` row. `cast/identifiers.md` gained `completion` and `snippet`. `Source/generated/` is regenerated.
- `PLAN-ling-9-completion.md`:
  - The stored document answers the request, and Step 5 sends no change message.
  - The callable carries no score, and the comment is one view.
  - `Id::parameters` holds Tokens, and the paragraphs "The unnamed parameter" and "The leading attribute" record the rulings.
  - Step 6 item 3 uses the word `getVocabulary`.
  - The Risks are updated.
- `PLAN-ling-0-tracker.md:27` — the Part 9 row.
- `.clangd`, `.project`, `compile_commands.json`, `docs/` — tool output.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered: `getVocab` became `getVocabulary` (Rule 4).
- [x] MANIFESTO.md principles applied: the AST is built once and read in place, with no second parse (**E**, Complete at Creation).

### Problems Solved
- **The manifest loaded no documents.** `ConfigValidator` rejected `0x100` and the `type` cells of a cast manifest. The fix reads the manifest as `jam::MarkdownDocument`.
- **Unnamed parameters had wrong or no placeholders.** `const String&` was parsed as a variable named `String`, and `int` gave no placeholder. The named rule now separates the two cases.
- **5119 assertions fired at `addParameter`, and 108 at `jam_Document.cpp:12` (a reversed span).**
  - Cause: a parameter part that starts with `[[` or `(` has its boundary at its first token.
  - Evidence: temporary `debug::Log` lines found the cause. The lines are removed.
  - Result: after the fix, the manifest run gives 0 assertions.
- **Step 6 on LING's manifest:** this manifest stands in for END's, because END's build compiles no `jam_syntax_tree` file (tracker:25).
  - Item 2 passes. The `addCommentToken` case gives its comment and the snippet `${1:cursor}, ${2:vocabulary}`. `getOffset` comes first after `.`. The parameter `cursor` is in the result.
  - Item 3 passes. After the watcher parse, `getVocabulary` and `getVocabularyForFamily` are in the result.
  - Item 4 passes: `cmp` gives 0.

### State for Continuation
- `cast/Source/generated/Identifiers.h:48` still declares `comment`. Run cast on cast's own spell file, then read that line again.
- Regenerate the doxygen XML of jam and ling. The header doxygen of `getBoundary` and `addParameter` changed after the last regeneration (`jam_CLanguageDocument.h:349-353`, `:660-663`).
- Step 6 item 5 (scout) is not run, because `ling` writes no samples file (see Sprint 8). Step 7 (Windows) is ARCHITECT's.
- A Pathfinder doxygen check ran `ninja -C Builds/Debug -n doxygen`, which printed "Re-running CMake". `Builds/Debug/build.ninja` may have been regenerated.
- The `Processor` class doxygen says that the lookups "keep nothing", but `slab` is mutable working memory. Nobody has checked this block against the code.
- `getBoundary` now skips `[[...]]` for every statement, so Part 4 statements such as `[[nodiscard]] int f()` become declarations. The parse has no rule for `__attribute__((...))`, which it reads as a function call shape.
- `CLAUDE.md` still says "No code exists yet."

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 10: JSON Through jam::JsonDocument; the Manifest Parse Validates Once ✅

**Date:** 2026-11-14
**Duration:** one session, from 2026-11-13 00:18 UTC. Not measured. `jam/carol/SPRINT-LOG.md` Sprint 187 is the primary record.
**Scope:** `jam/PLAN-json.md` (ARCHITECT: *"possibly update our tooling that read/write json, with this module as SSOT"*; scope *"All of them"*) and `jam/PLAN-document-contract.md` (ARCHITECT: *"Document::Validator is to validate ALL DOCS … only get and set"*).

### Agents Participated
- COUNSELOR: delegation, the read of each return, the doxygen check, this log.
- Engineer (sonnet): the ling call sites, the builds.
- Pathfinder (haiku): reads, the doxygen pass.

### Files Modified
- `Source/Client.cpp`, `Source/Connection.cpp`, `Source/Main.cpp`, `Source/Server.h` — JSON reads through `jam::JsonDocument`; writes through `jam::JsonWriter`.
- `Source/JsonWriter.h`, `.cpp` — the append helpers that `jam::JsonWriter` now gives are removed.
- `Source/CompilerManifest.h`, `.cpp` — `compile_commands.json` through `jam::JsonDocument::parse`; on nullptr the message carries `JsonDocument::isValid` (`.cpp:77-85`). The `JsonValidator` call is gone. `addCLanguageUnit` and `getCommandArguments` take the parsed document (`.h:78`, `:89`, the `@param document` lines).
- `Source/Manifest.cpp:28-29` — nullptr from `ConfigDocument::parse` gives `ConfigDocument::isValid`; the ling `Validator` (manifest schema) stays.
- `Source/Processor.h`, `.cpp` — the parse result of `SyntaxTree::parse` is used as a valid document; nullptr gives `SyntaxTree::isValid`.
- `Source/Help.cpp:19` — `setDocument (std::move (*jam::MarkdownDocument::parse (…)))`.
- `project-info.md`, `CMakeLists.txt` — the `jam_data_structures` description drops "JSON conversion". `.clangd`, `.project`, `compile_commands.json`, `docs/` — tool output.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- Two JSON paths (hand appends and `juce::JSON`) became one: `jam::JsonDocument`.
- Re-validation after the parse is gone; the Result API runs only on the failure path.
- Doxygen warned on the undocumented `document` parameter of two functions.

### State for Continuation
- `CLAUDE.md` says "design-stage project. No code exists yet." and "No sprint entries exist yet." The repo has code and sprint entries.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 9: LING Part 8, the shared daemon: lock-free, no wait, no pending request ✅

**Date:** 2026-11-13
**Duration:** not measured. The sprint ran over two sessions (one compaction).

### Agents Participated
- COUNSELOR — executed `PLAN-ling-8-daemon.md` Steps 1-6. Updated the PLAN on ARCHITECT's "update the plan" (designs C1-C6). Validated each Engineer return by its own reads. Filtered the 34 Auditor findings and the 4 findings of the doxygen pass. Made the trivial fixes: the empty argument (F4), one spacing fix, one doxygen `@param` fix.
- Pathfinder — discovery reads and the doxygen pass, with one follow-up.
- Engineer — Steps 1-6 and the accepted audit fixes.
- Auditor — one pass over the sprint. It reported 34 findings.

### Files Modified (about 40 total)
**jam:**
- `jam_subprocess/subprocess/jam_ChildProcess.h:44-59` — `start` takes `const jam::Strings&`. New `static bool startDetached (const jam::Strings&)`.
- `native/jam_ChildProcess_posix.cpp` — file-static `getArgumentBytes` and `getArgumentPointers`, shared by `Native` and `startDetached`. `startDetached` uses `posix_spawn` with `POSIX_SPAWN_SETSID` and `/dev/null`. An empty argument is no longer dropped (F4, MANIFESTO B Guard Rule; `jam_Array.h:494` already accepts a count of zero).
- `native/jam_ChildProcess_windows.cpp` — file-static `getCommandLine`, shared by `start` and `startDetached`. `startDetached` uses `CreateProcessW` with `DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP | CREATE_UNICODE_ENVIRONMENT | CREATE_BREAKAWAY_FROM_JOB`. `getQuotedArgument` quotes an empty argument as `""` (F4).
- `jam_subprocess/.../jam_Subprocess.h`, `.cpp` — `launch` and `Worker` take `jam::Strings`. `Chars::carriageReturn` at `.cpp:155`.
- `jam_core/text/jam_Format.h:412`, `.cpp:374-382` — `appendHex (std::string&, std::uint64_t)` with `std::to_chars`.

**scout, cast:**
- `scout/Source/Toolchain.cpp:21`, `scout/Source/Benchmark.cpp:110`, `cast/Source/Toolchain.cpp:21` — callers pass `jam::Strings { ... }`.

**ling:**
- `Source/Manifest.h`, `.cpp` (new) — the manifest read on a one-thread pool: parse, validate, `getTimeout`, `CompilerManifest::getFiles`, then `callAsync` to `setFiles`, which writes `Id::timeout` and replaces `FILES`. The `ninja` wait never runs on the message thread (C1, audit #31 split out of `Processor`).
- `Source/Processor.h`, `.cpp` — a `REQUEST` row runs at once. The pending-request scan and `valueTreeChildRemoved` are deleted. A reload erases only the documents whose path has no row in the new `FILES` row (C2).
- `Source/LINGModel.h`, `.cpp` — `FILES` and `CONNECTIONS` rows, `Id::timeout`, `defaultTimeoutMs`, `coalesceMs`, `getDaemonName` (Wyhash hex of path, newline, toolchain argument), `getCacheDirectory`. The `Id::manifest` reaction only adds the watcher folder. The watcher covers each compiler manifest and dependency record; `fileChanged` sends `Id::manifest` for them.
- `Source/Server.h`, `.cpp` (new) — `InterprocessConnectionServer` with no lock: `createConnectionObject` hands ownership to the message thread through `callAsync`. The port file and the timer start at the first `Id::documents`. Each `REQUEST` restarts the timer. The `message` attachment sends each overlay message to every connection as a JSON string (audit #4).
- `Source/Connection.h`, `.cpp` (new) — one connection row; a message `[operation, folder, args...]` becomes a `REQUEST` row with `Id::directory` (C4). The result goes back and the row is removed in the same event. `connectionLost` removes the object from its owner.
- `Source/Client.h`, `.cpp` (new) — no timer (C6). Connect, or `startDetached` the daemon; the port file event connects; `connectionMade` sends; a reply that starts with `"` sets the message, any other reply prints and quits; `connectionLost` gives `failDaemonLost`.
- `Source/CompilerManifest.h`, `.cpp` — `getDependencyArguments` returns `jam::Strings`. The compiler manifest and record paths are appended to `Id::compilerManifests` and `Id::dependencyRecords`.
- `Source/Validator.h`, `.cpp:27-47` — the `daemon` rule with `failDaemonTimeout`.
- `Source/Main.cpp` — `runDaemon` (`InterProcessLock` `enter (0)`, then processor, manifest, server), `runOperation` (the client). `model.addListener` only in the manifest run. The in-process request path is deleted.
- `Source/HELP.md` — the daemon row and the re-padded table.
- `cast/files.md` (`lingCache`, `lingLocalCache`, `lingPort`), `cast/text.md` (`failDaemonStart`, `failDaemonLost`, `failDaemonTimeout`), `cast/c_language.md` (`dependencyRecord`), `cast/identifiers.md` (`compilerManifests`, `connection`, `connections`, `daemon`, `dependencyRecords`, `timeout`).
- `PLAN-ling-8-daemon.md` — "The update of 2026-11-13" (C1-C6), the renumbered Steps, Step 7 items, the Names table, Risks, and the audit text for #6, #13, #14, #26. `PLAN-ling-0-tracker.md:26` — Part 8 status.

### Alignment Check
- [x] BLESSED principles followed. No lock and no wait in LING code on the message thread.
- [x] NAMES.md adhered. Every new name is in the PLAN-8 Names table with its family.
- [x] MANIFESTO.md principles applied

### Problems Solved
- **The locked PLAN broke lock-free, no wait, event driven.** ARCHITECT: "i thought the whole contract is lock-free at hot path, no wait, fully event driven?" and "why Processor wait?". The update removes `connectionsLock`, the pending request, the `ninja` wait on the message thread, and the client timer ("why wait? not event?").
- **Auditor findings.** Resolved: #1, #2/#10, #3, #4, #7, #11, #12, #21, #23, #26 (`Client::message` renamed `request`), #29, #31 (the `Manifest` split), #34. PLAN text: #6, #13, #14, #26. The others were rejected with reads. ARCHITECT received each one in chat.
- **Doxygen pass findings.** F4 accepted and fixed (above). F1 rejected: paths are appended only for an existing compiler manifest (`CompilerManifest.cpp:71-75`) or after `ninja` exits 0 (`:155-162`). F2 rejected: after `connectionLost` JUCE touches only the shared `SafeAction` (`juce_InterprocessConnection.cpp:54-60`, `:269-275`). F3 rejected: `ParameterText::setValue` runs on any thread (PLAN C1). Two blocks were corrected (`startDetached` "This object", `Manifest::addJob` "compiler commands"). `Main.cpp` keeps its blocks, because `Application` has no header.
- **Doxygen warning.** `Main.cpp:442` `@param tree` named an unnamed parameter. The line is deleted.

### State for Continuation
- ARCHITECT ran cast in `ling/`: `Source/generated/` holds the Step 2 rows (`Files.h:33`, `Text.h:41`, `Identifiers.h:40`). The LING doxygen run reported one warning, `Main.cpp` `@param tree`, now fixed.
- Step 7 items 2-11 (the runtime checks) and Step 8 (Windows) are ARCHITECT's. No result is reported in this session.
- The clangd index for JAM is stale: `startDetached` and `appendHex` are not found. Reopen JAM in nvim before an LSP check.
- PLAN-8 Risks hold the known facts: `connectToSocket` and `sendMessage` are synchronous in JUCE; a daemon message reaches every client; a client waits until Ctrl-C if the daemon dies before its port file; the C2 erase costs documents × rows per reload; the `Manifest` pool destructor waits for `ninja`.
- Step 2 markdown backups are in the scratchpad `bak/` folder only.

### Debts Paid
- None

### Debts Deferred
- Step 8, the Windows build and run (`PLAN-ling-8-daemon.md`, Step 8). It is ARCHITECT's (ruling 78). No `DEBT.md` entry exists.

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
