# PLAN: LING Part 4 — `CLanguageDocument`: Tokens and the Declaration Tree

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 22, 23, 25, 26, 28, 31, 32, 35, 39; RFC rulings 4 and 9. Ruling 80.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

This part adds `jam::CLanguageDocument` to `jam_syntax_tree`. It reads one C family source file and builds one tree. Each element of the tree is one declaration: its name, its type, and its position. An element is a child of the declaration that holds it.
The type derives from `CodeDocument`, so the scanners stay in one place. The tables get the rows that this type reads (ruling 35).
A last step lets `ling <file>` print the tree, so that the result can be read and compared.

## What this part does not do

- No link between elements: no reference, no caller, no base. Part 5.
- No manifest. One call parses one file, and one document holds one file (ruling 40). Part 6.
- No lookup across files and no query. Part 7.
- No second language. A second language is a second type and its rows (rulings 23 and 25).

## Language / Framework Constraints

- CODING.md, one object, one translation unit: `CLanguageDocument` is one header and one `.cpp`. MANIFESTO **L**: the 300-line threshold is a smell detector for responsibility count. The type has one responsibility: C family source to a declaration tree.
- MANIFESTO **L**, 3 branches: every choice by token type, keyword, byte, or scope type is a lookup (`jam::Function::Map`, a generated table), not a branch chain. NAMES.md Rule 8: a dispatch table is a noun.
- CODING.md: no `==` on strings. A keyword is known by its `map::CppKeyword` key. A punctuation byte is a `Chars` constant. A name compare between two spans is `std::string_view::compare (...) == 0` over the two views of the source (ruling 50).
- CODING.md: no out-parameter, no fake-carrier struct, no local struct, no anonymous namespace, no bail-out guard, `.at()` for indexed access, `not`/`and`/`or`.
- MANIFESTO **E**, Complete at Creation: the tree is complete when `build()` returns. `build()` can use work buffers and several passes. No reader changes the tree.
- NAMES.md Rule 6: `kind` and `node` are banned. The words are `Type` and `Element`.
- CAROL.md Code Hygiene: no comment and no doxygen in this sprint.
- `jam/.claude/CLAUDE.md:104`: "Agents never build. ARCHITECT builds."
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Line numbers are reads of 2026-11-10. Base path: `~/Documents/Poems/dev/jam/`. `CodeDocument` paths are the paths before Part 2 moves the files.

### The engine and the sibling types

