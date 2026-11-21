# PLAN: ling runs from anywhere

**RFC:** RFC-ling-runs-from-anywhere.md
**Date:** 2026-11-19
**BLESSED Compliance:** verified, except the 3 open questions at the end
**Language Constraints:** C++17 / JUCE / JAM; cLang language library written as `language.cast` + `construction.md` + `spell.md` (cast)

After approval, this file is copied to `/Users/jreng/Documents/Poems/dev/ling/PLAN-ling-runs-from-anywhere.md` (plan-writer: plan at the project root).

## Context

ARCHITECT: *"ling should be able to run from anywhere. while project-info.md as manifest still adhere to its contract."*

Today:
- `ling project-info.md` fails in a folder with no manifest.
- It prints the guide on stdout (`Main.cpp:161`), and that breaks the LSP header that Claude Code reads.
- The sessions that fail are `~/.carol`, `~/.config`, JAM, CAKE and CAROLINE (RFC R1).

This plan maps the RFC decisions D1 to D8 to steps:
- D1, D2, D4, D7 and D8 are code steps.
- D3 and D5 are void by D2.
- D6 is MACHINIST's work after the change ships.

It runs after the `apply`→`set` rename that Engineer is doing now. That rename also touches `Relay::applyMessage`, which becomes `setMessage`.

## Framework Inventory

### API
| Need | API | Source | Step |
|---|---|---|---|
| Parent folder | `juce::File::getParentDirectory()`; on the root it returns the root | JUCE `juce_File.h:286-293` | 4 |
| Root stop | `juce::File::isRoot()` | `juce_File.h:136-142` (exact line: re-read in step 4) | 4 |
| Child by name | `juce::File::getChildFile (StringRef)` | `juce_File.h:275` | 4 |
| File exists | `juce::File::existsAsFile()` | `juce_File.h:124` | 4 |
| Working folder | `juce::File::getCurrentWorkingDirectory()` | `juce_File.h:1019`; used at `Relay.cpp:272` | 4 |
| Manifest from the argument | `getManifestFile` → `resolveAsFile().getLinkedTarget()` | `Main.cpp:149-152` | 4 |
| Failure line on stderr | `juce::ConsoleApplication::fail (getErrorLine (...))` | `Main.cpp:162` | 3 |
| Answer without the daemon | `Relay::sendResponse (document, idToken, member, appendValue)` | `Relay.h:121-136`; `shutdown` sends `null` at `Relay.cpp:317-321` | 5 |
| Platform value table | `ling::WordMap` from a `construction.md` table through `:::[list]:::` | `getLibraryPaths` at `language.cast:1911-1919`; table at `construction.md:493-498`; spell row at `spell.md:334` | 1 |
| Platform key | `currentPlatform` | `language.cast:74-80` | 1, 2 |

### Pattern usage
| Pattern | Exemplar | Use sites | Step |
|---|---|---|---|
| Platform table read: `getX().getAddress (currentPlatform)` | `language.cast:669` | `:648`, `:669`, `:673`, `:2293`, `:2672` | 1, 2 |
| Optional platform row: `contains (currentPlatform) ? … : {}` | `language.cast:673` | `:673`, `:2672` | 1 |
| Relay answers a request itself | `Relay::shutdown` (`Relay.cpp:317-321`) | `initialize` `:229-246`; `sendError` `:213-227` | 5 |
| Request dispatch | `Relay::applyMessage` → `requestMethods` / `operations` / `sendError` (`Relay.cpp:160-181`) | the only dispatch | 5 |

### Names
| Family | Members | Intent | Step |
|---|---|---|---|
| construction platform tables | `libraryPaths`, `sdkCommands`, `sdkFlags` (`construction.md:493-510`) | plural noun for what the table holds, keyed by platform | 1: `juceModulePaths` (RFC D8) |
| platform table getters | `getLibraryPaths`, `getSdkCommands`, `getSdkFlags` (`language.cast:1911-1941`) | `get` returns the static map | 1: `getJuceModulePaths` |
| Main manifest functions | `getManifestFile`, `getExistingManifestFile` (`Main.cpp:149-163`) | `get` returns a `juce::File` | 4: the search goes inside `getManifestFile` (no new name) |

### Synthesis
| Rule | Established form | Exemplar |
|---|---|---|
| "No bail-out guards" | positive nesting, with a result return | `getExistingManifestFile` `Main.cpp:158-159` |
| "Use the framework API fully" | `juce::File` for paths; `std::filesystem` only inside the language library | `Main.cpp:151`; `language.cast:2154` |
| "No `==` / `!=` on strings" | `std::string_view::compare (...) == 0` in cLang | `language.cast:2152` |
| "Owner first" | a platform value lives in `construction.md`, never in code | `construction.md:493-510` |

