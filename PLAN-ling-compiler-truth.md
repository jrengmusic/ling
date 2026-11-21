# PLAN: ling answers from the compiler only

**Date:** 2026-11-20
**BLESSED Compliance:** verified per step, except the preconditions named in steps 3 and 4
**Language Constraints:** C++17 / JUCE / JAM engine (`Source/`); cLang library as `language/cLang/language.cast` + `construction.md` + `spell.md` (cast); test data in `test/`

## Context

ARCHITECT, this session:
- *"no guess. why guess? it does not make any sense."*
- *"compiler knows exactly what to compile"*, *"we have the complete index, complete AST"*, *"all are deterministic"*.
- *"what's the most BLESSED approach to solve our problems?"* / `/goplan`.
- On the conflict between the Part 2 ruling *"Scope answer, name-indexed"* (`PLAN-ling-lsp-0-tracker.md:131`) and ruling 35 *"ling MUST NOT GUESSED. but GET the accurate read"* (`:111`): selected *"Ruling 35 governs"*. Option text: "Delete the name fallback (Query.h:298-331, Index::names). A skipped branch gets its answer from the compiler: the unit is indexed again under the platform context whose branch holds the line, as clice merges other compile contexts (write.cpp:856-880). The platform context source is designed in the plan."

The reference survey (clangd, ccls, clice) agrees on one design. The compiler's symbol binds a name. A request does not wait for unrelated indexing. Each of the three answers code that a context skips only from a real compile context. ling breaks this design in four places:

| Defect | Evidence | Rows |
|---|---|---|
| Name fallback | `Query.h:330-331` → `applyToScopeDeclarations` `:298-317` → `Index::names` (`Index.cpp:451-459`) | T14 `folder` 157:33 (104, 264, 385); the `owner` row at 260 passes only by accident |
| Whole-index wait | `cast/readiness.md:16` and its other `Id::files` rows; `Processor.cpp:229`, `:347` | the 60 s Claude Code timeout; END hover over 420 s |
| Overload set in a template body | `IndexBody.cpp:493-500` reports each overload; the index skips instantiations (`IndexDecl.cpp:794-795`; `language.cast:396`) | T14 `process` 257:22 (394, 406) |
| Code the build skips | one context for each unit (`language.cast:2407-2408`); no platform pass | T14 `memcpy` 358:26, `add` 415:17 (`[]`); `folder` after step 1 |

Item 24 and item 25 of `PLAN-ling-lsp-0-tracker.md` (`:415-416`) record the code-read answers for these rows. The expected files do not change.

## Framework Inventory

### API
| Need | API | Source | Step |
|---|---|---|---|
| Declarations of a symbol | `Query::applyToDeclarations` | `Query.h:289-296` | 1 |
| Readiness scope | `map::Readiness`, generated from `cast/readiness.md` | `HashMaps.h:32-61`; `cast/spell.md:34`, `:232`, `:238` | 2 |
| File row readiness | `Processor::setIndexed (int)` | `Processor.cpp:316-332` | 2 |
| Instantiation bodies in the index | `CXIndexOpt_IndexImplicitTemplateInstantiations = 0x4` | `clang-c/Index.h:6648-6652`; `Indexing.cpp:417-418`; `IndexDecl.cpp:794-795` | 3 |
| Instance reference → pattern | `IndexingContext::handleDeclOccurrence` | `IndexingContext.cpp:405-412` | 3 |
| Explicit instantiation counts as implicit for the index | `isTemplateImplicitInstantiation` | `IndexingContext.cpp:180-191` | 3 |
| Skipped ranges of a unit | `clang_getSkippedRanges`, `clang_disposeSourceRangeList` | `clang-c/Index.h:544-561`; `CIndex.cpp:9891-9935`, `:9965-9970` | 4 |
| Preprocessing record (precondition of skipped ranges) | `detailedPreprocessingRecord` | `language.cast:264`, `:614`; `CIndex.cpp:4415-4420` | 4 |
| Platform key | `currentPlatform` | `language.cast:69-80` | 4 |

### Pattern usage
| Pattern | Exemplar | Use sites | Step |
|---|---|---|---|
| Symbol-only declaration lookup | `Query::goToImplementation` | `Query.cpp:1307`, `Query.h:495`, `:541`, `Query.cpp:1443` | 1 |
| Readiness by data row | `Id::hover` `Id::file` | `cast/readiness.md` rows with `Id::file` | 2 |
| Platform table read | `getLibraryPaths().getAddress (currentPlatform)` | `language.cast:648`, `:669`, `:2243` | 4 |
| libclang export row | `getCursorReferenced` | `construction.md:427` under `## libclangSymbols` (`:393`) | 4 |
| One occurrence once | `IndexContext::addSymbol` occurrence set | `language.cast:2740-2743` | 3, 4 |

