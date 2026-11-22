# RFC — Empty MCP Responses in `agy` and Global Manifest CWD Resolution

Date: 2026-11-22
Status: Ready for COUNSELOR handoff
Author: COUNSELOR (END session)
Targets: `D:/Poems/dev/ling/`, `D:/Poems/dev/jam/jam_model_context_protocol/`

## Problem Statement

Every `carol_ling` MCP call from `agy` (Antigravity CLI 1.5.14) returns 0-byte empty output on Windows, even for files whose `.symbols` index already exists in `.cache/ling/` (for example, `D:/Poems/dev/end/Source/ui/EditorView.cpp`). Two defects combine to produce this failure:

1. `jam::ModelContextServer::sendResponse` emits `"content": []` and places the payload only in `"structuredContent"`. `agy` reads `content` and returns 0 bytes when `content` is empty.
2. `ling --mcp project-info.md` runs in a working folder that has no `project-info.md`. `getManifestFile` falls back to that folder, `ManifestJob` finds 0 files, and the daemon writes `-1` (`Server::noPort`) and exits. `ToolRelay` then answers every request with `null`.

In addition, the installed `C:\Users\jreng\.local\bin\ling.exe` is built from commit `acbeaf8`, which predates commit `a550b12` (*Fix Windows cLang clang-cl indexing, path normalization, and addUnits contract*).

## Research Summary

### 1. Observed `carol_ling` runs in `agy`
- `call_mcp_tool` on `carol_ling` `documentSymbol` (`D:/Poems/dev/end/Source/ui/EditorView.cpp` and `Source/ui/EditorView.cpp`) returned 0 bytes (not `[]`).
- `call_mcp_tool` on `carol_ling` `diagnostics` (`Source/ui/EditorView.cpp`) returned 0 bytes (not `[]`).
- `call_mcp_tool` on `carol_ling` `documentSymbol` (`D:/Poems/dev/eve/Source/EVEView.cpp`) returned 0 bytes.
- Prior calls in the same session for `workspaceSymbol` and `goToDefinition` on `jam_TextEditor.cpp` also returned 0 bytes (`transcript_full.jsonl:18-21,26-27`).
- `Query::applyToArrayResult` (`D:/Poems/dev/ling/Source/Query.h:643-655`) always wraps array results with `jam::JsonWriter::appendArray`, producing at least `[]`. It never produces 0 bytes.

### 2. Transport defect: empty `content` array in `jam::ModelContextServer`
- `D:/Poems/dev/jam/jam_model_context_protocol/server/jam_ModelContextServer.cpp:109-125`:
  `ModelContextServer::sendResponse` writes `"content": []` and writes the JSON payload only under `"structuredContent"`.
- `D:/Poems/dev/jam/PLAN-model-context-protocol-0-tracker.md:164` (`H3`) verified this schema against Claude Code 2.1.326 only.
- In `agy` 1.5.14, an empty `"content": []` produces 0-byte tool output even when `"structuredContent"` is populated.
- `ling --mcp`, `whatdbg --mcp`, and `scout --mcp` all share `jam::ModelContextServer` (`D:/Poems/dev/ling/Source/ToolRelay.h:131`).

### 3. Manifest and working-directory resolution defect
- `C:/Users/jreng/.gemini/config/plugins/carol/mcp_config.json:1-6` launches `ling` with `args: ["--mcp", "project-info.md"]` and no workspace-specific `cwd`.
- The live `ling` process (PID `10328`, `ling --mcp project-info.md`, started `2026-11-22 20:25:42`) wrote `-1` (`Server::noPort`) to `C:\Users\jreng\AppData\Local\ling\315aa18997443bbf\315aa18997443bbf7ccd73618a3b5074.port` at `20:25:43`.
- `D:/Poems/dev/ling/Source/Main.cpp:150-178`: when `project-info.md` is missing from `workingFolder` and its parents, `getManifestFile` returns `workingFolder`, and `getExistingManifestFile` accepts it because `manifestFile.exists()` is `true` for a directory.
- `D:/Poems/dev/ling/Source/Manifest.cpp:121-127`: when `manifestFile.isDirectory()`, `ManifestJob::runJob` constructs an empty `toolchain` tree.
- `D:/Poems/dev/ling/Source/Server.cpp:90-95`: when `files` has 0 children, the daemon writes `-1` (`noPort`) to the `.port` file and quits.
- `D:/Poems/dev/ling/Source/ToolRelay.cpp:124-135, 160-166`: when `Id::port == Server::noPort`, `ToolRelay` calls `server.sendResponse (id, "null")` for every request without surfacing an error.
- Meanwhile, END's own index already exists on disk from `Builds/Release/compile_commands.json` (`2026-11-22 19:18:49`): `D:\Poems\dev\end\.cache\ling\8c894f2a8fd13c0b\Source\ui\EditorView.cpp.symbols` (61,856,626 bytes, `19:26:40`). Neither `8c894f2a8fd13c0b` (`end`) nor `2101c14c12f7404f` (`eve`) has an active `.port` file.

### 4. Multi-workspace and Windows-specific observations
- A session with multiple active workspaces (`D:\Poems\dev\end` and `D:\Poems\dev\eve`) has only one global `ling --mcp project-info.md` process bound to one manifest at startup. Queries targeting files in another workspace fail to match `index.paths` (`D:/Poems/dev/ling/Source/Query.h:540-556`).
- `D:/Poems/dev/jam/jam_subprocess/subprocess/native/jam_ChildProcess_windows.cpp:243-246`: `ChildProcess::startDetached` passes `CREATE_BREAKAWAY_FROM_JOB`. Inside a restricted Windows Job Object (such as a sandboxed command shell), `ling --mcp D:/Poems/dev/ling/project-info.md` fails with `ling: --mcp: daemon did not start` (exit code 1).
- `C:\Users\jreng\.local\bin\ling.exe` reports `ling 0.0.1 (acbeaf8)` (`2026-11-22 14:27:08`), predating `D:/Poems/dev/ling` HEAD `a550b12` (`2026-11-22 18:11:08`).

