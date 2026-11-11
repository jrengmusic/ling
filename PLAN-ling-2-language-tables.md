# PLAN: LING Part 2 — The `jam_syntax_tree` Module

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 30, 35, 36.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

This part creates the JAM module `jam_syntax_tree`. It moves `jam::CodeDocument` out of `jam_core` into that module (ruling 30). It moves the nine tables that only `CodeDocument` reads out of `cast/core.md` into a new `cast/syntax_tree.md` (ruling 36).
This part changes no behavior. It adds no table, no row, and no type.
The work is in the JAM repository, `~/Documents/Poems/dev/jam/`. One row is added in the LING project so that a compiler reads the new module.

## What this part does not do

- It adds no row to a table. Ruling 35 extends the tables for LING. Each new row comes in the part that holds its first reader (MANIFESTO **L**, YAGNI: "A parameter nobody passes yet"). Part 4 adds the token rows that `CLanguageDocument` reads. Part 6 adds the file extension rows that the manifest reader needs.
- It does not change `CodeDocument`. The type moves as it is. `getKeywordsForFamily` keeps its switch (`jam_CodeDocument.cpp:226-238`).
- It does not move `## DocumentTokenType`, `## Byte`, `## tokens`, `## markupLanguage`, or `## extensions`. Other modules read them (see the inventory).
- It does not run `cast sync` to KANJUT. See Risks.

## Language / Framework Constraints

- CODING.md, HEADER DISCIPLINE: a submodule header and source file hold no include. The topmost module header holds every include. `jam_CodeDocument.h` and `jam_CodeDocument.cpp` hold no include now, and they hold none after the move.
- CODING.md, TEMPLATE, CONSTEXPR, AND TRANSLATION UNIT PLACEMENT: one object, one header and one `.cpp`. `CodeDocument` keeps its two files.
- CAROL.md Refactor-Rewrite Discipline: delete first. The files leave `jam_core` in Step 1. The compiler shows what remains.
- CAROL.md Git: no step runs a git command. A file move is a plain `mv`.
- `jam/.claude/CLAUDE.md:103-104`: "JAM has no build of its own. Downstream projects compile the modules." and "Agents never build. ARCHITECT builds." Steps 7 and 8 are ARCHITECT's builds, or COUNSELOR's on ARCHITECT's explicit request.
- The parallel `jam::Function::Map` session can change line numbers in `jam_CodeDocument.cpp`. This part moves the file whole, so the content of the file at the time of execution is the content that moves.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Every line number is a read of 2026-11-10. Base path: `~/Documents/Poems/dev/jam/`, unless the row gives another.

### The readers of each symbol

Source: LSP `findReferences` at each declaration in `generated/jam_Core.h`, with END's `compile_commands.json` (`jam/.clangd:2`), plus the doxygen XML of each project.

| Symbol | Declared | Readers outside `generated/` |
|---|---|---|
| `jam::CodeDocument` | `jam_core/document/jam_CodeDocument.h:25` | 23 references, all in `jam_CodeDocument.h` and `jam_CodeDocument.cpp` |
| `CodeDocument::parse` | `jam_CodeDocument.h:40` | the declaration and the definition only; no caller |
| `map::SyntaxTokenType` | `jam_Core.h:577` | `jam_CodeDocument.cpp:87`, `:102`, `:112`, `:148`, `:165` |
| `map::Family` | `jam_Core.h:609` | `jam_CodeDocument.cpp`; the `languageFamily` entries in `jam_Core.cpp:155-181` |
| `map::CppKeyword` | `jam_Core.h:642` | `jam_CodeDocument.cpp:230`, `:236` |
| `map::JsKeyword` | `jam_Core.h:856` | `jam_CodeDocument.cpp:229`, `:235` |
| `map::PythonKeyword` | `jam_Core.h:968` | `jam_CodeDocument.cpp:228`, `:234` |
| `map::code` | `jam_Core.h:1185` | `jam_CodeDocument.cpp:172`, `:179` |
| `map::markup` | `jam_Core.h:1225` | `jam_CodeDocument.cpp:185` |
| `map::data` | `jam_Core.h:1244` | `jam_CodeDocument.cpp:192` |
| `map::languageFamily` | `jam_Core.h:1257`, defined `jam_Core.cpp:153` | `jam_CodeDocument.cpp:253` |
| `map::DocumentTokenType` | `jam_Core.h:516` | 31 references in `jam_core`, `jam_document`, `jam_markdown`, `jam_terminal`, `jam_web`. It stays. |
| `map::Byte` | `jam_Core.h:546` | 160 references in `jam_core`, `jam_markdown`, `jam_terminal`, and four generated headers. It stays. |

