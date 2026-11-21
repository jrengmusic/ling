## index

+-------+------------------+
| alias | symbol           |
+=======+==================+
| @int  | int              |
| @view | std::string_view |
+-------+------------------+

## Method

```
@brief The LSP 3.17 method names that ling serves.
```

+-------+----------------------+-------------------------------------+-------------+
| type  | name                 | value                               | description |
+=======+======================+=====================================+=============+
| @view | initialize           | `initialize`                        |             |
| @view | initialized          | `initialized`                       |             |
| @view | shutdown             | `shutdown`                          |             |
| @view | exit                 | `exit`                              |             |
| @view | cancelRequest        | `$/cancelRequest`                   |             |
| @view | didOpen              | `textDocument/didOpen`              |             |
| @view | didChange            | `textDocument/didChange`            |             |
| @view | didClose             | `textDocument/didClose`             |             |
| @view | publishDiagnostics   | `textDocument/publishDiagnostics`   |             |
| @view | showMessage          | `window/showMessage`                |             |
| @view | progress             | `$/progress`                        |             |
| @view | definition           | `textDocument/definition`           |             |
| @view | references           | `textDocument/references`           |             |
| @view | hover                | `textDocument/hover`                |             |
| @view | documentSymbol       | `textDocument/documentSymbol`       |             |
| @view | workspaceSymbol      | `workspace/symbol`                  |             |
| @view | completion           | `textDocument/completion`           |             |
| @view | implementation       | `textDocument/implementation`       |             |
| @view | prepareCallHierarchy | `textDocument/prepareCallHierarchy` |             |
| @view | incomingCalls        | `callHierarchy/incomingCalls`       |             |
| @view | outgoingCalls        | `callHierarchy/outgoingCalls`       |             |
| @view | inlayHint            | `textDocument/inlayHint`            |             |
| @view | declaration          | `textDocument/declaration`          |             |
| @view | typeDefinition       | `textDocument/typeDefinition`       |             |
| @view | documentHighlight    | `textDocument/documentHighlight`    |             |
| @view | rename               | `textDocument/rename`               |             |
| @view | codeAction           | `textDocument/codeAction`           |             |
| @view | switchSourceHeader   | `textDocument/switchSourceHeader`   |             |
+-------+----------------------+-------------------------------------+-------------+

## Member

```
@brief The LSP 3.17 member names of the messages that ling writes.
```

