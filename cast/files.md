## index

+---------+--------------+
| alias   | symbol       |
+=========+==============+
| @string | juce::String |
+---------+--------------+

## files

```
@brief File, folder, and variable names that ling reads or writes, such as the help and the language files.
```

+----------------------------+---------+-------------------------------+-------------------------------------------------------------------------------------+
| name                       | type    | value                         | description                                                                         |
+============================+=========+===============================+=====================================================================================+
| archiveExtension           | @string | `zip`                         | Extension of the language archives in the binary data.                              |
| castJuceCache              | @string | `.cache/cast/juce`            | Cache folder of the patched JUCE of cast below the home folder.                     |
| castJuceLocalCache         | @string | `cast/juce`                   | Cache folder of the patched JUCE of cast in %LOCALAPPDATA%.                         |
| core                       | @string | `core.md`                     | Character and token table file of a language build.                                 |
| defaultLanguage            | @string | `cLang`                       | Name of the language whose files LING embeds.                                       |
| language                   | @string | `language`                    | Folder of the language folders, below the config folder and below the cache folder. |
| languageConstruction       | @string | `construction.md`             | Construction table file of a language.                                              |
| languageCore               | @string | `core`                        | Folder of the carried library in the language archives.                             |
| languageCoreHeader         | @string | `ling_LanguageCore.h`         | Header of the carried library of a language build.                                  |
| languageCoreLibrary        | @string | `build/liblanguageCore.a`     | Static library that the carried library build writes, below its folder.             |
| languageCoreLibraryWindows | @string | `build/languageCore.lib`      | Static library that the carried library build writes on Windows, below its folder.  |
| languageInterface          | @string | `jam_LanguageInterface.h`     | Boundary header of a language library.                                              |
| languageLibrary            | @string | `build/liblanguage.dylib`     | Library that a language build writes, below its cache folder.                       |
| languageLibraryWindows     | @string | `build/language.dll`          | Library that a language build writes on Windows, below its cache folder.            |
| languageSpell              | @string | `spell.md`                    | Spell file of a language.                                                           |
| languageTemplate           | @string | `language.cast`               | Template file of a language.                                                        |
| lingCache                  | @string | `.cache/ling`                 | Cache folder of LING below the home folder or a repository root.                    |
| lingConfig                 | @string | `.config/ling`                | Config folder of LING below the home folder.                                        |
| lingHelp                   | @string | `HELP.frame`                  | Rendered help text.                                                                 |
| lingLocalCache             | @string | `ling`                        | Cache folder of LING in %LOCALAPPDATA%.                                             |
| lingPort                   | @string | `port`                        | Extension of the port file of a daemon.                                             |
| lingSwap                   | @string | `swap`                        | Extension of the swap file of a source file.                                        |
| lingSymbols                | @string | `symbols`                     | Extension of the symbol swap file of a unit.                                        |
| lingTools                  | @string | `tools.json`                  | MCP tool list.                                                                      |
| lsp                        | @string | `lsp.md`                      | LSP value table file of a language build.                                           |
| screenErase                | @string | `erase.frame`                 | Screen erase sequence.                                                              |
| sourceDirectoryVariable    | @string | `${CMAKE_CURRENT_SOURCE_DIR}` | The CMake variable that a manifest cell uses for the project folder.                |
| syntaxTree                 | @string | `syntax_tree.md`              | Syntax table file of a language build.                                              |
| userModulePathVariable     | @string | `${CAST_USER_MODULE_PATH}`    | The CMake variable that a manifest cell uses for the user module folder.            |
| userModulesInfo            | @string | `user-modules-info.md`        | File name of the module table in the user module folder.                            |
+----------------------------+---------+-------------------------------+-------------------------------------------------------------------------------------+
