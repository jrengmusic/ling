# PLAN: LING Part 1 — The Console Project

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

This part makes the `ling` console project from the cli archetype.
At its end, `ling --version` and `ling --help` work, and the binary installs to `~/.local/bin`.
The part adds no index, no manifest read, and no daemon. Later parts add them.

Tracker and canon: `PLAN-ling-0-tracker.md`. This part depends on Part 0 only.

## Language / Framework Constraints

- LANGUAGE.md, C++ / JUCE: one object is one header and one `.cpp`.
- CODING.md: no anonymous namespace. File-local functions use `static` linkage, as `scout/Source/Main.cpp` does.
- CODING.md: no `==` on strings. A flag test is `std::string_view (argv[flagArgIndex]).compare (<flag>) == 0`, in the form of `scout/Source/Main.cpp:71-72`. `argv` is not a JUCE input, so it stays a byte view (ruling 50).
- Ruling 50: no `juce::String` and no `juce::StringArray`, except where a JUCE API returns one or takes one as an argument. Text is `std::string_view` over existing bytes, or a `std::string` that the code creates. A generated `@string` row is `extern const juce::String` (`Id::diagnosticSeparator`, `generated/jam_Core.h`), so it is read through `jam::Strings::getView (const juce::String&)` (`jam_core/text/jam_Strings.h:633`). A `Chars` row is `static constexpr const char*` (`Chars::doubleDash`, `generated/jam_Core.h`).
- CODING.md: Windows SDK macro names are not identifiers.
- ARCHETYPE.md §1: every generated file is a build artifact. No step edits `CMakeLists.txt`, `Source/generated/*`, `entitlements.plist`, `gh.cmake`, or `RELEASE.md` by hand.
- ARCHETYPE.md §1: every cast table needs a `+---` separator line between records.

## Dependency & API Inventory

Paths are relative to `~/Documents/Poems/dev/`.

### The generator

| Need | Existing piece | Where |
|---|---|---|
| Make the skeleton | `cast INIT.md <target>`, run from the archetype directory. "Always give a target directory." | `archetype/ARCHETYPE.md:92-95` |
| What the run writes | `project-info.md`, `cast/spell.md`, `cast/signing.md`, `RELEASE.md` | `archetype/cli/INIT.md:43-55`; `ARCHETYPE.md:96-98` |
| The framework cells | JAM values, no edit needed | `archetype/cli/INIT.md:21-41` |
| The placeholder list | `MyApp`, `myapp`, `MyCompany`, `My Company Ltd`, `com.mycompany.*`, `0.1.0` / `0x100`, `Your Name`, `TEAMID0000`, `info@example.com`, `https://example.com`, the zero thumbprint | `ARCHETYPE.md:108-116` |
| The templates stay in the archetype | `cast/spell.md` points at `../../archetype/cli/cast/cmake.cast` and `installer.cast`, as scout does | `scout/cast/spell.md:9-10` |
| A `LICENSE` file at the root | The cli set needs it for the installer fences | `ARCHETYPE.md:143-146`; `archetype/cli/cast/cmake.cast:306`, `:312` |
| The build flows | `cast cast/spell.md --debug` makes a Debug build in `Builds/Debug` | `ARCHETYPE.md:159-163`; `archetype/cli/init.cast:172-176` |
| The install | "Post-build always: copy the binary to `installDirectory`" (`$ENV{HOME}/.local/bin`) | `ARCHETYPE.md:270`; `init.cast:63` |
| The commit in the version line | `CAST_COMMIT`, set by the template from the git commit hash | `archetype/cli/cast/cmake.cast:28`, `:92` |
| A converged project | "a second `cast` run produces an empty diff" | `ARCHETYPE.md:478-479` |

### The nearest sibling: scout

