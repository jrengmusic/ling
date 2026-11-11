# PLAN: LING Part 7 — The Lookups and the Nine Operations

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 21, 22, 40, 42, 43; tracker Risks item 7; RFC rulings 4 and 9. Rulings 70, 73, 74.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

Part 6 gives one document for each file row, held by `Processor`. This part adds the lookups that go across those documents, and the nine operations that a client calls (ruling 43).
Every lookup uses the navigation of `jam::Document`: the root, the parent, the id, the children, the properties (ruling 42). The tree is the index. No second structure is built.
The output is JSON on stdout.

## What this part does not do

- No daemon. Each call reads the manifest, parses every file, and answers one operation. Part 8 keeps the documents in one process.
- No completion. Part 9.
- No type analysis and no macro expansion. The limits of Parts 4 and 5 stay.

## Language / Framework Constraints

- Ruling 42 (d): every lookup uses the lingua of `jam::Document`. A lookup reads the documents in place. It builds no table, no hash of a name, and no list that it keeps (MANIFESTO **S**, Stateless).
- MANIFESTO **S** (Stateless): a lookup holds transient state only, inside one call.
- Ruling 22: no language name in LING. A lookup reads `Element::id`, `parent`, the children, `Id::name`, `Id::type`, `Id::member`, `Id::to`, and `Id::in`. `map::SyntaxTokenType` is the type vocabulary of every language (rulings 35 and 39).
- Tracker Risks item 7: a lookup returns every declaration of the first scope that has one. It selects none by a heuristic.
- MANIFESTO **L**, 3 branches: the operation is selected through a table.
- CODING.md: no out-parameter; no fake-carrier struct: a result goes to a callback, as `Element::applyFunctionRecursively` does; no string `==`.
- CAROL.md Profiling: measurement uses scout.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Line numbers are reads of 2026-11-10. Base path for JAM: `~/Documents/Poems/dev/jam/`.

