# PLAN: LING Part 11 — The Swap: Trees on Disk

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 14, 15, 22, 42 (a), (d), (e), 45, 50, 51, 54, 55, 58 to 69, 72, 73, 74.
**Date:** 2026-11-11
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

This part keeps the tree of each source file on disk, in one swap file (ruling 54).
A job of `Processor` reads the tree from the swap file. When the source bytes changed, the job parses the file and replaces the swap file: this is the normal flow. Every other mismatch is a failure of a `Document::Validator` rule, shown as a message that says to remove the swap folder and build it again (ruling 69). The job repairs nothing.
The model tree, the watcher, and the listeners of Part 6 do not change: file, watcher, model, callback (ruling 54). Only the body of the job changes.
Each repository root holds the swap files of its own files (ruling 55). LING reads the roots from `project-info.md` (rulings 59, 65).

## What this part does not do

- No index file and no segment file. One swap file holds one tree (ruling 63). The index form waits for a scout measurement.
- No change of a query. Every lookup reads `jam::Document`, as before (ruling 42 (d)).
- No copy of the source text. The source file is the truth (ruling 14); a swap file holds the tree only.
- No change of cast, of a spell file, or of a table (ruling 66).

## The facts that give the design

1. A token span is "Start and end byte offsets of the span, into the owning Document's source" (`jam_Document.h:63`). `Id::name` is a token list. So a tree from the swap needs the source text of its file. The job loads the file text in both cases. The swap replaces the tokenize and build work, not the file read.
2. `Document::parse (const char* data, int size)` is two statements: `addSource (preprocess (juce::String::createStringFromData (data, size)));` and `parse();` (`jam_Document.cpp:621-625`). `addSource` also makes `lineOffsets` (`jam_Document.cpp:261-274`). So a read is the first statement, then the tree from a stream in place of `parse()`.
3. `CodeDocument` is a complete type: it overrides `getVocabulary`, `getToken`, and `build` (`jam_CodeDocument.h:45`, `:62`, `:65`). A tree from the swap is plain data. It needs no hook of a language. So the read makes a `CodeDocument`, and no table from a family to a read function is needed (ruling 22).
4. `CodeDocument::tokens` is read by `build()` only: the walk (PLAN-ling-4-language-document.md, The walk) and `addComment` (PLAN-ling-9-completion.md, Two properties at creation). No query reads it after the build. So the swap does not hold it.
5. The elements of one document hold no link into another document (ruling 40). So an `Element*` value is written as the number of its element in the same file.

## Language / Framework Constraints

- Ruling 50: no `juce::String` and no `juce::StringArray`, except where a JUCE API returns one or takes one. Ruling 51: JAM first.
- MANIFESTO **E**, Permitted copies, item 1: "OS or device boundary. `read` or `write` into the buffer of the owner." The write of a swap file and the map of a swap file are this copy.
- MANIFESTO **E**: "Published data is immutable ... A state update replaces the slot. It does not edit it." A swap file is written whole to a temporary file and then replaces the target.
- MANIFESTO **D**: equal source bytes and an equal version give equal swap bytes. No time, no pointer value, and no hash map order goes into a swap file.
- Ruling 69 and MANIFESTO **D** ("Pessimistic and defensive programming are the same violation from opposite sides"): `Document::write` and `Document::read` hold no assert, no check, and no failure path. The Validator is the one place that judges a tree.
- CODING.md: one object, one translation unit; no out-parameter; no fake-carrier struct; `.at()`; no magic value.
- CAROL.md Profiling: measurement uses scout.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Line numbers are reads of 2026-11-11. Base path for JAM: `~/Documents/Poems/dev/jam/`.

