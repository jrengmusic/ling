# ling

**Identity**

- Name: ling
- Type: design-stage project. No code exists yet.
- Root path: /Users/jreng/Documents/Poems/dev/ling

**Purpose**

LING reads a project's build manifest. It writes a shared index of that code: symbols, occurrences, and relations. A reader answers queries from the index. nvim, agents, and the documentation output use the same reader. The plan is in `PLAN-ling-0-tracker.md` and its eleven parts.

**Contents**

- `carol/`: CAROL contract files. Most are symlinks to `~/.carol/`. Also holds `SPRINT-LOG.md` and `config.yml`.
- `.claude/`: harness configuration.

**Key documents**

| Document | Purpose |
|---|---|
| `README.md` | States what LING is. Records the name, its reason, and the name collision. |
| `PLAN-ling-0-tracker.md` | Rulings, the contract of every part, the status of each part, and the risks. |
| `PLAN-ling-1-project.md` to `PLAN-ling-11-swap.md` | The eleven parts of the plan. |
| `carol/SPRINT-LOG.md` | Sprint log. No sprint entries exist yet. |
