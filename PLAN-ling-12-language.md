# PLAN: LING language layer as data, and parity operations

**RFC:** none. The objective comes from ARCHITECT's discussion of 2026-11-14.
**Date:** 2026-11-14
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8 / JAM. cast generates and builds.
**Status:** Locked 2026-11-14 by ARCHITECT's approval.

## Context

LING parses only the C family, through hand-written C++: `jam::CLanguageDocument` has 58 functions (`jam_CLanguageDocument.cpp:1-974`). `jam::SyntaxTree::getDocumentConstruction` holds the `cLang` entry only (`jam_SyntaxTree.cpp:37-40`).

`Processor` also encodes C-family meaning:
- `memberTypes` and `functionTypes` (`Processor.h:118,125`);
- the `classToken`, `structToken` and `base` tests in `applyToImplementations` and `applyToBases`.

ARCHITECT's direction, verbatim:
- *"ling designed as crossplatfrom, language agnostic graph"*
- *"the implementation pattern should not be hardcoded into ling engine"*
- *"ling carry C lang data table binary, to build its engine (as the default) OOTB first citizen"*
- *"those table data contract exacly like config-default-fallback-override contract … always copy to disk … disk always win"*
- *"engine redesign to be GENERAL, UNIVERSAL, hot reload runtime"*
- *"accepted criteria: runtime behaviour exactly identical like we have now"*
- *"is <language>Document class generatable? what if we use cast method to generate and compile ourselves? … dynamic library + header? then we could runtime?"*
- *"cast as deps is acceptable"*, and "no" to JAM/JUCE headers at the library build.
- *"after that we add missing features how to `speak` to compiler, in parity of what clangd can give"*

Rulings taken in this session:
- One PLAN, in this order: version reset, redesign, new operations, MACHINIST RFC.
- The command line does not change. The manifest selects the language (HELP.md:14).
- The built library lives in the cache folder.
- `CodeDocument` and the other families stay in JAM for now. ARCHITECT: *"keep it for now. could be migrated later"*.
- The fix-its arrive through `diagnostics`. No separate check run is added.

## Language / Framework Constraints

- **Tracker contract (tracker:40-52):**
  - JAM first, then JUCE.
  - `std::string_view`, `jam::Format` and `jam::Strings` for text.
  - Builds run through cast.
  - Both platform bodies are written from cited facts.
- **The generated library includes nothing from JAM or JUCE.** Its boundary is C functions only, as the ENAX boundary is (`jam_ENAXClientExtensions.cpp:67-81`, `enax_get_extension`).
- **CODING.md:** one object, one translation unit; no comments or doxygen before the audit.

## Dependency & API Inventory

| Need | Existing API | Evidence |
|---|---|---|
| Load a library, find a function | `juce::DynamicLibrary::open`, `getFunction<Fn>` | `juce_DynamicLibrary.h`; END `Nexus.cpp:135` |
| C boundary by name | `enax_get_extension (const char*)` → `const void*` | `jam_ENAXClientExtensions.cpp:67-81` |
| Copy a default when the file is absent; the disk wins | `ConfigTheme::saveToPath`, `ConfigModel::loadFromPath` | `end/Source/config/ConfigTheme.cpp:82-101`; `ConfigModel.cpp:70-113` |
| Watch a folder and reload | `jam::File::Watcher` | `LINGModel.cpp:15-16,137-159`; END `ConfigModel.cpp:125-136` |
| Run cast as a child process | `jam::Subprocess::launch (command, dir, onComplete)` | `jam_Subprocess.h:49-111`; `Processor.cpp:678-725` |
| Embedded files | `## binary` rows; BinaryData | `ling/project-info.md:181-188` (a JAM file embedded through `../jam/...`) |
| Cache folder | `LINGModel::getCacheDirectory` | `LINGModel.cpp:40-52` |
| Generated code from rows | `.cast` fences, `@template:fence` structure | `cast/SPEC.md` §6.3, §7; `jam/cast/code.cast:333-379` |
| Toolchain rows | `## toolchain` with `argument`, `command`, `flag` | `ling/project-info.md:197-215`; `cast/SPEC.md` §6.9 |
| Tokenizer | `Document::getTokens (text, vocabulary, start)`, `Vocabulary` | `jam_Document.h:782-789,1095` |
| Swap check | `SwapValidator` (`Id::version`) | `ling/Source/SwapValidator.cpp:3-16` |