## Principles and Rationale

- **E (Explicit):** A missing manifest or dead daemon (`port == -1`) must fail loud with an MCP error response, never silently return `null` inside an empty `content` array.
- **S (SSOT):** `jam::ModelContextServer` is the single MCP response serializer for `ling`, `whatdbg`, and `scout`. Fixing MCP compatibility in `jam::ModelContextServer` fixes all three servers in one place.
- **E (Encapsulation):** The MCP specification (2025-06-18 `CallToolResult`) recommends returning a serialized text content block alongside `structuredContent` for clients that read `content` only. Relying on Claude Code's private fallback (`H3`) couples `jam::ModelContextServer` to one client implementation.
- **D (Deterministic):** Resolving the manifest from the requested file's directory (or failing loud when no manifest exists) ensures identical behavior across single-workspace and multi-workspace sessions on macOS and Windows.

Considered and rejected:
- Adding a per-harness JavaScript bridge (`ling-bridge.mjs`) — rejected because `install.sh:22` already removed it and a wrapper duplicates manifest resolution outside `ling` (**S** — SSOT).

## Scaffold

### 1. `jam::ModelContextServer::sendResponse` (`D:/Poems/dev/jam/jam_model_context_protocol/server/jam_ModelContextServer.cpp:109-125`)
Populate `content` with a `text` content item carrying the serialized JSON of `structuredContent`, alongside `structuredContent`, so both `content`-reading clients (`agy`) and `structuredContent`-reading clients (Claude Code) receive the payload.

### 2. `ToolRelay` error signaling (`D:/Poems/dev/ling/Source/ToolRelay.cpp:124-135, 160-166`)
When `Id::port == Server::noPort`, emit an explicit MCP tool error (`isError: true` with the resolved manifest path and reason) instead of `sendEmptyResult` (`null`).

### 3. Manifest resolution for multi-workspace MCP sessions (`D:/Poems/dev/ling/Source/Main.cpp:150-178`, `ToolRelay.cpp`)
Support resolving a manifest per target file path (walking up from the queried `file` argument to `project-info.md` when no explicit manifest file exists at startup, or routing requests to the daemon for that manifest).

## BLESSED Compliance Checklist

- [x] Bounds — daemon lifetime and `.port` ownership stay bound to each manifest root
- [x] Lean — fix lives at the two existing seams (`ModelContextServer::sendResponse` and `ToolRelay` / `Main`)
- [x] Explicit — missing manifest and `noPort` report explicit errors instead of silent empty output
- [x] SSOT — one MCP response serializer in `jam::ModelContextServer` serves `ling`, `whatdbg`, and `scout`
- [x] Stateless — no per-client flags in `ModelContextServer`
- [x] Encapsulation — MCP clients receive standard `content` + `structuredContent` without harness-specific workarounds
- [x] Deterministic — same query returns identical symbols regardless of MCP client or spawn CWD

## Open Questions

1. **MCP `content` + `structuredContent` payload (`jam::ModelContextServer::sendResponse`):** Should `sendResponse` always emit a `{"type": "text", "text": "<json>"}` block in `content` alongside `structuredContent` (per MCP 2025-06-18 spec compatibility guidance)?
   - Option A: Emit both `content: [{"type": "text", "text": ...}]` and `structuredContent` unconditionally. (Recommended — **E** Encapsulation, **S** SSOT.)
   - Option B: Emit `content` only and drop `structuredContent`.
   - Decider: ARCHITECT / `ling` COUNSELOR.

2. **Manifest resolution when spawned with relative `project-info.md` or across multiple workspaces:**
   - Option A: Let `ToolRelay` resolve the manifest from the `file` parameter of each request (walking parents to `project-info.md`) and route to that manifest's daemon when the startup argument does not name an existing manifest file.
   - Option B: Require a manifest path at startup and fail loud on startup if `manifestFile.isDirectory()`, plus configure workspace-relative MCP launch in `agy`.
   - Decider: ARCHITECT / `ling` COUNSELOR.

3. **`CREATE_BREAKAWAY_FROM_JOB` fallback on Windows (`jam_ChildProcess_windows.cpp:243-246`):** Should `ChildProcess::startDetached` retry without `CREATE_BREAKAWAY_FROM_JOB` when `CreateProcessW` fails with `ERROR_ACCESS_DENIED` inside a restricted Job Object?
   - Option A: Retry without `CREATE_BREAKAWAY_FROM_JOB` on `ERROR_ACCESS_DENIED`.
   - Option B: Keep breakaway mandatory and document the Job Object constraint.
   - Decider: ARCHITECT / `ling` COUNSELOR.

## Handoff Notes

- Rebuild and install `D:/Poems/dev/ling` HEAD (`a550b12` or later) to `C:\Users\jreng\.local\bin\ling.exe` so the Windows `clang-cl` indexing fixes are active.
- `jam::ModelContextServer::sendResponse` changes in `jam` also restore `whatdbg --mcp` and `scout --mcp` tool output in `agy`.
- ARCHITECT's directive from the END session: *"unless there is genuiene defects on ling's engine you must write RFC to ling's repo for counselor there to fix it."*
