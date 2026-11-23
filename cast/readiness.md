## readiness

```
@brief The tree whose Id::isIndexed each operation waits for: the file row or the files child.
```

+--------------------------+------------------+
| key                      | value            |
+==========================+==================+
| Id::documentSymbol       | Id::file         |
| Id::hover                | Id::file         |
| Id::inlayHint            | Id::file         |
| Id::switchSourceHeader   | Id::file         |
| Id::diagnostics          | Id::file         |
| Id::publishDiagnostics   | Id::file         |
| Id::goToDefinition       | Id::dependencies |
| Id::findReferences       | Id::files        |
| Id::workspaceSymbol      | Id::files        |
| Id::completion           | Id::file         |
| Id::goToImplementation   | Id::files        |
| Id::prepareCallHierarchy | Id::dependencies |
| Id::incomingCalls        | Id::files        |
| Id::outgoingCalls        | Id::dependencies |
| Id::setBuffer            | Id::file         |
| Id::removeBuffer         | Id::file         |
| Id::semanticTokens       | Id::file         |
| Id::semanticTokensRange  | Id::file         |
| Id::selectionRange       | Id::file         |
| Id::textObject           | Id::file         |
| Id::documentHighlight    | Id::file         |
| Id::codeAction           | Id::file         |
| Id::typeDefinition       | Id::dependencies |
| Id::rename               | Id::files        |
+--------------------------+------------------+