## Design

- **Folders:**
  - `~/.config/ling/language/<language>/` holds the spell, the tables (`.md`) and the template (`.cast`). LING copies them from its BinaryData when they are absent, and the disk then wins.
  - `~/.cache/ling/language/<language>/` holds the generated source, the build folder and the library.
  - `<language>` is the family name, `cLang` (NAMES Rule 5; `syntax_tree.md:84`).
- **The library is the language.**
  - It exports one C function. That function returns, by name:
    - the lexical tables: 256 byte classes, the comment literals, the escape byte, and the keyword list;
    - the role of each declaration type;
    - the extension pairs;
    - the build function.
  - The build function takes the source bytes and the token records. It writes element records: parent index, type, name-token indices, comment-token indices, parameter-token indices, member flag, and the link indices of `to` and `in`.
- **JAM stays the one engine (SSOT).**
  - LING builds the `Vocabulary` and the keyword map from the library's lexical tables.
  - `Document::getTokens` tokenizes with them.
  - The library's build function returns records, and a new `jam_syntax_tree` document turns the records into `jam::Document` elements.
  - `CodeDocument` keeps its own C tables for highlighting, from the same `jam/cast/syntax_tree.md`, which LING embeds directly. Only the user's disk copy can differ, as an override.
- **The template is `CLanguageDocument`'s procedures.** Every literal and keyword test becomes a table lookup (MANIFESTO L), and the code is plain C++17. The output is the baseline by construction.
- **Hot reload, with no wait:**
  1. A change in the config folder starts cast through `jam::Subprocess`.
  2. The Completion opens the new library.
  3. Each `FILES` row gets a path change, so every file is parsed again.
- **Swap identity.** The swap root holds the library's hash beside `Id::version`, and `SwapValidator` checks both.

## Validation Gate

COUNSELOR validates each step before the next step. The checks are against MANIFESTO.md (BLESSED), NAMES.md, CODING.md and the locked PLAN, with no deviation and no scope drift. @Auditor runs ONCE, after the final step and after ARCHITECT says the runtime test may start. One cast build runs at each "Build" step only.

## Steps

### Step 1: Version 0.0.1
**Scope:** `ling/project-info.md:29` (`versionString`), `:74` (`buildVersion`); `Source/HELP.md:3,13`; `cast/installer/mac/distribution.xml:35`, `cast/installer/win/installer.nsi:22`.
**Action:**
1. Set each `0.1.0` to `0.0.1`.
2. Check whether cast generates the two installer files from `project-info.md`. If it does, let cast regenerate them and do not edit them by hand.
3. Build: `cast cast/spell.md --debug` in `ling/`.

**Validation:** `ling --version` prints `ling 0.0.1 (<commit>)`. A grep of `0.1.0` outside `Source/generated` gives no hit.

### Step 2: Baseline
**Scope:** the scratchpad `baseline/` folder only. No source file changes.
**Action:**
1. Remove the four swap roots.
2. Run `ling end/project-info.md --debug` and `ling ling/project-info.md --debug`, and keep both outputs.
3. Copy the four swap roots.
4. Run each operation at fixed positions, and keep each JSON. The positions:
   - `goToDefinition` and `findReferences` at `jam_CodeDocument.cpp:43:88` and `jam_CodeDocument.h:107:28`;
   - `hover`, `prepareCallHierarchy`, `incomingCalls` and `outgoingCalls` at the same two positions;
   - `documentSymbol` on `jam_CodeDocument.h`;
   - `workspaceSymbol` with `CodeDoc` and with `get`;
   - `goToImplementation` on `Document::build` (`jam_Document.h`);
   - `completion` at `jam_CodeDocument.cpp:43:88`;
   - on END's manifest (`end/project-info.md --debug`): `documentSymbol` on `end/Source/ui/TabView.cpp`, and `workspaceSymbol` with `TabView`.
5. Write the run script to the same folder, so that Step 7 repeats it exactly.

**Compare rule (correction at execution, 2026-11-14).** Step 4 deletes `jam_CLanguageDocument.*` and `jam_CLanguageValidator.*`, and it adds the new document files. All of them are in LING's own build (`ling/project-info.md:123`). END's build compiles no `jam_syntax_tree` file (PLAN-ling-7:205).
- Each END output is compared whole.
- Each LING output is compared after removing the rows and entries whose path is one of those changed files.

