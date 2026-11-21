---
show-metadata: false
column-widths:
- Command: 0.4
- Method: 0.27
- Operation: 0.23
- Arguments: 0.16
- Word: 0.24
- File: 0.27
- Entry: 0.26
- Kind: 0.16
- Callback: 0.21
- Called by: 0.13
---

# LING: Language Index Navigation Graph

**Version 0.0.1**

This guide describes ling. ling prints this guide when you run `ling` with no argument, or with `ling --help`.

## Server

`ling <manifest> [--<word>]` starts the server. The server speaks LSP over stdio.

The server connects to the daemon of the manifest and the word. When no daemon runs, the server starts one.

When the manifest is a bare file name that the working folder does not hold, the server uses the nearest parent folder that holds it.

When no folder holds it, the server uses the working folder as the project. The daemon of that folder asks each language for its units with no toolchain rows.

A daemon with zero units writes `-1` to its port file and ends at once. Then the server answers each operation with a `null` result.

The daemon reads the manifest. It parses each file of each unit and keeps the index.

The daemon answers the requests of each connected server. A request waits until the files that it needs are indexed.

The daemon writes its port file when it starts. The port file holds the local port of the daemon.

The port file path is `<cache>/<project name>/<daemon name>.port`. `<cache>` is `~/.cache/ling`. On Windows, it is `%LOCALAPPDATA%\ling`.

The project name is a hash of the manifest path and the word. The daemon name is the project name and a hash of the ling build.

Each daemon name runs one daemon. A second `ling daemon` with the same name exits at once.

The daemon ends when no client connects for the timeout. Then it deletes its port file.

The server uses UTF-8 positions when the client offers them. Otherwise, it uses UTF-16 positions.

Before `initialize`, each request gets error `-32002`. After `shutdown`, each request gets error `-32600`.

## Command Line

| Command                                                     | Result                                                                                                                                                                 | Exit                       |
| :---------------------------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :------------------------- |
| `ling`                                                      | Prints this guide.                                                                                                                                                     | 0                          |
| `ling --help`                                               | Prints this guide.                                                                                                                                                     | 0                          |
| `ling --version`                                            | Prints `ling <version> (<commit>)`.                                                                                                                                    | 0                          |
| `ling --build <folder>`                                     | Builds the language library of the folder. See Build.                                                                                                                  | The exit code of `cast`    |
| `ling --mcp <manifest> [--<word>]`                          | Serves the operations of the daemon as Model Context Protocol tools over stdio (revision 2026-07-28). Ends when stdin closes.                                          | 0                          |
| `ling <manifest> [--<word>]`                                | Starts the server on stdio. See Server.                                                                                                                                | 0 after `shutdown` and `exit`, otherwise 1 |
| `ling <manifest>` with no folder that holds it              | Starts the server on stdio for the working folder. See Server.                                                                                                         | 0 after `shutdown` and `exit`, otherwise 1 |
| `ling daemon <manifest> [--<word>]`                         | Starts the daemon. Exits at once when a daemon with the same name runs.                                                                                                | 0                          |
| `ling <operation> <manifest> [--<word>] <arguments>`        | Sends one request to the daemon, and starts the daemon when none answers. Prints the JSON-RPC response on stdout. See Operation Words.                                 | 0, or 1 when the response holds an error |
| `ling <path>/<missing>`                                     | Prints `ling: <path>/<missing>: not found` on stderr.                                                                                                                  | 1                          |
| `ling <manifest> <other>`                                   | Prints `ling: <other>: unknown flag` on stderr.                                                                                                                        | 1                          |

On a terminal, `ling` clears the screen first.

Each failure prints `ling: <first argument>: <message>` on stderr.

| Message                                             | Cause                                                                                   |
| :-------------------------------------------------- | :-------------------------------------------------------------------------------------- |
| `daemon did not start`                              | The server or the operation cannot start the daemon.                                    |
| `daemon connection lost`                            | The connection to the daemon ends before the answer.                                    |
| `<word>: toolchain argument not declared`           | No row of the `## toolchain` table holds the word in its `argument` column.             |
| `language library did not load`                     | A language library does not open, or has no entry.                                      |
| `file did not parse`                                | The language does not parse the file.                                                   |
| `unit index failed`                                 | The index pass of a unit returns a nonzero value.                                       |
| `no language folder holds a construction file`      | No folder of the language folders holds `construction.md`.                             |

## Methods

The server answers these LSP methods. The operation word is the word of the command line that gives the same answer. See Operation Words.

