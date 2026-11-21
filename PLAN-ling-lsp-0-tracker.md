# PLAN: ling is the one LSP — Tracker

**RFC:** none — objective from ARCHITECT's prompts of 2026-11-17.
**Date:** 2026-11-17. Consolidated 2026-11-19 (ruling 33).
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8 / JAM (LANGUAGE.md, C++ / JUCE). Lua for the nvim side. Builds only through cast. Measurement only through scout.

## Overview

This file is the one plan of ling. It holds the objective, the rulings, the status, the tests, the open items and the remaining steps.

On 2026-11-19 ARCHITECT ruled the consolidation (ruling 33). Parts 1-7 are done. Their part files, `PLAN-ling-lsp-8-close.md`, and every plan that this plan replaced are deleted. Each open item of those files is in "Open items". Each test definition is in "Tests". The remaining steps of Part 8 are in "Steps".

`PLAN-ling-0-tracker.md` stays. It holds canon rulings 1-81, which this file cites.

## Objective, verbatim

ARCHITECT:
- *"Claude Code use clangd as LSP plugin, my nvim use clangd as its LSP, ours is ling. language agnostic, dynamic lib loaded runtime per manifest. able to reliably build AST especially juce project (which clangd often failed), have feature parity as clangd, faster, less cpu usage (at runtime), user extensible with any language, so not hardcoding anything C language family into its engine"*
- *"we probably need re-plan EVERYTHING start from current state, and break it down into multi-part plan, with tracker. the plan must be COHERENT, COMPREHENSIVE, CLEAR and UNAMBIGUOUS. leave no room for interpretation, no room for assumptions, no room for inference. no open questions, no risk, all must be answered by reading."*
- The four items, verbatim:
  1. *"fix the runtime compiler, do we need to pack small dynamic/static lib for compiling language extension? so the language itself could use jam/juce modules that optimised for runtime process? why dont we make it isomorphic to tree-sitter? the pattern is battle-tested, we only need to adopt to our domain, as BLESSED compliance"*
  2. *"carry supported in-house jam documents language"*
  3. *"fix current issues with daemon server, why high cpu? why slow? how can we make FASTEST, identify code smells, what the contract violations? what framework API, established pattern, inventory that we missed?"*
  4. *"add features to replace tree-sitter"*

## Status

The part files of Parts 1-7 are deleted (ruling 33). Each Status cell keeps its result.

| Part | Plan | Subject | Status | Depends on |
|---|---|---|---|---|
| 0 | this file | Tracker, canon, current state | Written. Consolidated 2026-11-19 | |
| 1 | `PLAN-ling-lsp-1-language-runtime.md` | No compiler at run time. ling's build compiles each default language library and carries it in BinaryData (END's EVE shape). The daemon writes the carried libraries to a cache folder named by their hash (atomic publish), checks the ABI version, and loads a library that `ling --build` wrote in the config folder before the carried one. `ling --build` is the author's command. The default language folder is written to the config folder again when absent | Done 2026-11-17. With the config language folder moved aside: T4 END 263 agree, 12 both-empty, 25 ling-miss, 0 disagree; T6 KANJUT 276, 12, 12, 0. Daemon children: `clang-format`, `sh`, `bash` only | 0 |
| 2 | `PLAN-ling-lsp-2-library-jam.md` | The carried library `language/core/`: one translation unit holds every JAM and JUCE body and every explicit template instantiation that cLang uses, behind `ling_LanguageCore.h`, a header of declarations only: no template, no definition (rulings 1, 2, 3, 10, 12, 25, 26, 28). The header declares a closed set of carried classes, one for each distinct instantiation (ruling 28). Each row of cLang is packed words through `jam::Union` (ruling 9). cLang uses the carried classes in place of `std::vector`, `std::tuple`, `std::pair`, `std::array`, its own `LookupTable`, `jam::HashMap`, `popen` and `dlopen`. Every libclang type takes a NAMES name in place of the `CX` prefix. A record is pure metadata: symbol hash, name length, positions, role, type. The boundary sends one call for each record with its hover. The engine keys symbols by the hash. The Step 8 hover contract of `PLAN-ling-clangd-replacement.md` holds, and Step 8 passes T4, T6, T7, T8 | Done 2026-11-18, build 28. T4 END 263 agree, 12 both-empty, 25 ling-miss; T6 KANJUT 276, 13, 11; T7 END 27 equal, 13 differ; T8 KANJUT 28, 12. END cold index 93 s (was 767.5 s, `step4/SCORES.md:17`) | 1 |
| 3 | `PLAN-ling-lsp-3-document.md` | Query and Index take `jam::Document`. The role and the name lookup come from the language of the file, not from a `LanguageDocument` member | Done 2026-11-18, build 44. T4 END 286/3/8/3; T7 35 equal, 3 differ, 2 stale; T6 KANJUT 286/5/5/4; T8 39/1. END cold index 83 s. KANJUT cold index 65 s | 2 |
| 4 | `PLAN-ling-lsp-4-jam-languages.md` | Markdown, mermaid, json, css, xml and html are built-in languages behind one `Language` type with library languages. The manifest tables `source glob`, `layout glob` and `binary` give their files. The JAM parsers give a span to each named element (ruling 21). Ansi is a decode layer that keeps the offsets | Done 2026-11-18, build 65. T4 END 286/3/8/3; T7 33 equal, 3 differ, 4 stale; T6 KANJUT 286/5/5/4; T8 39/1. On END a markdown fixture: documentSymbol lists the headings; definition and findReferences join a link `#x` to its heading; an SGR sequence moves no offset. Tracker notes 9 to 13 hold the rulings and the causes | 1, 3 |
| 5 | `PLAN-ling-lsp-5-daemon.md` | The daemon cost. The swap serializer of `jam::Document` becomes `SwapWriter` and `SwapReader`, with a name table and a view read. Hover text is formatted at hover time in a `FormatJob`, so no formatter child runs during the index. A `daemon` `threads` row sets the pool. scout measures before and after | Done 2026-11-18, build 68. T4 END 284/3/8/5 (two misses in `jam_Document.cpp`, see tracker note 17); T7 33/3/4; T6 286/5/5/4; T8 39/1. scout, warm `end-cold-sample`: epoch 2.359e10 to 1.136e10 ns, user 5.334e10 to 1.440e10 ns, system 1.878e10 to 1.472e10 ns; `end-debug`: epoch 1.336e10 to 1.040e10, user 4.659e10 to 1.495e10, system 1.141e10 to 1.447e10. Samples `jam:: document::read` 4273 to 1607, `index::read swap file` 5061 to 2054. No `clang-format` row. `threads` 5 (half of 10 CPUs), warm: epoch 1.713e10, user 1.860e10, system 1.101e10 | 2, 4 |
| 6 | `PLAN-ling-lsp-6-buffer.md` | The tree-sitter features from ling for cLang and the six built-ins (ruling 27): element ranges and loop and conditional elements, the token stream kept on each root, full buffer sync, semantic tokens, selection ranges for flash, `ling/textObject` for the eight text objects, and the nvim side | Done 2026-11-18, build 88. T4 END 284/3/8/5; T7 34 equal, 3 differ, 3 stale; T6 KANJUT 286/5/5/4; T8 39/1. `test/buffer.lua`: T23 buffer sync (an unsaved edit shows in `documentSymbol` after 0.01 s), T24 semantic tokens, T25 selection range, T26 the eight text objects equal tree-sitter: all pass. `test/keymaps.lua` T27 (no tree-sitter highlighter on a cpp buffer, tree-sitter kept on lua, the eight real keymaps, flash matches, the JAM highlight groups): all pass. Tracker note 18 holds the deviations from the plan text | 3, 4, 5 |
| 7 | `PLAN-ling-lsp-7-parity.md` | The open method steps of `PLAN-ling-clangd-replacement.md` (old Steps 10-17 and 19): the PCH probe, declaration, switchSourceHeader, documentHighlight, rename, documentSymbol from `addOutline` for every language, typeDefinition, completion, codeAction, HELP.md | Executed. Its methods are accepted in Part 8 Step 1 (T11, T12, T14-T18, T20) | 2, 4, 6 |
| 8 | this file, "Steps" | Acceptance on END and KANJUT (T1-T29), speed and CPU against clangd, the switch of nvim and Claude from clangd to ling, the audit, the doxygen pass, plan hygiene, the log | In progress: Step 1 (round 8 below, and "Open items"). Step 2 done 2026-11-19. Step 5 void (ruling 36). Step 6: the plans are deleted; the cache libraries are open | 1-7; MCP Parts 1-5 (done) |

## Rulings of 2026-11-17, verbatim

Each ruling quotes ARCHITECT. A selected option quotes its label and its text as presented.

