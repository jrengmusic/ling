# PLAN: LING Part 6 — The State Model: A Row and a Document for Each File

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 16, 17, 19, 20, 22, 23, 40, 41, 42, 44, 45; RFC ruling 4. Rulings 70, 72 to 76, and 79.
**Date:** 2026-11-10
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

This part gives LING its state model, in the form of END (ruling 45: *"the SSOT model is FileWatcher to ValueTree, listener react"*).
The source files on disk are the truth. A `jam::Model` holds one row for each source file. The file watcher and the compiler manifests are the authors of those rows. `Processor` listens to the rows and holds one `jam::CodeDocument` for each row: it parses a file when its row comes, parses it again when its row changes, and drops it when its row goes.
`LINGModel` and `Processor` name no language. A new JAM type, `jam::SyntaxTree`, holds the table from a language family to its parse function (ruling 41). A new LING type, `CompilerManifest`, holds the table from a language family to the read of its compiler manifest (ruling 74). The files of each unit come from the dependency record that the build wrote (rulings 75 and 76). LING walks no `#include`.
The manifest of LING is the `project-info.md` of the project, as it is (rulings 65, 72, and 73). The compiler manifest of a language is the file that the build of that language writes. For the C family it is `compile_commands.json` (ruling 74). LING finds it from the `## toolchain` rows of the manifest. It reads the files of each unit through the `ninja` row of the same toolchain rows.

## What this part does not do

- No lookup across files. Part 7.
- No daemon. The process parses and exits.
- No macro expansion, no `#if` evaluation, and no `#include` walk. The files are the files that the compiler read at the last build (ruling 75).
- No swap. A document comes from a parse of its source file. Part 11 keeps the trees on disk (ruling 54).

## Language / Framework Constraints

- Rulings 22 and 74: no language is hard-coded in LING. `LINGModel`, `Processor`, and `Main.cpp` name no language, no build tool, no file form, and no flag word. They call `jam::SyntaxTree`, `jam::CodeDocument`, and `CompilerManifest` only. `CompilerManifest` is the one LING object that names a family together with its compiler manifest, and its words are table rows (ruling 23: *"simply as adding new table, new function to register"*).
- MANIFESTO **E**, Complete at Creation: a document is complete when its parse returns. The owner adds complete documents and changes none.
- MANIFESTO **B**: one owner holds every document, and "Threads are bound to their context": a pool thread makes a document, and only the message thread touches the tree and the map of documents.
- `end/ARCHITECTURE.md:886-907`, the Orchestrator Law: "one sole AUTHOR writes the change; every listener reacts automatically on its own lane".
- MANIFESTO **E** (Explicit): a unit file of a compiler manifest that does not exist is an error with a message. It is not skipped.
- CODING.md: no fake-carrier struct. A file and its document are a key and a value of one map, not a struct.
- CODING.md: no string `==`. A flag is found by a table row.
- CAROL.md Code Hygiene: no comment and no doxygen in this sprint.
- `jam/.claude/CLAUDE.md:104`: "Agents never build. ARCHITECT builds."
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Line numbers are reads of 2026-11-10.

### The compiler manifest of the C family, as read on this machine

Source: `end/Builds/Debug/compile_commands.json`.

| Fact | Value |
|---|---|
| Entries | 142 |
| Keys of an entry | `command`, `directory`, `file`, `output` |
| The command | one string. No `arguments` key. |
| `file` | an absolute path in all 142 entries |
| File extensions of the units | `c`, `cpp`, `cxx`, `mm` |
| Units under `end/Source/` | 16 |
| Units that are JUCE or JAM module files | 38. A module unit includes its sources: `jam_core.cpp` holds `#include "document/jam_Document.cpp"` (`jam/jam_core/jam_core.cpp:14`). |
| Flags of `end/Source/Main.cpp` | 22 `-I`, 1 `-isystem`, 49 `-D`, 2 `-include` behind `-Xclang`, no `-isysroot` |
| The compiler | `/usr/bin/c++`. Its own search paths are not in the compiler manifest (tracker, Reference data: one sysroot and seven search paths). The dependency record holds the files below them (section The dependency record of the C family, as read on this machine). |

`end/Builds/Release/compile_commands.json`, which the tracker names, was not on disk at the read of 2026-11-10. A read of 2026-11-11 finds it.

### The dependency record of the C family, as read on this machine

Source: `ninja -C end/Builds/Debug -t deps`, ninja 1.13.2. Reads of 2026-11-11. The ninja source lines are of `master`, fetched 2026-11-11.

| Fact | Value |
|---|---|
| The rule of a compile | `depfile = $DEP_FILE`, `deps = gcc`, and `-MD -MT $out -MF $DEP_FILE` in the command (`end/Builds/Debug/CMakeFiles/rules.ninja:23-26`) |
| What ninja does with it | "process the `depfile` immediately after the compilation finishes, then delete the `.d` file"; it saves "a compact form of the same information in a Ninja-internal database", "stored as `.ninja_deps` in the `builddir`" (ninja manual, C/C++ header dependencies) |
| The record file | `end/Builds/Debug/.ninja_deps`, 2.1 MB. LING does not read this file (ruling 76). Part 8 watches it (ruling 77). |
| The tool | "`deps`:: show all dependencies stored in the `.ninja_deps` file." (ninja manual, `manual.asciidoc:441-442`). It builds nothing. |
| The output form | one head line, `<output>: #deps <count>, deps mtime <time> (VALID)` or `(STALE)`; then one line of four spaces and a path for each file; then one blank line (`ninja.cc:640-652`) |
| Size and time of one run | 12,460,131 bytes; 0.05 s, 0.05 s, 0.04 s |
| A head | the output path relative to the build folder: `CMakeFiles/END.dir/Source/Main.cpp.o` |
| The units | each of the 142 units of the compiler manifest has one record, and no record is stale. The other records are 26 `.spv` shader outputs, with no entry in the compiler manifest. |
| The source file of a unit | in its own record, for 142 of 142 units |
| The files | 110,813 file lines; 5635 distinct files; at most 3339 in one record; every path absolute. The manual: a depfile "may result in a mixture of relative and absolute paths". |
| Where the files are | 2534 below the SDK or the compiler (735 in `c++/v1`, for example `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1/vector`); 1820 JUCE; 1005 JAM; 127 END |
| File names with no extension | 96, for example `vector` |
| The compiler manifest holds no dependency flag | no `-MD` in the 142 commands. CMake adds the depfile flags to the ninja rule only (`cmNinjaTargetGenerator.cxx:997-1002`, `:1578-1644`). |
| Windows | `deps = msvc`: the tool "outputs header dependencies in the form produced by the Visual Studio compiler's /showIncludes flag", and ninja stores them in the same record (ninja manual, Rule variables). Not run: no Windows tools on this machine. |

### The manifest, as read on this machine

Source: `end/project-info.md`. Reads of 2026-11-11.

| Fact | Value |
|---|---|
| `## toolchain` | the columns `argument`, `command`, `flag` (`end/project-info.md:109-113`) |
| The default-flow rows | the rows with a blank `argument` cell; one of them is `cmake`, `-S . -B Builds/Release -G Ninja ...` (`:118`) |
| The rows of the toolchain argument `debug` | one of them is `cmake`, `-S . -B Builds/Debug -G Ninja ...` (`:128`) |
| A `cmake` row with no build folder | `cmake`, `-P gh.cmake` (`:122`) |
| The `ninja` rows | `ninja`, `-C Builds/Release` (`:120`); `debug`, `ninja`, `-C Builds/Debug` (`:130`). Each comes after the `cmake` row of its toolchain argument. |
| The text `compile_commands` | not in the file. The build writes the file because `CMAKE_EXPORT_COMPILE_COMMANDS` is `ON` (`archetype/cli/cast/cmake.cast:7`). |
| The flag cells of the two build rows | no quote byte and no backslash |

