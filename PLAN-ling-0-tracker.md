# PLAN: LING — Tracker and Canon

**RFC:** none — objective from ARCHITECT prompt. ARCHITECT removed `RFC-ling.md`; the rulings are in Canon.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

This file is the index, the status tracker, and the canon of the LING work.
LING is the Language Index Navigation Graph. For each source file it holds navigation points, and it holds the links between them.
The numbered files together are one PLAN, in smaller parts. ARCHITECT reviews the whole PLAN and locks it as one (ruling 38).

## Status

| Part | Plan | Subject | Status | Depends on |
|---|---|---|---|---|
| 0 | this file | Tracker, canon, inventory | Written. Locked 2026-11-11 (ruling 81). | |
| 1 | `PLAN-ling-1-project.md` | The console project, from the cli archetype | Written. Locked 2026-11-11 (ruling 81). No open item. | 0 |
| 2 | `PLAN-ling-2-language-tables.md` | The `jam_syntax_tree` module: `CodeDocument` and its nine tables moved in. Each new table row comes in the part that holds its first reader. | Written. Locked 2026-11-11 (ruling 81). No open item. | 0, 1 (Step 7 builds LING) |
| 3 | `PLAN-ling-3-document-segments.md` | The `Document` engine: line comments and escapes in `getTokens`; the C family vocabulary sets them | Written. Locked 2026-11-11 (ruling 81). No open item. | 2 |
| 4 | `PLAN-ling-4-language-document.md` | `CLanguageDocument`: the token rows it reads, tokens, the declaration tree, and `ling <file>` to print it | Written. Locked 2026-11-11 (ruling 81). No open item. | 1, 3 |
| 5 | `PLAN-ling-5-links.md` | References and relations as metadata of the file tree; links to locals and parameters | Written. Locked 2026-11-11 (ruling 81). No open item. | 4 |
| 6 | `PLAN-ling-6-manifest.md` | The state model in END's form: `LINGModel` with one row per file, the file watcher and the compiler manifests as authors, `Processor` as the listener that holds one document per row; `jam::SyntaxTree`; `Validator`, the check of the manifest before the read (ruling 79); `CompilerManifest`, the table from a language family to the read of its compiler manifest, with its words in `ling/cast/c_language.md` (rulings 73 and 74); the files of each unit from the dependency record of the build, read with the `ninja` toolchain row and `-t deps` (rulings 75 and 76) | Written. Locked 2026-11-11 (ruling 81). No open item. | 1, 5 |
| 7 | `PLAN-ling-7-queries.md` | The lookups across the documents of all files, the nine operations, and the name search | Written. Locked 2026-11-11 (ruling 81). No open item. | 5, 6 |
| 8 | `PLAN-ling-8-daemon.md` | The shared daemon (rulings 29, 46, 47, 48): create or join over 127.0.0.1, one instance for each manifest and toolchain argument, its own timer ends it, with a default length that a manifest row overrides (ruling 71), checked by the `daemon` rule of `Validator` (ruling 79); `jam::ChildProcess::startDetached`; `jam::Format::appendHex`; the port file and the folder of LING at `~/.cache/ling/` (ruling 58); a connection and a request are rows; the manifest, each compiler manifest, and each dependency record file are watched (ruling 77) | Written. Locked 2026-11-11 (ruling 81). No open item. | 7 |
| 9 | `PLAN-ling-9-completion.md` | Completion and snippets: `Id::comment` and `Id::parameters` written by the parse; the scope walk with a name match; the request parses its file first (rulings 14, 45; MANIFESTO **D**) | Written. Locked 2026-11-11 (ruling 81). No open item. | 7, 8 |
| 10 | `PLAN-ling-10-diagnostics.md` | Diagnostics (rulings 49, 50, 51): the compiler manifest's own compile command of a unit, with the check flags of its language family, which are rows of `ling/cast/c_language.md` (ruling 74); LING's own precompiled header; the compiler's lines, unchanged | Written. Locked 2026-11-11 (ruling 81). No open item. | 6, 8 |
| 11 | `PLAN-ling-11-swap.md` | The swap (rulings 15, 54, 55, 58 to 69): one swap file for each source file under `.cache/ling` at its repository root; `Document::write` and `read`; the roots from `project-info.md`; a file is parsed only when its bytes or the version of LING changed | Written. Locked 2026-11-11 (ruling 81). No open item. | 6, 8, 10 |

Work outside the numbered parts:

| Work | Owner |
|---|---|
| nvim Lua, the Claude Code harness entry, the doxygen-protocol skill | MACHINIST (CAROL.md Roles). ARCHITECT: *"user want nvim integration? they need to write their lua script, plugins whatever."* |
| Checks on a Windows machine | A step inside each part that needs one. ARCHITECT runs it, and it blocks no part (ruling 78) |

The part list is a proposal. ARCHITECT changes it at will.

## The contract of every part

ARCHITECT, verbatim:

- *"our framework is SOPHISTICATED, this is FACT. jam + juce, can build anything. we already build our own crossplatform DAP (whatdbg), profiler (scout), codegen and meta-toolchain (cast), so this is not unchartered territory. this is just another puzzle into our integrated development environment. there is no new pattern to be reinvented, no foreign semantics from your garbage corpus need to be written, the codebase has anything you need as building blocks OOTB"*
- *"our framework is SOPHISTICATED and RICH. we have full stack end-to-end working pattern. most of it already measured, profiled, optimised. Even Document already designed to be O(log n), so there is no new pattern to reinvent. only need to put the puzzle pieces together, use existing API, or extend following the established pattern verbatim"*
- *"each part is executable action, executive summary READY to be implemented with thorough inventory"*
- *"there's a parallel session working on Function::Map refactor, it should not changed your understanding about our objectives."* Each part uses the `jam::Function::Map` form that `jam_core/function_map/jam_Function.h` declares at the time of execution.
- Ruling 50: *"dont use juce::String nor juce::StringArray except for: - Input, when juce API return - Final output, juce API args demands"* / *"always use jam::Format and jam::Strings"*. Text is `std::string_view` over the bytes where they are, and `std::string` where a part creates text, built with `jam::Format`. A list of strings is `jam::Strings`. A `juce::String` exists only where a JUCE API returns one, or where a JUCE API takes one as an argument.
- Ruling 51: *"my point is always jam FIRST, then juce. jam may have juce counterparts. this is not re-implementation. ours always measured and profiled against juce native. ours always FASTEST, no copy, no alloc, MOST BLESSED compliant"*. Each part uses the JAM type where one exists: `jam::Array`, `jam::HashMap`, `jam::Owner`, `jam::Strings`, `jam::Format`, `jam::Wyhash`, `jam::File`, `jam::ChildProcess`, `jam::Subprocess`. A JUCE type is used only where JAM has no counterpart: `juce::JSON`, `juce::Timer`, `juce::InterprocessConnection`, `juce::ThreadPool`, `juce::MessageManager::callAsync`, `juce::ValueTree`, `juce::File`.
- cast runs. In `jam/`, `cast cast/spell.md` formats and generates; "JAM has no build of its own" (`jam/.claude/CLAUDE.md:103`). In `ling/`, each "Run cast" of a part is `cast cast/spell.md --debug` (PLAN-ling-1-project.md Step 10). It formats, generates, and builds: cast has no flag that stops the build (`archetype/ARCHETYPE.md:147-153`). A build is ARCHITECT's, or COUNSELOR's on ARCHITECT's explicit request (CAROL.md Build; `jam/.claude/CLAUDE.md:104`).
- The two manifests (rulings 73 and 74). "Manifest" names the `project-info.md` of a project only. It is the manifest of cast and of LING, as it is (rulings 65 and 72). The "compiler manifest" of a language is the file that the build of that language writes; for the C family it is `compile_commands.json`. `LINGModel`, `Processor`, and `Main.cpp` name no language, no build tool, no file form, and no flag word. `CompilerManifest` is the one LING object that names a family together with its compiler manifest, and its words are rows of the table file of that family in `ling/cast/`. A new language is a new table file and a new entry (ruling 23). The files of a unit come from the dependency record that the build of that language wrote (ruling 75); LING walks no `#include`, holds no search path, and asks no compiler.
- Windows (rulings 16 and 78). *"code must crossplatform with branching only allowed at inevitable seam where each platform are genuinely differs"* / *"just fucking implement it with fact you have, then development there will proof what works"*. Each part writes both platform bodies of a seam from the cited facts. The Windows step of each part is ARCHITECT's, and it blocks no other step and no other part.

## Language / Framework Constraints

- LANGUAGE.md, C++ / JUCE: one object is one header and one `.cpp`. The 300-line threshold is a smell detector for responsibility count, not a file rule.
- CODING.md: no includes in a submodule header or source file. The topmost module header holds every include.
- CODING.md: no anonymous namespace, no `namespace detail`, no out-parameter, no fake-carrier struct, no local struct, no forward declaration.
- NAMES.md Rule -1: a new domain term needs ARCHITECT's approval. A name from an established family (Rule 5) needs none.
- NAMES.md Rule 6: `kind` is banned. The codebase word is `Type`.

## Canon: rulings, verbatim

### From the removed RFC

ARCHITECT removed `RFC-ling.md` on 2026-11-11: *"i deliberately deleted RFC, since the PLAN should be executable. RFC only will becomes noise."*
The parts cite four of its rulings. Their text in this PLAN is the verbatim fragments below. No other RFC text is carried.

- RFC ruling 4: *"definition, jump, etc always correct"* / *"ULTRA FAST indexing"* / *"not hogging cpu nor memory"*
- RFC ruling 8: *"our tools only `compile` what compiler need"*
- RFC ruling 9: *"our own COMPLETE AST"* / *"definition, jump, snippet, completion"*
- RFC ruling 11: *"completion and snippet support yes. i think it goes hand in hand. our codebase + juce already have best example on well structured and documented code"*