| Need | Existing piece | Where |
|---|---|---|
| A document over a document | `struct ConfigDocument : MarkdownDocument` | `jam_markdown/document/jam_ConfigDocument.h:16` |
| The three hooks | `getVocabulary`, `getToken`, `build` | `jam_core/document/jam_Document.h:1203-1209` |
| A parse of a byte view | `MarkdownDocument::parse (std::string_view)` | `jam_markdown/document/jam_MarkdownDocument.h:47`; `jam_MarkdownDocument.cpp:23-26` |
| A view of a `juce::String` that a JUCE API returns | `jam::Strings::getView (const juce::String&)`, "a zero-copy view over a juce::String's UTF-8 bytes" | `jam_core/text/jam_Strings.h:620-633` |
| The row of an offset, 1-based | `Document::getLineNumber (uint32_t)` | `jam_Document.h:959` |
| The start offset of each row | `lineOffsets`, protected | `jam_Document.h:1214-1215` |
| An element and its properties | `Element::id`, `add<ValueType> (key, ...)`, `find`, `get (key, defaultValue)`, `contains` | `jam_Document.h:225-235`, `:335`, `:352`, `:373`, `:453` |
| The value types of an element property | `int`, `juce::String`, `Tokens`, `Element*`, `Elements`, and others | `jam_Document.h:192-206` |
| The column of an offset | none exists. `getLineNumber` is a binary search over `lineOffsets`. | `jam_Document.cpp:282-287` |
| Add a child | `Document::addChild (Element& parent, const juce::Identifier& id)` | `jam_Document.h:1027` |
| Find a name inside a scope | `Document::getChildByID (parent, id)` | `jam_Document.h:929` |
| The text of a token | `Document::getText (const Token&)` | `jam_Document.h:948` |
| A token property | `Token::add<ValueType> (key, ...)`, `Token::find`, `Token::get` | `jam_Document.h:113-181` |
| A walk over a subtree | `Element::applyFunctionRecursively` | `jam_Document.h:419-435` |
| Tokens to a tree by a table | `treeConstruction`, a `Function::Map` keyed by token type, with a stack of `Element*` | `jam_core/xml/jam_Xml.cpp:224-259` |
| A handler that does nothing | `skipToken` | `jam_Xml.cpp:687-690` |
| A prelude, then a block | `getPreludeEnd`: scan to the open brace or the semicolon. `getRuleBlock`: a depth counter to the matching close brace. | `jam_web/css/jam_Css.cpp:1685-1717` |
| Block contents by a table | `getBlockConstruction`, a `Function::Map` keyed by token type; each handler returns the next position; `getBlockContents` loops | `jam_Css.cpp:1896-1934` |
| Recursion for a nested block | `addRule` calls `getRules` for a nested rule | `jam_Css.cpp:1733-1735` |
| Mark bad input, do not stop | a property on the token | `jam_Css.cpp:1994-1998` |
| A keyword lookup from text | `map::AtRuleType::getInstance()->find (name)` gives the key | `jam_Css.cpp:1669` |
| Rules after `build()` (ruling 31) | `Document::Validator`, `Rules`, `isValid`, `getFailures`; siblings `XmlValidator`, `CssValidator` | `jam_Document.h:839-871`; `jam_core/xml/jam_XmlValidator.h`; `jam_web/css/jam_CssValidator.h` |
| A static lookup by an integer key, written in a `.cpp` | `static constexpr jam::LookupTable<int, int, 256> replacements { { { key, value }, ... } };`, inside the function that reads it. "Every slot not named in the constructor's entry list holds @p defaultValue"; "The first row's value is the table's default." | `jam_core/document/jam_Document.cpp:292-299`; `jam_core/utils/jam_LookupTable.h:39-40`, `:90-92` |
| Several values in one table slot | a `jam::Union` as the value of a `jam::LookupTable`: `{ MermaidOperator::dottedForward, jam::Union<uint8_t, uint8_t>::pack (MermaidEdgeStroke::dotted, true) }`; the capacity `MermaidOperator::invisible + 1`. "Packs 2-4 values of mixed trivially copyable types into a single uint32_t or uint64_t"; read by a structured binding, `auto [w, h] = size;` | `jam/generated/jam_Mermaid.h:1670-1680`; `jam_core/utilities/jam_Union.h:11`, `:21`, `:88-91`, `:197-211` |

### `CodeDocument` as it is

| Fact | Where |
|---|---|
| `tokens` and `family` are private | `jam_CodeDocument.h:74-75` |
| A keyword gets a token. Another identifier gets none. | `jam_CodeDocument.cpp:152-168` |
| A punctuation token is a run of operator bytes | `jam_CodeDocument.cpp:105-113` |
| `build()` moves the tokens to the root under `Id::tokens` | `jam_CodeDocument.cpp:71-74` |
| The keyword set of a family | `getKeywordsForFamily (family)`, a `jam::Bimap<int>` | `jam_CodeDocument.cpp:226-238` |
| `#` (0x23) is an operator byte in `map::code` | `cast/core.md:2426-2469` |

### Tables and identifiers

| Need | Existing piece | Where |
|---|---|---|
| The token type table | `## SyntaxTokenType`: `keyword` 0, `string` 1, `comment` 2, `number` 3, `punctuation` 4 | `cast/core.md:226-244` (in `cast/syntax_tree.md` after Part 2) |
| The C++ keywords with keys | `## CppKeyword`, 96 rows, keys 0 to 95: `tokenClass` 13, `tokenEnum` 31, `tokenNamespace` 45, `tokenOperator` 51, `tokenStatic` 64, `tokenStruct` 67, `tokenTemplate` 69, and others | `cast/core.md:268-377` |
| Identifiers | `Id::name` `:118`, `Id::type` `:152`, `Id::key` `:105`, `Id::tokens` `:147` | `cast/core.md` `## identifiers` |
| Punctuation bytes | `Chars::openBrace`, `closeBrace`, `openParen`, `closeParen`, `openBracket`, `closeBracket`, `lessThan`, `greaterThan`, `colon`, `comma`, `semicolon`, `equals`, `hash`, `tilde`, `asterisk`, `ampersand` | `cast/core.md` `## chars`, `:558-690` |

### The LING side

