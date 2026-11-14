## statementConstruction

| key                                 | construction                                                                                                                                |
| ----------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| map::CppKeyword::tokenAlignas       | addDeclaration                                                                                                                              |
| map::CppKeyword::tokenTemplate      | skipTemplateParameters                                                                                                                      |
| map::CppKeyword::tokenNamespace     | addNamespace                                                                                                                                |
| map::CppKeyword::tokenClass         | addClass                                                                                                                                    |
| map::CppKeyword::tokenStruct        | addClass                                                                                                                                    |
| map::CppKeyword::tokenEnum          | addEnum                                                                                                                                     |
| map::CppKeyword::tokenExtern        | addExtern                                                                                                                                   |
| map::CppKeyword::tokenPublic        | [] (const int&, CLanguageDocument&, int, const Tokens&, int position, int end) { return std::min (position + accessSpecifierLength, end); } |
| map::CppKeyword::tokenPrivate       | [] (const int&, CLanguageDocument&, int, const Tokens&, int position, int end) { return std::min (position + accessSpecifierLength, end); } |
| map::CppKeyword::tokenProtected     | [] (const int&, CLanguageDocument&, int, const Tokens&, int position, int end) { return std::min (position + accessSpecifierLength, end); } |
| map::CppKeyword::tokenIf            | addControl                                                                                                                                  |
| map::CppKeyword::tokenFor           | addControl                                                                                                                                  |
| map::CppKeyword::tokenWhile         | addControl                                                                                                                                  |
| map::CppKeyword::tokenSwitch        | addControl                                                                                                                                  |
| map::CppKeyword::tokenCatch         | addControl                                                                                                                                  |
| map::CppKeyword::tokenElse          | [] (const int&, CLanguageDocument&, int, const Tokens&, int position, int) { return position + 1; }                                         |
| map::CppKeyword::tokenDo            | [] (const int&, CLanguageDocument&, int, const Tokens&, int position, int) { return position + 1; }                                         |
| map::CppKeyword::tokenTry           | [] (const int&, CLanguageDocument&, int, const Tokens&, int position, int) { return position + 1; }                                         |
| map::CppKeyword::tokenReturn        | skipStatement                                                                                                                               |
| map::CppKeyword::tokenThrow         | skipStatement                                                                                                                               |
| map::CppKeyword::tokenBreak         | skipStatement                                                                                                                               |
| map::CppKeyword::tokenContinue      | skipStatement                                                                                                                               |
| map::CppKeyword::tokenGoto          | skipStatement                                                                                                                               |
| map::CppKeyword::tokenDelete        | skipStatement                                                                                                                               |
| map::CppKeyword::tokenCase          | skipStatement                                                                                                                               |
| map::CppKeyword::tokenDefault       | skipStatement                                                                                                                               |
| map::CppKeyword::tokenUsing         | skipStatement                                                                                                                               |
| map::CppKeyword::tokenTypedef       | skipStatement                                                                                                                               |
| map::CppKeyword::tokenFriend        | skipStatement                                                                                                                               |
| map::CppKeyword::tokenStatic_assert | skipStatement                                                                                                                               |
| map::CppKeyword::tokenUnion         | skipStatement                                                                                                                               |

## punctuationConstruction

| key                  | construction   |
| -------------------- | -------------- |
| Chars::nullCharacter | addDeclaration |
| Chars::semicolon     | skipStatement  |
| Chars::openBrace     | addBlock       |
| Chars::hash          | addDirective   |

## depthChanges

| key                  | value |
| -------------------- | ----- |
| Chars::nullCharacter | 0     |
| Chars::openParen     | 1     |
| Chars::openBracket   | 1     |
| Chars::openBrace     | 1     |
| Chars::closeParen    | -1    |
| Chars::closeBracket  | -1    |
| Chars::closeBrace    | -1    |

## scopeTypes

| key                                  | value                                |
| ------------------------------------ | ------------------------------------ |
| map::SyntaxTokenType::namespaceToken | map::SyntaxTokenType::namespaceToken |
| map::SyntaxTokenType::classToken     | map::SyntaxTokenType::classToken     |
| map::SyntaxTokenType::structToken    | map::SyntaxTokenType::classToken     |
| map::SyntaxTokenType::function       | map::SyntaxTokenType::function       |
| map::SyntaxTokenType::instanceMethod | map::SyntaxTokenType::function       |
| map::SyntaxTokenType::staticMethod   | map::SyntaxTokenType::function       |
| map::SyntaxTokenType::constructor    | map::SyntaxTokenType::function       |
| map::SyntaxTokenType::destructor     | map::SyntaxTokenType::function       |

