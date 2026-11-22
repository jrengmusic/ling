# PLAN: CLI Notification Operations Hang

**RFC:** `RFC-cli-notification-operations-hang.md`
**Date:** 2026-11-22
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8 / JAM (`LANGUAGE.md` C++ / JUCE)

## Overview

Fix the infinite hang on CLI buffer notification operations and remove foreign `did*` semantics per `NAMES.md`. Replace `Id::didOpen`/`Id::didChange` with `Id::setBuffer`, replace `Id::didClose` with `Id::removeBuffer`, unify `Relay::didOpen`/`Relay::didChange`/`Relay::didClose` into `Relay::setBuffer` and `Relay::removeBuffer`, and reply `{"jsonrpc":"2.0","id":null,"result":null}` from `Processor::setBufferChange` for CLI `Operation` connections while keeping LSP `Relay` connections silent.

**ARCHITECT rulings (verbatim):**
- *"Option A — Keep as CLI operations: reply with {"jsonrpc":"2.0","id":null,"result":null} and exit 0 for Operation connections only (Relay stays silent per LSP). Keeps `ling didClose` usable by scripts/JANITOR to drop buffers and force disk reload."*
- *"please also fix the names per @NAMES.md did* is foreign semantics"*

**RFC Open Question 2 (closed by CONTRACT):** `Processor::setBufferChange` replies right after `sendPropertyChangeMessage (Id::path)`. That call synchronously runs `setFile` -> `addJob` -> `setIndexed (rowIndex)` (`Source/Processor.cpp:691, 697-704, 980-993, 332-348`) and sets `Id::isIndexed` to `false` before the reply leaves the daemon. Any next `ling` query for that file waits in `Processor::isRequestReady` (`Source/Processor.cpp:210-246`) until re-parse finishes. Waiting inside `setBufferChange` would add machinery state and duplicate the readiness gate, violating MANIFESTO **S** (Stateless), **S** (SSOT), and **L** (Lean).

**External LSP names (`NAMES.md` Rule 2):** `lsp::Method::didOpen`, `lsp::Method::didChange`, and `lsp::Method::didClose` (`cast/lsp.md:24-26`) remain unchanged because they are external LSP 3.17 wire method constants (`"textDocument/didOpen"`, `"textDocument/didChange"`, `"textDocument/didClose"`).

## Language / Framework Constraints

- **One object, one translation unit (`CODING.md`, `LANGUAGE.md` C++ / JUCE):** Template functions stay in the header (`Query::setResult` in `Source/Query.h`). Non-template member bodies stay in the object's `.cpp` (`Processor::setBufferChange` in `Source/Processor.cpp`, `Relay::setBuffer`/`Relay::removeBuffer` in `Source/Relay.cpp`).
- **Codegen SSOT:** `Source/generated/Identifiers.h`, `Source/generated/Generated.cpp`, and `Source/generated/HashMaps.h` are generated from `cast/identifiers.md`, `cast/readiness.md`, and `cast/parameters.md` (`cast/spell.md:162-253`). Never hand-edit `Source/generated/*`; ARCHITECT's `cast cast/spell.md` build regenerates them.
- **Code hygiene (`CAROL.md` Code Hygiene):** No comments and no doxygen edits during implementation steps. Doxygen updates run in the dedicated post-audit pass via `@Pathfinder`.

## Framework Inventory

### 1. API

| Need | Module | API | Source | Step |
|---|---|---|---|---|
| Build JSON-RPC `result` frame and set `Id::result` on request row | `ling` | `Query::setResult (juce::ValueTree&, const Function&)` | `Source/Query.h:1107-1128` (`docs/xml/index.xml:758`) | Step 2, Step 3 |
| Write JSON `null` literal into reply buffer | `jam_core` | `jam::JsonWriter::appendNull` | `jam/jam_core/jam_core.h:122` (`jam/docs/xml/index.xml:9867`) | Step 3 |
| Detect LSP `Relay` connection vs CLI `Operation` connection | `juce_data_structures` / `ling` | `juce::ValueTree::hasProperty (Id::positionEncoding)` | `Source/Relay.cpp:275-281`, `Source/Connection.cpp:32-35` | Step 3 |
| Send reply on `Id::result` property change and remove request row | `ling` | `Connection::valueTreePropertyChanged` | `Source/Connection.cpp:47-54` (`docs/xml/index.xml:139`) | Step 3 |
| Trigger file re-parse and clear `Id::isIndexed` synchronously | `ling` | `Processor::setBufferChange` -> `sendPropertyChangeMessage (Id::path)` | `Source/Processor.cpp:683-695, 697-704, 980-993, 332-348` (`docs/xml/index.xml:691`) | Step 3 |
| Extract LSP buffer text from JSON params | `jam_core` | `jam::JsonDocument::contains`, `get`, `getChildByToken`, `getString` | `jam/jam_core/jam_core.h:120`, `Source/Relay.cpp:327-347` | Step 4 |

