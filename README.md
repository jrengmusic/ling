# LING

**Language Index Navigation Graph**

LING reads a project's build manifest and writes a shared index of its code: symbols, occurrences, and relations. A reader answers queries from that index. nvim, agents, and the documentation output all use the same reader.

Status: design stage. The rulings and the plan are in `PLAN-ling-0-tracker.md` and its eleven parts, `PLAN-ling-1-project.md` to `PLAN-ling-11-swap.md`.

The long form of the name is "Language Index Navigation Graph" (`PLAN-ling-0-tracker.md`, ruling 24).

---

## The name

### Reason

ARCHITECT's reason: "means origin, short for lingua."

### Fact check

A source checked each claim about the word. Etymonline and Merriam-Webster returned HTTP 403. The OED was not reached. No primary text was read for the Sanskrit and Chinese claims.

| Claim | Status | Source |
|---|---|---|
| Latin *lingua* means "tongue" (the organ) and "language" | Verified | Wiktionary `lingua` |
| English "language" comes from *lingua*, through Old French *language* | Verified | Wiktionary `language` |
| "linguist" comes from *lingua* | Unverified | No source fetched |
| "lingual" comes from *lingua* | Partial | Wiktionary lists it as a related term only |
| Sanskrit *liṅga* means "mark", "sign", "emblem", "characteristic" | Verified | Wikipedia `Linga`, which cites Monier-Williams |
| *liṅga* also names grammatical gender, the sign of an inference, a symptom, and the subtle body | Verified | Wikipedia `Linga` |
| *liṅga* also names the Shaiva aniconic emblem | Verified | Wikipedia `Linga` |
| The Shaiva emblem has a phallic reading | Contested | Doniger and Dasgupta support it. Winternitz and Sivananda reject it. |
| 灵 (靈) shows a shaman who prays for rain, and its first sense is ritual efficacy | Contested, unverified | Wiktionary `靈`: phonetic 霝 and semantic 玉 "jade". Three oracle-bone readings compete. No source states the efficacy sense. |
| A classical text defines 灵 as "空而明者" | Unverified | No source found. The nearest attested phrase is 虛靈不昧. |
| English "-ling" and the fish "ling" come from PIE *lenk-* "to bend" | Contradicted | Wiktionary `-ling`: Proto-Germanic *-lingaz*. Wiktionary `ling`: the fish is probably related to "long". |

### What the name uses

- *lingua* ("tongue", "language"): it matches the "Language" in the long form.
- *liṅga* ("mark", "sign"): it fits an index of symbol marks. The same word names a religious emblem, and the name inherits that association.

The Chinese and Germanic-root claims are not used. They are contested, unverified, or contradicted.

### Collision

Cloudozer's LING (formerly Erlang on Xen) uses the same name. It is an Erlang runtime that runs as a unikernel on Xen. Its maintenance state is unclear: a 2020 Elixir Forum post calls it inactive, and no recent commit date was read. The check ran two queries and found no code-intelligence tool named Ling. It is not exhaustive.

On the development machine, `ling` is not on PATH and Homebrew has no formula `ling`.