| Need | Existing piece | Where |
|---|---|---|
| A file argument in `Main.cpp` | scout's `runDocument`, `runDocumentNotFound`, `getDocumentFile` | `scout/Source/Main.cpp:158-172`, `:220-252` |
| An error line | `printError`, and a `cast/text.md` row | PLAN-ling-1-project.md Steps 6 and 9 |
| Key to name | `jam::Bimap`: "a bidirectional key↔literal registry via getInstance()" | `jam/cast/spell.md:231` |

### Names

| Name | Source |
|---|---|
| `CLanguageDocument` | ruling 26, verbatim |
| `CLanguageValidator` | the family `XmlValidator`, `CssValidator` |
| Row names `identifier`, `function`, `instanceMethod`, `staticMethod`, `constructor`, `destructor`, `variable`, `staticVariable`, `field`, `enumConstant`, `macro` | the measured type list (PLAN-ling-0-tracker.md, Reference data), in camelCase |
| Row names `namespaceToken`, `classToken`, `structToken`, `enumToken` | ruling 39 |
| `tokenDefine` | the `CppKeyword` row family, `tokenClass` (`cast/core.md:294`) |
| `Document::getColumnNumber` | the sibling `Document::getLineNumber` (`jam_Document.h:959`) |
| `statementConstruction`, `getStatementConstruction`, `punctuationConstruction`, `getPunctuationConstruction` | `blockConstruction`, `getBlockConstruction` (`jam_Css.cpp:1896-1915`) |
| `getPreludeEnd`, `getBlockEnd` | `getPreludeEnd`, `getRuleBlock` (`jam_Css.cpp:1685`, `:1695`) |
| `addStatements`, `addNamespace`, `addClass`, `addEnum`, `addControl`, `addDirective`, `addDeclaration`, `addParameters`, `addElement`, `skipStatement` | the `add*` family and `skipToken` (`jam_Xml.cpp:687-746`; `jam_Css.cpp:1885`) |
| `declarationTypes` | NAMES.md Rule 8, a plain noun for what the table holds |
| `functionType`, `staticFunctionType`, `variableType`, `staticVariableType`, `scopeType` | ruling 80: the form that ARCHITECT selected, verbatim |
| `isFunctionShape`, `hasStaticKeyword` | NAMES.md Rule 1, the boolean forms `is` and `has`; the shape of Declarations item 4; the keyword row `tokenStatic` |
| `runDocument`, `getDocumentFile`, `runDocumentNotFound` | scout, verbatim |

## The design

### The tokens

`CLanguageDocument` reads these token types from the scanners of `CodeDocument`:

| Type | Content |
|---|---|
| `keyword` | a word in the family keyword set. The token holds the keyword key as an `int` property under `Id::key`. |
| `identifier` | a word that is not a keyword |
| `punctuation` | one operator byte per token |
| `string`, `number`, `comment` | as now |

### The element of a declaration

One document holds one file (ruling 40). The tree comes from the bytes of that file only.

| Part | Value |
|---|---|
| `Element::id` | the declared name as a `juce::Identifier`. An operator function has the id `operator`. A destructor has the id of its class. |
| `Id::type` | `int`, a `map::SyntaxTokenType` key |
| `Id::name` | `Tokens`: the identifier tokens of the name, in source order. A qualified name `Foo::bar` gives two tokens. The last token is the declared name. |
| parent | the element of the declaration that holds this one: a namespace, a class, an enum, or a function. A top-level declaration is a child of the root. |

The position of a declaration is the span of its last `Id::name` token. A reader gets the row from `Document::getLineNumber` and the column from `Document::getColumnNumber`. No property repeats them (MANIFESTO **S**, SSOT). The file is the key under which the owner holds the document; no element holds a path.

### The walk

`build()` walks the tokens as statements. A statement is a prelude and then a terminator, as a CSS rule is (`jam_Css.cpp:1685-1717`).

- The prelude runs to the first `;` or `{` that is outside every `(` `)` and `[` `]` pair.
- A `{` terminator has a block. The block runs to the matching `}`.
- Comment tokens are not part of the walk. `build()` makes one work array of the tokens that are not comments, and walks that array.

The first token of a statement selects the handler, through the table `statementConstruction` for a keyword and the table `punctuationConstruction` for a punctuation byte:

