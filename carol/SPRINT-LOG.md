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

## Sprint 5: ling Builds with cast's Templates — No archetype, ccache ✅

**Date:** 2026-11-12
**Duration:** not measured (one session)
**Scope:** ARCHITECT: *"fix ling like all other cli"*.

### Agents Participated
- COUNSELOR: scaffold read, the two missing `## cmake` keys, validation by read, this log.
- Engineer (sonnet): template and artwork copies, manifest and table conformance, `## library`, the `spell` symlink.

### Files Modified (7 total)
- `cast/cmake.cast`, `cast/installer.cast`, `cast/installer/resources/*` — byte copies of cast's (ccache block, `## library` lane).
- `cast/spell.md` — `@cmake`/`@installer` → local files (was `../../archetype/cli/cast/…`); CMakeLists row as cast's: no `[description]` binding, no `architecture` list row, nine flag rows, `define` → `macro`, `library` → `value`; CMakeLists.txt row removed from `## headers`.
- `project-info.md` — `description` → `projectDescription`; `architecture` key; `installerResourceDirectory` → `${CMAKE_SOURCE_DIR}/cast/installer/resources`; `## architecture` deleted; `## define` → `## macro`; `## library` header-only; `compileJobMemory` (`1280`) and `precompiledHeader` (`JuceHeader.h`) added at `:90-93` — the template's `math(EXPR … / :::compileJobMemory:::)` failed on the empty key.
- `spell` — symlink → `cast/spell.md`, relative.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered
- [x] MANIFESTO.md principles applied

### Problems Solved
- archetype dependency removed.
- `math cannot parse the expression: "11985 / "` — missing `compileJobMemory` key.
- ccache confirmed in the regenerated `CMakeLists.txt:53-57`.

### State for Continuation
- ling's single identifier `help` has no twin in jam's generated headers; a full build result was not reported in this session.
- macOS build not run this session.

### Debts Paid
- None

### Debts Deferred
- None

## Sprint 4: LING Part 4, `CLanguageDocument`: tokens and the declaration tree ✅

**Date:** 2026-11-11
**Duration:** not measured. The sprint is one session.

### Agents Participated
- COUNSELOR — executed `PLAN-ling-4-language-document.md` Steps 1-8. Validated each Engineer return by its own reads. Filtered the 32 Auditor findings. Made small fixes in `Main.cpp`, `text.md`, `HELP.md`, and the PLAN.
- Pathfinder — discovery reads, the doxygen pass, and the Step 9 runs of `ling`.
- Engineer — Steps 1-3, Steps 4-6 (the walk), Steps 7-8, the audit corrections, and the clamp and name corrections.
- Auditor — one pass over Steps 1-8. It reported 32 findings.

### Files Modified (20 total)
**jam:**
- `cast/syntax_tree.md` — `## SyntaxTokenType` rows 5-19 (`identifier` to `macro`). `## CppKeyword` row `tokenDefine` 96. Row `import` renamed `tokenImport` (NAMES Rule 5). Both table briefs rewritten.
- `generated/jam_SyntaxTree.h` — cast output only.
- `user-modules-info.md` — four `jam_syntax_tree` submodule rows for `jam_CLanguageDocument` and `jam_CLanguageValidator`.
- `jam_syntax_tree/jam_syntax_tree.h`, `.cpp` — cast added the include lines.
- `jam_syntax_tree/document/jam_CodeDocument.h`, `.cpp` — `tokens` and `family` are `protected`. A keyword token holds its key under `Id::key`, and any other word is an `identifier` token (`.cpp:169-178`). A punctuation token is one byte (`.cpp:122-128`). `parse` takes `std::string_view codeText` (ruling 50).
- `jam_syntax_tree/document/jam_CLanguageDocument.h`, `.cpp` (new) — `parse (std::string_view)`. `build()` runs the statement walk through `statementConstruction` and `punctuationConstruction`. Lookups `declarationTypes` (ruling 80), `scopeTypes`, `depthChanges`, `comparisonPrefixes`, and `declaratorBytes`. `addElement` is the only writer of `Id::type` and `Id::name`.
- `jam_syntax_tree/document/jam_CLanguageValidator.h`, `.cpp` (new) — the rules `Id::type` and `Id::name` (ruling 31), in the `XmlValidator` form.
- `jam_core/document/jam_Document.h`, `.cpp:290-293` — `getColumnNumber`. The bare `addChild` comment is now a doxygen block.