### Session 2026-11-10

13. *"you know we have sophisticated jam::Document to build any AST"*
14. *"isnt this identical to what we actually doing with TextEditor? a swap file + metadata as AST for operation?"* / *"the file is not swap, but the actual source code"* / *"we only build AST for its AST, the graph, the relations, the docs, the deps"* / *"all deterministically from any build manifest"*
15. *"still we could store the index in swap file no? not temporary, but as shared index to read?"*
16. *"must be crossplatform."*
17. *"although our project are mostly C++, the design should be generic and universal, could be any language"*
18. *"then why do we need to link? why not just dynamically load whatever environment use? using ling as tools means the environment already have the infra for building anything no"*
19. *"my point is this probably isomorphic to cast with its manifest. cast doesnt care about the language, the  environment, its user's responsibility to setup, to define, to declare, cast only execute."*
20. *"so ling need table manifest OR read project manifest + additional optional metadata, user define their environment, cast only generate graph. for our project specifically, ling could read compile_command.json + project_info.md for example, we dont need extra manifest."*
21. *"user want nvim integration? they need to write their lua script, plugins whatever. ling will deterministically provide the graph, the data, the feedback."*
22. *"C lang family is the MVP now. since this is ours. but i want this to be language agnostic as DESIGN BY CONTRACT, because we want future extensibility and scalability is just another table as rules to add. NOT to be hardcoded in ling source, not only for C lang, and definitely not need to be redesign each time we add new language support."*
23. *"my over simplistic understanding the AST parser to build per language is just another Rules, Tokens, Vocabulary for jam::Document, table driven, generated by cast. C lang family now is the FIRST CITIZEN, the native, the priority. other language could be deferred, could be added in the future, simply as adding new table, new function to register."*
24. *"so the ISOMORPHIC ABSTRACTION as UNIVERSAL and GENERIC : Language Index Navigation Graph as its SPEC contract live to its name semantically ACCURATE."*
25. *"my point is probably something like the already established pattern, for now we need: - C table Rules, Tokens, Vocabulary - then perhaps CDocument as parser, AST"* / *"in the future same pattern - Lua ... - LuaDocument"* / *"and so on"*
26. *"the convention for naming `language` document probably better like : CLanguageDocument, LuaLanguageDocument, etc"*
27. On a shared server: *"~/Documents/Poems/dev/endless/ probably not the best example, but we have done this in the past with old END daemon server. at least it's working"*

### Session 2026-11-10, Step 1 (open rulings)

ARCHITECT answered by option selection. Each ruling quotes the selected label and the option text as presented.

28. Open item 1, supersession: *"Tables + declared front end"*. Option text: "CLanguageDocument from cast tables is the MVP. A front end library is loaded only when the user's manifest declares one (ruling 18, ruling 19)."
29. Open item 2, process shape: *"Shared daemon"*. Option text: "endless nexus pattern (Daemon.h:60-62, ruling 27): one process, many clients over 127.0.0.1, a port file, and a lock."
30. Open item 3, location: *"we need dedicated module to hold all `language` documents. we alrady have xml/html/css/markdown/mermaid probably web isnt the right place for css/html? since IIUC it only holds docs?"* / *"jam_syntax_tree? for present and future programming language"* / selected *"jam_syntax_tree + CodeDocument"*. Option text: "Same module. CodeDocument (jam_core/document, C/Python/shell/JS tokenizer) also moves into it as a sibling programming-language document."
31. Open item 4, "Rules": *"Validator::Rules"*. Option text: "The existing Validator::Rules (jam_Document.h:841) checks the built tree after build(); construction stays in dispatch tables."
32. Open item 5, MVP constructs: *"All 13 + macros"*. Option text: "The 13 measured types plus #define names."
33. Open item 6, unit of a document: *"One unit, spliced"*. Option text: "One document per compile_commands entry, its #include files spliced in with Document::appendChildren (jam_Document.h:1086), as cast's Model does."
34. On prior art: *"those reference only to learn the isomorphic pattern. we have our own CONTRACT."* / *"DESIGN BY CONTRACT"* / *"ask only questions no read can answer"*
35. On tables: *"we probably alrady have static table vocabulary for various programming language that we use to tokenize/colourize syntax highlight. we should not add new pattern, new table, instead scale, extend those table for whatever ling need"*

### Session 2026-11-10, Part 2

36. On the home of the nine tables that only `CodeDocument` reads (`SyntaxTokenType`, `Family`, `CppKeyword`, `JsKeyword`, `PythonKeyword`, `code`, `markup`, `data`, `languageFamily`): *"Move with CodeDocument"*. Option text: "The nine tables move to a new cast/syntax_tree.md, generated as generated/jam_SyntaxTree.h and .cpp. Basis: the sibling shape of jam_web, which owns web.md and jam_Web.h (jam_web.h:29). DocumentTokenType, Byte, tokens and markupLanguage stay in core.md because other modules read them."

37. On C tokens. Finding: `Document::getTokens` has no escape rule and no line comment (`jam_Document.cpp:385-386`, `:558-564`; `jam_Document.h:784-785`). ARCHITECT selected *"Fix the engine (Recommended)"*. Option text: "Document::Vocabulary gains a line-comment literal and an escape byte; Document::getTokens honours them. CodeDocument and every quoted language become correct too. Basis: MANIFESTO D, the invariant is set once at the owner; the owner of segmentation is getTokens. Blast radius: the eight Document types that implement getVocabulary." Part 3 sets the two values for the C family only; each other owner sets them when it needs them (MANIFESTO **L**, YAGNI).
39. On the names of the `SyntaxTokenType` rows that collide with C++ keywords: *"Token suffix"*. Option text: "classToken, structToken, enumToken, namespaceToken. Family: the `## tokens` literal `classToken` (cast/core.md:779-838) and the CssRuleType rows that end in the table noun, such as styleRule (cast/web.md:102). The other ten rows stay plain." The plain rows are `identifier`, `function`, `instanceMethod`, `staticMethod`, `constructor`, `destructor`, `variable`, `staticVariable`, `field`, `enumConstant`, `macro`.
40. On the shape of the documents. ARCHITECT first selected *"One document per manifest (Recommended)"*, then replaced it: *"why not one document per file?"* / *"one Document per file is more deterministic no? we dont need to track which file to which tree CMIIW"* / *"one document per file, but the tree also have relationship state as metadata. why is that not the contract?"* / *"but mermaid AST is not materialisation either. it's always metadata-only. mermaid materialised into diagram. while with code, the data already materialised, docs only store the tree"*. Result: one `jam::Document` for each source file. The tree holds the relations of that file as metadata. Ruling 33 ("One unit, spliced") is replaced. By MANIFESTO **E**, Complete at Creation, and **B**, a file tree holds no link into another document; the lookup across files is a read by the store (Part 7).
41. On the type that holds the table from a language family to its parse function: *"New type jam::SyntaxTree"*. Option text: "A new type named after the module holds the table and the entry. `CodeDocument` and `CLanguageDocument` do not change. `SyntaxTree` is a new domain term that you approve here."
42. On names and design: *"i still dont like fresh. you keep introducing new semantics."* / *"resident cold evict and rehydrate are foreign semantics, pre existing still violates contract"* / *"jam::Document already have established lingua, derived from our sibling juce::XmlDocument."* / *"wrong name usually means wrong mental model, wrong design,"* / *"DESIGN BY CONTRACT"*. Result, by the CONTRACT: (a) the source file is the Model (ruling 14); a document is its tree, "built once, read in place" (MANIFESTO **E**, Complete at Creation); (b) a file form of a tree is "A second copy of the same complete data" (MANIFESTO, Still forbidden; **S**, shadow state), and a test that compares the two copies is the symptom of that shadow state; (c) the only permitted exception is closed-list item 4, "Coalescing or layout conversion at creation ... A measurement must show the gain"; (d) every lookup uses the lingua of `jam::Document`: `getRoot`, `getChildByID`, `parent`, `properties`, `getOrCreate`; (e) a name with no family is corrected in the design, not ratified (NAMES.md Rule -1). Two written parts, a binary store and its validity test, are withdrawn.
43. On the command form: *"The nine LSP tool names"*. Option text: "Subcommands named as the Claude Code LSP tool names them: goToDefinition, findReferences, hover, documentSymbol, workspaceSymbol, goToImplementation, prepareCallHierarchy, incomingCalls, outgoingCalls. Output is JSON through cast's JsonWriter (cast/Source/JsonWriter.h:14-66)."
44. On the daemon: *"do no translate or port byte-to-byte. just because it works, does not mean it was the best example. our architecture and topology now significantly cleaner, organized, structured fully event driven"*. The `endless` nexus of ruling 27 is not the model. Part 8 follows the architecture that `end/ARCHITECTURE.md:880-947` states.
45. On the state model: *"READ. END is now FULLY EVENT DRIVEN, APVTS, Model"* / *"then the SSOT model is FileWatcher to ValueTree, listener react"*. On what the tree holds, ARCHITECT selected *"A row per file (Recommended)"*. Option text: "The tree holds one row for each source file, with its path. The watcher writes the row. A listener reacts and replaces the jam::Document of that file, which it holds in a map. This is the Session form: a row names a file in Id::plugin, and the listener loads the object for it (end/Source/plugins/Session.cpp:59-69). Basis: ruling 42, every lookup uses the jam::Document lingua; the document is the one form of the AST."
46. On the transport of Part 8. Finding: END and JAM have no transport between processes. ARCHITECT selected *"JUCE connection, 127.0.0.1"*. Option text: "Ruling 29 text: juce::InterprocessConnectionServer in the daemon, port file, juce::InterProcessLock. A message adds a REQUEST row; Id::result goes back on that connection. A thin stdio `ling` relays for stdio-only clients (RFC-ling.md:223)."
47. On the start and the life of the daemon. Finding: no JUCE or JAM API starts a detached child (`juce_SharedCode_posix.h:1122-1154`; `jam_ChildProcess_posix.cpp:45-58`). ARCHITECT: *"must be crossplatform. although with windows we're mostly using msys2. what is setsid? PID?"* / *"so what is the most BLESSED solution?"* / *"why the contract, not isomorphic to LIFO? SharedInstance? Vulkan device?"* / then `/go`. Result: the daemon has the form of `jam::SharedInstance`, "create-or-join ref-counted holder ... last drop destroys" (`jam_SharedInstance.h:1-3`). A client joins a live daemon or starts one. A connection is a holder. The last connection ends the daemon. The start is detached, through a new static `jam::ChildProcess::startDetached` in the native pair of `jam::ChildProcess`.
48. On the life of the daemon. Finding: a CLI call connects, answers, and disconnects, so "last drop destroys" ends the daemon after each agent call. ARCHITECT: *"this is trivial no? - virgin run? spawn server - still running, parallel agents need access? shared - last agents exit, nothing using it? kill"* / *"ling no way to see an agent, but it know its own lifetime. multiple instance is not allowed. each handover whatever request is checking should it kept alive or kill itself"*. On the length of the quiet period, ARCHITECT selected *"project-info.md row"*. Option text: "The user declares it, as rulings 19 and 20 give the environment to the user: one row in a project-info.md table." Result: each request restarts the daemon's own timer; when it fires with no request pending, the daemon ends. "Last drop destroys" of ruling 47 is replaced.
38. On the lock: *"nothing is locked yet. we'll review the whole plan as PLAN. since multi-part is A PLAN. only broken down into smaller chunks."*
49. On diagnostics. ARCHITECT: *"then the design is wrong. ARCHITECT need diagnostics, especially with vim. that's exactly why we develop LING in the first place. using nvim is top most priority especially accurate diagnostics using juce. because NO LSP can give accurate diagnostics ESPECIALLY using juce. the closest one is IDE with xcode or msvc, even those are sometimes false alarms. saying ling cannot diagnose is absolute bullshit. we're on environement where we can build DETERMINISITICALLY. of course we know what to compile, any language have manifest, we can read the manifest"*. On the PCH: *"why running manifests pch entry need to writes to build tree"*. On the format: *"what can format both nvim and agents could understand without circus of translating and parsing"*. On a part: *"4. yes"*. Result: Part 10. LING runs the manifest's own compile command of a unit with declared check flags, and passes the compiler's lines through unchanged.
50. On text types: see The contract of every part.
51. On JAM first: see The contract of every part.