| Need | Existing piece | Where |
|---|---|---|
| A value written to a stream and read from a stream | `bool write (juce::OutputStream& output) const;` and `static AttributedString read (juce::InputStream& input);`; each array is "an int element count followed by its raw elements"; each text is "an int byte count followed by its UTF-8 bytes" | `jam_core/text/jam_AttributedString.h:330-352`; `jam_AttributedString.cpp:371-482` |
| Swap bytes read into a `jam::Document` | `document.addChild (*document.getRoot(), Id::line)`, then `line->add<AttributedString> (Id::cells, AttributedString::read (input))` | `jam_terminal/document/jam_TerminalHistory.cpp:3-31` |
| Map a file read-only and read it as a stream | `juce::MemoryMappedFile { file, juce::MemoryMappedFile::readOnly }`, then `juce::MemoryInputStream` over `getData()`; "If the file can't be opened for some reason, the getData() method will return a null pointer." | `jam_TerminalHistory.cpp:283-296`; JUCE `juce_core/files/juce_MemoryMappedFile.h:72`, `:101` |
| Add an element and a property outside a parse | `Element* addChild (Element& parent, const juce::Identifier& id);`; `Element::add<ValueType> (key, args...)`; `Token::add<ValueType> (key, args...)` | `jam_core/document/jam_Document.h:1027`, `:452-458`, `:175-181` |
| The value types of an element | `std::monostate`, `bool`, `int`, `double`, `juce::uint32`, `juce::Identifier`, `juce::String`, `Tokens`, `juce::int64`, `juce::var`, `Element*`, `Elements`, `Identifiers`, `Numbers`, `AttributedString` | `jam_Document.h:192-206` |
| The value types of a token | `bool`, `double`, `juce::String`, `Token`, `jam::Array<Property<Token>>`, `int`, `jam::Union<uint16_t, uint16_t>` | `jam_Document.h:94-100` |
| One arm for each alternative of a variant | "Every Element::Value alternative is matched with an explicit constexpr arm, so a new alternative added to the variant fails to compile here until its rebase behavior is decided." | `jam_Document.h:1145-1173` (`setSpanOffsets`) |
| A walk of the tree | `Element::applyFunctionRecursively`; the links `firstChild`, `nextSibling` | `jam_Document.h:419-435`, `:229-233` |
| Source and line offsets with no parse | `size_t addSource (std::string_view text);` | `jam_Document.h:945`; `jam_Document.cpp:261-274` |
| The protected parse that a derived static calls | `void parse (const char* data, int size);`; `CLanguageDocument::parse (std::string_view)` calls it | `jam_Document.h:1193`; PLAN-ling-4-language-document.md Step 3 item 3 |
| Stream integers in a fixed byte order | `writeInt`: "Writes a 32-bit integer to the stream in a little-endian byte order."; `writeInt64`, `writeBool`, `writeDouble`; `readInt`, `readInt64`, `readBool`, `readDouble` | JUCE `juce_core/streams/juce_OutputStream.h:109-168`; `juce_InputStream.h:105-184` |
| A `juce::var` to a stream | `void writeToStream (OutputStream& output) const;`, `static var readFromStream (InputStream& input);` | JUCE `juce_core/containers/juce_Variant.h:316`, `:323` |
| Publish a complete file | `explicit TemporaryFile (const File& targetFile);`, then `overwriteTargetFileWithTemporary()`: "If the file move succeeds, this returns true, and the temporary file will have disappeared." | JUCE `juce_core/files/juce_TemporaryFile.h:124`, `:185` |
| Make the parent folder before a write | `file.getParentDirectory().createDirectory();` | `jam_markdown/document/jam_ConfigDocument.cpp:747` |
| A hash of bytes | `jam::Wyhash::hashBytes (const void*, std::size_t) noexcept` | `jam_core/utils/jam_HashMap.h:221` |
| The hexadecimal text of a hash | `jam::Format::appendHex`; `LINGModel::getDaemonName` uses it for a path | PLAN-ling-8-daemon.md Step 1 item 8; In LING: `Server` and `Connection` |
| The version of the program | `ProjectInfo::versionString`, from the `versionString` row | `scout/Source/generated/ProjectInfo.h:37`; PLAN-ling-1-project.md Step 2 |
| The folder of LING on the machine | `LINGModel::getCacheDirectory()`; the file rows `lingCache`, `lingLocalCache` | PLAN-ling-8-daemon.md, In LING: `Server` and `Connection`; Step 2 item 2 |
| A file name from a stem and an extension | `jam::Format::toFileName (juce::StringRef name, juce::StringRef extension)`: the name, a dot, and the extension | `jam_core/text/jam_Format.cpp:141-146` |
| An alias cell | a cell that starts with `Chars::at` names a row of `## index`; its value is the `symbol` cell of that row. `jucePath` is a path in END and `@juce-path` in scout. | `cast/Source/Model.cpp:66-84`; `end/project-info.md:64`; `scout/project-info.md:58` |
| A tool index at a repository root | `end/.cache/clangd/index`; `.cache/` is in `end/.gitignore` and in `jam/.gitignore:49` | read of 2026-11-11 |
| Read the tables of `project-info.md` | `jam::ConfigDocument::parse`; `getTables`, `getTableRows`, `getTableValue` | `scout/Source/Processor.cpp:13`, `:36-55`; PLAN-ling-6-manifest.md, item 2 of `LINGModel` |
| The user module root | `## index` row `@user-module`, symbol `${CMAKE_CURRENT_SOURCE_DIR}/../jam` | `end/project-info.md:10` |
| The JUCE root and version | `## cmake` rows `jucePath` (`${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE`) and `juceVersion` (`8.0.14`) | `end/project-info.md:64`, `:70` |
| The modules file of a root | "Each root carries a `user-modules-info.md` file at its top level"; cast's own row `userModulesInfo` | `cast/Source/HELP.md:85`; `cast/Source/generated/Generated.cpp:119` |
| The patch list | `## submodule`, columns `module`, `path`, `class`, `aggregator`; the 14 rows whose `module` cell is `patch`; the list rule `@modules:submodule:module=patch` | `jam/user-modules-info.md:181-185`, `:1344-1357`; `end/cast/spell.md:233` |
| The path of a patch file | `"${CAST_USER_MODULE_PATH}/:::module:::/:::path:::"` | `jam/cast/juce.cast:95-97` |
| The rule of the patched folder | the cache root is `$ENV{LOCALAPPDATA}/cast/juce` on Windows and `$ENV{HOME}/.cache/cast/juce` elsewhere; the text is `CAST_JUCE_VERSION`, then the content of each patch file in list order; the folder is the cache root plus the SHA256 of that text | `jam/cast/juce.cast:8-25` |
| SHA256 | `SHA256 (const void* data, size_t numBytes)`; `toHexString()`: "Returns the checksum as a 64-digit hex string."; module `juce_cryptography`, dependency `juce_core` | JUCE `juce_cryptography/hashing/juce_SHA256.h:74`, `:102`; `juce_cryptography.h` |
| `%LOCALAPPDATA%` | `juce::File::windowsLocalAppData` | JUCE `juce_File.h:993-994` |
| A file inside a folder; a path relative to a folder | `bool isAChildOf (const File& potentialParentDirectory) const;`; `String getRelativePathFrom (const File& directoryToBeRelativeTo) const;` | JUCE `juce_core/files/juce_File.h:304`, `:199` |
| The version of a document is root metadata | `getVersion`: "Returns the version value of the root metadata block." | `jam_markdown/document/jam_MarkdownDocument.h:482` |
| A rule set for a document | `Document::Validator`: "isValid() runs every rule deterministically and aggregates every failure"; `using Rules = jam::Function::Map<juce::String, jam::Function::pointerCapacity, juce::Result (const Document&)>`; `getFailures (const jam::Strings&)` | `jam_core/document/jam_Document.h:831-871` |
| A validator for a document that is read from a swap | `AnsiValidator`: "ANSI/VT structural validation, keyed into Document::Validator's rule set."; siblings `XmlValidator`, `CssValidator` | `jam_terminal/document/jam_AnsiValidator.h:22`; `jam_core/xml/jam_XmlValidator.h:21`; `jam_web/css/jam_CssValidator.h:21` |
| A Validator failure is a message, and the document is kept | "A document that `CLanguageValidator` rejects is a `setMessage` with the failure text, and it is kept" | PLAN-ling-6-manifest.md, In LING: the documents |
| The job of a `FILE` row | "A job loads the text of the file with `juce::File::loadFileAsString()` ... and calls `SyntaxTree::parse (text, family)` on the pool thread." | PLAN-ling-6-manifest.md, In LING: the documents |
| The state tree and its authors | `LINGModel`; the `FILES` and `OVERLAY` rows | PLAN-ling-6-manifest.md, In LING: the state tree |