| Need | Existing piece | Where |
|---|---|---|
| Project values of the same author | `companyName`, `legalCompanyName`, `productWebsite`, `companyEmail`, `companyCopyright`, `qaDirectory`, the `jrengmusic/<name>` repository, the `com.jrengmusic.<name>` bundle identifier | `scout/project-info.md:26-34`, `:64`, `:68`, `:92`, `:94` |
| The signing values of the same author | six rows | `scout/cast/signing.md:6-16` |
| The JUCE module rows | six rows | `scout/project-info.md:115-120` |
| The user module rows | fifteen rows | `scout/project-info.md:128-142` |
| The define rows | thirteen rows | `scout/project-info.md:159-171` |
| The include rows | four rows | `scout/project-info.md:179-182` |
| The binary rows | `help`, `styleSheet` | `scout/project-info.md:190-191` |
| The project word tables | `cast/identifiers.md`, `cast/text.md`, `cast/files.md`, `cast/banner.md`, `cast/source.cast` | `scout/cast/` |
| The manifest wiring for those tables | index rows, header rows, output groups, the `## output index` table | `scout/cast/spell.md:21-33`, `:53-77`, `:146-197`, `:199-228` |
| The entry file | `printBanner`, `printError`, `getVersionFlag`, `getHelpFlag`, `isVersion`, `isHelp`, `writeVersion`, `isTerminalOutput`, `printBannerAndHelp`, `runCommandLine`, `main` | `scout/Source/Main.cpp:19-23`, `:31-34`, `:45-59`, `:69-87`, `:93-102`, `:108-115`, `:137-145`, `:220-252`, `:263-278` |
| The help printer | `printHelp (const juce::String&)` | `scout/Source/Help.h:20`; `scout/Source/Help.cpp:3-35` |
| The help style sheet | `Source/style.css`, 630 lines, embedded as BinaryData | `scout/Source/style.css`; `scout/project-info.md:191` |
| The help file name constant | `files::scoutHelp`; cast's is `files::castHelp` | `scout/cast/files.md:18`; `cast/cast/files.md` |

### Words the framework already holds

| Word | Where |
|---|---|
| `Id::version` | `jam/cast/core.md:156` |
| `Id::diagnosticSeparator` (`: `) | `jam/cast/core.md:70` |
| `Chars::doubleDash`, `Chars::space`, `Chars::openParen`, `Chars::closeParen`, `Chars::newline` | used at `scout/Source/Main.cpp:47`, `:96-100`, `:139` |
| `jam::TerminalEncoder::writeEraseScreen` | used at `scout/Source/Main.cpp:272` |
| `Id::help` is not in JAM. scout declares it in its own table. | `scout/cast/identifiers.md:42` |

### The module rows that this part needs

The help printer uses `jam::TextEditor`, `jam::MarkdownDocument`, `jam::StyleManager`, `jam::StyleMarkdown`, `jam::TerminalBuffer`, and `jam::TerminalGraphicsContext` (`scout/Source/Help.cpp:5-32`).
The dependency lines of the module headers (`jam/jam_<name>/jam_<name>.h`, `dependencies:`) give this closure for `jam_markdown_graphics` and `jam_terminal`:

`jam_core`, `jam_document`, `jam_markdown`, `jam_freetype`, `jam_graphics`, `jam_data_structures`, `jam_animation`, `jam_web`, `jam_style`, `jam_gui`, `jam_mermaid_diagram`, `jam_markdown_graphics`, `jam_terminal`.

These are thirteen of scout's fifteen rows. The two rows left out are `jam_profiler` and `jam_subprocess`. No code of this part uses them.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: Generate the skeleton
**Scope:** `ling/project-info.md`, `ling/cast/spell.md`, `ling/cast/signing.md`, `ling/RELEASE.md` (all new).
**Action:**
1. Go to `~/Documents/Poems/dev/archetype/cli`.
2. Run `cast INIT.md ~/Documents/Poems/dev/ling`.
3. Change no cell of `INIT.md`. The framework is JAM.

**Validation:**
- The four files exist under `ling/`.
- The archetype's own stubs are unchanged: `archetype/cli/project-info.md`, `cast/spell.md`, `cast/signing.md`, and `RELEASE.md` have 7 lines each.
- `ling/README.md`, `ling/CLAUDE.md`, and `ling/carol/` are unchanged.

