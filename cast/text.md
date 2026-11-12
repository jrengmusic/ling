## diagnostics

```
@brief ling message texts, emitted verbatim when a run fails.
```

+------------------------------+-----------------------------------+-----------------------------------------------------------------------+
| name                         | value                             | description                                                           |
+==============================+===================================+=======================================================================+
| failNotFound                 | `not found`                       | A file argument that does not exist.                                  |
| failFlagUnknown              | `unknown flag`                    | Not --help or --version, or a rejected argument after the manifest.   |
| failUnitNotFound             | `unit file not found`             | A unit file of a compiler manifest that does not exist.               |
| failCompilerManifestInvalid  | `compiler manifest is not valid`  | A compiler manifest whose root is not an array.                       |
| failCompilerManifestNotFound | `compiler manifest not found`     | A compiler manifest that does not exist in its build folder.          |
| failDependenciesNotFound     | `dependencies not found`          | A unit with no record in the dependency record.                       |
| failToolchain                | `toolchain command failed`        | A toolchain row's process could not start or exited nonzero.          |
| failToolchainArgument        | `toolchain argument not declared` | A --\<word\> CLI argument matched no toolchain row's argument column. |
| failToolchainColumn          | `toolchain column not declared`   | A ## toolchain table's header row declares no command or flag column. |
+------------------------------+-----------------------------------+-----------------------------------------------------------------------+