### Existing pieces

| Need | Existing piece | Where |
|---|---|---|
| Read JSON | `juce::JSON::parse` gives a `juce::var` | `whatdbg/Source/dap/Reader.cpp:98` |
| Split a command string into argv | the rule of clang's own reader: Windows command-line rules on Windows, its GNU parser elsewhere (`unescapeCommandLine`, `clang/lib/Tooling/JSONCompilationDatabase.cpp:130-152`). The GNU parser: spaces separate arguments; `"..."` groups, with `\` escaping the next byte; `'...'` groups with no escape; outside quotes `\` escapes the next byte (`:47-128`). The spec: `command` is "a single shell-escaped string ... with '"' and '\' being the only special characters"; `arguments` is "ready to pass to execvp()" (clang.llvm.org/docs/JSONCompilationDatabase.html) | new in this part: `jam::ChildProcess::getArguments` |
| JUCE tokens keep quotes and backslashes | `addTokens` copies the span from the token start to its end; `findEndOfToken` only tracks the quote state (`juce_core/text/juce_StringArray.cpp:341-368`; `juce_CharacterFunctions.h:822-846`). So `-DCAST_PLUGINS_HASH=\"…\"` of END's compiler manifest keeps its backslashes. `jam::Strings::addTokens` and `fromTokens` call `juce::StringArray::fromTokens` (`jam_core/text/jam_Strings.cpp:430-454`), so they keep them too. | JUCE |
| The Windows splitter | `CommandLineToArgvW`: "Parses a Unicode command line string and returns an array of pointers to the command line arguments"; free with one `LocalFree`; `shellapi.h`, `Shell32.lib` (learn.microsoft.com). JUCE calls it the same way (`juce_events/messages/juce_ApplicationBase.cpp:174-186`). | Win32 |
| A `jam::Strings` from a wide argv | `Strings (const wchar_t* const* strings, int numberOfStrings)` | `jam_core/text/jam_Strings.h:160` |
| A path below a directory | `juce::File::getChildFile` | JUCE `juce_core/files/juce_File.h` |
| A state owner that is a global instance | `class ENDModel : public jam::Model, public jam::Instance<ENDModel>`; rows by `getOrCreateChildWithName`; `createAndAddParameter<jam::ParameterText>`; `setMessage` | `end/Source/end/ENDModel.h:19-33`; `ENDModel.cpp:3-19` |
| Load from disk, then watch; a change runs the load again | `loadFromPath(); startWatcher();`; `fileChanged` | `end/Source/config/ConfigModel.cpp:31-32`, `:116-127` |
| The file watcher | `jam::File::Watcher`: `addFolder`, `coalesceEvents`, `addListener`; `Listener::fileChanged (file, event)` on the message thread; six events, `undefined`, `fileCreated`, `fileDeleted`, `fileUpdated`, `fileRenamedOldName`, `fileRenamedNewName`; "All events that arrive within the time window for a particular file will be coalesced into one event with the type of the most recent event" | `jam/jam_core/file/jam_Listener.h:22-120` |
| A listener that reacts to one property of its rows and replaces the object for it | `Session::valueTreePropertyChanged`; the async result is kept only when the row still names the same file | `end/Source/plugins/Session.cpp:69-92`, `:110-118` |
| A change message with no new value | `juce::ValueTree::sendPropertyChangeMessage`; find a row by a property: `getChildWithProperty` | JUCE `juce_data_structures/values/juce_ValueTree.h` |
| Read tables of a markdown manifest | `jam::ConfigDocument::parse`; `getTables`, `getTableRows`, `getTableValue` | `scout/Source/Processor.cpp:13`, `:36-55`; `jam/jam_markdown/document/jam_MarkdownDocument.h:89-96` |
| The toolchain rows of a manifest | `manifest.getTables (Id::toolchain)`, `manifest.getTableRows (*table)`, `manifest.getTableValue (row, Id::command)`, `Id::flag`; `row` is a `const jam::ConfigDocument::Element&`; a row runs in `documentFile.getParentDirectory()` | `scout/Source/Processor.cpp:36-56` |
| A check of the manifest before a read | `Validator {}.isValid (manifest)`, and the run only when the result is ok; `struct Validator : jam::ConfigValidator` with `static void addRules (Rules& rules)` and `const Rules& getRules() const override`; the rule `toolchain` fails a `## toolchain` table whose header row has no `command` cell or no `flag` cell. cast holds the same check: "the invariant Processor::generate() trusts unconditionally" | `scout/Source/Processor.cpp:19-22`; `scout/Source/Validator.h:17-37`; `scout/Source/Validator.cpp:4-61`; `cast/Source/Validator.h:690-700`; `cast/Source/Validator.cpp:915-929` |
| The Validator base for a `project-info.md` | `jam::MarkdownValidator`: the four structural rules `headerRow`, `tableRow`, `columns`, `alignment`; `getLocation`; `forEachCell`. cast's `Validator` has this base, and cast runs it on a `project-info.md`. `jam::ConfigValidator` adds a `type` rule that "fails a cell that is empty, or that is not a key of ConfigDocument::getValueTypes()"; the `## project info` table of END holds `@char` cells and empty cells in its `type` column. `jam::ConfigDocument` is a `jam::MarkdownDocument`. | `jam/jam_markdown/document/jam_MarkdownValidator.h:18-84`; `cast/Source/Validator.h:19`; `cast/Source/Processor.cpp:26`; `jam_markdown/document/jam_ConfigValidator.h:23`; `end/project-info.md:24-46`; `jam/docs/xml/structjam_1_1ConfigDocument.xml:5` |
| The form of a rule | `rules.add (Id::headerRow.toString(), [] (const juce::String&, const Document& document) -> juce::Result { ... return getFailures (failures); })`: the entry takes the key first, collects each failure in a `jam::Strings`, and returns `Document::Validator::getFailures`. `Rules` is `jam::Function::Map<juce::String, jam::Function::pointerCapacity, juce::Result (const Document&)>`. | `jam/jam_markdown/document/jam_MarkdownValidator.cpp:16-29`, `:85-97`; `jam_core/document/jam_Document.h:841`, `:870` |
| A missing cell | `getTableCell (row, colId)`: "A pointer to the matching cell, or nullptr if not found"; `getTableValue (row, colId)`: "an empty string if the cell is not found"; `Id::headerRow` | `jam_markdown/document/jam_MarkdownDocument.h:223-229`; `jam/cast/markdown.md:54` |
| The toolchain argument | `Processor::run (const juce::String& toolchainArgument)`: a row is used when `getColumnValue (*row, Id::argument).compare (toolchainArgument) == 0`; an argument that is given and matches no row fails with `failToolchainArgument`. On the command line it is the text after the double dash of the argument that follows the manifest. "A blank `argument` cell marks a default-flow row." | `cast/Source/Processor.cpp:61-87`; `cast/Source/main.cpp:367-392`; `cast/Source/HELP.md:24-25`, `:539-541`; `cast/cast/text.md:31` |
| The tokens of a `flag` cell in cast | `Toolchain::getToolchainArguments`: `addTokens (flag, true)`, then every double quote removed. For a cell with no quote and no backslash, `jam::ChildProcess::getArguments` gives the same tokens. | `cast/Source/Toolchain.cpp:3-13` |
| A table keyed by family, read with `contains` and then `get`; each entry takes the key as its first parameter | `getVocabularyByFamily`, `getVocabularyForFamily`; `getDataVocabulary (const int&)` | `jam_CodeDocument.cpp:190`, `:196-224` |
| A walk over a generated bimap | `for (auto& [key, stem] : map::FileConfig::getInstance()->get())`; the holder `static const jam::SharedInstance<map::CppKeyword> cppKeyword { std::in_place };`; `map::Family` is a `@code:bimap` with `base: @bimap` | `end/Source/config/ConfigModel.cpp:51`; `jam_CodeDocument.cpp:228-230`; `jam/cast/spell.md:394-402` |
| A generated struct of text rows | the group `@text:diagnostics`, `@code:struct`, name `Diagnostics`, `@code:char` | `scout/cast/spell.md:173-180` |
| A message from an object that takes the model | `model.setMessage (...)` | `end/Source/plugins/Session.cpp:114`, `:122` |
| A run of a toolchain row that blocks until the child ends | `Toolchain::runProcess`: `subprocess.launch (arguments, workingDirectory, onComplete, onChunk)`, `finished.wait()` on a `juce::WaitableEvent`; "`launch()` and `kill()` may be called from any thread, including a console main thread that blocks on completion via a `juce::WaitableEvent`"; the row runs in `model->getFile (row).getParentDirectory()` | `scout/Source/Toolchain.cpp:15-41`; `jam_subprocess/subprocess/jam_Subprocess.h:12-13`, `:95-98`; `cast/Source/Processor.cpp:135-147` |
| Every chunk of a child, with no limit | `processChunk` gives each chunk to `chunkCallback`; the cap of `byteCap` (100,000 bytes) is on the capture of `onComplete` only | `jam_subprocess/subprocess/jam_Subprocess.cpp:134-148`, `:163-173` |
| A full path from a text that can be absolute | `juce::File::getChildFile`: "If the string is actually an absolute path, it will be treated as such"; `..` resolved | JUCE `juce_core/files/juce_File.h:261-275` |
| Work off the message thread, and back | `juce::ThreadPool`; `juce::MessageManager::callAsync` | JUCE `juce_core/threads/juce_ThreadPool.h`; `juce_events/messages/juce_MessageManager.h` |
| A document made on the heap from a parse | `std::make_unique<CssDocument> (parse (...))` | `jam/jam_web/css/jam_Css.cpp:108` |
| A registered function per family | `getVocabularyByFamily`, a `Function::Map` keyed by `map::Family` | `jam_CodeDocument.cpp:196-215` |
| Extension to family | `map::languageFamily`; `CodeDocument::getLanguageFamily`, private, with `invalidFamily` | `jam_CodeDocument.cpp:246-262`; `jam_CodeDocument.h:70`, `:207` |
| Extension rows | `## extensions` has `c`, `cpp`, `h`. It has no `cxx` and no `mm`. | `jam/cast/core.md:2365-2407` |
| `Id::path` | exists | `jam/cast/core.md:125` |
| The file argument and the error line of LING | `getDocumentFile`, `runDocument`, `runDocumentNotFound`, `printError` | PLAN-ling-4-language-document.md Step 8 |
| LING's own words | `cast/text.md`, `cast/identifiers.md`, generated into `Source/generated/` | PLAN-ling-1-project.md Steps 6 and 7 |

