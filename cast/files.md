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

+---------+-------------------------+-------------------------------+-------------------------------------------------------------------------------------+
| type    | name                    | value                         | description                                                                         |
+=========+=========================+===============================+=====================================================================================+
| @string | castJuceCache           | `.cache/cast/juce`            | Cache folder of the patched JUCE of cast below the home folder.                     |
| @string | castJuceLocalCache      | `cast/juce`                   | Cache folder of the patched JUCE of cast in %LOCALAPPDATA%.                         |
| @string | core                    | `core.md`                     | Character and token table file of a language build.                                 |
| @string | defaultLanguage         | `cLang`                       | Name of the language whose files LING embeds.                                       |
| @string | language                | `language`                    | Folder of the language folders, below the config folder and below the cache folder. |
| @string | languageConstruction    | `construction.md`             | Construction table file of a language.                                              |
| @string | languageInterface       | `jam_LanguageInterface.h`     | Boundary header of a language library.                                              |
| @string | languageLibrary         | `build/liblanguage.dylib`     | Library that a language build writes, below its cache folder.                       |
| @string | languageLibraryWindows  | `build/language.dll`          | Library that a language build writes on Windows, below its cache folder.            |
| @string | languageSpell           | `spell.md`                    | Spell file of a language.                                                           |
| @string | languageTemplate        | `language.cast`               | Template file of a language.                                                        |
| @string | lingCache               | `.cache/ling`                 | Cache folder of LING below the home folder or a repository root.                    |
| @string | lingConfig              | `.config/ling`                | Config folder of LING below the home folder.                                        |
| @string | lingHelp                | `HELP.md`                     | Rendered help text.                                                                 |
| @string | lingLocalCache          | `ling`                        | Cache folder of LING in %LOCALAPPDATA%.                                             |
| @string | lingPort                | `port`                        | Extension of the port file of a daemon.                                             |
| @string | lingSwap                | `swap`                        | Extension of the swap file of a source file.                                        |
| @string | sourceDirectoryVariable | `${CMAKE_CURRENT_SOURCE_DIR}` | The CMake variable that a manifest cell uses for the project folder.                |
| @string | syntaxTree              | `syntax_tree.md`              | Syntax table file of a language build.                                              |
| @string | userModulesInfo         | `user-modules-info.md`        | File name of the module table in the user module folder.                            |
+---------+-------------------------+-------------------------------+-------------------------------------------------------------------------------------+
