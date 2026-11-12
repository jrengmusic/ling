# LING: Language Index Navigation Graph

**Version 0.1.0**

You see this guide because you ran `ling` with no argument or `ling --help`.

## Command Line

| Command                           | Result                                                                                                                                                                                                                                                                                                 | Exit |
| :-------------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :--- |
| `ling`                            | Prints this guide.                                                                                                                                                                                                                                                                                     | 0    |
| `ling --help`                     | Prints this guide.                                                                                                                                                                                                                                                                                     | 0    |
| `ling --version`                  | Prints `ling 0.1.0 (<commit>)`.                                                                                                                                                                                                                                                                        | 0    |
| `ling <project-info.md>`          | Reads the manifest. For each language family, reads the compiler manifest that the default-flow `## toolchain` rows give, then parses each file of each unit. Prints one row per document in path order, `<path>: <declarations> declarations, <references> references`, then `<documents> documents`. | 0    |
| `ling <project-info.md> --<word>` | The same, with the `## toolchain` rows whose `argument` cell is `<word>`, as cast takes it.                                                                                                                                                                                                            | 0    |
| `ling <project-info.md>`          | Each failure prints `ling: <project-info.md>: <failure>` on stderr. The rows still print.                                                                                                                                                                                                              | 0    |
| `ling <missing>`                  | Prints this guide, then `ling: <missing>: not found` on stderr.                                                                                                                                                                                                                                        | 1    |
| `ling <project-info.md> <other>`  | Prints `ling: <other>: unknown flag` on stderr.                                                                                                                                                                                                                                                        | 1    |

For the C family, LING reads `compile_commands.json` in the build folder that the `cmake` row names (`-B <folder>`), and the files of each unit from the record that `ninja -C <folder> -t deps` prints for the `ninja` row.

On a terminal, ling clears the screen first.