| First token | Handler | Result |
|---|---|---|
| punctuation `;` | `skipStatement` | next position |
| punctuation `{` | `addStatements` | inside a function: the block is walked in the same scope. In another scope: the block is skipped. |
| punctuation `#` | `addDirective` | see Directives |
| keyword `tokenTemplate` | (prefix) | when `<` follows, the walk skips to the matching `>` by a depth count of `<` and `>`. The token after it selects the handler. |
| keyword `tokenNamespace` | `addNamespace` | one `namespaceToken` element for each name of `A::B`, nested. The block is walked in namespace scope. A statement with no block makes no element. A namespace with no name is walked in the scope that holds it. |
| keyword `tokenClass`, `tokenStruct` | `addClass` | with a block: one `classToken` or `structToken` element. Its name is the last identifier before the first single `:` or the `{`. The block is walked in class scope. The walk then skips to the `;`. With no block: no element. |
| keyword `tokenEnum` | `addEnum` | one `enumToken` element, named by the first identifier after `enum` and an optional `class` or `struct`. In the block, the first identifier, and each identifier directly after a `,` outside brackets, is one `enumConstant` child. An enum with no name makes no element, and its constants are children of the scope. |
| keyword `tokenExtern` | `addStatements` or `addDeclaration` | a string token and then a block: the block is walked in the same scope. Otherwise: a declaration. |
| keyword `tokenPublic`, `tokenPrivate`, `tokenProtected` | `skipStatement` | the keyword and the `:` after it are skipped |
| keyword `tokenIf`, `tokenFor`, `tokenWhile`, `tokenSwitch`, `tokenCatch` | `addControl` | the `(` `)` group is read as one declaration candidate, up to its first `;` or single `:`. The walk continues after the `)`. |
| keyword `tokenElse`, `tokenDo`, `tokenTry` | `skipStatement` | the keyword alone is skipped |
| keyword `tokenReturn`, `tokenThrow`, `tokenBreak`, `tokenContinue`, `tokenGoto`, `tokenDelete`, `tokenCase`, `tokenDefault`, `tokenUsing`, `tokenTypedef`, `tokenFriend`, `tokenStatic_assert`, `tokenUnion` | `skipStatement` | the statement is skipped to its `;`, with every block in it |
| anything else | `addDeclaration` | see Declarations |

`tokenStatic_assert` and each other key are the names of the `## CppKeyword` rows. The executor reads the exact row names there.

### Declarations

`addDeclaration` reads the prelude outside brackets:

1. **The boundary** is the first of: a single `=` that is not next to `=`, `!`, `<`, or `>`, and that does not follow the keyword `operator`; the first `(`; the first `[`; the end of the prelude.
2. **The name** is the last identifier before the boundary. When the keyword `operator` is before the first `(`, the name is that keyword token. Identifiers joined to the name by `::` before it are its qualifier.
3. **The declaration test.** The statement is a declaration when the token before the name and its qualifier is an identifier, a keyword, or one of the bytes `*`, `&`, `>`. In class scope it is also a declaration when the name is equal to the class id (a constructor), or when the byte `~` is before the name (a destructor). In namespace scope the same two cases apply when the last qualifier is equal to the name. A statement that fails the test makes no element and is skipped to its `;`, with every block in it.
4. **The shape** is `function` when the boundary is a `(`. Otherwise it is `variable`.
5. **The type.** Three cases come before the table, in this order. A constructor case of item 3 gives `constructor`. A destructor case gives `destructor`. A function shape with a qualifier, in the root or in a `namespaceToken` scope, gives `instanceMethod`. Each other declaration takes its type from the table `declarationTypes` (ruling 80). The key is the scope type. The value holds four types, in this order:

| Scope type, the key | `functionType` | `staticFunctionType` | `variableType` | `staticVariableType` |
|---|---|---|---|---|
| `namespaceToken`; the root reads as `namespaceToken` (Step 5 item 5) | `function` | `function` | `variable` | `staticVariable` |
| `classToken`, `structToken` | `instanceMethod` | `staticMethod` | `field` | `staticVariable` |
| `function`, `instanceMethod`, `staticMethod`, `constructor`, `destructor` | `variable` | `staticVariable` | `variable` | `staticVariable` |

   The read is `const auto [functionType, staticFunctionType, variableType, staticVariableType] { declarationTypes[scopeType] };`, a structured binding of the `jam::Union` (`jam_Union.h:21`, `:197-211`). `isFunctionShape` selects the pair: it is true for the shape `function` of item 4. `hasStaticKeyword` selects the member of the pair: it is true when a keyword token of the prelude, outside brackets, holds `Id::key` equal to `map::CppKeyword::tokenStatic`. The read thus holds two conditions and no chain on the scope type. The first entry of the table is the `namespaceToken` entry, and its value is the value of each slot that no entry names (`jam_LookupTable.h:39-40`, `:90-92`). The walk makes a declaration in these eight scope types only: an enum block gives constants, the block of a variable is not walked (item 9), and a parameter is a `variable` by item 7.