### Names

| Name | Source |
|---|---|
| `jam::SyntaxTree` | ruling 41, verbatim |
| `documentConstruction`, `getDocumentConstruction` | `treeConstruction` (`jam_Xml.cpp:226`), `getBlockConstruction` (`jam_Css.cpp:1896`) |
| `SyntaxTree::getFamily`, `SyntaxTree::parse` | the verb `get`; `parse`, as every document type |
| `jam::ChildProcess::getArguments` | `Toolchain::getToolchainArguments`, which returns an argv from a command and a flag (`scout/Source/Toolchain.cpp:3-13`); `getQuotedArgument`, its reverse in the native pair (`jam_ChildProcess_windows.cpp:115-145`) |
| `paths` | the plural of `Id::path` |
| `LINGModel` | `ENDModel`: the project name and `Model` (`end/Source/end/ENDModel.h:19`) |
| `Processor` | scout and cast, verbatim; MANIFESTO: "The Processor holds the business logic" |
| `setMessage`, `Id::message`, `Id::overlay` | `ENDModel`, verbatim (`ENDModel.cpp:8-19`) |
| `fileChanged`, `watcher`, `coalesceMs` | `ConfigModel`, verbatim (`ConfigModel.cpp:116-127`) |
| `Id::dependencies` | ninja's word, `deps`, in full; ruling 14: "the graph, the relations, the docs, the deps" |
| `Id::output` | the key `output` of an entry of `compile_commands.json`, verbatim |
| `CompilerManifest` | ARCHITECT's term, ruling 74: *"the compiler manifest is `compile_command.json`"* |
| `CompilerManifest::getFiles` | the verb `get`; NAMES.md Rule 7: named for what it yields, the `FILES` row |
| `filesConstruction`, `getFilesConstruction` | `documentConstruction`, `getDocumentConstruction` (this part) |
| `addCLanguageFiles` | the verb `add` and its container, `files`; `CLanguage` from `CLanguageDocument` (ruling 26) |
| `Id::toolchainArgument`, `getToolchainArgument`, text row `failToolchainArgument` | cast, verbatim (`cast/Source/Processor.cpp:61`; `cast/Source/main.cpp:385`; `cast/cast/text.md:31`) |
| `Id::toolchain`, `Id::argument`, `Id::flag` | cast's rows, verbatim (`cast/cast/identifiers.md:83`, `:23`, `:51`) |
| `cast/c_language.md` | the file family `syntax_tree.md`, `data_structures.md` (`jam/cast/`); `CLanguage` from ruling 26 |
| `text::CLanguage` | `text::Diagnostics` (`scout/cast/spell.md:178-179`) |
| Rows `compilerManifestCommand`, `flagBuildDirectory`, `compilerManifest` | `command` and `flag`, the column words of `## toolchain`; `Builds/Debug`, the build folder of the `-B` flag (`end/project-info.md:128`); ARCHITECT's term (ruling 74) |
| Rows `dependencyCommand`, `flagDependencyDirectory`, `dependencyFlag`, `dependencyHead` | the same column words; `dependency` from `Id::dependencies`; the folder of the `-C` flag (`end/project-info.md:130`); the head line of a record |
| Text rows `failUnitNotFound`, `failCompilerManifestInvalid`, `failCompilerManifestNotFound`, `failDependenciesNotFound` | the LING row `failFlagUnknown` (PLAN-ling-1-project.md Step 6); `failNotFound` of scout (`scout/Source/Processor.cpp:83`) |
| Text row `failToolchain` | cast and scout, verbatim (`cast/cast/text.md:30`; `scout/Source/Toolchain.cpp:38`) |
| `Validator`, `addRules`, `getRules` | scout and cast, verbatim (`scout/Source/Validator.h:17-36`; `cast/Source/Validator.h:19`) |
| Rule key `toolchain`; text row `failToolchainColumn` | scout and cast, verbatim (`scout/Source/Validator.cpp:8`; `scout/cast/text.md:12`; `cast/cast/text.md:32`) |

## The design

### In JAM

**The registry.** `struct SyntaxTree` has no state:

- `static int getFamily (const juce::File& file);` returns `CodeDocument::getLanguageFamily` of the file extension without its dot. `getLanguageFamily` and `invalidFamily` become public.
- `static std::unique_ptr<CodeDocument> parse (std::string_view sourceText, int family);` reads the table `documentConstruction`, keyed by `map::Family`. Each entry makes one document on the heap from the source text. A family with no entry gives `nullptr`.
- The table has one entry in this part: `map::Family::cLang` gives `std::make_unique<CLanguageDocument> (CLanguageDocument::parse (sourceText))`.

A second language is one more entry (ruling 23).

**The argument splitter.** `static jam::Strings getArguments (std::string_view commandLine);` is a new static member of `jam::ChildProcess` (`jam_subprocess/subprocess/jam_ChildProcess.h:15-93`). It returns the argv of a `command` string of a compiler manifest of the C family, by the rule of clang's own reader (`JSONCompilationDatabase.cpp:130-152`). It is the reverse of the quoting that the native pair already owns: `getQuotedArgument` (`native/jam_ChildProcess_windows.cpp:115-145`).