More evidence:

- `#include` search over `~/Documents/Poems/dev` gives two lines only: `jam_core/jam_core.h:105` and `jam_core/jam_core.cpp:16`.
- The doxygen XML of `end`, `scout`, `whatdbg`, `eve`, `ggwp`, `sandbox`, and `whelmed` holds no `CodeDocument`, `SyntaxTokenType`, `CppKeyword`, or `languageFamily`. The one match in the XML of `cast` is in `diff/Generated.h`, not in `cast/Source/`. Step 7 builds cast.
- The aliases `@cLang`, `@python`, `@shell`, `@markup`, `@data`, `@js` (`cast/core.md:18-23`) occur in `core.md` only in the `## languageFamily` rows (`core.md:2518-2558`).
- The aliases `@operators` and `@quote` (`core.md:16-17`) occur in `## markupLanguage` also, which stays. `cast/web.md:16-17` declares the same two aliases for its own tables.

Limit of the evidence: the LSP reads the END database only. JAM modules that END does not compile (`jam_markdown_graphics`, `jam_mermaid_diagram`, `jam_opentype`, `jam_pdf`, `jam_plugin_bootstrap`, `jam_fuzzy`, `jam_dsp`, `jam_subprocess`) are not in that result. Steps 7 and 8 close this limit with the compiler (CAROL.md DCF 2).

### The module pattern

| Need | Existing piece | Where |
|---|---|---|
| A module that depends on `juce_core` and `jam_core` and reads its own generated tables | `jam_document`: 33-line header, 5-line `.cpp` | `jam_document/jam_document.h:1-33`; `jam_document/jam_document.cpp:1-5` |
| The same shape with a table file named after the module | `jam_web`, `cast/web.md`, `generated/jam_Web.h` | `jam_web/jam_web.h:27-35`; `jam_web/jam_web.cpp:1-7` |
| The module declaration block | cast writes it between `BEGIN_JUCE_MODULE_DECLARATION` and `END_JUCE_MODULE_DECLARATION` from the `## module` row | `cast/spell.md:3810-3813`; `cast/code.cast:375-393`; `user-modules-info.md:32`, `:171-172` |
| The submodule include lines | cast writes them between `BEGIN_SUBMODULE_INCLUDES` and `END_SUBMODULE_INCLUDES` from the `## submodule` rows, by the `aggregator` cell | `cast/spell.md:3815-3823`; `cast/code.cast:57-59`; `user-modules-info.md:181`, `:1240-1247` |
| The regions that cast owns | `## region` | `user-modules-info.md:1360-1369` |
| The manifest law | every `jam_*` directory is declared in `## module`; every file of a kernel directory is named by one `## submodule` row | `cast/Source/HELP.md:85-89`; `cast/SPEC.md:237-240`, `:1362` |
| The class of the moved files | `kernel` | `user-modules-info.md:235`, `:262` |
| Module rows in name order | `jam_style` `:148`, `jam_subprocess` `:152`, `jam_terminal` `:156` | `user-modules-info.md` |
| The submodule directory name | `document/`: `jam_core/document/`, `jam_document/document/`, `jam_markdown/document/`, `jam_terminal/document/` | directory listing |

### The table pattern