| Need | Existing piece | Where |
|---|---|---|
| The documents | `Processor::documents`, keyed by the index of the path in `Processor::paths` | PLAN-ling-6-manifest.md, In LING: the documents |
| An events map | "Each reactive object owns an events map (`jam::Function::Map` keyed by `juce::Identifier`) — direct lookup dispatch"; `ConfigModel::getEventKey` | `end/ARCHITECTURE.md:906-907`; `end/Source/config/ConfigModel.cpp:106-114` |
| A command id | "A command id is a callback id, owned by the owner of the state that its callback changes." | `end/ARCHITECTURE.md:841-842` |
| A type that holds parsed documents and "the row and reference lookups the rest of the engine reads it through" | cast's `Model`, with `getValue`, `getTable`, `getSource`, `getFile`, each `const` | `cast/Source/Model.h:7-21`, `:93-124`, `:486-527` |
| The root, the parent, the children | `getRoot`; `Element::parent`, `firstChild`, `nextSibling`; `begin`, `end` | `jam_core/document/jam_Document.h:229-265`, `:910-913` |
| A child by id | `Element::getChildByID`: "the mirror of juce::XmlElement::getChildByName"; `Document::getChildByID (parent, id)`, O(1) | `jam_Document.h:268-290`, `:921-929` |
| A list of ids | `Document::Identifiers`, `jam::Array<juce::Identifier>` | `jam_Document.h:188` |
| A walk with a callback | `Element::applyFunctionRecursively`; `applyToProperties` | `jam_Document.h:419-442` |
| A property read | `get (key, defaultValue)`, `find`, `contains` | `jam_Document.h:335`, `:359`, `:373` |
| The row and the column of an offset | `Document::getLineNumber`; `Document::getColumnNumber` | `jam_Document.h:959`; PLAN-ling-4-language-document.md Step 3 |
| The element by a position in the engine | `Document::Index::getElement (lineNumber)` | `jam_Document.h:546` |
| The local links | `Id::to`, `Id::in` on a reference and on a local declaration | PLAN-ling-5-links.md, The local link |
| A name match with a score | `Fuzzy::Pattern::parse (query, caseMode, shouldNormalize, scheme)`, then `Pattern::match (text, slab)`; `Fuzzy::Result::isMatch()`, `Result::score` | `jam_fuzzy/finder/jam_BonusMatrix.h:225-248`; doxygen XML `structjam_1_1Fuzzy_1_1Result.xml`; `jam_fuzzy/jam_fuzzy.h:9-10` |
| The working memory of a match | `Fuzzy::Slab`: "Pre-allocated arena ... One Slab per thread" | `jam_fuzzy/finder/jam_BonusMatrix.h:170-193` |
| JSON text from a model | cast's `struct JsonWriter`: no state; `getText (...)` returns the JSON; `getString` uses `juce::JSON::escapeString` | `cast/Source/JsonWriter.h:14-66` |
| Text that the code creates | a `std::string`, built with `jam::Format::appendDecimal`, `appendUtf8`, `appendQuoted` (ruling 50) | `jam_core/text/jam_Format.h:404-432` |
| A JSON string escape | none in JAM: `appendQuoted` replaces only the quote byte (`jam_Format.h:422-432`), and JSON also escapes `\` and control bytes. `juce::JSON::escapeString` (ruling 51: JUCE where JAM has none) | JUCE `juce_core/json/juce_JSON.h:217-218` |
| The nine names | the operations of the Claude Code LSP tool: `goToDefinition`, `findReferences`, `hover`, `documentSymbol`, `workspaceSymbol`, `goToImplementation`, `prepareCallHierarchy`, `incomingCalls`, `outgoingCalls` | `~/.carol/skills/doxygen-protocol/SKILL.md:12-21` |

### Names

| Name | Source |
|---|---|
| The nine operation names | ruling 43, verbatim |
| `Processor::getElement (file, line, column)` | `Document::Index::getElement` |
| `Processor::getPath` | cast's `Processor::getFile`, `getValue`: `get` and one noun. "path" is the word of `Id::path` for where a thing is; here it is the ids from the root to an element. |
| `Processor::getElements (document, path)` | `get` and the plural of `Element`; `Document::Elements` (`jam_Document.h:187`) |
| `Processor::applyToDeclarations`, `Processor::applyToReferences` | `Element::applyToProperties`; "declaration" and "reference" are the words of the tracker's working description |
| `JsonWriter` | cast, verbatim (`cast/Source/JsonWriter.h:14`) |
| `appendString`, `appendElement` | the `append*` family of `jam::Format`: `appendDecimal`, `appendUtf8`, `appendQuoted` (`jam_core/text/jam_Format.h:412-432`) |
| `events` | the events map of END (`end/ARCHITECTURE.md:906-907`); its type is END's `Events` (`end/Source/config/ConfigModel.h:62`) |
| `Id::request`, `Id::command`, `Id::arguments`, `Id::result` | plain nouns, as `Id::message`. `arguments` is the JSON key of an entry of `compile_commands.json` (PLAN-ling-6-manifest.md Step 4). |
| JSON keys `path`, `line`, `column`, `type`, `name`, `scope`, `children` | `Id::path`, `Id::line`, `Id::type`, `Id::name`, `Id::children` where JAM has them; a `ling/cast/identifiers.md` row for each key that JAM does not have |

## The design

### The path of an element

`static jam::Document::Identifiers Processor::getPath (const Element& element);`

The path is the ids of the parents of the element, from the root down, and then the segments of its own `Id::name`. For `namespace jam { struct Foo { void bar(); }; }` the path of `bar` is `jam`, `Foo`, `bar`. For the definition `void Foo::bar()` inside `namespace jam`, `Id::name` holds `Foo` and `bar`, so the path is the same. Two elements with one path are one symbol at two points (tracker Risks item 7).

`static jam::Array<const Element*> Processor::getElements (const jam::CodeDocument& document, const jam::Document::Identifiers& path);`

It starts at the root of one document. For each id of the path it takes every child of the elements it holds whose path continues with that id. A declaration whose `Id::name` holds several segments matches several ids in one step. It returns the declaration elements at the end of the path. A document that does not hold the path returns an empty array.

### The declarations of an element

`template <typename Function> void Processor::applyToDeclarations (const jam::CodeDocument& document, const Element& element, const Function& function) const;`

`function` receives the index of the file in `paths`, the document, and the declaration: `function (int pathIndex, const jam::CodeDocument& declarationDocument, const Element& declaration)`. The index is a handle, not a copy of the path (MANIFESTO, Not a copy).

| The element | The declarations |
|---|---|
| a declaration | every declaration with the same path, in every document |
| a reference with `Id::to` | the elements of `Id::to`. They are locals or parameters of the same document. |
| a reference with `Id::member` | every declaration, in every document, whose id is the last name segment and whose type is `field`, `instanceMethod`, `staticMethod`, or `staticVariable` |
| any other reference | the scope walk |

**The scope walk.** Let the scope be the path of the parent of the reference, and the name be the segments of its `Id::name`. The walk tries the scope plus the name, then the scope without its last id plus the name, and so on to the name alone. At each try it calls `getElements` in every document. The first try that gives one declaration or more ends the walk, and all declarations of that try are the result.

**The base step.** When a try gives nothing, and the scope of that try is the path of a `classToken` or `structToken` declaration, the walk takes each `base` child of each such declaration, gets its declarations by this same function, and tries the path of each of them plus the name. It keeps the paths it has tried, so a class that names itself as a base ends the step. The base step comes before the next shorter scope.

### The references to a declaration

`template <typename Function> void Processor::applyToReferences (const Element& declaration, const Function& function) const;`

`function` receives the index of the file in `paths`, the document, and the reference.

- For a declaration with `Id::in` (a local or a parameter): the elements of `Id::in`.
- For any other declaration: every element of type `reference`, `call`, or `base`, in every document, whose id is the id of the declaration, and whose declarations (by `applyToDeclarations`) hold an element with the path of the declaration.

### The element at a position

`const Element* Processor::getElement (const jam::CodeDocument& document, int line, int column) const;`

It walks the document and returns the element whose last `Id::name` token is on that row and covers that column, by `getLineNumber` and `getColumnNumber`. It returns `nullptr` when no name is there.

### The nine operations

A request is a row of the state tree, and the answer is a property of that row:

```
LING
  REQUEST (Id::request)     Id::command (the operation), Id::arguments (the arguments), Id::result (the JSON)