### Names

| Name | Source |
|---|---|
| "swap" | ARCHITECT: "index is in swap file" (ruling 54); `jam::TerminalSwap` |
| `Document::write`, `Document::read`, `CodeDocument::read` | `AttributedString::write`, `AttributedString::read` (`jam_AttributedString.h:341`, `:352`); the protected `Document::parse` and the static `parse` of a derived type |
| `Id::roots`, the `ROOT` rows | the plural row and the singular row, as `Id::files` and `Id::file` (PLAN-ling-6-manifest.md); `Id::root` exists in `jam/cast/core.md:132` |
| `Id::swap` | ARCHITECT's word |
| `Id::version` on the root | exists in `jam/cast/core.md:156`; "the version value of the root metadata block" (`jam_MarkdownDocument.h:482`) |
| `Id::hash` on the root | `jam::Wyhash::hashBytes`; ruling 62, "Hash of the bytes" |
| `SwapValidator` | `AnsiValidator`, `XmlValidator`, `CssValidator`: the domain word, then `Validator` |
| `failSwapVersion` | `failUnitNotFound`, `failCompilerManifestInvalid` (PLAN-ling-6-manifest.md Step 4 item 4) |
| `LINGModel::getSwapFile` | the verb `get`; `Server::getPortFile` (PLAN-ling-8-daemon.md, Names) |
| `LINGModel::getPatchedDirectory` | the verb `get`; "patched" from `CAST_PATCHED_JUCE_PATH` (`juce.cast:25`); `LINGModel::getCacheDirectory` (PLAN-ling-8-daemon.md, Names) |
| `lingSwap` | `lingPort` (PLAN-ling-8-daemon.md Step 2 item 3): the extension of a file of LING, joined with `jam::Format::toFileName` |
| `userModulesInfo` | cast's row of the same file name (`cast/Source/generated/Generated.cpp:119`) |
| `castJuceCache`, `castJuceLocalCache` | `lingCache`, `lingLocalCache` (PLAN-ling-8-daemon.md Step 2 item 2); `CAST_JUCE_CACHE_DIR` (`juce.cast:9-11`) |
| `sourceDirectoryVariable` | the CMake name `CMAKE_CURRENT_SOURCE_DIR` (`end/project-info.md:10`) |
| `Id::cmake`, `Id::jucePath`, `Id::juceVersion`, `Id::submodule`, `Id::module`, `Id::patch`, `Id::symbol`, `Id::userModule` | the table names, column names, and cell words that LING reads, verbatim (`end/project-info.md:3-12`, `:47`, `:64`, `:70`; `jam/user-modules-info.md:181-185`, `:1344`) |