| Platform | The split |
|---|---|
| POSIX (`native/jam_ChildProcess_posix.cpp`) | the GNU parser of clang (`JSONCompilationDatabase.cpp:47-128`): spaces separate arguments; `"..."` groups, and `\` escapes the next byte in it; `'...'` groups with no escape; outside quotes `\` escapes the next byte. Each argument is built in one `std::string` work buffer and added with `jam::Strings::add (std::string_view)` (`jam_Strings.h:224`). This is creation. |
| Windows (`native/jam_ChildProcess_windows.cpp`) | `CommandLineToArgvW`, which takes UTF-16 text (`LPCWSTR`). The text is `juce::String::fromUTF8 (...).toWideCharPointer()`, a JUCE argument (ruling 50). The result goes to `jam::Strings (const wchar_t* const*, int)` (`jam_Strings.h:160`), then one `LocalFree`, as `juce_ApplicationBase.cpp:174-186` does. The Windows step checks it against the command lines that CMake writes on Windows (`cmOutputConverter.cxx:613-635`: "On Windows only backslashes and double-quotes need escaping"). |

### In LING: the state tree

`class LINGModel : public jam::Model, public jam::Instance<LINGModel>` has the form of `ENDModel` (`end/Source/end/ENDModel.h:19-21`; `ENDModel.cpp:3-19`). It also listens to a `jam::File::Watcher`, as `ConfigModel` does (`end/Source/config/ConfigModel.cpp:116-127`).

```
LING (Id::ling)                 Id::toolchainArgument, Id::manifest (path), Id::documents (int)
  FILES (Id::files)
    FILE (Id::file)             Id::path, Id::family, Id::output, Id::dependencies
  OVERLAY (Id::overlay)         Id::message (ParameterText)
```

| Row or property | Author | Meaning |
|---|---|---|
| `Id::toolchainArgument`, `Id::manifest` on the root | the application, from the command line | the toolchain argument, which is empty for the default flow, and the path of the manifest, `project-info.md` |
| a `FILE` row | `CompilerManifest`, for each unit of a compiler manifest and for each file of its dependency record | one source file. `Id::path` is its full path. `Id::family` is its `map::Family` key. A unit row also holds `Id::output`, the full path of its output, and `Id::dependencies`, the child indices, in the `FILES` row, of the files that the compiler read for it. |
| a change message for `Id::path` of a `FILE` row | `LINGModel::fileChanged` | an event of the watcher for the file: the disk can hold new text, or no file |
| `Id::message` | any object, through `LINGModel::setMessage` | one line for the user: a failure, or the end of a load |
| `Id::documents` on the root | `Processor` | the count of documents, written each time no parse is pending; one change message each time, also for an equal count |

`LINGModel`:

1. The constructor makes the `FILES` row and the `OVERLAY` row with its `Id::message` parameter, and starts the watcher: `watcher.coalesceEvents (coalesceMs); watcher.addListener (this);`.
2. `valueTreePropertyChanged` calls the `jam::Model` body first (`jam_data_structures/model/jam_Model.h:843`; `jam_Model.cpp:364-378`): `jam::Model` uses this callback for its parameters. For `Id::manifest` on the root it then reads the file that the property names with `jam::ConfigDocument::parse`, in the form of `scout/Source/Processor.cpp:13`. It then runs `Validator {}.isValid (manifest)` on the document, as scout does before its run (`scout/Source/Processor.cpp:19-22`; section In LING: the manifest check). When the result is ok, it calls `CompilerManifest::getFiles` with itself, the document, the file, and the text of `Id::toolchainArgument` of the root (section In LING: the compiler manifest). The text is the `juce::String` that the property gives, a JUCE output (ruling 50). `getFiles` returns a new `FILES` row off the tree, a `juce::ValueTree` with no parent, with one `FILE` row for each file. No listener hears the adds to that row. This is the form of `ConfigModel::loadFromPath`, which builds the complete tree before it gives it to the model (`end/Source/config/ConfigModel.cpp:82-98`). `LINGModel` then removes the `FILES` row of the tree with `removeChild` and adds the new one with `appendChild` (`jam_data_structures/model/jam_Model.h:866`): "A complete state replaces a complete state" (MANIFESTO **E**). JUCE sends one `valueTreeChildAdded` for the one call (`juce_data_structures/values/juce_ValueTree.cpp:273-275`). So each listener gets the rows of every compiler manifest complete, in one event. A `FILES` row with no child is a valid result. When the result of `isValid` is a failure, the reaction calls `setMessage` with `result.getErrorMessage()` (the text that scout prints, `scout/Source/Main.cpp:170`) and reads no table of that manifest. It removes the `FILES` row with `removeChild` and makes the row again as item 1 makes it. The same manifest thus gives the same tree, whatever the tree held before (MANIFESTO **D**). `Processor` then writes `Id::documents` at once, as for each `FILES` row with no child, and the application prints the message and the row `0 documents`, as for each other failure of this read (Step 7 item 4).
3. `valueTreeChildAdded` calls the `jam::Model` body first (`jam_Model.h:852`; `jam_Model.cpp:402-405`). For the `FILES` row it then adds the parent directory of each `FILE` child to the watcher with `watcher.addFolder`.
4. `fileChanged (file, event)` finds the `FILE` row with that path by `getChildWithProperty (Id::path, ...)`. For any event type it calls `sendPropertyChangeMessage (Id::path)` on the row, as `ConfigModel` reloads for any event type (`end/Source/config/ConfigModel.h:147-155`, `ConfigModel.cpp:123-127`). The job reads the disk, and the disk is the truth. A file with no row gives nothing. So an editor that saves by rename gives one more parse: after coalescing, the last event of the file is `fileCreated` or `fileRenamedNewName` (`jam/jam_core/file/jam_Listener.h:89-118`).

`jam::Model` is already a `juce::ValueTree::Listener` (`jam/jam_data_structures/model/jam_Model.h:38-41`). The watcher calls its listener on the message thread (`jam/jam_core/file/jam_Listener.h:71-77`).

### In LING: the manifest check

cast and scout check a manifest before they read it (`cast/Source/Validator.h:690-700`; `scout/Source/Processor.cpp:19-22`). LING does the same (ruling 79).

`struct Validator : jam::MarkdownValidator` has the form of scout's `Validator` (`scout/Source/Validator.h:17-37`; `scout/Source/Validator.cpp:4-61`), with the base of cast's `Validator` (`cast/Source/Validator.h:19`). The base is not `jam::ConfigValidator`: its `type` rule fails an empty `type` cell (`jam_ConfigValidator.h:23`), and the `## project info` table of a cast manifest holds such cells (`end/project-info.md:24-46`). `Validator` has no state.

- `static void addRules (Rules& rules);` calls `jam::MarkdownValidator::addRules (rules)`, then adds the rule of the table below.
- `const Rules& getRules() const override;` returns the rules, built one time, in the form of `MarkdownValidator::getRules` (`jam_MarkdownValidator.cpp:85-97`).

| Rule key | Failure |
|---|---|
| `Id::toolchain.toString()` | a `## toolchain` table whose header row, `config.getTableRow (*table, Id::headerRow)`, has no `Id::command` cell or no `Id::flag` cell: `config.getTableCell (*headerRow, ...) == nullptr`. The failure text is `getLocation (*table, *headerRow, Id::toolchain.toString())`, then `Id::diagnosticSeparator`, then the text row `failToolchainColumn`, as scout and cast write it (`scout/Source/Validator.cpp:13-21`; `cast/Source/Validator.cpp:917-925`). |