### Session 2026-11-11

52. On item 17, the JAM run APIs that take `juce::StringArray`: *"Change the JAM signatures"*. Option text: "`start`, `launch`, `startDetached` take `jam::Strings`. `jam::Strings` gets a public view of one element. The scout callers change too (`Toolchain.cpp:21`, `Benchmark.cpp:110`). This widens the scope to JAM and scout."
53. On the first `timeout` value: *"1 minute. why need to parse again? not memoized? cant we just store index to file per project? swap? indexed when file changes"*. Result: the row `timeout` holds `60000`.
54. On the persistent index: *"then the design is wrong. index is in swap file, persistent, only parsed when changed always relative to project. Model tree isnt second truth, it's ephemeral state when daemon running, needed as event driver. unless we could rely entirely on FileWatcher, but then we need to reinvent whole machinery of event to callback. UNIDIRECTIONAL: file - watcher - model - callback, what's the issue with this? this is exactly settings/config pattern CMIIW"* / *"my rulings as principles stand corrected, i dont find any conflicting rationale."* Result: ruling 15 stands. Results (b) and (c) of ruling 42 were ORACLE's reading of the CONTRACT, not ARCHITECT's words, and they are withdrawn. Results (a), (d), and (e) of ruling 42 stay, and so does ARCHITECT's rejection of the foreign names. The index is a swap file that stays on disk. A file is parsed only when it changed. The model tree is state that lives while the daemon runs, and it drives the events. The chain is one way: file, watcher, model, callback, as the Config Chain of END (`end/ARCHITECTURE.md:129`, `:684-703`). A write to disk is permitted copy 1, "OS or device boundary" (MANIFESTO **E**, Permitted copies). Part 11 holds the swap index.
55. On the place of the swap index: *"deps shared cache? at module root(repo), juce, jam, kanjut, cium, whatever. project cache? at project root (repo). ling does not care, project, modules, swap is swap, cache is cache, if project deps are modules, ling build all as complete AST, because project always `linked` to deps including modules"*. Result: each repository root holds the swap index of its own files. The project root holds the index of the project files. A module root (JUCE, JAM, KANJUT, CIUM, or any other) holds the index of that module, and every project that uses the module reads that one index. LING makes no difference between a project and a module. The complete AST of a project is its own index and the index of each dependency. The roots are declared rows: `project-info.md` `## index` gives `@juce-path` and `@user-module` (`scout/project-info.md:9-10`), and `## user module` gives one row for each module (`scout/project-info.md:123-142`).
56. Replaced by ruling 58. On the swap index of the system headers, which are in no repository (`## system include`, PLAN-ling-6-manifest.md:198): *"~/.config/ling/cache/"*. Ruling 72 removes the `## system include` table.
57. Replaced by ruling 58. On the folder of LING's own files outside a repository: *"system wide env ~/.config/... everything goes here. what's the issue? why we need root cache?"*. Result: LING has one root on the machine, the config root (`CAST_CONFIG_DIRECTORY`, PLAN-ling-8-daemon.md:248). The precompiled header of Part 10 and the index of the system headers are in its `cache` folder (ruling 56). `~/.cache/ling` and `%LOCALAPPDATA%/ling` of Part 10 came from the shape of `CAST_JUCE_CACHE_DIR` (`jam/cast/juce.cast:8-12`), not from a ruling, and they are withdrawn.
58. Correction of rulings 56 and 57: *"correction. make it consistent as cast. system wide env at ~/.cache/ling/"*. Result: rulings 56 and 57 are replaced. LING's files outside a repository are in `~/.cache/ling/`, in the shape of cast's cache root: `$ENV{HOME}/.cache/cast/juce`, and `$ENV{LOCALAPPDATA}/cast/juce` on Windows (`jam/cast/juce.cast:8-12`). The folder of Part 10 stands. Part 8 holds its rows and `LINGModel::getCacheDirectory()`, and the port file of Part 8 is in the same folder (PLAN-ling-8-daemon.md Step 2 item 2; In LING: `Server` and `Connection`). The index of the system headers is in the same folder.
59. On how LING learns the root of a file. ARCHITECT on the patched JUCE: *"end compiles from cache, because we patch juce, while keeping the juce source code vanilla. what's wrong with index at repo root? should we make ling `patch` aware? still universal and generic? language agnostic?"* Then: *"options:: 1 explicit table manifest, like cast, scout, ggwp (always win) 2 without 1,  specifically to our project, ling use project-info.md as its manifest. isnt all already defined there?"* Result: LING reads the roots from a manifest, never from a tool marker. An explicit table manifest wins when the user gives one. With none, LING reads the `project-info.md` of the project. This is the form of ruling 20. LING holds no knowledge of a patch.
60. On the patched JUCE folder. ARCHITECT: *"who is sole author of  patch path?"* / *"why not ling ask cast then? isnt cast can return table entry? it's not defined as table entry? dynamically created?"* Facts given: cast is the sole author (`jam/cast/juce.cast:1-71`); CMake computes the hash at configure time (`juce.cast:22`); no table holds the path; every input is a table entry (`end/project-info.md:64`, `:70`; `jam/user-modules-info.md:1344`). Two sources were named: 1, the compile manifest, whose `-I` flags name the folder; 2, the tables, with the hash rule of `juce.cast:14-25`. ARCHITECT selected *"2"*. Result: the patched folder comes from the tables, not from the `-I` flags.
61. On who computes the patched folder: *"LING computes"*. Option text: "LING reads the version row and the patch rows and applies the same rule with `juce::SHA256`. cast does not change. The rule then exists in two places: `juce.cast` and LING."
62. On the test for a changed file: *"Hash of the bytes (Recommended)"*. Option text: "The swap record holds `jam::Wyhash::hashBytes` of the source text. The job loads the text, as it does today, and compares the hash. Equal bytes give an equal hash (MANIFESTO D). No second read of the file."
63. On the place and the shape of the swap at one root. Options given: one swap file per source file, or one index and segments in the form of `TerminalSwap`; both in `<root>/.cache/ling/`. ARCHITECT: *"which is more BLESSED? most deterministic? more importantly FASTEST?"* Facts given: one file per source file is selected by MANIFESTO **B** (two daemons share a module root, ruling 55), by "A state update replaces the slot. It does not edit it.", and by determinism (the swap bytes depend on one source file only); the speed is not measured; the index form is a coalesced layout, permitted copy 4, "A measurement must show the gain". ARCHITECT: *"next"*. Result: one swap file for each source file, at the same relative path under `<root>/.cache/ling/`. The index form waits for a scout measurement.
64. On the swap of the patched JUCE files: *"At the vanilla JUCE root, by hash"*. Option text: "`Poems/JUCE/.cache/ling/<hash>/`. The swap is at the repository root of JUCE (ruling 55), and the hash keeps patch sets apart." The hash is the hash that names the patched folder (rulings 60 and 61). The JUCE root is the `jucePath` row (`end/project-info.md:64`).
65. On the manifest of LING. On the root rows of an explicit manifest, ARCHITECT selected *"Same rows as project-info.md"*, then: *"essentially like the following: - both cast and ling manifest is exactly identical : project-info.md - project that not using cast, but using ling must define its `project-info.md` but only table/data what ling actually need - so the logic business of parsing both cast and ling actually IDENTICAL, only differs on what to do with the data - since our project is using cast, our ling got the manifest for free"*. Result: LING has one manifest form, `project-info.md`. The file of a cast project serves LING as it is. A project with no cast writes a `project-info.md` that holds only the tables that LING reads. One read path serves both. The two options of ruling 59 are one file form.
66. On how LING reaches the patch rows. ARCHITECT: *"discuss first. how can we make consistent then? put what both cast and ling need at project-info.md? might change cast engine?"* / *"SSOT"*. Facts given: the patch list has one place, `jam/user-modules-info.md`, `## submodule`, rows with `module` = `patch`; a copy in each `project-info.md` breaks MANIFESTO **S**; cast fixes the file name for every root, "Each root carries a `user-modules-info.md` file at its top level" (`cast/Source/HELP.md:85`; `cast/Source/generated/Generated.cpp:119`); the root is the `@user-module` row of `project-info.md` (`end/project-info.md:10`). The form given: LING reads `project-info.md`, takes the `@user-module` root, and reads `user-modules-info.md` at that root; no row moves, the cast engine and the spell files do not change; a project with no `@user-module` row has no patched folder. ARCHITECT: *"ok."*
67. On a swap file from another build of LING. ARCHITECT: *"why FileWatcher checking bytes is not suffice?"* / *"ling binary change? why parser changed matter?"* / *"trivial."* / *"each swap that ling produce must stamp its version to swap metadata. SSOT. each project already declared. so different swap with different binary invalidated"*. Result: each swap file holds the version of the LING that wrote it. A swap file with another version is not read; the file is parsed again. The version is the value that the project already declares; no second number exists.
68. On the value of the stamp of ruling 67: *"versionString only"*. Option text: "`ProjectInfo::versionString`, the `versionString` row of `project-info.md`. The swap files stay valid until you change that row. Two builds with one version read each other's swap files."
69. On a swap that does not fit. ARCHITECT: *"why swap even matter it always replaced when source code changed no?"* / *"if it was mismatch Validator should show error, suggest to prune and rebuild. meaning Document NOT ALLOWED to write GARBAGE DEFENSIVE PROGRAMMING. no bullshit jassert anywhere, no excessive checking garbage, always trust validator. why overcomplicate?"* / then `/go`. Result: a changed source file replaces its swap file; this is the normal flow. Every other mismatch is a failure of a `Document::Validator` rule, shown as a message that says to remove the swap folder and build again. The job repairs nothing. `Document::write` and `Document::read` hold no assert, no check, and no failure path. By the established pattern: the stamp is root metadata of the document, as "the version value of the root metadata block" (`jam_MarkdownDocument.h:482`); the rule set is a `Document::Validator` in the family of `AnsiValidator`, `XmlValidator`, `CssValidator`; a failure is a `setMessage` and the document is kept (PLAN-ling-6-manifest.md, In LING: the documents). The silent parse-and-write on a version mismatch, the text row `failSwapWrite`, and the `bool` result of `write` are withdrawn from Part 11.
70. On the first argument of `ling`. Finding: Part 6 kept `ling <manifest>` and `ling <source file>`, and no part gave the test between them. ARCHITECT selected *"File is always the manifest"*. Option text: "From Part 6 on, an existing file as first argument is the manifest. The source file print of Parts 4 and 5 leaves Main.cpp in Part 6. PLAN-ling-6-manifest.md:198, its Step 7 item 5, and PLAN-ling-7-queries.md:146 change. No family test in Main.cpp."
71. On the length of the daemon timer: *"daemon timeout should have default value, table definition override."* Result: LING holds the default, 60000 ms (ruling 53). The `timeout` row of a `## daemon` table in the manifest of the project overrides it. The "project-info.md row" of ruling 48 is that override.
72. On the manifest. On a new `project-info.md` table for the include flags: *"why cant we use project-info.md as is? this is overengineered. a manifest to build now polluted with ling table. WHY?"* Then: *"my point is you keep conflating project-info.md is shared manifest. NO. project-info.md IS the manifest. cast and ling use the exact same manifest."* / *"there is no such thing as ling project. a project is project. ling declare metadata table for it to generate AST."* / *"MY COMPILER CAN BUILD ALL PROJECT WITH CAST, why the fuck a compiler that can build binary miss anything for an language server to build AST, it does not make any sense"*. Result: the `project-info.md` of a project serves LING as it is (ruling 65). `ling/project-info.md` is the build manifest of the tool; no part adds a table that LING reads at run time to it, or to the manifest of another project. Open after this ruling: the source of the compiler's own search paths (Part 6, `## system include`), the home of the check flags (Part 10, `## diagnostics`), and the home of the include flag words (Part 6). Ruling 74 closes the second and the third: they are rows of `ling/cast/c_language.md`. Rulings 75 and 76 close the first.
73. On the command line of `ling`. Finding: the PLAN had `ling <manifest> [project-info.md]`, and its word "manifest" meant `compile_commands.json`. ARCHITECT selected *"project-info.md only"*. Option text: "`ling <project-info.md>`. LING takes the compile database from the build tree that a `## toolchain` flow names (`-B <folder>`), the default flow or a flow word as cast takes it (`--debug`). One argument. LING then reads one cmake flag from the toolchain rows." Facts behind it: `## toolchain` is `| argument | command | flag |`; the default flow is the rows with an empty `argument` cell, and `--<word>` selects the rows whose `argument` cell is that word (`cast/Source/HELP.md:18`, `:24`, `:539`, `:960`); END's flows name `-B Builds/Release` and `-B Builds/Debug` (`end/project-info.md:118`, `:128`); `CMAKE_EXPORT_COMPILE_COMMANDS` is `ON` (`archetype/cli/cast/cmake.cast:7`). Result: "manifest" names `project-info.md` only. The file `compile_commands.json` is "the compile database". Ruling 74 replaces that word: the file is the compiler manifest of the C family. LING reads the toolchain rows through the entry of that family, and no LING code names `cmake` or a flag.
74. On the languages and the two manifests. ORACLE's question on the source of the compiler's own search paths named a CMake file and a C++ compiler. ARCHITECT: *"assuming that ling only with cmake and only for C++ is wrong. ling is language agnostic, just like cast. this is the non-negotiable contract. C++ is first citizen, but the ISOMORPHIC ABSTRACTION must HOLD."* / *"each language has its own `manifest` since we're using cmake, ninja, the compiler manifest is `compile_command.json` but ling's MANIFEST is project-info.md == cast's manifest"*. Findings given: Parts 2 to 5 hold the abstraction, and Part 6 has the form already, `documentConstruction` keyed by `map::Family`; Part 6 read `compile_commands.json` in `LINGModel` with no family key; `-I`, `-isystem`, and `-iquote` were LING text rows with no family; ruling 73 said "LING then reads one cmake flag"; Part 10 held the check flags in a `project-info.md` table. Direction given: one entry for each family holds all that belongs to its compiler manifest (where the file is, from the `## toolchain` rows of the toolchain argument; the read of the file; the include flag words; the check flags); `cmake`, `-B`, `compile_commands.json`, and the flag words become rows of the C family entry; the entry lives in LING, keyed by `map::Family`, because it has one user (CODING.md, narrowest scope) and `jam_syntax_tree` depends on `juce_core` and `jam_core` only; `LINGModel`, `Processor`, and `Main.cpp` name no file form, no flag word, and no build tool; only the C family entry is written now (ruling 23). ARCHITECT: `/go`. Result: the LING type `CompilerManifest` (PLAN-ling-6-manifest.md, In LING: the compiler manifest), the table file `ling/cast/c_language.md`, the root properties `Id::toolchainArgument` and `Id::manifest`, and the command forms `ling <project-info.md> [--<word>]`, `ling <operation> <project-info.md> [--<word>] <arguments>`, and `ling daemon <project-info.md> [--<word>]`. `Id::projectInfo`, the `## system include` table, and the `## diagnostics` table are removed. ORACLE's choices from CONTRACT under this ruling, which ARCHITECT can reverse: (a) one daemon for each pair of a manifest and a toolchain argument (MANIFESTO **D**; PLAN-ling-8-daemon.md, The form); (b) the precompiled header rule of Part 10 stays in `Processor`, because it reads row properties only and names no file form and no flag; (c) the toolchain argument follows the manifest in each command form, as in cast (`cast/Source/HELP.md:24-25`); (d) a change of the manifest or of a compiler manifest runs the one reaction to `Id::manifest` again (PLAN-ling-8-daemon.md, In LING: the watched manifest).
75. On the source of the standard library files. ORACLE asked where the C family entry reads the compiler's own search paths, with three options: read none, ask the compiler, or read a CMake file of the build folder. ARCHITECT: *"there must be a determistic way, language agnostic, ISOMORPHIC ABSTRACTION we could get without even assuming any toolchain to get standards library deps to be included"*. Facts given, read on END Debug with ninja 1.13.2: the compile rule holds `depfile = $DEP_FILE`, `deps = gcc`, and `-MD -MT $out -MF $DEP_FILE` (`end/Builds/Debug/CMakeFiles/rules.ninja:23-26`); the build folder holds the record, `.ninja_deps`; `ninja -t deps` prints 194 records with 5635 distinct files, every path absolute, among them `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1/vector`; none of the 142 commands of `compile_commands.json` holds `-MD`. Facts given from the Librarian read: on Windows, `deps = msvc` stores the `/showIncludes` lines in the same record; other builds write the same kind of record (the `<object>.d` files of the CMake Makefile generators, `dep-info` of rustc, `-emit-dependencies` of swiftc, `go list -deps`, `/sourceDependencies` of MSVC). ARCHITECT: *"deps record"*. Result: the files of a unit are the files that the compiler read at the last build, from the dependency record of the build. LING holds no search path and no include flag, walks no `#include`, and asks no compiler. `Id::searchPaths`, `getDependencyFile`, the include flag tables, `applyToIncludes`, the `#include` element of `CLanguageDocument` with `Id::include`, `Id::local`, and the `SyntaxTokenType` row `include`, and the `CppKeyword` row `tokenInclude` of Part 4 are removed: each has no reader (PLAN-ling-2-language-tables.md, What this part does not do). A unit row holds `Id::dependencies`.
76. On the read of the ninja record. Options given: run `ninja -t deps`, or read `.ninja_deps` with a reader in LING (signature `# ninjadeps\n`, version 4, `deps_log.h:52-62`; ninja deletes a file of another version, `deps_log.cc:176-182`). Measured on END Debug: 0.05 s and 12.5 MB of text for one run. ARCHITECT selected *"Run ninja -t deps"*. Result: the C family entry runs the `ninja` row of the toolchain rows with the flag `-t deps`, in the folder of the manifest, and reads the text. ninja owns its file format; LING holds no reader for `.ninja_deps`. The words `ninja`, `-C`, `-t deps`, and `: #deps` are rows of `ling/cast/c_language.md`. ORACLE's choices from CONTRACT under rulings 75 and 76, which ARCHITECT can reverse: (a) `CompilerManifest` adds one `FILE` row for each distinct file of the records, at creation, and `Processor` adds no row (MANIFESTO **E**, Complete at Creation); (b) a file takes the family of the first unit whose record holds it; (c) a record with no unit row is not read; (d) a unit with no record gets a message and keeps its own row; (e) the run blocks the reaction until ninja ends, in the form of `Toolchain::runProcess` (`scout/Source/Toolchain.cpp:15-41`), so the `FILES` row is complete in one event.
77. On a build under a live daemon. Finding: Part 8 watched the manifest and each compiler manifest, and a build changes neither. Options given: watch the record file, or no watch. ARCHITECT selected *"Watch the record file (Recommended)"*. Option text: "One more row in c_language.md names `.ninja_deps`. LING watches that file and never reads it; any event runs the one `Id::manifest` reaction again, as for compile_commands.json. Basis: ruling 45, \"FileWatcher to ValueTree, listener react\", and the Part 8 form for compiler manifests. Cost: one build gives many events (the watcher coalesces them per file), and each reload replaces the FILES row and parses every file again; Part 11 reads unchanged files from the swap." Result: the row `dependencyRecord` of `ling/cast/c_language.md`, and `Id::dependencyRecords` on the `FILES` row (PLAN-ling-8-daemon.md, In LING: the watched manifest). ORACLE's choice from CONTRACT, which ARCHITECT can reverse: the record file has its own property, `Id::dependencyRecords`, and is not in `Id::compilerManifests`, because ruling 74 gives "compiler manifest" to `compile_commands.json` (NAMES.md Rule 3).
78. On the Windows steps. ORACLE listed "who runs the Windows steps" as an owed item. ARCHITECT: *"of course ARCHITECT will build, run, and debug on windows."* / *"we're on mac, code must crossplatform with branching only allowed at inevitable seam where each platform are genuinely differs."* / *"just fucking implement it with fact you have, then development there will proof what works"*. Result: each part writes the cross-platform code in full, from the facts that are read on this machine. A platform branch exists only at a seam where the two platforms genuinely differ. The Windows step of a part is ARCHITECT's: ARCHITECT builds, runs, and debugs on the Windows machine. That step blocks no other step and no other part. A fact that the Windows development gives corrects the code at that time. No part holds an open question about Windows.
79. On the check of the manifest. Finding: cast and scout check a manifest before they read it (`scout/Source/Processor.cpp:19-22`; `cast/Source/Validator.h:690-700`), the inventory named the piece (The tool side, Manifest checks), and no part wrote it or called it. A `## toolchain` table with no `command` column gave no row and no message (`jam_MarkdownDocument.h:226-229`). ORACLE stated the write: `struct Validator : jam::MarkdownValidator`, called in the reaction to `Id::manifest` before `CompilerManifest::getFiles`; the rule `toolchain` with scout's body; a failure as a `setMessage`; in Part 8 a rule for the `timeout` row in the form of cast's `width` test. ARCHITECT: `/go`. Result: Part 6 holds `Validator` and its call (In LING: the manifest check). Part 8 adds the rule `daemon`. The base is cast's base, because the `type` rule of `jam::ConfigValidator` fails the `## project info` table of a cast manifest (`jam_ConfigValidator.h:23`; `end/project-info.md:24-46`). By the read, with no ruling of ARCHITECT, and open to a change: when the check fails, the reaction reads no table, the `FILES` row is replaced by a row with no child (MANIFESTO **D**: the same manifest gives the same tree), `Id::timeout` gets `defaultTimeoutMs`, and the `ROOTS` row holds the project row only. On the `daemon` rule, ARCHITECT: *"daemon timeout undeclared does not mean failure, it should have internal default. table declaration at project-info.md is optional only to override"*. Result: a manifest with no `## daemon` table, or with no `timeout` row, passes the rule and gives no message; `Id::timeout` holds `defaultTimeoutMs`. The rule judges only a value that a project declares.
80. On the form of `declarationTypes`. Finding: Part 4 named the table and gave no type (Declarations item 5; Step 5 item 2), and its table held a fourth input, the qualifier, that Step 5 did not name. Options given: the scope type as the key, with a `jam::Union` of four types as the value (20 slots); or a packed key, `jam::Union<...>::pack (...).bits` (`jam_LookupTable.h:34-36`), with about 65,800 slots, because `pack` shifts by byte (`jam_Union.h:82-83`). ARCHITECT selected *"Scope key, packed value"*. Option text: "Key is the scope type. Value is a `jam::Union` of four types. Structured binding reads it; shape and `static` select with two conditions. 20 slots. Value-side `jam::Union` is the form of `mermaidBlockLinks` (`jam/generated/jam_Mermaid.h:1670`)." The selected form holds the names `functionType`, `staticFunctionType`, `variableType`, `staticVariableType`, and `scopeType`, and the capacity `map::SyntaxTokenType::macro + 1`. Result: Part 4 holds the type, the eight entries, and the read. ORACLE first stated "generated by cast"; that was wrong against ruling 35, which adds no table to cast. The table is hand-written inside the function that reads it, in the form of `jam_Document.cpp:292-299`. By the read, with no ruling of ARCHITECT, and open to a change: the function with a qualifier in a namespace scope is a case before the table, as the constructor and the destructor are; the `namespaceToken` entry is the first entry, and so the default.
81. On the lock. ORACLE reported: no open item in a part, rulings 1 to 80 in Canon, and three limits (the last edits were checked by dry-run, count, residue search, and a read of each changed section; the choices that the read made under rulings 74, 76, 77, 79, and 80; the facts that the execution measures first). ARCHITECT: *"lock them all."* Result: the PLAN is locked, as one PLAN (ruling 38), on 2026-11-11. The Step Gate of CAROL.md governs from here. COUNSELOR executes the parts in Status order.