## The design

### In JAM: a tree to a stream and back

Two members of `jam::Document` (`jam_core/document/jam_Document.h`), and one static member of `jam::CodeDocument`. None of the three holds an assert, a check, or a failure path (ruling 69).

**`void write (juce::OutputStream& output) const;`** is public, next to `getSource` (`jam_Document.h:932`). It writes the tree, and nothing of the source. Every integer is written with `writeInt` or `writeInt64`, so the byte order is little-endian on every platform. A text is an int byte count and then its UTF-8 bytes, the form of `AttributedString::write`. The order:

1. The count of elements, as an int.
2. For each element, in the order of a walk from the root (an element, then its children from `firstChild` through `nextSibling`): the text of `id`, and the number of its parent. The number of an element is its position in this walk. The root is number 0, and its parent number is -1.
3. For each element, in the same order: the count of its properties, then each property: the text of `name`, the index of the alternative that the value holds (`std::variant::index`), and the value.

A value is written by its alternative:

| Alternative | Written as |
|---|---|
| `std::monostate` | nothing |
| `bool` | `writeBool` |
| `int`, `juce::uint32` | `writeInt` |
| `double` | `writeDouble` |
| `juce::int64` | `writeInt64` |
| `juce::Identifier`, `juce::String` | a text |
| `Tokens` | a count, then each token |
| `juce::var` | `juce::var::writeToStream` |
| `Element*` | the number of the element; -1 for `nullptr` |
| `Elements` | a count, then the number of each element |
| `Identifiers` | a count, then each text |
| `Numbers` | a count, then each `writeDouble` |
| `AttributedString` | `AttributedString::write` |

A token is written as: the start and the end of `span`, each with `writeInt`; `type`; the count of its properties; then each property as the text of `name`, the index of its alternative, and its value. A token value is written by its alternative in the same way: `bool`, `double`, `juce::String`, `int` as above; a `Token` as a token; a `jam::Array<Property<Token>>` as a count, then the text of each `name` and each token; a `jam::Union<uint16_t, uint16_t>` as its two parts, each with `writeInt`.

`write` matches every alternative of `Document::Value` and of `Token::Value` with an explicit constexpr arm, in the form of `setSpanOffsets` (`jam_Document.h:1145-1173`). A new alternative then fails to compile in `write` and in `read` until its form is decided.

The element numbers come from one transient map from `const Element*` to `int`, which lives inside `write` (MANIFESTO **S**, Stateless: "Transient state only"). `write` returns nothing: no caller reads a result (MANIFESTO **L**, YAGNI).

**`void read (const char* data, int size, juce::InputStream& input);`** is protected, next to `parse (const char* data, int size)` (`jam_Document.h:1193`). It is the creation of a document from a stream:

1. `addSource (preprocess (juce::String::createStringFromData (data, size)));`, the first statement of `parse (const char*, int)` (`jam_Document.cpp:623`). The source and `lineOffsets` are then the same as after a parse.
2. Read the count of elements. Element 0 is `getRoot()`. For each other element, read the text of `id` and the parent number, and call `addChild (*parent, id)`. A transient array holds each `Element*` by its number. A parent comes before its children in the walk, so the parent pointer is already in the array.
3. For each element, read its properties and add each one with `Element::add<ValueType>`. An `Element*` value and each member of an `Elements` value come from the array by `.at (number)`.

`read` is creation: "Creation can use multiple passes ... work buffers, and memory allocation" (MANIFESTO **E**, Complete at Creation). The document is complete when `read` returns.

**`static CodeDocument read (std::string_view sourceText, juce::InputStream& input, int family);`** is a public member of `jam::CodeDocument`. It makes a document, sets `family`, calls `Document::read (sourceText.data(), static_cast<int> (sourceText.size()), input)`, and returns the document. It is the sibling of `CLanguageDocument::parse (std::string_view)` (PLAN-ling-4-language-document.md Step 3 item 3). The document holds the tree that a language document made. It runs no hook of a language.

### In LING: the roots

`LINGModel` gets one more child row, made in its constructor:

```
LING (Id::ling)
  FILES (Id::files)
  ROOTS (Id::roots)
    ROOT (Id::root)             Id::path, Id::swap
  OVERLAY (Id::overlay)
```

| Row or property | Author | Meaning |
|---|---|---|
| a `ROOT` row | `LINGModel`, when it reads the manifest (`Id::manifest`) | one folder whose files have their swap files in one place. `Id::path` is the full path of the folder. `Id::swap` is the full path of the folder that holds the swap files. |