| Need | Existing piece | Where |
|---|---|---|
| A table file | banner fence, `## index` with aliases, then one `##` section per table | `cast/web.md:1-19`, `:63` |
| File aliases for a generated pair and its table file | `@jam_Web`, `@jam_WebSource`, `@web` | `cast/spell.md:29-30`, `:62` |
| File aliases for a module header and source | `@jam_web`, `@jam_webSource` | `cast/spell.md:188-189` |
| The header brief of a generated file | a `## headers` row keyed by the file name | `cast/spell.md:272-276` |
| The first output row of a generated header | `- macro: #pragma once`, `- generated:`, `- [description]: @headers:brief` | `cast/spell.md:1185-1190` |
| A first row that opens `namespace map` with the brief | the `@mermaid:DiagramType` row: `- generated:`, `- name: map`, `- [description]: @headers:brief` | `cast/spell.md:2321-2326` |
| A `Bimap` output row | `@code:bimap` with `@code:name-entry` or `@code:bimap-entry`, and `@code:enum-entry` | `cast/spell.md:380-439`; `cast/code.cast:160-194` |
| A byte class table output row | `@code:lookup-table`, `- type: @byteClass` | `cast/spell.md:485-511`; `:88`; `cast/code.cast:287-294` |
| A hash map: declaration in the header, definition in the source | `@code:hash-map` and `@code:hash-map-definition` with `@code:map-entry` | `cast/spell.md:512-521`, `:550-558`; `cast/code.cast:138-152` |
| Consecutive rows with one namespace name give one namespace block | `namespace map` holds `data`, then the four `extern` lines | `generated/jam_Core.h:1244-1263` |
| The generation command | "`cast cast/spell.md` regenerates `generated/`." | `jam/.claude/CLAUDE.md:106` |

### The nine tables that move (ruling 36)

| Section | `cast/core.md` | Output row in `cast/spell.md` |
|---|---|---|
| `## SyntaxTokenType` | `:226-244` | `:380-391` |
| `## Family` | `:246-266` | `:392-403` |
| `## CppKeyword` | `:268-377` | `:404-415` |
| `## JsKeyword` | `:379-439` | `:416-427` |
| `## PythonKeyword` | `:441-488` | `:428-439` |
| `## code` | `:2426-2469` | `:485-493` |
| `## markup` | `:2471-2491` | `:494-502` |
| `## data` | `:2493-2516` | `:503-511` |
| `## languageFamily` | `:2518-2558` | declaration `:512-521`; definition `:550-558` |

### Names in this part

| Name | Source |
|---|---|
| `jam_syntax_tree` | ruling 30, verbatim |
| `syntax_tree.md` | the family `<module>.md`: `data_structures.md`, `markdown_graphics.md`, `plugin_bootstrap.md` (`cast/spell.md:53-67`) |
| `jam_SyntaxTree.h`, `jam_SyntaxTree.cpp` | the family `jam_<Module>`: `jam_DataStructures`, `jam_MarkdownGraphics`, `jam_PluginBootstrap` (`cast/spell.md:21-22`, `:35-36`, `:43-44`) |
| `@jam_SyntaxTree`, `@jam_SyntaxTreeSource`, `@syntax_tree`, `@jam_syntax_tree`, `@jam_syntax_treeSource` | the alias families at `cast/spell.md:29-30`, `:62`, `:188-189` |
| Module title `JAM Syntax Tree` | the family `JAM Web`, `JAM Fuzzy` (`user-modules-info.md:79`, `:171`) |

No other name is new. No type, function, or variable is added.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: Move the two files out of `jam_core`
**Scope:** `jam/jam_core/document/jam_CodeDocument.h`, `jam/jam_core/document/jam_CodeDocument.cpp`, new directory `jam/jam_syntax_tree/document/`.
**Action:**
1. Make the directory `jam/jam_syntax_tree/document/`.
2. Move `jam_CodeDocument.h` and `jam_CodeDocument.cpp` into it with `mv`. Do not use a git command.
3. Change no byte of the two files.