Arguments named `uri` and `position` come from the LSP request. A position is zero-based. The server adds one to each line and column before the lookup.

Each answer is a JSON array unless the row says otherwise. Each array is sorted by path, then by position, unless the row says otherwise.

| Method                              | Operation              | Arguments                                 | Answer                                                                                                                                                  |
| :---------------------------------- | :--------------------- | :---------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `initialize`                        | none                   | `capabilities.general.positionEncodings`  | `result` with `capabilities` and `serverInfo`. Sets the position encoding of the session.                                                              |
| `initialized`                       | none                   | none                                      | None. Adds the connection to the daemon.                                                                                                                 |
| `shutdown`                          | none                   | none                                      | `result` is `null`. Later requests get error `-32600`.                                                                                                   |
| `exit`                              | none                   | none                                      | None. Ends the server. The exit code is 0 after `shutdown`, otherwise 1.                                                                                 |
| `$/cancelRequest`                   | none                   | `id`                                      | None. The pending request with that `id` answers with error `-32800`.                                                                                    |
| `textDocument/didOpen`              | didOpen                | `textDocument.uri`, `textDocument.text`   | None. Sets the buffer text of the file. The file parses from the buffer text.                                                                            |
| `textDocument/didChange`            | didChange              | `uri`, `contentChanges[0].text`           | None. Sync is Full. The first change holds the whole text, and it replaces the buffer text.                                                              |
| `textDocument/didClose`             | didClose               | `uri`                                     | None. Removes the buffer text. The file parses from disk again.                                                                                          |
| `textDocument/definition`           | goToDefinition         | `uri`, `position`                         | Array of `Location`: the declarations of the element. When the element has none, the declarations of the enclosing scopes.                              |
| `textDocument/declaration`          | goToDefinition         | `uri`, `position`                         | The same answer as `textDocument/definition`.                                                                                                            |
| `textDocument/typeDefinition`       | typeDefinition         | `uri`, `position`                         | Array of `Location`: the declarations of the type of the element.                                                                                        |
| `textDocument/references`           | findReferences         | `uri`, `position`                         | Array of `Location`: each declaration of the symbol, then each reference to it.                                                                          |
| `textDocument/documentHighlight`    | documentHighlight      | `uri`, `position`                         | Array of `{range, kind}` for the references in the same file. `kind` is always 1 (Text).                                                                 |
| `textDocument/rename`               | rename                 | `uri`, `position`, `newName`              | `WorkspaceEdit`: `changes` maps the URI of each file to its `TextEdit` array. `null` when no reference exists.                                           |
| `textDocument/codeAction`           | codeAction             | `uri`, `range`                            | Array of `CodeAction`: one for each fix-it that overlaps the range. Each has the title `apply fix-it`, the kind `quickfix`, and `edit.changes`.          |
| `textDocument/hover`                | hover                  | `uri`, `position`                         | `Hover`: `contents` with `kind` `plaintext` and `value`, and `range`. `null` when the element has no hover text. The definition text is read from the source of its declaration. |
| `textDocument/documentSymbol`       | documentSymbol         | `uri`                                     | Nested array of `DocumentSymbol`: `name`, `kind`, `range`, `selectionRange`, and `children`. Not sorted by path. The order follows the tree.             |
| `workspace/symbol`                  | workspaceSymbol        | `query`                                   | Array of `SymbolInformation`: `name`, `kind`, and `location`. The best fuzzy score comes first.                                                          |
| `textDocument/completion`           | completion             | `uri`, `position`                         | Array of `CompletionItem`: `label`, `kind`, `insertText`, and `insertTextFormat`. The position is the cursor after the typed name. Empty when the name has no reference role. |
| `textDocument/implementation`       | goToImplementation     | `uri`, `position`                         | Array of `Location`. For a class, each derived class. For a member, the members with the same name in the derived classes.                              |
| `textDocument/prepareCallHierarchy` | prepareCallHierarchy   | `uri`, `position`                         | Array of `CallHierarchyItem`: the function declarations at the position.                                                                                 |
| `callHierarchy/incomingCalls`       | incomingCalls          | `item.uri`, `item.selectionRange.start`   | Array of `{from, fromRanges}`: each declaration that calls the item, and the call ranges.                                                               |
| `callHierarchy/outgoingCalls`       | outgoingCalls          | `item.uri`, `item.selectionRange.start`   | Array of `{to, fromRanges}`: one entry for each call in the item, in document order. `fromRanges` holds the range of that call.                          |
| `textDocument/inlayHint`            | inlayHint              | `uri`                                     | Array of `{position, label, kind, paddingLeft, paddingRight}`. The labels come from the language library.                                                |
| `textDocument/semanticTokens/full`  | semanticTokens         | `uri`                                     | `{data}`: five integers for each token of the file. The integers are the line delta, the start delta, the length, the type index, and the modifier bits. |
| `textDocument/semanticTokens/range` | semanticTokensRange    | `uri`, `range`                            | The same shape as `textDocument/semanticTokens/full`, for the tokens in the range.                                                                      |
| `textDocument/selectionRange`       | selectionRange         | `uri`, the first entry of `positions`     | Array with one `SelectionRange`. The innermost range comes first, then each enclosing range up to the file. The file range is always present.             |
| `ling/textObject`                   | textObject             | `uri`, `position`, params `type`, `scope` | `Range` with `start` and `end`, or `null`. `type` is `function`, `class`, `loop`, or `conditional`. `scope` is `outer` for the whole element or `inner` for the body. |
| `textDocument/switchSourceHeader`   | switchSourceHeader     | `uri`                                     | The URI of the first counterpart file, or `null`. A counterpart has the same name and an extension that the language pairs with it.                     |
| `textDocument/publishDiagnostics`   | diagnostics            | none (sent by the daemon)                 | A notification after each indexed file: `uri`, and `diagnostics` of `{range, severity, message}`. The daemon sends it to each initialized server.       |

