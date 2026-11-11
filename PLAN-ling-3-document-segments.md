# PLAN: LING Part 3 — Line Comments and Escapes in the `Document` Segments

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, ruling 37.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

`Document::getTokens` splits a source into segments before a language type reads it. It does not know a line comment, and it does not know an escape inside a quote. C source has both, so its segments are wrong.
This part gives `Document::Vocabulary` a line-comment literal and an escape byte, and `Document::getTokens` honours them (ruling 37). The C family vocabulary sets both. Every other vocabulary keeps its values, so no other document changes.
The work is in the JAM repository, `~/Documents/Poems/dev/jam/`. It comes after Part 2, so `CodeDocument` is in `jam_syntax_tree`.

## The defect, with evidence

| Source text | What `getTokens` does now | Where |
|---|---|---|
| `"\""` | Each quote byte toggles the quote state. The escaped quote closes the string. The third quote opens a new one. | `jam_core/document/jam_Document.cpp:385-386`, `:558-564` |
| `'"'` | Correct now: the quote state holds the opening byte, and another quote byte does not close it. | `jam_Document.cpp:560-563` |
| `// don't` | `/` is an operator byte. `'` opens a quote that runs to the next `'` in the file. | `Vocabulary` holds one comment pair, `jam_Document.h:784-785`; `map::code`, `cast/core.md:2426` |
| The same string, in `CodeDocument` | `addStringToken` stops at the first byte equal to the opening quote. It has no escape rule. | `jam_CodeDocument.cpp:90-103` |

After a wrong toggle, every later segment has the wrong type up to the next accidental toggle.

## Language / Framework Constraints

- MANIFESTO **D**: "The only correct fix is establishing the invariant once, at the owner". The owner of segmentation is `Document::getTokens`.
- MANIFESTO **E** (Explicit): all parameters are visible in the signature. The two changed private functions take the literals they read.
- CODING.md: no bail-out guard; `not`, `and`, `or`; brace initialization; no out-parameter; no magic value.
- CODING.md, one object, one translation unit: every body stays in `jam_Document.cpp` and `jam_CodeDocument.cpp`.
- NAMES.md Rule 5: each new name joins its nearest family (see Names).
- CAROL.md Code Hygiene: no comment and no doxygen in this sprint. The doxygen blocks of the changed declarations are stale after this part until the doxygen pass.
- `jam/.claude/CLAUDE.md:104`: "Agents never build. ARCHITECT builds." Steps 6 and 7 are ARCHITECT's builds, or COUNSELOR's on ARCHITECT's explicit request.
- The parallel `jam::Function::Map` session can change line numbers. Find each site by its text.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Every line number is a read of 2026-11-10, before Part 2 moves `CodeDocument`. Base path: `~/Documents/Poems/dev/jam/`.

### The engine

| Piece | Where |
|---|---|
| `struct Vocabulary`: `classes`, `commentOpen`, `commentClose`, `getClass`, `isPlainText` | `jam_core/document/jam_Document.h:780-799` |
| `Vocabulary::isPlainText`: `commentOpen.empty()` and every class is `map::Byte::text` | `jam_Document.cpp:141-145` |
| `Document::getTokens (text, vocabulary, start)`: the loop, the quote state, the depth | `jam_Document.h:1024`; `jam_Document.cpp:358-401` |
| `Document::addCommentToken (tokens, text, cursor, vocabulary)`, private static | `jam_Document.h:1131-1134`; `jam_Document.cpp:528-547` |
| `Document::isCommentOpen (text, cursor, vocabulary, insideQuoteOrRegion)`, private static | `jam_Document.h:1136-1139`; `jam_Document.cpp:549-556` |
| `Document::getQuote (currentQuote, character)`, private static | `jam_Document.h:1141`; `jam_Document.cpp:558-564` |
| `Document::parse()`: `getToken` returns a length; the segment index follows the cursor offset | `jam_Document.cpp:627-670` |
| The callers of `getTokens`: `Document::parse` only | LSP `findReferences` at `jam_Document.h:1024:30`: `jam_Document.cpp:358`, `:646` |
| The readers of `commentClose` outside the engine | LSP `findReferences` at `jam_Document.h:785:26`: `jam_web/css/jam_Css.cpp:139`; `jam_CodeDocument.cpp:81`, `:85`; `jam_web/html/jam_Html.cpp:90`, `:94` |