### ARCHITECT's own description of the document

ARCHITECT asked: *"so what document are we building? reference of specific navigation point? file:col:row for any symbol?"* and *"so this actually like grep? fuzzy finder? but with linkage of relationship?"*
The answer given in the session, on the measured data, is yes to both. It is not a ruling. It is the working description until ARCHITECT changes it:

- One navigation point holds a position (file, row, column), the symbol at that position, the role of the point (declaration, definition, reference, call), and the parent symbol.
- The graph is three links: the same symbol at two points, the parent of a call (the caller), and the base of a class.

### The front end

Rulings 23, 25, and 26 name the C front end: cast tables and a `CLanguageDocument`. Ruling 28 confirms it for the C family. A front end library loads only when the user's manifest declares one.

The libclang runs of this session stay as reference data (see Reference data).

## Dependency & API Inventory

Every row is an existing piece. No row is new design.

### The language side

| Need | Existing piece | Where |
|---|---|---|
| The parse engine | `jam::Document`: `preprocess`, `getTokens`, `parse`, then the three hooks `getVocabulary`, `getToken`, `build` | `jam/jam_core/document/jam_Document.h:972`, `:1024`, `:1193`, `:1203-1209`; `jam_Document.cpp:627-670` |
| Vocabulary as a table | `Document::Vocabulary`, a view over a generated byte-class table and the comment literals | `jam_Document.h:776-799` |
| The C byte classes | `## code`, 29 rows, generated as `map::code` (`LookupTable<int, int, 256>`) | `jam/cast/core.md:2426`; `jam/generated/jam_Core.h:1185` |
| The C vocabulary | `getCLangVocabulary()`: `map::code` with `Chars::cssCommentOpen` and `Chars::cssCommentClose` | `jam_CodeDocument.cpp:170-175` |
| The C++ keywords | `## CppKeyword`, 96 rows, generated as `map::CppKeyword` (`Bimap<int>`) | `core.md:268`; `jam_Core.h:640` |
| File extension to language | `## languageFamily` (`cpp`, `c`, `h` give `cLang`), generated as `map::languageFamily` (`HashMap<juce::String, int>`) | `core.md:2518`; `jam_Core.cpp:153-162` |
| A token type table | `<Name>TokenType`: `XmlTokenType`, `CssTokenType`, `HtmlTokenType`, `MarkdownTokenType`, `AnsiTokenType`, `SyntaxTokenType` | `core.md:161`, `:226`; `jam/cast/web.md:63`, `:126`; `markdown.md:115`; `terminal.md:101` |
| Table to code | a `core.md` section, an output row in `spell.md`, a fence in `code.cast` (`bimap` `:160-178`, `lookup-table` `:287-294`, `hash-map` `:138-141`), then `cast cast/spell.md` | `jam/cast/spell.md:344-550`; `jam/cast/code.cast` |
| A token with a position | `Token::span`, a byte range into the source; properties for sub-spans and flags | `jam_Document.h:61-66`, `:93-102` |
| Token dispatch by segment | a `Function::Map` keyed by `map::DocumentTokenType` inside `getToken` | `jam_CodeDocument.cpp:26-69`; `jam/jam_web/css/jam_Css.cpp:120-167` |
| Tokens to a tree, flat nesting | a `Function::Map` named `treeConstruction`, keyed by token type, with a stack of `Element*` | `jam/jam_core/xml/jam_Xml.cpp:224-259`, `:692-746` |
| Tokens to a tree, nested blocks | recursion for nested rules and a brace depth counter; a block table `getBlockConstruction` | `jam_Css.cpp:1695-1717`, `:1719-1757`, `:1896-1934` |
| Lookahead | an index into `jam::Array<Document::Token>`; a copy of the `Cursor` | `jam_Css.cpp:1676`; `jam_Xml.cpp:559-562` |
| Bad input | the root stays unchanged with a `debug::Log` line; or a property marks the bad token | `jam_Xml.cpp:248-258`; `jam_Css.cpp:1994-1998` |
| File, offset, row on an element | `Id::path`, `Id::offset`, `Id::line`, set by `MarkdownDocument::parse` with an origin | `jam/jam_markdown/document/jam_MarkdownDocument.h:60-71` |
| Row from an offset | `Document::getLineNumber`, a binary search over `lineOffsets` | `jam_Document.h:959`; `jam_Document.cpp:282-287` |
| A link to a non-child element | an `Element*` property and an `Elements` list: `Id::from`, `Id::to`, `Id::out`, `Id::in` | `jam/jam_mermaid_diagram/diagram/jam_MermaidDiagram.h:850-857`; `jam_Document.h:202-203` |
| Find a name inside a scope | `Document::getChildByID (parent, id)`, an O(1) probe over (parent, id) | `jam_Document.h:929`, `:237`; `jam_Document.cpp:403-416` |
| Several files in one document | `Document::appendChildren (Document&&)`, with `addSource` and span rebasing; cast's `Model` splices every declared file into one document | `jam_Document.h:1086`, `:1112`; `cast/Source/Model.h:9-21` |
| A complete document is const | "A complete document is held const" | `jam_Document.h:218-223` |
| A registered function per language | a `Function::Map` from a family to a function | `jam_CodeDocument.h:168`; `jam_CodeDocument.cpp:196-215` |
| Where a language type lives | every language type is in a JAM module; the nearest sibling is `jam_core/document/jam_CodeDocument.h`, included at `jam_core.h:105` | `jam/jam_core/jam_core.h:104-105` |