The reaction of `LINGModel` to `Id::manifest` on the root (PLAN-ling-8-daemon.md, The `## daemon` table) also makes the `ROOT` rows. It builds a new `ROOTS` row off the tree, adds the rows below to it in this order, and then replaces the `ROOTS` row of the tree with it, as it replaces the `FILES` row (PLAN-ling-6-manifest.md, item 2 of `LINGModel`): "A complete state replaces a complete state" (MANIFESTO **E**). `<project>` is the parent folder of the `project-info.md` file. A cell that starts with `Chars::at` is an alias: its value is the `symbol` cell of the `## index` row with that alias, the rule of cast (`cast/Source/Model.cpp:66-84`). In each cell that it reads as a path, the text `files::sourceDirectoryVariable` (`${CMAKE_CURRENT_SOURCE_DIR}`) is replaced by the full path of `<project>`, and the result goes through `juce::File::getChildFile` of `<project>`, which takes away each `..` segment. When the check of the manifest fails (PLAN-ling-6-manifest.md, In LING: the manifest check; ruling 79), the reaction reads no table, so the new `ROOTS` row holds the project row only: that row needs no table.

| Row | `Id::path` | `Id::swap` | Added when |
|---|---|---|---|
| the project | `<project>` | `<project>/<files::lingCache>` | always |
| the user modules | the `symbol` cell of the `## index` row whose `alias` cell is `@user-module` | that folder plus `files::lingCache` | the row exists |
| the patched JUCE | `getPatchedDirectory` | the `jucePath` cell of `## cmake`, plus `files::lingCache`, plus the hash text | `getPatchedDirectory` is an existing folder |

`files::lingCache` is `.cache/ling` (PLAN-ling-8-daemon.md Step 2 item 2), the form of `end/.cache/clangd/index` (ruling 63). Below a repository root it is the same text on each platform.

**`static juce::File getPatchedDirectory (std::string_view version, const jam::Strings& patchFiles);`** applies the rule of `jam/cast/juce.cast:8-25` (rulings 60, 61). It builds one `std::string`: the bytes of `version`, then the bytes of each patch file in list order. Each patch file is read with `loadFileAsString()`, and its bytes come from `jam::Strings::getView`, as the job reads a source file (PLAN-ling-6-manifest.md, In LING: the documents). It returns the cast cache root plus `juce::SHA256 (data, numBytes).toHexString()`. The cast cache root is the home folder plus `files::castJuceCache` (`.cache/cast/juce`), and inside `#if JUCE_WINDOWS` it is `juce::File::windowsLocalAppData` plus `files::castJuceLocalCache` (`cast/juce`), as `LINGModel::getCacheDirectory()` has it (PLAN-ling-8-daemon.md, In LING: `Server` and `Connection`). `toHexString` gives a `juce::String`; it goes to `getChildFile`, a JUCE API (ruling 50).

The arguments come from the tables (ruling 66):
- `version` is the `juceVersion` cell of `## cmake`.
- `patchFiles` holds one full path for each row of `## submodule` whose `module` cell is `patch`, in table order, from the file `files::userModulesInfo` (`user-modules-info.md`) in the user module folder. Each path is the user module folder, the `module` cell, and the `path` cell (`juce.cast:95-97`).

A `project-info.md` with no `@user-module` row, with no `juceVersion` row, or with no patch row gives no patched row. A project with no cast then needs none of these rows (ruling 65).

**`juce::File getSwapFile (const juce::File& file) const;`** is a member of `LINGModel`. It returns the swap file of one source file:

- It takes the `ROOT` row with the longest `Id::path` for which `file.isAChildOf (path)` is true. The swap file is in the `Id::swap` folder. Its path in that folder is `jam::Format::toFileName` of the path of the file relative to `Id::path` (`getRelativePathFrom`) and `files::lingSwap`: the relative path, a dot, and `swap`.
- A file inside no `ROOT` row uses the folder of LING on the machine: `LINGModel::getCacheDirectory()` (`~/.cache/ling`, ruling 58; PLAN-ling-8-daemon.md, In LING: `Server` and `Connection`). Its file name is `jam::Format::toFileName` of the `jam::Format::appendHex` text of `jam::Wyhash::hashBytes` over the UTF-8 bytes of the full path, and `files::lingSwap`. This is the form of `LINGModel::getDaemonName`. A file below no root has an absolute path, and on Windows that path holds a drive letter; the hash text is a legal file name on each platform.

### In LING: the swap file

A swap file is `document.write (output)` and nothing more. The stamp is metadata of the tree, on the root element, in the way a document holds its version: "the version value of the root metadata block" (`jam_MarkdownDocument.h:482`).

| Root property | Value | Author |
|---|---|---|
| `Id::version` | `juce::String`: the text of `ProjectInfo::versionString` (rulings 67, 68). `juce::String` is the text alternative of `Document::Value` (`jam_Document.h:198`), a JAM API that takes one. | the job, after `SyntaxTree::parse` |
| `Id::hash` | `juce::int64`: `static_cast<juce::int64>` of `jam::Wyhash::hashBytes` over the bytes of the loaded text (`jam::Strings::getView`), before `preprocess` (ruling 62). `Document::Value` holds `juce::int64`, and no unsigned 64-bit type (`jam_Document.h:192-206`). | the job, after `SyntaxTree::parse` |