6. **More declarators.** For a variable shape, each `,` outside brackets, followed by zero or more `*` or `&` bytes and then an identifier, gives one more element of the same type.
7. **Parameters.** For a function shape outside a function scope, `addParameters` reads the `(` `)` group. Each part between `,` bytes outside brackets is one declaration candidate with the boundary and name rules of items 1 and 2 and the test of item 3. Each one that passes is a `variable` child of the function element.
8. **The body.** For a function shape with a `{` terminator, the block is walked in the scope of the function element, so each local is a child of it. When a single `:` follows the parameter group (a constructor initializer list), the body is the last block of the chain: after a block, a `,` continues the list and the next block is read; a `{` with no `,` before it is the body.
9. **A variable with a block.** For a variable shape with a `{` terminator (a brace initializer or a lambda), the walk skips to the `;` outside every block. The block is not walked.

### Directives

A `#` token starts a directive. The directive holds every token on the row of the `#` (`Document::getLineNumber` of each token offset).

- `#` then the keyword `tokenDefine` then an identifier: one `macro` element, a child of the scope, named by that identifier.
- Every other directive makes no element. No part reads `#include`: the files of a unit come from the dependency record of the build (PLAN-ling-6-manifest.md, In LING: the compiler manifest, The files).

### The rules after `build()` (ruling 31)

`CLanguageValidator : Document::Validator` holds these rules. Each rule walks the tree and collects failures with `Validator::getFailures`:

| Key | Rule |
|---|---|
| `type` | every element below the root has `Id::type` as an `int`, and the value is a `map::SyntaxTokenType` key from `namespaceToken` to `macro` |
| `name` | every element below the root has `Id::name` as `Tokens` with one token or more, and each span is inside the source |

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: Add the rows
**Scope:** `jam/cast/syntax_tree.md` (`## SyntaxTokenType`, `## CppKeyword`); `jam/generated/`.
**Action:**
1. `## SyntaxTokenType`: add fifteen rows after `punctuation`, with these keys: `identifier` 5, `namespaceToken` 6, `classToken` 7, `structToken` 8, `enumToken` 9, `enumConstant` 10, `function` 11, `instanceMethod` 12, `staticMethod` 13, `constructor` 14, `destructor` 15, `variable` 16, `staticVariable` 17, `field` 18, `macro` 19.
2. `## CppKeyword`: add one row after key 95: `tokenDefine`, 96, `` `define` ``.
3. Run cast on `jam/cast/spell.md`.

**Validation:** `generated/jam_SyntaxTree.h` holds the fifteen new enum values and the one new keyword value. No existing key changed. No table was added (ruling 35). NAMES.md: each row name is in the Names table. No row name is a C++ keyword or a name of the Platform macro names table (CODING.md).

### Step 2: Make `CodeDocument` give the tokens that a tree needs
**Scope:** `jam/jam_syntax_tree/document/jam_CodeDocument.h` and `.cpp`.
**Action:**
1. Move `tokens` and `family` from `private` to `protected`.
2. `addIdentifierToken` (`jam_CodeDocument.cpp:152-168`): look the word up once in `getKeywordsForFamily (family)`. When it is a keyword, add a `map::SyntaxTokenType::keyword` token and add its key to the token: `add<int> (Id::key, key)`. When it is not, add a `map::SyntaxTokenType::identifier` token.
3. `addPunctuationToken` (`:105-113`): the token holds one operator byte. The cursor advances by one byte.
4. Change no other scanner.

**Validation:** One lookup per word (MANIFESTO **S**, SSOT: the lexer finds the keyword once, and `build()` reads `Id::key`). `CodeDocument` has no caller today (PLAN-ling-2-language-tables.md, inventory), so no reader of the old token stream exists. CODING.md: no friend; the derived type reaches the two members through `protected`, as `ConfigDocument` reaches `MarkdownDocument`.