### Absence
| Need | Query | Result |
|---|---|---|
| Search the parent folders by file name | jam doxygen index (`jam/docs/xml/index.xml`) for `findFile`, `getManifest`, `Upward`, `Ancestor`; the `juce::File` API | `findFile` and `getManifest` return nothing. No `juce::File` member walks upward. → a hand-written loop over `getParentDirectory` and `isRoot` |
| Parser for `BEGIN_JUCE_MODULE_DECLARATION` | quoted literal in JUCE modules, jam, ling; JUCE doxygen for `ModuleDescription` / `juceaide` | No hit in modules, jam or ling. The parser exists only in the Projucer and juceaide sources, which are not a library. → cLang reads the declaration block itself |
| C++ standard value for a module | `language/` searched for `-std`, `std=`, `standard` | No source found (open question 3) |

## Validation Gate
COUNSELOR validates each step before the next one, against:
- MANIFESTO.md (BLESSED), NAMES.md and ~/.carol/CODING.md;
- the Inventory rows of the step;
- the locked PLAN decisions, with no deviation and no scope drift.

@Auditor runs once, after the final step. That run is on hold, because ARCHITECT ruled: *"hold audit and doxygen. until i explicitly said so."*

## Steps

### Step 1: `juceModulePaths` table (D8)
- **Scope:** `language/cLang/construction.md`, `language/cLang/spell.md`, `language/cLang/language.cast`.
- **Action:**
  1. Add `## juceModulePaths` after `## libraryPaths` (`construction.md:493-498`), in the same `| key | value |` shape. It has one row for each platform key. The values come from open question 2.
  2. Add a spell row like `spell.md:334` (`@construction:juceModulePaths`, `@language:member`, `@Library`).
  3. Add a `getJuceModulePaths` fence like `getLibraryPaths` (`language.cast:1911-1919`).
- **The source copy:** the repo copy is the source.
  - `CMakeLists.txt:387-389` names `language/cLang/*`.
  - ARCHITECT: *"always delete on disk, let the binary build and replace"*.
  - So the `~/.config/ling/language/cLang/` copy is not edited. The build replaces it.
- **Inventory:** API "Platform value table"; Names "construction platform tables" and "platform table getters".
- **Validation:** the new table, spell row and fence match their siblings exactly. No path appears in code.

### Step 2: compile arguments from the module declaration (D7)
- **Scope:** `language/cLang/language.cast`, `construction.md` (the words of the declaration, such as `ID:`, `dependencies:` and the markers).
- **Action:** this step runs for a C++ file that is under a JUCE module folder and has no compile-database entry.
  1. Read the declaration block of the module header, between `BEGIN_JUCE_MODULE_DECLARATION` and `END_JUCE_MODULE_DECLARATION`.
  2. Read the declarations of the module's dependencies in the same way. A `juce_*` dependency resolves under `getJuceModulePaths()`. Any other dependency resolves under the parent of the module folder.
  3. Build these arguments:
     - the include root, which is the parent of the module folder, plus the JUCE modules folder;
     - `-DJUCE_MODULE_AVAILABLE_<id>=1` for the module and for each dependency;
     - `-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1`;
     - the C++ standard (open question 3).
  4. Read the words from a `construction.md` table, as `toolchainWords` holds the cmake words (`construction.md:326-346`). A file that a project compiles keeps its database arguments (RFC R8).
- **Inventory:** Absence "Parser for `BEGIN_JUCE_MODULE_DECLARATION`"; Pattern "Platform table read".
- **Validation:**
  - Each word comes from a table, and the code holds no magic value.
  - The arguments are built once, at creation of the unit (MANIFESTO E, Complete at Creation).
  - The engine stays language-agnostic. All of this is in cLang.
- **Blocked by open question 1.**

### Step 3: a failed start writes nothing to stdout (D4)
- **Scope:** `Source/Main.cpp:154-163`, `Source/HELP.md:48`.
- **Action:**
  1. Delete `printBannerAndHelp();` at `Main.cpp:161`. The `fail (...)` line on stderr stays.
  2. Change `HELP.md:48` to `Prints \`ling: <missing>: not found\` on stderr.` with exit 1.
- **Inventory:** API "Failure line on stderr".
- **Validation:** the guide prints only for no argument (`:343`) and for `--help` (`:287`).

