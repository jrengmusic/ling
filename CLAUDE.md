# ling

**Identity**

- Name: ling
- Type: language-agnostic LSP server, indexer daemon, and CLI tool
- Stack: C++17 / JUCE 8 / JAM
- Root path: /Users/jreng/Documents/Poems/dev/ling
- Version: 0.0.1

**Current state**

- Last sprint: Sprint 18 (2026-11-22) — universal MCP content + structuredContent response contract, explicit ToolRelay noPort error, Windows ChildProcess breakaway fallback, Query::applyToImplementations root-parent fix, audit and doxygen pass.
- Active debt: none (`DEBT.md` is empty).

**Layout**

- `Source/`: daemon (`Server`, `Processor`, `Index`, `Query`, `SymbolDocument`, `Manifest`), stdio relay (`Relay`, `ToolRelay`, `Client`, `Operation`), built-in languages (`BuiltInLanguage`, `languages/`), generated tables (`generated/`), `HELP.md`, and `tools.json`.
- `language/`: carried language extension sources (`cLang/`, `core/`).
- `cast/`: cast codegen tables and `spell.md`.

**Key documents**

| Document | Purpose |
|---|---|
| `README.md` | States what LING is. Records the name, its reason, and the name collision. |
| `Source/HELP.md` | Command line, LSP methods, manifest contract, and language library guide. |
| `carol/SPRINT-LOG.md` | Sprint log (through Sprint 18). |
| `DEBT.md` | Inter-sprint debt ledger. |

**API docs**

- Doxygen XML: `docs/xml/` and `docs/tagfile.xml`.

**Build**

- Built via `cast cast/spell.md` (default Release, `--debug`, `--no-sign`, defined in `project-info.md` `## toolchain`).