### Every place that builds a `Vocabulary`

Source: LSP `findReferences` at `jam_Document.h:780:12`, 70 references in 18 files. Each construction is an aggregate with three values: the table, `commentOpen`, `commentClose`.

| Type | Where |
|---|---|
| `CodeDocument`, four vocabularies | `jam_CodeDocument.cpp:172-173`, `:179`, `:185-186`, `:192` |
| `XmlDocument` | `jam_core/xml/jam_Xml.cpp:269` |
| `CssDocument` | `jam_web/css/jam_Css.cpp:920` |
| `HtmlDocument` | `jam_web/html/jam_Html.cpp:78` |
| `MarkdownDocument` | `jam_markdown/document/jam_MarkdownDocument.cpp:840` |
| `ReflowDocument` | `jam_document/document/jam_ReflowDocument.cpp:612` |
| `AnsiDocument` | `jam_terminal/document/jam_AnsiDocument.cpp:18-20` |
| `ImageDocument` | `jam_terminal/image/jam_ImageDocument.cpp:168` |

A new member with a default member initializer, placed after `commentClose`, leaves each of these aggregates valid and equal in behavior (C++17 aggregate initialization).

### Characters and literals

| Need | Existing piece | Where |
|---|---|---|
| The backslash | `Chars::backslash`, 0x5c | `cast/core.md:578` |
| The newline | `Chars::newline`, 0xa | `cast/core.md:633` |
| "No character" | `Chars::nullCharacter` | `jam_Document.cpp:560` |
| A comment literal as a token row | `cssCommentOpen` `/*`, `cssCommentClose` `*/`, `markupCommentOpen` `<!--` | `cast/core.md:794-795`, `:813` |
| A doubled character as a token row | `doubleDash` `--`, `doublePercent` `%%` ("Mermaid comment marker.") | `cast/core.md:798`, `:801` |
| `//` as a token row | none exists | `cast/core.md:779-838` |

### Names

| Name | Source |
|---|---|
| `Vocabulary::lineCommentOpen` | the family `commentOpen`, `commentClose` (`jam_Document.h:784-785`) |
| `Vocabulary::escape` | the word of the `## escape` map, "Character to the letter written after a backslash" (`cast/core.md:691-693`) |
| `Chars::doubleSlash` | the family `doubleDash`, `doublePercent` (`cast/core.md:798`, `:801`) |
| `Document::getEscapeLength` | the verb `get` (NAMES.md Verb Contract); sibling `getQuote`, `getDepth` (`jam_Document.h:1141-1143`) |

No other name is new.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: Add the `//` literal
**Scope:** `jam/cast/core.md`, `## tokens`; `jam/generated/jam_Core.h`.
**Action:**
1. In `## tokens`, add one row in name order, after `doublePercent`: name `doubleSlash`, value `` `//` ``, description `Line comment open.` Use the cell shape of the `doublePercent` row (`:801`).
2. Run cast on `jam/cast/spell.md` (`jam/.claude/CLAUDE.md:106-107`).

**Validation:** `generated/jam_Core.h` holds `doubleSlash` in `struct Chars`, in the form of `doublePercent`. No other generated line changed.

### Step 2: Give `Vocabulary` the two members
**Scope:** `jam/jam_core/document/jam_Document.h`, `struct Vocabulary`; `jam/jam_core/document/jam_Document.cpp`, `Vocabulary::isPlainText`.
**Action:**
1. After `commentClose`, add two members, in this order:
   - `std::string_view lineCommentOpen {};`
   - `juce::juce_wchar escape { Chars::nullCharacter };`
2. `isPlainText` returns true only when `lineCommentOpen` is empty also. Add `and lineCommentOpen.empty()` to the expression at `jam_Document.cpp:143`.