The rule has the form of the JAM rules (`jam_MarkdownValidator.cpp:18-29`): the entry takes the key first, reads the document as `static_cast<const jam::ConfigDocument&> (document)` (`scout/Source/Validator.cpp:11`), adds each failure to a `jam::Strings`, and returns `getFailures (failures)`. No rule checks the `argument` column: cast has no such rule (`cast/Source/Validator.cpp:915-929`), and a row with no `argument` cell reads as a blank cell, the default flow (`jam_MarkdownDocument.h:226-229`).

`LINGModel` runs the check one time for each read of the manifest, before it reads a table (item 2 of `LINGModel`). `CompilerManifest` and each later reader of that document trust the result and hold no check of a column (MANIFESTO **D**; ruling 69: *"always trust validator"*).

### In LING: the compiler manifest

Ruling 74: each language has its own compiler manifest, and the manifest of LING is `project-info.md`. `struct CompilerManifest` has no state. It has the form of `jam::SyntaxTree` (section In JAM): static functions and a table keyed by `map::Family`. It is the one object of LING that names a family together with a build tool, a file name, or a flag word. A second language is one more table file in `ling/cast/` and one more entry (ruling 23).

`static juce::ValueTree getFiles (LINGModel& model, const jam::ConfigDocument& manifest, const juce::File& manifestFile, const juce::String& toolchainArgument);`

1. It makes a `juce::ValueTree` of the type of `Id::files`, with no parent.
2. It walks the `## toolchain` rows as cast does (`cast/Source/Processor.cpp:66-79`): `manifest.getTables (Id::toolchain)`, then `manifest.getTableRows (*table)`, in authored order (`scout/Source/Processor.cpp:38-39`). It reads a row when `manifest.getTableValue (*row, Id::argument).compare (toolchainArgument) == 0`. An empty toolchain argument thus reads the default-flow rows, whose `argument` cell is blank (`cast/Source/HELP.md:541`).
3. For each row that it reads, and for each key of `map::Family` that `filesConstruction` holds, it calls the entry with `model`, the new row, `manifest`, the toolchain row, and `manifestFile.getParentDirectory()`. A toolchain row runs "in the directory of the file that declares its `## toolchain` table" (`HELP.md:539`). The keys come from `map::Family::getInstance()->get()`, in the form of `end/Source/config/ConfigModel.cpp:51`, with the holder `static const jam::SharedInstance<map::Family>` in the form of `jam_CodeDocument.cpp:228-230`. `contains` comes before `get`, as in `getVocabularyForFamily` (`jam_CodeDocument.cpp:221-223`).
4. When the toolchain argument is not empty and no row is read, it calls `model.setMessage` with the argument, `Id::diagnosticSeparator`, and the text row `failToolchainArgument`, as cast fails (`cast/Source/Processor.cpp:82-84`).
5. It returns the row.

`filesConstruction` is a static table behind the private `getFilesConstruction()`, in the form of `getVocabularyByFamily` (`jam_CodeDocument.cpp:196-215`). The map type holds the signature: `jam::Function::Map<int, jam::Function::pointerCapacity, void (LINGModel&, juce::ValueTree&, const jam::ConfigDocument&, const jam::ConfigDocument::Element&, const juce::File&)>`, in the `jam::Function::Map` form that `jam_core/function_map/jam_Function.h` declares at the time of execution. The table has one entry in this part: `map::Family::cLang` gives `addCLanguageFiles`.

**The C family entry.** `static void addCLanguageFiles (const int& family, LINGModel& model, juce::ValueTree& files, const jam::ConfigDocument& manifest, const jam::ConfigDocument::Element& row, const juce::File& directory);` is private. An entry takes the key as its first parameter, as the entries of `getVocabularyByFamily` do (`jam_CodeDocument.cpp:190`, `:204-207`). Its words are rows of `ling/cast/c_language.md` (Step 4):

| Row | Value | Generated name |
|---|---|---|
| `compilerManifestCommand` | `cmake` | `text::CLanguage::compilerManifestCommand` |
| `flagBuildDirectory` | `-B` | `text::CLanguage::flagBuildDirectory` |
| `compilerManifest` | `compile_commands.json` | `text::CLanguage::compilerManifest` |
| `dependencyCommand` | `ninja` | `text::CLanguage::dependencyCommand` |
| `flagDependencyDirectory` | `-C` | `text::CLanguage::flagDependencyDirectory` |
| `dependencyFlag` | `-t deps` | `text::CLanguage::dependencyFlag` |
| `dependencyHead` | `: #deps` | `text::CLanguage::dependencyHead` |

The `command` cell of the toolchain row selects one of two reads, each with `manifest.getTableValue (row, Id::command).compare (...) == 0`. The text `compilerManifestCommand` gives the units. The text `dependencyCommand` gives the files. A row with another command gives nothing.

**The units**, from the `cmake` row:

1. The tokens of the `flag` cell are `jam::ChildProcess::getArguments (jam::Strings::getView (flag))`. The token after the token `flagBuildDirectory` is the build folder. END and the cli archetype write the flag and the folder as two tokens (`end/project-info.md:118`, `:128`; `archetype/cli/init.cast:166`, `:174`). A row with no such token gives nothing, for example `cmake`, `-P gh.cmake` (`end/project-info.md:122`).
2. The compiler manifest is `directory.getChildFile (<build folder>).getChildFile (text::CLanguage::compilerManifest)`. A file that does not exist is a `model.setMessage` with `failCompilerManifestNotFound` and its path. No row is added.
3. It reads the file with `juce::JSON::parse`. The root is an array. Another root is a `model.setMessage` with `failCompilerManifestInvalid` and the path, and no row is added. Each entry gives `file`, `directory`, `output`, and the command tokens: the `arguments` array when the entry has one, otherwise `jam::ChildProcess::getArguments (jam::Strings::getView (command))`. `command` is the `juce::String` that `juce::var` gives, a JUCE input (ruling 50). This part reads no token of the command; Part 10 keeps the tokens.
4. It adds one `FILE` row to `files` for each entry. `Id::path` is the full path of `file`. `Id::family` is `jam::SyntaxTree::getFamily` of the file. `Id::output` is the full path of `output`: `juce::File (directory).getChildFile (output)`. A unit file that does not exist is a `model.setMessage` with `failUnitNotFound` and its path; no row is added for it.

**The files**, from the `ninja` row (rulings 75 and 76):

1. The tokens of the `flag` cell are `jam::ChildProcess::getArguments (jam::Strings::getView (flag))`. The build folder is `directory.getChildFile` of the token after the token `flagDependencyDirectory`. With no such token the build folder is `directory`.
2. The argv is the `command` cell, then each token of the `flag` cell, then each token of `jam::ChildProcess::getArguments` of `text::CLanguage::dependencyFlag`. It is a `juce::StringArray`, because `jam::Subprocess::launch` takes one in this part (`jam_subprocess/subprocess/jam_Subprocess.h:95`); each token is read with `jam::Strings::at`. Part 8 Step 1 items 6 and 7 change `launch` and this call to `jam::Strings` (ruling 52).
3. It runs the argv in `directory`, in the form of `Toolchain::runProcess` (`scout/Source/Toolchain.cpp:15-41`): `jam::Subprocess::launch` with `onChunk` and `onComplete`, then a wait on a `juce::WaitableEvent`. `onChunk` appends each chunk to one `std::string`. `onComplete` keeps the exit code and signals the event. cast runs the same row in the same folder (`cast/Source/Processor.cpp:135-147`); the flag `-t deps` makes ninja print the record and build nothing. An exit code that is not 0 is a `model.setMessage` with the command, the flag, and the text row `failToolchain`, and nothing is read.
4. It reads the text by lines, each a `std::string_view` over the one `std::string` (ruling 50). A line that does not start with `Chars::space` and that holds `text::CLanguage::dependencyHead` is a head. The text before `dependencyHead` is an output, and its full path is `<build folder>.getChildFile (<output>)`. The unit of the head is `files.getChildWithProperty (Id::output, <full path>)`. A head with no unit row is not read: its file lines are passed over.
5. A line that starts with `Chars::space` is a file of the current head: its text after the leading spaces. Its full path is `<build folder>.getChildFile (<text>)`; an absolute path stays as it is.
6. For each file of a head with a unit, the entry finds the child index of the `FILE` row with that path, or adds the row. A local `jam::Strings paths` holds the `Id::path` of each child of `files`, in child order, so `paths.indexOf (path, false, 0)` is the child index, in O(1) (`jam_core/text/jam_Strings.h:334-348`). A path that `paths` does not hold gets a new `FILE` row with `Id::path` and the `Id::family` of the unit, and goes into `paths`. The local is a work buffer of creation (MANIFESTO **S**, transient state).
7. The child indices of one record, in the order of its lines, go to `Id::dependencies` of the unit row, a `juce::var` array, because a `juce::ValueTree` property takes a `juce::var` (ruling 50).
8. After the last line, each unit row with no `Id::dependencies` is a `model.setMessage` with `failDependenciesNotFound` and its path.

