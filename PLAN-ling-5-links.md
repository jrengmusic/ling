# PLAN: LING Part 5 — References and Relations as Metadata of the File Tree

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 13, 14, 28, 31, 35, 40; tracker Risks item 7; RFC rulings 4 and 9.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

Part 4 gives one tree of declarations for one file. This part adds the relations of that file to the same tree, as metadata (ruling 40). Each reference to a name becomes one reference element, a child of the declaration that holds the reference.
A reference to a local or a parameter is linked to its declaration inside the tree. Every other reference keeps the name as it is written and the scope around it. That is the complete relation that one file can state. A reader that holds the documents of all files finds the declaration from it (Part 7).
The work is in `jam::CLanguageDocument`, in the JAM repository.

## Why the file tree holds no link to another file

- MANIFESTO **E**, Complete at Creation: "A parsed document tree is complete the instant its build operation returns; no downstream consumer mutates it". A link to a declaration in another file can be written only after that other file is parsed. That write would change a complete document.
- MANIFESTO **B**: "Every resource, object, and dependency has exactly one owner." One document owns its elements (`jam_core/document/jam_Document.h:1211`). A pointer to an element of another document names an element of another owner.
- Ruling 40: the tree of a file comes from that file. The owner does not track which tree holds which file.
- Tracker Risks item 7: "A link to one declaration that no binding chose can be wrong." Inside one file, only a function scope is closed. A namespace continues in other files, and a class has members in its header. So a link made from one file alone is certain only for a local and a parameter.

## What this part does not do

- No lookup across files. Part 7 does it over the documents of all files, with the navigation of `jam::Document`.
- No type analysis. A member access (`a.b`, `p->b`) is marked as a member access. Part 7 gives it every member with that name (tracker Risks item 7). A declared front end library gives one target (ruling 28).

## Language / Framework Constraints

- MANIFESTO **S** (SSOT): the caller of a reference is its parent element. No property repeats the parent. The position of a reference is the span of its name token.
- MANIFESTO **D**: equal bytes of a file give an equal tree. No other file and no order of files enters it.
- MANIFESTO **L**, 3 branches: a choice by token type or byte is a lookup.
- CODING.md: no out-parameter, no fake-carrier struct, structured bindings, `.at()`, `not`/`and`/`or`, no string `==`.
- CAROL.md Code Hygiene: no comment and no doxygen in this sprint.
- `jam/.claude/CLAUDE.md:104`: "Agents never build. ARCHITECT builds."
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Line numbers are reads of 2026-11-10. Base path: `~/Documents/Poems/dev/jam/`.

| Need | Existing piece | Where |
|---|---|---|
| A link to an element of the same document | an `Element*` property and an `Elements` property; `Elements` is `jam::Array<Element*>` | `jam_core/document/jam_Document.h:187`, `:202-203` |
| The edge pattern inside one document | an edge holds `Id::from` and `Id::to`; each end vertex holds the edge in its own `Id::out` or `Id::in` list. `addEdgeEndpoint (vertex, key, edge)` makes the list on the first call. `addEdgeEndpoints (edge, source, target)` sets both sides. | `jam_mermaid_diagram/diagram/jam_MermaidDiagram.h:852-859` |
| Walk the children of an element | `Element::begin`, `end`; `nextSibling` | `jam_Document.h:229-265` |
| An id compare | `juce::Identifier` equality is a pointer compare | CODING.md, CRITICAL RULES |
| The element writer of Part 4 | `addElement (scope, nameTokens, type)` | PLAN-ling-4-language-document.md Step 4 |
| The statement walk of Part 4 | `addStatements`, `addDeclaration`, `addClass`, `addControl`, `skipStatement`, and the other handlers | PLAN-ling-4-language-document.md, The walk |
| The type table | `## SyntaxTokenType`, keys 0 to 19 after Part 4 | `cast/syntax_tree.md` |
| `Id::to`, `Id::in` | exist in `jam_mermaid_diagram` only | `cast/mermaid.md:197`, `:238` |
| `Id::member` | none exists | `cast/*.md` |
| Rules after `build()` | `CLanguageValidator` | PLAN-ling-4-language-document.md Step 7 |
| The print of the tree | `runDocument` in `ling/Source/Main.cpp` | PLAN-ling-4-language-document.md Step 8 |