**Validation:** The two members have default initializers. No construction site in the inventory changed. `isPlainText` reads `commentOpen`, `lineCommentOpen`, and `classes`.

### Step 3: Make the comment functions take their literals
**Scope:** `jam_Document.h:1131-1139`; `jam_Document.cpp:528-556`.
**Action:**
1. `isCommentOpen` takes the opening literal in place of the vocabulary:
   `static bool isCommentOpen (const std::string& text, size_t cursor, std::string_view open, bool insideQuoteOrRegion) noexcept;`
   It returns `(not insideQuoteOrRegion) and not open.empty() and text.compare (cursor, open.size(), open) == 0`.
2. `addCommentToken` takes the two literals in place of the vocabulary:
   `static int addCommentToken (jam::Array<Token>& tokens, const std::string& text, size_t cursor, std::string_view open, std::string_view close);`
   The body is the body at `:533-546`, with `open` for `vocabulary.commentOpen` and `close` for `vocabulary.commentClose`.
3. Change no other function in this step. `getTokens` does not compile until Step 4.
4. Correction after the audit (ARCHITECT: "update plan"). `isCommentOpen` is a `protected` literal test. Its declaration follows `getVocabulary` (`jam_Document.h:1211`):
   `static bool isCommentOpen (std::string_view text, size_t cursor, std::string_view open) noexcept;`
   It returns `not open.empty() and text.compare (cursor, open.size(), open) == 0` (`jam_Document.cpp:559-562`). The caller tests `insideQuoteOrRegion`. Reasons:
   - `CodeDocument::addCommentToken` and `CodeDocument::addPunctuationToken` call it (Step 5). MANIFESTO **E**: "expose only when there is a proven external caller".
   - MANIFESTO **S**: "Check if functionality already exists before creating new implementations".
   - The CodeDocument scanners have no quote state, so the quote test stays at the caller in `getTokens`.

**Validation:** One function tests an opening literal. One function consumes a comment. Neither reads a `Vocabulary` (MANIFESTO **S**, SSOT: no second copy of the comment logic for the line comment).

### Step 4: Honour the line comment and the escape in `getTokens`
**Scope:** `jam_Document.h`, private section; `jam_Document.cpp:358-401`.
**Action:**
1. Declare, next to `getQuote` and `getDepth`:
   `static size_t getEscapeLength (const std::string& text, size_t cursor, char quote, const Vocabulary& vocabulary) noexcept;`
   `cursor` is the offset of the byte that the loop just read. The function returns 1 when all of these are true, and 0 otherwise:
   - `quote != Chars::nullCharacter`
   - `vocabulary.escape != Chars::nullCharacter`
   - the byte at `cursor` is equal to `vocabulary.escape`
   - `cursor + 1 < text.size()`
2. In the `getTokens` loop, the block comment test passes `vocabulary.commentOpen` and `vocabulary.commentClose` to the functions of Step 3.
3. Directly after it, add the same test for the line comment. It passes `vocabulary.lineCommentOpen` as the opening literal. The closing literal is the one byte `Chars::newline`, as a `std::string_view` over one `static constexpr char lineCommentClose { static_cast<char> (Chars::newline) };` inside `getTokens`, its one user (CODING.md, Scope and ownership of values, rule 2). `Chars::newline` is a `juce::juce_wchar` (`generated/jam_Core.h:301`), so a view of one byte needs this one-byte constant. The token type is `map::DocumentTokenType::comment`, as for the block comment.
4. After the loop reads `character` (`:379`) and before the quote state changes (`:385-386`), add `getEscapeLength (text, beforeChar, quote, vocabulary)` to `cursor`. The escaped byte is then inside the region segment and never reaches `getQuote`.
5. Keep every other line of the loop.
6. Correction after the audit (ARCHITECT: "update plan"):
   - `getEscapeLength` takes `juce::juce_wchar escape` in place of the vocabulary (`jam_Document.h:1143`). It reads only that member. Constraints, above: "The two changed private functions take the literals they read". MANIFESTO **E**: "All parameters visible in the function signature".
   - It returns `sizeof (char)` or `0`, and the next-byte test is `cursor + sizeof (char) < text.size()` (`jam_Document.cpp:580-588`). MANIFESTO **E**: "No magic values".
   - The two comment tests are two `if` blocks, block then line (`jam_Document.cpp:376-387`). Each one tests `(not insideQuoteOrRegion)` and then calls `isCommentOpen`. The one loop over the two literal pairs is removed. It added 18 lines against 11, a staged pair array, and three names that the Names table does not hold (`comments`, `comment`, `literals`). Names: "No other name is new".

