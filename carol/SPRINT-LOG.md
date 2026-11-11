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

## Sprint 1: LING Part 1 console project, and JAM markdown help on the terminal cell grid ✅

**Date:** 2026-11-11
**Duration:** not measured. The sprint spans the sessions from 2026-11-10.

### Agents Participated
- COUNSELOR — planned, delegated, and validated each return by its own reads. Filtered the Auditor findings.
- Pathfinder — discovery reads. Wrote the comment pass of the JAM fix.
- Engineer — implemented the Part 1 steps, the `Function::Map` conversions, the mono stamp, and the accepted Auditor fixes.
- Auditor — one pass for Part 1. One pass for the JAM fix.

### Files Modified
**ling (Part 1, `PLAN-ling-1-project.md` Steps 1-10):**
- `project-info.md`, `LICENSE`, `RELEASE.md`, `cast/signing.md` — skeleton from `archetype/cli` and the project, module, define, include, binary, and signing values (Steps 1-5).
- `cast/identifiers.md`, `cast/text.md`, `cast/files.md`, `cast/banner.md`, `cast/source.cast` — the project word tables (Step 6).
- `cast/spell.md` — the tables wired in the manifest (Step 7).
- `Source/style.css`, `Source/Help.h`, `Source/Help.cpp`, `Source/HELP.md` — the help files. `printHelp` takes `std::string_view` (ruling 50) (Step 8).
- `Source/Main.cpp` — the entry file: `--version`, `--help`, no argument, unknown flag (Step 9).
- `Source/generated/*` — generated by cast: `ProjectInfo.h`, `Identifiers.h`, `Text.h`, `Files.h`, `HashMaps.h`, `Generated.h`, `Generated.cpp`.
- The Part 1 Auditor findings were resolved earlier in this sprint. The per-finding record is in the session transcript `/Users/jreng/.claude/projects/-Users-jreng-Documents-Poems-dev-ling/1a1b2755-cff5-470b-8eb1-b1e354f5c8b1.jsonl`.

**jam (markdown help on the cell grid):**
- `jam_style/style_manager/jam_StyleManager.h:182-192`, `.cpp:241-248` — new `StyleManager::getWidthClass (sheetType)`. It returns `AttributedChar::narrow` when the unit of the sheet's `fontSize` (`Format::toType (Id::fontSize)`, the jam_core identifier) is `em`. Otherwise it returns `AttributedChar::proportional`. It uses `HashMap::get`, which gives a default on a miss.
- `jam_markdown/document/jam_MarkdownDocument.cpp:1123`, `.h:770-772` — `addStyle` stamps the width class from that query at atom creation. `AttributedString::append` then sets narrow or wide and adds the spacer.
- `jam_markdown_graphics/style/jam_StyleMarkdown.cpp:91-98, 252-263`, `.h:159-162, 220-221, 356-362` — for a narrow sheet, the cell width and the list marker character width are the base font size. A proportional sheet measures, as before.

**ling, cast, scout, ggwp — `Source/style.css`:**
- `:119` `fontSize: 1em;`. `:133-137` heading1-5 letter spacing set to `0`. `:2-9` header comment: ling line 2 now names ling, and lines 6-9 state the cell-grid meaning.

**cast, scout, ggwp — `Function::Map` conversions (JAM commit `a55270275` API):**
- `cast/Source/Pack.cpp:16-34`, `cast/Source/Transforms.cpp:221-366` — maps declare their uniform signature in the type.
- `scout/Source/Validator.cpp:8-9, 27-28` — same conversion.
- `ggwp/Source/Artwork.cpp:23-38` — same conversion.

### Alignment Check
- [x] BLESSED principles followed
- [x] NAMES.md adhered. `getWidthClass` joins the family of `AttributedChar::getWidthClass`. No new identifier.
- [x] MANIFESTO.md principles applied
- [x] `addStyle` reads the process `StyleManager` instance that its signature does not show. ARCHITECT ruled this: "Read from StyleManager".

### Problems Solved
- **Markdown `--help` drew overlapping text in the terminal.** Runs after the first on a line started at about half their column. Cause: JAM commit `2f94e2f0b` removed the cell source, so every markdown atom was proportional, and font advances at font size 1 are fractions of a cell. Solution: the stylesheet declares the stamp. A sheet whose `fontSize` has the unit `em` stamps narrow, and one cell is the base font size. The one advance method `GlyphArrangement::getCellAdvance` stays unchanged. EVE (`fontSize: 16;`, no unit) stays proportional.
- **Heading letter spacing gave fractional mono advances.** `getCellAdvance` adds `extraKerning` to every width class (`jam_GlyphArrangement.cpp:260`). ARCHITECT chose "Terminal sheets set 0".
- **Ruling 9 collision.** A jam_style `fontSize` row would define `Id::fontSize` again (jam_core `generated/jam_Core.h:83`). The query uses the jam_core identifier, because `Format::toType` maps `fontSize` and `font-size` to `FONT_SIZE`.
- **JAM `Function::Map` API change broke the consumers.** cast, scout, and ggwp now build.
- Verification by ARCHITECT: EVE `test/clusters.sh` passed. cast, scout, ggwp, and ling build, and `--help` renders correctly. Part 1 Step 10 checks all passed.

### State for Continuation
- Next work: Part 2, `PLAN-ling-2-language-tables.md` (tracker `PLAN-ling-0-tracker.md:20`).
- JAM doxygen XML regenerates with `<leader>bd`. Before you rely on `docs/xml` for `StyleManager::getWidthClass`, check that it is there.
- The `ling/Source/style.css` file and the `scout/Source/style.css` file are identical except line 2.

### Debts Paid
- None

### Debts Deferred
- Part 1 Step 11, the Windows build and check (`PLAN-ling-1-project.md:284-288`). ARCHITECT: "windows deffered". No `DEBT.md` entry exists.

---

**End of SPRINT-LOG.md Template**

Copy this template to your project root as `SPRINT-LOG.md` and customize:
- Project name
- Repository URL/path
- Starting date
- Add project-specific rules to CRITICAL RULES section

Rock 'n Roll!  
**JRENG!**