`jam_syntax_tree` does not depend on `jam_mermaid_diagram`. `jam_mermaid_diagram` depends on `jam_core` (`jam_mermaid_diagram/jam_mermaid_diagram.h:10-16`). Two generated files cannot both define `Id::to`. The two rows that both modules read move to the module below both, `jam_core`.

### Names

| Name | Source |
|---|---|
| Row names `reference`, `call` | the roles of the tracker's working description: "declaration, definition, reference, call" |
| Row name `base` | the tracker's working description: "the base of a class" |
| `Id::member` | a plain noun, as `Id::name`, `Id::type` |
| `addReferences` | the `add*` family of Part 4 |
| `addLocalLinks` | the pass family of `Model`: `addDescriptions`, `addChains` (`cast/Source/Model.h:358`, `:785`) |
| `addLinkEndpoint`, `addLinkEndpoints` | `addEdgeEndpoint`, `addEdgeEndpoints` (`jam_MermaidDiagram.h:853-859`) |
| `addDeclarationContents`, `getLocalDeclarations` | the `add*` and `get*` families of Part 4 (NAMES.md Rule 5), set in execution |
| `getReferenceOwner`, `getBaseStart`, `getCandidateEnd` | the `getPreludeEnd`, `getBodyStart`, `getQualifierStart` family (NAMES.md Rule 5), set in execution |
| `isMemberAccess`, `isLocalReference` | NAMES.md Rule 1, the boolean form `is`, set in execution |
| `getElementFailures` | `Validator::getFailures` (`jam_Document.h:878`), set in execution |
| `appendPosition`, `appendName`, `appendLinks`, `appendDeclaration`, `linkIndent` | the `jam::Format::appendUtf8`, `appendDecimal` family, set in execution |

## The design

### The reference element

A reference is one more element, made by `addElement` of Part 4:

| Part | Value |
|---|---|
| parent | the declaration whose statement holds the reference. A reference in a function body, in its parameters, in its return type, or in a lambda inside it is a child of that function. A reference in the prelude of a class is a child of the class. A reference in a statement that declares nothing is a child of the scope element. |
| `Element::id` | the name that is used |
| `Id::type` | `reference`; or `call` when the byte `(` follows the name; or `base` for a name in the base list of a class |
| `Id::name` | `Tokens`: the qualifier identifiers and the name, as in Part 4. `A::b` gives two tokens. |
| `Id::member` | `bool`, present and true only when `.` or `->` is directly before the name |
| `Id::to` | `Elements`: the local or parameter declarations that the name means. Present only for a local link. |

A local or parameter declaration that a reference names holds that reference in its own `Id::in` list.

### Which tokens are references

Every `identifier` token that is not a token of the `Id::name` of a declaration is one reference. The walk of Part 4 reads every token one time. Each handler gives each token range that it does not walk as statements to `addReferences (owner, start, end, type)`:

| Range | Owner |
|---|---|
| the prelude of a declaration, without its name tokens; its parameter group; its initializer; a block that Part 4 skips (a brace initializer, a lambda body) | the element of that declaration |
| the prelude of a class, an enum, or a namespace | that element |
| a statement that makes no element (a call, an assignment, a `return`, a macro call) | the scope element |
| the `(` `)` group of a control statement | the scope element |
| a directive row | no owner: a directive gives no reference in this part |

In the prelude of a class, the names after the first single `:` are `base` references. Each other name is a `reference`.

A keyword, a number, a string, and a punctuation token give no reference.

### The local link