### Names
| Family | Members | Intent | Step |
|---|---|---|---|
| construction platform tables | `libraryPaths`, `sdkCommands`, `sdkFlags`, `juceModulePaths`, `homeVariables`, `cppStandards`, `moduleExtensions` (`construction.md:506-551`) | plural noun for what the table holds, keyed by platform | 4: `platformTargets` |
| platform keys | `platformNone`, `platformMac`, `platformWindows` (`language.cast:70-72`) | one constant per platform | 4: `platformLinux` |
| libclang export rows | `getCursorReferenced`, `getCursorUSR` (`construction.md:427-428`) | `get` + the libclang noun | 4: `getSkippedRanges`, `disposeSourceRangeList` |
| index option flags | `indexFunctionLocalSymbols`, `indexImplicitTemplateInstantiations` (`language.cast:374-382`) | existing enum | 3: no new name |

### Synthesis
| Rule | Established form | Exemplar |
|---|---|---|
| "Refactor-Rewrite Discipline": delete first | old code removed in the first step | CAROL.md |
| "Owner first" | a platform value lives in `construction.md` | `construction.md:506-551` |
| "dont hardcode C language into engine must be universal" | all compiler work inside cLang | `language.cast` |
| "Complete at Creation" | a record is complete when the indexer adds it | `language.cast:2722-2768` |

### Absence
| Need | Query | Result |
|---|---|---|
| A libclang flag that tells a pattern reference from an instantiation reference | `SymbolRole` (`IndexSymbol.h:102-134`), `ASTNodeInfo` (`IndexDataConsumer.h:28-33`), `CXIdxEntityRefInfo` (`clang-c/Index.h:6473-6504`) | none. Both are `CXCursor_MemberRefExpr` (`CXCursor.cpp:621-630`) |
| ling use of skipped ranges | grep of ling for `getSkippedRanges`, `SkippedRange` | none |
| A Linux platform key | `construction.md` platform rows; `language.cast:69-80` | none |
| The clang target flag spelling | `Options.td` | absent from the llvm-project checkout |

## Validation Gate

COUNSELOR validates each step before the next one against MANIFESTO.md, CODING.md, NAMES.md, the Inventory rows of the step, and this PLAN. ARCHITECT: *"dont abuse build for code validation. dont build for each turn of code implementation. only build for final debug test and profiling."* One build runs, in step 5. Audit and doxygen stay on hold: *"hold audit and doxygen. until i explicitly said so."*

## Steps

### Step 1: delete the name fallback
- **Scope:** `Source/Query.h`, `Source/Query.cpp`, `Source/Index.h`, `Source/Index.cpp`.
- **Action:**
  1. Delete `applyToScopeDeclarations` (`Query.h:298-317`) and `applyToDeclarationsOrScope` (`:319-332`).
  2. The two callers call `applyToDeclarations` (`Query.h:289-296`): `Query::goToDefinition` (`Query.cpp:383`) and `Query::getRecordType` (`Query.cpp:991`).
  3. Delete `Index::names` (`Index.h:237`), `removeNames`, `addNames`, `getNameHash` (`Index.h:269-273`; `Index.cpp:270-275`, `:422-459`) and their calls (`Index.cpp:20`, `:101`, `:105`). Delete `isNameBefore` if no caller remains (read the compiler output).
- **Inventory:** API "Declarations of a symbol"; Pattern "Symbol-only declaration lookup"; Synthesis "delete first".
- **Validation:** `index.names` has no reader left (`Query.h:304` was its only reader). No record means no declaration.

### Step 2: each request waits only for its own file
- **Scope:** `cast/readiness.md`.
- **Action:** every row with a file argument gets `Id::file`: `goToDefinition`, `findReferences`, `completion`, `goToImplementation`, `prepareCallHierarchy`, `incomingCalls`, `outgoingCalls`, `typeDefinition`, `rename`. `workspaceSymbol` has a query and no file, so it keeps `Id::files`: `getReadiness` returns the files child for it either way (`Processor.cpp:220`, `:229`).
- **Why it holds the contract:**
  - The index changes one row at a time, on the message thread. Each change replaces the complete state of that row (`Processor.cpp:1164-1196`).
  - No job thread writes an `Index` member (`Processor.cpp:714-766`, `:786-862`, `:1330-1353`).
  - A query runs on the same thread (`Connection.cpp:3-4`; `juce_InterprocessConnection.h:75-79`), so it reads complete row states in place. MANIFESTO E: *"A state update is not mutation. A complete state replaces a complete state."*