Each text compare is `compare (...) == 0` (CODING.md).

**The files are those of the last build** (ruling 75). The record holds each file that the compiler read for a unit: the project files, the module files, and the files below the compiler's own search paths. LING holds no search path and no include flag, and it asks no compiler. A file takes the family of the first unit, in the order of the compiler manifest, whose record holds it.

### In LING: the documents

`class Processor : public juce::ValueTree::Listener` holds the logic (MANIFESTO **E**, MVP). It listens to the tree of `LINGModel` with `jam::Model::addListener`, because `LINGModel` replaces the `FILES` row; it does so as `Session` listens to its rows (`end/Source/plugins/Session.cpp:21-28`, `:59-69`). It holds:

- `jam::Strings paths;`: each full path one time, in the order it first comes. `paths.indexOf (path, false, 0)` is "a single O(1) hash lookup ... zero allocation" (`jam_core/text/jam_Strings.h:334-348`). A path is never removed, so its index never changes.
- `jam::HashMap<int, std::unique_ptr<jam::CodeDocument>> documents;`: the key is the index of the path in `paths`. A path with no document has no entry, so no reader tests a document for null (MANIFESTO **D**), and a file and its document are a key and a value of one map (Language / Framework Constraints). `addOrReplace` puts or replaces a document (`jam_data_structures/model/jam_Model.cpp:193-194`), `erase` removes it, and `contains` and `at` read it. This is the registry form: "identity is assigned and released through the registry's own API" (MANIFESTO **E**, Complete at Creation). `paths` assigns the index;
- `juce::ThreadPool pool;`.

| Event | Reaction |
|---|---|
| `valueTreeChildAdded`: the `FILES` row | start one job in `pool` for each `FILE` child |
| `valueTreePropertyChanged`: `Id::path` of a `FILE` row | start one job in `pool` for that row |
| `valueTreeChildRemoved`: the `FILES` row | erase the document of each `FILE` child of the removed row |

`Processor` gets every row of the compiler manifests in one event, and starts every job at that event. The jobs run in parallel in `pool`. When the `FILES` row has no child, no job starts, and `Processor` writes `Id::documents` at once.

A job loads the text of the file with `juce::File::loadFileAsString()` (`scout/Source/Processor.cpp:13`; a JUCE return, ruling 50) and calls `SyntaxTree::parse (text, family)` on the pool thread. A file that does not exist gives no document: on the message thread, `Processor` erases the document of that path. The row stays, because the row names the path and the file can come back. It then gives the document to the message thread with `juce::MessageManager::callAsync`. On the message thread, `Processor` keeps the document only when the row is still below the root of the tree (`isAChildOf`) and still has that path, as `Session` checks its row before it adds a plugin (`Session.cpp:115-117`). It puts the document into `documents` with `addOrReplace`; a document that is already there for that path is replaced (MANIFESTO: "A complete state replaces a complete state"). `Processor` adds no row: every row comes from `CompilerManifest`, in the one event.

A file whose family is `invalidFamily`, or whose family has no `documentConstruction` entry, gets no document. A document that `CLanguageValidator` rejects is a `setMessage` with the failure text, and it is kept: the tree is what the file gives.

When a job ends and no job is pending, `Processor` writes the count of documents to `Id::documents` on the root. `setProperty` sends no message for an equal value (`juce_data_structures/values/juce_ValueTree.cpp:144-145`; `juce_core/containers/juce_NamedValueSet.cpp:156-159`). When the count is equal to the value that the property holds, `Processor` calls `sendPropertyChangeMessage (Id::documents)` in place of the write, the change message with no new value (`juce_ValueTree.h:575-578`). Thus each end of the pending jobs gives exactly one `Id::documents` event.

### In LING: the application

`ling <project-info.md> [--<word>]`:

1. `main` makes the JUCE message loop, a `LINGModel`, and a `Processor`. It writes `Id::toolchainArgument` and then `Id::manifest` on the root.
2. The application holds one `jam::Model::ParameterAttachment` on the `Id::message` parameter, as `MessageOverlay` does (`end/ARCHITECTURE.md:686-688`). Its callback prints each text to stderr with `printError`. `setValue` tells the attachment at once on the message thread (`jam_data_structures/model/jam_ParameterText.cpp:43-47`; `jam_Model.cpp:220-234`), so each `setMessage` gives one print. The tree is not the source of the print: the flush writes the text one time for each timer tick, and only when it differs (`jam_Model.cpp:33-58`). The application also listens to the root. On the change of `Id::documents` it prints the rows below to stdout and stops the message loop with exit code 0.
   - One row for each document, in sorted path order: `<path>: <declarations> declarations, <references> references`.
   - One last row: `<documents> documents`.

The exit at the first `Id::documents` change is the form of this part only. Part 8 keeps the process, and then a file change gives a new document with no new start.

A first argument that is an existing file is the manifest (ruling 70). The argument after the manifest is the toolchain argument when it starts with `Chars::doubleDash`: its text after the two dashes, as cast takes it (`cast/Source/main.cpp:367-392`; `cast/Source/HELP.md:24-25`). With no such argument the toolchain argument is empty, and LING reads the default-flow rows. `ling end/project-info.md --debug` reads the rows that `cast cast/spell.md --debug` runs. The tree print of Parts 4 and 5 leaves `Main.cpp` in this part.

No part adds a table to a `project-info.md` for this read (ruling 72). The manifest of a project serves LING as it is.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: Add the rows
**Scope:** `jam/cast/core.md` (`## extensions`); `jam/cast/syntax_tree.md` (`## languageFamily`); `jam/generated/`.
**Action:**
1. `## extensions`: add `cxx` (`` `cxx` ``, `C++ source.`) and `mm` (`` `mm` ``, `Objective-C++ source.`), in name order, in the cell shape of the `cpp` row (`core.md:2381`).
2. `## languageFamily`: add `Extensions::cxx` and `Extensions::mm`, each with `@cLang`, after the `Extensions::h` row.
3. Run cast on `jam/cast/spell.md`.

**Validation:** The two extensions are the two unit extensions of the compiler manifest that the table did not hold. No other row is added (YAGNI). No table was added (ruling 35).

### Step 2: `CodeDocument`
**Scope:** `jam/jam_syntax_tree/document/jam_CodeDocument.h`.
**Action:** Move `getLanguageFamily` and `invalidFamily` to `public`.

**Validation:** No other member of `CodeDocument` changes. The reader of the two is `SyntaxTree::getFamily` (Step 3). No `#include` element comes in this part: it has no reader (rulings 75 and 76; PLAN-ling-2-language-tables.md, What this part does not do: each new row comes in the part that holds its first reader).