**Validation:** each file exists, and two runs of the script give byte-equal files.

### Step 3: The `cLang` language folder in the repository
**Scope:** new `ling/language/cLang/`: `spell.md`, the table files, and `language.cast`.
**Action:**
1. Move the procedures of `jam_CLanguageDocument.cpp:23-971` into `language.cast` as plain C++17, with no JAM or JUCE.
2. Turn every literal test into a table row:
   - the keyword constructions (`getStatementConstruction`, :920-951);
   - the punctuation constructions (:953-971);
   - the depth characters (`getDepthChange`, :104-119);
   - the boundary characters (`getBoundary`, :578-608);
   - the scope types and declaration types (:86-102, :709-736).
3. Embed the C keywords and byte classes from `jam/cast/syntax_tree.md` (`CppKeyword` :92-203, `code` :316-359) through `## binary` rows. Do not copy them.
4. Give the spell's `## toolchain` rows that generate the library source and build the shared library: cmake plus ninja, with platform rows as `c_language.md` already has them.

**Validation:**
- `cast language/cLang/spell.md` builds the library.
- The library includes no JAM or JUCE header.
- Every literal of the 58 functions is either a row or a named constant of the template.

### Step 4: The language document in `jam_syntax_tree`
**Scope:** `jam_syntax_tree`: the new document and its validator; `jam_SyntaxTree.h/.cpp`. Delete `jam_CLanguageDocument.h/.cpp` and `jam_CLanguageValidator.h/.cpp` first (Refactor-Rewrite Discipline).
**Action:**
1. The new document takes the library's lexical tables, runs `Document::getTokens`, calls the build function, and adds one element for each record.
2. Its validator keeps the five rules of `CLanguageValidator` (`jam_CLanguageValidator.cpp:5-87`). The declaration-type range comes from the library's roles.
3. `SyntaxTree::parse` takes the loaded language in place of the static `cLang` entry.

**Validation:**
- The compiler finds no remaining caller of the deleted types.
- No C-family literal remains in `jam_syntax_tree`.

### Step 5: LING loads the language
**Scope:** `LINGModel`, `Processor`, `Manifest`, `cast/files.md`, `cast/identifiers.md`, `project-info.md` (`## binary` rows for `language/cLang/*`).
**Action:**
1. On start, copy each embedded language file to `~/.config/ling/language/cLang/` when it is absent, in the `ConfigTheme::saveToPath` form (`ConfigTheme.cpp:82-101`).
2. Start cast on that spell through `jam::Subprocess`. The output folder is `~/.cache/ling/language/cLang/`.
3. In the Completion, open the library with `juce::DynamicLibrary`, and set it as the state that the parse jobs read.
4. Watch the config folder. A change runs cast again, and the next Completion reloads the library and sends a path change to each `FILES` row.
5. The swap root records the library hash, and `SwapValidator` checks it beside `Id::version`.

**Validation:**
- Event chain only, with no wait.
- One owner for the library.
- No C-family word in `LINGModel`, `Processor`, `Manifest` or `Main.cpp` (tracker:51).

### Step 6: Roles in place of C types
**Scope:** `Processor.h/.cpp`.
**Action:** replace these with the library's role table:
- `memberTypes` and `functionTypes` (`Processor.h:118,125`);
- `isDeclaration`'s range (`Processor.cpp:16-21`);
- the class and struct tests in `applyToImplementations` and `applyToBases`;
- the `base` test in `applyToDerived`.

**Validation:** `Processor` names no `map::SyntaxTokenType` C value.

### Step 7: Build and byte-equal check
**Action:**
1. Build.
2. Remove the four swap roots and the config and cache language folders.
3. Run the Step 2 script.
4. Compare each file with the baseline.
5. Run the script again on the written swaps.
6. Edit one row of `~/.config/ling/language/cLang/` that changes no result, for example the order of two keyword rows. The library rebuilds, and the outputs stay equal.

**Validation:**
- Every output and every swap file is byte-equal to the baseline, except each swap's version and library-hash fields.
- Hot reload runs with no restart of the daemon.