+-------+---------------------------+-----------------------------+-------------+
| type  | name                      | value                       | description |
+=======+===========================+=============================+=============+
| @view | uri                       | `uri`                       |             |
| @view | range                     | `range`                     |             |
| @view | start                     | `start`                     |             |
| @view | end                       | `end`                       |             |
| @view | line                      | `line`                      |             |
| @view | character                 | `character`                 |             |
| @view | selectionRange            | `selectionRange`            |             |
| @view | name                      | `name`                      |             |
| @view | type                      | `kind`                      |             |
| @view | children                  | `children`                  |             |
| @view | contents                  | `contents`                  |             |
| @view | value                     | `value`                     |             |
| @view | label                     | `label`                     |             |
| @view | position                  | `position`                  |             |
| @view | paddingLeft               | `paddingLeft`               |             |
| @view | paddingRight              | `paddingRight`              |             |
| @view | severity                  | `severity`                  |             |
| @view | message                   | `message`                   |             |
| @view | documentation             | `documentation`             |             |
| @view | insertText                | `insertText`                |             |
| @view | insertTextFormat          | `insertTextFormat`          |             |
| @view | from                      | `from`                      |             |
| @view | to                        | `to`                        |             |
| @view | fromRanges                | `fromRanges`                |             |
| @view | location                  | `location`                  |             |
| @view | result                    | `result`                    |             |
| @view | method                    | `method`                    |             |
| @view | params                    | `params`                    |             |
| @view | messageType               | `type`                      |             |
| @view | jsonrpc                   | `jsonrpc`                   |             |
| @view | id                        | `id`                        |             |
| @view | error                     | `error`                     |             |
| @view | code                      | `code`                      |             |
| @view | textDocument              | `textDocument`              |             |
| @view | text                      | `text`                      |             |
| @view | contentChanges            | `contentChanges`            |             |
| @view | query                     | `query`                     |             |
| @view | item                      | `item`                      |             |
| @view | capabilities              | `capabilities`              |             |
| @view | general                   | `general`                   |             |
| @view | positionEncodings         | `positionEncodings`         |             |
| @view | positionEncoding          | `positionEncoding`          |             |
| @view | textDocumentSync          | `textDocumentSync`          |             |
| @view | hoverProvider             | `hoverProvider`             |             |
| @view | definitionProvider        | `definitionProvider`        |             |
| @view | referencesProvider        | `referencesProvider`        |             |
| @view | documentSymbolProvider    | `documentSymbolProvider`    |             |
| @view | workspaceSymbolProvider   | `workspaceSymbolProvider`   |             |
| @view | completionProvider        | `completionProvider`        |             |
| @view | implementationProvider    | `implementationProvider`    |             |
| @view | callHierarchyProvider     | `callHierarchyProvider`     |             |
| @view | inlayHintProvider         | `inlayHintProvider`         |             |
| @view | semanticTokensProvider    | `semanticTokensProvider`    |             |
| @view | selectionRangeProvider    | `selectionRangeProvider`    |             |
| @view | serverInfo                | `serverInfo`                |             |
| @view | version                   | `version`                   |             |
| @view | diagnostics               | `diagnostics`               |             |
| @view | legend                    | `legend`                    |             |
| @view | tokenTypes                | `tokenTypes`                |             |
| @view | tokenModifiers            | `tokenModifiers`            |             |
| @view | full                      | `full`                      |             |
| @view | data                      | `data`                      |             |
| @view | positions                 | `positions`                 |             |
| @view | parent                    | `parent`                    |             |
| @view | declarationProvider       | `declarationProvider`       |             |
| @view | documentHighlightProvider | `documentHighlightProvider` |             |
| @view | renameProvider            | `renameProvider`            |             |
| @view | changes                   | `changes`                   |             |
| @view | newText                   | `newText`                   |             |
| @view | title                     | `title`                     |             |
| @view | edit                      | `edit`                      |             |
| @view | typeDefinitionProvider    | `typeDefinitionProvider`    |             |
| @view | codeActionProvider        | `codeActionProvider`        |             |
| @view | token                     | `token`                     |             |
| @view | percentage                | `percentage`                |             |
+-------+---------------------------+-----------------------------+-------------+

## Message

```
@brief The LSP 3.17 base protocol version.
```

+-------+---------+-------+-------------+
| type  | name    | value | description |
+=======+=========+=======+=============+
| @view | jsonrpc | `2.0` |             |
+-------+---------+-------+-------------+

## MarkupType

```
@brief The LSP 3.17 MarkupKind values.
```

+-------+-----------+-------------+-------------+
| type  | name      | value       | description |
+=======+===========+=============+=============+
| @view | PlainText | `plaintext` |             |
| @view | Markdown  | `markdown`  |             |
+-------+-----------+-------------+-------------+

## WorkDoneProgressKind

```
@brief The LSP 3.17 WorkDoneProgressKind values.
```

+-------+--------+----------+-------------+
| type  | name   | value    | description |
+=======+========+==========+=============+
| @view | Begin  | `begin`  |             |
| @view | Report | `report` |             |
| @view | End    | `end`    |             |
+-------+--------+----------+-------------+

## MessageType

```
@brief The LSP 3.17 MessageType values.
```

+------+---------+-------+-------------+
| type | name    | value | description |
+======+=========+=======+=============+
| @int | Error   | 1     |             |
| @int | Warning | 2     |             |
| @int | Info    | 3     |             |
| @int | Log     | 4     |             |
+------+---------+-------+-------------+

## InsertTextFormat

```
@brief The LSP 3.17 InsertTextFormat values.
```

+------+-----------+-------+-------------+
| type | name      | value | description |
+======+===========+=======+=============+
| @int | PlainText | 1     |             |
| @int | Snippet   | 2     |             |
+------+-----------+-------+-------------+

## operations

```
@brief The ling operation that answers each LSP request method.
```