### Step 2: Set the project values
**Scope:** `ling/project-info.md`, tables `## project info` and `## cmake`.
**Action:** Replace each placeholder. Change no other cell.

`## project info`:

| name | value | description cell |
|---|---|---|
| `projectName` | `ling` | `Language Index Navigation Graph` |
| `companyName` | `JRENG` | unchanged |
| `legalCompanyName` | `Jubilant Research of Eclectic Novelty Generation` | unchanged |
| `versionString` | `0.1.0` | unchanged |
| `versionNumber` | `0x100` | unchanged |
| `productWebsite` | `` `https://jrengmusic.com` `` | unchanged |
| `companyEmail` | `info@jrengmusic.com` | unchanged |
| `presetExtension` | `ling` | unchanged |
| `presetDefault` | `INIT` | unchanged |

`## cmake`:

| key | value |
|---|---|
| `description` | `Language Index Navigation Graph` |
| `banner` | `ling` |
| `bundleIdentifier` | `com.jrengmusic.ling` |
| `targetName` | `ling` |
| `companyCopyright` | `© 2026 JRENG` |
| `productName` | `ling` |
| `buildVersion` | `0.1.0` |
| `documentExtensions` | `ling` |
| `qaDirectory` | `` `$ENV{HOME}/Documents/Poems/dev/___builds___` `` |
| `repository` | `jrengmusic/ling` |

**Validation:**
- No placeholder of `ARCHETYPE.md:108-116` remains in `project-info.md`.
- The long name is "Language Index Navigation Graph" (ruling 24). It holds no "and".
- Each value matches the scout row of the same key, with `scout` replaced by `ling` (`scout/project-info.md:26-34`, `:42-98`).
- The file holds no `compileJobMemory` row and no `precompiledHeader` row. The cli template reads neither.

### Step 3: Set the module, define, include, and binary rows
**Scope:** `ling/project-info.md`, tables `## juce module`, `## user module`, `## define`, `## include`, `## binary`.
**Action:**
1. `## juce module`: write the six rows of `scout/project-info.md:115-120`.
2. `## user module`: write the thirteen rows that the inventory lists, in scout's order, with scout's description cells (`scout/project-info.md:128-140`). Leave out `jam_profiler` and `jam_subprocess`.
3. `## define`: write the thirteen rows of `scout/project-info.md:159-171`.
4. `## include`: write the four rows of `scout/project-info.md:179-182`.
5. `## binary`: write the two rows of `scout/project-info.md:190-191`.
6. Change no row of `## toolchain`, `## architecture`, `## source glob`, `## pack`, `## release`, or `## debug`.

**Validation:**
- Each row is identical to its scout row.
- The user module list equals the closure in the inventory. No row names a module that no code of this part uses.
- Each table has a `+---` separator line between records where the generated table has one.

### Step 4: Set the signing values
**Scope:** `ling/cast/signing.md`.
**Action:** Write the six value cells of `scout/cast/signing.md:6-16` into the six rows.
**Validation:** `ling/cast/signing.md` is identical to `scout/cast/signing.md`. No placeholder (`Your Name`, `TEAMID0000`, the zero thumbprint) remains.

### Step 5: Add the license file
**Scope:** `ling/LICENSE` (new).
**Action:** Copy `scout/LICENSE` to `ling/LICENSE` with no change. ARCHITECT selected: *"Same MIT file"*. cast, scout, whatdbg, and END hold this one file, byte-identical.
**Validation:** `ling/LICENSE` is byte-identical to `scout/LICENSE`. The `distribution` and `installer` fences and the two `LICENSE` entries of the cli templates find it (`archetype/cli/cast/installer.cast:6`, `:53`; `cmake.cast:306`, `:312`).

