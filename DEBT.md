# DEBT.md

**Purpose:** Inter-sprint ledger of debts — bugs, nitpicks, friction observed during usage. Drained by sprints via `/pay` (COUNSELOR planning) and `/log` (hygiene drain). **JRENG = paid in full, cash. No triage.**

**Format:** Each entry uses **O / D / E** articulation — Observation, Divergence, Expectation. IDs are UTC timestamps (`DEBT-YYYYMMDDTHHMMSS`). Newest entries at top. Add via `carol debt add`.

**Lifecycle:** Created lazily on first `carol debt add`. Entries appended via interactive prompt. Entries removed by `carol debt clear <id>` (called by `/log` hygiene step after SPRINT-LOG receipt is written). Survives `carol reset` — debts persist across protocol resets.

---

## DEBT-20261121T005809

**Observation:** ARCHITECT, 2026-11-21: "ling now eats memory and cpu like crazy again, my fan is hot". The ling-project daemon of the 07:52:41 build (pid 76765, started 07:54:28) went from 371.9% to 251.3% CPU, and RSS from 2477024 KB to 3710704 KB, in two ps reads 4 s apart. After it was stopped, the nvim relay started pid 77595 at 07:57:35, at 197.3% CPU and 1057040 KB RSS. daemon.log shows addDocument and setIndexed lines for SDK libc++ headers, for example /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1/__concepts/semiregular.h.
**Divergence:** ARCHITECT: "why cold index now hot again". The cause is not read yet. Earlier samples in this sprint put the index threads in addContextPass (a full parse without the PCH for each platform pass, Step 15a). An earlier run reached about 238k setIndexed lines in about 4 minutes.
**Expectation:** ARCHITECT: "no redundancy no copy no rebuild no reparse 0 rules". PLAN-ling-contract-fix.md Step 16: T9, T10 and T21 are all below clangd.

---