The capabilities of `initialize` hold `textDocumentSync` 1 (Full). They hold each provider of the methods above, except `switchSourceHeader`. The server answers `switchSourceHeader` without a capability.

## Operation Words

Each operation word is a command of the command line. Its answer is the result of its method in the Methods table. The `diagnostics` word has no method. It answers with the diagnostics of the file.

The argument shapes are:

- `file`: `<file>`.
- `position`: `<file> <line> <column>`. Line and column are 1-based. The column counts UTF-16 code units.
- `range`: `<file> <line> <column> <end line> <end column>`. Line and column are 1-based.
- `query`: `<query>`.

A relative file resolves against the working folder of the command.

The stdout of an operation is one JSON-RPC response: `jsonrpc` is `2.0`, `id` is `null`, and `result` holds the answer. Positions in the answer are zero-based.

| Word                 | Method                              | Arguments | Note                                                                                |
| :------------------- | :---------------------------------- | :-------- | :---------------------------------------------------------------------------------- |
| `goToDefinition`     | `textDocument/definition`           | position  |                                                                                     |
| `findReferences`     | `textDocument/references`           | position  |                                                                                     |
| `hover`              | `textDocument/hover`                | position  |                                                                                     |
| `documentSymbol`     | `textDocument/documentSymbol`       | file      |                                                                                     |
| `workspaceSymbol`    | `workspace/symbol`                  | query     |                                                                                     |
| `completion`         | `textDocument/completion`           | position  |                                                                                     |
| `goToImplementation` | `textDocument/implementation`       | position  |                                                                                     |
| `prepareCallHierarchy` | `textDocument/prepareCallHierarchy` | position  |                                                                                   |
| `incomingCalls`      | `callHierarchy/incomingCalls`       | position  |                                                                                     |
| `outgoingCalls`      | `callHierarchy/outgoingCalls`       | position  |                                                                                     |
| `switchSourceHeader` | `textDocument/switchSourceHeader`   | file      |                                                                                     |
| `inlayHint`          | `textDocument/inlayHint`            | file      |                                                                                     |
| `diagnostics`        | none                                | file      | The answer is an array of `{range, severity, message}`.                             |
| `publishDiagnostics` | `textDocument/publishDiagnostics`   | file      | The answer is a notification. The command does not end.                             |
| `didOpen`            | `textDocument/didOpen`              | file      | The command sends no text. The buffer text is empty.                                |
| `didChange`          | `textDocument/didChange`            | file      | The command sends no text. The buffer text is empty.                                |
| `didClose`           | `textDocument/didClose`             | file      |                                                                                     |
| `semanticTokens`     | `textDocument/semanticTokens/full`  | file      |                                                                                     |
| `semanticTokensRange` | `textDocument/semanticTokens/range` | range     |                                                                                     |
| `selectionRange`     | `textDocument/selectionRange`       | position  |                                                                                     |
| `textObject`         | `ling/textObject`                   | position  | The command sends no `type` and no `scope`.                                         |
| `documentHighlight`  | `textDocument/documentHighlight`    | position  |                                                                                     |
| `rename`             | `textDocument/rename`               | position  | The command sends no `newName`.                                                     |
| `codeAction`         | `textDocument/codeAction`           | range     |                                                                                     |
| `typeDefinition`     | `textDocument/typeDefinition`       | position  |                                                                                     |

