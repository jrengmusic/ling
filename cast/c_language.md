## cLanguage

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