**ling:**
- `Source/Main.cpp` — `runDocument`, `runDocumentNotFound`, `getDocumentFile`, and `postFlagArgIndex`. The file branch prints `<path>:<line>:<column>: <type> <name>`.
- `cast/text.md` — row `failNotFound`.
- `Source/HELP.md` — rows for `ling <file>`, the rule failure, `ling <missing>`, and `ling <file> <other>`.
- `PLAN-ling-4-language-document.md` — the executed names in the Names table, `addBlock` and `addExtern` in The walk, the Declarations item 8 audit correction, the Step 8 validation, and the Risks line on `define`.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered. The PLAN Names table records each new name with its family.
- [x] MANIFESTO.md principles applied
- [ ] MANIFESTO L: `jam_CLanguageDocument.cpp` holds about 660 lines. The type has one responsibility (PLAN Language Constraints). `getStatementConstruction` is 32 lines: it is one table.

### Problems Solved
- **`Token` is not copyable.** Its properties are a `jam::Array` (`jam_Array.h:895`). `build()` moves the tokens into the work array. `getNameTokens` builds each token from `span`.
- **The PLAN text of item 8 took a member initializer block as the body.** A `{` after an identifier or `>` is now an initializer. The PLAN records this correction.
- **Auditor findings.** 14, 15, 17, 19, 24, 25, 26, 29 and 31 are resolved. 12 is resolved in the PLAN text. 16, 18, 20, 21, 27 and 30 are partly resolved. 1-11 are rejected: the code follows PLAN Declarations items 1-9 and The walk table. 13, 22, 23 and 28 are rejected with reads. 32 is no finding. ARCHITECT received each one in chat.
- **Two of my rejections were wrong**, found when I read the contract again. Finding 27 broke CODING.md:1072, so I re-delegated with each local name verbatim. Finding 16 broke MANIFESTO.md:184, so I added `getNextPosition`.
- **`runDocumentNotFound` had a parameter it did not use.** My spec caused it. The parameter is removed.
- **`juce::ScopedJuceInitialiser_GUI` in `runDocument`** is removed. My stated cause was wrong. `JUCE v8.0.14` goes to stderr, and it still appears there.
- Verification by ARCHITECT and Pathfinder: LING builds, "ling 0.1.0 (4139530)". Step 9 items 2-5 pass. The two `SessionView.cpp` outputs are equal (`cmp` 0, 48 lines each). Stdout holds only declaration rows.

### State for Continuation
- Part 5 is next (tracker `PLAN-ling-0-tracker.md`). The tracker row for Part 4 does not say Done yet.
- The JAM doxygen XML is stale until ARCHITECT runs `<leader>bd`. The warning count is not measured.
- COUNSELOR has not read these doxygen blocks against the code: `jam_CLanguageValidator.h`, the two `jam_Document.h` blocks, the `jam_CodeDocument.h` blocks, and the two `syntax_tree.md` briefs.
- The Step 9 output has rows from Auditor findings 1, 2 and 8: `->` targets, the `(` inside `<...>`, and an unnamed parameter named by its type. They follow the PLAN rules, and a change is a change to the PLAN.
- `JUCE v8.0.14` is written to stderr on each `ling <file>` run. Its source is not read.

### Debts Paid
- None

### Debts Deferred
- Step 10, the Windows build and runs (`PLAN-ling-4-language-document.md`, Step 10). ARCHITECT: "windows deferred". No `DEBT.md` entry exists.

## Sprint 3: LING Part 3, line comments and escapes in the `Document` segments ✅

**Date:** 2026-11-11
**Duration:** not measured. The sprint is one session.

### Agents Participated
- COUNSELOR — executed `PLAN-ling-3-document-segments.md`. Validated each Engineer return by its own reads. Filtered the Auditor findings, and derived the corrections from CONTRACT. Updated the PLAN and the tracker.
- Pathfinder — discovery reads of the engine, `CodeDocument`, and the eleven `getToken` overriders. Wrote the doxygen pass.
- Engineer — Steps 1-5, the cast run with a snapshot diff, and the audit corrections.
- Auditor — one pass over the sprint. It reported eleven findings.