- **Inventory:** API "Readiness scope", "File row readiness"; Pattern "Readiness by data row".
- **Validation:** `HELP.md:158` states *"A request waits until the files that it needs are indexed"*, and the code then matches it. No code change in `Source/`; the cast build generates `HashMaps.h`.

### Step 3: the compiler picks the overload
- **Scope:** `language/cLang/language.cast`, `language/cLang/construction.md`.
- **Precondition read (COUNSELOR, before the Engineer prompt):** `clang_getCursorReferenced` for a `CXCursor_MemberRefExpr` over an `UnresolvedMemberExpr` (`CIndex.cpp:7150-7200`, where it calls `MakeCursorOverloadedDeclRef`). That read decides how `onReference` sees an overload-set reference. The two indexer reports have the same cursor kind (Absence row 1).
- **Action:**
  1. `indexOptions` (`language.cast:396`) adds `IndexOptionFlags::indexImplicitTemplateInstantiations`. Each instantiation body is then indexed. Its `MemberExpr` reports the chosen member, rewritten to its pattern (`IndexingContext.cpp:405-412`). This covers the explicit instantiations at `FilterStripAudioProcessor.cpp:422-427` (`IndexingContext.cpp:180-191`).
  2. `onReference` records no symbol for a reference to an overload set (`IndexBody.cpp:493-500`, one report for each candidate). It uses the test that the precondition read gives. A template that has no instantiation then answers nothing at that call. That is item 425's precedent: *"no one declaration answers, and the seed is `null` (ruling 35)"*.
- **Inventory:** API "Instantiation bodies in the index", "Instance reference → pattern", "Explicit instantiation counts as implicit"; Pattern "One occurrence once".
- **Validation:** at `257:22` the record set holds one symbol, the pattern of `kuassa_ChannelOpsBase.h:394`. The engine has no change.

### Step 4: code the build skips is indexed under its platform
- **Scope:** `language/cLang/language.cast`, `language/cLang/construction.md`, `language/cLang/spell.md`.
- **Precondition read (Librarian, library mode, before the Engineer prompt):** the clang driver spelling of the target flag and one target triple for each platform key. `Options.td` is not in the checkout.
- **Action:**
  1. `language.cast:70-72`: add `platformLinux`. Add `## platformTargets` to `construction.md` after `## moduleExtensions` (`:546-551`). It has one row for each platform key, with the value the precondition read gives. Add its spell row in the same shape as the sibling rows, and a getter fence in the same shape as `getModuleExtensions`.
  2. Add the libclang rows `getAllSkippedRanges` and `disposeSourceRangeList` under `## libclangSymbols`, in the shape of `construction.md:427`. `getAllSkippedRanges` (`CIndex.cpp:9937-9963`) gives the skipped ranges of every file of the unit in one call. `SourceRangeList` mirrors `CXSourceRangeList` (`CXSourceLocation.h:278-283`).
  - Execution detail, 2026-11-20 (from the read of `addUnitRecords`, `language.cast:5118-5129`, and the output map):
    - A platform pass calls only `addHovers` and `addChosenSymbols`, so it sends records and hovers only.
    - `addSymbol`, `addHover` and `addOutline` keep a location only when `IndexContext::isKept` holds for it. `isKept` holds for every location in the main pass, and inside a main-pass skipped range in a platform pass.
    - `platformTargets` is a TextMap keyed by triple, valued by platform, read through `applyToEntries`. `WordMap` has no entry walk (`core.cast:68-88`).
    - The flag word is the `toolchainWords` row `targetFlag`.
    - The `IndexerCallbacks` initializer moves to `getIndexerCallbacks`, so both passes share it.
  3. In the index entry (`language.cast:5128-5184`): after the main pass, read the unit's skipped ranges (`clang_getSkippedRanges`, `CIndex.cpp:9891-9935`). `detailedPreprocessingRecord` is already set (`language.cast:614`). If any skipped range exists, parse and index the unit again once for each other platform row, with that row's target flag added to the command line. Keep a record of that pass only when its location lies in a skipped range of the main pass. clice keeps a stored entry only inside the inactive regions of the current compile (`write.cpp:607-613`, `:869-871`).
  4. The records join the same `IndexContext`. The occurrence set (`language.cast:2740-2743`) keeps each occurrence once.