### The tool side

| Need | Existing piece | Where |
|---|---|---|
| Create the project | `cast INIT.md <target>` from the cli archetype; then `cast cast/spell.md` | `archetype/ARCHETYPE.md:92-98`, `:147-149`; `archetype/cli/INIT.md`; `archetype/cli/init.cast` |
| Console target and install | `juce_add_console_app`; post-build copy to `$ENV{HOME}/.local/bin` | `archetype/cli/cast/cmake.cast:1-202`, `:284-294`; `scout/CMakeLists.txt:207-208`, `:391-395` |
| Entry and arguments | the scout `Main.cpp` family: `printError`, `getVersionFlag`, `getHelpFlag`, `isVersion`, `isHelp`, `runDocument`, `runDocumentNotFound`, `getDocumentFile`, `runCommandLine`, `main` | `scout/Source/Main.cpp:31-34`, `:158-172`, `:220-252`, `:263-278` |
| Help | `printHelp`; `HELP.md` as BinaryData | `scout/Source/Help.h:20`; `scout/Source/Help.cpp:3-35` |
| The run object | `struct Processor`: constructor parses the manifest, `generate()` validates and runs | `scout/Source/Processor.h:20-114`; `scout/Source/Processor.cpp:11-34` |
| Manifest checks | `struct Validator : jam::ConfigValidator` in scout; `struct Validator : jam::MarkdownValidator` in cast, the base that LING takes (ruling 79) | `scout/Source/Validator.h:17-37`; `cast/Source/Validator.h:19`; `jam/jam_markdown/document/jam_MarkdownValidator.h:18-84` |
| Read manifest tables | `jam::ConfigDocument::parse`; `getTables`, `getTableRows`, `getTableValue` | `scout/Source/Processor.cpp:13`, `:36-55`; `jam_MarkdownDocument.h:89`, `:169`, `:209` |
| Read `compile_commands.json` | `juce::JSON::parse` | `whatdbg/Source/dap/Reader.cpp:98` |
| The project's own words | `cast/identifiers.md`, `files.md`, `text.md`, generated into `Source/generated/` | `scout/cast/identifiers.md`; `scout/cast/text.md`; `scout/cast/spell.md:79-228` |
| Child process | `Toolchain::runProcess` over `jam::Subprocess` | `scout/Source/Toolchain.cpp:15-41` |
| Parallel units | `Jobs::run (count, function)` into slots of a sized `jam::Array` | `cast/Source/Jobs.h:14-39`; `cast/Source/Processor.cpp:33` |
| JSON output | `JsonWriter` with `juce::JSON::escapeString` | `cast/Source/JsonWriter.h:14-66`; `cast/Source/JsonWriter.cpp:3-80` |
| Name search | `jam_fuzzy`, "Fuzzy search (Levenshtein) and fzf-algorithm matching engine" | `jam/jam_fuzzy/jam_fuzzy.h:9-10` |
| Diagnostics log | `jam::debug::Log` with a `Scope`; it does not write to stdout | `jam/jam_core/debug/jam_Log.h:21-121` |