### Step 3: Add the type, the parse entry, and the column of an offset
**Scope:** new files `jam/jam_syntax_tree/document/jam_CLanguageDocument.h` and `.cpp`; `jam/user-modules-info.md`; `jam/jam_core/document/jam_Document.h` and `.cpp`.
**Action:**
1. `user-modules-info.md` `## submodule`: add two `jam_syntax_tree` rows after the `jam_CodeDocument` rows: `document/jam_CLanguageDocument.h` with aggregator `jam_syntax_tree.h`, and `document/jam_CLanguageDocument.cpp` with aggregator `jam_syntax_tree.cpp`, class `kernel`. Run cast, so the module header and source include the two files.
2. Declare `struct CLanguageDocument : CodeDocument` with:
   - `static CLanguageDocument parse (std::string_view sourceText);`
   - `protected: void build() override;`
3. `parse (std::string_view)` makes a document, sets `family` to `map::Family::cLang`, calls `Document::parse (data, size)` (the form of `jam_MarkdownDocument.cpp:25-26`), and returns the document.
4. `build()` is empty in this step. In no step does it call `CodeDocument::build()`: that body moves the tokens to the root under `Id::tokens` (`jam_CodeDocument.cpp:71-74`), and the tree of this type holds no token list (PLAN-ling-11-swap.md, fact 4).
5. In `Document`, next to `getLineNumber` (`jam_Document.h:959`), add `uint32_t getColumnNumber (uint32_t byteOffset) const noexcept;`. It returns the 1-based byte column of the offset in its row: the offset minus the start offset of that row in `lineOffsets`, plus 1. It finds the row as `getLineNumber` does (`jam_Document.cpp:282-287`).
6. The two new files hold no include (CODING.md, HEADER DISCIPLINE).

**Validation:** The files are named in the manifest, and cast wrote the two include lines. The type derives from `CodeDocument` and adds no scanner. `parse` has the byte-view form of `MarkdownDocument::parse` only. A caller that holds a `juce::String` from a JUCE API passes `jam::Strings::getView` of it (ruling 50). `getColumnNumber` allocates nothing and reads `lineOffsets` only, as its sibling does.

### Step 4: The statement walk and the scope statements
**Scope:** `jam_CLanguageDocument.h` and `.cpp`.
**Action:** Write the walk of section The walk.
1. `build()` makes the work array of tokens without comments, then calls `addStatements` for the whole array with the root as the scope.
2. `addStatements (scope, start, end)` loops: it reads the first token at the position, gets the handler from `getStatementConstruction()`, and continues at the position that the handler returns. This is the loop of `getBlockContents` (`jam_Css.cpp:1918-1934`).
3. Two static tables hold the handlers. `getStatementConstruction()` returns `statementConstruction`, keyed by the `Id::key` of a keyword token. `getPunctuationConstruction()` returns `punctuationConstruction`, keyed by the byte of a punctuation token (`Chars::semicolon`, `Chars::openBrace`, `Chars::hash`). The entries of both tables have one signature, declared in the map type (CODING.md, Uniform signature in the type). `addStatements` selects the table by the token type: `keyword`, `punctuation`, or any other type. A token that has no entry goes to `addDeclaration`. The map type is `jam::Function::Map<int, jam::Function::pointerCapacity, int (CLanguageDocument&, Element&, const jam::Array<Document::Token>&, int, int)>`: the document, the scope, the work array, the position, and the end of the range. A handler returns the next position. This is the form of `getBlockConstruction`, whose handlers are static members with the parameter list of the map after the key (`jam_web/css/jam_Css.cpp:1895-1914`). The loop calls `contains` and then `get`, as `getBlockContents` does (`jam_Css.cpp:1927-1929`): `get` throws for an absent key (`jam_core/function_map/jam_Function.h:229-234`).
4. Write `getPreludeEnd` and `getBlockEnd` with the depth counts of `jam_Css.cpp:1685-1717`.
5. Write `skipStatement`, `addNamespace`, `addClass`, `addEnum`, and the `tokenTemplate` prefix and `tokenExtern` cases, each as the table in section The walk gives it.
6. Write `addElement (scope, nameTokens, type)`: it calls `addChild (scope, id)` with the text of the last name token, then adds `Id::type` and `Id::name`.
7. Every element of this step is made by `addElement`.