`addLocalLinks` runs at the end of `build()`. For each reference that has no qualifier and no `Id::member`, it goes up the parents from the reference. It stops at the first element that is not a function type. At each function-type element on the way it collects every `variable` or `staticVariable` child with the same id. The first level that gives one declaration or more ends the walk, and all declarations of that level are the `Id::to` list (tracker Risks item 7). Each of them gets the reference in its `Id::in` list.

A reference that finds no local keeps no `Id::to`. Its relation is then stated by its id, its `Id::name`, its `Id::member`, and its chain of parents. These four are what the lookup across files reads.

### What a reader gets from one file tree

| Relation | From the tree of one file |
|---|---|
| A reference to a local or a parameter, and all references to it | `Id::to` of the reference; `Id::in` of the declaration |
| The caller of a call | the parent of the `call` element |
| The bases of a class, as written | the `base` children of the class element |
| The qualified name of a declaration | the ids of its parents, then its own id |
| A reference that is not local | the name as written, the member mark, and its parents. Part 7 walks the parents and calls `getChildByID` in each document. |

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: Move two identifiers down, and add the rows
**Scope:** `jam/cast/mermaid.md`, `jam/cast/core.md`, `jam/cast/syntax_tree.md`, `jam/generated/`.
**Action:**
1. Cut the rows `in` (`mermaid.md:197`) and `to` (`mermaid.md:238`) from `mermaid.md` `## identifiers`. Paste them into `core.md` `## identifiers`, in name order, with the cell shape of the rows there.
2. Add the row `@id`, `member`, `` `member` `` to `core.md` `## identifiers`, in name order.
3. `## SyntaxTokenType`: add three rows after `macro`: `reference` 20, `call` 21, `base` 22.
4. Run cast on `jam/cast/spell.md`.

Execution correction: the row `link` (`mermaid.md:203`) moved with `in` and `to`. Step 4 names the rule key `link`, and `jam_syntax_tree` does not depend on `jam_mermaid_diagram`. The rule of the inventory applies: a row that both modules read moves to `jam_core`.

**Validation:** `Id::in`, `Id::to`, and `Id::link` are in `generated/jam_Core.h` and `.cpp`, and not in `generated/jam_Mermaid.h` and `.cpp`. Each is defined one time (MANIFESTO **S**, SSOT). `Id::out` stays in `mermaid.md`: this part does not read it. No table was added.

### Step 2: Make the reference elements in the walk
**Scope:** `jam/jam_syntax_tree/document/jam_CLanguageDocument.h` and `.cpp`.
**Action:**
1. Write `addReferences (owner, start, end, type)`. `type` is `map::SyntaxTokenType::reference` or `map::SyntaxTokenType::base`. It reads the tokens of the range. For each `identifier` token it collects the qualifier tokens that `::` joins before it, calls `addElement (owner, nameTokens, elementType)`, and adds `Id::member` when `.` or `->` is directly before the name. `elementType` is `call` when `type` is `reference` and `(` follows the name. Otherwise it is `type`. `addClass` passes `base` for the range after the first single `:` of its prelude.
2. In each handler of Part 4, call `addReferences` for each range of the table in section Which tokens are references. After this step, every token of the work array is read by exactly one of: a name of a declaration, a statement walk, or `addReferences`.
3. Change no rule of Part 4 for what is a declaration.

**Validation:** No identifier token is in two elements. No identifier token outside a directive is in no element. `addElement` stays the one writer of `Id::type` and `Id::name`. The caller is the parent; no `Id::from` is written (SSOT).

### Step 3: The local links
**Scope:** `jam_CLanguageDocument.h` and `.cpp`.
**Action:**
1. Write `addLinkEndpoint (element, key, linked)` and `addLinkEndpoints (reference, targets)` with the behavior of `addEdgeEndpoint` and `addEdgeEndpoints` (`jam_MermaidDiagram.h:852-859`): the first sets or extends an `Elements` property; the second sets `Id::to` on the reference and adds the reference to the `Id::in` list of each target.
2. Write `addLocalLinks()` as section The local link gives it. `build()` calls it as its last step.