The job adds the two properties while it still makes the document: "creation and state-update code holds the document mutably" (`jam_Document.h:220-222`). A document from `read` already holds them, because `write` wrote them.

**`struct SwapValidator : jam::Document::Validator`** is in LING. It has the form of `XmlValidator` (`jam_core/xml/jam_XmlValidator.h`). `getRules()` returns a static `Rules` table with one rule, key `Id::version`: the text of the root's `Id::version` compares equal to `ProjectInfo::versionString` (`juce::String::compare (...) == 0`). A failure is the text row `failSwapVersion`, `swap of another version: remove the swap folder and build again`, through `Validator::getFailures`.

### In LING: the job

The reactions of `Processor` to the `FILES` row do not change (PLAN-ling-6-manifest.md, In LING: the documents, the table of events). At the start of a job, on the message thread, `Processor` takes `LINGModel::getSwapFile` of the row's file and gives it to the job by value. The pool thread does not read the model tree.

The job, on the pool thread:

1. Load the text of the source file, as today. A file that does not exist gives no document, as today; the job also deletes the swap file with `deleteFile`.
2. Take the hash of the text, as the row `Id::hash` of section The swap file gives it.
3. Map the swap file with `juce::MemoryMappedFile { swapFile, juce::MemoryMappedFile::readOnly }`. When `getData()` is not `nullptr`, the document is `std::make_unique<jam::CodeDocument> (jam::CodeDocument::read (text, input, family))`, with `input` a `juce::MemoryInputStream` over the mapped bytes. The mapping ends before the job gives the document to the message thread.
4. When the document from the swap holds a root `Id::hash` that is equal to the hash of item 2: the job keeps that document. `SwapValidator().isValid (*document)` runs. A failure goes to the message thread as a `setMessage` with the failure text and the path of the swap file, and the document is kept: the form of every Validator failure (PLAN-ling-6-manifest.md, In LING: the documents). The job does not parse the file and does not write the swap file. The message tells the user to remove the swap folder; the next run then finds no swap file and builds it.
5. When no swap file exists, or the root `Id::hash` is not equal: the source changed, or the tree was never written. The document is `SyntaxTree::parse (text, family)`, as today. The job adds the root properties `Id::version` and `Id::hash`, then publishes the swap file: `swapFile.getParentDirectory().createDirectory()`; `juce::TemporaryFile temporary { swapFile }`; `document->write (output)` into a `juce::FileOutputStream` of `temporary.getFile()`; the stream ends; `temporary.overwriteTargetFileWithTemporary()`.
6. Give the document to the message thread, as today. `CLanguageValidator` runs on it as Part 6 gives, for a document from `read` and for a document from `parse`.

A file whose family has no `documentConstruction` entry gets no document and no swap file, as today (PLAN-ling-6-manifest.md, In LING: the documents).

Two daemons can share one root (ruling 55). Each publishes a whole file. Equal source bytes and an equal version give equal swap bytes, so the file is the same from either daemon.

### The manifest of a run

`project-info.md` is the manifest of LING for a project (ruling 65), as it is (ruling 72). It is the one file argument of `ling` and of `ling daemon` (ruling 73). The one table that LING declares for it is the optional `## daemon` table (ruling 71). The roots of this part come from tables that the manifest of a cast project already holds: `## index` and `## cmake`. A run on END takes `end/project-info.md`.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: `Document::write`, `Document::read`, `CodeDocument::read`
**Scope:** `jam/jam_core/document/jam_Document.h` and `.cpp`; `jam/jam_syntax_tree/document/jam_CodeDocument.h` and `.cpp`.
**Action:**
1. Declare `void write (juce::OutputStream& output) const;` after `getSource` (`jam_Document.h:932`), and `void read (const char* data, int size, juce::InputStream& input);` after the protected `parse (const char* data, int size)` (`:1193`).
2. Write the two bodies in `jam_Document.cpp` as section In JAM gives them. One file-static function writes a text, and one reads a text, in the form of `writeText` of `jam_AttributedString.cpp:371-405`. One function writes a token and one reads a token; each calls itself for a `Token` value.
3. Declare `static CodeDocument read (std::string_view sourceText, juce::InputStream& input, int family);` after `parse` in `jam_CodeDocument.h`, and write its body.

**Validation:** Each alternative of the two variants has one explicit arm in `write` and one in `read`. No `jassert`, no check of a count, and no failure path is in `write`, in `read`, or in `CodeDocument::read` (ruling 69). `write` is `const` and changes nothing. `read` uses `addSource`, `addChild`, and `add` only: the creation API (`jam_Document.h:218-223`). No source byte goes to the stream. No pointer value goes to the stream. No file holds an include (CODING.md).

