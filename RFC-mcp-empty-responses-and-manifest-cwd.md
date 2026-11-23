# RFC — ling Empty Responses: Completed Fixes, Manifest Resolution, and Failed Unit Indexes

Date: 2026-11-24
Status: Ready for COUNSELOR handoff
Author: COUNSELOR (END session)
Targets: `D:/Poems/dev/ling/`, `D:/Poems/dev/jam/jam_syntax_tree/`
Supersedes: the 2026-11-22 text of this file. Its three points are all carried below, each with its current state.

## Problem Statement

ARCHITECT: *"we need ling to make our work here even more effective and efficient. dont assume, update the RFC, what actually correct, what actually completed, what actually still outstanding need to be fixed? i will open parallel session to work on ling"*.

ling on Windows gives empty results in the END session. The cause is no longer the MCP transport. The outstanding defects are:

1. One daemon run failed to index 10 of END's 16 units. The daemon keeps that failure until it exits. It does not try again.
2. A request on a failed unit answers with an empty result. The MCP client receives no error.
3. `workspaceSymbol` answers from an incomplete index with no sign that the index is incomplete.
4. The manifest is still resolved one time, at startup, from the working folder.

Answer to *"is it windows-msvc specifics?"*: the evidence does not show a Windows or MSVC cause for the failed units. The same binary, compiler, `compile_commands.json` and PCH failed in one daemon run and succeeded in the next (Research Summary 3). No macOS run was made, so this RFC makes no claim about macOS.

## Research Summary

### 1. Completed (verified in code on 2026-11-24)

| Item of the 2026-11-22 RFC | State | Evidence |
|---|---|---|
| Open Question 1: MCP `content` text block beside `structuredContent` | Completed, Option A | `jam/jam_model_context_protocol/server/jam_ModelContextServer.cpp:109-139` (`appendTextContent`, `sendResponse`); `carol/SPRINT-LOG.md:149` |
| Scaffold 2: explicit error for `Server::noPort` | Completed | `Source/ToolRelay.cpp:113-135, 160-171` (`sendNoDaemonError`); `carol/SPRINT-LOG.md:150` |
| Open Question 3: `CREATE_BREAKAWAY_FROM_JOB` retry | Completed, Option A | `jam/jam_subprocess/subprocess/native/jam_ChildProcess_windows.cpp:245-257`; `carol/SPRINT-LOG.md:151` |
| `goToImplementation` daemon crash | Logged as completed, not verified again here | `carol/SPRINT-LOG.md:152` |
| Windows `clang-cl` produced zero `.symbols` files | Completed | `carol/SPRINT-LOG.md:201`; on disk, the Debug cache `9ae3984493da1a92` and the no-sign cache `8c894f2a8fd13c0b` of END each hold 16 of 16 unit `.symbols` |
| Windows path separator mismatch | Logged as completed | `carol/SPRINT-LOG.md:202` |
| Handoff: rebuild and install ling | Installed: `ling 0.0.1 (a550b12)`, 19824320 bytes, 2026-11-22 23:30:43 | `ling --version`. ling HEAD is `41587de` (*Report explicit MCP noPort errors and fix goToImplementation root-parent crash*). The version string names the commit before it. The build date is after the Sprint 18 work (`carol/SPRINT-LOG.md:156`), so the string does not prove which Sprint 18 edits the binary holds |

### 2. Not completed: manifest resolution (2026-11-22 Open Question 2)