**Validation:** `jam/jam_core/document/` holds no `jam_CodeDocument.*`. The two files in `jam/jam_syntax_tree/document/` are byte-identical to the files before the move (compare the checksums taken before the move). No git command ran.

### Step 2: Declare the module and its files in the manifest
**Scope:** `jam/user-modules-info.md`.
**Action:**
1. In `## module`, add one row group between the `jam_subprocess` group (`:152`) and the `jam_terminal` group (`:156`), in the shape of the `jam_document` group (`:68-69`):
   - `name`: `jam_syntax_tree`
   - `class`: `kernel`
   - `title`: `JAM Syntax Tree`
   - `description`: `Programming-language tokenizers and parsers over jam::Document`
   - `dependencies`: `juce_core` on the first line, `jam_core` on the second line
   - every other cell empty, as in the `jam_document` group
2. In `## submodule`, delete the two `jam_core` rows of `document/jam_CodeDocument.h` (`:235`) and `document/jam_CodeDocument.cpp` (`:262`).
3. In `## submodule`, add four rows for `jam_syntax_tree`, in the shape and order of the `jam_document` rows (`:292-295`):

   | module | path | class | aggregator |
   |---|---|---|---|
   | `jam_syntax_tree` | `document/jam_CodeDocument.h` | `kernel` | `jam_syntax_tree.h` |
   | `jam_syntax_tree` | `document/jam_CodeDocument.cpp` | `kernel` | `jam_syntax_tree.cpp` |
   | `jam_syntax_tree` | `jam_syntax_tree.cpp` | `kernel` | |
   | `jam_syntax_tree` | `jam_syntax_tree.h` | `kernel` | |

   Put them where the module order of the table puts them. The description cell is empty, as in the `jam_document` rows.
4. Keep the column widths of each table.

**Validation:** No `## submodule` row names `jam_core` with `jam_CodeDocument`. The four new rows name exactly the four files that `jam/jam_syntax_tree/` holds after Step 3 (`cast/SPEC.md:237-240`). The module row has class `kernel`, the class of the two moved rows.

### Step 3: Write the module header and the module source
**Scope:** new files `jam/jam_syntax_tree/jam_syntax_tree.h` and `jam/jam_syntax_tree/jam_syntax_tree.cpp`.
**Action:**
1. Copy `jam/jam_document/jam_document.h` to `jam_syntax_tree.h`. Then change only these lines:
   - `@file jam_document.h` becomes `@file jam_syntax_tree.h`.
   - `@brief Module header aggregating all jam_document submodule includes.` becomes `@brief Module header aggregating all jam_syntax_tree submodule includes.`
   - `#include <generated/jam_Reflow.h>` becomes `#include <generated/jam_SyntaxTree.h>`.
   - Delete the lines between `BEGIN_JUCE_MODULE_DECLARATION` and `END_JUCE_MODULE_DECLARATION`. cast writes them in Step 6.
   - Delete the line between `// BEGIN_SUBMODULE_INCLUDES` and `// END_SUBMODULE_INCLUDES`. cast writes it in Step 6.
2. Copy `jam/jam_document/jam_document.cpp` to `jam_syntax_tree.cpp`. Then change only these lines:
   - `#include "jam_document.h"` becomes `#include "jam_syntax_tree.h"`.
   - `#include <generated/jam_Reflow.cpp>` becomes `#include <generated/jam_SyntaxTree.cpp>`.
   - Delete the line between `// BEGIN_SUBMODULE_INCLUDES` and `// END_SUBMODULE_INCLUDES`.

**Validation:** The header holds three includes, in this order: `<juce_core/juce_core.h>`, `<jam_core/jam_core.h>`, `<generated/jam_SyntaxTree.h>` (the order of `jam_document.h:27-29`). The two files hold the four region markers of `jam_document`. No other text differs from the `jam_document` files.