### The store side

No part reads these rows after ruling 63. Part 11 takes its pieces from its own inventory. The line numbers are reads of 2026-11-10.

| Need | Existing piece | Where |
|---|---|---|
| A fixed-size index entry and a direct seek | `indexEntryBytes`; entry address from the key; record at the offset | `jam/jam_terminal/document/jam_TerminalSwap.h:156`; `jam_TerminalSwap.cpp:150-173`, `:448-477` |
| A second index over records of free size | `TerminalImageRecord`: 32-byte entries, fields at fixed offsets | `jam_TerminalImageRecord.h:93-125`; `jam_TerminalImageRecord.cpp:25-47` |
| Read with no copy | `juce::MemoryMappedFile::readOnly`, mapped inside one call | `jam_TerminalSwap.cpp:359`, `:463` |
| Several processes read one file | "other apps can also open the same memory mapped file" | JUCE `juce_core/files/juce_MemoryMappedFile.h:67-72` |
| Record fields | `jam::BinaryCodec`, little-endian integers and length-prefixed UTF-8 | `jam/jam_core/binary_codec/jam_BinaryCodec.h:16-61` |
| A value written and read as one unit | `AttributedString::write` and `AttributedString::read` | `jam/jam_core/text/jam_AttributedString.h:341`, `:352` |
| Publish a complete file | `juce::TemporaryFile (targetFile)`, then `overwriteTargetFileWithTemporary()` | JUCE `juce_core/files/juce_TemporaryFile.h:110-116`; `juce_TemporaryFile.cpp:94-119` |
| A cache folder keyed by a hash | `CAST_JUCE_CACHE_DIR`, `$HOME/.cache/cast/juce` and `$LOCALAPPDATA/cast/juce`, keyed by SHA256 | `jam/cast/juce.cast:8-26` |

### The process shape: three existing patterns

| Pattern | Existing piece | Where |
|---|---|---|
| One process per request | cast and scout: `runCommandLine`, then an exit code | `scout/Source/Main.cpp:220-252` |
| One long-lived stdio process per client | whatdbg: `dap::Reader` (`Content-Length` framing, a reader thread) and `writeMessage` | `whatdbg/Source/dap/Reader.h:19-42`; `Reader.cpp:78-98`; `whatdbg/Source/Whatdbg.cpp:414-425` |
| One shared daemon for several clients | endless nexus: `Daemon : juce::InterprocessConnectionServer`, `Channel`, `Link`, `Message`, port file, `juce::InterProcessLock`, detached spawn | `endless/Source/nexus/Daemon.h:60-62`; `Daemon.cpp:46`; `Daemon.mm:40-64`; `DaemonWindows.cpp:51-61`; `endless/Source/Main.cpp:84-88`, `:345-362` |

### Reference data, measured in this session

Machine: macOS 26.5, arm64, 10 cores, 16 GB. Compiler manifest: `end/Builds/Release/compile_commands.json`, 143 entries, of which 141 are source units.