**Validation:** Each handler returns a position greater than the position it received, so the loop ends. No handler holds a chain of more than three branches on a token type, a keyword, or a byte. No string `==`. `addElement` is the only place that writes the two properties (SSOT). The names are the names of the Names table.

### Step 5: Declarations, parameters, bodies, and control statements
**Scope:** `jam_CLanguageDocument.h` and `.cpp`.
**Action:**
1. Write `addDeclaration` as section Declarations gives it, items 1 to 6, 8, and 9.
2. Write `declarationTypes` from the table in item 5: `static constexpr jam::LookupTable<int, jam::Union<uint8_t, uint8_t, uint8_t, uint8_t>, map::SyntaxTokenType::macro + 1> declarationTypes`. It is inside the one function that reads it (CODING.md, Scope and ownership of values), in the form of `replacements` (`jam_core/document/jam_Document.cpp:292-299`). The capacity has the form `MermaidOperator::invisible + 1` (`jam/generated/jam_Mermaid.h:1670`). Each entry is `{ <scope type>, jam::Union<uint8_t, uint8_t, uint8_t, uint8_t>::pack (<the four types of its row>) }`, in the form of `jam_Mermaid.h:1672`. The table has eight entries, one for each scope type of item 5, and the `namespaceToken` entry is first. Write the read as item 5 gives it, after the three cases. The table is hand-written: ruling 35 adds no table to cast.
3. Write `addParameters` as item 7 gives it.
4. Write `addControl` as the table in section The walk gives it. It calls the declaration candidate logic of `addDeclaration` on the range inside the `(` `)` group.
5. The scope type of an element is its `Id::type`. The root has none: read it with `get<int> (Id::type, map::SyntaxTokenType::namespaceToken)` (`jam_Document.h:335`).

**Validation:** Each scope type of the `declarationTypes` table in this plan is one entry of the code table, with its four types in the column order. The three cases of item 5 come before the table lookup. The read holds two conditions and no chain on the scope type. A statement that fails the declaration test makes no element. MANIFESTO **L**: a function over 30 lines is split only where a part has its own responsibility (for example the boundary scan, the name scan, the declaration test).

### Step 6: The `#define` directive
**Scope:** `jam_CLanguageDocument.h` and `.cpp`.
**Action:** Write `addDirective` as section Directives gives it. Add its entry to `punctuationConstruction` under the `Chars::hash` key.
**Validation:** A `#define NAME` row gives one `macro` element named `NAME`. Every other directive gives none, and the walk continues on the next row. The keyword is known by `Id::key` equal to `map::CppKeyword::tokenDefine`, not by its text.

### Step 7: The rules
**Scope:** new files `jam/jam_syntax_tree/document/jam_CLanguageValidator.h` and `.cpp`; `jam/user-modules-info.md`.
**Action:**
1. Add the two files to `## submodule`, as Step 3 item 1 does, and run cast.
2. Write `struct CLanguageValidator : Document::Validator` in the form of `XmlValidator` (`jam_core/xml/jam_XmlValidator.h`). `getRules()` returns a static `Rules` table with the two rules of section The rules after `build()`.

**Validation:** The validator reads a `const Document&` and changes nothing. Each rule returns `Validator::getFailures` of what it collected. Construction stays in `build()` (ruling 31).

### Step 8: Print the tree from `ling <file>`
**Scope:** `ling/Source/Main.cpp`; `ling/cast/text.md`; `ling/Source/HELP.md`.
**Action:**
1. Add scout's `getDocumentFile`, `runDocument`, and `runDocumentNotFound` to `Main.cpp`, with a branch for one file argument in `runCommandLine` (`scout/Source/Main.cpp:158-172`, `:220-252`). Add the text row that `runDocumentNotFound` prints, with scout's row as the form.
2. `runDocument (file)`:
   - reads the file text with `juce::File::loadFileAsString()`, the load of scout and cast (`scout/Source/Processor.cpp:13`; `cast/Source/Model.cpp:8`), and calls `jam::CLanguageDocument::parse (jam::Strings::getView (text))`. The `juce::String` is a JUCE return (ruling 50);
   - calls `jam::CLanguageValidator().isValid (document)`; on a failure it prints the message with `printError` and returns 1;
   - walks the tree with `applyFunctionRecursively` and prints one row per element below the root to stdout: `<path>:<line>:<column>: <type name> <name>`. `<path>` is the path of the file argument. `<line>` and `<column>` are `getLineNumber` and `getColumnNumber` of the offset of the last `Id::name` token. `<type name>` is the `map::SyntaxTokenType` value of `Id::type`. `<name>` is the element id;
   - returns 0.