### 2. Pattern usage

| Pattern | Exemplar | Use sites | Step |
|---|---|---|---|
| Multiple LSP methods mapping to one `ling` operation | `textDocument/definition` and `textDocument/declaration` both map to `Id::goToDefinition` (`cast/lsp.md:202-232`, `Source/HELP.md:100-101`) | `Source/Relay.cpp:84-86` (`didOpen` + `didChange` -> `setBuffer`, `didClose` -> `removeBuffer`) | Step 1, Step 4, Step 5 |
| 1:1 naming between `Id::*` operation and `Processor`/`Query` method | `events.add (Id::goToDefinition, ... query.goToDefinition ...)` (`Source/Processor.cpp:116-137`) | `Source/Processor.cpp:139-141` (`Id::setBuffer` -> `setBuffer`, `Id::removeBuffer` -> `removeBuffer`) | Step 1, Step 3 |
| Public static JSON-RPC reply helpers on `Query` | `Query::setError` (`Source/Query.h:256`, `Source/Query.cpp:3-30`) and `Query::appendJsonRpc` (`Source/Query.h:243-248`) | `Source/Connection.cpp:39`, `Source/Processor.cpp:427`, `Source/Operation.cpp:57`, `Source/Relay.h:134` | Step 2 |
| Connection sends `Id::result` and removes child row automatically | `Connection::valueTreePropertyChanged` (`Source/Connection.cpp:47-54`) | Every `Query::*` operation in `Source/Query.cpp` via `Query::setResult` (`Source/Query.h:1127`) | Step 3 |
| `Relay` marks initialized LSP connection with `Id::positionEncoding` | `Relay::initialized` (`Source/Relay.cpp:275-281`) copied in `Connection::messageReceived` (`Source/Connection.cpp:32-35`) | `Source/Relay.cpp:279`; `Operation` (`Source/Main.cpp:452-476`) never sends `Id::connection` | Step 3 |

### 3. Names

| Family | Members | Intent | Step |
|---|---|---|---|
| Buffer mutation family (`NAMES.md` Rule 1, Rule 5) | `Processor::setBuffer` (`Source/Processor.h:710`), `Processor::removeBuffer` (`Source/Processor.h:717`), `Id::setBuffer`, `Id::removeBuffer`, `Relay::setBuffer`, `Relay::removeBuffer` | `set` writes buffer text onto `Id::buffers`; `remove` deletes buffer row from `Id::buffers` | Step 1, Step 3, Step 4, Step 5 |
| `Query` static response writers | `Query::appendJsonRpc` (`Source/Query.h:244`), `Query::setError` (`Source/Query.h:256`), `Query::setResult` (`Source/Query.h:1108`) | Writes JSON-RPC frame onto `request` | Step 2, Step 3 |
| External LSP wire methods (`NAMES.md` Rule 2) | `lsp::Method::didOpen`, `lsp::Method::didChange`, `lsp::Method::didClose` (`cast/lsp.md:24-26`) | External LSP 3.17 protocol strings; kept verbatim | Step 4 |

### 4. Synthesis

| Rule | Established form | Exemplar |
|---|---|---|
| "One object, one translation unit" (`CODING.md:1092`) | Template member `Query::setResult` stays in `Source/Query.h`; non-template bodies stay in `.cpp` | `Source/Query.h:243-248`, `Source/Processor.cpp:683-695`, `Source/Relay.cpp:323-359` |
| "No bail-out guards — ALWAYS use nested positive checks" (`CODING.md:1068-1069`) | Positive `if (request.getParent().hasProperty (Id::positionEncoding))` with `else` branch | `Source/Processor.cpp:656-666` |
| "Use the framework API fully" (`CODING.md:1085`) | Reuse `Query::setResult` + `jam::JsonWriter::appendNull` | `Source/Query.h:1107-1128`, `Source/Relay.cpp:234` |
| "DRY — Never duplicate code" (`CODING.md:7-8`) | Unify `Relay::didOpen` and `Relay::didChange` into one `Relay::setBuffer` | `Source/Relay.cpp:323-349` |
| "No comments pre-audit" (`CAROL.md` Code Hygiene) | Zero comment or doxygen changes in implementation steps | `CAROL.md` Code Hygiene |