- **Inventory:** API "Skipped ranges of a unit", "Preprocessing record", "Platform key"; Pattern "Platform table read", "libclang export row"; Names "construction platform tables", "platform keys", "libclang export rows".
- **Validation:**
  - Every platform value is in a table, and no value is in code.
  - A unit with no skipped range gets no second pass.
  - The engine stays language-agnostic, because the work is inside cLang.
  - Expected after the build: `folder` → `:264` (Linux pass), `add` → `juce_OwnedArray.h:324` (Linux pass), `memcpy` → the declaration that the Windows pass reaches. Item 25 records `_string.h:78`.

### Step 5: build and measure
1. Run `cast cast/spell.md --no-sign` once.
2. Delete `~/.config/ling/language/cLang/` (ARCHITECT: *"always delete on disk, let the binary build and replace"*).
3. Run these harnesses through Pathfinder. Bound each one with `perl -e 'alarm N; exec @ARGV'`, and give each a 15-minute watchdog:
   - `nvim -l test/methods.lua test/config/kanjut-methods.json`: the target is T14 300/0.
   - `test/config/end-methods.json`: T14 274/0. The item 15 rows (`juce_Component.cpp` 3698, 3969, 4160) are in `JUCE_UNIT_TESTS` code, which a platform pass does not reach. They stay open.
   - `test/hover.lua` on END and KANJUT: no regression from 249/25 and 280/20.
4. Time the KANJUT cold index before and after step 4. The Part 2 ruling: *"building index for a project half an hour? … this is unacceptable"*.

## BLESSED Alignment
- **B:** each record has one producer, the compiler pass that made it.
- **L:**
  - Step 1 removes one map and four functions.
  - Step 2 is data only.
  - Step 4 adds one table, two export rows and one loop in the index entry.
- **E:** no answer without a record. A dependent call with no instantiation answers nothing; it does not answer a set.
- **S:**
  - The compiler symbol is the only identity, and `Index::names` is gone.
  - The platform targets live in one table.
- **S (Stateless):** a request reads the index in place. It asks no global job flag.
- **E (Encapsulation):** all compiler work stays in cLang. The engine takes records as it does today.
- **D:** each record comes from a compile command, so the same unit gives the same records.

## Risks / Open Questions
1. Step 3 precondition, closed 2026-11-20. For an `UnresolvedMemberExpr` cursor, `clang_getCursorReferenced` takes the expression branch (`CIndex.cpp:7175`). `getDeclFromExpr` has no `OverloadExpr` case and returns `nullptr` (`CIndex.cpp:5233-5282`). The result is `MakeCursorOverloadedDeclRef`, a cursor of kind `CXCursor_OverloadedDeclRef = 49` (`CIndex.cpp:7185-7186`; `CXCursor.cpp:1207-1215`; `clang-c/Index.h:1370`). For a `MemberExpr` cursor it returns the member declaration (`CIndex.cpp:5239-5240`, `:7178-7183`). Test: the referenced cursor kind equals 49.
2. Step 4 precondition, closed 2026-11-20.
   - Flag: `--target=<triple>` (`clang/include/clang/Options/Options.td:7034-7036`). `-target` is a legacy alias (`:7773-7775`). The last `--target=` wins (`Driver.cpp:663-664`).
   - With a non-Apple triple, `-arch` is read only for Mach-O targets (`Driver.cpp:675`, `:683-686`). Apple clang 17 warns `argument unused during compilation: '-arch …'`, observed with `/usr/bin/c++ -###`.
   - Triples and the macros they define:
     - `platformLinux` → `x86_64-unknown-linux-gnu` (`__linux__`, `OSTargets.h:383-384`).
     - `platformWindows` → `x86_64-pc-windows-msvc` (`_WIN32`, `_WIN64`, `OSTargets.cpp:288-292`).
     - `platformMac` → `arm64-apple-macosx` (`__APPLE_CC__`, `OSTargets.cpp:25-26`).
   - JUCE selects its platform from these macros (`juce_TargetPlatform.h:72`, `:79`, `:81`).
   - Open: whether libclang's `clang_parseTranslationUnit2FullArgv` runs the same driver path. Step 5 measures it.
3. Step 4 cost: for each unit with a skipped range, one more parse for each other platform. Step 5 item 4 measures it.
4. Out of this plan, not proven: two request-loss paths. The relay connects once with no retry (`Client.cpp:58`), and a failed send drops the request (`Relay.cpp:221-222`). The read found no run that hit either path. The relay was connected in every captured state.