### Files Modified (10 total)
**jam:**
- `cast/core.md:532` — `## tokens` row `doubleSlash`, `//`, "Line comment open.".
- `generated/jam_Core.h:376` — cast output `Chars::doubleSlash`. The snapshot diff showed no other generated change.
- `jam_core/document/jam_Document.h:788-789` — `Vocabulary::lineCommentOpen {}` and `Vocabulary::escape { Chars::nullCharacter }`. The defaults keep every existing aggregate construction valid and equal in behaviour.
- `jam_core/document/jam_Document.h:1147-1151`, `:1171`, `:1250` — `addCommentToken` takes `open` and `close`. `getEscapeLength` takes `escape`. `isCommentOpen` is a `protected` literal test, which `CodeDocument` also calls.
- `jam_core/document/jam_Document.cpp:141-146` — `isPlainText` also tests `lineCommentOpen`.
- `jam_core/document/jam_Document.cpp:359-412` — `getTokens` tests the block comment, then the line comment. The line comment closes at a one-byte newline view (`:363`). The escape inside a quote takes the next byte (`:390`).
- `jam_core/document/jam_Document.cpp:539-588` — `addCommentToken`, `isCommentOpen`, and `getEscapeLength`.
- `jam_syntax_tree/document/jam_CodeDocument.h`, `.cpp` — `getCLangVocabulary` sets `Chars::doubleSlash` and `Chars::backslash` (`.cpp:182-183`). `addCommentToken` has the line form (`:73-99`). `addStringToken` takes the vocabulary and honours the escape (`:101-120`). `addPunctuationToken` stops where a comment opens (`:122-133`). `getToken` has no whitespace branch, and `getWhitespaceLength` is deleted (`:57-65`).
- Doxygen pass: `jam_Document.h:776-806`, `:1010-1039`, `:1130-1171`, `:1239-1250`; `jam_CodeDocument.h:47-63`, `:78-117`, `:144-153`.

**ling:**
- `PLAN-ling-3-document-segments.md` — the audit corrections in Step 3 item 4, Step 4 item 6 and its Lean validation, Step 5 item 5, the Step 6 result, the `#include <a//b.h>` limit, and the doxygen line.
- `PLAN-ling-0-tracker.md:20-21` — Part 2 and Part 3 status set to Done.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered. The new names come from the PLAN Names table: `lineCommentOpen`, `escape`, `doubleSlash`, `getEscapeLength`. `lineCommentClose` is the local constant of PLAN Step 4.3. No other name is new.
- [x] MANIFESTO.md principles applied
- [ ] MANIFESTO L: `getTokens` is 54 lines (`jam_Document.cpp:359-412`). It was 44 before. A split must give back `depth`, `quote`, and `cursor` together. CODING.md forbids an out-parameter and a fake carrier, so the split fails its own check (`auditor.md` Lean). The PLAN Step 4 validation records this.

### Problems Solved
- **The punctuation scanner took the comment opener.** `/` is an operator byte (`cast/syntax_tree.md:324`). For `f();// note\nint y; /* a */`, `addPunctuationToken` took `();//`. `parse` then gave the comment segment to `getToken` in the middle of the comment. The block branch consumed code up to the later `*/`. The scanner now stops on the vocabulary's comment openers. This is the pattern of the sibling scanners (`jam_Css.cpp:141`, `jam_Html.cpp:90`, `jam_MermaidDiagram.cpp:60-62`). A cursor limited to its segment in `parse` was rejected: the XML and HTML markup scanners read a whole tag across segments (`jam_Xml.cpp:210-211`, `:408-441`).
- **The one-loop form made `getTokens` longer.** The PLAN fallback "one loop over the two literal pairs" took 18 lines against 11. It also staged an array and added three names that the PLAN does not hold. It is replaced by two `if` blocks.
- **Auditor findings, resolved or rejected:**
  1. Punctuation scanner past the comment opener: resolved (above).
  2. `getTokens` length: resolved to the simpler form. The remaining crossing is recorded (Alignment Check).
  3. Staged pair array as data in transit: rejected. `getTokens` is creation: "Creation can use … work buffers" (MANIFESTO:254). The array is removed anyway.
  4. Names outside the PLAN: resolved. `comments`, `comment`, `literals`, and `isEscape` are gone.
  5. `CodeDocument::getToken` had 4 `if` branches: resolved. The whitespace branch added no token, and the default branch gives the same tokens.
  6. Type ternary as a truth table: rejected. It has 3 outcomes, and the limit is "more than 3" (MANIFESTO:66).
  7. Comment scan in two files: rejected. Each document type scans its own comment (`jam_Css.cpp:141`, `jam_Html.cpp:90`, `jam_Xml.cpp:193-201`). MANIFESTO: "Find the established pattern. Extend it."
  8. `return isEscape ? 1 : 0;`: resolved to `sizeof (char)`.
  9. `getEscapeLength` took the whole `Vocabulary`: resolved. It takes `escape`.
  10. `escape` against `Chars::escapeCharacter`: rejected. No clause.
  11. Brace position on an initializer: rejected. The brace rule is for code blocks. The line is gone.
