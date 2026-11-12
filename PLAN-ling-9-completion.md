# PLAN: LING Part 9 — Completion and Snippets

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 14, 21, 22, 31, 35, 40, 42, 43, 45; tracker Risks items 7 and 15; RFC rulings 4, 9, 11.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

RFC ruling 11: *"completion and snippet support yes. i think it goes hand in hand. our codebase + juce already have best example on well structured and documented code"*.
This part adds one operation, `completion`. It takes a file, a line, and a column. It returns the declarations that the name at that position can complete to. Each entry holds the comment of the declaration. A function entry also holds a snippet with one placeholder for each parameter.
The parse gives the two facts that a reader cannot get from the tree today: the comment of a declaration, and the parameters of a function. Both are written at creation (MANIFESTO **E**, Complete at Creation). The lookup uses the scope walk of Part 7 with a name match in place of an id compare.

## What this part does not do

- No type analysis. A member name (after `.` or `->`) completes to every member declaration with a matching name, as Part 7 links a member reference (PLAN-ling-7-queries.md, The declarations of an element).
- No completion of an unsaved buffer. The source file is the Model (ruling 14), and the watcher is its one author (ruling 45). The client saves the file before it asks (ruling 21: the client acts).
- No comment in `hover` or in the other eight operations. Only `completion` writes the comment.
- No keyword completion. The tree holds declarations; keywords are rows of `## CppKeyword`, not declarations.

## Language / Framework Constraints

- Ruling 14 and ruling 45: the file on disk is the truth, and the watcher reaction is the only path from a file to a document. A request does not carry text.
- MANIFESTO **D**: the same file gives the same answer. The watcher coalesces events (`jam/jam_core/file/jam_Listener.h:22-120`), so a request that comes right after a save could read the document of the old file. The request therefore sends the same change message for its file row that the watcher sends (PLAN-ling-6-manifest.md, item 4 of `LINGModel`). The request runs when the parse ends (PLAN-ling-7-queries.md, The nine operations: "When a parse is pending, it runs the entry when `Id::documents` is written").
- MANIFESTO **E**, Complete at Creation: "A parsed document tree is complete the instant its build operation returns; no downstream consumer mutates it, copies its fields into a working buffer, or re-derives what that operation already produced." The comment and the parameter list are written by `build()`. No reader scans the source for them.
- Ruling 42 and MANIFESTO **S**: the lookup reads the documents in place, through the lingua of `jam::Document`. It builds no index of names.
- Tracker Risks item 15 and ruling 32: a parameter stays a `variable`. No new `SyntaxTokenType` row.
- Ruling 22: LING holds no language name. The comment and the parameters are generic element properties.
- Ruling 35: no new table. Rows only.
- CODING.md: no out-parameter; no fake-carrier struct; no string `==`; no includes in submodule files; uniform signature in the type of the events map.
- CAROL.md Code Hygiene: no comment and no doxygen in this sprint.
- `jam/.claude/CLAUDE.md:104`: "Agents never build. ARCHITECT builds."
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Line numbers are reads of 2026-11-10. Base path for JAM: `~/Documents/Poems/dev/jam/`.