### Step 4: find the manifest in the parent folders (D1)
- **Scope:** `Source/Main.cpp:149-152` (`getManifestFile`). The function serves all 3 callers: `:348`, `:399` and `:430`.
- **Action:**
  1. When the argument resolves to a file that exists, return it, as today.
  2. When it does not exist and the argument is a bare file name, with no folder part, check each parent of the working folder, nearest first, until `isRoot()`.
  3. At each parent, take `getChildFile (name)`. The first one where `existsAsFile()` is true is the manifest. When none exists, return the argument's file.
  4. `getExistingManifestFile` then fails as today, for the paths that step 5 does not cover.
- **Inventory:** API "Parent folder", "Root stop", "Child by name", "File exists", "Working folder"; Absence "Search the parent folders by file name"; Synthesis "No bail-out guards".
- **Validation:**
  - The search is a result-return loop with no guard.
  - One working folder gives one manifest, so the daemon name is deterministic (RFC D1).
  - The project hash input stays the manifest path.

### Step 5: the server runs with no manifest and serves nothing (D2)
- **Scope:** `Source/Main.cpp` (`runManifest`, `:339-385`), `Source/Relay.h/.cpp`, `Source/Client.h/.cpp`.
- **Action:**
  1. In `runManifest`, when step 4 finds no manifest for a bare name, construct the relay with no manifest. The relay then makes no `Client::start()` connection and starts no daemon (RFC D2, D5).
  2. `initialize`, `initialized`, `shutdown` and `exit` behave as today (`Relay.cpp:229-246`, `:317-321`).
  3. Each operation request (`operations->contains`, `Relay.cpp:172`) gets `null` through `sendResponse`, the same form as `shutdown`. LSP accepts `null` as the empty result of every operation that ling serves.
  4. `runDaemon` and `runOperation` are unchanged. A missing manifest still fails on stderr.
- **Inventory:** API "Answer without the daemon"; Pattern "Relay answers a request itself", "Request dispatch".
- **Validation:**
  - No daemon, no port file and no index exist when there is no manifest (Bound).
  - Nothing other than LSP frames is written to stdout.
  - The Client gets no state flag for "has no manifest". The design question goes to the Engineer prompt with the read of `Client.h`/`.cpp`, before code: does the relay own an optional client (`std::unique_ptr<Client>`)? Today Relay inherits from Client (`Relay.h:18`). If an optional client needs a change in ownership, the change is to composition, and it is reported in the step.
- **Execution design (from the reads, ruling 1):**
  1. Step 4 returns the working folder when a bare name is found in no folder. That folder is the "manifest" of the relay, the daemon and the operation. The project name is the hash of the folder path (`LINGModel.cpp:30-40`), so it stays deterministic.
  2. `LINGModel::getProjectDirectory` gives the folder itself for a folder, else the parent of the manifest. It replaces the project-folder reads at `Manifest.cpp:33`, `:186`, `:248` and `Processor.cpp:1293`.
  3. `ManifestJob::runJob` gives a folder an empty toolchain child with no message, the default timeout and threads, and the folder roots.
  4. A daemon whose files child has zero rows writes `Server::noPort` (`-1`) to its port file and quits. `-1` differs from a missing port file, which `getIntValue` reads as 0.
  5. `Client::connectToDaemon` writes the port that it reads to `model.state` `Id::port` (new row in `cast/identifiers.md`).
  6. Relay listens to `model.state` in place of `messages` and `requests`. Its callbacks already filter by tree (`Relay.cpp:137`, `:143`, `:156`). On `Id::port == noPort`, each request row gets a `null` result (`sendEmptyResult`). `sendResponse` takes the id text in place of the document and the token.
  7. Operation listens to `model.state`. On `noPort`, it prints a `null` result and exits 0.
  8. The relay owns no optional client and keeps no flag. The "no units" state is the port value in the Model.

### Step 6: guide text (D1, D2, D4)
- **Scope:** `Source/HELP.md` (the Server section `:13-35`, the Command Line table `:39-53`).
- **Action:**
  1. Server: add that a bare manifest name is found in the nearest parent folder that holds it.
  2. Server: add that with no manifest found, the server answers each operation with `null` and starts no daemon.
  3. Command Line: change row `:48` as in step 3.
- **Validation:** STE text. Each sentence states behaviour that steps 3 to 5 implement.