3. Add the file argument to `HELP.md`.

**Validation:** `Main.cpp` holds no language name other than the one call to `CLanguageDocument::parse`. Part 6 replaces that call with the manifest (ruling 22). The print reads the tree and changes nothing.

### Step 9: Build and check, macOS
**Scope:** builds of LING and END; runs of `ling`.
**Action:**
1. Build LING and END.
2. Run `ling ~/Documents/Poems/dev/jam/jam_syntax_tree/document/jam_CodeDocument.h`. Compare the output with the declarations of that file: one `namespaceToken` `jam`, one `structToken` `CodeDocument` inside it, and inside that the methods and fields that the header declares (`jam_CodeDocument.h:25-208`).
3. Run `ling ~/Documents/Poems/dev/jam/jam_syntax_tree/document/jam_CodeDocument.cpp`. Each function definition is one `instanceMethod` row with the id of the function. Each parameter and each local is one `variable` row.
4. Run `ling ~/Documents/Poems/dev/end/Source/ui/SessionView.cpp` two times. Compare the two outputs byte for byte.
5. Run `ling` on a file that holds `"\""`, `'"'`, and `// don't`. This is the run-time check of Part 3.

**Validation:** Items 2 and 3 match the files. Item 4: the two outputs are equal (MANIFESTO **D**). Item 5: the declarations after the three texts are present. ARCHITECT reads the output of item 4 against the file.

### Step 10: Build and check, Windows
**Scope:** the build and the runs of Step 9 on a Windows machine.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:** Build LING. Run items 2 and 4 of Step 9.
**Validation:** The outputs are equal to the macOS outputs, except for the path text (ruling 16).

## BLESSED Alignment

- **B — Bound.** The document owns every element. `build()` is the only writer.
- **L — Lean.** One type, one responsibility. Each choice is a table. No element type outside ruling 32 is made.
- **E — Explicit.** A keyword is a key. A byte is a named constant. A statement that the walk does not know makes no element; it does not make a guessed one.
- **S — SSOT.** The scanners are in `CodeDocument` only. `addElement` is the one writer of the position properties. The tables are the one source of the vocabulary.
- **S — Stateless.** The work array and the positions live inside `build()`.
- **E — Encapsulation.** The tree is complete when `build()` returns. `ling` holds no C++ rule: it calls `parse` and reads the tree.
- **D — Deterministic.** The walk has no heuristic that depends on order of execution or on another file. Step 9 item 4 proves equal output for equal input.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open.

Limits of the table front end. Ruling 28 makes this front end the MVP, and a declared front end library is the path to full correctness. Each limit is a fact for the review of the whole PLAN:

- **A macro call with no `;`.** `JUCE_LEAK_DETECTOR (Foo) int x;` is one statement for the walk. It fails the declaration test, so `x` makes no element. A macro call that is the last statement of a class body costs nothing.
- **A macro that expands to a declaration** makes no element. The walk does not expand macros.
- **A lambda body and a brace initializer** are not walked. Locals inside a lambda make no element.
- **A method that is defined outside its class** has the type `instanceMethod`. The walk cannot see `static` there. Part 5 links the definition to its declaration, which has the true type.
- **`a * b;` in a function body** passes the declaration test and gives a `variable` named `b`.
- **`union`** makes no element, and its body is skipped. `union` is not one of the types of ruling 32.
- **A type alias (`using`, `typedef`)** makes no element. It is not one of the types of ruling 32.
- **A template parameter** makes no element.
- **A `#define` with a `\` at the end of a row** continues on the next row for the compiler. The walk ends the directive at the end of the row.
- **The limits of Part 3** stay: raw string literals and digit separators.
- **Locals and parameters are in the tree.** The basis is RFC ruling 9, "our own COMPLETE AST", and its uses "definition, jump, snippet, completion" (tracker Canon, From the removed RFC). They have the type `variable`: ruling 32 gives no parameter type.
- **`CodeDocument` token changes.** A punctuation token is now one byte, and an identifier now has a token. A future highlighter reads the new stream.
- **`define` and `include` are keyword rows** of the C family after Step 1. An identifier with one of these two names gets a keyword token, not an identifier token.