| Need | Existing piece | Where |
|---|---|---|
| The source text of a token | the view of `Token::offset()` and `Token::length()` in `Document::getSource()`, a `const std::string&`. `Document::getText (const Token&)` returns a `juce::String`, so ruling 50 does not use it. | `jam_core/document/jam_Document.h:81-84`, `:932`, `:948` |
| What a document keeps after its parse | `elements`, `source`, `lineOffsets`, `root` | `jam_Document.h:1211-1218` |
| The tokens of a `CodeDocument`, comment tokens included | `tokens`, `protected` after Part 4 Step 2; type `map::SyntaxTokenType::comment` | PLAN-ling-4-language-document.md Step 2; `cast/syntax_tree.md` `## SyntaxTokenType` |
| The walk drops comment tokens | "`build()` makes one work array of the tokens that are not comments, and walks that array" | PLAN-ling-4-language-document.md, The walk |
| The parameter group of a function | `addParameters`: "Each one that passes is a `variable` child of the function element" | PLAN-ling-4-language-document.md, Declarations item 7 |
| A list of elements on an element | `Document::Elements`, `jam::Array<Element*>`; `Id::to`, `Id::in` | `jam_Document.h:187`, `:202-203`; PLAN-ling-5-links.md, The reference element |
| A row moved down to `jam_core` so that two generated files do not define one `Id` | Part 5 Step 1 moves `to` and `in` from `mermaid.md` to `core.md` | PLAN-ling-5-links.md Step 1 |
| `Id::comment` exists in the cast project only | `\| @id \| comment \| \`comment\` \|` | `cast/cast/identifiers.md:35`; no row in `jam/cast/*.md` or in any other project `cast/*.md` |
| No `Id::parameters` | none in `jam/cast/*.md` or in any project `cast/*.md` | read of 2026-11-10 |
| The `## identifiers` table of `jam_core` | `@id` rows in name order | `jam/cast/core.md:29-159` |
| The bytes of a snippet | `## chars`: `close brace` `:585`, `close paren` `:587`, `colon` `:588`, `comma` `:589`, `dollar sign` `:592`, `newline` `:633`, `open brace` `:638`, `open paren` `:640`, `space` `:652` | `jam/cast/core.md` |
| The rules after `build()` | `CLanguageValidator : Document::Validator` with the keys `type` and `name`; `Validator::getFailures` | PLAN-ling-4-language-document.md, The rules after `build()`; `jam_Document.h:839-870` |
| A name match with a score | `Fuzzy::Pattern::parse (query, caseMode, shouldNormalize, scheme)`; `Pattern::match (text, slab)`; `Result::isMatch()`, `Result::score` | `jam_fuzzy/finder/jam_BonusMatrix.h:225-248`; doxygen `structjam_1_1Fuzzy_1_1Result.xml` (`score` line 136, `isMatch` line 139) |
| The working memory of a match | `Fuzzy::Slab`: "Pre-allocated arena ... One Slab per thread" | `jam_BonusMatrix.h:170-193` |
| The case and scheme values | `CaseMode::smart`, `ignore`, `respect`; `Scheme::standard`, `path`, `history` | `jam_BonusMatrix.h:87`, `:147` |
| The slab of `Processor` | `jam::Fuzzy::Slab slab;`, used by `workspaceSymbol` on the message thread | PLAN-ling-7-queries.md Step 5 item 3 |
| The path, the elements of a path, the element at a position | `Processor::getPath`, `getElements`, `getElement` | PLAN-ling-7-queries.md, The path of an element; The element at a position |
| The scope walk and the base step | `Processor::applyToDeclarations` | PLAN-ling-7-queries.md, The declarations of an element |
| The member types | the `static constexpr` table of `map::SyntaxTokenType` values: `field`, `instanceMethod`, `staticMethod`, `staticVariable` | PLAN-ling-7-queries.md Step 2 |
| The function types | `function`, `instanceMethod`, `staticMethod`, `constructor`, `destructor` | PLAN-ling-7-queries.md, `prepareCallHierarchy` |
| The order of `workspaceSymbol` | "ordered by the score of the match and then by path, line, and column" | PLAN-ling-7-queries.md, The nine operations |
| The events map, the request row, the result | `Processor::events`; `REQUEST` with `Id::command`, `Id::arguments`, `Id::result` | PLAN-ling-7-queries.md Step 5 |
| The JSON of one element | `JsonWriter`, its element function; `hover` adds `scope` | PLAN-ling-7-queries.md Step 4; The nine operations |
| The change message of a file row | `sendPropertyChangeMessage (Id::path)` on the `FILE` row, for any watcher event | PLAN-ling-6-manifest.md, item 4 of `LINGModel` |
| The request in the daemon | a `Connection` adds the `REQUEST` row | PLAN-ling-8-daemon.md, In LING: `Server` and `Connection` |

### Names

| Name | Source |
|---|---|
| `completion`, the operation and `Id::completion` | RFC ruling 11, *"completion and snippet support yes"*; the LSP method `textDocument/completion`, the family of ruling 43 |
| `Id::comment`, JSON key `comment` | `cast/cast/identifiers.md:35`, moved down to `jam_core` as Part 5 Step 1 moves `to` and `in` |
| `Id::parameters` | "Parameters" and `addParameters` (PLAN-ling-4-language-document.md, Declarations item 7); a plural noun for an `Elements` list, as `Id::in` |
| `snippet`, JSON key and `Id::snippet` | RFC ruling 11, *"completion and snippet"*; RFC ruling 9, *"definition, jump, snippet, completion"* |
| `addComment` | the `add*` family of Part 4; it adds the `Id::comment` property to an element (NAMES.md Rule 1, `add`) |
| `applyToCompletions` | `applyToDeclarations`, `applyToReferences` (PLAN-ling-7-queries.md); the noun of the output (NAMES.md Rule 7) |
| `pattern` | `Fuzzy::Pattern` |
| `slab` | `Fuzzy::Slab` |