## declarationTypes

| key                                  | functionType                         | staticFunctionType                   | variableType                   | staticVariableType                   |
| ------------------------------------ | ------------------------------------ | ------------------------------------ | ------------------------------ | ------------------------------------ |
| map::SyntaxTokenType::namespaceToken | map::SyntaxTokenType::function       | map::SyntaxTokenType::function       | map::SyntaxTokenType::variable | map::SyntaxTokenType::staticVariable |
| map::SyntaxTokenType::classToken     | map::SyntaxTokenType::instanceMethod | map::SyntaxTokenType::staticMethod   | map::SyntaxTokenType::field    | map::SyntaxTokenType::staticVariable |
| map::SyntaxTokenType::structToken    | map::SyntaxTokenType::instanceMethod | map::SyntaxTokenType::staticMethod   | map::SyntaxTokenType::field    | map::SyntaxTokenType::staticVariable |
| map::SyntaxTokenType::function       | map::SyntaxTokenType::variable       | map::SyntaxTokenType::staticVariable | map::SyntaxTokenType::variable | map::SyntaxTokenType::staticVariable |
| map::SyntaxTokenType::instanceMethod | map::SyntaxTokenType::variable       | map::SyntaxTokenType::staticVariable | map::SyntaxTokenType::variable | map::SyntaxTokenType::staticVariable |
| map::SyntaxTokenType::staticMethod   | map::SyntaxTokenType::variable       | map::SyntaxTokenType::staticVariable | map::SyntaxTokenType::variable | map::SyntaxTokenType::staticVariable |
| map::SyntaxTokenType::constructor    | map::SyntaxTokenType::variable       | map::SyntaxTokenType::staticVariable | map::SyntaxTokenType::variable | map::SyntaxTokenType::staticVariable |
| map::SyntaxTokenType::destructor     | map::SyntaxTokenType::variable       | map::SyntaxTokenType::staticVariable | map::SyntaxTokenType::variable | map::SyntaxTokenType::staticVariable |

## comparisonPrefixes

| key                  | value |
| -------------------- | ----- |
| Chars::nullCharacter | false |
| Chars::equals        | true  |
| Chars::exclamation   | true  |
| Chars::lessThan      | true  |
| Chars::greaterThan   | true  |

## declaratorBytes

| key                  | value |
| -------------------- | ----- |
| Chars::nullCharacter | false |
| Chars::asterisk      | true  |
| Chars::ampersand     | true  |
| Chars::greaterThan   | true  |

## roles

| key                                  | declaration               | member               | function               | classToken               | constructor               | reference               | call               | base               |
| ------------------------------------ | ------------------------- | -------------------- | ---------------------- | ------------------------ | ------------------------- | ----------------------- | ------------------ | ------------------ |
| map::SyntaxTokenType::keyword        | 0                         | 0                    | 0                      | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::namespaceToken | LanguageRole::declaration | 0                    | 0                      | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::classToken     | LanguageRole::declaration | 0                    | 0                      | LanguageRole::classToken | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::structToken    | LanguageRole::declaration | 0                    | 0                      | LanguageRole::classToken | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::enumToken      | LanguageRole::declaration | 0                    | 0                      | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::enumConstant   | LanguageRole::declaration | 0                    | 0                      | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::function       | LanguageRole::declaration | 0                    | LanguageRole::function | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::instanceMethod | LanguageRole::declaration | LanguageRole::member | LanguageRole::function | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::staticMethod   | LanguageRole::declaration | LanguageRole::member | LanguageRole::function | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::constructor    | LanguageRole::declaration | 0                    | LanguageRole::function | 0                        | LanguageRole::constructor | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::destructor     | LanguageRole::declaration | 0                    | LanguageRole::function | 0                        | LanguageRole::constructor | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::variable       | LanguageRole::declaration | 0                    | 0                      | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::staticVariable | LanguageRole::declaration | LanguageRole::member | 0                      | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::field          | LanguageRole::declaration | LanguageRole::member | 0                      | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::macro          | LanguageRole::declaration | 0                    | 0                      | 0                        | 0                         | 0                       | 0                  | 0                  |
| map::SyntaxTokenType::reference      | 0                         | 0                    | 0                      | 0                        | 0                         | LanguageRole::reference | 0                  | 0                  |
| map::SyntaxTokenType::call           | 0                         | 0                    | 0                      | 0                        | 0                         | LanguageRole::reference | LanguageRole::call | 0                  |
| map::SyntaxTokenType::base           | 0                         | 0                    | 0                      | 0                        | 0                         | LanguageRole::reference | 0                  | LanguageRole::base |