### Step 4: Move the nine tables to `cast/syntax_tree.md`
**Scope:** `jam/cast/core.md`; new file `jam/cast/syntax_tree.md`.
**Action:**
1. Make `cast/syntax_tree.md`. Its lines 1-10 are lines 1-10 of `cast/web.md` (the banner fence and the blank line).
2. Add `## index` in the shape of `cast/web.md:11-19`, with these rows:

   | alias | symbol |
   |---|---|
   | `@operators` | `map::Byte::operators` |
   | `@quote` | `map::Byte::quote` |
   | `@cLang` | `map::Family::cLang` |
   | `@python` | `map::Family::python` |
   | `@shell` | `map::Family::shell` |
   | `@markup` | `map::Family::markup` |
   | `@data` | `map::Family::data` |
   | `@js` | `map::Family::js` |

3. Cut these nine sections from `cast/core.md` and paste them into `cast/syntax_tree.md`, in this order, each with its description fence and its table, unchanged: `## SyntaxTokenType`, `## Family`, `## CppKeyword`, `## JsKeyword`, `## PythonKeyword`, `## code`, `## markup`, `## data`, `## languageFamily`.
4. In `cast/core.md` `## index`, delete the six rows `@cLang`, `@python`, `@shell`, `@markup`, `@data`, `@js` (`:18-23`). Keep `@operators` and `@quote`: `## markupLanguage` reads them.

This is a scripted move of project text. Follow CAROL.md Destructive-Edit Discipline: dry-run, backup, verify.

**Validation:** `core.md` holds none of the nine section headings and none of the six aliases. `syntax_tree.md` holds each of the nine sections once. The count of table rows in each moved section is equal before and after (for example 5 in `SyntaxTokenType`, 6 in `Family`, 27 in `languageFamily`). `## DocumentTokenType`, `## Byte`, `## tokens`, `## markupLanguage`, and `## extensions` are unchanged in `core.md`. No row was added (this part adds none).

### Step 5: Wire the new files in `cast/spell.md`
**Scope:** `jam/cast/spell.md`.
**Action:**
1. `## index`: add five alias rows, each next to its sibling.
   - `@jam_SyntaxTree` gives `../generated/jam_SyntaxTree.h`; `@jam_SyntaxTreeSource` gives `../generated/jam_SyntaxTree.cpp` (shape of `:29-30`).
   - `@syntax_tree` gives `syntax_tree.md` (shape of `:62`).
   - `@jam_syntax_tree` gives `../jam_syntax_tree/jam_syntax_tree.h`; `@jam_syntax_treeSource` gives `../jam_syntax_tree/jam_syntax_tree.cpp` (shape of `:188-189`, after the `@jam_subprocess` row `:183`).
2. `## headers`: add one row in the shape of the `jam_Web.h` row (`:272-276`):
   - file: `jam_SyntaxTree.h`
   - brief: `@file jam_SyntaxTree.h`, then `@brief JAM Syntax Tree vocabulary — the maps and tables that jam_syntax_tree and the modules above it read.`
3. `## output`, the table rows. Cut the ten row groups that the inventory lists (`:380-439`, `:485-521`, `:550-558`) from the core group. Paste them as one new group directly after the `@jam_WebSource` row group (`:1282-1285`), in this order: `SyntaxTokenType`, `Family`, `CppKeyword`, `JsKeyword`, `PythonKeyword`, `code`, `markup`, `data`, the `languageFamily` declaration, the `languageFamily` definition. In each pasted group:
   - `@core:` becomes `@syntax_tree:` in every list cell.
   - `@jam_Core` becomes `@jam_SyntaxTree`. In the `languageFamily` definition group, `@jam_CoreSource` becomes `@jam_SyntaxTreeSource`.
   - Change nothing else.