### Step 6: Add the project word tables
**Scope:** `ling/cast/identifiers.md`, `ling/cast/text.md`, `ling/cast/files.md`, `ling/cast/banner.md`, `ling/cast/source.cast` (all new).
**Action:**
1. `cast/identifiers.md`: copy the `## index` table of `scout/cast/identifiers.md:1-7`. Write the `## identifiers` table with the brief `@brief LING vocabulary — the words of the ling command line.` and one row: type `@id`, name `help`, value `` `help` ``, empty description.
2. `cast/text.md`: write the `## diagnostics` table with the brief `@brief ling message texts, emitted verbatim when a run fails.` and one row: name `failFlagUnknown`, value `` `unknown flag` ``, description `An argument that is not --help or --version.`
3. `cast/files.md`: copy the `## index` table of `scout/cast/files.md:1-7`. Write the `## files` table with the brief `@brief File names ling reads: help.` and one row: type `@string`, name `lingHelp`, value `` `HELP.md` ``, description `Rendered help text.`
4. `cast/banner.md`: write a `## banner` table in the shape of `scout/cast/banner.md:1-18`. Copy the brief of `scout/cast/banner.md:3-5`. Write these thirteen rows, top to bottom. Keep every space of each value.

   | key | value |
   |---|---|
   | `creamyMint` | `████  ████                            ` |
   | `grapeGreen` | `████  ░░░░                            ` |
   | `sweetMintPesto` | `████  ████  ██████████      ██████████` |
   | `leek` | `████  ████  ████░░░░████  ████░░░░████` |
   | `snarkyMint` | `████  ████  ████    ████  ████    ████` |
   | `herbivore` | `████  ████  ████    ████  ████    ████` |
   | `jovialJade` | `████  ████  ████    ████  ████    ████` |
   | `basilSmash` | `████  ████  ████    ████  ████    ████` |
   | `lima` | `████  ████  ████    ████  ░░██████████` |
   | `lightMint` | `░░░░  ░░░░  ░░░░    ░░░░    ░░░░░░████` |
   | `bigSpender` | `                                  ████` |
   | `pastelPea` | `                            ████████░░` |
   | `feijoa` | `                            ░░░░░░░░  ` |

   The values are the thirteen glyph rows of `ling/ling-banner.sh:5-17`, without the colour codes. Each value has 38 cells. The keys are the thirteen names of `## ColourNames` nearest to the banner colour rgb(153,255,153) (`jam/cast/graphics.md:698`, `:1235`, `:2778`, `:1653`, `:2676`, `:1350`, `:1552`, `:246`, `:1687`, `:1673`, `:292`, `:2233`, `:972`). ARCHITECT: *"the colour does not matter, just pick from named colours, we have the table"*.
5. `cast/source.cast`: copy `scout/cast/source.cast` (6 lines) with no change.

**Validation:**
- `help` follows `scout/cast/identifiers.md:42`. `failFlagUnknown` follows `scout/cast/text.md:16`. `lingHelp` follows `scoutHelp` (`scout/cast/files.md:18`) and `castHelp`.
- No table holds a row that no code of this part reads (MANIFESTO L, YAGNI).
- No new domain term exists (NAMES.md Rule -1).
- `cast/banner.md` has thirteen rows. Each key is a different name of `## ColourNames`. Each value equals its row of `ling-banner.sh` with the colour codes removed.

### Step 7: Wire the tables in the manifest
**Scope:** `ling/cast/spell.md`.
**Action:**
1. `## index`: add the twelve rows of `scout/cast/spell.md:21-32` (`@source` to `@string`), after the `@Release` row.
2. `## headers`: add the five rows of `scout/cast/spell.md:53-77` (`Identifiers.h`, `Text.h`, `Files.h`, `HashMaps.h`, `Generated.h`). In each brief and description, write `ling` or `LING` where scout writes `scout` or `SCOUT`. In the `CMakeLists.txt` row, write `ling build manifest` where the generated text says `MyApp build manifest`.
3. `## output`: add the seven row groups of `scout/cast/spell.md:146-197` after the `@Release` group, with no change.
4. Add the `## output index` table of `scout/cast/spell.md:199-228`, with no change.