```

- The application adds a `REQUEST` row with `Id::command` and `Id::arguments`.
- `Processor` listens to the root. On `valueTreeChildAdded` of a `REQUEST` row it gets the entry of its events map for the identifier in `Id::command` and runs it with the row. The entry reads `Id::arguments` and writes the JSON to `Id::result` of that row. When a parse is pending, it runs the entry when `Id::documents` is written.
- The application listens to the row. On the change of `Id::result` it prints the JSON to stdout, removes the row, and stops the message loop.

The nine operation names are identifiers, and they are the keys of the events map (`end/ARCHITECTURE.md:841-842`, `:906-907`).

The command is `ling <operation> <project-info.md> [--<word>] <arguments>`. Each operation has a fixed count of arguments. The manifest and the toolchain argument are those of Part 6 (PLAN-ling-6-manifest.md, In LING: the application).

| Operation | Arguments | Result |
|---|---|---|
| `goToDefinition` | file, line, column | the declarations of the element at the position |
| `findReferences` | file, line, column | the references to each declaration of the element at the position |
| `hover` | file, line, column | the declarations, each with its `scope`: the ids of its path without the last one |
| `documentSymbol` | file | the declarations of that file as a tree, each with `children` |
| `workspaceSymbol` | query | every declaration, in every document, whose id matches the query by `Fuzzy::Pattern::match`, ordered by the score of the match and then by path, line, and column |
| `goToImplementation` | file, line, column | for a class: each class that holds a `base` reference to it, and the classes that derive from those. For a member of a class: the members with the same id in those classes. |
| `prepareCallHierarchy` | file, line, column | the declarations of the element at the position whose type is `function`, `instanceMethod`, `staticMethod`, `constructor`, or `destructor` |
| `incomingCalls` | file, line, column | for each `call` reference to those declarations: the declaration that is the parent of the reference |
| `outgoingCalls` | file, line, column | for each `call` reference below those declarations in the trees: its declarations |

The result is one JSON array. Each entry is an object with `path`, `line`, `column`, `type` (the `map::SyntaxTokenType` value), and `name`. `hover` adds `scope`, an array of strings. `documentSymbol` adds `children`, an array of entries. The entries of every operation except `workspaceSymbol` and `documentSymbol` are in the order of path, then line, then column. An empty result is `[]`. An operation that the events map does not hold, or a wrong count of arguments, is a `setMessage` with the text row `failArguments`, and the result is `[]`. `Processor` calls `events.contains` before `events.get`, as `Session::valueTreePropertyChanged` does (`end/Source/plugins/Session.cpp:88-92`): `get` throws for an absent key (`jam_core/function_map/jam_Function.h:229-234`). `Id::command` holds the word as text. `Processor` makes the `juce::Identifier` for `events.contains` only when the text is not empty: the constructor asserts on an empty string, "An Identifier cannot be created from an empty string!" (JUCE `juce_core/text/juce_Identifier.cpp:57-62`), and an argument can be the empty text `""`. An empty word is an operation that the events map does not hold. This is the one test of the word; `Main.cpp` and `Connection` hold none.

A first argument that is an existing file is the manifest: the run of Part 6 (ruling 70).

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: The path and the elements of a path
**Scope:** `ling/Source/Processor.h` and `.cpp`.
**Action:** Write `getPath`, `getElements`, and `getElement (document, line, column)`, as the design gives them. A type from `namespaceToken` to `macro` is a declaration.
**Validation:** Each function is `const` or `static`, reads the documents in place, and keeps nothing. `getElements` walks children by id; it builds no map. No out-parameter.

### Step 2: The declarations of an element
**Scope:** `ling/Source/Processor.h` and `.cpp`.
**Action:** Write `applyToDeclarations` with the four cases of its table, the scope walk, and the base step. The set of member types is a `static constexpr` table of `map::SyntaxTokenType` values. The paths that the base step has tried live inside the call.
**Validation:** The result is every declaration of the first try that has one, and none of a later try (tracker Risks item 7). The guard of the base step names its threat: a class that is its own base, directly or through others (MANIFESTO **B**, The Guard Rule). No rule of a language: the function reads ids, types, and marks.

### Step 3: The references to a declaration
**Scope:** `ling/Source/Processor.h` and `.cpp`.
**Action:** Write `applyToReferences` as the design gives it.
**Validation:** A reference is in the result only when `applyToDeclarations` of that reference gives the declaration. So `findReferences` and `goToDefinition` agree by construction (MANIFESTO **S**, SSOT: one lookup, used in both directions).

### Step 4: `JsonWriter`
**Scope:** new files `ling/Source/JsonWriter.h` and `.cpp`; `ling/cast/identifiers.md`.
**Action:**
1. Add the JSON key rows that JAM does not have to `ling/cast/identifiers.md`. Run cast (tracker, The contract of every part, cast runs).
2. Write `struct JsonWriter` with no state, in the form of cast's (`cast/Source/JsonWriter.h:14-66`), with the text types of ruling 50: the JSON is one `std::string` that the caller gives, and each function appends to it. `appendString` appends a quoted, escaped string with `juce::JSON::escapeString` (JAM has no JSON escape; ruling 51). `appendElement` appends the JSON object of one element from its path index, its document, and the element. Numbers go through `jam::Format::appendDecimal`. Names follow the `append*` family of `jam::Format`.
**Validation:** The writer reads the tree and the document. It holds no field. Each key is an identifier, not a string in the code.

### Step 5: The operations
**Scope:** `ling/Source/Processor.h` and `.cpp`; `ling/Source/Main.cpp`; `ling/project-info.md`; `ling/Source/HELP.md`; `ling/cast/text.md`; `ling/cast/identifiers.md`.
**Action:**
1. Add `jam_fuzzy` to `## user module` of `ling/project-info.md`.
2. `cast/identifiers.md`: add the nine operation names, and `request`, `result`. `Id::command` exists in `jam/cast/terminal.md:45` (`jam_terminal`, one of LING's modules). `cast/text.md`: add `failArguments` `wrong arguments`. Run cast (tracker, The contract of every part, cast runs).
3. In `Processor`, write the events map `events`, keyed by `juce::Identifier`, with one entry for each row of the table of The nine operations. The map type is END's `Events`: `jam::Function::Map<juce::Identifier, jam::Function::pointerCapacity, void (juce::ValueTree&)>` (`end/Source/config/ConfigModel.h:62`; `end/Source/plugins/Session.cpp:71-80`). Each entry takes the `REQUEST` row, calls the lookups of Steps 1 to 3, and writes the JSON text to `Id::result` of the row. An entry can thus also write its result later (Part 10). `Processor` owns one `jam::Fuzzy::Slab slab;`: every entry runs on the message thread, and the slab is "one per thread" (`jam_BonusMatrix.h:170-179`). The `workspaceSymbol` entry parses the query one time with `Fuzzy::Pattern::parse (query, Fuzzy::CaseMode::smart, true, Fuzzy::Scheme::standard)` and calls `pattern.match (id, slab)` for each declaration.
4. `Processor` reacts to a `REQUEST` row as section The nine operations gives it.
5. `Main.cpp`: a first argument that is no existing file is the operation word. The manifest is the argument after the word. `getToolchainArgument` (PLAN-ling-6-manifest.md Step 6 item 2) takes the index of the manifest. The arguments of the operation follow the manifest, or the toolchain argument when one is given. The application writes `Id::toolchainArgument` and `Id::manifest`, then adds the `REQUEST` row, with the word in `Id::command`, and reacts to `Id::result`. `Processor` judges the word: a word that `events` does not hold gives `failArguments` (section The nine operations). `Main.cpp` holds no list of the operation names.
6. Describe the nine operations in `HELP.md`.

**Validation:** No branch chain on the operation name: the events map selects. The nine names are the names of ruling 43, letter for letter. The application does not call a lookup; it writes a row and reacts to a property (`end/ARCHITECTURE.md:886-907`). `Processor` consumes the row that the event gives. The test for an empty word names its threat, the assert of `juce_Identifier.cpp:61` (MANIFESTO **B**, The Guard Rule), and it is in `Processor` only.


### Step 6: Build and check, macOS
**Scope:** build of LING; runs of `ling`; one scout manifest.
**Action:**
1. Build LING.
2. With END's manifest and the toolchain argument `--debug` (`ling <operation> ~/Documents/Poems/dev/end/project-info.md --debug <arguments>`), run each of these and read the result against the source files:
   - `goToDefinition` on a call to `addCommentToken` in `jam/jam_syntax_tree/document/jam_CodeDocument.cpp`: the declaration in `jam_CodeDocument.h` and the definition in `jam_CodeDocument.cpp`;
   - `findReferences` on the declaration of `addCommentToken` in `jam_CodeDocument.h`: the calls in `jam_CodeDocument.cpp`, and the definition is not among them;
   - `documentSymbol` on `jam_CodeDocument.h`: `jam`, then `CodeDocument`, then its members as `children`;
   - `workspaceSymbol` with the query `CodeDoc`: `CodeDocument` is in the result;
   - `incomingCalls` on `getVocabularyForFamily`: `getVocabulary` and `getToken` are in the result;
   - `goToImplementation` on `Document::build` in `jam_core/document/jam_Document.h`: the `build` members of the document types that derive from `Document`.
3. Run the first two of item 2 two times each. Compare the outputs byte for byte.
4. Compare `findReferences` on `CodeDocument::parse` with the LSP tool result for the same position (`jam_CodeDocument.h:40:25`). Record each difference with its cause.
5. Measure with scout: one benchmark for `goToDefinition` and one for `findReferences` on a common name, for example `get`. Record the medians.

**Validation:** Item 2 holds for each case. Item 3: the outputs are equal (MANIFESTO **D**). ARCHITECT reads item 4 against RFC ruling 4, "definition, jump, etc always correct", and item 5 against "ULTRA FAST".

### Step 7: Build and check, Windows
**Scope:** a Windows machine.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:** Build LING. Run item 3 of Step 6 on the Windows machine.
**Validation:** The outputs are equal between two runs (ruling 16).

## BLESSED Alignment

- **B — Bound.** `Processor` owns the documents. A lookup holds pointers to elements only for the length of one call.
- **L — Lean.** Three lookups serve nine operations. No index beside the trees.
- **E — Explicit.** A name with no declaration gives `[]`. Nothing is guessed.
- **S — SSOT.** Every lookup reads the documents (ruling 42 (d)). `applyToReferences` uses `applyToDeclarations`, so the two directions cannot disagree.
- **S — Stateless.** A lookup keeps nothing after it returns.
- **E — Encapsulation.** The data flows one way: a request row, the lookup, a result property, the client. The client acts on the JSON (ruling 21).
- **D — Deterministic.** Every result has a total order. Step 6 item 3 proves equal output for equal input.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open.

Facts for the review of the whole PLAN:

- **Each call reads the manifest and parses every file.** The time of one operation in this part is the time of Part 6 plus the lookup. Part 8 removes the parse from each call.
- **The cost of `findReferences`.** For a declaration that is not local, it walks every element of every document, and runs the scope walk for each reference with the same id. A common name, for example `get`, has many. Step 6 item 5 measures it. MANIFESTO permits a second structure only as "Coalescing or layout conversion at creation ... A measurement must show the gain."
- **A member reference gives every member with that name.** The reference data counts 171 symbols named `get` behind `JuceHeader.h`.
- **Overloads are one symbol**, and so are two declarations with one path in two files that never meet in one unit, for example two `static` functions with one name at the top level of two `.cpp` files.
- **`using namespace` opens no scope** for the scope walk.
- **`hover` gives no type text and no comment.** The tree holds the declaration, not the type of an expression. Part 9 reads the comment of a declaration for completion.
- **`goToDefinition` does not separate a declaration from its definition.** Both have one path, and the result holds both.
