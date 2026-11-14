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