## The design

### Two properties at creation

`CLanguageDocument::build()` writes two more properties.

| Property | On | Value | Author |
|---|---|---|---|
| `Id::comment` | each declaration element | `Tokens`: the comment tokens directly before the first token of its declaration candidate, in source order. "Directly before" means that no token of another type is between them. With no such token, the element has no `Id::comment`. | `addComment (element, candidateStart)`, called by each handler after `addElement` |
| `Id::parameters` | each function element that `addParameters` reads | `Elements`: the `variable` children that `addParameters` makes, in source order. An empty group gives an empty list. | `addParameters` |

`addComment` finds the first token of the candidate in `tokens` by its span offset, with `std::lower_bound` over the offsets (the tokens are in offset order). It then takes the tokens before it while their type is `comment`. This is creation: a work step inside `build()` (MANIFESTO **E**, "Creation can use multiple passes, look-ahead, reverse steps, work buffers").

The first token of a candidate is the token where the handler starts the statement: the first token of a declaration prelude, of a class, enum, or namespace statement (with a `template` prefix), of a `#define` row, of an enum constant, and of a parameter candidate. A second declarator of one statement (Part 4, Declarations item 6) shares the first token of its statement.

`CLanguageValidator` gets two more rules:

| Key | Rule |
|---|---|
| `comment` | each token of `Id::comment` has the type `comment`, and its span is inside the source |
| `parameters` | each element of `Id::parameters` is a `variable` child of the element that holds the list |

### The lookup

`template <typename Function> void Processor::applyToCompletions (const jam::CodeDocument& document, const Element& reference, const Fuzzy::Pattern& pattern, const Function& function);`

`function` receives the index of the file in `paths`, the document, the declaration, and the score: `function (int pathIndex, const jam::CodeDocument& declarationDocument, const Element& declaration, int score)`. A declaration is a candidate when `pattern.match (name, slab)` gives `isMatch()`, with `name` the text of the last `Id::name` token of the declaration (PLAN-ling-4-language-document.md, The element of a declaration).

| The reference | The candidates |
|---|---|
| with `Id::member` | every declaration, in every document, whose type is in the member type table of Part 7 |
| any other | the scope walk below |

**The scope walk.** Let the scope be the path of the parent of the reference, and the qualifier be the segments of its `Id::name` without the last one. The walk visits the scope plus the qualifier, then the scope without its last name plus the qualifier, and so on to the qualifier alone. At each visit it calls `getElements` in every document, and takes the declaration children of each element it gets. Unlike Part 7, the walk does not stop at the first visit that gives a result: a completion lists every name that the position can see. A local and a parameter are children of the function element on the scope path, so the walk gets them with no special case.

**The base step.** At a visit whose elements are a `classToken` or a `structToken` declaration, the walk also visits each class that `applyToDeclarations` gives for each of their `base` children. It keeps the paths it has visited inside the call, as Part 7 does, so a class that names itself as a base ends the step.

**One entry for one symbol.** Two elements with one path are one symbol at two points (PLAN-ling-7-queries.md, The path of an element). The walk keeps the paths it has given inside the call and gives each path once: the first element in the order of file path, line, and column. The `comment` of the entry is the `Id::comment` of the first element of that same order that has one. Both choices are a total order, not a heuristic (MANIFESTO **D**).

### The operation

`completion` is one more entry of the events map of Part 7, keyed by `Id::completion`.

| Operation | Arguments | Result |
|---|---|---|
| `completion` | file, line, column | the candidates of the name that ends before the column |