| Fact | Value |
|---|---|
| One END unit (`Source/ui/SessionView.cpp`), as a compiler front end reports it | 373 declarations, 1,326 references, 26 bases, in 16 files |
| The 16 END units together | 8,276 declarations, 15,885 references |
| Symbol types that occur in two END units | instance method 318, variable 134, field 112, enum constant 94, constructor 62, static method 33, class 29, destructor 26, struct 18, enum 10, static variable 9, namespace 8, function 7 |
| Reference roles in two END units | reference 2,595, call 483, read 6, write 3 |
| The headers behind `JuceHeader.h` | 316,505 declarations, 142,759 distinct symbols, 66,493 distinct names |
| Names that belong to one symbol only | 48,707 of 66,493 |
| One name, several symbols | `get` 171, `add` 58, `getText` 32, `parse` 21, `setBounds` 5 |
| A symbol identity | the qualified name and the parameter types; 0 of 373 held a file name |
| The compiler manifest on 2026-11-10, later read | `end/Builds/Debug/compile_commands.json`: 142 entries; keys `command`, `directory`, `file`, `output`; unit extensions `c`, `cpp`, `cxx`, `mm`. `end/Builds/Release/compile_commands.json` was not on disk at that read. A read of 2026-11-11 finds it (10 Nov 23:43). |
| The Release compiler manifest | each unit holds two `-arch` flags; the root `end/compile_commands.json` is a stale copy that names a removed `Builds/Debug` |
| Implicit search paths of `/usr/bin/c++` | one sysroot and seven search paths, from `/usr/bin/c++ -E -x c++ - -v` |
| A replace of a mapped file, macOS | the old map stays correct; a new open reads the new file |
| A replace of a mapped file, Windows | not measured |

The measurement programs are in the session scratchpad. They are not part of LING.

### The module facts behind ruling 30

| Fact | Where |
|---|---|
| `jam_web` holds the HTML and CSS parsers: "HTML authored-subset and CSS Syntax Level 3 subset tokenizers and parsers" | `jam/jam_web/jam_web.h:9` |
| `jam_style` depends on `jam_web` | `jam/jam_style/jam_style.h:10-12` |
| The module name `jam_document` is taken: "Universal line break and reflow (UAX #14)", one type `ReflowDocument` | `jam/jam_document/jam_document.h:9`, `:32` |
| `jam_markdown` depends on `jam_document` | `jam/jam_markdown/jam_markdown.h:10-12` |
| `jam_mermaid_diagram` depends on `juce_gui_basics` | `jam/jam_mermaid_diagram/jam_mermaid_diagram.h:10-12` |
| No module named `jam_syntax_tree` exists | `jam/` module list |
| `CodeDocument` today: `jam/jam_core/document/jam_CodeDocument.h`, included by `jam_core.h` | `jam/jam_core/jam_core.h:104-105` |
| A module with its own generated table header: `#include <generated/jam_Web.h>` | `jam/jam_web/jam_web.h:29` |
| A 64-bit string hash | `juce::String::hashCode64()`, `juce_core/text/juce_String.h:211` |

### The existing syntax tables behind ruling 35

LING extends these tables with rows. It adds no table (ruling 35).

| Table | What it holds now | Where | Generated as |
|---|---|---|---|
| `## DocumentTokenType` | segment types: text, operators, region, comment | `jam/cast/core.md:185-203` | `Bimap<int>` |
| `## Byte` | byte classes: text, operators, regionOpen, regionClose, quote | `core.md:205-224` | `Bimap<int>` |
| `## SyntaxTokenType` | highlight token types: keyword, string, comment, number, punctuation | `core.md:226-244` | `struct SyntaxTokenType : jam::Bimap<int>`, `jam/generated/jam_Core.h:577` |
| `## Family` | language families: cLang, python, shell, markup, data, js | `core.md:246-266` | `struct Family : jam::Bimap<int>`, `jam_Core.h:609` |
| `## CppKeyword` | 96 C++ keywords, including `class`, `struct`, `enum`, `namespace` | `core.md:268` | `struct CppKeyword : jam::Bimap<int>`, `jam_Core.h:642` |
| `## JsKeyword`, `## PythonKeyword` | keyword sets of two more families | `core.md:379`, `:441` | `Bimap<int>` |
| `## tokens` | literal tokens, for example `cssCommentOpen` `/*`, `cssCommentClose` `*/`, `classToken` `class` | `core.md:779-838` | constants |
| `## code` | C family byte classes; `#` (0x23) is an operator byte | `core.md:2426-2469` | `LookupTable<int, int, 256>` |
| `## languageFamily` | file extension to family: `cpp`, `c`, `h` give `cLang`; no `hpp`, `mm`, `cc` row | `core.md:2518-2558` | `HashMap<juce::String, int>`, `jam_Core.h:1257` |

Facts that a part needs to extend them:

- `CodeDocument` keeps a token for a keyword only. "a non-keyword identifier consumes bytes with no token added" (`jam_CodeDocument.h:130-133`).
- No table holds the preprocessor directive words (`define`, `include`).
- The readers are known. With END's compiler manifest (`compile_commands.json`), LSP `findReferences` gives `jam_CodeDocument.cpp` as the only reader of `SyntaxTokenType`, `Family`, `CppKeyword`, `JsKeyword`, `PythonKeyword`, `code`, `markup`, `data`, and `languageFamily`. `CodeDocument::parse` has no caller. `DocumentTokenType` (31 references) and `Byte` (160 references) have readers in `jam_core`, `jam_document`, `jam_markdown`, `jam_terminal`, and `jam_web`. The full table is in `PLAN-ling-2-language-tables.md`.
- cast writes each JAM module header region from `jam/user-modules-info.md` (`## module`, `## submodule`), through `jam/cast/spell.md:3540-3824`. A new module is a row group there, not a hand-written include list.

### Prior art: the isomorphic pattern (reference only, ruling 34)

clangd, ccls, and clice were read to learn the common pattern. They are not design inputs. The CONTRACT decides (ruling 34).

| Part of the pattern | clangd | ccls | clice |
|---|---|---|---|
| A symbol ID: a 64-bit hash of the canonical declaration | first 8 bytes of SHA-1 over the USR (`index/SymbolID.cpp`) | SipHash of the USR (`src/utils.cc:80-90`, `src/indexer.cc:608-626`) | xxh3 over the entity bytes (`src/semantic/identity.cpp:262-302`) |
| A symbol record: name, type, scope, places, doc, signature | `Symbol` (`index/Symbol.h`) | `FuncDef`, `TypeDef`, `VarDef` (`src/indexer.hh:138-230`) | `Symbol` (`src/index/symbol.cppm:252-322`) |
| An occurrence: a range, the target ID, role bits | `Ref`, `RefKind` (`index/Ref.h`) | `Use`, `Role` (`src/indexer.hh:40-53`, `:95-102`) | `Occurrence` (`symbol.cppm:101-108`); the role is on the relation |
| A relation: subject, type, object | `Relation`, 2 types (`index/Relation.h`) | fields `bases`, `derived`, `callees`, `type`, `instances` (`src/indexer.cc:1061-1202`) | `Relation`, 24 types (`src/semantic/symbol.cppm:64-125`) |
| The caller of a use: the symbol whose body holds it | `Ref::Container` | an extent search (`src/messages/ccls_call.cc:91-99`) | `Caller` and `Callee` relations (`src/feature/hierarchy.cpp:212-269`) |
| One adapter from the front end to plain records | `SymbolCollector` | `IndexDataConsumer::handleDeclOccurrence` (`src/indexer.cc:851`) | `build_tu_index` (`src/index/indexer.cppm:49`) |
| A re-index replaces the whole record set of a file | `FileSymbols` | `createDelta`, `applyIndexUpdate` (`src/query.cc:114-383`) | one shard per content hash (`src/index/shard.cppm:14-20`) |
| When a stored result is still valid: a file stamp and the compile command | content digest | mtime and arguments (`src/pipeline.cc:111-136`) | size and mtime, then a content hash; a command hash (`docs/en/design/incremental-parse.md:69-72`) |
| Read in place | one `.idx` per file | one `.blob` per file (`src/pipeline.cc:401-402`) | LMDB, "validated once and then used directly as a memory-mapped, zero-copy view" (`docs/en/design/symbol-index.md:129`) |
| Query set: lookup, references, relations, symbol at a position, name search | `SymbolIndex` (`index/Index.h`) | handlers over `DB` (`src/query.hh:138-171`) | `IndexQuery` (`src/index/query.cppm:257-478`) |
| Name search | trigrams and a fuzzy score | a subsequence filter, then a fuzzy rank (`src/messages/workspace.cc:139-198`) | trigrams over word splits (`symbol-index.md:161`) |

clangd paths are under `llvm-project/clang-tools-extra/clangd/`, read on the web without line numbers. ccls paths are under `~/Documents/Poems/dev/ccls/`. clice paths are under `~/Documents/Poems/dev/clice/`.

Each in-house piece for the same part is in the inventory above: `Element` id and properties, `Token::span`, `Element*` links, the `getToken` and `build` hooks, `TemporaryFile` replace, `MemoryMappedFile`, `Function::Map`, `jam_fuzzy`.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

Each numbered part is one sprint: COUNSELOR executes its steps, @Auditor runs once at its end, and the sprint is logged (Step 3). The lock is one, for the whole PLAN (ruling 38).

## Steps

### Step 1: Close the open rulings
**Scope:** this file, section Risks / Open Questions.
**Action:** ARCHITECT rules on each open item. The plan author adds each ruling, verbatim, to Canon and removes the item.
**Validation:** Each ruling in Canon is a verbatim quote. No open item remains that a part in the next step needs.

### Step 2: Write the part plans in Status order
**Scope:** `PLAN-ling-1-project.md` to `PLAN-ling-11-swap.md` at the project root.
**Action:** Write each part with the plan-writer skill. Take every dependency from the inventory of this file. Add a row to the inventory first when a part needs a piece that this file does not list, with its file:line. Give every identifier in the part: a name from an established family with its citation, or a name that Canon holds.
**Validation:** Each step of a part names its files, its functions, and the existing piece it follows. No step holds a name that is neither in Canon nor in a cited family. No part holds an open question at its lock.