A request waits until the files that it needs are indexed. Then the daemon answers it.

## Manifest

ling reads these tables of the manifest. A cell that starts with `@` names a row of the `## index` table. The text `${CMAKE_CURRENT_SOURCE_DIR}` stands for the folder of the manifest.

### index

The `## index` table maps each alias in the `alias` column to a value in the `symbol` column.

### toolchain

The `## toolchain` table has the columns `argument`, `command`, and `flag`. The manifest fails validation when the table lacks the `command` or the `flag` column.

A row with an empty `argument` belongs to the default flow. `--<word>` selects the rows whose `argument` is `<word>`. When a word is given, the rows with an empty `argument` are not used.

ling passes the `command` and `flag` of each row to each language. Each language reads the rows that it handles.

### daemon

The `## daemon` table holds the rows `timeout` and `threads` in its `value` column. Each value is a positive integer. The first `## daemon` table that holds the row sets the value.

| Row       | Meaning                                                                        | Default |
| :-------- | :----------------------------------------------------------------------------- | :------ |
| `timeout` | The time in milliseconds that the daemon waits for a client before it ends.    | 60000   |
| `threads` | The count of threads for the parse and index jobs.                             | 4       |

The parse and index threads run at background priority.

### source glob

The `## source glob` table has the columns `path` and `extension`. Each row adds the files of the folder `path` that have the extension. Each file is one unit.

### layout glob

The `## layout glob` table has the column `value`. Each value is a path whose last part is a wildcard file name. Each row adds the files that the wildcard matches. Each file is one unit.

### binary

The `## binary` table has the column `value`. Each value is a path whose last part is a wildcard file name. Each row adds the files that the wildcard matches. Each file is one unit.

The rows of `source glob`, `layout glob`, and `binary` apply to every `--<word>`.

### Built-in languages

The built-in languages read the rows of the three glob tables whose wildcard extension they handle. The built-in languages parse each file. They do not run the index pass.

| Extension | Language | Declares                    | Refers to          |
| :-------- | :------- | :-------------------------- | :----------------- |
| `md`      | markdown | a heading                   | a link             |
| `mmd`     | mermaid  | a node                      | an edge            |
| `json`    | json     | a key                       |                    |
| `css`     | css      | a selector                  |                    |
| `xml`     | xml      | a tag and the `id` value    |                    |
| `html`    | html     | a tag and the `id` value    |                    |

## Language Folder

A language folder holds the files of one language. ling finds each folder in `~/.config/ling/language/` that holds `construction.md`. The example language folder of this project is `language/cLang/`.

| File                          | Role                                                                                 |
| :---------------------------- | :----------------------------------------------------------------------------------- |
| `construction.md`             | The construction tables of the language. Required.                                   |
| `spell.md`                    | The cast spell that builds the language library.                                     |
| `language.cast`               | The source that the spell reads.                                                     |
| `build/liblanguage.dylib`     | The language library on macOS. On Windows, it is `build/language.dll`.               |

When no language folder holds `construction.md`, ling writes the default folder with `spell.md`, `language.cast`, and `construction.md`.

### extensions

The `## extensions` table has one row for each file extension. The key column holds each extension without its dot. ling sends each file with one of these extensions to the language.

### Library entries

A language library answers these entries. The library exports each entry by name.

| Entry                    | Kind               | Meaning                                                                                              |
| :----------------------- | :----------------- | :--------------------------------------------------------------------------------------------------- |
| `build`                  | function           | Builds the element tree of one source from its token records.                                        |
| `index`                  | function           | Indexes one unit and reports its symbols through the callbacks. Returns zero on success.            |
| `units`                  | function           | Lists the units of each row through the callbacks.                                                   |
| `code`                   | int table, 256 rows | The byte class of each byte value.                                                                  |
| `commentOpen`            | string             | The opening text of a block comment.                                                                 |
| `commentClose`           | string             | The closing text of a block comment.                                                                 |
| `lineCommentOpen`        | string             | The opening text of a line comment.                                                                  |
| `escape`                 | int                | The escape byte.                                                                                     |
| `rawStringOpen`          | string             | The opening text of a raw string.                                                                    |
| `keywords`               | string array       | The keyword texts. The index of a keyword is its key. The array ends with a null entry.              |
| `roles`                  | int table          | The role set of each token type.                                                                     |
| `symbolTypes`            | int table          | The symbol kind of each token type.                                                                  |
| `completionTypes`        | int table          | The completion kind of each token type.                                                              |
| `semanticTokenTypes`     | string array       | The legend token types. The array ends with a null entry.                                            |
| `semanticTokenModifiers` | string array       | The legend modifiers. Bit `n` is entry `n`. The array ends with a null entry.                        |
| `semanticTypes`          | int table          | The index of the legend type of each token type.                                                     |
| `semanticModifiers`      | int table          | The modifier bits of each token type.                                                                |
| `extensionPairs`         | string array       | Pairs of extensions, two entries for each pair. `switchSourceHeader` reads them. The array ends with a null entry. |

