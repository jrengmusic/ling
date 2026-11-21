# MACHINIST Report — ling LSP Test (build 5096c02)

Date: 2026-11-21  
Binary: `ling 0.0.1 (5096c02)`, built 2026-11-21 01:22:23  
Target project: END (`~/Documents/Poems/dev/end/`), word `--debug`  
Test method: headless LSP over stdio (Python DAP-style client, no nvim)

## Results

| Method                      | Result | Time   | Detail                                                              |
| --------------------------- | ------ | ------ | ------------------------------------------------------------------- |
| `initialize`                | OK     | —      | Server name: ling. All capabilities: hover, definition, typeDefinition, documentHighlight, documentSymbol |
| `textDocument/hover`        | OK     | 0.0 s  | `field commandManager` — type, doxygen brief, declaration, source file attribution |
| `textDocument/definition`   | OK     | 0.3 s  | `ENDView.h` line 98                                                 |
| `switchSourceHeader`        | OK     | 0.0 s  | `ENDView.h`                                                         |
| `documentHighlight`         | OK     | 0.0 s  | 7 highlights                                                        |
| `documentSymbol`            | OK     | 0.0 s  | 32 symbols. First: `ENDView::ENDView`, `~ENDView`, `resized`, `keyPressed`, `valueTreePropertyChanged` |
| `shutdown` / `exit`         | OK     | —      | Exit code 0                                                         |

## Definition — fixed

The RFC reported definition at `ENDView.cpp:41:6` returning `{}` on the previous binary (70.5 s timeout). This build returns `ENDView.h` line 98 in 0.3 s.

## CPU

| Metric               | Previous binary (RFC) | This build (5096c02) |
| --------------------- | --------------------- | -------------------- |
| Cold index CPU time   | 7:48                  | 8.18 s               |
| Cold index peak CPU % | 837%                  | not measured (daemon at 0.2% by the time of check — indexing completed within the 3 s wait) |
| Daemon RSS            | —                     | 3.9 GB               |

The 8.18 s CPU vs. the RFC's 7:48 is not comparable: the previous test ran an older binary without the `isForeign` funnels (the ling sprint of 2026-11-20). This build holds that sprint — the engine skips records, hover, outline, completion, hint, diagnostic and fix-it for foreign files, which is why the cold index finishes in seconds of CPU instead of minutes.

## Open

1. No word test: `ling project-info.md` with no word was not retested. The RFC reported `{}` for every method on END (Release-only default flow, no `Builds/Debug`). This is an END manifest issue, not a ling defect.
2. CPU measurement under sustained load (T21 from the tracker) is not covered by this test.