## extensionPairs

| key   | value   |
| ----- | ------- |
| `cpp` | `h`     |
| `c`   | `h`     |
| `cxx` | `h`     |
| `mm`  | `h`     |

## compilerManifest

```
@brief The words of the C family compiler manifest and dependency record.
```

+-------------------------+-------------------------+-------------------------------------------------------------------------------+
| name                    | value                   | description                                                                   |
+=========================+=========================+===============================================================================+
| compilerManifestCommand | `cmake`                 | The command cell of the toolchain row that names the build folder.            |
| flagBuildDirectory      | `-B`                    | The flag whose next token is the build folder.                                |
| compilerManifest        | `compile_commands.json` | The file name of the compiler manifest in the build folder.                   |
| dependencyCommand       | `ninja`                 | The command cell of the toolchain row that gives the dependency record.       |
| flagDependencyDirectory | `-C`                    | The flag whose next token is the build folder of that row.                    |
| dependencyFlag          | `-t deps`               | The flag that makes the command print the dependency record.                  |
| dependencyHead          | `: #deps`               | The text that follows the output path in the head line of a record.           |
| dependencyRecord        | `.ninja_deps`           | The file of the build folder that changes when the dependency record changes. |
+-------------------------+-------------------------+-------------------------------------------------------------------------------+

## checkFlags mac

```
@brief The flags that make a C family compile command check the code and write no output file, per platform.
```

+----------------------------------+
| value                            |
+==================================+
| `-fsyntax-only`                  |
+----------------------------------+
| `-fdiagnostics-format=msvc`      |
+----------------------------------+
| `-fdiagnostics-absolute-paths`   |
+----------------------------------+
| `-fno-color-diagnostics`         |
+----------------------------------+
| `-fdiagnostics-parseable-fixits` |
+----------------------------------+

## checkFlags windows

+-----------------------+
| value                 |
+=======================+
| `/Zs`                 |
+-----------------------+
| `/diagnostics:column` |
+-----------------------+

## toolchainWords

| name                   | value                |
| ---------------------- | -------------------- |
| installDirectoryPrefix | `InstalledDir:`      |
| versionFlag            | `--version`          |
| workingDirectoryFlag   | `-working-directory` |
| precompiledSuffix      | `.pch`               |
| pipeReadMode           | `r`                  |

## toolchainValues

| name                 | value |
| -------------------- | ----- |
| precompiledArguments | 4     |

## libclangSymbols

| name                    | value                              |
| ----------------------- | ---------------------------------- |
| createIndex             | `clang_createIndex`                |
| createIndexAction       | `clang_IndexAction_create`         |
| indexSourceFileFullArgv | `clang_indexSourceFileFullArgv`    |
| getCXXClassDeclInfo     | `clang_index_getCXXClassDeclInfo`  |
| getFileLocation         | `clang_indexLoc_getFileLocation`   |
| getFileName             | `clang_getFileName`                |
| getCString              | `clang_getCString`                 |
| disposeString           | `clang_disposeString`              |
| disposeIndex            | `clang_disposeIndex`               |
| disposeIndexAction      | `clang_IndexAction_dispose`        |

## libraryPaths

| key             | value                     |
| --------------- | ------------------------- |
| platformMac     | `../lib/libclang.dylib`   |
| platformWindows | `libclang.dll`            |

## sdkCommands

| key         | value                   |
| ----------- | ----------------------- |
| platformMac | `xcrun --show-sdk-path` |

## sdkFlags

| key         | value       |
| ----------- | ----------- |
| platformMac | `-isysroot` |

## drivers

| key             | value             |
| --------------- | ----------------- |
| `cc`            | `clang`           |
| `c++`           | `clang++`         |
| `clang`         | `clang`           |
| `clang++`       | `clang++`         |
| `clang-cl.exe`  | `clang-cl.exe`    |

## argumentRemovals

| key      | value |
| -------- | ----- |
| `-o`     | 1     |
| `-MF`    | 1     |
| `-MT`    | 1     |
| `-MQ`    | 1     |
| `-c`     | 0     |
| `-MD`    | 0     |
| `-MMD`   | 0     |