- Verification: ARCHITECT built LING, and it runs: "ling 0.1.0 (c89f5d5)".

### State for Continuation
- Next work: Part 4, `PLAN-ling-4-language-document.md` (tracker `PLAN-ling-0-tracker.md:22`). `CLanguageDocument` is the first run-time reader of the C segments.
- The JAM doxygen XML is stale until ARCHITECT runs `<leader>bd`. Doxygen must then give zero warnings for `jam_Document.h` and `jam_CodeDocument.h`.
- The END, cast, and scout builds of PLAN Step 6 have no recorded result from this session. Only the LING build is recorded.
- `jam_CodeDocument.h:120` and `:132` (`addNumberToken`, `addIdentifierToken`) still say "starting at position". These symbols did not change in this sprint.
- clangd returned only declarations for `findReferences` on the framework files. Load the jam project in nvim before you rely on LSP references there.

### Debts Paid
- None

### Debts Deferred
- Step 7, the LING and END builds on Windows (`PLAN-ling-3-document-segments.md`, Step 7). It is ARCHITECT's, and it blocks no part (ruling 78). No `DEBT.md` entry exists.

---

## Sprint 2: LING Part 2, the `jam_syntax_tree` module ✅

**Date:** 2026-11-11
**Duration:** not measured. The sprint is one session.

### Agents Participated
- COUNSELOR — executed `PLAN-ling-2-language-tables.md`, validated each step by its own reads, made the 1-line fixes, and recorded the rulings in the PLAN. Filtered the Auditor findings.
- Pathfinder — discovery reads of the manifest, the cast tables and the generated output. Read the cast file-merge code (`cast/Source/Writer.cpp`, `Shapes.cpp`).
- Engineer — the file move, the manifest rows, the module header and source, the table move, the `spell.md` wiring, the `@code:line` enclosure, the `@pragmaOnce` alias, and the cast runs with snapshot diffs.
- Auditor — one pass over the sprint. It reported four findings.

### Files Modified (16 total)
**jam:**
- `jam_syntax_tree/document/jam_CodeDocument.h`, `.cpp` — moved byte-identical from `jam_core/document/` (sha256 h `d362ffbc…dad9`, cpp `e62cdaf1…d38d6`). Ruling 30: `CodeDocument` leaves `jam_core`.
- `jam_syntax_tree/jam_syntax_tree.h`, `.cpp` — new module header and source, copied from `jam_document` with only the module names and the generated include changed. Cast writes the declaration block and the submodule includes.
- `jam_core/jam_core.h`, `jam_core/jam_core.cpp` — cast removed the two `CodeDocument` includes.
- `user-modules-info.md:155-158` — the `jam_syntax_tree` module group (`kernel`; depends on `juce_core`, `jam_core`). `:1145-1148` — its four submodule rows. The two `jam_core` CodeDocument rows are deleted.
- `cast/syntax_tree.md` — new table file. Its `## index` holds `@operators`, `@quote`, and the six `Family` aliases. It holds the nine tables, unchanged (ruling 36).
- `cast/core.md` — the nine tables and the six `Family` aliases are removed.
- `cast/spell.md:31-32`, `:65`, `:188-189` — aliases of the generated pair, the table file and the module files. `:71` — `@pragmaOnce` holds `#pragma once`. `:284-289` — the `jam_SyntaxTree.h` headers row. `:1192-1324` — the moved output rows. Each of the nine map rows encloses its `@code:namespace` in `@code:line` with `- macro: @pragmaOnce`. The first row holds the file brief and an empty separator cell. The module rows of `jam_syntax_tree` follow the `jam_subprocess` group.
- `generated/jam_SyntaxTree.h`, `.cpp` — new cast output. One `namespace map` block holds the eight tables and the `languageFamily` declaration. The `.cpp` holds the `languageFamily` definition, 27 entries.
- `generated/jam_Core.h`, `.cpp` — the nine tables are removed. No line is added.