### Step 2: The rows
**Scope:** `ling/cast/identifiers.md`; `ling/cast/files.md`; `ling/cast/text.md`; `ling/project-info.md`.
**Action:**
1. `cast/identifiers.md`: add `cmake`, `hash`, `jucePath`, `juceVersion`, `module`, `patch`, `roots`, `submodule`, `swap`, `symbol`, in name order, and `userModule` with the value `` `@user-module` ``. `Id::root`, `Id::path`, `Id::index`, `Id::alias`, `Id::version` exist in `jam/cast/core.md` (`:132`, `:125`, `:102`, `:41`, `:156`); a second row would define them two times.
2. `cast/files.md`: add `@string` rows: `lingSwap` `` `swap` ``; `userModulesInfo` `` `user-modules-info.md` ``; `castJuceCache` `` `.cache/cast/juce` ``; `castJuceLocalCache` `` `cast/juce` ``; `sourceDirectoryVariable` `` `${CMAKE_CURRENT_SOURCE_DIR}` ``. The row `lingCache` exists (PLAN-ling-8-daemon.md Step 2 item 2).
3. `cast/text.md`: add `failSwapVersion` `swap of another version: remove the swap folder and build again`.
4. `project-info.md` `## juce module`: add the row `cryptography`, `juce_cryptography`, after the six rows of Part 1 (PLAN-ling-1-project.md Step 3 item 1).
5. Run cast (tracker, The contract of every part, cast runs).

**Validation:** Each new word is a row, and each row has one reader. No literal of a path, of a table name, or of an extension is in a `.cpp` file (MANIFESTO **E**, no magic values).

### Step 3: The roots
**Scope:** `ling/Source/LINGModel.h` and `.cpp`.
**Action:**
1. Make the `ROOTS` row in the constructor of `LINGModel`.
2. Write `getPatchedDirectory`, and add the `ROOT` rows in the reaction to `Id::manifest` (PLAN-ling-8-daemon.md Step 3 item 1), as section The roots gives them.
3. Write `getSwapFile`.

**Validation:** `LINGModel` is the one author of a `ROOT` row. No function names cast, JUCE, or a patch outside the rows of Step 2: the rule is data (ruling 22). `getSwapFile` is `const` and returns its value. A `project-info.md` with none of the optional rows gives one `ROOT` row, the project.

### Step 4: The job
**Scope:** new files `ling/Source/SwapValidator.h` and `.cpp`; `ling/Source/Processor.h` and `.cpp`.
**Action:**
1. Write `SwapValidator` as section The swap file gives it.
2. At the start of a job, take `getSwapFile` on the message thread and give it to the job.
3. Write the six items of section The job.

**Validation:** The reactions to the `FILES` row are unchanged: file, watcher, model, callback (ruling 54). The pool thread reads no `juce::ValueTree`. A document from `read` and a document from `parse` go to the message thread by the same path. The mapping lives inside one job (MANIFESTO **B**). The swap file is replaced whole; no process edits a published file. The job holds no repair path: a changed source replaces the swap file, and every other mismatch is a Validator message (ruling 69).

### Step 5: The manifest of the check runs
**Scope:** `end/project-info.md`, read only.
**Action:**
1. Read `end/project-info.md`. Check that it holds the rows that this part reads: the `@user-module` row of `## index` (`:10`), and the `jucePath` and `juceVersion` rows of `## cmake` (`:64`, `:70`). No part of LING adds a table to it (ruling 72). This step changes no file.
2. Each check run on END takes `~/Documents/Poems/dev/end/project-info.md` as its manifest, with the toolchain argument `--debug`.

**Validation:** One file is the manifest of cast and of LING for END, as it is (rulings 65 and 72).

### Step 6: Build and check, macOS
**Scope:** builds of JAM consumers and LING; runs of `ling`; one scout manifest.
**Action:**
1. ARCHITECT builds LING, END, cast, scout, and ggwp. `jam_core` changed.
2. Remove `end/.cache/ling`, `jam/.cache/ling`, `~/Documents/Poems/JUCE/.cache/ling`, and `~/.cache/ling`. Run `ling ~/Documents/Poems/dev/end/project-info.md --debug`. Keep the output as run A.
3. Check the four places. `end/.cache/ling` holds one `.swap` file for each END file of the output, at the same relative path. `jam/.cache/ling` holds the JAM files. `~/Documents/Poems/JUCE/.cache/ling/<hash>/` holds the JUCE files, and `<hash>` is equal to the folder name in the `-I` flags of the compiler manifest, `end/Builds/Debug/compile_commands.json`. `~/.cache/ling` holds the files of no root.
4. Copy the four folders. Run item 2 again without the removal. Keep the output as run B. Run A and run B are byte-equal. No swap file has a new modification time.
5. Remove the four folders again and run item 2. Compare each swap file with the copy of item 4, byte for byte.
6. Change one byte of one END source file and run again. Exactly one swap file has a new modification time. Put the byte back.
7. Change the `versionString` row of `ling/project-info.md`, build LING, and run again. The run prints one `failSwapVersion` message for each swap file, and no swap file has a new modification time. Remove the four folders and run again: no message, and every swap file is written. Put the row back, build, and remove the four folders.
8. Start the daemon (PLAN-ling-8-daemon.md Step 7 item 2, with `end/project-info.md`). Save one source file with a change. The swap file of that file has a new modification time, and no other one has.
9. Measure with scout, with the epochs and the baseline that the profile skill gives: benchmark 1 runs item 2 after the removal of the four folders; benchmark 2 runs item 2 with the swap files in place. Record the two medians and the size of the four folders in the sprint notes.