- `Source/Main.cpp:150-168` (`getManifestFile`) resolves one manifest at startup. When the file name is in no parent folder, it returns the working folder (`:164`).
- `Source/Main.cpp:170-178` (`getExistingManifestFile`) accepts the folder, because `exists()` is true for a folder.
- The daemon then writes `-1` (`Server::noPort`). Observed on disk with the port suffix of the installed binary (`c09abd051f09f83`): `%LOCALAPPDATA%\ling\b1fb6ba79995c76\`, 2026-11-23 11:42:22, content `-1`.
- The result is now an explicit tool error (item 1), but a session in another folder still has no index.
- The multi-workspace limit of the 2026-11-22 text stands: one `ling --mcp` process binds one manifest.

### 3. Outstanding: a failed unit index stays failed

Observed (END, Windows 11, clang-cl and libclang 20.1.8 from `VC\Tools\Llvm\x64\bin`, 16 logical processors, 12273524 KB physical memory):

- The default flow daemon (PID 23212, started 2026-11-23 21:44:10, cache `2449c4f9ccc7318a`) wrote 6 unit `.symbols` at 21:44:26 to 21:44:47: `cmake_pch.cxx`, `Main.cpp`, `ConfigDirectory.cpp`, `ConfigModel.cpp`, `ConfigShader.cpp`, `ConfigTheme.cpp`. It wrote none for `ENDLookAndFeel.cpp`, `Nexus.cpp`, `Session.cpp`, `EditorView.cpp`, `ENDView.cpp`, `SessionView.cpp`, `TabView.cpp`, `ENDModel.cpp`, `Files.cpp`, `Identifiers.cpp`.
- The parse `.swap` files of those 10 units exist (21:44:27.58 to 21:44:27.64).
- `D:\Poems\dev\jam\.cache\ling\2449c4f9ccc7318a` holds 1017 `.swap` and 0 `.symbols`.
- Approximately three hours later, the same daemon still answered `ling: documentSymbol: D:\Poems\dev\end\Source\ui\EditorView.cpp: unit index failed` in 329 ms. The `.symbols` count did not change.
- The `--no-sign` flow uses the same `Builds/Release/compile_commands.json` (2026-11-23 21:23:00) and the same PCH (21:23:32). Neither file changed. A new daemon for it, started 2026-11-24 00:38:19, gave `EditorView.cpp` (4816 bytes, 37.6 s), `Files.cpp` (2221 bytes) and `Nexus.cpp` (4447 bytes) with empty stderr, and wrote 16 of 16 `.symbols`.
- The `--debug` flow indexes all 16 units. EVE (`Source/EVEView.cpp`, 10563 bytes) and ling (`Source/Processor.cpp`, 14447 bytes) also answer in their default flow.
- `clang-cl -fsyntax-only` with the arguments that ling builds passes for `EditorView.cpp` and `ConfigTheme.cpp`, with no diagnostic.

Therefore the failure is not in the source, the compile arguments, the PCH, or the compiler. It belongs to that one daemon run.

Memory, measured after the fact:

- Before the no-sign run: 3209300 KB physical free, 30727127040 of 37733912576 bytes committed.
- After it: 1242112 KB physical free, 35836502016 of 37733912576 bytes committed.
- Three END daemons were alive together, one for each flow. Private bytes: 5099896832 (default), 5336629248 (debug), 5223575552 (no-sign).
- Memory at 21:44 was not recorded. The Application and System logs hold no event for 21:44:00 to 21:46:00, and no resource-exhaustion event (2004) on 2026-11-23. A memory cause is not proven.

Code path:

- `Source/Processor.cpp:871-877` (`IndexJob::addSymbols`): `result = language->index (...)`. The value is not logged and not stored. `builder.buildToSwap` runs only when it is 0.
- `Source/Processor.cpp:836-839`: an exception becomes `indexFailure`. The exception text is dropped.
- `language/cLang/language.cast:721-728`: `result` starts as `indexFailure` and stays so when the libclang thread does not run the lambda. `:728` returns the same value when the session does not load. Thus one value covers three different failures.
- `Source/Processor.cpp:1172-1176` (`addBindings`): the failure becomes the row property `message` with the fixed text `unit index failed` (`Generated/Text.h:43`).
- `Source/Processor.cpp:342-343` (`setIndexed`): a unit with `message` counts as indexed. Nothing schedules it again. `addIndexJob` (`:997`) is called only from `addDocument` (`:1114-1158`), after a parse job of the unit or one of its dependencies.
- `Source/SymbolDocument.cpp:434` and `Source/Index.cpp:81`: the result of `overwriteTargetFileWithTemporary()` is ignored. `buildToSwap` then opens whatever file is on disk (`:436`). It does not explain the 10 units, because they have no file. It is the same silent failure.
- `language.cast:5948-5954`: a libclang result of 0 with a null translation unit returns 0. `index` then succeeds with no records.

### 4. Outstanding: the failure does not reach the caller

- `Source/Processor.cpp:413-414` (`runRequest`) sends the unit's `message` with `model.setMessage`, then answers the request normally. The CLI prints it on stderr with exit code 0. The MCP client receives only the result.
- MCP results in the END session for failed units: `documentSymbol` gave `[]`, `hover` gave null, `diagnostics` gave `[]`. `diagnostics` `[]` on a failed unit reads as "no problems".
- `findReferences` on a symbol whose users are in failed units gave the declaration only. The result looks complete.

### 5. Outstanding: `workspaceSymbol` on an incomplete index

- `workspaceSymbol` waits for `files` (`cast/readiness.md:18`). `files` is indexed when no job is pending (`Source/Processor.cpp:372-376`). A failed unit has no pending job, so the request runs on the partial index.
- Default flow: `workspaceSymbol EditorView` gave the header only. `findAncestorRow` gave `[]`, although `--debug` and `--no-sign` `documentSymbol` both list it in `EditorView.cpp`.
- `--debug` flow: `workspaceSymbol HWNDComponentPeer` gave `[]` in 1875 ms. The JUCE and jam module units are in `compile_commands.json`. Whether the `juce_gui_basics` unit of the debug flow is indexed, failed, or filtered is not known.

## Principles and Rationale

- **E (Explicit):** *"Prefer assert over silent fail — invalid state is loud, never swallowed"*. An empty array for a failed unit is a silent fail. CAROL rules take an empty ling result as proof of absence, so a silent empty result gives a false fact to every agent.
- **S (SSOT):** the failure is already in the Model (row `message`). The answer must read it. No second flag is needed.
- **S (Stateless) and D (Deterministic):** the same request on the same files must give the same answer. A sticky failure from one run makes the answer depend on daemon history.
- **L (Lean):** all four defects are at existing seams: `IndexJob`, `setIndexed`, `runRequest`, `getManifestFile`.

Considered and rejected:

- A per-harness bridge (`ling-bridge.mjs`). `install.sh:22` already removed it, and a wrapper duplicates manifest resolution outside ling (**S** — SSOT).
- Clearing caches by hand, or a restart of the daemon by the user, as the remedy. It hides the failure and does not remove it.

## Scaffold

1. **Failure value.** `language->index` gives distinct results for: session not loaded, libclang thread not run, the libclang error code, and `saveTranslationUnit` failure. `IndexJob` keeps that value, and the row `message` names it.
2. **No sticky failure.** A failed unit is indexed again under a stated condition. The condition is Open Question 1.
3. **Loud answer.** A file-scope request on a unit with `message` answers with an error that carries that message: JSON-RPC error on stdio and socket, `isError: true` through `ToolRelay` (`sendToolError` exists), non-zero CLI exit. Today `Query::setError` is the existing error call (`Source/Processor.cpp:426`).
4. **Partial index.** A `files`-scope request, and `findReferences`, state which units are missing when any unit has `message`. The form is Open Question 2.
5. **Swap write result.** `SymbolDocument.cpp:434` and `Index.cpp:81` act on the result of the overwrite.
6. **Null translation unit.** `language.cast:5948-5954` returns a failure when libclang gives 0 and no translation unit.
7. **Manifest.** Resolution per requested file, or a loud startup failure (Open Question 3).

## BLESSED Compliance Checklist

- [x] Bounds — each daemon stays the one owner of its cache and port
- [x] Lean — changes stay in the existing seams
- [x] Explicit — a failed unit, a partial index and a missing manifest are all loud
- [x] SSOT — the row `message` is the one record of the failure
- [x] Stateless — no retry counters or flags outside the Model
- [x] Encapsulation — clients need no workaround
- [x] Deterministic — an answer does not depend on daemon history

## Open Questions

1. **When does a failed unit index run again?**
   - Option A: at the next request that needs the unit.
   - Option B: when the job queue is empty, one time, then at a change of the unit or its dependencies.
   - Decider: ARCHITECT / `ling` COUNSELOR.

2. **How does a partial index answer `workspaceSymbol` and `findReferences`?**
   - Option A: an error that names the failed units.
   - Option B: the partial result, with the failed units named beside it in the MCP `content` text.
   - Decider: ARCHITECT / `ling` COUNSELOR.

3. **Manifest resolution across workspaces** (2026-11-22 Open Question 2, still open).
   - Option A: `ToolRelay` resolves the manifest from the `file` parameter of each request and routes it to the daemon of that manifest.
   - Option B: a manifest path is required at startup, and startup fails loud when the resolved path is a folder.
   - Decider: ARCHITECT / `ling` COUNSELOR.

4. **What made the 10 units fail at 21:44?** It cannot be read after the fact, because no code is kept. Scaffold 1 comes first. The default flow daemon (PID 23212) still held the failure at 2026-11-24 00:42.

5. **Why does `workspaceSymbol HWNDComponentPeer` give `[]` in the debug flow** (Research Summary 5)?

## Handoff Notes

- Order: Scaffold 1 and 3 first. They make every later failure readable.
- To keep the failed state for study, do not restart the default flow daemon of END before its state is read. To make ling usable for END at once, a new daemon is enough: the no-sign run shows it indexes 16 of 16.
- Each of END's three flows starts its own daemon of approximately 5 GB private bytes. Two of them (default and no-sign) use the same build directory.
- The no-sign daemon (PID 8568) was started by the END session's evidence run.
- `ling --version` names the commit before the Sprint 18 commit. A build after `41587de` removes the doubt.
- ARCHITECT's directive from the END session: *"unless there is genuiene defects on ling's engine you must write RFC to ling's repo for counselor there to fix it."*