### Step 7: build and measure
1. Run `cast cast/spell.md --no-sign` once.
2. Delete the `~/.config/ling/language/cLang/` folder, so that the binary writes the folder again (ARCHITECT's ruling).
3. Run these checks, each bounded with `perl -e 'alarm N; exec @ARGV'`:
   - From `~/.carol`: `ling project-info.md`, then send `initialize`, `textDocument/documentSymbol` and `shutdown` on stdin. Expect LSP frames only on stdout, `null` for the operation, and no daemon in `ps`.
   - From `ling/Source`: `ling documentSymbol project-info.md Source/Main.cpp` finds `ling/project-info.md`. Expect the same answer as from the ling root.
   - `ling /tmp/missing.md` from `~/.carol`: nothing on stdout, `ling: /tmp/missing.md: not found` on stderr, exit 1. RFC D4 (`RFC-ling-runs-from-anywhere.md:158`) keeps this failure only for an argument with a path part. A bare name that no folder holds runs the server (D2).
   - The hover and methods harness on END and KANJUT, to check for regressions.
   - After step 2: a JAM header from a session with no project, once open question 1 is answered.

### Step 8: hand off to MACHINIST (D6)
`~/.carol/skills/doxygen-protocol/SKILL.md:23` and `:44-46` change after ship. Those edits are MACHINIST's, not COUNSELOR's. `plugin.json` does not change.

## BLESSED Alignment
- **B:** one daemon for each manifest and word. No manifest means no daemon (step 5).
- **L:** one loop in an existing function (step 4), one table and one getter in an existing family (step 1).
- **E:**
  - A failure goes to stderr only (step 3).
  - Every value comes from a table (steps 1 and 2).
- **S:**
  - The manifest stays the only source of units and toolchain.
  - The module declaration is the only source of module facts (step 2).
  - The repo copy is the source of `construction.md`.
- **S (Stateless):** the search keeps no state between runs.
- **E (Encapsulation):** the plugin passes no project knowledge, and the engine stays language-agnostic.
- **D:** one folder gives one manifest.

## Rulings (ARCHITECT, 2026-11-19, answers to the three questions)
1. D2 against D7: *"Module folder gets a daemon"*.
   - The option text: with no manifest, a folder whose ancestors hold a `BEGIN_JUCE_MODULE_DECLARATION` header starts a daemon. Its units are the module files, with arguments from the declarations. Every other folder follows D2.
   - Effect on step 5:
     1. With no manifest, the relay names the daemon by the working folder and the word. The project name is the hash of the folder path, so it is still deterministic.
     2. The daemon reads no manifest tables. It calls each language's `units` entry with zero toolchain rows and the folder, through the existing `LanguageUnits` contract.
     3. cLang finds the module headers that hold a declaration, in the folder and its parent folders, and adds their files as units with the step 2 arguments.
     4. A daemon whose languages give zero units ends at once, and the relay answers `null`. Thus no daemon stays alive for a folder that is not a module folder (Bound). The engine stays language-agnostic, because the module detection is inside cLang.
2. Windows JUCE path: *"Same layout under home"*.
   - `juceModulePaths` holds `Documents/Poems/JUCE/modules` for `platformMac` and for `platformWindows`. The value is relative to the user home folder.
   - The home folder comes from a sibling platform table, `homeVariables`: `platformMac` gives `HOME` and `platformWindows` gives `USERPROFILE`. cLang reads it with `std::getenv`, in the same `getX().getAddress (currentPlatform)` form.
3. C++ standard: *"Declaration, else table"*.
   - Use the module's `minimumCppStandard` when it is set.
   - Otherwise use the row of one `construction.md` table, `cppStandards`, with the value `17`. ling's own `CMakeLists.txt:19` uses the same value.

## Risks / Open Questions
None. The three questions are closed by the rulings above.

## Superseded open questions (kept for the record)
1. **RFC D2 and D7 conflict.**
   - D2: *"No manifest found: the server runs and serves no units. It starts no daemon."*
   - D7: *"A folder of JUCE modules, such as JAM, KANJUT, CIUM, needs no project and no manifest"*. ling builds compile arguments from the module declaration.
   - A JAM session has no `project-info.md` in any parent folder. Under D2 it gets no units and no daemon, so step 2 never runs there.
   - The rule that decides which one applies to a JUCE module folder with no manifest is ARCHITECT's.
2. **The Windows value of `juceModulePaths`.**
   - On this Mac the value is `/Users/jreng/Documents/Poems/JUCE/modules`, read with `ls -d`. END's manifest gives `jucePath` as `${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE`.
   - The JUCE folder on the two Windows machines is not readable from this session.
   - D8 requires both platform keys.
3. **The C++ standard for step 2.**
   - D7 says *"the C++ standard of the language folder"*. No such value exists in `language/`, from a search for `-std`, `std=` and `standard`.
   - Each JUCE module declaration has a `minimumCppStandard` field. It is empty in `jam_core.h` (RFC R9).
   - The value and its source table are ARCHITECT's choice.
