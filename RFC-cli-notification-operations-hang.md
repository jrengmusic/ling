# RFC — CLI Notification Operations Hang Forever
Date: 2026-11-22
Status: Ready for COUNSELOR handoff
Author: MACHINIST
Targets: `Source/Main.cpp`, `Source/Operation.cpp`, `Source/Processor.cpp`, `Source/Connection.cpp`, `Source/HELP.md`

## Problem Statement

Running `ling didClose <manifest> <file>` on the command line hangs forever. `ling didOpen` and `ling didChange` follow the same code path and hang for the same reason.

The CLI registers all words in `map::Readiness` as one-shot operations, including LSP notifications whose documented answer is `None`. `Operation` exits only after the daemon sends a JSON-RPC reply. Because `Processor::setBufferChange` removes the request node without setting `Id::result`, `Connection` sends no reply and `Operation` blocks forever.

## Research Summary

1. **CLI command registration** (`Source/Main.cpp:307-308`):
   - `Application::addCommands` iterates `map::Readiness {}` and binds every word to `runOperation (arguments)`.
   - `runOperation` (`Source/Main.cpp:452-476`) creates an `Operation` client and starts it.
2. **Notification dispatch** (`Source/Processor.cpp:139-141`):
   - `Id::didOpen` and `Id::didChange` call `Processor::setBuffer (request)`.
   - `Id::didClose` calls `Processor::removeBuffer (request)`.
3. **No reply set** (`Source/Processor.cpp:650-695`):
   - Both `setBuffer` and `removeBuffer` call `setBufferChange (request)`.
   - `setBufferChange` notifies the file row (`sendPropertyChangeMessage (Id::path)`) and removes the request from its parent (`request.getParent().removeChild (request, nullptr)`).
   - It never sets `Id::result` on `request`.
4. **Daemon send gate** (`Source/Connection.cpp:47-54`):
   - `Connection::valueTreePropertyChanged` calls `sendMessage (*tree[Id::result].getBinaryData())` only when `property == Id::result`.
   - With `Id::result` unset, the daemon sends nothing over the socket.
5. **CLI exit gate** (`Source/Operation.cpp:21-47`):
   - `Operation::messageReceived` calls `juce::JUCEApplicationBase::quit()` only when it receives a reply that has no `method` member (`else` branch, lines 37–46).
   - Without a reply, `Operation` never quits.
6. **Observed reproduction**:
   - Command: `cd ~/Documents/Poems/dev/ling && ling didClose project-info.md Source/Main.cpp`
   - Observed result: timed out after `3.01 s`, `stdout=None`, `stderr=None`.
7. **Documentation contrast** (`ling --help` Operation Words table):
   - `publishDiagnostics` explicitly states `The command does not end.`
   - `didOpen`, `didChange`, and `didClose` have no such note, and `ling --help` Command Line states that `ling <operation> ...` prints the JSON-RPC response on stdout and exits `0` (or `1` on error).

## Principles and Rationale

- **Deterministic (D):** A one-shot CLI operation must either terminate with a defined exit code and output, or be rejected as not a CLI operation. An undocumented infinite hang breaks deterministic scripting and agent tooling.
- **Explicit (E):** `didOpen`, `didChange`, and `didClose` are LSP notifications (`Answer: None` in `ling --help` Methods). Their CLI contract must be explicit in both code and `HELP.md`.
- **SSOT (S):** `map::Readiness` currently serves two roles: daemon readiness dispatch (requests + notifications) and CLI operation registration (`Main.cpp:307`). Splitting notification handling or returning a JSON-RPC `result: null` for `Operation` requests restores a single contract per surface.

## Scaffold

None.

## BLESSED Compliance Checklist
- [ ] Bounds  - [ ] Lean  - [ ] Explicit  - [ ] SSOT
- [ ] Stateless  - [ ] Encapsulation  - [ ] Deterministic

## Open Questions

1. **CLI behavior for `didOpen`, `didChange`, `didClose` — settled by ARCHITECT:**
   - **Option A — Acknowledge and exit `0`:** In `Processor::setBufferChange` (or when `request` has no LSP `id` from `Operation`), set `Id::result` to a JSON-RPC `result: null` reply for `Operation` connections so `ling didClose` prints `{"jsonrpc":"2.0","id":null,"result":null}` and exits `0` once the file re-parse is queued (or finished). Keeps `didClose` usable from scripts to drop a buffer or force a disk reload.
   - **Option B — Remove from CLI operations:** Exclude `didOpen`, `didChange`, and `didClose` from CLI command registration in `Main.cpp:307` and from the Operation Words table in `HELP.md` (they remain LSP stdio server notifications only).
   - **Option C — Keep as CLI and return after `sendMessage`:** Have `Operation` exit immediately after sending a notification word, with no JSON-RPC response printed (matches LSP `Answer: None`, but gives no confirmation that the daemon processed the message).

2. **Timing of `didClose` reload if Option A is chosen — settled by ARCHITECT / COUNSELOR:**
   - Should `ling didClose` reply immediately after `sendPropertyChangeMessage (Id::path)` queues the parse job, or wait until the re-parse finishes?

## Handoff Notes

- Discovered while wiring `ling` into JANITOR (`~/.carol/local/janitor/ling.lua`).
- ARCHITECT directed: *"write RFC to ling repo to fix the bug."*
- No source files in `~/Documents/Poems/dev/ling/` were modified.