### Roles

A role set is a bit set. Each token type holds one role set in the `roles` entry.

| Role          | Value | Meaning                                                |
| :------------ | :---- | :----------------------------------------------------- |
| `declaration` | 1     | The token type declares a name.                        |
| `member`      | 2     | The token type is a member.                            |
| `function`    | 4     | The token type is a function.                          |
| `classToken`  | 8     | The token type is a class.                             |
| `constructor` | 16    | The token type is a constructor.                       |
| `reference`   | 32    | The token type refers to a name.                       |
| `call`        | 64    | The token type calls a name.                           |
| `base`        | 128   | The token type names a base class.                     |
| `loop`        | 256   | The token type is a loop. `ling/textObject` reads it.  |
| `conditional` | 512   | The token type is a conditional. `ling/textObject` reads it. |

### Callbacks

A library calls these callbacks. ling gives each callback a context and a function. The callbacks of the build entry fill the element tree. The callbacks of the other entries report results to ling.

| Callback            | Called by | Meaning                                                                                                    |
| :------------------ | :-------- | :--------------------------------------------------------------------------------------------------------- |
| `addElement`        | build     | Adds a child element under `parent`. Returns the index of the element.                                     |
| `addComment`        | build     | Stores the comment tokens on an element.                                                                   |
| `addParameters`     | build     | Stores the parameter tokens on an element.                                                                 |
| `addMember`         | build     | Marks an element as a member.                                                                              |
| `addLinkEndpoints`  | build     | Links a reference element and a target element in both directions.                                        |
| `addSymbol`         | index     | Adds one symbol record: the path, the line, the column, the 64-bit symbol hash, the name length, the role, the type, the hover index, and the type symbols. |
| `addInlayHint`      | index     | Adds one inlay hint: the position, the label, the type, and the padding.                                   |
| `addDiagnostic`     | index     | Adds one diagnostic: the range, the severity, and the message.                                             |
| `addOutline`        | index     | Adds one outline entry. Returns its index. A parent that the callback returned earlier gives the nesting. A negative parent gives no parent. |
| `addCompletion`     | index     | Adds one completion item to a scope symbol: the label, the insert text, and the token type.                |
| `addFixIt`          | index     | Adds one replacement text for a range.                                                                     |
| `shouldExit`        | index     | Returns a nonzero value to stop the index pass.                                                            |
| `addUnit`           | units     | Adds one unit: the path, the directory, the output, the arguments, the dependencies, and the prerequisites. |
| `addBuildFile`      | units     | Adds a build file. A change of that file runs the manifest again.                                          |
| `addMessage`        | units     | Sets a failure text for the unit list.                                                                     |
| `addHover`          | index     | Adds one hover text of a file, with the line and column span of its definition in the source. Returns its index in the file. |

## Carried Libraries

The release carries the language libraries as archives. The daemon unpacks the archives once into `<cache>/language/<hash>`. The cache is `~/.cache/ling`.

A language folder with a library in `build/` uses that library. Otherwise, the daemon uses the carried library of the folder name.

The daemon opens a copy of the library. The copy is in `<cache>/language/<name>/` and its name is the hash of the library bytes.

## Build

`ling --build <folder>` builds one language folder. It does these steps in order:

1. Writes `core.md`, `syntax_tree.md`, `lsp.md`, and `jam_LanguageInterface.h` into the folder.
2. Copies the files of the carried `core` folder into the folder.
3. Runs `cast spell.md` in the folder, and prints the output of cast.
4. Exits with the exit code of cast.

The command needs `cast` on the search path. The spell names the other tools that it needs.

When the build writes the library into `build/`, the daemon loads the new library. No restart is needed.