1. The column is the position of the cursor, after the typed name. The entry calls `getElement (document, line, column - 1)`, so the last byte of the name is under the position.
2. When that element is a `reference`, a `call`, or a `base`, the pattern is `Fuzzy::Pattern::parse (name, Fuzzy::CaseMode::smart, true, Fuzzy::Scheme::standard)`, with `name` the text of the last `Id::name` token of the element, and `applyToCompletions` gives the candidates. Any other element, or no element, gives `[]`.
3. Order: the score, high first; then the path; then the file path, the line, and the column. This is the order of `workspaceSymbol`.
4. Each entry is the JSON object of Part 7 (`path`, `line`, `column`, `type`, `name`), and two more keys, as `hover` adds `scope`:
   - `comment`: the text of the `Id::comment` tokens, each the view of its span in `Document::getSource()`, joined with `Chars::newline`, escaped as every JSON string of `JsonWriter` is. The text is the source text, with its comment markers. The client formats it (ruling 21). No comment gives no key.
   - `snippet`: for a type in the function types of Part 7 only. The text is the name, `Chars::openParen`, one placeholder for each element of `Id::parameters`, separated by `Chars::comma` and `Chars::space`, then `Chars::closeParen`. A placeholder is `Chars::dollarSign`, `Chars::openBrace`, its 1-based number, `Chars::colon`, the name of the parameter, `Chars::closeBrace`. For example `setBounds(${1:x}, ${2:y}, ${3:width}, ${4:height})`. A function with no parameters gives `name()`.

**The file is parsed before the answer.** When `Processor` gets a `REQUEST` row with `Id::completion`, it first sends `sendPropertyChangeMessage (Id::path)` on the `FILE` row of the file argument, the reaction of the watcher (PLAN-ling-6-manifest.md, item 4 of `LINGModel`). That starts a parse of the file as it is on disk, so a parse is pending. The entry then runs when `Id::documents` is written (PLAN-ling-7-queries.md, The nine operations). A file with no `FILE` row gives `[]`.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: The rows
**Scope:** `jam/cast/core.md`; `cast/cast/identifiers.md`; `ling/cast/identifiers.md`; `jam/generated/`; `cast/Source/generated/`; `ling/Source/generated/`.
**Action:**
1. Cut the row `comment` from `cast/cast/identifiers.md:35`. Add the row `@id`, `comment`, `` `comment` `` to `jam/cast/core.md` `## identifiers`, in name order, with the cell shape of the rows there.
2. Add the row `@id`, `parameters`, `` `parameters` `` to `jam/cast/core.md` `## identifiers`, in name order.
3. Run cast on `jam/cast/spell.md`. Run cast on the spell file of the cast project.
4. `ling/cast/identifiers.md`: add `completion` and `snippet`, in name order. Run cast on `ling/cast/spell.md`.

**Validation:** `Id::comment` and `Id::parameters` are in `jam/generated/jam_Core.h` and `.cpp`. `Id::comment` is not in the generated files of the cast project. Each identifier is defined one time (MANIFESTO **S**, SSOT). No table was added (ruling 35).

### Step 2: The comment and the parameters at creation
**Scope:** `jam/jam_syntax_tree/document/jam_CLanguageDocument.h` and `.cpp`.
**Action:**
1. Write `addComment (Element& element, const Token& candidateStart)` as section Two properties at creation gives it. Call it from each handler after `addElement`, with the first token of the candidate.
2. In `addParameters`, add the `Id::parameters` list to the function element, and append each `variable` child that it makes, in source order.
3. Add the rules `comment` and `parameters` to `CLanguageValidator`.

**Validation:** Both properties are written inside `build()`. No reader writes them, and no reader scans the source for them (MANIFESTO **E**, Complete at Creation). The search for the first token uses `std::lower_bound` over the offsets, not a hand loop (CODING.md, use the framework API fully). No new type and no new `SyntaxTokenType` row (tracker Risks item 15). No include in the files.

### Step 3: The lookup
**Scope:** `ling/Source/Processor.h` and `.cpp`.
**Action:** Write `applyToCompletions` as section The lookup gives it. The visited paths and the given paths live inside the call.
**Validation:** It reads the documents in place through `getPath`, `getElements`, and the element lingua; it keeps nothing after it returns (MANIFESTO **S**, Stateless). The base step uses `applyToDeclarations`, so a base has one meaning in both lookups (MANIFESTO **S**, SSOT). The guard of the base step names its threat: a class that is its own base (MANIFESTO **B**, The Guard Rule). The one-entry rule is a total order (MANIFESTO **D**).

### Step 4: The JSON of an entry
**Scope:** `ling/Source/JsonWriter.h` and `.cpp`.
**Action:** Add the keys `comment` and `snippet` to the entry of `completion`, as item 4 of section The operation gives them, in the form in which `hover` adds `scope`.
**Validation:** Each byte of the snippet comes from a `Chars` row. Each key is an identifier. The writer holds no field.