+-------------------------------------+--------------------------+
| key                                 | value                    |
+=====================================+==========================+
| `textDocument/definition`           | Id::goToDefinition       |
| `textDocument/references`           | Id::findReferences       |
| `textDocument/hover`                | Id::hover                |
| `textDocument/documentSymbol`       | Id::documentSymbol       |
| `workspace/symbol`                  | Id::workspaceSymbol      |
| `textDocument/completion`           | Id::completion           |
| `textDocument/implementation`       | Id::goToImplementation   |
| `textDocument/prepareCallHierarchy` | Id::prepareCallHierarchy |
| `callHierarchy/incomingCalls`       | Id::incomingCalls        |
| `callHierarchy/outgoingCalls`       | Id::outgoingCalls        |
| `textDocument/inlayHint`            | Id::inlayHint            |
| `textDocument/semanticTokens/full`  | Id::semanticTokens       |
| `textDocument/semanticTokens/range` | Id::semanticTokensRange  |
| `textDocument/selectionRange`       | Id::selectionRange       |
| `ling/textObject`                   | Id::textObject           |
| `textDocument/declaration`          | Id::goToDefinition       |
| `textDocument/typeDefinition`       | Id::typeDefinition       |
| `textDocument/documentHighlight`    | Id::documentHighlight    |
| `textDocument/rename`               | Id::rename               |
| `textDocument/codeAction`           | Id::codeAction           |
| `textDocument/switchSourceHeader`   | Id::switchSourceHeader   |
+-------------------------------------+--------------------------+

## SymbolType

```
@brief The LSP 3.17 SymbolKind values.
```

+------+---------------+-------+-------------+
| type | name          | value | description |
+======+===============+=======+=============+
| @int | File          | 1     |             |
| @int | Module        | 2     |             |
| @int | Namespace     | 3     |             |
| @int | Package       | 4     |             |
| @int | Class         | 5     |             |
| @int | Method        | 6     |             |
| @int | Property      | 7     |             |
| @int | Field         | 8     |             |
| @int | Constructor   | 9     |             |
| @int | Enum          | 10    |             |
| @int | Interface     | 11    |             |
| @int | Function      | 12    |             |
| @int | Variable      | 13    |             |
| @int | Constant      | 14    |             |
| @int | String        | 15    |             |
| @int | Number        | 16    |             |
| @int | Boolean       | 17    |             |
| @int | Array         | 18    |             |
| @int | Object        | 19    |             |
| @int | Key           | 20    |             |
| @int | Null          | 21    |             |
| @int | EnumMember    | 22    |             |
| @int | Struct        | 23    |             |
| @int | Event         | 24    |             |
| @int | Operator      | 25    |             |
| @int | TypeParameter | 26    |             |
+------+---------------+-------+-------------+

## CompletionItemType

```
@brief The LSP 3.17 CompletionItemKind values.
```

+------+---------------+-------+-------------+
| type | name          | value | description |
+======+===============+=======+=============+
| @int | Text          | 1     |             |
| @int | Method        | 2     |             |
| @int | Function      | 3     |             |
| @int | Constructor   | 4     |             |
| @int | Field         | 5     |             |
| @int | Variable      | 6     |             |
| @int | Class         | 7     |             |
| @int | Interface     | 8     |             |
| @int | Module        | 9     |             |
| @int | Property      | 10    |             |
| @int | Unit          | 11    |             |
| @int | Value         | 12    |             |
| @int | Enum          | 13    |             |
| @int | Keyword       | 14    |             |
| @int | Snippet       | 15    |             |
| @int | Color         | 16    |             |
| @int | File          | 17    |             |
| @int | Reference     | 18    |             |
| @int | Folder        | 19    |             |
| @int | EnumMember    | 20    |             |
| @int | Constant      | 21    |             |
| @int | Struct        | 22    |             |
| @int | Event         | 23    |             |
| @int | Operator      | 24    |             |
| @int | TypeParameter | 25    |             |
+------+---------------+-------+-------------+

## InlayHintType

```
@brief The LSP 3.17 InlayHintKind values.
```

+------+-----------+-------+-------------+
| type | name      | value | description |
+======+===========+=======+=============+
| @int | Type      | 1     |             |
| @int | Parameter | 2     |             |
+------+-----------+-------+-------------+

## DocumentHighlightType

```
@brief The LSP 3.17 DocumentHighlightKind values.
```

+------+-------+-------+-------------+
| type | name  | value | description |
+======+=======+=======+=============+
| @int | Text  | 1     |             |
| @int | Read  | 2     |             |
| @int | Write | 3     |             |
+------+-------+-------+-------------+