4. Each of the nine pasted groups of `@jam_SyntaxTree` encloses its `@code:namespace` in a `@code:line` (the form of `:347-353`, `@code:line` enclosing `@code:struct`). The `line` structure of every group holds `- macro: @pragmaOnce`. The `## index` row `@pragmaOnce` holds `#pragma once` (the value-alias family of `@bimap`), so the value is declared once (MANIFESTO S). ARCHITECT: "Index alias". The first group (`SyntaxTokenType`) also holds `- generated:` and `- [description]: @headers:brief`. ARCHITECT: "enclosed means make it as list. although only one. so no dangling macro". Reason: cast groups the rows of one file by the first structure line and its placeholder values (`cast/Source/Shapes.cpp:438-458`). A `- macro:` on one `@code:namespace` row only gives two `namespace map` blocks. Equal `@code:line` rows give one `line` group, and its list merges the nine namespaces into one block (`Shapes.cpp:427-429`, `:301`, `:481-504`).
5. The `languageFamily` declaration group came from `:512-521` and holds `- generated:` and `- [description]: @headers:brief`. Delete these two lines from the pasted group. The first group now holds them.
5a. The separator cell of the first group (`SyntaxTokenType`) is empty. Cast takes the row join of a file from the first row of the file (`cast/Source/Writer.cpp:261-268`). The first row of each sibling file has an empty separator cell (`@core:identifiers`, `@mermaid:identifiers`), so the blocks inside `namespace map` follow each other with no `//====` line, as in `jam_Core.h`. Source: Step 6 Validation, "correct the `spell.md` rows from the cited sibling rows".
6. `## output`, the module rows. Add three row groups after the `jam_subprocess` module group, in the shape of the `jam_web` groups (`:3810-3823`):
   - `@modules:identity`, `@modules:framework`, `@modules:module:name=jam_syntax_tree`, file `@jam_syntax_tree`.
   - `@modules:submodule:aggregator=jam_syntax_tree.h`, file `@jam_syntax_tree`.
   - `@modules:submodule:aggregator=jam_syntax_tree.cpp`, file `@jam_syntax_treeSource`.
7. Keep the column widths of each table.

**Validation:** No `@core:` cell names one of the nine tables. Each of the nine tables has its output rows once, with file `@jam_SyntaxTree` or `@jam_SyntaxTreeSource`. The `romanNumerals`, `entities`, and `diacritics` groups (`:522-545`, `:559-585`) are unchanged. Every alias that a new row uses is in `## index`. NAMES.md: every new alias is a member of a cited family.

### Step 6: Generate, and read the output
**Scope:** `jam/generated/`, `jam/jam_syntax_tree/jam_syntax_tree.h`, `jam/jam_syntax_tree/jam_syntax_tree.cpp`, `jam/jam_core/jam_core.h`, `jam/jam_core/jam_core.cpp`.
**Action:**
1. Run cast on `jam/cast/spell.md`, as `jam/.claude/CLAUDE.md:106-107` gives it. Use a cast binary built after cast sprint `description-reserved`.
2. Read the result:
   - `generated/jam_SyntaxTree.h` exists. Its head has the form of `generated/jam_Web.h:1-23`: the banner, `@file jam_SyntaxTree.h`, the brief of Step 5, `#pragma once`. Then one `namespace map` block holds `SyntaxTokenType`, `Family`, `CppKeyword`, `JsKeyword`, `PythonKeyword`, `code`, `markup`, `data`, and `extern const jam::HashMap<juce::String, int> languageFamily;`.
   - `generated/jam_SyntaxTree.cpp` exists. It holds `namespace map` with the `languageFamily` definition, 27 entries.
   - `generated/jam_Core.h` and `generated/jam_Core.cpp`: the only change is that the nine tables are gone. `romanNumerals`, `entities`, and `diacritics` are unchanged (`jam_Core.h:1258-1260`).
   - `jam_syntax_tree.h`: the declaration block holds `ID: jam_syntax_tree`, `name: JAM Syntax Tree`, and the two dependency lines. The submodule region holds `#include "document/jam_CodeDocument.h"`.
   - `jam_syntax_tree.cpp`: the submodule region holds `#include "document/jam_CodeDocument.cpp"`.
   - `jam_core.h` no longer holds `#include "document/jam_CodeDocument.h"` (`:105`). `jam_core.cpp` no longer holds `#include "document/jam_CodeDocument.cpp"` (`:16`).