**Validation:** Every `Id::to` and `Id::in` entry is an element of the same document. A target is a `variable` or `staticVariable` child of a function-type element that is a parent of the reference. No link reaches a class, a namespace, or the root level (tracker Risks item 7). The tree is complete when `build()` returns, and nothing writes to it after that.

### Step 4: The rules for the references
**Scope:** `jam/jam_syntax_tree/document/jam_CLanguageValidator.h` and `.cpp`.
**Action:**
1. The `type` rule of Part 4 accepts the keys from `namespaceToken` to `base`.
2. Add one rule, key `link`: every element of an `Id::to` list has the id of the reference; every element `e` of the `Id::to` list of `r` holds `r` in its `Id::in` list, and the reverse.

**Validation:** The rules read a `const Document&` and change nothing.

### Step 5: Print the references
**Scope:** `ling/Source/Main.cpp`.
**Action:** In `runDocument`:
1. A reference row has the form of a declaration row. When `Id::member` is present, the row ends with ` member`. When `Id::name` holds a qualifier, `<name>` is the qualifier and the name joined with `::`.
2. After the row of an element that has an `Id::to` list, print one row for each target: four spaces, `to `, then `<line>:<column>` of the target.

**Validation:** The print reads the tree and changes nothing. `Main.cpp` holds no rule of the language.

### Step 6: Build and check, macOS
**Scope:** builds of LING, END, cast, and scout; runs of `ling`.
**Action:**
1. Build LING, END, cast, and scout. LING, cast, and scout compile `jam_mermaid_diagram` (PLAN-ling-1-project.md, inventory, the module closure). The builds prove that the moved identifiers are defined one time.
2. Run `ling ~/Documents/Poems/dev/jam/jam_syntax_tree/document/jam_CodeDocument.cpp`. Check four cases against the file:
   - a reference to the parameter `cursor` inside `addStringToken` has one `to` row, the parameter of that function;
   - a reference to a local, for example `startOffset`, has one `to` row, its declaration in the same function;
   - the `call` to `addCommentToken` inside `getToken` has no `to` row, and its parent is the `getToken` element;
   - a reference to `Document::Token` prints the name `Document::Token` and has no `to` row.
3. Run `ling ~/Documents/Poems/dev/end/Source/ui/SessionView.cpp` two times. Compare the two outputs byte for byte.

**Validation:** The four cases hold. The two outputs are equal (MANIFESTO **D**). The validator reports no failure.

### Step 7: Build and check, Windows
**Scope:** the build and item 3 of Step 6 on a Windows machine.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:** Build LING. Run item 3.
**Validation:** The output is equal to the macOS output, except for the path text (ruling 16).

## BLESSED Alignment

- **B — Bound.** One document owns its elements and its links. No pointer leaves the document.
- **L — Lean.** One element shape serves references, calls, and bases. The tree holds no relation that one file cannot state.
- **E — Explicit.** A reference that is not local carries the name as written. Nothing is guessed from one file.
- **S — SSOT.** The parent is the caller. The span of the name token is the position. `Id::to` and `Id::in` are defined one time, in `jam_core`.
- **S — Stateless.** The walk state lives inside `build()`.
- **E — Encapsulation.** The tree is complete when `build()` returns. No later pass writes to it.
- **D — Deterministic.** The tree is a function of the bytes of one file. Step 6 item 3 proves equal output for equal input.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open.

Limits of the table front end. Each is a fact for the review of the whole PLAN:

- **The order of declarations is not read.** A reference before a local declaration of the same name still links to it.
- **A local that hides a parameter or an outer local** takes the link, by the first-level rule. A reference before the inner declaration then links to the inner one.
- **A macro argument** is a reference of the scope. The walk does not expand the macro.
- **Template arguments and dependent names** are plain references.
- **The limits of Parts 3 and 4** stay.
- **The lookup across files** (namespaces that continue in other files, a definition outside its class, base classes, member access, overloads, `using`) is in Part 7. Its limits are listed there.
