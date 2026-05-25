# Ubiquitous Language

## Heroband Moral Design

| Term | Definition | Aliases to avoid |
| ---- | ---------- | ---------------- |
| **Heroband** | A morally heroic fork of Angband 4.2 where player power must come from clean heroic sources. | Sanitized Angband |
| **Central Design Rule** | The rule that the player may fight evil, but may not wield evil. | Theme, vibe |
| **Player-Accessible Evil Power** | Any player-usable benefit derived from demons, devils, evil spirits, necromancy, soul pacts, blood magic, dark rituals, forbidden occultism, or corrupt shadow/dark power. | Dark option, evil-flavored ability |
| **Enemy-Only Evil Content** | Evil monsters, lore, curses, corruption, or antagonistic forces that the player opposes and cannot wield as beneficial power. | Evil reference, bad content |
| **Moral Restriction** | A gameplay or content rule that prevents playable characters from using forbidden power sources. | Content filter, flavor pass |
| **Clean Heroic Power** | Player power sourced from natural skill, courage, discipline, craft, lawful authority, healing, light, faith, music, tactics, nature, or heroic resolve. | Reskinned dark power |
| **Replacement Class** | A new playable class that preserves a gameplay niche while replacing forbidden power sources with clean heroic mechanics. | Rename, reflavor |

## Testing And Playtest

| Term | Definition | Aliases to avoid |
| ---- | ---------- | ---------------- |
| **Deterministic Test** | An automated validation path with repeatable pass/fail output, such as a C unit test or `-mtest` scripted run. | Script, smoke test |
| **Angband Test Session** | A live or scripted Angband process that an agent drives and inspects. | Run, launch |
| **Direct Gameplay Pass** | A tmux-driven GCU session where the agent plays the terminal game enough to validate behavior end to end. | Manual test, UI poke |
| **Test Contract** | The invariant, expected correct behavior, step-by-step procedure, and evidence plan declared before direct gameplay starts. | Test plan, notes |
| **Invariant** | The specific behavior that must remain true during a test. | Requirement, assertion |
| **Evidence Plan** | The planned captures, messages, or screen states that will prove whether the invariant held. | Logging, screenshots |
| **Feedback Loop** | The cycle of building, launching, driving a scenario, observing behavior, diagnosing, patching, and repeating. | Test cycle, iteration |
| **Promotion Rule** | The rule that repeatable direct-play discoveries should become deterministic tests when practical. | Follow-up, automation idea |

## Relationships

- A **Direct Gameplay Pass** must begin with a **Test Contract**.
- A **Test Contract** contains exactly one or more **Invariants** and one **Evidence Plan**.
- A **Feedback Loop** can include both **Deterministic Tests** and a **Direct Gameplay Pass**.
- **Player-Accessible Evil Power** violates the **Central Design Rule**.
- **Enemy-Only Evil Content** can remain when it is clearly antagonistic.
- A **Replacement Class** must use **Clean Heroic Power**, not a renamed forbidden mechanic.
- The **Promotion Rule** turns durable findings from an **Angband Test Session** into **Deterministic Tests** when the test frontend or unit-test layer can express them.

## Example Dialogue

> **Dev:** "This patch removes a class from birth. Is an `alltests` run enough?"
>
> **Domain expert:** "Run the **Deterministic Tests**, then do a **Direct Gameplay Pass** because the player-facing birth flow is involved."
>
> **Dev:** "What should the **Test Contract** say?"
>
> **Domain expert:** "State the **Invariant**: no **Player-Accessible Evil Power** is selectable during birth. Then list the class-screen steps and the **Evidence Plan** for captured screens."
>
> **Dev:** "If the tmux run exposes a regression, do we leave it as a transcript?"
>
> **Domain expert:** "Only if automation is not practical. Apply the **Promotion Rule** and add a **Deterministic Test** when `-mtest` or a unit test can cover it."

## Flagged Ambiguities

- "Evil content" is too broad: use **Player-Accessible Evil Power** for forbidden player benefits and **Enemy-Only Evil Content** for allowed antagonistic material.
- "Manual test" is vague: use **Direct Gameplay Pass** when an agent drives the terminal game through tmux with a declared **Test Contract**.
- "Reflavor" can imply a forbidden mechanic with new text: use **Replacement Class** only when the source and mechanic become genuinely clean.