1. On a language library that may use JAM and JUCE: *"why cant we use juce?"*. Then the selected option, *"Embedded JAM+JUCE (Recommended)"*. Option text: "ling carries a prebuilt JAM+JUCE static library and its headers in BinaryData. It writes them to the cache when the stamp hash changes (END's EVE shape, Nexus.cpp:217-229), and the language library links them. Basis: CODING.md 'Use the framework API fully' and MANIFESTO S (DRY): language.cast:46-79 rewrites jam::LookupTable." This replaces the "no" to JAM/JUCE headers at the library build of the removed Part 12 (`HEAD:PLAN-ling-12-language.md:25`, `:42`).
2. On the shape of the carried library: *"dylib mac, dll for windows, compiling language could use include it to compile, so we never expose source code static lib + header only"* / *"explicit instatiations with alias then wrap into narrow API? so the header that call it as lean as clean as possible"* / *"we need to design this properly, in ling toolchain, so if future we need to refactor, add/remove deps for compiling language, should be trivial. - ling build must chain the library it carry - rebuild if stale probably the shape identical with END, carry EVE. EVE itself deps as binary and sidecar - this all could be wired with cast so the library binary itself as tiny project to compile? subproject? submodule inside ling?"*
3. On the sidecar: *"why prebuild library need sidecar? we can just keep it internal no? load from BinaryData"*. Result: no sidecar. The library and its header are BinaryData of ling, written to the cache, as END writes EVE (`end/Source/plugins/Nexus.cpp:200-229`).
4. On the tree-sitter pattern: *"ling keep the shape of current compiling dynamic lib for language extension, but make it more reliable, learn the patterh from tree-sitter"*.
5. On the in-house documents: *"ling carry jam docs parser OOTB, so xml, html, css, markdown, mermaid, whatever jam already parse are in-house built in feature without extension"*. On their features: *"exactly the same with language extension"*.
6. On the coherence of the document type: *"LanguageDocument is jam::Document all other jam parser are jam::Document why including them as in-house built in language need to redesign everything? this is smell the LanguageDocument itself is garbage. why? because jam::Document languages that we support are battle tested we used for everything. LanguageDocument only being used by ling, newly added, why they are not compatible to the whole design? why there's no coherence, consistency, for seamless integration?"*
7. On the tree-sitter features: *"we add missing puzzle, via terminal protocol ling can replace tree-sitter feature that i use"*. On "terminal protocol", ARCHITECT selected *"a"*: the stdio transport, `jam::StdioServer` and `jam::StdioProtocol` (`jam/jam_terminal/display/`), which ling already speaks LSP over (`Source/Relay.cpp:70`, `:102`).
8. On the nvim side: *"COUNSELOR, in this plan"*. Option text: "The plan scope includes the ~/.config/nvim files: lua/lsp/ling.lua, the flash and text-object adapter, and the keymaps. Same form as locked Step 3." This replaces `PLAN-ling-0-tracker.md:36` for this PLAN.
9. On the record storage of the library: *"use jam::Array designed to be lighter than vector, use bitpack lighter and faster than tuple."* Ruling 1 makes it possible: the library links JAM.
10. On the header of the carried library, ARCHITECT selected *"Narrow wrapper header"*. Option text: "Your earlier idea: explicit instantiations with aliases in the static library, behind one lean header with no JAM or JUCE include. language.cast uses only the listed wrappers. A new need means a change to the subproject's narrow API and a rebuild of ling."
11. On the environment: *"my point is, whatever ling runtime need to compile must not depends on environment, no juce, no jam, only the dynamic library itself, feasible? or do we still need clang compiler?"* / *"no compiler at run time"* / *"we could have `ling --build <language-ext>` like tree-sitter? without compiler?"* / `/go`. Result: the daemon compiles nothing. ling's build compiles the default language libraries and carries them. `ling --build <language folder>` is the author's command, and it needs cast and a C++ compiler, as `tree-sitter build` needs a C compiler (`tree-sitter/docs/src/creating-parsers/1-getting-started.md:11-12`; `docs/src/cli/build.md:10`). This replaces the run-time compile of rulings 1 and 4: ruling 1 now holds for the build of ling and of `ling --build`, and ruling 4 holds for the loader (ABI version, atomic publish, staleness by hash).
12. On the interface of the narrow header, ARCHITECT selected *"C++ classes"*. Option text: "C++ wrapper classes (PIMPL, as CODING.md's namespace section permits) with the instantiations inside. The author must use a compiler and standard library with the same C++ ABI as ling's build."
13. On the repeated `LookupTable` of the cLang library, ARCHITECT selected *"JAM header ships alone"*. Option text: "Part 2 makes jam_LookupTable.h and jam_LookupEntry.h free of JUCE (a standard assert in place of jassertfalse), and ships them like jam_HashMap.h. language.cast includes them and deletes its copy. The scope widens to these two JAM headers." **Void.** COUNSELOR asked this question on its own false premise that the library has no JAM. ARCHITECT: *"you asked wrong questions"*. Rulings 1, 2, 3, 10 and 12 govern: the library gets `jam::LookupTable` through the carried JAM+JUCE library, and no JAM header changes for the library.
14. On the containers and the strings of the records: *"what ruling? if we can include jam and juce, why still using vector and tuple?"* / *"then the design is wrong. why hold string? string is materialisation. why not pure metadata?"* / `/go`. Result: every `std::vector` and `std::tuple` of the library converts. A record holds no string: its symbol is a 64-bit hash, and its name is a length at its offset in the source. The engine keys symbols by the hash. Rulings 1, 2, 3, 10 and 12 hold: the language library links the JAM+JUCE static library that ling carries, behind the narrow wrapper header, and nothing of it is exposed to the user. (Correction of 2026-11-17: an earlier text of this ruling said that the JAM headers ship with no JUCE and that the narrow library is not built. That was COUNSELOR's inference, not ARCHITECT's words, and it is deleted. ARCHITECT: *"i thought we already agreed, dynamic library is carrying juce and jam. but it was not exposed to user"*.)
15. On the files of the in-house documents. COUNSELOR asked which markdown, css, json, xml and mermaid files ling indexes, with three options: "Open buffers only", "Scan project roots", "Manifest names them". ARCHITECT: *"what's the most DETERMINISTIC? most BLESSED approach?"*. COUNSELOR answered "Manifest names them": the same manifest and the same files give the same index (**D**); the file set is a visible input (**E**: "All parameters visible… nothing pulled from implicit context"); the manifest is the one source of the project (**S**). ARCHITECT: `/go`. Result: a manifest table names the files of the in-house documents. The built-in `units` reads it, as the cLang `units` reads the toolchain rows.
16. On the table of ruling 15, ARCHITECT selected *"Read existing tables (Recommended)"*. Option text: "ling reads the rows of `layout glob`, `binary` and `source glob` whose extension maps to a built-in family. No manifest edit. S (SSOT): the manifest already lists these files once, so a second list would be shadow truth."
17. On the JAM parsers, ARCHITECT selected *"Widen to JAM parsers (Recommended)"*. Option text: "The scope adds jam_MarkdownBlockParser, jam_Css, jam_Xml and jam_Html. Each parser adds the span of each named element at creation, and HtmlDocument gets a public parse. MANIFESTO E, Complete at Creation: "A parse operation gives a complete AST.""
18. On the set of documents, ARCHITECT selected *"Json"* and *"Ansi"*, besides the five named in ruling 5 (xml, html, css, markdown, mermaid). Not selected: "Image, OpenType, Pdf" and "Code, Reflow".
19. On Ansi: *"why ansi need extension? what's the convention? should not `ansi` means encoding? decoding? any extension could be read as"*. COUNSELOR then asked what makes ling decode a file as Ansi. ARCHITECT selected *"Both"*. Option text: "The bytes decide for files on disk, and the client decides for open buffers." The two parts of "Both", as presented: "A file whose bytes hold the ESC introducer decodes through AnsiDocument first. The language of its extension then reads the decoded text. Same bytes, same result (D)." and "nvim marks the buffer as Ansi when it opens it (the languageId of didOpen), for example a terminal buffer or a log. Part 6 carries it." Result: Ansi is a decode layer, not a family, and it has no extension.
20. On how the engine holds a built-in language, ARCHITECT selected *"One interface (Recommended)"*. Option text: "One abstract language type with parse, read, units, the role test and the symbol kind. LanguageLibrary and one class for each JAM document implement it. One map and one dispatch (S, SSOT)."
21. On the named elements of each JAM document. ARCHITECT's note on the first draft: *"pandoc grid format"*. On the grid draft, ARCHITECT selected *"Approve the table"*. The approved table, verbatim:

```
+----------+-----------------+------------+------------------------+
| document | declares        | kind       | refers                 |
+==========+=================+============+========================+
| markdown | heading         | String     | link href `#a`         |
|          |                 |            | link href `f.md#a`     |
|          |                 |            | link href `f.md`       |
| mermaid  | node id         | Object     | edge from, edge to     |
| json     | object key      | Key        |                        |
| css      | rule selector   | Class      |                        |
| xml      | element tag     | Object     |                        |
|          | `id` attribute  | Constant   |                        |
| html     | element tag     | Object     | `href="#id"`           |
|          | `id` attribute  | Constant   |                        |
+----------+-----------------+------------+------------------------+

symbol: Wyhash (absolute path + Format::toValidID (text, true))
`f.md` alone refers to the file: definition is line 1.
```
22. On the swap serializer of `jam::Document`. COUNSELOR asked whether Part 5 can edit `jam::Document::write` and `read`, and named the file-static free functions of `jam_Document.cpp:726-1051` (`readToken`, `readIdentifier`, `readValue` and their family). ARCHITECT: *"what's jam::read? smells. why free function on namespace"*, then *"no not allowed, those are all garbage. not codebase pattern. there are no jam::function, must be encapsulated"*. Result: Part 5 edits `jam_Document.h` and `.cpp`. No free function in the `jam` namespace stays on the stored-tree path. An object owns the read state and the write state.
23. On the hover formatter, ARCHITECT selected *"Format at hover time"*. Option text: "The record keeps the raw definition. The library formats one definition when hover asks for it, through a new boundary entry. No formatter child runs during the index."
24. On the size of the Processor pool, ARCHITECT selected *"Manifest daemon row"*. Option text: "A `threads` row in the `daemon` table sets the count, read as `getTimeout` reads `timeout`. With no row, the JUCE default (all CPUs) applies, so only the difference is configured (CODING.md)."
25. On JAM templates in the language library. COUNSELOR showed that the static library cannot hold `jam::Array<Token>`, because `Token` exists only in `language.cast:82-83`. ARCHITECT: *"what if template  from jam explicit instantiation first at dynamic lib, so the lib carry no template"*, then `/go`. Result: each JAM template that a language library uses is instantiated in the language library itself (one translation unit, `language.cast:4447`). The carried static library holds only non-template JAM and JUCE bodies, behind the narrow wrapper header of rulings 10 and 12. The JAM template headers, and the JUCE headers that they include, travel as headers with the static library. This follows ruling 1's option text: "ling carries a prebuilt JAM+JUCE static library and its headers". No JAM header changes for the library (ruling 13, void).
26. On what the language library includes. COUNSELOR asked how cLang reaches non-template JAM and JUCE code after ruling 25. ARCHITECT: *"make it so header only include. no definition no template etc everything already well defined at TU"*. COUNSELOR asked which translation unit. ARCHITECT selected *"The carried library TU"*. Option text: "The carried library defines everything: the JAM/JUCE bodies and every template instantiation that cLang uses. The header declares only. cLang's own types, such as Token and Element, then become declarations in that header, with their definitions in the carried library." Result: this replaces the template part of ruling 25. The carried library is one translation unit that includes JAM and JUCE. It holds every JAM/JUCE body and every template instantiation that the language library uses. Its header holds declarations only: no function body and no template. cLang includes that header, `jam_LanguageInterface.h`, and the standard library only. This is rulings 10 and 12: explicit instantiations in the static library, behind one lean header, as C++ classes with the instantiations inside.
27. On the filetypes that move from tree-sitter to ling, ARCHITECT selected *"ling's languages (Recommended)"*. Option text: "cLang and the six built-ins move to ling for highlighting, text objects and flash. Tree-sitter stays for every other filetype until a language library for it exists. Ruling 5: the built-ins have the features of an extension."
28. On ruling 26 and CODING.md "No near-duplicate struct proliferation … If parameterization is needed, use a template". COUNSELOR offered a template in the carried header as an option. ARCHITECT: *"NEVER EVER SUGGEST BULLSHIT such as `include template in header` again"* / *"i have told you no fucking source code exposed, no fucking template on header, make it explicit instatitations"* / *"NEVER EVER CREATE SUGGEST CONTRACT VIOLATIONS, if your options breaks contract, then it was not an option at all"*. Result: ruling 26 holds without exception. Each template is explicitly instantiated in the carried TU. The header holds no template and no definition. CODING's near-duplicate rule holds at the same time: one carried class for each distinct instantiation, never two classes for one instantiation. Every option that COUNSELOR presents complies with CONTRACT and with every ruling.
29. On the visit structs of Part 2 choice 8 (`InitializerVisit`, `EnumeratorVisit`, `CursorVisit`). ARCHITECT: *"new struct violates @carol/MANIFESTO.md you create stateful object on top already exists complete AST + state machine"* / *"if complete AST + state machine cant satisfy whatever we need to communicate with third party lib such as libclang, then the design is garbage. harcoding libclang semantics also garbage, ling contract must be able to communicate with any language deps not just libclang."* / *"what's the point having extensible, compilable manifest if you hardcode external library lingua to our engine"* / *"so visit is libclang semantics never ours"*. Result: Part 2 choice 8 is void. No new struct carries state to or from a third-party callback; the existing document and state machine of the language carry it. No type, name or concept of a third-party library becomes part of the ling contract.

## Rulings of 2026-11-18, verbatim

30. On the 1 ms wait after each stdin read (`TerminalClock` gave `period = min (bufferSize / 2 / rate, max)`, and `TerminalClock::prepare` asserted a maximum above 0), ARCHITECT selected *"jam: period 0 allowed"*. Result: `jam_TerminalClock.cpp:9` asserts `newMaximumPeriodMilliseconds >= 0.0`. `Relay.h:255` holds `maximumPeriodMilliseconds { 0.0 }`.
31. On the completion positions where clangd's answer is cut short, ARCHITECT answered: *"what is the contract? why keep asking trivial question? did you not read anything? design by contract MANIFESTO.md"*. COUNSELOR applied the CONTRACT:
    - MANIFESTO **D**. A clangd answer with `isIncomplete` is a subset of the true set, so ling's set is scored as a superset of it (`test/methods.lua`, `completionAt` and `completionVerdict`).
    - MANIFESTO **S** (one truth for each file). The index reads the open buffers (T23, T24). `LanguageIndex` takes the buffer paths and texts (`languageVersion` 7).
32. ARCHITECT: *"let's start jam mcp then. probably not ALL. but the infrastructure, steps of plan to the point LING can achieve its core feature first. this means you need also track jam mcp plan, update LING's plan. then continue our work here. LING's mcp could wait until we achieve ling-clangd objectives parity."* / *"DEEP DIVE, INVENTORISE THOROUGHLY, dont leave any gaps for inference, no room for assumptions. update our PLAN-ling-lsp, sync with jam mcp plan, update the tracker too"*. On "Which parts run now?", ARCHITECT selected *"Parts 1-5 now"*. Option text: "jam module + framing (1), JSON Schema (2), ling schema build + suite runner (3), StdioClient (4), ModelContextServer (5). Then LING Part 8 to parity. Then Part 6 ling --mcp; Parts 7-8 after." The question stated the reads: the ling LSP path uses no MCP part (`Relay.h:258`, `Relay.cpp:132`), and the T9/T10 cost lane uses the whatdbg smoke pattern with a `## sample` table.

## Rulings of 2026-11-19, verbatim

