# ling

**Identity**

- Name: ling
- Type: design-stage project. No code exists yet.
- Root path: /Users/jreng/Documents/Poems/dev/ling

**Purpose**

LING reads a project's build manifest. It writes a shared index of that code: symbols, occurrences, and relations. A reader answers queries from the index. nvim, agents, and the documentation output use the same reader. The plan is in `PLAN-ling-lsp-0-tracker.md`. `PLAN-ling-0-tracker.md` holds canon rulings 1-81.

**Contents**

- `carol/`: CAROL contract files. Most are symlinks to `~/.carol/`. Also holds `SPRINT-LOG.md` and `config.yml`.
- `.claude/`: harness configuration.

**Key documents**

| Document | Purpose |
|---|---|
| `README.md` | States what LING is. Records the name, its reason, and the name collision. |
| `PLAN-ling-0-tracker.md` | Canon rulings 1-81. |
| `PLAN-ling-lsp-0-tracker.md` | The one plan: rulings, status, tests, open items, steps. |
| `carol/SPRINT-LOG.md` | Sprint log. No sprint entries exist yet. |