**ling:**
- `project-info.md:125` — the `jam_syntax_tree` user-module row, with the module description.
- `PLAN-ling-2-language-tables.md` — Step 5 items 4 and 5a, and Step 7 item 1, record the rulings and the corrections below.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered. Every new name is from a cited family: `jam_syntax_tree` (ruling 30), `syntax_tree.md`, `jam_SyntaxTree`, the alias families, and `@pragmaOnce` (the value-alias family of `@bimap`).
- [x] MANIFESTO.md principles applied. MANIFESTO S: `#pragma once` is declared once, in the index, and the nine rows reference it. ARCHITECT: "Index alias".

### Problems Solved
- **Two `namespace map` blocks in `jam_SyntaxTree.h`.** Cast groups the rows of one file by the template and the placeholder values of the first structure line (`cast/Source/Shapes.cpp:438-458`). `- macro:` is a placeholder of the `namespace` fence (`code.cast:11`), so a macro on the first row only split the block. ARCHITECT: "enclosed means make it as list. although only one. so no dangling macro". Each map row now encloses its namespace in `@code:line` (the form of `spell.md:347-353`). The nine equal `line` rows give one group, and its list merges the nine namespaces into one block (`Shapes.cpp:427-429`, `:301`, `:481-504`).
- **A `//====` line between the blocks.** Cast takes the row join of a file from the separator cell of the file's first row (`Writer.cpp:261-268`). The first row now has an empty separator cell, as the first row of each sibling file does. The moved text is equal line for line to the text that left `jam_Core.h`.
- **The same value nine times.** The nine rows read `@pragmaOnce`. Cast output is byte-identical before and after the alias.
- Auditor findings, resolved or rejected:
  1. `spell.md` first-row separator: resolved, recorded as PLAN Step 5 item 5a.
  2. `core.md` "Extensions pair with map::languageFamily." as a layer violation: rejected. MANIFESTO.md:238 says the topology rule "applies at compile time", and `jam_core` holds no include of `jam_syntax_tree` and no read of its symbols.
  3. `project-info.md:125` held the title: resolved. It now holds the module description, as `:126` does.
  4. `@operators` and `@quote` in three table files: rejected. "Aliases are scoped to the index" (`cast/Source/HELP.md:510`), and `web.md:16-17` declares the same pair for its own tables.
- Verification: the cast runs exit 0, and a second run changes nothing. ARCHITECT built the four macOS projects clean. `ling --version` prints `ling 0.1.0 (c998599)`.

### State for Continuation
- Next work: Part 3, `PLAN-ling-3-document-segments.md` (tracker `PLAN-ling-0-tracker.md:21`).
- `PLAN-ling-0-tracker.md:20` still reads "Written. Locked". The Part 2 status is not updated.
- `jam/.claude/CLAUDE.md:33-61` (MODULE LAYER ORDER) does not list `jam_syntax_tree` (layer 2, depends on core). That file belongs to MACHINIST.
- The JAM doxygen XML is stale after the move until ARCHITECT regenerates it. Before you rely on `docs/xml` for `CodeDocument`, check that it names `jam_syntax_tree/document/`.
- No `cast sync` to KANJUT ran. The module class is `kernel`, so a sync carries it (`cast/Source/HELP.md:85-89`).
- To check the cast output again: run `cast ../jam/cast/spell.md` from `~/Documents/Poems/dev/cast/`, then check that `generated/` does not change.

### Debts Paid
- None

### Debts Deferred
- Step 8, the LING and END builds on Windows (PLAN-ling-2:252-256). It is ARCHITECT's, and it blocks no part (ruling 78). No `DEBT.md` entry exists.

---

**End of SPRINT-LOG.md Template**

Copy this template to your project root as `SPRINT-LOG.md` and customize:
- Project name
- Repository URL/path
- Starting date
- Add project-specific rules to CRITICAL RULES section

Rock 'n Roll!  
**JRENG!**