### 5. Absence

| Need | Query | Result |
|---|---|---|
| Existing tests or benchmarks calling `didOpen`/`didChange`/`didClose` | `rg -n "didOpen\|didChange\|didClose" test/ benchmark/ Source/tools.json` | Empty (0 matches); `test/buffer.lua:81-97` tests LSP buffer sync through Neovim only |
| `Id::didOpen`/`Id::didChange`/`Id::didClose` references outside `cast/`, `Processor.cpp`, `Relay.cpp`, `HELP.md` | Doxygen XML index (`docs/xml/index.xml`, `_identifiers_8h.xml`, `_hash_maps_8h.xml`, `class_relay.xml`, `class_processor.xml`) | Empty outside those files (plus `Source/generated/*` codegen outputs) |

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against `MANIFESTO.md` (BLESSED), `NAMES.md`, `~/.carol/CODING.md`, and the locked PLAN decisions (no deviation, no scope drift). `@Auditor` runs ONCE, after the final step, covering the whole sprint — never per step.

## Steps

### Step 1: Replace `did*` identifiers in `cast/` tables with `setBuffer` and `removeBuffer`

**Scope:** `cast/identifiers.md`, `cast/readiness.md`, `cast/parameters.md`
**Action:**
1. In `cast/identifiers.md:40-42`, delete rows `didChange`, `didClose`, `didOpen`. Insert `removeBuffer` (alphabetically beside `rename`/`request`) and `setBuffer` (alphabetically beside `selectionRange`/`semanticTokens`) with `@id` type and matching backticked values. Keep table column alignment intact.
2. In `cast/readiness.md:24-26`, replace the three rows (`Id::didOpen`, `Id::didChange`, `Id::didClose`) with two rows: `Id::setBuffer` -> `Id::file` and `Id::removeBuffer` -> `Id::file`.
3. In `cast/parameters.md:32-34`, replace the three rows (`Id::didOpen`, `Id::didChange`, `Id::didClose`) with two rows: `Id::setBuffer` -> `Id::file` and `Id::removeBuffer` -> `Id::file`.
Do not edit `cast/lsp.md` (`lsp::Method::didOpen/didChange/didClose` are external LSP wire strings) and do not hand-edit `Source/generated/*`.
**Inventory:**
- Pattern: `textDocument/definition` + `textDocument/declaration` -> `Id::goToDefinition` (`cast/lsp.md:202-232`)
- Names: `Id::setBuffer`, `Id::removeBuffer` (joining `Processor::setBuffer` / `Processor::removeBuffer`)
**Validation:**
- Read `cast/identifiers.md`, `cast/readiness.md`, and `cast/parameters.md` at modified lines.
- Confirm zero `didOpen`/`didChange`/`didClose` rows remain, alphabetical order in `cast/identifiers.md`, and intact grid table borders.

### Step 2: Expose `Query::setResult` as `public static`

**Scope:** `Source/Query.h`
**Action:** Move `Query::setResult` (`Source/Query.h:1107-1128`) from `private:` to `public:` beside `Query::setError` (`Source/Query.h:256`) and change its signature to `static void setResult (juce::ValueTree& request, const Function& appendValue)` (removing `const`). Keep its body identical. Do not edit comments or doxygen blocks.
**Inventory:**
- API: `Query::setResult` (`Source/Query.h:1107-1128`)
- Pattern: `Query::appendJsonRpc` (`Source/Query.h:243-248`), `Query::setError` (`Source/Query.h:256`)
- Names: `Query` static response writers (`appendJsonRpc`, `setError`, `setResult`)
- Synthesis: One object, one translation unit (`CODING.md:1092`); template stays in header
**Validation:**
- Read `Source/Query.h` at both the old and new line ranges.
- Confirm `setResult` is `public` and `static`, body unchanged, zero duplicate declarations, and zero comment/doxygen edits.

### Step 3: Wire `Id::setBuffer`/`Id::removeBuffer` and reply `result: null` for CLI `Operation` in `Processor`