## CodeActionKind

```
@brief The LSP 3.17 CodeActionKind values.
```

+-------+----------+------------+-------------+
| type  | name     | value      | description |
+=======+==========+============+=============+
| @view | QuickFix | `quickfix` |             |
+-------+----------+------------+-------------+

## DiagnosticSeverity

```
@brief The LSP 3.17 DiagnosticSeverity values.
```

+------+-------------+-------+-------------+
| type | name        | value | description |
+======+=============+=======+=============+
| @int | Error       | 1     |             |
| @int | Warning     | 2     |             |
| @int | Information | 3     |             |
| @int | Hint        | 4     |             |
+------+-------------+-------+-------------+

## TextDocumentSyncType

```
@brief The LSP 3.17 TextDocumentSyncKind values.
```

+------+-------------+-------+-------------+
| type | name        | value | description |
+======+=============+=======+=============+
| @int | None        | 0     |             |
| @int | Full        | 1     |             |
| @int | Incremental | 2     |             |
+------+-------------+-------+-------------+

## PositionEncodingType

```
@brief The LSP 3.17 PositionEncodingKind values.
```

+-------+-------+----------+-------------+
| type  | name  | value    | description |
+=======+=======+==========+=============+
| @view | UTF8  | `utf-8`  |             |
| @view | UTF16 | `utf-16` |             |
| @view | UTF32 | `utf-32` |             |
+-------+-------+----------+-------------+

## ErrorCodes

```
@brief The LSP 3.17 ErrorCodes and LSPErrorCodes values.
```

+------+----------------------+--------+-------------+
| type | name                 | value  | description |
+======+======================+========+=============+
| @int | ParseError           | -32700 |             |
| @int | InvalidRequest       | -32600 |             |
| @int | MethodNotFound       | -32601 |             |
| @int | InvalidParams        | -32602 |             |
| @int | InternalError        | -32603 |             |
| @int | ServerNotInitialized | -32002 |             |
| @int | UnknownErrorCode     | -32001 |             |
| @int | RequestFailed        | -32803 |             |
| @int | ServerCancelled      | -32802 |             |
| @int | ContentModified      | -32801 |             |
| @int | RequestCancelled     | -32800 |             |
+------+----------------------+--------+-------------+

## SemanticTokenType

```
@brief The LSP 3.17 SemanticTokenTypes predefined values, in legend order.
```

+----------------+-----+-----------------+
| name           | key | value           |
+================+=====+=================+
| namespaceToken | 0   | `namespace`     |
| type           | 1   | `type`          |
| classToken     | 2   | `class`         |
| enumToken      | 3   | `enum`          |
| interfaceToken | 4   | `interface`     |
| structToken    | 5   | `struct`        |
| typeParameter  | 6   | `typeParameter` |
| parameter      | 7   | `parameter`     |
| variable       | 8   | `variable`      |
| property       | 9   | `property`      |
| enumMember     | 10  | `enumMember`    |
| event          | 11  | `event`         |
| function       | 12  | `function`      |
| method         | 13  | `method`        |
| macro          | 14  | `macro`         |
| keyword        | 15  | `keyword`       |
| modifier       | 16  | `modifier`      |
| comment        | 17  | `comment`       |
| string         | 18  | `string`        |
| number         | 19  | `number`        |
| regexp         | 20  | `regexp`        |
| operatorToken  | 21  | `operator`      |
| decorator      | 22  | `decorator`     |
+----------------+-----+-----------------+

## SemanticTokenModifier

```
@brief The LSP 3.17 SemanticTokenModifiers predefined values, in legend order.
```

+----------------+-----+------------------+
| name           | key | value            |
+================+=====+==================+
| declaration    | 0   | `declaration`    |
| definition     | 1   | `definition`     |
| readonly       | 2   | `readonly`       |
| staticToken    | 3   | `static`         |
| deprecated     | 4   | `deprecated`     |
| abstract       | 5   | `abstract`       |
| async          | 6   | `async`          |
| modification   | 7   | `modification`   |
| documentation  | 8   | `documentation`  |
| defaultLibrary | 9   | `defaultLibrary` |
+----------------+-----+------------------+