### Step 3: `jam::SyntaxTree`
**Scope:** new files `jam/jam_syntax_tree/document/jam_SyntaxTree.h` and `.cpp`; `jam/user-modules-info.md`.
**Action:**
1. Add the two files to `## submodule` after the `jam_CLanguageValidator` rows, class `kernel`, with the aggregators `jam_syntax_tree.h` and `jam_syntax_tree.cpp`. Run cast on `jam/cast/spell.md` (tracker, The contract of every part, cast runs).
2. Write `struct SyntaxTree` with `getFamily`, `parse`, and the private `getDocumentConstruction()` that returns the static table `documentConstruction`, as section In JAM gives them. The map type holds the signature: `jam::Function::Map<int, jam::Function::pointerCapacity, std::unique_ptr<CodeDocument> (std::string_view)>`, in the `jam::Function::Map` form that `jam_core/function_map/jam_Function.h` declares at the time of execution.

**Validation:** `SyntaxTree` is the only place that names a language type together with a family. Its header comes after `jam_CLanguageDocument.h` in the module header, by the order of the manifest rows. No member data.

### Step 4: `Validator`, `CompilerManifest`, and `LINGModel`
**Scope:** new files `ling/Source/Validator.h` and `.cpp`, `ling/Source/CompilerManifest.h` and `.cpp`, `ling/Source/LINGModel.h` and `.cpp`, and `ling/cast/c_language.md`; `ling/cast/spell.md`; `ling/cast/text.md`; `ling/cast/identifiers.md`; `ling/project-info.md` (`## user module` only); `jam/jam_subprocess/subprocess/jam_ChildProcess.h`; `native/jam_ChildProcess_posix.cpp`; `native/jam_ChildProcess_windows.cpp`.
**Action:**
1. `jam::ChildProcess::getArguments`: declare it after `start` in `jam_ChildProcess.h`, and write the two native bodies as section In JAM, The argument splitter gives them. The platform header `<shellapi.h>` goes at the top of the Windows native file. `ling/project-info.md` `## user module`: add the `jam_subprocess` row of `scout/project-info.md:142`, in scout's order.
2. `cast/c_language.md`, new. It holds the words of the C family entry and no code. It has one table, `## cLanguage`, in the form of the `## diagnostics` table of `cast/text.md` (PLAN-ling-1-project.md Step 6 item 2): the brief `@brief The words of the C family compiler manifest and dependency record.`, the columns `name`, `value`, `description`, and these rows, each value between backticks:

   | name | value | description |
   |---|---|---|
   | compilerManifestCommand | `cmake` | The command cell of the toolchain row that names the build folder. |
   | flagBuildDirectory | `-B` | The flag whose next token is the build folder. |
   | compilerManifest | `compile_commands.json` | The file name of the compiler manifest in the build folder. |
   | dependencyCommand | `ninja` | The command cell of the toolchain row that gives the dependency record. |
   | flagDependencyDirectory | `-C` | The flag whose next token is the build folder of that row. |
   | dependencyFlag | `-t deps` | The flag that makes the command print the dependency record. |
   | dependencyHead | `: #deps` | The text that follows the output path in the head line of a record. |

3. `cast/spell.md`:
   - `## index`: add the row `@cLanguage`, `c_language.md` after the `@text` row.
   - `## output`: add one row group after the `@text:diagnostics` group, in its form (`scout/cast/spell.md:173-180`) without the `- macro:` line and the `- [description]:` line: list `> - [list]: @cLanguage:cLanguage`; structure `@code:namespace`, `- name: text`, then `@code:struct`, `- name: CLanguage`, `> - [list]: @code:char`; file `@Text`.