33. On the plans: *"my point is, this is confusing. so many plan, we should just moving on, carry the outstanding, update the state, the tracker. anything completed/supersed should just be deleted."* Result: this file is the one plan. The part files and the replaced plans are deleted.
34. On diagnostics: *"ling still must have diagnostics, that's the parity feature with clangd."* Result: T13 and T18 are parity tests. The design of the deleted `PLAN-ling-10-diagnostics.md` was replaced before this date by `addDiagnostic` and `addFixIt` in the language library. The code holds it: `Processor.cpp:893-895`, `:950`, `:1062`; `Query.cpp:648`, `:784`; `SwapValidator.cpp:90`, `:157`.
35. On the verdict of an answer:
    - *"clangd is reference baseline only. ling ultimate objectives is CODEBASE ground of truth. that is exactly why we develop ling. we have most accurate, most reliable language agnostic LSP"*.
    - *"when in doubt: discrepancy between ling vs clangd. trust THE CODEBASE READ as the absolute ground of truth. ling MUST NOT GUESSED. but GET the accurate read"*.
    - *"if ling already able to get clangd answers (correctly, agree with the code), at this point we should leave clangd answers entirely. we only want clangd parity feature. correctness always must agree with the CODE READ."*
    - *"IIUC at this point clangd as reference should only for feature parity/performance. not for answer anymore, because clangd answers itself unreliable always wrong for juce project. that's exactly why develop ling, language agnostic, extensible, first citizen c language family works with juce. probably at this point for juce ling already answer better. so to validate answers, we check the CODE read, is it give the result as we expect as the ground of truth? should not waste token compare to clangd anymore."*
    - Result: an answer passes when it equals the answer that a read of the code gives at file:line. The harness does not ask clangd for answers. clangd stays the reference for the feature set and for speed and CPU (T9, T10, T21).
36. On the switch and the runtime check:
    - *"you could delegate to replacement of clangd to my nvim now. so we could also have runtime validation with ARCHITECT's vim. the binary will always fresh, to our latest changes."*
    - *"likewise wire to Claude, so you can also validate runtime whether it return correctly as you expect because you can still both the result and the code."*
    - *"switching clangd for claude and my nvim should be trivial, you already have context. no need rfc to machinist. just delegate to engineer."*
    - *"carol also need update? switch clangd to ling"*.
    - Result: Step 2 ran on 2026-11-19 for nvim, for the CAROL plugin and for the CAROL text. Step 5 (the RFC for MACHINIST) is void.
37. On index progress. ARCHITECT asked: *"with clang i could see the clangd bar percentage when building, it's slow but there's visual feedback. is ling index building faster? also behave the same with visual bar update?"*. Then: *"yes feature parity. ling must be drop in replacement for clangd, especially with my nvim"*. Result: T29. ling sends `$/progress` while it indexes. nvim shows it through noice, which replaces the `$/progress` handler (`~/.local/share/nvim/lazy/noice.nvim/lua/noice/lsp/progress.lua:101-102`; default on, `noice/config/init.lua:117-118`).
38. On the source of `jam::Document`: *"what is borrowedSource? smells"* / *"borrowed is foreign semantics. reflection of wrong mental model. we already have the complete AST, what's wrong with reference?"* / `/go`. Result: one `std::string_view source` over a boxed arena, `std::unique_ptr<std::string> arena` (`jam_Document.h:1457-1458`). The moves are `= default` (`jam_Document.h:1025-1026`).
39. On `JsonDocument::find`: *"Validator should check ONCE, at parse time. invalid no ops"* / *"valid create complate AST"* / *"downstream, consumer trust 100% VALID document"* / *"there are no mutations, creation could be done in couple of steps."* / *"state update is not mutation"* / *"0 rules @carol/MANIFESTO.md always applied."* / *"do we need find? otherwise delete."* / scope *"including: - end - eve - scout - ggwp - cast - whatdbg"*. Result: `JsonDocument::find` is deleted. `get` throws on an absent key (`jam_JsonDocument.cpp:133-153`). A request has an id and a notification has none, so `Relay` and `ModelContextServer` each hold `requestMethods` and `notificationMethods`.
40. On the jam pull: *"HEAD MUST WIN"* for each line that the pull changed, and the local MCP work *"should also WIN"*. Result: all 17 overlapping jam paths hold the HEAD lines and the local work.
41. From the handoff: *"after the pull then you continue to complete your plan to completion, including fixing `kind`"* and *"dont abuse build permission to validate each turn code implementation. only build to measure and debug runtime parity features"*.

## Rulings recorded in the part notes, verbatim

- Part 2, the index time: *"building index for a project half an hour? even clangd only took couple of minutes, i already considered that as slow. this is unacceptable. SPEC violations"*.
- Part 2, the validator: *"the established pattern already Validator everywhere, no consumer need to check anything. it's always build AST, validate, invalid return error and no ops at all"*. Result: an invalid swap gives nullptr, and the file is parsed again.
- Part 2, uncompiled code: ARCHITECT selected *"Scope answer, name-indexed"*. Result: when no record answers, `goToDefinition` runs `applyToScopeDeclarations`. It reads `Index::names`, keyed by the hash of the last name segment and kept by `addNames` and `removeNames`.
- Part 2, transport patterns: *"implement those 3, then continue your work to meet our objectives."* The three patterns: a request has one lifetime, its row; a request is validated, then answered with a result or an error; the daemon name carries the build (the Wyhash of the executable bytes).
- Part 4 Step 1, the name spans: *"why cant we use exsisting identifier?"*, *"is it case sensitive?"*, *"then just use different case? toUpperCase? toTag?"*, *"then dont use toType, if we dont communicate with juce native API, we dont want to use juce::String"*, *"NO. name and NAME are not unique identifier NOT allowed."*, *"just add one. dont overcomplicate"*. Result: the name spans are under one jam `Id` row, `nameTokens` (`jam/cast/core.md`). xml and html reserve it in `getStructuralKeys`.
- Part 4 Step 2, the element tables: ARCHITECT selected *"Row per token slot (Recommended)"*. Option text: "Row key: language, element id or `*` (any other id), and the position of the token in `Id::nameTokens`. The row gives role and kind. Tag = slot 0 Object, `id` attribute = slot 1 Constant, html `href` = slot 2 reference. The engine reads role and name for each token. The tree shape of the six documents stays as built in Step 1, so no existing consumer sees a change (MANIFESTO E, data shape)."
- Part 4 Step 3 note 6, the build: *"BUILD ABUSE just validate each turn code implementation IS NOT ALLOWED"* and *"use build to NOT TO VALIDATE EACH TURN. always use EFFICIENTLY"*. COUNSELOR validates each change by reading. One build follows each batch.
- Part 4 note 9, `find` and the document contract: *"delete find. replace all find with get. NEVER ADD FIND again. we never want one, we never need one. if we need one, the design is garbage. AST wants GET, never find"*; *"Document consumer NEVER CHECK. Document must be valid, validated by Validator ONCE at parse time before memoization, cached. this is the contract. invalid documents no ops, show error."*; *"my rule is not new pattern, IT IS THE CODEBASE ESTABLISHED PATTERN."*; *"what is ordinary input? if it's throw, the problem is not the framework, nor the contract. ALWAYS DATA or USAGE"*; *"find may lead to silent fail, and you churn tokens to debug garbage. throw, will show you invalid documents, so you can fix the data and YOU CANT NEVER HACK the usage."* Rules that follow: a Validator may use `contains`; creation code (parsers, builders) may use `contains` then `get`; a consumer uses `get` only, or `get (key, default)` for an optional value type; a producer that omits a key its consumers read is a data defect, fixed at the producer and required by the Validator.

## Sync with the jam MCP plan

- **Order (ruling 32; `jam/PLAN-model-context-protocol-0-tracker.md` R14):**
  1. `jam/PLAN-model-context-protocol-1-module.md` to `-5-server.md`.
  2. Part 8 Steps 1-7 of this plan.
  3. MCP Parts 6-8.
  4. The MCP Close.
- **The macOS check of each MCP part gates the next step.** ARCHITECT runs the Windows check on the Windows host, and it blocks no part (canon ruling 78; ARCHITECT 2026-11-13, `PLAN-ling-0-tracker.md:53`).
- **What MCP Parts 3 and 4 add to ling:**
  - Part 3:
    - the `jam_model_context_protocol` row in `## user module`;
    - the `lingUsingSchema` macro row (after `project-info.md:173`);
    - three `schema` rows in `profile-info.md`;
    - the `LING_USING_SCHEMA` branch before the `#if JAM_USING_PROFILER` at `Main.cpp:352`;
    - `Source/SchemaSuite.h`, `.cpp`;
    - `benchmark/schema/`;
    - `benchmark/schema.md`;
    - the `schema` and `suite` rows of `cast/identifiers.md`.
  - Part 4:
    - `Source/StdioClientProbe.h`, `.cpp`;
    - `benchmark/stdio-client.md`, `stdio-client-windows.md`, `stdio-client.sh`;
    - `test/baseline/stdio-client-macos.txt`, `stdio-client-windows.txt`;
    - the `stdioClient`, `streams` and `input` rows of `cast/identifiers.md`.
- **The LSP path is unchanged.**
  - The schema build passes `-DCAST_PROFILE=ON`, so it does not install over `~/.local/bin/ling` (`CMakeLists.txt:488`).
  - The `--no-sign` build, which nvim and the harness start (`~/.config/nvim/lua/lsp/ling.lua:23-27`), keeps the `Relay` branch (`Main.cpp:362-364`).
- **Audit coverage:** Part 8 Step 3 audits ling `Source/` and `cast/`, so it covers the files of MCP Parts 3-4 in ling. MCP Close 1 audits them again, by its own scope.

## Standing rulings that govern every step

- Canon rulings 1 to 81 of `PLAN-ling-0-tracker.md:63-157` stay in force, except where a ruling above replaces one.
- *"FULLY EVENT DRIVEN… DESIGN BY CONTRACT. NO HANDROLL"*
- *"no copy. no alloc. no mutations. no temporary containers. no shadow state"*
- *"clangd parity as result but no hardcoded clangd logic on engine"*; *"no `clang` on ling engine"*.
- *"until ling green for all tests with 100% accuracy and faster than clangd for all cases"*. Ruling 35 sets what "accuracy" means: the code read, not clangd.
- *"use nvim with lua as harness, permission granted"*.
- *"windows will conform when we get there"*. Canon ruling 78 governs the Windows steps.
- KANJUT is not edited.

## State of 2026-11-19

The working tree, not a commit, is the state (CAROL.md Git).

- **jam pull merged (ruling 40).** All 17 overlapping jam paths hold the pulled lines and the local work.
- **`jam::Document` source (ruling 38).** `source` is a view over the boxed `arena` (`jam_Document.h:1457-1458`). `getSource` returns `source`, and `getArena` returns `*arena` (`jam_Document.cpp:243-250`).
- **`JsonDocument::find` deleted (ruling 39)** in jam, ling and whatdbg.
  - `JsonDocument::get` is at `jam_JsonDocument.h:188`.
  - `Relay` holds `requestMethods` and `notificationMethods` (`Relay.h:243`, `:245`; `Relay.cpp:79-86`, `:160-181`).
  - `ModelContextServer` holds the same pair.
- **Builds.** ling built (`cast cast/spell.md --no-sign`, exit 0). whatdbg built (ARCHITECT, 2026-11-19).
- **`Id::endLine` defined twice.** The build printed `ld: warning: duplicate symbol 'Id::endLine'`. ling `cast/identifiers.md` dropped its row. jam `cast/terminal.md:55` keeps the definition.
- **Doxygen warnings.** The jam library doc run printed warnings for `jam_LanguageDocument.h`, `jam_LanguageLibrary.h`, `jam_MarkdownDocument.h` and `generated/jam_Unicode.h`. Each block is now corrected, and a rerun printed no warning.
- **The switch (Step 2, ruling 36):** see Step 2.

## Part 8 Step 1, round 8 (2026-11-18)

**Results on END:**

| Test | Agree | Not agreeing | ling median | clangd median |
|---|---|---|---|---|
| T12 | 9 | 1 disagree | | |
| T14 | 286 | 6 disagree, 5 miss | 0.202 ms | 0.094 ms |
| T15 | 192 | 13 disagree, 18 miss | 0.187 ms | 0.086 ms |
| T16 | 40 | 0 | | |
| T17 | 40 | 0 | 0.271 ms | 0.269 ms |
| T20 | 10 | 0 | 0.158 ms | 0.070 ms |
| T11 | 133 | 13 disagree, 50 extra, 72 miss | 5.27 ms | 18.9 ms |

