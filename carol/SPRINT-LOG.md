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

## Sprint 8: LING Part 7, the lookups across the documents and the nine operations ✅

**Date:** 2026-11-13
**Duration:** not measured. The sprint is one session, with one pause for an API rate limit.

### Agents Participated
- COUNSELOR — executed `PLAN-ling-7-queries.md` Steps 1-6. Validated each Engineer return by its own reads. Diagnosed the two Step 6 failures. Filtered the 29 Auditor findings and the 3 findings of the doxygen pass. Amended the PLAN for each contract correction, and recorded every new name in its Names table.
- Pathfinder — discovery reads, the Step 6 runs of `ling` (five rounds), and the doxygen pass.
- Engineer — Steps 1-5, two Step 2 and Step 5 corrections, the constructor fix, the audit fixes, and the cast builds.
- Auditor — one pass over the sprint. It reported 29 findings.

### Files Modified (13 total)
**ling:**
- `Source/Processor.h`, `.cpp` — the lookups: `getPath`, `getElement`, `applyToDeclarations` (element and path forms), `applyToReferences` (element and path forms), the scope walk (`getDeclarationPaths`, `getMemberPaths`) with the base step (`applyToBases`) and the member lookup (`applyToMembers`). The nine operations are entries of `events` (END's `Events` form), selected by `runRequest`. `applyToPosition`, `applyToFunctions`, `applyToDerived`, `applyToImplementations`, `setResult`, `getMatches` (`jam::Fuzzy`), `appendDocumentSymbols`. A `REQUEST` row runs when no parse is pending, or from `setDocuments`.
- `Source/JsonWriter.h`, `.cpp` (new) — `appendString` (`juce::JSON::escapeString`), `appendKey`, `appendSeparator`, `appendArray`, `appendElement` (the object of one element, with a members callback). It holds all JSON structure; `Processor` writes no bracket, brace, or comma.
- `Source/Main.cpp` — the operation run: `addRequest` adds the `REQUEST` row (`Id::command`, `Id::arguments`, `Id::directory`), then writes `Id::toolchainArgument` and `Id::manifest`. The `Id::result` reaction prints the JSON and quits. The `Id::documents` print runs only when the root holds no `REQUEST` row. `getFlags` (a `Function::Map`, the form of `jam_CodeDocument.cpp:27-43`) and `getFlagWord` replace `isVersion`, `isHelp`, `getVersionFlag`, `getHelpFlag`. `getDocumentFile` takes the argument index.
- `Source/HELP.md` — the operation rows, the `## Operations` table, the entry keys, and `type` as the numeric `map::SyntaxTokenType` value.
- `cast/identifiers.md` — `children`, `column`, `scope`, the nine operation names, `request`, `result`. `cast/text.md` — `failArguments` `wrong arguments`. `project-info.md` — the `jam_fuzzy` `## user module` row. `Source/generated/*` — cast output.
- `PLAN-ling-7-queries.md` — the name compare, the base step union, the constructor rule of the scope walk, the path-index signatures, the path overload in place of `getElements`, the unnamed declarations of `documentSymbol`, the request order and `Id::directory`, the `JsonWriter` structure, the Names table, the `jam_Array.h` path, Step 6 item 2 on LING's manifest.
- `PLAN-ling-0-tracker.md:25` — Part 7 status.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered. Every new name is in the PLAN-7 Names table with its family.
- [x] MANIFESTO.md principles applied
- [ ] MANIFESTO §L: `Processor.cpp` is about 560 lines and `Processor.h` about 870 with doxygen. The Auditor ladder (finding 6) found the DRY repeats (findings 1-4, 7), which are resolved. No split was proposed with the checks that the Auditor Lean ladder requires.

### Problems Solved
- **Step 6 item 2 returned `[]` on END's manifest.** END's build compiles no `jam_syntax_tree` file: `ling end/project-info.md --debug` lists 5600 documents and no `jam_CodeDocument` row. The checks now run on LING's manifest, which compiles `jam_syntax_tree`.
- **`goToImplementation` on `Document::build` returned `[]`.** The base `Document` of `struct MarkdownDocument : Document` resolved to the constructor `Document::Document` (`jam_Document.h:910`): the try `[jam, Document, Document]` found it. A try whose declarations are all constructors or destructors now gives nothing, so the walk reaches the class. The run then gives the `build` overrides of ten document types.
- **Contract corrections during the steps:** `isSameName` compares two tokens of one source (`jam_CLanguageDocument.h:54`) and names a language, so names compare as views. The base step takes the union of all bases (a first-found base depends on hash-map order). `applyToReferences` needs the declaration's document. `applyToDeclarations` (35 lines) and `workspaceSymbol` (57 lines) were split by responsibility (`applyToMembers`, `getMatches`). A `.substr` on `std::string` copied in `JsonWriter`.
- **Auditor findings.** Resolved: 1, 2, 3, 4, 5, 6 (Q1), 7, 8 (`Id::directory`), 9, 11, 13, 14, 16, 17, 21, 23, 24, 25, 26, 29, and the shadowed local `paths` that Engineer flagged. Rejected with a read: 8 (symbolic link: no read shows the row form), 10 (the path arrays hold views; "a view passed by value does not copy the data it names"), 12 (`Pattern::match` takes only `juce::String`, `jam_BonusMatrix.h:242`), 15 (each name was in a delegation prompt and has a family), 18 (one element per identifier, `jam_CLanguageDocument.cpp:756`), 19 and 20 (a reference's owner is the declared element or the scope, `jam_CLanguageDocument.h:420-432`), 22 (no clause), 27 (no CODING clause), 28 (`jam::Format::append*` takes `std::string&`, `jam_Format.h:390`).
- **Doxygen pass findings.** F1 and F2 (asserts on the name) rejected: the collectors establish the invariant, and a second assert is the MANIFESTO D violation that finding 16 removed. F3 (`callAsync` after destruction) rejected: messages are dispatched only inside `runDispatchLoop` (`juce_ApplicationBase.cpp:277`), which ends before the application is destroyed (`:268`, `:281`).
- **Verification (Step 6), on LING's manifest, after the audit fixes.** Item 2: `goToDefinition` gives `jam_CodeDocument.h:103` and `.cpp:72`; `findReferences` gives the call `.cpp:33:88` and not the definition; `documentSymbol` gives `jam`, `CodeDocument`, 22 members; `workspaceSymbol CodeDoc` holds `jam_CodeDocument.h:25`; `incomingCalls` gives `getVocabulary` and `getToken`; `goToImplementation` gives the `build` overrides. Item 3: both pairs byte-equal. `hover` adds `scope`. A wrong word or count prints `wrong arguments` and `[]`. Every output is valid JSON. Item 4: `findReferences` on `CodeDocument::parse` gives 9 member calls named `parse` (a member reference gives every member with that name, PLAN Risks); the LSP tool gives only the declaration, because its index does not resolve JAM `.cpp` files.

### State for Continuation
- Next sprint, ARCHITECT: "we discuss json document next sprint after wrapping up". The design under discussion: a JAM `JsonDocument` with a `std::string` writer, which replaces `jam::json` and `ling/Source/JsonWriter`. Open point: `Document::Writer::getText` returns `juce::String` (`jam_Document.h:891`), and `CssWriter`, `MarkdownWriter`, `MermaidWriter` derive from it.
- Step 6 item 5 is not run. A scout workload writes a samples file (`~/.carol/skills/profile/SKILL.md:317-324`), and `ling` writes none. The same item of Part 6 is open (Sprint 7).
- Timing on LING's manifest (4770 documents, Debug): one operation takes about 65 s, which is the parse. `findReferences` on `parse` takes about 115 s. `goToImplementation` on `Document::build` took 435 s alone. PLAN Risks names this cost; Part 8 removes the parse from each call.
- `ling` has no `doxygen` ninja target (`ninja -t targets all`). A doxygen run over the five changed files gave 0 warnings (`scratchpad/doxcheck/warn.log`).
- `Fuzzy::Pattern::match` takes only `juce::String` (`jam_BonusMatrix.h:242`), so `workspaceSymbol` makes one `juce::String` per declaration.

### Debts Paid
- None

### Debts Deferred
- Step 7, the Windows build and run (`PLAN-ling-7-queries.md`, Step 7). It is ARCHITECT's (ruling 78). No `DEBT.md` entry exists.

## Sprint 7: LING Part 6, the state model: one row per file, parsed in parallel, from the compiler manifest ✅

**Date:** 2026-11-13
**Duration:** not measured. The sprint ran over two sessions (one compaction).

### Agents Participated
- COUNSELOR — executed `PLAN-ling-6-manifest.md` Steps 1-7. Validated each Engineer return by its own reads. Ran the ODE diagnosis and the whatdbg attach sample. Filtered the 35 Auditor findings and the 3 points of the doxygen pass. Amended PLAN Parts 4-9 and 11 for the two contract corrections below, and recorded every new name in the PLAN-6 Names table.
- Pathfinder — discovery reads, the Step 7 runs of `ling` (four rounds), the ninja-against-ling file compare, the whatdbg `sample` run, and the doxygen pass.
- Engineer — Steps 1-6, the `JUCEApplicationBase` rewrite of `Main.cpp`, the `Id::syntax` change, and two rounds of audit fixes.
- Auditor — one pass over the sprint. It reported 35 findings.

### Files Modified (about 45 total)
**jam:**
- `cast/core.md:2118`, `:2128` — extension rows `cxx`, `mm`. `:140` — identifier `syntax`.
- `cast/syntax_tree.md:424-425` — `Extensions::cxx`, `Extensions::mm` give `cLang`.
- `user-modules-info.md:1151-1152` — the `jam_SyntaxTree` submodule rows. `generated/jam_Core.*`, `generated/jam_SyntaxTree.*`, `jam_syntax_tree/jam_syntax_tree.h`, `.cpp` — cast output only.
- `jam_syntax_tree/document/jam_CodeDocument.h` — `invalidFamily` and `getLanguageFamily` are public.
- `jam_syntax_tree/document/jam_SyntaxTree.h`, `.cpp` (new) — `struct SyntaxTree`: `getFamily`, `parse`, `isValid`, with the tables `getDocumentConstruction` and `getValidators` keyed by `map::Family`. LING names no language type (ruling 22).
- `jam_syntax_tree/document/jam_CLanguageDocument.h`, `.cpp` — `addElement` gives every element the id `Id::syntax` (`.cpp:731`). The local links compare names with `isSameName` (`.cpp:764-791`), which is public (`.h:54`).
- `jam_syntax_tree/document/jam_CLanguageValidator.h`, `.cpp` — the `link` rule compares names. The failure text gives the name read from the source.
- `jam_subprocess/subprocess/jam_ChildProcess.h:59`, `native/jam_ChildProcess_posix.cpp:211-254`, `native/jam_ChildProcess_windows.cpp:205-221` — `getArguments`: the clang GNU split on POSIX, `CommandLineToArgvW` on Windows. An empty line gives no argument, and a failed Windows split asserts.
- `jam_core/text/jam_Strings.h:578`, `.cpp:626-630` — `getView (int)`, taken forward from PLAN-8 Step 1 item 4 (ruling 52).

**ling:**
- `Source/Main.cpp` — `class Application : juce::JUCEApplicationBase`, started by `START_JUCE_APPLICATION` (`:281`, last line). `initialise` writes `Id::toolchainArgument` and `Id::manifest`, or sets the exit code and calls `quit()`. The `Id::documents` reaction prints the rows and calls `quit()`. The arguments come from `getCommandLineParameterArray()`.
- `Source/LINGModel.h`, `.cpp` (new) — the state tree, the `Id::message` parameter, the watcher. The `Id::manifest` reaction parses, validates, and replaces the `FILES` row whole.
- `Source/Validator.h`, `.cpp` (new) — the `toolchain` column rule.
- `Source/CompilerManifest.h`, `.cpp` (new) — the units from `compile_commands.json`, and the files of each unit from `ninja -t deps`. A path gets one row.
- `Source/Processor.h`, `.cpp` (new) — one parse job per row on a `juce::ThreadPool`. No `juce::ValueTree` reaches the pool thread. A result is kept only when the row at its index still has its path and the file still has the read's modification time.
- `Source/HELP.md`; `cast/c_language.md` (new), `cast/spell.md`, `cast/text.md`, `cast/identifiers.md`, `project-info.md` (`jam_subprocess`); `Source/generated/*` — cast output.
- `PLAN-ling-6-manifest.md` — Step 6 item 1 (the application), Step 3 (`isValid`), the documents paragraph (index, path, time), the Step 6 Validation (`applyToDocuments`), the Names table. `PLAN-ling-4`, `-5`, `-7`, `-9`, `-11` — the element id is `Id::syntax` and every id read becomes a name read. `PLAN-ling-8` Step 1 item 4 is marked done. `PLAN-ling-0-tracker.md:22-24` — status of Parts 4, 5, 6.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered. Every new name is in the PLAN-6 Names table with its family. `Id::syntax` is ARCHITECT's: "Id::syntax? Id::keyword? @NAMES.md", then "/go".
- [x] MANIFESTO.md principles applied
- [ ] MANIFESTO §L: `jam_CLanguageDocument.cpp` is about 875 lines. The split that the Auditor offered only relocates one responsibility (MANIFESTO §L, file size), so it is rejected.

### Problems Solved
- **The message loop never ran on macOS.** The ODE log showed `runDispatchLoop` return at once with no stop message. `runDispatchLoop` is `[NSApp run]` (`juce_MessageManager_mac.mm:323-351`), and only `JUCEApplicationBase::main (argc, argv)` makes the `NSApplication` (`juce_ApplicationBase.cpp:242-244`). ARCHITECT: "juce CAN be used to create CLI and TUI ... what's the established pattern? what's the isomorphic abstraction?" The application is now a `juce::JUCEApplicationBase` (`juce_events`), as tit uses `START_JUCE_APPLICATION`. My first options (an `.mm` file, a forward declaration, `JUCEApplication`) were wrong; ARCHITECT rejected them.
- **The parse jobs ran one at a time.** A whatdbg attach sample showed ten pool threads in `__psynch_mutexwait` under `juce::Identifier::Identifier` → `StringPool::getPooledString`. `addElement` made one pooled id per element, and the pool has one process-wide lock. The name already lives in `Id::name`, so the id was a second copy (MANIFESTO S). The id is now the tag `Id::syntax`. END `--debug`: 71 s real, 227 s user, against a run that did not end in 120 s.
- **Two contract findings on the jobs.** F4: a `juce::ValueTree` was copied and destroyed on the pool thread (MANIFESTO B). F6: an older parse could replace a newer one (MANIFESTO D). The job now carries the row index, the path, and the modification time. ARCHITECT: "why always leave ANYTHING HALF-ASSED?"
- **Auditor findings.** Resolved: F2, F4, F6, F8, F9, F10, F12 (PLAN text), F13, F16, F17, F21, F25, F27, F29, F33, F35. Rejected with a read or a PLAN line: F1, F3, F5, F7, F11, F14, F15, F18, F19, F20, F22, F23, F24, F26, F28, F30, F31, F32, F34. ARCHITECT received each one in chat.
- **The doxygen pass.** The stale id blocks are corrected. COUNSELOR fixed the `@return` tag and added the why to the `addElement` block. A bad `@returns` replacement gave 6 warnings (`@returnThe`). Fixed, and ARCHITECT's next build ran with no further report.
- **Verification (Step 7).** `ling end/project-info.md --debug`: 5600 documents. Two runs are byte-equal. The rows for `end/Source/Main.cpp`, `jam_core/document/jam_Document.cpp`, `juce_core.h` and `vector` exist, and no path has two rows. Against `ninja -C Builds/Debug -t deps`, normalised: ling has no path that ninja lacks. The 35 paths that only ninja has are 33 shader files of the `spv` units, which are not in `compile_commands.json`, and 2 files that no longer exist on disk. `--nothing`, the column fixture, and `ling end/Source/Main.cpp` print `0 documents` with the expected diagnostic. `ling end/project-info.md --debug extra` prints `ling: extra: unknown flag`.

### State for Continuation
- Step 7 item 6 is not run. whatdbg launch takes `program` and `cwd` only, with no arguments (`~/.carol/skills/trace/SKILL.md:93-106`). The attach form works (this sprint's sample ran with a Monitor-started process), but a breakpoint at the return of `getArguments` stops 142 times and the attach must come before `CompilerManifest` runs. ARCHITECT decides the form.
- Step 7 item 7 is not run. A scout epoch needs a samples file that the command writes (`~/.carol/skills/profile/SKILL.md:80-82`, `:94`), and ling writes none. ARCHITECT decides the form.
- Step 8 (Windows) is ARCHITECT's (ruling 78). The Auditor raised a possible `\r\n` from ninja on Windows (F11), not observed (`CompilerManifest.cpp`, the record read).
- A run on the 142 END units with no `ninja` row (`scratchpad/probe/units`) took 31 s before the `Id::syntax` change. It was not timed again.
- Each run of the one-file probe now takes 0.20 s.
- `Source/Main.cpp.bak` was a scratchpad backup only. No file in the project holds it.

### Debts Paid
- None

### Debts Deferred
- Step 8, the Windows build and run (`PLAN-ling-6-manifest.md`, Step 8). It is ARCHITECT's (ruling 78). No `DEBT.md` entry exists.

## Sprint 6: LING Part 5, references and local links as metadata of the file tree ✅

**Date:** 2026-11-12
**Duration:** not measured. The sprint is one session.

### Agents Participated
- COUNSELOR — executed `PLAN-ling-5-links.md` Steps 1-6. Wrote the exact code of each brief. Validated each return by its own reads. Filtered the 24 Auditor findings and the 3 points of the doxygen pass. Made one doxygen line fix and recorded the executed names and the `link` correction in the PLAN.
- Pathfinder — discovery reads, the doxygen pass, the Step 6 runs of `ling`, and the state of the repos.
- Engineer — Step 1 and the cast runs with snapshot diffs. Steps 2-3. Steps 4-5. The six accepted audit fixes.
- Auditor — one pass over Steps 1-5. It reported 24 findings.

### Files Modified (12 total)
**jam:**
- `cast/core.md:95`, `:106`, `:111`, `:142` — rows `in`, `link`, `member`, `to`. `in`, `to`, and `link` moved down from `cast/mermaid.md` (PLAN inventory: a row that both modules read moves to `jam_core`).
- `cast/mermaid.md` `## identifiers` — rows `in`, `link`, `to` removed. `out` stays (`:211`).
- `cast/syntax_tree.md:63-65` — `SyntaxTokenType` rows `reference` 20, `call` 21, `base` 22. The brief at `:28-40` names the reference types.
- `generated/jam_Core.h`, `.cpp`, `jam_Mermaid.h`, `.cpp`, `jam_SyntaxTree.h` — cast output only. The snapshot diffs showed no other change.
- `jam_syntax_tree/document/jam_CLanguageDocument.h`, `.cpp` — `addReferences` makes one reference element for each identifier that is not a declared name. Each handler gives the ranges that it does not walk as statements to `addReferences`. `getReferenceOwner`: inside a function the function owns the references, otherwise the declared element or the scope. `addDeclarationContents`, `getBaseStart`, `getCandidateEnd` hold the reference ranges and keep each function near the limit. `addLocalLinks` runs at the end of `build()` and writes `Id::to` and `Id::in` through `addLinkEndpoint` and `addLinkEndpoints`. `isMemberAccess`, `isLocalReference`, `getLocalDeclarations`. Doxygen pass on every new and changed block.
- `jam_syntax_tree/document/jam_CLanguageValidator.h`, `.cpp` — the `Id::type` rule accepts keys up to `base`. A new `Id::link` rule. `getElementFailures` holds the walk that the three rules share (Auditor 11, MANIFESTO S).

**ling:**
- `Source/Main.cpp:88-148` — `appendPosition`, `appendName`, `appendLinks`, `appendDeclaration`. A row prints the `Id::name` tokens joined with `::`, ` member` for a member access, and one `    to <line>:<column>` row for each target.
- `PLAN-ling-5-links.md` — the executed names in the Names table, and the `link` execution correction in Step 1.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered. Every new name is in a family (NAMES Rule 5). The PLAN Names table records each one.
- [x] MANIFESTO.md principles applied
- [ ] MANIFESTO L: `addClass` (`jam_CLanguageDocument.cpp:297-328`) is 32 lines. The Auditor's ladder found no fix for it (finding 22). `jam_CLanguageDocument.cpp` is about 880 lines. The type has one responsibility (PLAN-ling-4 Language Constraints).

### Problems Solved
- **`Id::link` lived only in `jam_mermaid_diagram`.** The Step 4 rule key is `link`, and `jam_syntax_tree` does not depend on that module. The row moved to `jam_core` with `in` and `to`.
- **The local link walks top-down.** A function-type element never has a function-type parent (`getDeclarationType`, `jam_CLanguageDocument.cpp:626-667`). Thus the PLAN walk up the parents is one level, and a top-down walk over the mutable children gives the same links. The Auditor confirmed this.
- **My `scopeTypes` capacity edit was wrong.** `LookupTable::operator[]` gives the default row for a key out of range (`jam_LookupTable.h:157-160`). I reverted the edit.
- **Auditor findings.** 5, 9, 11, 14, 15 and 18 are resolved. 1, 2, 3, 4, 6, 7, 8, 10, 12, 13, 16, 17, 19, 20, 21, 22, 23 and 24 are rejected with a read or a PLAN line. ARCHITECT received each one in chat.
- **The doxygen pass.** One line of the `addLocalLinks` block was false ("every element of the subtree of scope"). It is corrected. The pass reported 3 contract points: the `bool` inputs, the two 32-line functions, and the directive inside a declaration. All 3 are rejected.
- **Verification.** ARCHITECT built LING, END, cast, and scout on macOS clean, and the Windows build is clean up to Part 4. The doxygen output is clean. Step 6: `ling 0.1.0 (a9b659c)`. The four `jam_CodeDocument.cpp` cases hold. Every use of `cursor` in `addStringToken` (`:102-118`) has `to 100:55`. The two `SessionView.cpp` outputs are equal (`cmp` 0, 321 lines each). The validator reports no failure.

### State for Continuation
- Part 6 is next (`PLAN-ling-6-manifest.md`). `PLAN-ling-0-tracker.md:22-23` still says "Written" for Parts 4 and 5.
- Known limits that the Step 6 output shows. A statement that starts with `p->x`, `*p`, or `a > b` gives a fake local through the Part 4 declaration test (`jam_CLanguageDocument.cpp:599-619`), and later references can link to it (Auditor 1). A trailing return type after `->` gets `Id::member` (Auditor 2). A lambda parameter can link to an outer local (Auditor 20). A change to any of these is a change to the PLAN.
- Declaration rows now print the qualified name, for example `CodeDocument::getToken` (`Main.cpp:97-110`).
- The JUCE banner `JUCE v8.0.17` still goes to stderr on each run.
- clangd does not resolve the framework `.cpp` files. `findReferences` gives only the header declaration.

### Debts Paid
- None

### Debts Deferred
- Step 7, the Windows build and run (`PLAN-ling-5-links.md`, Step 7). ARCHITECT: "it's pointless to test anything on windows, when we're still half assed implementation." No `DEBT.md` entry exists.

---

**End of SPRINT-LOG.md Template**

Copy this template to your project root as `SPRINT-LOG.md` and customize:
- Project name
- Repository URL/path
- Starting date
- Add project-specific rules to CRITICAL RULES section

Rock 'n Roll!  
**JRENG!**