### Step 5: The operation
**Scope:** `ling/Source/Processor.h` and `.cpp`; `ling/Source/HELP.md`.
**Action:**
1. Add the entry `Id::completion` to `events`, with the signature that the map type declares. It runs items 1 to 3 of section The operation.
2. In the reaction to a `REQUEST` row, send the change message of the file row first for `Id::completion`, as section The operation gives it. Select this with one compare of the identifier in `Id::command` with `Id::completion`: an Identifier compare is a pointer compare (CODING.md, No `==` on strings), and one positive check is no branch chain (MANIFESTO **L**). No table holds one entry (MANIFESTO **L**, YAGNI). The entry itself does not send the message: it would need a flag for its second call (MANIFESTO **S**, Stateless).
3. Describe `completion` in `HELP.md`: its arguments, that the column is the cursor after the typed name, that the client saves the file before it asks, and the keys `comment` and `snippet`.

**Validation:** The request does not carry text and does not parse; the watcher reaction parses (rulings 14 and 45). No branch chain on the operation name: the events map selects. `Processor` consumes the row that the event gives.

### Step 6: Build and check, macOS
**Scope:** builds of JAM consumers, cast, and LING; runs of `ling`; one scout manifest.
**Action:**
1. ARCHITECT builds cast, LING, and every project that has `jam_syntax_tree` or reads `Id::comment`.
2. With END's manifest, run `completion` and read the result against the source files:
   - at a call to `addCommentToken` in `jam/jam_syntax_tree/document/jam_CodeDocument.cpp`, with the column after its sixth byte (`addCom`): `addCommentToken` is in the result. Its `comment` is the comment directly above its declaration, when one exists there. Its `snippet` has one placeholder for each parameter of that declaration.
   - at a member call after `.` in the same file: the result holds member declarations whose names match the typed name.
   - at a reference to a parameter inside a function body: the parameter is in the result.
3. Add one line `getVocab` inside the body of a function in `jam_CodeDocument.cpp`, save, and run `completion` at once, at the end of that line. `getVocabulary` and `getVocabularyForFamily` are in the result. Remove the line and save.
4. Run item 2, first case, two times. Compare the outputs byte for byte.
5. Measure with scout: one benchmark for `completion` against a daemon whose parse has ended, the parse of one file included. Record the median beside the medians of Part 8 Step 7 item 9.

**Validation:** Items 2 and 3 hold. Item 4: the outputs are equal (MANIFESTO **D**). ARCHITECT reads item 5 against RFC ruling 4, "ULTRA FAST".

### Step 7: Build and check, Windows
**Scope:** a Windows machine.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:** Build LING. Run item 4 of Step 6 with the Windows manifest.
**Validation:** The outputs are equal between two runs (ruling 16).

## BLESSED Alignment

- **B — Bound.** The document owns the comment and the parameter list as its own properties. `Processor` owns the slab. A lookup holds pointers to elements only for one call.
- **L — Lean.** Two properties, one lookup, one events entry, two JSON keys. No new type, no new table, no new token type.
- **E — Explicit.** A position with no name gives `[]`. A request reads the file as it is on disk; the client saves first.
- **S — SSOT.** The source file is the one truth; the request carries no copy of it. The comment and the parameters are written once, at the parse. The base step and the member types are the ones of Part 7.
- **S — Stateless.** The lookup keeps nothing after it returns.
- **E — Encapsulation.** The data flows one way: the file, the watcher reaction, the document, the lookup, the JSON, the client. The client formats the comment and inserts the snippet (ruling 21).
- **D — Deterministic.** The request waits for the parse of its file, so the answer does not depend on the timing of the watcher. Every result has a total order. Step 6 item 4 proves equal output for equal input.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open.

Facts for the review of the whole PLAN:

- **A member name completes to every member with a matching name.** No type analysis (Part 7). The reference data counts 171 symbols named `get` behind `JuceHeader.h`.
- **The name needs one byte.** Directly after `.`, `->`, or `::`, no name token is at the position, so the result is `[]`.
- **A save of a statement that is not complete changes the walk.** Part 4 reads a prelude to the first `;` or `{` (PLAN-ling-4-language-document.md, The walk). A typed name with no `;` joins the next statement. The name is still a reference, but its parent can change.
- **Each `completion` parses its file once.** The watcher event that follows the save parses the same file again and replaces the document with an equal one.
- **The other eight operations do not parse their file first.** A `goToDefinition` right after a save can read the document of the old file.
- **The comment is the source text with its markers** (`//`, `/**`, `*/`). A comment after a declaration on the same row is not attached.
- **A placeholder holds the parameter name, not its type.** The tree holds the name of a parameter; its type names are references, not text.