- T4 (compare): 284 of 300 agree, 8 disagree, 5 miss.
- `test/buffer.lua`: 21/21. The T23 "appear" case takes 10.1 s, down from 53.7 s.
- `test/keymaps.lua`: 17/17.
- `test/builtin.lua`: markdown and css pass. json, xml, html and mermaid are SKIP, because END's config folder has no such file. T28 (the built-in language test, T23 in the deleted close plan) requires one mermaid file.
- T22 passes.
- T10 warm: ling's first answer takes 4.4-7.6 s; clangd's takes 2.9-6.7 s. It does not pass reliably.
- Not yet re-run on this build: T1-T3, T6, T8, T13, T21.

**Latency per request** (back-to-back probe, scratchpad `probe-latency.lua`):

| Hop | Cost |
|---|---|
| Relay inbound | 0.11-0.17 ms |
| Relay to daemon | about 0.06 ms |
| Daemon | 0.07-0.15 ms |
| Daemon to Relay | about 0.06 ms |
| Relay stdout write | 0.05 ms |
| Whole request, ling | about 0.26 ms |
| Whole request, clangd | about 0.05 ms |

**Fixes of rounds 1-8:**
1. `SwapValidator`: the `completions` rule applies only when `file->id == Id::file`. Before, every document swap failed, and each daemon start parsed every file again.
2. `Index::pathRows` (`jam::HashMap<juce::String, int>`, built in `addUnitRows`). `getReadiness`, `applyBufferChange` and `getCounterpart` use it in place of a scan of the file rows.
3. Owner-only storage. `IndexJob::foreignPaths` and `isForeign`: six callbacks keep only the records of the files that the unit owns. The END swaps dropped from 6.6 GB to 788 MB.
4. Buffers into the library. `LanguageIndex` takes `bufferPaths`, `bufferTexts` and `bufferCount` (`languageVersion` 7). cLang passes them to `parseTranslationUnit2FullArgv` as `UnsavedFile`. A buffer parse queues only its owner unit (`index.unitRows`).
5. typeDefinition:
   - `LanguageRole::definition` (`1 << 10`);
   - `addTypeDefinition` for declarations and references, with `typedSymbols`;
   - `Query::typeDefinition` prefers definitions and walks up to the qualified element.
6. Completion:
   - `addCompletion` carries the file path;
   - `completionStride` is 5;
   - the scopes now cover functions, locals, members and system headers;
   - `applyToScopeSymbols` reads the previous name token (`jam::Document::getPreviousNameToken`, `jam_Document.h:1114`);
   - the filter is the `jam::Fuzzy` prefix pattern `^word`.
7. `Processor::moveRequestJobsToFront`: a request that is not ready moves the parse and index jobs of its unit and of the unit's prerequisites to the front. It marks them `Id::isRequested`. `runRequest` clears the mark.
8. The stdin wait: ruling 30.

**Open in Step 1:**
- T9: speed.
- T10: first answer.
- The tails of T11 (fuzzy all-scopes labels, `f(x).g` member access, locals), T15, T14/T4 and T12.
- A mermaid file for T28.
- CSS comment highlighting.
- The mermaid subgraph range: it ends at the last child, not at the `end` line.

**Withdrawn design.** COUNSELOR had proposed one serial context for the Relay's stdin, socket and stdout. ARCHITECT asked *"which design better? more universal and generic? more BLESSED? most importantly most DETERMINISTIC?"*. The reads gave these answers:
- `InterprocessConnection (false)` calls back *"on the connection's own thread"* (`juce_InterprocessConnection.h:75-79`). So the design keeps a hop, or it makes two writers on the one-writer ring (`jam_TerminalFIFO.h:16-17`), or it takes the request rows out of the Model (`Model::flush` is message-thread only, `jam_Model.h:926`).
- The current pattern stays. MCP Part 5 follows it.
- The T9 gap goes to measurement first (Part 8 Step 1 item 5).

## Tests

The verdict of every answer test is the code read at file:line (ruling 35). Where a pass condition below says "agree", "equals clangd" or "equals the clangd set", read it as: ling's answer equals the answer that a read of the code gives. clangd stays the reference for the feature set and for speed and CPU (T9, T10, T21).

**T1-T13** (carried verbatim from the deleted `RFC-clangd-parity.md:300-314`; the last column is the result of 2026-11-15):

| ID | Test | Command | Pass condition | Result this session |
|---|---|---|---|---|
| T1 | Index on a universal build | `ling project-info.md` in END; `ling project-info.md --debug` in KANJUT | no `unit index failed` line | 140 failures (END), 245 failures (KANJUT) |
| T2 | No build PCH read | `ling diagnostics project-info.md Source/ui/TabView.cpp` in END, with a build PCH older than a header | no `has been modified since the precompiled header` line; `~/.cache/ling/<daemon>/` holds the PCH that the check uses | fails |
| T3 | Index survives a restart | Kill the daemon, start it again, then run T4 | T4 passes after the restart | fails (Finding 3) |
| T4 | Definitions, END `debug` | `LING_WORD=--debug python3 compare.py end3.json` | every position with a non-empty ground truth is `agree`; no `ling-miss`, `disagree` or `partial` | 236 of 315 |
| T5 | Definitions, END default lane | T4 with `ccdir` built from `Builds/Release` and `LING_WORD` unset | the same as T4 | 0 symbols (Finding 1) |
| T6 | Definitions, KANJUT `debug` | `LING_WORD=--debug python3 compare.py kanjut.json` | the same as T4 | 0 of 26 declarations answered |
| T7 | References, END `debug` | `LING_WORD=--debug python3 refs.py refs-end3.json` | each set equals the clangd set, after Open Question 6 rules on locations only in LING | 30 of 40 |
| T8 | References, KANJUT `debug` | `LING_WORD=--debug python3 refs.py refs-kanjut.json` | the same as T7 | not run (Finding 1) |
| T9 | Query latency | the `lingTime` and `clangdTime` fields of T4 to T8 | LING time below clangd time, per operation, at the median and at p95 | LING median 0.206 s; clangd median 0.000 s |
| T10 | Start and restart latency | time from `ling daemon` to the first symbol answer; time of the first query after an idle stop | below clangd's time from file open to the first answer (0.6 s to 3.0 s this session) | 134.1 s cold; 19.1 s to 57.5 s restart |
| T11 | Completion | `ling completion` at the cursor positions of T4, against clangd `textDocument/completion` | the result set and its latency meet the rule of Open Question 8 | 1.30 s to 1.40 s; not scope-aware |
| T12 | Hover and inlay hints | `ling hover` and `ling inlayHint` against clangd `textDocument/hover` and `textDocument/inlayHint` on the T4 files | content meets the rule of Open Question 8; latency below clangd | name and scope only; 85 of 187 calls with parameters |
| T13 | Diagnostics on submodules | `ling diagnostics` on each submodule file of T4 and T6 | the output equals the exact-command clang check of the unit; time below clangd's open-to-diagnostics time | correct output; 6.27 s |

The Python commands are replaced. Each test runs as `nvim --headless -l test/<script>.lua <config.json>` (`test/compare.lua`, `refs.lua`, `hover.lua`, `inlay.lua`, `methods.lua`, `lsp.lua`).

**T14-T22** (carried verbatim from the deleted `PLAN-ling-clangd-replacement.md:121-131`):

| ID | Test | Pass condition |
|---|---|---|
| T14 | `textDocument/declaration` at the T4 positions | equals clangd under Risks 6 |
| T15 | `textDocument/typeDefinition` at the T4 positions | equals clangd under Risks 6 |
| T16 | `textDocument/documentHighlight` at the T7 symbols | equals clangd's set in the file |
| T17 | `textDocument/rename` at the T7 symbols | the edit set equals the T7 reference set |
| T18 | `textDocument/codeAction` on the T13 files | each clangd quick fix of a diagnostic is present |
| T19 | `textDocument/semanticTokens/full` on the T4 files | type and modifiers equal clangd at each token that gfx.lua:273-316 colours |
| T20 | `textDocument/switchSourceHeader` on the T4 files | equals clangd |
| T21 | CPU time at run time: user plus system time of the ling processes (daemon and server) and of clangd, over the same T4-T20 session, and over 60 s idle after the index ends | ling below clangd in both windows (*"less cpu usage (at runtime)"*) |
| T22 | Engine agnostic: a read of `Source/`, `cast/` and `jam_syntax_tree` for each concept of Context, Open 12 | no hit (*"not hardcoding anything C language family into its engine"*) |

**T23-T29.** T23-T27 are the IDs that `test/buffer.lua` and `test/keymaps.lua` report. The deleted close plan gave T23-T26 other meanings. The code is the source, so its IDs stand. The built-in language test of the close plan becomes T28.

| ID | Test | Pass condition |
|---|---|---|
| T23 | Buffer sync (`test/buffer.lua:144-149`: appear, modified, restore, disk) | an unsaved edit shows in `documentSymbol`; the answer follows the buffer, not the disk |
| T24 | Semantic tokens (`test/buffer.lua:230-254`: highlights, tokens, range) | each check passes |
| T25 | Selection range (`test/buffer.lua:355-364`: chain, outermost) | each check passes |
| T26 | The eight text objects (`test/buffer.lua:403-425`) | each range equals the tree-sitter capture of `queries/c/textobjects.scm` and `queries/cpp/textobjects.scm` |
| T27 | Keymaps (`test/keymaps.lua:138-271`) | no tree-sitter highlighter on a cpp buffer, tree-sitter kept on lua, the eight real keymaps, flash matches, the JAM highlight groups |
| T28 | Built-in languages: documentSymbol, definition and references on the markdown, css and json files of END's config folder, and on one mermaid file (`test/builtin.lua`) | each heading, selector and key of ruling 21 is listed; each link and edge reaches its declaration |
| T29 | Index progress (ruling 37) | from the first index job to the last, ling sends `$/progress` notifications, and noice in ARCHITECT's nvim shows them |

**Last results** (round 8, 2026-11-18, above): T4 284/300 agree, 8 disagree, 5 miss; T11, T12, T14, T15, T16, T17, T20 in the round 8 table; T22 passes; T23-T27 pass (21/21 and 17/17); T28 passes markdown and css, and skips json, xml, html and mermaid. Not run on the current build: T1-T3, T6, T8, T13, T21. No result recorded: T18, T19. T29 has no implementation.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps (Part 8)

### Step 1: acceptance, speed and CPU
**Action:**
1. COUNSELOR runs `cast cast/spell.md --no-sign` once for each batch of changes (ruling 41).
2. The harness asks ling only for answers (ruling 35). `test/compare.lua`, `refs.lua`, `hover.lua`, `inlay.lua` and `methods.lua` drop the clangd answer requests. COUNSELOR reads the code at each position of the fixed samples and records the expected answer once.
3. T1-T29 run on END and KANJUT over stdio. The runtime checks also run in ARCHITECT's nvim and through Claude's LSP tool, on the same installed ling (ruling 36).
4. Each failure goes back to its cause, by a read and a trace. A C-family cause is fixed in the language folder.
5. Each speed or CPU miss (T9, T10, T21) goes to a scout manifest that COUNSELOR writes, and the fix goes to the cost site that its report names.
   - **Manifest form:** the whatdbg smoke shape (`whatdbg/benchmark/smoke.md`; `smoke.sh:1-10`).
     - The `command` is a `.sh`. It runs `nvim --headless -l test/methods.lua <config.json>`, then writes the empty samples file, as `smoke.sh:9` does.
     - The manifest has a `## sample` table with `intervalMs` and `stackDepth` (profile skill, sample), and a `## toolchain` table with `cast | ../cast/spell.md --no-sign`.
     - COUNSELOR runs it as `sudo -n scout <absolute manifest path>`.
   - **Why this form:**
     - A profile build serves no LSP: under `JAM_USING_PROFILER`, `runManifest` creates no `Relay` (`Main.cpp:352-361` against `:362-364`).
     - The `--no-sign` build installs the Release ling (`CMakeLists.txt:488`) that the harness, nvim and Claude start (`~/.config/nvim/lua/lsp/ling.lua:23-27`; `~/.carol/.claude-plugin/plugin.json:16-18`).
     - The Where lane resolves function names without debug symbols (profile skill, Lanes).
   - **The verdict lane is unchanged.** T9 and T10 pass or fail in the nvim harness. The timing and the median and p95 come from `test/lsp.lua:100-109` and `:248-260`.
6. Each item of "Open items" is closed in this step, except the items that name Step 3 or Step 4.
7. Repeat until every test passes.
**Validation:** every answer test equals the code read. T9 for each method, at the median and p95, is below clangd. T10 is below clangd's time from file open to the first answer. T21 is below clangd. T22 has no hit. T29 shows the bar in nvim.