**Validation:** Items 3 to 8 hold. Item 3 proves the hash rule of ruling 61 against the compiler manifest. Item 5 proves MANIFESTO **D**. ARCHITECT reads the scout report of item 9: it is the measurement that ruling 63 names for the index form.

### Step 7: Build and check, Windows
**Scope:** a Windows machine.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:**
1. ARCHITECT builds LING. Run items 2 to 8 of Step 6 on the Windows machine, with the Windows cache roots (`%LOCALAPPDATA%\cast\juce`, `%LOCALAPPDATA%\ling`).
2. Run two daemons on two projects that share the JAM root. Both end their first load with the same swap files in `jam/.cache/ling`.

**Validation:** The same checks hold on Windows.

## BLESSED Alignment

- **B — Bound.** A job owns its mapping and its temporary file for one call. `LINGModel` owns the `ROOT` rows. Each root folder holds the swap files of its own files.
- **L — Lean.** One swap file for one tree. No index, no segment, no lock between processes, no table from a family to a read function.
- **E — Explicit.** The roots are rows of `project-info.md`. Each path, extension, and table name is a generated row. A swap of another version is a message, never a silent repair.
- **D, no defence.** `write` and `read` check nothing. `SwapValidator` and `CLanguageValidator` judge the tree (ruling 69).
- **S — SSOT.** The source file is the truth (ruling 14). The patch list stays in `jam/user-modules-info.md` (ruling 66). The version is the `versionString` row (ruling 68). The swap is a write at an OS boundary, permitted copy 1; it is not a second truth (ruling 54).
- **S — Stateless.** `write` is `const`. The swap file path is a function of the rows and the file path. The job holds transient state only.
- **E — Encapsulation.** The data flows one way: file, watcher, model, callback (ruling 54). A document is complete when `read` or `parse` returns. A published swap file is replaced, never edited.
- **D — Deterministic.** Equal source bytes and an equal version give equal swap bytes. Step 6 item 5 proves it.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open.

Facts for the review of the whole PLAN:

- **The hash rule exists in two places** (ruling 61): `jam/cast/juce.cast:14-25` and `LINGModel::getPatchedDirectory`. A change of the rule in cast needs the same change in LING. Step 6 item 3 shows a difference at once: the hash folder is then not equal to the `-I` folder, and no patched row is added.
- **Two builds with one `versionString` read each other's swap files** (ruling 68). A change of the parser with no change of the row leaves old trees in use. The swap folders are then removed by hand, or the row is changed.
- **`read` runs before the version rule.** The stamp is root metadata, so the tree is read first and judged after (ruling 69: the Document checks nothing, the Validator judges). A swap file whose byte form differs from the form of this build is read as this form. The `failSwapVersion` message then follows only when the read gives a root `Id::hash` equal to the source hash; in the other case the job parses and replaces the file.
- **A document from a swap of another version is kept and used** until the user removes the swap folder, as a document that `CLanguageValidator` rejects is kept (PLAN-ling-6-manifest.md, In LING: the documents).
- **A publish that fails gives no message.** `overwriteTargetFileWithTemporary()` returns `false` for a folder that cannot be written. The document is complete in memory, and the next start parses the file again.
- **`Poems/JUCE/.gitignore` has no `.cache` line** (70 lines, read of 2026-11-11). The JUCE repository then lists `.cache/` as untracked (ruling 64). `end/.gitignore` and `jam/.gitignore:49` ignore `.cache/`.
- **A swap file outlives a source file that is removed while no daemon runs.** The job deletes a swap file only when it runs for a row whose file does not exist.
- **The speed is not measured.** No LING code exists. Step 6 item 9 is the first measurement. The job opens the source file and the swap file for each document.
- **A file that is in two roots** takes the root with the longest path. On this machine the three roots do not hold each other.
- **The `juce::var` form is JUCE's own** (`juce_Variant.h:316`). No part of the plan writes a `juce::var` on an element today; the arm exists because the variant holds the type.
- **`AttributedString::read` reads on from a wrong position after a count that fails its check** (`jam_AttributedString.cpp:415-429`). No part of the plan writes an `AttributedString` on an element of a language document today.
- **No LING table is in a `project-info.md`** (ruling 72), except the optional `## daemon` override that a project can hold (ruling 71). This part reads only rows that cast already reads.