4. `cast/text.md`: add the rows `failUnitNotFound` `` `unit file not found` ``, `failCompilerManifestInvalid` `` `compiler manifest is not valid` ``, `failCompilerManifestNotFound` `` `compiler manifest not found` ``, `failDependenciesNotFound` `` `dependencies not found` ``, and two rows with the texts of cast (`cast/cast/text.md:30-31`): `failToolchain` `` `toolchain command failed` `` and `failToolchainArgument` `` `toolchain argument not declared` ``. Add one more row with the text of scout and cast (`scout/cast/text.md:12`; `cast/cast/text.md:32`): `failToolchainColumn` `` `toolchain column not declared` ``.
5. Identifiers: use the JAM `Id` where it exists (`Id::file`, `Id::path`, `Id::message`, `Id::overlay`). Add a row to `ling/cast/identifiers.md` for each one that JAM does not have: `ling`, `manifest`, `toolchainArgument`, `toolchain`, `argument`, `flag`, `files`, `family`, `output`, `dependencies`, `documents`, and the JSON key `arguments`. `output` is in `jam/cast/vulkan.md:60`, and LING does not compile `jam_vulkan` (tracker Risks item 18). No `@id` row in a `jam/cast/*.md` file holds `dependencies` (read of 2026-11-11). No `@id` row in a `jam/cast/*.md` file holds `toolchain`, `argument`, or `flag` (read of 2026-11-11). `Id::directory` exists in `jam/cast/core.md:71`, and `Id::command` in `jam/cast/terminal.md:45` (`jam_terminal`, one of LING's thirteen modules, PLAN-ling-1-project.md, The module rows that this part needs). A second row would define either one two times. `Id::headerRow` exists in `jam/cast/markdown.md:54` (`jam_markdown`, one of the thirteen modules), and `Id::diagnosticSeparator` in `jam/cast/core.md:70`. Run cast (tracker, The contract of every part, cast runs).
6. Write `Validator` as section In LING: the manifest check gives it. `Validator.h` includes `<JuceHeader.h>` and `generated/Generated.h`, as `scout/Source/Validator.h:2-3`.
7. Write `CompilerManifest` as section In LING: the compiler manifest gives it. `Source/generated/Text.h` holds `text::CLanguage` after item 5.
8. Write `LINGModel` as section In LING: the state tree gives it. `setMessage` has the body of `ENDModel::setMessage` (`ENDModel.cpp:12-19`).

**Validation:** `LINGModel` holds the tree, the watcher, and no document. Each row and each property has one author, as the table gives it. `CompilerManifest` has no member data. Each word of the C family is a row of `cast/c_language.md`: `CompilerManifest.cpp` holds no string literal for a tool, a file name, or a flag, and `LINGModel` holds none. No `==` on strings. A failure is a `setMessage`; no function returns an error code for it. `Validator` has no member data. It holds the one check of the manifest: `CompilerManifest` and `LINGModel` test no column and no header (MANIFESTO **D**; ruling 69).

### Step 5: `Processor`
**Scope:** new files `ling/Source/Processor.h` and `.cpp`.
**Action:** Write `Processor` as section In LING: the documents gives it. The constructor takes `LINGModel&` and adds itself as a listener of the tree with `jam::Model::addListener`. The destructor removes the listener and lets `pool` end its jobs first, by the order of its members.

**Validation:** `Processor` reacts to exactly the row and the property that the event gives; it does not walk the tree to find what changed (CODING.md, Consume the event payload). A pool thread reads a file and makes a document; only the message thread touches `documents` and the tree (MANIFESTO **B**: "Threads are bound to their context"). The same job serves a new row and a changed row, so the start and a change have one code path. `ling/Source` holds no `CLanguageDocument` and no file extension text. `Processor` holds no flag word, no build tool name, and no file name of a compiler manifest.

### Step 6: The application
**Scope:** `ling/Source/Main.cpp`; `ling/Source/HELP.md`.
**Action:**
1. Delete the body of `runDocument` of Parts 4 and 5, the tree print, first (CAROL.md Refactor-Rewrite Discipline; ruling 70). Write the manifest form of section In LING: the application in its place. The message loop is `juce::ScopedJuceInitialiser_GUI` with `juce::MessageManager::getInstance()->runDispatchLoop()`, and `stopDispatchLoop()` ends it.
2. `static std::string_view getToolchainArgument (int argc, char* argv[], int manifestIndex);` returns the text after `Chars::doubleDash` of the argument that follows `argv[manifestIndex]`, when that argument exists and starts with `Chars::doubleDash`. Otherwise it returns an empty view. It has the role of cast's `getToolchainArgument` (`cast/Source/main.cpp:367-392`), with the text types of ruling 50. This part passes `flagArgIndex` as `manifestIndex`.
3. Describe in `HELP.md`: the manifest argument, which is the `project-info.md` of the project; the toolchain argument `--<word>`, with the meaning that cast gives it; and that, for the C family, LING reads `compile_commands.json` in the build folder that the `cmake` row of those toolchain rows names.

**Validation:** The application writes two properties, and reacts to one property and one parameter. It does not call `Processor` and does not read `documents` through a getter that decides for it: the count comes as the event. This part runs no compiler. Part 10 runs the compiler manifest's own compile command (ruling 49).

### Step 7: Build and check, macOS
**Scope:** builds of LING and END; runs of `ling`; one scout manifest.
**Action:**
1. Build LING and END.
2. Run `ling ~/Documents/Poems/dev/end/project-info.md --debug` two times. The argument is END's manifest, as it is (ruling 65). The run reads `end/Builds/Debug/compile_commands.json`, the compiler manifest of the build folder that the `debug` rows name (`end/project-info.md:128`). Compare the two outputs byte for byte. No run prints a failure of `Validator`: END's manifest passes the four structural rules and the `toolchain` rule.
3. In the output, check: a row exists for `end/Source/Main.cpp`; a row exists for `jam/jam_core/document/jam_Document.cpp`, which only a module unit includes; a row exists for one JUCE header; a row exists for the C++ standard header `vector`; no path has two rows. The count of the last row is the count of distinct files that `ninja -C ~/Documents/Poems/dev/end/Builds/Debug -t deps` prints for the 142 units.
4. Run `ling ~/Documents/Poems/dev/end/project-info.md`. The run reads `end/Builds/Release/compile_commands.json` (`end/project-info.md:118`), and a row exists for `end/Source/Main.cpp`. When that file is not on disk, the run prints the `failCompilerManifestNotFound` message with its path and the row `0 documents`. Then run `ling ~/Documents/Poems/dev/end/project-info.md --nothing`: the run prints the `failToolchainArgument` message and the row `0 documents`. Then write a file `project-info.md` in a temporary folder with one `## toolchain` table whose header row holds `argument` and `command` only, and run `ling` with that file: the run prints the location, the `failToolchainColumn` text, and the row `0 documents`.
5. Run `ling ~/Documents/Poems/dev/end/Source/Main.cpp`. The file is an existing file, so it is the manifest (ruling 70). It holds no `## toolchain` table: the run prints the row `0 documents`.
6. In END's compiler manifest, one `command` holds `-DCAST_PLUGINS_HASH=\"<hash>\"`. `getArguments` gives the argument `-DCAST_PLUGINS_HASH="<hash>"`: the bytes that the shell gives, with no backslash. Read the argument with whatdbg at the return of `getArguments` for that entry (CAROL.md Debugging; the trace skill).
7. Measure with scout: one benchmark that runs the command of item 2, with the epochs and the baseline that the profile skill gives. Record the median time and the peak memory.

**Validation:** Items 2 to 6 hold. The order of the rows is the sorted path order, so it does not depend on which job ended first (MANIFESTO **D**). ARCHITECT reads the scout report of item 7 against RFC ruling 4: "ULTRA FAST indexing" and "not hogging cpu nor memory". The check that a file change gives a new document is in Part 8, where the process stays.

### Step 8: Build and check, Windows
**Scope:** a Windows machine.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:**
1. ARCHITECT builds END on the Windows machine with its toolchain rows.
2. Run `ninja -C <the build folder> -t deps` and read one record. Check that its head and its file lines have the form of section The dependency record of the C family, as read on this machine. Record whether a path is relative.
3. Read one `command` of the Windows `compile_commands.json`. Check that `getArguments` gives the argv that CMake escaped (`cmOutputConverter.cxx:613-635`), with whatdbg at its return.
4. Build LING. Run items 2 and 3 of Step 7 on the Windows machine. A row exists for one standard header of the Windows compiler.

**Validation:** The two runs are equal. This step closes the open measurement "the `clang-cl` manifest shape" (tracker Risks item 13).


## BLESSED Alignment

- **B — Bound.** `LINGModel` owns the tree and the watcher. `Processor` owns the documents and the pool. A pool thread owns its text and its new document until it gives the document to the message thread.
- **L — Lean.** Four objects: `LINGModel`, `Processor`, `CompilerManifest`, and `Validator`. No swap in this part. Two extension rows only.
- **E — Explicit.** The manifest and the compiler manifests that its toolchain rows name are the only inputs. A failure is a message. This part runs no compiler and guesses no path. It runs the `ninja` row of the manifest with the flag that prints the record; that run builds nothing.
- **S — SSOT.** The source file is the truth. One row and one document for one file. The start and a change run the same job. The parse table is in `SyntaxTree` only. The compiler manifest table is in `CompilerManifest` only, and its words are in `cast/c_language.md` only. The build is the one source of the files of a unit: LING holds no search path.
- **S — Stateless.** The state is in the tree. `Processor` holds the documents, which are derived from the rows, and no flag.
- **E — Encapsulation.** Tell, never ask: the watcher and the read of the manifest write the tree, and the listeners react. `LINGModel` and `Processor` know `SyntaxTree`, `CodeDocument`, and `CompilerManifest`; they do not know a language, a build tool, or a file form.
- **D — Deterministic.** The documents are a function of the files that the rows name. The output order is the sorted path order. `Validator` checks the manifest one time, before the read, and no reader checks it again. A manifest that fails gives a `FILES` row with no child, whatever the tree held before.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open.

Facts for the review of the whole PLAN:

- **The files are the files of the last build.** A file that an edit includes after that build has no row until the next build. A file that an edit no longer includes keeps its row until the next build.
- **A record can be older than its output.** ninja marks it `(STALE)` in the head line. LING reads it as it is.
- **Forced includes** (`-include`, here behind `-Xclang`) are in the record, because the compiler read them. In END they name the precompiled header, and its files are in the record of each unit that loads it.
- **A file takes the family of the first unit whose record holds it.** A file with no extension, for example `vector`, thus has a family.
- **`#if` and macros need no evaluation in LING.** The record holds what the compiler read after it evaluated them.
- **The `cmake` row comes before the `ninja` row.** The unit rows exist when the record is read. END and the cli archetype have this order. A `ninja` row that comes first gives no file. A flow with no `ninja` row gives the unit rows only, and no message.
- **The watcher watches the directory of every row**, also the directories below the SDK and the compiler.
- **A record with no unit is not read.** In END these are the 26 `.spv` shader outputs.
- **The run of the `ninja` row blocks the message thread until the child ends.** One run took 0.05 s on END Debug. The read of 12.5 MB of text comes after it, in the same reaction.
- **A compiler manifest that is not on disk gives a message and no row.** A project that is not built for the toolchain argument has no `compile_commands.json` in that build folder.
- **A manifest with no `## toolchain` table gives no row and no message.** The row `0 documents` is the result.
- **A `## toolchain` table with no `command` column or no `flag` column gives the `failToolchainColumn` message and no row** (ruling 79). No table of that manifest is read.
- **The build folder is the token after `-B`.** A `cmake` row that joins the flag and the folder in one token gives no compiler manifest. END and the cli archetype write two tokens.
- **`jam_data_structures`** is one of the thirteen modules of Part 1, so `jam::Model` needs no new module row.
- **Size and time.** The headers behind `JuceHeader.h` hold 316,505 declarations (tracker, Reference data). END Debug gives 5635 files from 142 records. All documents stay in memory. Step 7 item 7 measures the time and the memory. Part 11 keeps the trees on disk (ruling 54); this measurement is its baseline.