**Validation:**
- A text comparison of `ling/cast/spell.md` and `scout/cast/spell.md` shows differences only in the words `ling`/`LING` against `scout`/`SCOUT`.
- The arity law holds (`ARCHETYPE.md:251-255`): no source line of the `@cmake:cmake` group is added or removed.

### Step 8: Add the help files
**Scope:** `ling/Source/HELP.md`, `ling/Source/style.css`, `ling/Source/Help.h`, `ling/Source/Help.cpp` (all new).
**Action:**
1. `Source/style.css`: copy `scout/Source/style.css` with no change.
2. `Source/Help.h`: copy `scout/Source/Help.h`. Write no doxygen block (CAROL.md Code Hygiene). The file holds `#pragma once`, the two includes, and the declaration `void printHelp (std::string_view helpText);`. scout declares `const juce::String&`; ruling 50 gives the view.
3. `Source/Help.cpp`: copy `scout/Source/Help.cpp`. Replace `files::scoutHelp` with `files::lingHelp` (line 19). The parameter is `std::string_view helpText`. `jam::MarkdownDocument::parse (helpText, files::lingHelp)` then calls the overload `parse (std::string_view documentText, const juce::String& origin)` (`jam/jam_markdown/document/jam_MarkdownDocument.h:83`). Change nothing else.
4. `Source/HELP.md`: write this text.

```markdown
# LING: Language Index Navigation Graph

**Version 0.1.0**

You see this guide because you ran `ling` with no argument or `ling --help`.

## Command Line

| Command          | Result                                         | Exit |
| :--------------- | :--------------------------------------------- | :--- |
| `ling`           | Prints this guide.                             | 0    |
| `ling --help`    | Prints this guide.                             | 0    |
| `ling --version` | Prints `ling 0.1.0 (<commit>)`.                | 0    |
| `ling <other>`   | Prints `ling: <other>: unknown flag` on stderr. | 1    |

On a terminal, ling clears the screen first.
```

**Validation:**
- `Help.cpp` differs from `scout/Source/Help.cpp` in one identifier and the type of one parameter only.
- `HELP.md` states only the behavior that Step 9 builds.
- The files hold no comment and no doxygen block.

### Step 9: Add the entry file
**Scope:** `ling/Source/Main.cpp` (new).
**Action:** Write the file from the scout family (`scout/Source/Main.cpp`). Write no comment and no doxygen block.

1. Includes: lines 7-16 of the scout file, without `#include "Processor.h"`.
2. Take these functions with their scout bodies, unchanged: `printBanner` (:19-23), `isTerminalOutput` (:108-115), `main` (:263-278). Take `printError` (:31-34), `getVersionFlag` (:45-49), `getHelpFlag` (:55-59), and `writeVersion` (:93-102) with their scout names and control flow, and with the text types of ruling 50:
   - `static void printError (std::string_view message)`: one `std::string` of `ProjectInfo::projectName`, `jam::Strings::getView (Id::diagnosticSeparator)`, `message`, and `Chars::newline`, written to stderr with `fwrite`.
   - `static const std::string& getVersionFlag()` and `getHelpFlag()`: a function-static `std::string` of `Chars::doubleDash` and `jam::Strings::getView (Id::version.toString())` or `Id::help.toString()`.
   - `writeVersion`: one `std::string` of `ProjectInfo::projectName`, `Chars::space`, `ProjectInfo::versionString`, `Chars::space`, `Chars::openParen`, `CAST_COMMIT`, `Chars::closeParen`, and `Chars::newline`, written to stdout with `fwrite`.