**Scope:** `Source/Processor.cpp`
**Action:**
1. In `Processor::registerEvents()` (`Source/Processor.cpp:139-141`), replace the three `events.add` lines (`Id::didOpen`, `Id::didChange`, `Id::didClose`) with:
```cpp
    events.add (Id::setBuffer, [this] (const juce::Identifier&, juce::ValueTree& request) { setBuffer (request); });
    events.add (Id::removeBuffer, [this] (const juce::Identifier&, juce::ValueTree& request) { removeBuffer (request); });
```
2. In `Processor::setBufferChange (juce::ValueTree& request)` (`Source/Processor.cpp:683-695`), replace `request.getParent().removeChild (request, nullptr);` with:
```cpp
    if (request.getParent().hasProperty (Id::positionEncoding))
        request.getParent().removeChild (request, nullptr);
    else
        Query::setResult (request, [] (juce::MemoryBlock& message) { jam::JsonWriter::appendNull (message); });
```
Leave `sendPropertyChangeMessage (Id::path)` before the branch so `Id::isIndexed` clears synchronously before the reply is sent. Do not touch `Source/Operation.cpp`, `Source/Connection.cpp`, or `Source/Main.cpp`.
**Inventory:**
- API: `Query::setResult`, `jam::JsonWriter::appendNull`, `juce::ValueTree::hasProperty (Id::positionEncoding)`, `Connection::valueTreePropertyChanged`
- Pattern: `Connection` sends `Id::result` and removes `request`; `Relay` holds `Id::positionEncoding`, `Operation` does not; 1:1 `Id::*` to `Processor::*` naming
- Names: `Id::setBuffer`, `Id::removeBuffer`, `Processor::setBufferChange`, `Query::setResult`
- Synthesis: Positive condition, `CODING.md` formatting, reuse `Query::setResult` (SSOT), zero new state
**Validation:**
- Read `Source/Processor.cpp:135-145` and `Source/Processor.cpp:680-700`.
- Confirm `Relay` removes `request` silently and `Operation` receives `result: null`.
- Confirm zero `did*` identifiers remain in `Source/Processor.cpp`, zero comments added, and zero edits outside `Source/Processor.cpp`.

### Step 4: Replace `Relay::didOpen`/`didChange`/`didClose` with `Relay::setBuffer` and `Relay::removeBuffer`

**Scope:** `Source/Relay.h`, `Source/Relay.cpp`
**Action:**
1. In `Source/Relay.h:182-204`, replace declarations of `didOpen`, `didChange`, and `didClose` (and remove their old doxygen blocks so no stale doxygen remains pre-audit; post-audit Pathfinder writes fresh doxygen) with:
```cpp
    void setBuffer (const jam::JsonDocument& document, const jam::Document::Element& root);
    void removeBuffer (const jam::JsonDocument& document, const jam::Document::Element& root);
```
2. In `Source/Relay.cpp:84-86`, bind `lsp::Method::didOpen` and `lsp::Method::didChange` to `setBuffer (document, root)`, and bind `lsp::Method::didClose` to `removeBuffer (document, root)`.
3. In `Source/Relay.cpp:323-359`, replace `Relay::didOpen`, `Relay::didChange`, and `Relay::didClose` with `Relay::setBuffer` (reading `params.contentChanges[0].text` when `document.contains (params, lsp::Member::contentChanges)` else `params.textDocument.text`, queueing `Id::setBuffer`) and `Relay::removeBuffer` (queueing `Id::removeBuffer`).
**Inventory:**
- API: `jam::JsonDocument::contains`, `get`, `getChildByToken`, `getString`; `lsp::Method::didOpen/didChange/didClose`
- Pattern: `Relay::getRequest` + `requests.appendChild` (`Source/Relay.cpp:300-359`)
- Names: `Relay::setBuffer`, `Relay::removeBuffer`, `Id::setBuffer`, `Id::removeBuffer`
- Synthesis: DRY (`CODING.md:7-8`), positive checks (`CODING.md:1069`), `.at (0)` (`CODING.md:1071`), brace init (`CODING.md:1066`), no comments pre-audit
**Validation:**
- Read `Source/Relay.h:175-210` and `Source/Relay.cpp:75-92, 320-355`.
- Confirm zero `Relay::did*` symbols or `Id::did*` uses remain, `lsp::Method::didOpen/didChange/didClose` remain intact for LSP wire dispatch, and `setBuffer` handles both `textDocument.text` and `contentChanges[0].text`.

### Step 5: Document `setBuffer` and `removeBuffer` in `Source/HELP.md`

**Scope:** `Source/HELP.md`
**Action:**
1. In `## Methods` (`Source/HELP.md:97-99`), update the `Operation` column:
   - `textDocument/didOpen`: `setBuffer`
   - `textDocument/didChange`: `setBuffer`
   - `textDocument/didClose`: `removeBuffer`