### Step 2: the switch — done 2026-11-19
**Done (ruling 36), each change read by COUNSELOR:**
- nvim:
  - `lua/lsp/clangd.lua` is renamed to `lua/lsp/servers.lua`. It holds `lua_ls`, `pyright` and `cmake`, and the attach handlers. The clangd entry, `clangdRootDir` and `M.restart` are deleted.
  - `plugins/lsp.lua:8-9` requires `lsp.servers`.
  - The `LingSwitchSourceHeader` command is created for a ling client (`servers.lua`).
  - `doc/KEYMAPS.md` deletes the `<leader>ls` row and names the group `lsp-ling`. `cast/spell.md:23`, `:30` guard it on `client.name == 'ling'` and the C-family filetypes. cast generated `lua/core/keymaps.lua:84-89`.
  - `lsp/cpp-stub.lua:132`, `:283`, `:457` find the `ling` client.
- Claude Code: `~/.carol/.claude-plugin/plugin.json:16-39` runs `ling project-info.md` for the C family and the six built-in documents. `~/.carol/bin/carol-clangd.js` is deleted.
- CAROL text: `CAROL.md:361`, `README.md:161`, `skills/doxygen-protocol/SKILL.md:3`, `:23-24`, `:44-47`, `skills/profile/SKILL.md:294-297` name ling.
- Not switched: `~/.carol/opencode/HARNESS.md:17` (OpenCode's `lsp`). No CAROL file configures the OpenCode LSP.
**Validation:** in nvim, a cpp buffer of END has one LSP client, ling, and every T1-T29 feature works from it. Claude's LSP tool answers from ling after a Claude Code restart.

### Step 3: audit
**Action:** @Auditor runs once on:
- ling `Source/`, `language/`, `cast/`, `test/`;
- jam `jam_syntax_tree/document/`, `jam_core/document/jam_Document.*`, `jam_core/json/jam_JsonDocument.*`, `jam_model_context_protocol/`, and the JAM parser files of Part 4 Step 1 and Part 6 Step 2;
- the whatdbg files of the `find` deletion (`Source/Processor.*`, `Launcher.cpp`, `BreakpointManager.cpp`, `Session.cpp`, `Session_mac.cpp`);
- the nvim files of Parts 6 and 8 and of Step 2.
The scope includes the ling files that MCP Parts 3-4 add ("Sync with the jam MCP plan"). The Auditor carry list is in "Open items", group C. COUNSELOR resolves each validated finding.

### Step 4: doxygen pass
**Action:** @Pathfinder writes the pass on COUNSELOR's targets. COUNSELOR reads each block against the code. The targets:
- each header that this plan changed;
- `Index.h:78`, `:85-90`, `:97`, `:217-232`. Their prose says that `addDocument`, `removeDocuments` and `setDocuments` change the symbol maps. `Index.cpp:91-99`, `:8-34` and `:306-318` do not;
- `jam_LanguageInterface.h:79`: "ten entries" over twelve names;
- `jam_JsonDocument.h:194`: the `contains` block names the deleted `find`;
- `Relay.h:154`, `:175`, `:242`;
- `Manifest.h` and `Language*.h` (tracker note of Part 5: the doc text is stale);
- `Server.h:14` and `:60` (the daemon timer, from the deleted `PLAN-ling-clangd-replacement.md:68`).

### Step 5: RFC update for MACHINIST — void
Ruling 36: *"no need rfc to machinist. just delegate to engineer."* Step 2 did the nvim and Claude work.

### Step 6: plan hygiene
**Done 2026-11-19 (ruling 33):** these files are deleted: `PLAN-clangd-parity.md`, `PLAN-design-conformance.md`, `PLAN-stdio-transport.md`, `PLAN-ling-10-diagnostics.md`, `PLAN-ling-11-swap.md`, `PLAN-ling-clangd-replacement.md`, `PLAN-units-contract.md`, `RFC-clangd-parity.md`, `PLAN-ling-lsp-1-language-runtime.md` to `PLAN-ling-lsp-7-parity.md`, and `PLAN-ling-lsp-8-close.md`.
**Open:** @Engineer deletes the stale hash-named libraries in `~/.cache/ling/language/cLang` after a read lists them. COUNSELOR sets each Status row.
**Validation:** the root holds `PLAN-ling-0-tracker.md`, this file, and no other ling plan.

### Step 7: log
**Action:** on ARCHITECT's "log sprint", COUNSELOR writes the sprint log per `/log` and the commit messages in chat.

## Open items

Each item names its source. A source in a deleted file is quoted.

**A. Acceptance (Step 1)**
1. The open list of round 8 (below): T9, T10, the tails of T11, T15, T14/T4 and T12, a mermaid file for T28, CSS comment highlighting, the mermaid subgraph range.
2. Not run on the current build: T1-T3, T6, T8, T13, T21. No result: T18, T19.
3. The two T4 misses of Part 5 at `jam_Document.cpp` 547:57 (`nextSibling`, inside a `jassert`) and 817:20 (`VariantType`). The code read decides each one (ruling 35). This session edited `jam_Document.cpp`, so read the positions again first.
4. T7 stale picks: `step4/end-def.json` holds old lines of `jam_Document.cpp` (`test/refs.lua:50`).
5. KANJUT build 28 printed `BinaryData151.cpp: unit index failed` once, and the failure path is not known. `index` returns `indexFailure` (1) for an empty `--version` install folder and for a libclang that does not load, and libclang's failure code is also 1 (T1).
6. Before Part 1, with no connection, the daemon quit 60 s after the files child was added, also during a cold index, and the `ThreadPool` destructor killed a long index job (exit 139). Check this on the current build (T1, T10).
7. The cancel path is not tested at run time. A query runs in the request listener, so a cancel acts only on a request that waits for its file.
8. A `get` that throws inside a pool job is swallowed by `juce::ThreadPool::runNextJob` (`juce_ThreadPool.cpp:387-394`), so the job never finishes and the daemon waits for ever. A job needs one boundary catch that sets the message and finishes the job.
9. `LanguageDocument::addElement` adds `Id::comment` and `Id::parameters` to every library element. Measure the swap size and the parse time against the baseline (scout).
10. Part 5 baseline: the tree was slower than before Part 4 (`end-cold-sample` epoch 2.3594e10 ns against 1.14988e10 ns). T9 and T21 measure it.
11. Built-ins not run on a project manifest: the mermaid edge end, the css selector (END's `style.css` has no `addElementStyle` rule), json, xml, html. The documentSymbol of END `theme/gfx/style.css` is empty (T28).
12. A `didOpen` before `initialize` is queued, not refused.
13. The `abs` records:
    - triplication at `language/cLang/language.cast` 90:61 (222 locations), cause not confirmed (`carol/SPRINT-LOG.md:146`);
    - the `cstdlib:134` `using ::abs` record is missing. The fix path is in the overloaded-declaration API (`carol/SPRINT-LOG.md:147`);
    - from the deleted `PLAN-clangd-parity.md:272`: *"Open: `getInstance` (`juce_Singleton.h:142`, one macro-body token that every singleton class declares) and `abs` (`using ::abs;`, `cstdlib:134`, one token for the overload set). Each is a token that stands for several entities, while symbol-per-token holds one."*
14. `construction.md:461` keeps `unsigned (int kind)`. NAMES Rule 6 bans `kind`, and ruling 29 keeps third-party names out of the ling contract. Rename it to `type` (ruling 41: *"including fixing `kind`"*).
15. Uncompiled-lambda locals (`juce_Component.cpp` 3698, 3969) and `expectEquals` (4160). Not investigated (`carol/SPRINT-LOG.md:149`).
16. The units contract ran Steps 1-4. Its reordered-manifest run was not run.
17. Confirm that the next build prints no `duplicate symbol 'Id::endLine'` warning.
18. T29: ling has no progress notification (a search of `Source/` and `cast/` for `progress` and `workDone` finds none).
    - Implemented 2026-11-19, engine only, language-agnostic. `Processor::setIndexed()` sets two `jam::Parameter<int>` values on the overlay row, `jobCount` and `pendingJobCount`, from the six job maps (`Source/Processor.cpp:352-359`; `Source/LINGModel.cpp:14-15`). The Server `progress` attachment, a sibling of `message`, sends `$/progress` to every connection (`Source/Server.h:131-140`). `Server::getProgress` picks the kind once and dispatches the value fields through `progressValues` (`Source/Server.cpp:76-136`).
    - No `window/workDoneProgress/create` request: nvim 0.12.6 accepts a token that the server did not create (`runtime/lua/vim/lsp/handlers.lua:55-86`), and noice reads the `LspProgress` autocmd (`noice/lsp/progress.lua:92-96`). A client response would also throw in Relay (`Relay.cpp:163`, `jam_JsonDocument.cpp:141`).
    - Tables: `cast/lsp.md` (`progress` method, `token` and `percentage` members, `WorkDoneProgressKind`), `cast/text.md` (`## progress`), `cast/identifiers.md` (`jobCount`, `pendingJobCount`), `cast/spell.md:207-215`, `:369-377`.
    - Result (scratchpad `progress-probe.txt`): 1838 `$/progress` events reached nvim, from `report 1795/5657` (31 %) to `end`.
    - A client that connected during an index run got `report` and `end` but no `begin`, so nvim showed no title (`handlers.lua:69-72` takes the title from `begin` only). Fixed: `Server::valueTreeChildAdded` sends `begin` to the connections when a connection joins and jobs are pending (`Source/Server.cpp:54-68`). `getProgress` takes the kind from its caller. The `begin` percentage is `done / total`, which is 0 at the start of a run. Result: the first event is `begin`, `title = "Indexing"`, 31 %. 2050 events end with `end`.
19. The first Claude Code session after a restart starts `ling project-info.md` from `plugin.json`. Check that Claude's LSP tool answers from ling.
20. `textDocument/definition` on the namespace `jam` (481 locations) costs about 20 ms in the daemon, against a T4 median of 0.21 ms. Measured 2026-11-19: 18 597 such requests took 6 min 26 s, with the `ling daemon` at about 100 % of one core and nvim at about 2 % (scratchpad `jam-probe.log`). The cost site goes to a scout manifest (Step 1 item 5).
21. Expected answers (2026-11-19). `test/expected/end-definition.json` holds 274 positions, each decided by a code read or by a seed that matched the code. Every JUCE path uses the `$juceRoot` token. The harness pins the lane with `toolchainArgument` `debug`. T4 END result: 271 agree, 3 disagree. The 3 disagree positions are item 15 (`juce_Component.cpp` 3698, 3969, 4160; `JUCE_UNIT_TESTS` code). The reference check is ARCHITECT's choice, *"Mechanical code check"*: each location's source line holds the word, and the definition from that location equals the definition of the queried position. T7 END result: 25 agree, 2 confirmed, 6 disagree, run time 6 min 20 s. A constructor declaration names its class, so a constructor token is a reference to the class, and the check flags it as `foreign` because its definition is the constructor (`jam_PaneComponent.cpp:12`, `:21`; `jam_PaneComponent.h:46`, `:55`, `:173`).
22. T7 defect, `retain` (`jam_Listener.cpp:26:52`, definition `NSObject.h:36`): ling's references include `os_retain (handle)` at `juce_AudioWorkgroup.cpp:131`, a line that does not hold the word `retain`.
24. T6 defect, a guess in a template body. `highPeak.process (channel, sample)` at `FilterStripAudioProcessor.cpp:257:22` is inside the template `FilterStripAudioProcessor::process<SampleType>` (`:248-249`). `highPeak` is `OrfanidisMSED` (`FilterStripAudioProcessor.h:399`), which is `kuassa::dsp::ChannelProcessorMSED<…>` (`:395`), so the definition is `kuassa_ChannelOpsBase.h:394`. ling answers with the enclosing `FilterStripAudioProcessor::process` (`:249`, `:424`, `:425`, `.h:229`). Ruling 35: *"ling MUST NOT GUESSED"*.
25. T6 defects in code that the lane does not compile (item 15 family), `kuassa_Listener.cpp`. The branches are `JUCE_MAC` `:12-113`, `JUCE_LINUX` `:116-273` and `JUCE_WINDOWS` `:276-391`. `folder` at `:157:33` (Linux branch) answers with all three platform members, `:104`, `:264` and `:385`; the code gives `:264`. `memcpy` at `:358:26` (Windows branch) has no answer; the code gives `_string.h:78`. `add` at `:415:17` (Linux branch; `watched` is `juce::OwnedArray<Impl>`, `kuassa_Listener.h:258`) has no answer; the code gives `juce_OwnedArray.h:324`. T6 KANJUT result: 290 agree, 1 disagree, 9 unread; median 0.22 ms, p95 2.52 ms; run time 1 min 1 s.
26. T8 KANJUT result: 33 agree, 7 disagree, run time 37 s. `FilterStripProcessor`, `Registry` and `AudioProcessor` flag constructor tokens only (constructor definitions and declarations, `new FilterStripProcessor()`, `using AudioProcessor::AudioProcessor;`, `: AudioProcessor (…)`, `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR`). Each one names the class, so ling's set is correct and is the expected set. `retain` is item 22 again. `move` (`kuassa_StyleManager.cpp:9:22`, definition `__utility/move.h:30`, 1018 references) has 193 `foreign` locations. All 1018 reference lines are read. 1017 hold a `std::move (` call, and the last one is the definition, `move.h:30`. The set is correct and is the expected set. The `foreign` flags come from call sites in dependent template code, where ling gives no definition. Harness rule: an expected entry (a code read) decides the verdict. The mechanical check decides only a position that has no expected entry yet (`refs.lua` `getVerdict`). The seeded `retain` sets held the defect line `juce_AudioWorkgroup.cpp:131` (`os_retain (handle);`). It is removed from `end-references.json` and `kanjut-references.json`, so item 22 still fails.
23. T7 defect, `addAndMakeVisible` (`jam_TabbedComponent.cpp:12:5`, definition the `Component*` overload, `juce_Component.cpp:1476`, `juce_Component.h:760`): ling's references include a call to the `Component&` overload, `parent->addAndMakeVisible (*pointer)` at `jam_ComponentUtils.h:33`, and header call sites whose definition resolves elsewhere: `juce_LiveConstantEditor.h:167`, `jam_CollapsibleSectionBase.h:98`, `:103`, `:184`.
27. Harness batch 2 (2026-11-19). `hover.lua`, `inlay.lua`, `methods.lua`, `buffer.lua`, `keymaps.lua` and `builtin.lua` ask ling only (ruling 35). clangd gives timing only, when a config has `reference`. Configs: `test/config/{end,kanjut}-{methods,hover,inlay}.json`, `end-buffer.json`, `end-keymaps.json`, `end-builtin.json`. Expected files: `test/expected/{end,kanjut}-{methods,hover,inlay}.json`. `test/lsp.lua` `resolvePath` is renamed `getFilePath` (NAMES Rule 6, `resolve`). END results: T17 33 agree; T20 10 agree after the seed (each pair read on disk); T14 seeded from the definition sets, because `textDocument/declaration` runs `Id::goToDefinition` (`cast/lsp.md:209`), and the diff is 271 equal and 3 item 15; T15 not seeded (typeDefinition needs its own code read: 53 equal the definition, 142 answer a type, 79 empty); T12, T16, T11 and inlay and hover not seeded. Hover median 116 ms (T9). T18: no ling diagnostic on the 10 END files, so no codeAction request ran.
    - END seeds, each from a code read (2026-11-19): T12 595 symbols (each name spelled at its `selectionRange`; a shifted-column control fails all 590 non-destructor entries); T16 32 positions (the code-read reference set in the queried file, one range for each occurrence; the 13 lines with two or more occurrences were read); T20 10; T14 274; T15 256; hover 247 (the name and the `Type:` line match the declaration, or the type is the code-read deduction of an `auto` or an enumerator). Results on the current build: T12 10 files agree (2026-11-19 run; median 0.45 ms, p95 2.82 ms), T14 270/4 (item 15), T15 255/1 (item 15), T16 32, T17 33, T20 10, `buffer.lua` 21/21, `keymaps.lua` 17/17.
    - Hover defect: a macro gets no hover (`jassert`, `jam_Viewport.cpp:23:5`, `juce_Component.cpp:884:9`; the code holds `#define jassert` at `juce_PlatformDefs.h:178`). Cause: `onMacro` passes `invalidPosition` to `addSymbol` and never calls `addHover` (`language/cLang/language.cast:4091`). `Processor::IndexJob::addSymbol` stores `Id::hover` only for a non-null hover (`Source/Processor.cpp:929-930`). `Query::applyToHover` skips a record with no hover (`Source/Query.h:64`). libclang `getCursorPrettyPrinted` prints declarations only (`CIndex.cpp:5873-5887`), so the macro definition text comes from the source bytes of the cursor extent. Fix in cLang: `addHover` gets a `SourceLocation` overload, `onMacro` adds a hover for the definition cursor, `getHoverMacroDefinition` gives `#define` and the source text, and `hoverWords` gets a `macro` row. clangd reference for `jam_Viewport.cpp:23:5` (clangd 17, plaintext): `macro jassert`, then `provided by "juce_core/system/juce_PlatformDefs.h"`, then the `#define` source text, then `// Expands to` and the expansion. libclang exports two macro functions, `clang_Cursor_isMacroFunctionLike` (`CIndex.cpp:4577`) and `clang_Cursor_isMacroBuiltin` (`CIndex.cpp:4589`). Neither returns expansion text. clangd answers `namespace juce` at `juce_Component.cpp:884:9`, because the file is not in `compile_commands.json` and clangd inferred its command from `juce_gui_basics_2.cpp`. The other null hovers are uncompiled code: `#if JAM_USING_PROFILER` blocks (`ConfigModel.cpp:73-75`, `:101-103`; `ENDView.cpp:155-158`), `JUCE_UNIT_TESTS` code (item 15), and the Linux and Windows branches of `jam_Listener.cpp` (item 25).
    - Open seeds: T11 (completion), T18 (code actions, which need a diagnostic), inlay hints.
    - KANJUT on the current build: T14 296/4 (items 24 and 25), T17 40. The other KANJUT tests wait for their seeds.
    - KANJUT seeds, each from a code read (2026-11-19): T12 240 symbols in 10 files (the name check gives 0 failures); T16 40 positions (9 positions have lines with two or more occurrences, and each line has one range for each occurrence); T20 10 (`FilterStripViewComponents.cpp` has no header beside it, so the seed is `null`; `methods.lua:297-299` gives `agree` when ling answers null). T15 and hover wait for the code read of their flagged rows.
    - KANJUT hover seed rules (2026-11-19): 17 `kuassa_Listener.cpp` keys sit in `#ifdef JUCE_LINUX` (`:116-273`), `#ifdef JUCE_WINDOWS` (`:276-391`) or `#if JUCE_LINUX` (`:413-416`). They are not seeded (item 25). `kuassa_ButtonSVG.cpp:4:62:f` and `:20:38:f` sit on the suffix of the literals `1.2f` and `0.75f`, so the seed is `null`. `FilterStripViewComponents.cpp:52:33:getDefault` calls through the template parameter `MapType`, so no one declaration answers, and the seed is `null` (ruling 35). The 41 rows that fail the mechanical type test are `auto`, `constexpr` or enumerator declarations, and each hover type matches the declaration line.
    - KANJUT T15 seed, from a code read of each declaration (scratchpad `K-T15-decl.txt`): 131 non-empty answers and 89 empty answers. The 89 are 25 namespace names, 33 variables of a builtin type, and 31 functions that return `void` or a builtin type. Not seeded: 14 `kuassa_Listener.cpp` Linux and Windows keys (item 25), and 2 uses of the template parameter `SampleType` (`FilterStripAudioProcessor.cpp:333:56`, `:268:40`). `coeffs` (`:415:9`, `auto coeffs { …getCoefficients() }`) answers `juce::Array`, while its hover type is the alias `Filter::Coefficients`. It is not seeded until the libclang read decides alias against canonical.
    - KANJUT T15 seed, second pass (2026-11-19): 10 more answers, from code reads and the clangd reference. `getReference` → `$juceRoot/modules/juce_core/files/juce_File.h:58`; `param` → `juce_AudioProcessorParameter.h:46`; `coeffs` → `kuassa_Filter.h:157`; `reg` → `kuassa_Registry.h:23`; `tabInfo` → `kuassa_ButtonBar.h:222`; `channelState` → `kuassa_Oversampler.h:207`; `fontsSection` → `juce_ValueTree.h:80`; `paths` and `NSArray` → `NSArray.h:17` (MacOSX26.5 SDK); `SampleType` (`:333:56`) → `FilterStripAudioProcessor.cpp:315`. The section holds 230 positions. Not seeded: `at` (`:412:50`, alias `ButterworthBase` or class `Butterworth`), `at` (`kuassa_StyleManager.cpp:107:22`, alias `Typeface::Ptr` or class `ReferenceCountedObjectPtr`), `label` (a binding from a dependent `HashMap` element), `sampleR` (a dependent `SampleType&`), `retain`, and the item 25 keys.
    - T15 defects, KANJUT, open: a class type answers `[]` at `FilterStripProcessor.cpp:43:35:getReference` (returns `const juce::File&`, `juce_Array.h:293`), `FilterStripAudioProcessor.cpp:384:65:param` (`auto& param`, `juce::AudioProcessorParameter *const &`), `:412:50:at` (`std::array::at`, `const_reference`), `FilterStripViewComponents.cpp:124:14:reg` (`auto* reg`, `kuassa::Registry *`), `kuassa_ButtonBar.cpp:140:13:tabInfo` (`auto& tabInfo`), `kuassa_Oversampler.cpp:61:9:channelState` (`auto& channelState`, `value_type &`), `kuassa_StyleManager.cpp:107:22:at` (returns `MappedQ&`), `:123:14:fontsSection` (`const auto& fontsSection`). Each type is an `auto` reference or pointer, or a template-substituted return type. The structured binding `label` (`FilterStripViewComponents.cpp:33:69`) and the Objective-C keys `NSArray`, `retain` and `paths` (`kuassa_Listener.cpp:26:19`, `:26:52`, `:54:10`) are the two families above.
    - Hover causes (cLang, 2026-11-19). `getNumChannels` (`:286:23`): `onReference` passes `getCursorReferenced (reference->cursor)` (`language.cast:3560`), which is a null cursor for a dependent member call. `hoverWords` gives its row zero, `variable`, for any kind it does not list (`jam_LookupTable.h:38-43`, `:157-160`), so the text is `variable ` with no name. Fixed: `addHover` returns `invalidPosition` for an invalid cursor (`language.cast:2604-2629`), and `onMacro` relies on that check. `SampleType` (`:333:56`): `addTemplateParameterSymbols` writes the record with `invalidPosition` (`language.cast:4599`). Fix: it calls `addHover` for the parameter cursor. `dsp` (`:424:64`): cLang has no `namespaceReference` cursor kind (`CXCursor_NamespaceRef = 46`, `Index.h:1311`) and no `cursorRecords` row for it, and the indexer gives no `onReference` at this position. Open.
    - Type causes (libclang, read from llvm release/17.x and main `CXType.cpp`). `clang_getCursorType` on a `BindingDecl` returns the binding type (`:256-257`). `clang_getTypeDeclaration` has no `ObjCObjectPointer` case (`:572-577`). `clang_getPointeeType` handles it (`:497-499`). Fix: `ValueCategory::objCObjectPointer = 109` joins `isIndirectType`. `CursorType::unexposedDeclaration` joins `isTypedDeclaration`, and `onBinding` calls `addTypeDefinition`, as `onDeclaration` does (`language.cast:3543`).
    - clangd 17 reference on KANJUT (2026-11-19, `Builds/Debug/compile_commands.json`, which holds `FilterStripAudioProcessor.cpp` and `FilterStripProcessor.cpp`). `getNumChannels` (`:286:23`): hover `instance-method getNumChannels`, `provided by "juce_dsp/juce_dsp.h"`, `→ size_t (aka unsigned long)`; typeDefinition empty. `SampleType` (`:333:56`): hover `template-type-param SampleType`; typeDefinition `FilterStripAudioProcessor.cpp:315` (`template <typename SampleType>`). `dsp` (`:424:64`): hover empty and typeDefinition empty, so ling's null agrees, and `dsp` is not a defect. `getReference` (`FilterStripProcessor.cpp:43:35`): typeDefinition `$juceRoot/modules/juce_core/files/juce_File.h:58` (KANJUT's JUCE copy is `~/.cache/cast/juce/d72cb6bb…`). `coeffs` (`:415:9`): hover `Type: Filter::Coefficients (aka Array<Coefficient>)`; typeDefinition is the alias at `kuassa_Filter.h:157`, so ling's canonical `juce::Array` answer is a defect. `kuassa_ButtonBar.cpp` is not in `compile_commands.json`, and clangd inferred its command from `kuassa_animation.cpp`, so its `tabInfo` answer is not a reference.
    - `coeffs` cause: `getDeclarationValueType` replaces an `auto` type with its canonical type (`language/cLang/language.cast:3291`), and a canonical type has no alias. libclang `clang_getTypeDeclaration` unwraps `Type::Auto` through `getDeducedType`, which keeps the typedef (`CXType.cpp`, `case Type::Auto`). `clang_getPointeeType` unwraps `Type::Auto` the same way. Fix: `getDeclarationValueType` returns `getCursorType` unchanged, and `reportTypeOf` takes the pointee of an `auto` type whose pointee is valid. Run check: `auto` smart pointers through `getSmartPointee` (`:3239-3245`), because `GetTemplateArguments` reads `getAs<TemplateSpecializationType>` and `getAsCXXRecordDecl` (`CXType.cpp:185-197`, release/17.x), and I have not read whether those see through `auto`.
    - Crash, fixed in source (2026-11-19): the daemon died with SIGSEGV at address 0x8 (`~/Library/Logs/DiagnosticReports/ling-2026-11-19-174105.ips`). The faulting thread runs `addHovers` → `addDefinitions` → `getHoverDefinition` → `getHoverMacroDefinition` → `clang_getFileContents`. A built-in or command-line macro has no source file, and `isPreprocessed` admits a null file (`language.cast:2541`). Both ling probes saw the dead daemon as `daemon connection lost` (nvim `lsp.log`). Fix: `getHoverMacroDefinition` returns no text when the file is null. clangd also prints no `#define` line when the definition range is unavailable (`Hover.cpp:824-834`).
    - `getNumChannels` parity, open: after the `addHover` fix ling answers null, while clangd resolves the dependent call to `AudioBlock::getNumChannels`.
    - Hover defects, open: `FilterStripAudioProcessor.cpp:333:56:SampleType` (a use of the template type parameter in a lambda in a template body) and `:424:64:dsp` (the `juce::dsp` namespace in an explicit instantiation) answer null. `:286:23:getNumChannels` (`block.getNumChannels()` in a template body; the definition is `juce_AudioBlock.h:242`) answers `variable ` with no name.
28. documentSymbol defect: a class template lists no members. `jam_Array.h` gives `jam` (Namespace) and `Array` (Class) only, though `~Array` (`:57`), the move constructor (`:75`) and `operator=` (`:93`) have bodies. Non-template classes list their members (`ENDView.h` 33 methods, `jam_Document.h` 188). The gate is `isOutlineParent` (`Builds/languages/cLang/language.cpp:7936-7941`), which needs the hash of the member's lexical parent to equal the hash that `reportOutline` stores for the container (`:7979-7980`). This fails T24 function-name, T25 chain and outermost, T26 function.outer and inner (`test/buffer.lua`), and T27 function.outer, inner and matches (`test/keymaps.lua`). Each of these found its function from clangd's documentSymbol before the switch.
    - Cause (libclang source, llvm-project main): a class template reaches `indexDeclaration` as its templated `CXXRecordDecl` (`clang/lib/Index/IndexDecl.cpp:727-738`; `clang/tools/libclang/CXIndexDataConsumer.cpp:1012`). `clang_getCursorLexicalParent` maps a member's parent to the `ClassTemplateDecl` (`CIndex.cpp:9167-9169`, `:9232`). `clang_hashCursor` hashes `(kind, data[0])` (`CIndex.cpp:6776-6783`), so the two keys differ. `CXIdxDeclInfo::lexicalContainer->cursor` is built with no template mapping (`CXIndexDataConsumer.cpp:1188`).
    - Fix: `language/cLang/language.cast` adds `IndexContainerInfo` (it mirrors `CXIdxContainerInfo`), types the three container fields with it, and `isOutlineParent` and `getOutlineParent` read `declaration->lexicalContainer->cursor`. Result: `jam_Array.h` lists 55 methods and 6 constructors. `buffer.lua` 21/21, `keymaps.lua` 17/17.
    - Second defect, same output: a template constructor or destructor name runs past the name (`Array` at `:53` prints 46 bytes, the length of `Array<ElementType, TypeOfCriticalSectionToUse>`). `reportOutline` takes `std::strlen (entityInfo->name)`. Fix: `getOutlineNameLength`, the length of the name as the source spells it at the location.
    - `Builds/languages/cLang/language.cast` is an old copy that the build does not read (`CMakeLists.txt:387-389` names `language/cLang/`). It holds the first fix only.
    - Third defect, same output: a declaration whose name the source does not spell at its location (`JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ENDView)`, `ENDView.h:269`) got name length 0, and two such records merged under one key (`Source/Processor.cpp:970`). The engine reads each outline name as a view of the document (`Source/Query.cpp:731`). Fix in the cLang library only (ARCHITECT: *"dont hardcode C language into engine must be universal"*): `reportOutline` reports a declaration only when its spelled name length is above 0 (`language/cLang/language.cast:3151`, `:3156`).
    - Fourth defect, from the second fix: `getOutlineNameLength` read the file contents, and `IndexContext::getFileContents` is empty for a file that did not enter through the main-file or included-file callback (`language.cast:2556-2563`, `:4819-4828`). `substr` then throws inside a libclang callback, and `test/buffer.lua` hung at its first documentSymbol request (580 s). Fix: the name length comes from the owner API, `clang_Cursor_getSpellingNameRange` (`construction.md:483`), as `language.cast:4439` and `:4497` already use it. No file contents are read for the name.
    - Fifth defect, from the fourth fix: for a declaration that a macro makes, the spelling name range spans the macro call, so `ENDView.h:268` listed `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ENDView)` as a method and a field. Fix: `getOutlineNameLength` tokenizes the name range and reports a length only when the first token begins the entity name (`language/cLang/language.cast:3091-3107`; token API as `addFunctionReturnHint`, `:4497-4511`). Result: `buffer.lua` 21/21, `keymaps.lua` 17/17, and `jam_Array.h` lists `Array`, `~Array` and `operator=` by name.
29. T15 (typeDefinition) END, code read of 221 positions where ling differs from the definition (scratchpad `T15-lines.jsonl`):
    - Defect, engine, fixed: on a namespace qualifier, `Query::typeDefinition` retried with the last name token (`Source/Query.cpp:877-880`, removed), so `Id` in `Id::display` answered `juce::Identifier`. 23 positions. Ruling 35: *"ling MUST NOT GUESSED"*.
    - Defect, open: a structured binding gets no type (`TabView.cpp:295:41` `action`, `jam_PaneComponent.cpp:147:9` `menu`). Cause: `onBinding` (`language/cLang/language.cast:4556-4573`) adds the binding record and reports no type. `onReference` passes the binding cursor (`unexposedDeclaration`) to `addTypeDefinition`, which accepts only `isTypedDeclaration` and `isSelfTypeDeclaration` kinds (`:3317`, `:3288-3302`). `Query::typeDefinition` then reads `typeSymbol == noSymbol` and answers `[]` (`Source/Query.cpp:840`). The fix waits for a read of libclang `clang_getCursorType` on a `BindingDecl`.
    - Defect, open: an Objective-C object pointer gets no type (`jam_Listener.cpp:26:9`, `NSArray* paths`, declared at `:106`). `ValueCategory` has no Objective-C kind (`language.cast:136-152`), so `isIndirectType` (`:3202-3206`) gives false. The definition of `paths` at `:26:9` and `:54:12` also answers nothing, while `queue` at `:33:9` answers `:107`. The fix waits for a read of libclang `clang_getTypeDeclaration` and `clang_getPointeeType` on `ObjCObjectPointer`.
    - Item 25 family: the Linux and Windows branches of `jam_Listener.cpp` (`:215`-`:384`) have no answer. Item 15 family: `juce_Component.cpp` 3698, 3859, 3969, 4160.
    - Seed error, corrected: `juce_Component.cpp:3531:46:bounds` held `NSView.h:139`, `CALayer.h:207`, `CARenderer.h:76`. The code declares `Rectangle<int> bounds` at `:3519` (`JUCE_UNIT_TESTS` code). ling answered the same three ObjC properties, so T4 scored it agree. It is a guess by name in item 15 code.
    - Expected: `test/expected/end-methods.json` `typeDefinition` holds 256 positions. The 18 open positions above stay unread.
    - Fixed (2026-11-19, not yet built and measured): bindings, Objective-C object pointers and template type parameters report a type. `auto` keeps its alias. A dependent reference takes the indexer's entity cursor. The causes and the libclang reads are in item 27.
    - Defect, from the `auto` fix, fixed: every daemon ended about 3 s after start, with signal 4 (SIGILL) and no crash report. With `LIBCLANG_DISABLE_CRASH_RECOVERY=1` it ended with signal 10 (SIGBUS). Under whatdbg it did not end (400 s). A temporary depth print in `reportTypeOf` showed `depth 200 kind 118 auto`. Cause: for an `auto` with no deduced type, `clang_getPointeeType` returns the input type (llvm-project `clang/tools/libclang/CXType.cpp:504-509`, `:514`). The condition `getPointeeType (type).Type != invalid` (`language.cast:3267`) then recursed without end, and the 8 MB libclang thread stack overflowed. Fix: `type.Type == ValueCategory::typeAuto and isIndirectType (libclang.getCanonicalType (type))`. The canonical type of an undeduced `auto` is the `auto` itself. The diagnostics are removed.
    - Probe after the fix: KANJUT `getNumChannels` `instance-method`, `SampleType` `template-type-param` and `:315`, `dsp` null, `coeffs` `kuassa_Filter.h:157`, END `paths` `NSArray.h:17`, `jam_Viewport.cpp:82:56` null. All agree with the reference.
    - Defect, open: `coeffs` (`FilterStripAudioProcessor.cpp:415:9`) answers `kuassa_Filter.h:157` two times. `Query::typeDefinition` writes one location for each record at the token (`Source/Query.cpp:838-863`). `goToDefinition` also has no duplicate filter (`Source/Query.h:289-296`).
    - Defect, fixing: the `jassert` hover word is `variable`. The core tables index keys directly with 256 slots (`language/core/core.cast:298`, `:669-681`, `:705-717`), and `jam::LookupTable` drops an entry key at or above `Capacity` (`jam_LookupTable.h:99-102`). `CursorType::macroDefinition` is 501 (`language.cast:132`). Fix: capacity 512.
    - Bindings, reference read: clangd answers `TabView.cpp:295:41` `action` with `__tuple/tuple_element.h:28` and `jam_PaneComponent.cpp:147:9` `menu` with `__utility/pair.h:569`, the `tuple_element<…>::type` typedef. ling answers `[]`. Cause: the type declaration is a typedef in an instantiated class, and `clang_getSpecializedCursorTemplate` has no typedef case (`CIndexCXX.cpp:148-176`), so the USR is the instantiation's USR. Fix: `getTemplatePattern` maps a member of an instantiated class to the member of the same kind and the same location in the parent's pattern (`getMemberPattern`, `language.cast:3310-3328`; `clang_equalLocations`, `construction.md:427`). An instantiated member keeps the location of its pattern member. A member of an explicit specialization has its own location, so it keeps its own declaration.
    - Defect, fixed: two tables had a first row that is not neutral, and `jam::LookupTable` gives the first row to every key that it does not name (`jam_LookupTable.h:38-43`, `:94-95`). `templateParameterTypes` gave `classToken`, so `addTemplateParameterSymbols` added a second, class-typed record for every type, template and declaration reference. This was the cause of the two `coeffs` locations. `cursorRecords` gave `addBindingSymbols`, so every cursor that the table does not name recorded its unexposed children as variables, while `onCursor` already tests for `nullptr` (`language.cast:4686`). Fix: a neutral first row in each table (`construction.md:703` `unexposedExpression | nullptr`, `:789` `unexposedDeclaration | 0`), as the other cursor tables have (`definitionKinds`, `hoverTypes`, `hoverScopes`).
    - Harness after these fixes (2026-11-19): END T12 10, T14 270/4, T15 255/1/18, T16 32/1, T17 33, T20 10, the same as before. KANJUT T14 296/4, T15 221/9/70, T16 40, T17 40, T20 10. Open KANJUT T15 positions: `getReference` 43:35, `getNumChannels` 286:23, `param` 384:65, `reg` 124:14, `tabInfo` 140:13, `cursor` 344:53, `NSArray` 26:19, `channelState` 61:9, `fontsSection` 123:14. The hover runs were not read.
    - Probe after these fixes: `coeffs` one location `kuassa_Filter.h:157` and definition `:412`. `action` `tuple_element.h:28`, `menu` `pair.h:569`, `jassert` `macro jassert` with the `#define` text. All agree with clangd. clangd also prints `provided by` and `// Expands to`, which have no libclang export (item 30, group 3).
    - KANJUT T15 code read of the 9 open positions (2026-11-19). ling probe: scratchpad `probeC.result`. clangd probe: scratchpad `probeRC.result`. clangd answers the 4 project files. For the 5 module `.cpp` files, clangd hover answers `namespace kuassa`, so clangd has no translation unit for them, and those seeds come from a code read.
    - Defect, fixed: a substituted template parameter type got no type. libclang gives `CXType_Unexposed` for it (llvm-project `clang/tools/libclang/CXType.cpp:126-127`). `clang_getTypeDeclaration` has no case for it (`:573-578`), and `clang_getPointeeType` returns an invalid type (`:510-512`). Positions: `getReference` (`ElementType`), `param` (`auto&` over `Array<AudioProcessorParameter*>`), `reg` (`ObjectType*`, `kuassa_Instance.h:113`), `tabInfo`, `fontsSection`. Fix: when a type has no declaration and its canonical type is of another kind, `reportTypeOf` continues with the canonical type (`language/cLang/language.cast:3264-3287`). This also covers an `auto` that deduces a pointer, so the old `typeAuto` condition is removed. A type that has a declaration keeps it, so an alias stays an alias (clangd `XRefs.cpp:2340-2342`).
    - Defect, fixed: an Objective-C class name (`[NSArray arrayWithObject:]`, `kuassa_Listener.cpp:26:19`) got no type. `CursorType` had no Objective-C kind. Fix: `objCInterfaceDeclaration = 11` (`CXCursor_ObjCInterfaceDecl`, `clang-c/Index.h:1220`; `language.cast:107`), added to `isSelfTypeDeclaration` (`:3312-3314`).
    - Defect, fixed: every reference in a local variable initializer had two records, so definition, declaration and typeDefinition answered each location two times, and documentHighlight answered each range two times. The harness compares sets, so it did not show this. Cause: with function-local indexing, `VisitDeclStmt` indexes the declaration group (`clang/lib/Index/IndexBody.cpp:384-388`), `VisitVarDecl` indexes the initializer (`IndexDecl.cpp:310`), and the visitor then walks the `DeclStmt` children again. Runtime check: `FilterStripProcessor.cpp:40:51` (initializer) answered two locations, `FilterStripAudioProcessor.cpp:72:27` (statement) answered one. Fix: `IndexContext::addSymbol` adds each occurrence (file, offset, symbol, role) once, through `ling::WordSet occurrences` (`language.cast:2534`, `:2574-2606`).
    - Defect, fixed: a member of a class template or a member function template took its type from the first instantiation that the index met. The indexer reports a reference to an implicit instantiation against its pattern (`IndexingContext.cpp:405-411`, `:220-237`), so the symbol key is the pattern USR. `onReference` computed the type from `clang_getCursorReferenced`, which is the instantiation, and `typedSymbols` keeps the first type for each symbol (`language.cast:3358`). After the substituted type fix, END `Session.cpp:40:22` `at` (`jam::HashMap<jam::UUID, std::unique_ptr<PluginState>>`, `Session.h:211`; `at` is a member template, `jam_HashMap.h:3326-3327`) answered `AudioProcessorGraph::NodeID` from another instantiation. Fix: `onReference` computes the type from the indexer entity cursor, which owns the USR (`language.cast:3613`). Hover keeps the instantiation cursor, because hover is stored for each occurrence.
    - Seed error, corrected: `kuassa_Document.cpp:344:53` `cursor` held the class `juce_CharPointer_UTF8.h:46`, and `kuassa_Oversampler.cpp:61:9` `channelState` held the class `kuassa_Oversampler.h:207`. clangd points at an alias when the type spells one (`XRefs.cpp:2340-2342`). clangd check on a project file with the same shape, `FilterStripAudioProcessorRegistration.cpp:183-184` (`for (auto& channel : output.channels)`, `std::array`, `kuassa_ChannelOpsBase.h:81`): clangd answers `array:183` (`value_type`), and ling answers the same (scratchpad `probeG.result`). New seeds: `cursor` `juce_String.h:201` (`using CharPointerType = CharPointer_UTF8;`), `channelState` `array:183`.
    - Harness after these fixes (2026-11-19): END T15 255/1/18, T14 270/4, T16 32/1, T17 33, T20 10. KANJUT T15 225/5/70 before the seed correction, T14 296/4, T16 40, T17 40, T20 10.
    - Open, cause read: three KANJUT T15 positions differ because clangd answers from the expression type at each occurrence, and ling stores one type for each symbol (`Source/Processor.cpp:1035-1039` `typeSymbols.addOrReplace`, copied to each record at `:876-884`).
      - `FilterStripProcessor.cpp:43:35` `getReference`: clangd `juce_File.h:58` from the instantiation `Array<File>`. The symbol is the pattern `Array::getReference`, and its type is `ElementType`, so ling answers `[]`.
      - `FilterStripAudioProcessor.cpp:286:23` `getNumChannels`: a call in a template body. clangd answers `[]`; ling answers `size_t` (`__stddef_size_t.h:18`) from the declaration.
      - `kuassa_Document.cpp:560:16` `entry` (`std::unique_ptr<Element>&`, `kuassa_Owner.h:49`): clangd's rule gives the pointee and the smart pointer (`XRefs.cpp:2363-2369`). ling keeps one type, the last report, `Element` (`kuassa_Document.h:225`). The seed holds `unique_ptr.h:130` only (code read; clangd has no translation unit for this file).
    - The pass condition of T14 and T15 is "equals clangd under Risks 6". `PLAN-ling-clangd-replacement.md` is deleted (ruling 33), it has no commit in git history, and this file's Risks section says "None". The text of Risks 6 is not readable.
    - Fix path read: a type for each occurrence needs the language callback interface to carry it. The interface is JAM's `jam::LanguageCallbacks` (`~/Documents/Poems/dev/jam/jam_syntax_tree/document/jam_LanguageInterface.h:61-81`, `languageVersion` 7 at `:12`). `addSymbol` (`:69`) has no type field, and `addTypeDefinition` (`:78`) takes a symbol, not an occurrence. The change reaches JAM. Correction: `jam_syntax_tree/document/` is a named artifact of this plan (Step 3 audit list; Step 4 targets `jam_LanguageInterface.h:79`), so the change is in scope.
    - Design, from CONTRACT: the record carries its types when it is created (MANIFESTO E, Complete at Creation). Today `addTypeSymbols` adds `Id::typeSymbol` to complete records after the pass (`Source/Processor.cpp:871-885`). The type of an occurrence comes from that occurrence's referenced cursor, as clangd takes the expression type (`XRefs.cpp:2267-2275`). A dependent call has no type, as clangd answers `[]` for `getNumChannels`.
    - Hover harness on this build: END 247 agree / 27 unread, KANJUT 280 / 20, the same as before.
30. clangd case map (2026-11-19). ARCHITECT: *"we dont translate byte-to-byte no? at least we could learn what we're missing instead of one by one probing"*. Source: `~/Documents/Poems/dev/llvm-project` (llvm main `6ea5f558`, sparse checkout extended with `clang-tools-extra/clangd`, `clang/lib/Sema`, `clang/lib/Index`, `clang/include/clang/{AST,Index,Sema}`). Three code reads listed every case: hover (`clangd/Hover.cpp` 1-1893, 132 cases), typeDefinition, definition and documentHighlight (`clangd/XRefs.cpp`, 251 cases), and target lookup with the libclang indexer (`clangd/FindTarget.cpp`, `Sema/HeuristicResolver.cpp`, `Index/Index{Body,Decl,TypeSourceInfo,ingContext}.cpp`, `tools/libclang/{CXIndexDataConsumer,Indexing}.cpp`, 287 cases). Each case falls in one of three groups. All fixes are in `language/cLang`, and the engine stays language-agnostic.
    - Indexer facts that set the design:
      - The libclang indexer resolves dependent names with `HeuristicResolver` and reports a reference when there is exactly one target (`IndexBody.cpp:186-211`; `IndexTypeSourceInfo.cpp:212-219`).
      - The reported entity carries the resolved declaration's cursor (`Index.h:6335`; `CXIndexDataConsumer.cpp:1142`). cLang now uses that cursor when `getCursorReferenced` resolves to nothing (`language.cast:3565`).
      - `clang_indexSourceFile` maps only `IndexFunctionLocalSymbols` and `IndexImplicitTemplateInstantiations` (`Indexing.cpp:413-420`). `IndexTemplateParameters` stays false, so the indexer drops every template parameter reference (`IndexingContext.cpp:95-99`). That is why cLang adds template parameter records in `onCursor`.
      - The indexer skips an explicit or implicit instantiation and its whole subtree (`IndexDecl.cpp:790-795`; `IndexingContext.cpp:160-194`). clangd also answers nothing for `dsp` in `template void …process<float> (juce::dsp::AudioBlock<float>&)`.
      - Every occurrence is reported at its file location (`CXIndexDataConsumer.cpp:164-167`).
    - Group 2, libclang exports the API and cLang does not use it (`libclang.map`; `construction.md` wraps 107 of 430 exports):
      - Hover `Size:`, `Offset:`, padding and `alignment` lines for a record or field under the cursor (`Hover.cpp:1038-1076`, `:1537-1547`). APIs: `clang_Type_getSizeOf`, `clang_Type_getAlignOf`, `clang_Type_getOffsetOf`, `clang_Cursor_getOffsetOfField` (`libclang.map:69`, `:153`, `:166-167`).
      - typeDefinition through an array to its element type (`XRefs.cpp:2349-2350`). APIs: `clang_getArrayElementType`, `clang_getElementType` (`libclang.map:231`, `:293`). `isIndirectType` holds only pointers and references (`language.cast:3212`).
      - Hover and definition on an `#include` line (`Hover.cpp:1334-1345`; `XRefs.cpp:878-879`, `:290-291`). API: `clang_getIncludedFile` (`libclang.map:308`).
      - typeDefinition of a lambda answers its call operator's return type (`XRefs.cpp:2355-2358`). cLang status not read.
      - Hover on `this`, a string literal and `__func__` (`Hover.cpp:981-1003`). cLang status not read.
    - Group 3, no libclang export:
      - Macro `// Expands to` (`Hover.cpp:838-860`, `syntax::TokenBuffer::expansionStartingAt`). The only macro exports are `clang_Cursor_isMacroBuiltin` and `clang_Cursor_isMacroFunctionLike` (`libclang.map:91-92`).
      - The `provides` list on an `#include` hover (`Hover.cpp:1289-1310`, include-cleaner).
      - The textual index fallback for definition (`XRefs.cpp:928-956`). This is an engine feature, not a libclang one.

**B. Doxygen (Step 4):** the targets are in Step 4.

**C. Auditor carry (Step 3)**
1. Part 2: 45 functions of `language.cast` cross 30 lines or 3 branches (list: `scratchpad/fn_all.txt`). Each crossing goes through the auditor.md Lean ladder.
2. Part 5: `SwapReader::addPayload` and `readValue` take a `Target&` (CODING.md "No out-parameters"); `SwapReader::read<ScalarType>` has four compile-time cases; `Query::hover` is over 30 lines.
3. Part 6: `Query.cpp` size, `getUnbracedBodyEnd` (four tests), `Document::getArena`, `ParseJob::runJob` length, the duplicated `Token` stream of JSON, `test/buffer.lua` (450 lines).

## Windows (ARCHITECT)

Canon ruling 78. Each Windows run is ARCHITECT's, on the Windows host, and it blocks no step.
- MCP Part 4: `ling/benchmark/stdio-client-windows.md` (jam MCP tracker, Part 4 row).
- The ling Windows build and checks of each part.
- whatdbg: `benchmark/smoke-windows.md`.

## BLESSED Alignment

- **B:** one owner for each artefact: ling owns its carried libraries and writes them to the cache; the daemon owns the documents; the harness owns each server process and ends it with `shutdown`.
- **L:** one plan, one test table.
- **E:** the version, the hash stamp and the lock make each build state explicit. Each test states its pass condition.
- **S (SSOT):** one document type, `jam::Document`; one language contract for extensions and in-house documents; one LSP for ling's languages after the switch.
- **S (Stateless):** each test reads the answers only.
- **E (Encapsulation):** the engine names no language; each language is a library behind the C boundary (T22).
- **D:** the same sources, manifest and libraries give the same index and the same answers.

## Risks / Open Questions

None. Each open item has its step.
