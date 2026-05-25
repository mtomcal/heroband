---
name: heroband-test-quality-verifier
description: Repo-only Heroband workflow for auditing gameplay, class-power, birth, store, save/load, scenario-save, and moral-restriction tests for reward-hacking, weak assertions, and missing player-visible coverage.
metadata:
  short-description: Verify Heroband gameplay test quality
---

# Heroband Test Quality Verifier

Use this repo-local skill when adding or changing tests for Heroband gameplay, class powers, birth flow, stores, inventory, spell/order lists, save/load behavior, scenario saves, GCU playtests, or moral restrictions.

Heroband's central rule is: the player may fight evil, but may not wield evil. Test quality for this repo must check both behavior and moral accessibility.

## Required Flow

1. Identify the behavior under test and the public gameplay path that should prove it.
2. Inspect the new or changed tests, fixtures, and any playtest contract.
3. Classify setup mutations as legitimate prerequisites or reward-hacking risks.
4. Check whether the test would fail if the forbidden behavior or old content returned.
5. Run the smallest targeted test command, then the relevant broader gate when practical.
6. For scenario-save or GCU evidence, run `scripts/heroband-playtest
   validate-evidence --state-dir <evidence-root>` before judging transcripts.
7. Produce a structured report with pass/fail, gaps, and concrete fixes.

## What Counts As Good Evidence

Prefer tests that exercise real game paths:

- `player_make_simple()` or test-frontend birth for class availability
- `spell_cast()` and effect handling for class powers/orders
- store parser/runtime paths for store availability
- save/load or generated scenario saves for deep gameplay state
- GCU/tmux captures for player-visible menus, prompts, and play feel
- moral audits that combine data/text checks with player-access checks

Data-string checks are useful but not sufficient by themselves when a mechanic can remain player-accessible through another path.

## Reward-Hacking Checks

For each test, ask:

- Does setup mutate only prerequisites, or does it also force the expected result?
- Would the test fail if a forbidden player power such as bloodlust, shadow, nether, life drain, curse-benefit, demonic power, necromancy, or occult pact became player-accessible again?
- Would the test fail if the player-facing name stayed clean but the underlying evil mechanic returned?
- Does the test exercise actual command/effect/save/store/birth behavior rather than only asserting internal flags?
- Are assertions specific enough to catch regressions, or are they broad truthy checks?
- Does random failure chance make the test flaky? If retries are needed, is the reason explicit?
- Does a high-level or deep-floor test have GCU or scenario-save evidence when player-visible behavior matters?

## Heroband Moral Test Checklist

For playable classes and powers:

- Search player-facing class, spell/order, help, docs, birth, store, and customization text for old forbidden names.
- Confirm forbidden shared systems remain enemy-only or inaccessible to the player.
- Confirm class books, stores, drops, and birth flow do not reintroduce player access.
- Confirm replacements use clean power sources such as courage, discipline, tactics, armor mastery, endurance, mercy, light, healing, music, nature, or lawful command.

For enemy-only evil content:

- Do not demand removal merely because a string sounds evil.
- Verify the content is antagonistic or ambient, not a player-beneficial option.

## Scenario Save Review

For generated scenario saves, verify the manifest records:

- race, class, level, stats, hit points, mana, equipment, inventory, learned powers, and gold
- dungeon depth, seed if any, starting position, terrain, and monsters
- exact invariant and expected pressure
- save path, executable/build, and launch command
- what GCU actions and captures prove the scenario

Run the evidence-root validator whenever a playtest evidence root is available:

```sh
scripts/heroband-playtest validate-evidence --state-dir <evidence-root>
```

The verifier report must distinguish GCU-visible claims, such as menus,
messages, status flags, loading, and screen state, from deterministic assertions,
such as exact radius membership, actor counts, no drops, no teleport, stale
timer cleanup, unique resistance, and save/load fields.

If a deterministic unit scenario is used because no save generator exists, report that honestly as a fallback, not a full replacement for loaded-save GCU testing.

## Report Format

Use this structure:

```md
## Heroband Test Quality Report

Scope:
- Files reviewed:
- Behavior under test:
- Commands run:

Findings:
- PASS/FAIL: <specific issue or strength>

Reward-hacking review:
- Setup mutations:
- Behavior path exercised:
- Would fail if forbidden access returned:

Moral-access review:
- Player-accessible evil power:
- Enemy-only evil content affected:

Gaps:
- <missing coverage or none>

Verdict:
- PASS | PASS WITH GAPS | FAIL
```

## Common Fixes

- Replace name-only assertions with an access-path test.
- Add a negative assertion for stale forbidden class/power access.
- Exercise `spell_cast()` or the relevant command path instead of setting the final timed flag directly.
- Move unavoidable high-level setup into a named helper and keep assertions on behavior.
- Add GCU captures or a generated scenario save when menus, prompts, or play feel matter.
- Split one broad scenario into focused tests when a failure would not identify the broken behavior.