### Step 3: Track
**Scope:** this file, section Status.
**Action:** Set the status of a part when it is written, when ARCHITECT locks it, and when its sprint is logged.
**Validation:** The Status table matches the files at the project root and `carol/SPRINT-LOG.md`.

## BLESSED Alignment

- **B — Bound.** One owner holds the documents of all files. The process shape is one shared daemon (ruling 29).
- **L — Lean.** Each part is one responsibility. No part builds a feature that Canon does not name (YAGNI).
- **E — Explicit.** The user declares the inputs in the manifest (rulings 19 and 20). LING reads them and guesses nothing.
- **S — SSOT.** The source file is the truth (ruling 14). The language tables are the one source of a language's vocabulary, and cast generates the code from them.
- **S — Stateless.** A complete document is const (`jam_Document.h:218-223`). A query reads and changes nothing.
- **E — Encapsulation.** The language part is tables and one `Document` type in JAM. The tool holds no language name (ruling 22). Data flows one way: manifest, language document, store, query, client.
- **D — Deterministic.** The same sources and the same manifest give the same store. A part proves it with two runs and a byte comparison.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

### Closed by ARCHITECT's rulings

| Item | Ruling |
|---|---|
| 1. Supersession of the RFC producer | 28 |
| 2. Process shape | 29 |
| 3. Where the language type and its tables live | 30 |
| 4. "Rules" | 31 |
| 5. The MVP symbol types | 32. `#define` names are part of the MVP. |
| 6. `#include` and macros, unit of a document | 33. `#define` names: 32. |
| 14. Where the extended tables live | 36. The nine tables that only `CodeDocument` reads move to `cast/syntax_tree.md`. Each new row comes in the part that holds its first reader (MANIFESTO **L**, YAGNI). |

### Closed by a read of the CONTRACT and the rulings

7. **Several declarations with one scope and one name.** The link from a reference points to every declaration with that scope and name, as an `Elements` list (`jam_Document.h:202-203`). When the manifest declares a front end, its binding gives one declaration (ruling 28). Reasons:
   - Ruling 4 requires definition and jump to be "always correct". A link to one declaration that no binding chose can be wrong.
   - MANIFESTO **E**: no magic, fail fast. A choice by heuristic is a hidden guess.
   - MANIFESTO **D**: the same sources give the same links. The full set is deterministic.
8. **New domain terms.** Each part takes every name from the nearest family and cites it (NAMES.md Rule 5). A name that no family covers is a finding in the part, not a question (NAMES.md Rule -1). Only a term specific to the domain goes to ARCHITECT, in the part that needs it.
9. **Inputs that the compiler manifest of the C family lacks.** Closed by rulings 71, 74, 75, and 76. `compile_commands.json` does not hold the compiler's own search paths. LING needs none: the dependency record of the build holds each file that the compiler read for a unit, the standard library included (PLAN-ling-6-manifest.md, The dependency record of the C family, as read on this machine). The check flags are rows of `ling/cast/c_language.md` (ruling 74). The daemon timer has a default and an optional `## daemon` row (ruling 71).
10. **The long form of the name.** Ruling 24 writes "Language Index Navigation Graph". The later ruling governs (CAROL.md Instruction Hierarchy). `README.md:3` and `README.md:9` hold this form since 2026-11-11, on ARCHITECT's word *"clean up"*. No part edits `README.md`.
15. **Locals and parameters.** They are in the tree, as `variable`. RFC ruling 9 requires "our own COMPLETE AST" for "definition, jump, snippet, completion" (Canon, From the removed RFC). Ruling 32 gives no parameter type.
12. **A check of correctness.** MANIFESTO **D**: "Unit tests prove it formally — same input, bit-identical output". Each part proves determinism with two runs and a byte comparison. The reference data stays reference data (ruling 34).
13. **Windows.** Ruling 16 requires cross-platform behavior. Each part that touches a mapped file, a process, or a manifest holds a step that a Windows machine runs. The two open measurements stay: a replace of a mapped file, and the `clang-cl` manifest shape. Ruling 78: the Windows step is ARCHITECT's, and it blocks no part. The two measurements come from the Windows development.

### Open, not blocking a part

16. **The stdio LSP wrapper.** Closed by facts, 2026-11-10. The removed RFC named it. Neither client needs it for diagnostics. nvim reads the compiler lines through `getqflist` with an errorformat and `vim.diagnostic.fromqflist` (validated on nvim 0.12.6; the row `%f(%l\,%c): %trror: %m` is already in `~/.config/nvim/lua/core/build.lua:43`). Claude Code runs a `PostToolUse` hook after `Edit|Write`, with `tool_input.file_path`; exit code 2 "Shows stderr to Claude" (code.claude.com hooks reference). The Claude Code LSP tool has no completion operation and no diagnostics producer for LING. ARCHITECT asked for the comparison and took no wrapper: *"what can format both nvim and agents could understand without circus of translating and parsing"*.

17. **JAM run APIs that take `juce::StringArray`.** Closed by ruling 52, 2026-11-11: the signatures change to `jam::Strings`, with the scout callers, and `jam::Strings` gets a public view of one element. The facts behind the ruling: `jam::ChildProcess::start (const juce::StringArray&, const juce::File&)` (`jam_ChildProcess.h:44`) and `jam::Subprocess::launch (const juce::StringArray&, ...)` (`jam_Subprocess.h:95`) take the JUCE type. Ruling 50 permits `juce::StringArray` only where a JUCE API demands it. Known callers in scout's index: `scout/Source/Toolchain.cpp:21`, `scout/Source/Benchmark.cpp:110`. A third fact: `jam::Strings` has no public view of one element. `at (int)` returns `juce::String` by value (`jam_core/text/jam_Strings.h:565`), its iterator returns `juce::String` by value (`:590`), and `getView (Span)` is private (`:643`, `:754`). So each read of one element of a `jam::Strings` makes a `juce::String`, and an argv for `execvp` or `posix_spawn` (one NUL-terminated `char*` for each argument) can be built only through `at`. ARCHITECT decides: change both run signatures to `jam::Strings` with their callers, and give `jam::Strings` a public view of one element (changes outside the named parts), or keep them and convert at the call.

11. **SPEC.** Ruling 24 names a SPEC contract. No `SPEC.md` exists. Writing it is gated (CAROL.md Decision Gate). No part needs it.

18. **Identifier names that a JAM table also holds.** Closed by facts, 2026-11-11. Every generated table writes into one global `namespace Id` (`jam/generated/jam_Profiler.h:25`; `scout/Source/generated/Identifiers.h:24`). A name is defined two times only when the program compiles both tables. Three names that the parts add to `ling/cast/identifiers.md` are in JAM tables: `manifest` (`jam/cast/profiler.md:36`), `family` (`jam/cast/opentype.md:36`), `output` (`jam/cast/vulkan.md:60`). LING compiles none of `jam_profiler`, `jam_opentype`, `jam_vulkan`: they are not in the thirteen modules (PLAN-ling-1-project.md, The module rows that this part needs), and the modules that later parts add are `jam_syntax_tree`, `jam_subprocess`, and `jam_fuzzy`, each with the dependency `jam_core`. No module header names `jam_profiler` or `jam_opentype` as a dependency. Only `jam_plugin_bootstrap` names `jam_vulkan`. Thus each of the three names is defined one time in LING. The same shape exists: `scout/cast/identifiers.md:20` holds `output`, and scout does not compile `jam_vulkan`. The names that a compiled table holds get no LING row: `command` (`terminal.md:45`); `headerRow` (`markdown.md:54`); `directory`, `file`, `from`, `key`, `line`, `name`, `offset`, `path`, `tokens`, `type`, `value`, `version` (`core.md`); `mac`, `win`, `overlay` (`style.md`); `message` (`gui.md:55`); `in`, `to` (`mermaid.md`, moved to `core.md` by Part 5 Step 1). No part adds a row for one of these. The names with no row in any JAM table are: `argument`, `arguments`, `children`, `column`, `compilerManifests`, `completion`, `dependencyRecords`, `connection`, `connections`, `daemon`, `diagnostics`, `documents`, `dependencies`, `files`, `flag`, `ling`, `member`, `parameters`, `request`, `result`, `scope`, `snippet`, `timeout`, `toolchain`, `toolchainArgument`, and the nine operation names. Read of 2026-11-11: the rows that the later revisions of Parts 6, 9, 10, and 11 add (`comment`, `parameters`, `cmake`, `hash`, `jucePath`, `juceVersion`, `module`, `patch`, `roots`, `submodule`, `swap`, `symbol`, `userModule`, `completion`, `snippet`) have no `@id` row in any `jam/cast/*.md`. `output` is in `vulkan.md:60`, which LING does not compile.

19. **A build under a live daemon.** Closed by ruling 77, 2026-11-11. The files of a unit come from the dependency record (ruling 75), and a build writes new records with no change of `compile_commands.json`. LING watches the record file, `.ninja_deps` in the build folder, and never reads it. Any event runs the one reaction to `Id::manifest` again (PLAN-ling-8-daemon.md, In LING: the watched manifest). ninja appends to the file as each compile ends (`deps_log.h:65-67`), so one build can give more than one reload; PLAN-ling-8-daemon.md Step 7 item 10 counts them.

20. **An operation word that is empty text.** Closed by a read, 2026-11-11. `Id::command` holds a word of the command line, and `Processor` makes a `juce::Identifier` from it. The constructor asserts on an empty string (JUCE `juce_core/text/juce_Identifier.cpp:57-62`). It does not assert on another text: `isValidIdentifier` is a separate function (`juce_Identifier.cpp:80-83`). Part 7 now holds the one test, in `Processor`: an empty word is an operation that the events map does not hold, and the result is `failArguments`. Part 8 writes the word to the row as text.