**Validation:**
- With a default `Vocabulary` (empty `lineCommentOpen`, `escape` equal to `Chars::nullCharacter`), `getTokens` takes the same path as before for every input: `isCommentOpen` returns false for an empty literal, and `getEscapeLength` returns 0. This is the proof that the seven other document types do not change.
- No `if (not ...) return;` guard. No magic value: the newline is a named constant.
- `getTokens` is 54 lines (`jam_Document.cpp:359-412`). It was 44 lines before this part (`:358-401`). The Lean ladder (`auditor.md` Lean):
  1. No branch is extra.
  2. No lookup replaces the type ternary. `map::Byte` (`cast/core.md:213-217`) and `map::DocumentTokenType` (`:193-196`) do not map one to one, and the ternary has 3 outcomes. MANIFESTO **L**: "more than 3".
  3. The two `if` blocks are the simpler form.
  4. A split of the byte walk must give back `depth`, `quote`, and `cursor` together. That needs an out-parameter or a carrier struct. CODING.md forbids both: "No out-parameters", "No fake-carrier structs". `auditor.md` Lean: "A split that fails its own check is not a finding."

### Step 5: Set the C family vocabulary, and make `CodeDocument` read it
**Scope:** `jam/jam_syntax_tree/document/jam_CodeDocument.h` and `.cpp` (after Part 2).
**Action:**
1. `getCLangVocabulary` (`jam_CodeDocument.cpp:170-175`) builds its vocabulary with five values: `map::code`, `Chars::cssCommentOpen`, `Chars::cssCommentClose`, `Chars::doubleSlash`, `Chars::backslash`.
2. `addCommentToken (Cursor&, const Vocabulary&)` (`:76-88`): when `vocabulary.lineCommentOpen` is not empty and `cursor.startsWith (vocabulary.lineCommentOpen)`, the scan advances past the opening literal and then to the first `Chars::newline`, and consumes that newline when the cursor is not empty. In every other case the body is the body that exists now. Both cases return a `map::SyntaxTokenType::comment` token over the consumed bytes.
3. `addStringToken` takes the vocabulary, as its two siblings do: `static Document::Token addStringToken (Cursor& cursor, const Vocabulary& vocabulary);`. In its scan (`:96-97`), when `vocabulary.escape != Chars::nullCharacter` and `*cursor == vocabulary.escape`, the cursor advances past that byte and the next byte, when a next byte exists. The `region` entry of the `segments` table (`:36-37`) passes `vocabulary`.
4. Change no other vocabulary. Python, shell, markup, and data keep their three values.
5. Correction after the audit (ARCHITECT: "update plan"):
   - `addCommentToken` selects the line form with `isCommentOpen (cursor.source, cursor.getOffset(), vocabulary.lineCommentOpen)` (`jam_CodeDocument.cpp:77`). This is the test of Step 3, item 4.
   - In `addStringToken`, the escape byte advances the cursor once, and each pass then advances once when the cursor is not empty (`jam_CodeDocument.cpp:107-114`). The loop has no local flag.
   - `addPunctuationToken` also stops where `vocabulary.commentOpen` or `vocabulary.lineCommentOpen` opens (`jam_CodeDocument.cpp:126-130`). The defect: `/` is an operator byte (`cast/syntax_tree.md:324`). For `f();// note\nint y; /* a */`, the scanner took `();//`. `parse` then gave the comment segment to `getToken` in the middle of the comment, and the block branch consumed `int y;` up to the later `*/`. Each scanner stops on its own vocabulary literal. This is the established pattern (`jam_Css.cpp:141`, `jam_Html.cpp:90`, `jam_MermaidDiagram.cpp:60-62`; MANIFESTO **E**: "Find the established pattern. Extend it."). A cursor limited to its segment in `Document::parse` is not used. The XML and HTML markup scanners start on the `<` operator segment and read the whole tag (`jam_Xml.cpp:210-211`, `:408-441`), so that limit would change other document types.
   - `getToken` has no whitespace branch, and `getWhitespaceLength` is deleted (`jam_CodeDocument.cpp:57-65`). It added no token, and the default branch `singleByteAdvance` gives the same tokens. MANIFESTO **L**: "more than 3 `if/else` … Replace with direct lookup". `auditor.md` Lean, question 1: "Is the branch needed?"