3. Run cast a second time. No file changes.

**Validation:** Each read of item 2 holds. The moved table text in `jam_SyntaxTree.h` is equal, line for line, to the text that left `jam_Core.h` (for example the `SyntaxTokenType` block, `jam_Core.h:570-600` before the change). No other generated file changed. The second run changes nothing (MANIFESTO **D**). A difference outside this list is a discrepancy: correct the `spell.md` rows from the cited sibling rows, not the generated files.

### Step 7: Let a compiler read the module, macOS
**Scope:** `ling/project-info.md` (the LING project of Part 1); builds of LING and END.
**Action:**
1. In `ling/project-info.md` `## user module`, add one row after the `jam_core` row, in the form of the rows there: `| @user-module | jam_syntax_tree | Programming-language tokenizers and parsers over jam::Document |`. The description cell holds the module description of `user-modules-info.md`, as each row there does (`jam_document`: `project-info.md:126`, `user-modules-info.md:68-69`).
2. Build LING: `cast cast/spell.md --debug` in `~/Documents/Poems/dev/ling/` (PLAN-ling-1-project.md Step 10). This compiles `jam_syntax_tree` and `jam_core`.
3. Build END. END does not name the new module. This proves that no code of END, and no JAM module that END compiles, reads `CodeDocument` or one of the nine tables.
4. Build cast and scout. They compile `jam_markdown_graphics`, which END does not.

**Validation:** The four builds finish with no error. `ling --version` prints as in Part 1. If a build reports an unknown `CodeDocument` or an unknown table name, that module or project is a reader that the inventory did not see: add `jam_syntax_tree` to its `dependencies` row in `user-modules-info.md` (a JAM module) or to its `## user module` rows (a project), run cast, and build again. Do not move a table back.

### Step 8: Let a compiler read the module, Windows
**Scope:** the builds of Step 7, on a Windows machine.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:** Build LING and END with the Windows toolchain of each project.
**Validation:** Both builds finish with no error (ruling 16).

## BLESSED Alignment

- **B — Bound.** `jam_syntax_tree` owns `CodeDocument` and the nine tables. `jam_core` owns none of them after Step 6.
- **L — Lean.** The part moves and adds nothing. No row, no type, no parameter comes before its reader (YAGNI).
- **E — Explicit.** The manifest declares the module, its files, and its dependencies (`user-modules-info.md`). cast writes the include lines from those rows.
- **S — SSOT.** Each table has one section in one cast file. The generated headers are output, not source.
- **S — Stateless.** No state is added.
- **E — Encapsulation.** `jam_core` no longer holds the vocabulary of a type that it does not hold. The new module depends on `jam_core`; `jam_core` does not know the new module. The tables that several modules read stay in the lowest module that reads them.
- **D — Deterministic.** Step 6 proves that a second cast run changes nothing, and that the moved text is equal line for line.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open.

Facts that the executor needs:

- **Line numbers.** Every `spell.md`, `core.md`, and `user-modules-info.md` line number is from 2026-11-10. Find each row by its text when the file changed after that date.
- **The form of the first output row (Step 5, item 4).** No generated header of JAM starts without an `Id` namespace. The row combines two existing forms (`spell.md:1186-1189` and `:2321-2326`). Step 6 item 2 is the check.
- **KANJUT.** The module class is `kernel`, so `cast sync` carries it (`cast/Source/HELP.md:85-89`). A sync is not in this part. ARCHITECT runs it at will.
- **Documents outside the numbered parts (MACHINIST).** `jam/.claude/CLAUDE.md:33-61` lists the modules by layer. `jam_syntax_tree` is layer 2 with dependency `core`. The doxygen XML of JAM is stale after the move until ARCHITECT regenerates it.
- **Dependency on Part 1.** Step 7 needs the LING project. If Part 1 is not done, Step 7 items 1 and 2 wait for it; items 3 and 4 do not.
