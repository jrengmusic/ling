## parameters

```
@brief The argument shape of each operation: a position, a file, or a query.
```

+--------------------------+--------------+
| key                      | value        |
+==========================+==============+
| Id::goToDefinition       | Id::position |
| Id::findReferences       | Id::position |
| Id::hover                | Id::position |
| Id::completion           | Id::position |
| Id::goToImplementation   | Id::position |
| Id::prepareCallHierarchy | Id::position |
| Id::incomingCalls        | Id::position |
| Id::outgoingCalls        | Id::position |
| Id::selectionRange       | Id::position |
| Id::textObject           | Id::position |
| Id::typeDefinition       | Id::position |
| Id::documentHighlight    | Id::position |
| Id::rename               | Id::position |
| Id::codeAction           | Id::range    |
| Id::documentSymbol       | Id::file     |
| Id::switchSourceHeader   | Id::file     |
| Id::inlayHint            | Id::file     |
| Id::diagnostics          | Id::file     |
| Id::publishDiagnostics   | Id::file     |
| Id::semanticTokens       | Id::file     |
| Id::semanticTokensRange  | Id::range    |
| Id::workspaceSymbol      | Id::query    |
| Id::didOpen              | Id::file     |
| Id::didChange            | Id::file     |
| Id::didClose             | Id::file     |
+--------------------------+--------------+