**Validation:** The comment scanner and the string scanner consume exactly the bytes of the engine's segment for the same input, so `Document::parse` stays in step (`jam_Document.cpp:653-655`). NAMES.md: `addStringToken` has the parameter list of `addCommentToken` and `addPunctuationToken` (`jam_CodeDocument.h:84`, `:105`). No string compare with `==`: the literal test is `Cursor::startsWith` (`jam_Document.h:828`).

### Step 6: Build, macOS
**Scope:** builds of LING, END, cast, and scout.
**Action:** Build the four projects.
**Validation:** No error. END, cast, and scout show no change in their documents: no vocabulary of theirs sets the new members (Step 4, first validation line). The run-time proof of the C segments is in Part 4, where `CLanguageDocument` is the first reader of `CodeDocument`'s scanners on real source.
**Result (2026-11-11):** LING builds and runs. ARCHITECT: "ling 0.1.0 (c89f5d5)".

### Step 7: Build, Windows
**Scope:** builds of LING and END on a Windows machine.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:** Build both.
**Validation:** No error (ruling 16). `escape` is not in the Platform macro names table of CODING.md.

## BLESSED Alignment

- **B — Bound.** The engine owns segmentation. A language type owns its vocabulary values.
- **L — Lean.** Two members and one function are added. No new type. Only the C family sets the members; no other vocabulary gets a value before a reader needs it (YAGNI).
- **E — Explicit.** The comment functions take the literals they test. The line-comment close is a named constant.
- **S — SSOT.** One function tests a comment opening, and one function consumes a comment, for both comment forms.
- **S — Stateless.** `getTokens` keeps its transient loop state only.
- **E — Encapsulation.** The correction is in the parse, not in a consumer that repairs the segments ("the parse is wrong or the implementation is wrong").
- **D — Deterministic.** The invariant "a segment boundary is a true lexical boundary" is set once, at `getTokens`. No reader checks it again.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open.

Limits that stay after this part. Each is a fact for the review of the whole PLAN:

- **Raw string literals.** `R"(a"b)"` has no escape byte. The quote state toggles at the inner quote. One file of `jam_core`, `jam_web`, `jam_markdown`, `end/Source`, `scout/Source`, and `cast/Source` together holds the text `R"`.
- **Digit separators.** `1'000` (C++14) opens a quote at the apostrophe.
- **A backslash before a newline in a line comment.** The comment ends at the newline. The compiler continues it.
- **`//` in an include path.** `#include <a//b.h>` opens a line comment at `//`. `<` is an operator byte (`cast/syntax_tree.md:305`), not a quote.
- **Other vocabularies.** `CssDocument`, `HtmlDocument`, and `XmlDocument` have quote bytes and no escape value. They keep their present segments. Each owner sets `escape` when it needs it.
- **No run-time check in this part.** `CodeDocument::parse` has no caller (LSP `findReferences` at `jam_CodeDocument.h:40:25`: the declaration and the definition only). The first run on real source is in Part 4.
- **Doxygen.** The doxygen pass wrote the blocks on 2026-11-11: `jam_Document.h:776-806`, `:1010-1039`, `:1130-1171`, `:1239-1250`, and `jam_CodeDocument.h:47-63`, `:78-117`, `:144-153`. The JAM XML in `docs/xml` is stale until ARCHITECT runs `<leader>bd`. The zero-warning check follows that run.