2. In `## Operation Words` (`Source/HELP.md:156-158`), replace the three `didOpen`/`didChange`/`didClose` rows with two rows:
   - `setBuffer` | `textDocument/didOpen`, `textDocument/didChange` | `file` | `The command sends no text. The buffer text is empty. Answers with result null.`
   - `removeBuffer` | `textDocument/didClose` | `file` | `Removes the buffer text. Answers with result null.`
Keep table column alignment consistent with neighboring rows (`Source/HELP.md:90-166`).
**Inventory:**
- Source: `Source/HELP.md:97-101, 138, 155-158`
- Pattern: `textDocument/definition` + `textDocument/declaration` -> `goToDefinition` (`Source/HELP.md:100-101`)
**Validation:**
- Read `Source/HELP.md:95-105` and `Source/HELP.md:150-165`.
- Confirm `Methods` keeps LSP `Answer: None`, `Operation` column uses `setBuffer`/`removeBuffer`, and `Operation Words` states `Answers with result null.`

### Step 6: Audit, Post-Audit Doxygen Pass, and Verification Gate

**Scope:** `cast/identifiers.md`, `cast/readiness.md`, `cast/parameters.md`, `Source/Query.h`, `Source/Processor.h`, `Source/Processor.cpp`, `Source/Relay.h`, `Source/Relay.cpp`, `Source/HELP.md`
**Action:**
1. Run `@Auditor` once across Steps 1–5 against `MANIFESTO.md`, `CODING.md`, `NAMES.md`, and this locked PLAN. Resolve every validated finding before proceeding.
2. Delegate the post-audit Doxygen pass to `@Pathfinder` for:
   - `Source/Query.h` (`Query::setResult`)
   - `Source/Processor.h` (`setBuffer`, `removeBuffer`, `setBufferChange` at lines 705-724)
   - `Source/Relay.h` (`setBuffer`, `removeBuffer`)
   Verify every doxygen block against the code.
3. Wait for ARCHITECT to build (`cast cast/spell.md`, which regenerates `Source/generated/*` and builds `ling`) and verify:
   - `ling removeBuffer project-info.md Source/Main.cpp` prints `{"jsonrpc":"2.0","id":null,"result":null}` and exits `0`.
   - `ling setBuffer project-info.md Source/Main.cpp` prints `{"jsonrpc":"2.0","id":null,"result":null}` and exits `0`.
   - LSP buffer sync (`test/buffer.lua`) passes with zero spurious responses on stdio.
**Inventory:** Full Framework Inventory above.
**Validation:** Zero unresolved Auditor findings, zero stale doxygen claims, zero `did*` internal identifiers remaining, verified runtime behavior after ARCHITECT's build.

## BLESSED Alignment

- **B — Bound:** `Connection` owns the `Id::request` row; `Connection::valueTreePropertyChanged` removes the row after sending `Id::result` (`Source/Connection.cpp:51-52`), and the `Relay` branch removes it directly.
- **L — Lean:** Removes duplicate `Id::didOpen`/`Id::didChange` operation and duplicate `Relay::didOpen`/`Relay::didChange` methods; 4-line branch in `Processor::setBufferChange`.
- **E — Explicit:** `setBuffer` and `removeBuffer` follow `NAMES.md` Rule 1 Verb Contract; CLI replies explicitly with `{"jsonrpc":"2.0","id":null,"result":null}` and exits `0`; `HELP.md` states both contracts.
- **S — Single Source of Truth:** `Query::setResult` is the single JSON-RPC result builder; `Id::setBuffer` and `Id::removeBuffer` match `Processor::setBuffer` and `Processor::removeBuffer` 1:1; `Id::positionEncoding` on `Id::connection` distinguishes `Relay` from `Operation`.
- **S — Stateless:** `Processor` adds zero state or tracking maps; `isRequestReady` remains the sole readiness gate.
- **E — Encapsulation:** `Operation`, `Connection`, and `Main` stay untouched; `Processor` delegates JSON-RPC serialization to `Query::setResult`.
- **D — Deterministic:** Every CLI buffer operation terminates deterministically with `{"jsonrpc":"2.0","id":null,"result":null}` and exit `0` after `Id::isIndexed` clears synchronously; LSP stdio `Relay` emits zero replies for notifications.

## Risks / Open Questions

None — all RFC open questions and naming directives are closed.
