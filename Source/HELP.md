# LING: Language Index Navigation Graph

**Version 0.1.0**

You see this guide because you ran `ling` with no argument or `ling --help`.

## Command Line

| Command               | Result                                                                                      | Exit |
| :-------------------- | :------------------------------------------------------------------------------------------ | :--- |
| `ling`                | Prints this guide.                                                                          | 0    |
| `ling --help`         | Prints this guide.                                                                          | 0    |
| `ling --version`      | Prints `ling 0.1.0 (<commit>)`.                                                             | 0    |
| `ling <file>`         | Prints one row per declaration of a C family file: `<path>:<line>:<column>: <type> <name>`. | 0    |
| `ling <file>`         | When the tree fails a rule, prints `ling: <file>: <failure>` on stderr.                     | 1    |
| `ling <missing>`      | Prints this guide, then `ling: <missing>: not found` on stderr.                             | 1    |
| `ling <file> <other>` | Prints `ling: <other>: unknown flag` on stderr.                                             | 1    |

On a terminal, ling clears the screen first.
