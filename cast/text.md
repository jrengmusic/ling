## diagnostics

```
@brief ling message texts, emitted verbatim when a run fails.
```

+--------------------------+-------------------------------------------------------------------+-----------------------------------------------------------------------+
| name                     | value                                                             | description                                                           |
+==========================+===================================================================+=======================================================================+
| failNotFound             | `not found`                                                       | A file argument that does not exist.                                  |
| failFlagUnknown          | `unknown flag`                                                    | Not --help or --version, or a rejected argument after the manifest.   |
| failToolchainArgument    | `toolchain argument not declared`                                 | A --\<word\> CLI argument matched no toolchain row's argument column. |
| failToolchainColumn      | `toolchain column not declared`                                   | A ## toolchain table's header row declares no command or flag column. |
| failDaemonStart          | `daemon did not start`                                            | A daemon that did not start.                                          |
| failDaemonLost           | `daemon connection lost`                                          | A connection to the daemon that ended before the result.              |
| failDaemonTimeout        | `daemon timeout must be a positive integer`                       | A ## daemon timeout cell is not a positive integer.                   |
| failDaemonThreads        | `daemon threads must be a positive integer`                       | A ## daemon threads cell is not a positive integer.                   |
| failSwapVersion          | `swap of another version: remove the swap folder and build again` | A swap file that another version of LING wrote.                       |
| failSwapRecord           | `swap record incomplete`                                          | A swap file whose root or record misses a field.                      |
| failLanguageLoad         | `language library did not load`                                   | A language library that does not open or has no entry.                |
| failLanguageParse        | `file did not parse`                                              | A file that its language did not parse.                               |
| failUnitIndex            | `unit index failed`                                               | A unit whose index call returned nonzero.                             |
| failConstructionNotFound | `no language folder holds a construction file`                    | No folder of the language folder holds construction.md.               |
+--------------------------+-------------------------------------------------------------------+-----------------------------------------------------------------------+

## codeAction

```
@brief ling code action titles.
```

+------------+----------------+-------------+
| name       | value          | description |
+============+================+=============+
| fixItTitle | `apply fix-it` |             |
+------------+----------------+-------------+

## progress

```
@brief ling work done progress texts of the index.
```

+-------------------+--------------+-------------+
| name              | value        | description |
+===================+==============+=============+
| progressToken     | `ling/index` |             |
| progressTitle     | `Indexing`   |             |
| progressSeparator | `/`          |             |
+-------------------+--------------+-------------+