3. Constants: take `flagArgIndex { 1 }` and `flagOnlyArgumentCount { 2 }` (:36, :38). Do not take `postFlagArgIndex` or `postFlagArgumentCount`. This part has no manifest argument, so no flag follows one.
4. `isVersion` and `isHelp`: take the first operand of each scout expression only (:71, :85). Each returns `argc == flagOnlyArgumentCount and std::string_view (argv[flagArgIndex]).compare (<flag>) == 0`.
5. `printBannerAndHelp`: take the scout body (:137-145). Replace `files::scoutHelp` with `files::lingHelp`. The help text is a view over the resource bytes: `BinaryData::Raw` gives `data` and `size` (`jam/jam_core/binary_data/jam_Raw.h:81-85`). scout calls `BinaryData::getString`, which makes a `juce::String` (`jam_Raw.h:111-118`). The newline is `Chars::newline`, written with `fwrite`.
6. `runCommandLine`: write it with these branches, in this order, each a result return:
   - `isVersion` gives `writeVersion()` and `EXIT_SUCCESS` (scout :222-226).
   - `isHelp` gives `printBannerAndHelp()` and `EXIT_SUCCESS` (scout :228-232).
   - `argc >= flagOnlyArgumentCount` gives `printError` of one `std::string` of `argv[flagArgIndex]`, `jam::Strings::getView (Id::diagnosticSeparator)`, and `jam::Strings::getView (text::Diagnostics::failFlagUnknown)`, and `EXIT_FAILURE` (the form of scout :246-247).
   - Otherwise `printBannerAndHelp()` and `EXIT_SUCCESS` (scout :250-251).
7. Do not take `printDone`, `runDocument`, `runDocumentNotFound`, or `getDocumentFile`. No code of this part calls them.

**Validation:**
- Every function name is in `scout/Source/Main.cpp`. No new name exists.
- No `==` compares a string. No anonymous namespace exists. Each file-local function is `static`.
- `runCommandLine` has no bail-out guard. Each return carries the exit code (MANIFESTO E, result return).
- The file holds no function that nothing calls (MANIFESTO L, YAGNI).

### Step 10: Build and check on macOS
**Scope:** the `ling/` project. Generated outputs: `CMakeLists.txt`, `Source/generated/*`, `entitlements.plist`, `gh.cmake`, `cast/installer/*`.
**Action:**
1. From `ling/`, run `cast cast/spell.md --debug`.
2. Run `ling --version`, `ling --help`, `ling`, and `ling --wrong` from `~/.local/bin`.
3. Run `cast cast/spell.md --debug` a second time.

**Validation:**
- The first run ends with exit code 0. `Builds/Debug` holds the binary. `~/.local/bin/ling` exists.
- `ling --version` prints `ling 0.1.0 (<commit>)` and exits 0.
- `ling --help` and `ling` print the banner and the guide, and exit 0.
- `ling --wrong` prints `ling: --wrong: unknown flag` on stderr and exits 1.
- The second run changes no generated file (`ARCHETYPE.md:478-479`).
- `Source/generated/` holds `ProjectInfo.h`, `Identifiers.h`, `Text.h`, `Files.h`, `HashMaps.h`, `Generated.h`, `Generated.cpp`, as `scout/Source/generated/` does.

### Step 11: Build and check on Windows
**Scope:** the `ling/` project on a Windows machine (ruling 16).
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:** Do Step 10 on a Windows machine, with the binary at the install directory that the Debug build copies to.
**Validation:** The four commands of Step 10 give the same output and the same exit codes. `main` sets the console to UTF-8 (`SetConsoleOutputCP (CP_UTF8)`, scout :265-267).

## BLESSED Alignment

- **B — Bound.** The part adds no owned resource. `printBannerAndHelp` holds the JUCE initialiser in its own scope, as scout does.
- **L — Lean.** The part takes only the functions, constants, table rows, and module rows that its own code uses. Rows and functions for a manifest, a toolchain, or a profile stay in scout.
- **E — Explicit.** Each value is a table row. The entry file names its two argument constants. A wrong argument fails with a message and exit code 1.
- **S — SSOT.** `project-info.md` holds each project value once. The templates stay in the archetype, so the cli family has one template.
- **S — Stateless.** The process holds no state. It prints and exits.
- **E — Encapsulation.** Data flows one way: tables, `cast`, generated headers, code.
- **D — Deterministic.** A second `cast` run gives no change. The same argument gives the same output and exit code.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None. ARCHITECT closed the banner rows and the license on 2026-11-10.