### Step 8: `switchSourceHeader`
**Scope:** `Processor`, `cast/identifiers.md`, `HELP.md`, the `cLang` extension-pair rows.
**Action:** one argument, a file. The result is the `FILES` rows whose path has the same stem and whose extension pairs with the file's extension under the language's pair rows. Each result is an entry with `path`. The order is the result order.
**Validation:** `jam_CodeDocument.h` gives `jam_CodeDocument.cpp`, and the reverse. Two runs are byte-equal.

### Step 9: `inlayHint`
**Scope:** `Processor`, `JsonWriter`, `cast/identifiers.md`, `HELP.md`.
**Action:** one argument, a file. For each `call` reference in the file, give one entry with the call's position, plus `parameters`: the parameter texts of its first declaration (`Id::parameters`, as `appendSnippet` reads them at `JsonWriter.cpp:17-45`).
**Validation:** a call to `addCommentToken` gives `cursor` and `vocabulary`. Two runs are byte-equal.

### Step 10: Fix-its through `diagnostics`
**Scope:** `cast/c_language.md` (`cLanguageCheckFlags mac`), `HELP.md` (the `diagnostics` row).
**Action:** remove `-fno-caret-diagnostics` and add `-fdiagnostics-parseable-fixits`. The output stays the compiler's lines, unchanged (ruling 49). The `windows` rows stay as they are.
**Validation:** a missing `;` prints an error line and a `fix-it:"<file>":{L:C-L:C}:";"` line.

### Step 11: Build and runtime checks
**Action:**
1. Build.
2. Repeat Step 7 items 2-5.
3. Run Steps 8-10 on the END and LING manifests.
4. Run Part 8 Step 7 items 2-8 again.

### Step 12: RFC for MACHINIST
**Scope:** `~/.config/RFC-ling-nvim.md` (rfc-writer skill).
**Action:** write the RFC with these contents:
- The nvim Lua adapter over `ling <operation> <manifest> [--<word>] <arguments>`.
- How each JSON result maps to the matching nvim result.
- The `diagnostics` errorformat (`build.lua:43`), plus the `fix-it:` lines for code actions.
- `switchSourceHeader` for `gh` and `cpp-stub.lua`.
- `inlayHint`.
- The clangd surfaces that still remain: `rename` and `documentHighlight` come from Lua over `findReferences`.
- The Claude Code harness entry: a skill, and a `PostToolUse` hook on `ling diagnostics`.

**Validation:** every claim cites a file:line.

## BLESSED Alignment

- **B:** one owner each. LINGModel owns the watcher, and one object owns the library. The `Subprocess` objects are owned as in `Processor.cpp:680,712`.
- **L:** the literal tests become rows (MANIFESTO L), and the template keeps one responsibility for each function.
- **E:** each failure prints a `text.md` message. A failed cast run sets the overlay message.
- **S:**
  - One tokenizer (`Document::getTokens`).
  - One copy of the C keywords in the repository (embedded from `jam/cast/syntax_tree.md`).
  - The library is derived from config and lives in the cache.
- **S (Stateless):** the Model holds the library state, and the parse jobs read it.
- **E (Encapsulation):** `Processor` asks for roles, not C types. C-family words leave the engine.
- **D:** the acceptance check is byte-equal output (Step 7).

## Names ratified at approval (NAMES Rule -1)

- **Folders:** `language/` under `~/.config/ling/` and `~/.cache/ling/`. The `ling/language/` folder in the repository.
- **The exported C function:** `ling_get_language (const char* name)`, in the form of `enax_get_extension`.
- **The template file:** `language.cast`.
- **The new `jam_syntax_tree` document type and its validator.** The names are taken from the family: `LanguageDocument`, `LanguageValidator`.
- **Identifiers:**
  - `switchSourceHeader` and `inlayHint`, named as the LSP methods, like the existing operations;
  - `language`;
  - `roles`;
  - `extensionPairs`.
- **Text rows:** `failLanguageBuild`, plus `failLanguageLoad` for a library that does not open.

## Risks / Open Questions

None.

## Verification

- Step 2 baseline against Step 7 and Step 11 outputs: byte-equal.
- Hot reload (Step 7, item 6).
- The new operations (Steps 8-10).
- The daemon lifecycle (Part 8 Step 7 items 2-8).
- ARCHITECT's runtime test in nvim, after Step 12.
