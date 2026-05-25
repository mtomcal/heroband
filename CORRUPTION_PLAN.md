# Corruption Mechanics Implementation Plan
## Player-accessible evil power and corrupt artifact policy

> **Status: CORE CORRUPTION POLICY IMPLEMENTED** - 12 grill decisions resolved. Slices 1-10 are implemented and tested: corrupt item classification, equip/activation warnings, persistent character-bound corruption on confirmed corrupt use, hostile corruption thresholds, forbidden player-power removals, shadow-book access gates, scroll policy, and deep-corruption victory failure. Documentation and scenario-save playtest validation remain.

## Implementation Progress

- [x] Slice 1: Corrupt Item Classification
- [x] Slice 2: Warning Before Equipping Corrupt Gear
- [x] Slice 3: Warning Before Activating Corrupt Power
- [x] Slice 4: First Corruption State Consequence
- [x] Slice 5: Escalating Corruption Effects
- [x] Slice 6: Convert Player Drain Life Devices
- [x] Slice 7: Remove Vampiric and Bloodlust Player Power
- [x] Slice 8: Shadow Player Infrastructure Access Gate
- [x] Slice 9: Summon Undead and Curse Scroll Policy
- [x] Slice 10: Heroic Victory Gate and Corruption Failure Ending
- [x] Slice 11: Docs and Ubiquitous Language

## Overview

Implement a Heroband corruption policy that preserves evil as an antagonistic force while preventing playable characters from wielding evil as normal power. Corrupt artifacts may tempt the player with short-term benefit, but their use must be hostile, escalating, and capable of ruining the character. Ordinary forbidden player powers such as life drain, vampire form, bloodlust, and player shadow infrastructure should be removed, replaced, or converted into explicitly corrupt self-destructive mechanics.

## Decisions

| # | Question | Decision | Source |
|---|---|---|---|
| Q1 | What happens to iconic corrupt artifacts? | Keep them as corrupt artifacts or trophy/lore objects; prevent safe normal use. | Grill session |
| Q2 | Can corrupt artifacts offer short-term power? | Yes, if consequences are severe, escalating, and clearly hostile. | Grill session |
| Q3 | What separates corrupt temptation from forbidden player power? | Corrupt power must not be sustainable, optimal, class-defining, or framed as heroic. | Grill session |
| Q4 | Is corruption item-bound or character-bound? | Both: item-bound by default, character-bound for major evil or willful use. | Grill session |
| Q5 | Is corruption curable? | Three tiers: ordinary curable curses, costly heroic cleansing, and irreversible deep corruption. | Grill session |
| Q6 | Which existing powers convert rather than disappear? | Convert One Ring, Morgoth artifacts, Morgul weapons, and maybe Drain Life/Summon Undead. Remove bloodlust, vampirism, and shadow player infrastructure. | Grill session |
| Q7 | Is Drain Life a source or mechanic problem? | Both for player use; enemy use remains allowed. Corrupt artifact use may remain with corruption cost. | Grill session |
| Q8 | What happens to vampire and werewolf forms? | Remove vampire; replace werewolf with a clean nature/melee form. | Grill session |
| Q9 | Can players equip cursed objects? | Ordinary cursed gear remains permitted risk; corrupt gear is morally discouraged by rules and warnings. | Grill session |
| Q10 | Can corruption create unique failure endings? | Yes, rare and earned through clear warnings and escalating choices. | Grill session |
| Q11 | Can corrupt power support heroic victory? | No. Heroic victory requires rejecting, destroying, sealing, or resisting corrupt power. | Grill session |
| Q12 | What is the first implementation slice? | Add corrupt item classification and warnings/tests before broader rewrites. | Grill session |

## Current Code State

### Already correct

- Enemy-only evil content exists throughout `lib/gamedata/monster.txt`, `lib/gamedata/monster_spell.txt`, traps, curses, and lore. This can remain when antagonistic.
- The object curse system already supports hostile item effects, sticky removal, aggravation, impaired recovery, summoning, vulnerabilities, and drain flags.
- `BLACKBREATH`, stat drain, experience drain, curses, nether, darkness, demons, undead, and Morgoth are already hostile pressures.
- Clean opposition powers already exist: slay evil/demon/undead, protection from evil, light, holy damage, remove curse, healing, restoration, and heroic class powers.

### Out of alignment

- `lib/gamedata/artifact.txt` exposes the One Ring, Morgoth crown, and Hammer of the Underworld as powerful wearable artifacts with large benefits.
- `lib/gamedata/ego_item.txt` exposes `of Morgul` weapons with positive combat properties and evil-coded benefits.
- Player-usable life drain exists through `Drain Life` wands/rods and artifact activations in `lib/gamedata/object.txt` and `lib/gamedata/activation.txt`.
- `lib/gamedata/player_timed.txt` still defines `ATT_VAMP` and `BLOODLUST` as beneficial player timed effects.
- `lib/gamedata/shape.txt` still includes `vampire` and `werewolf` player forms.
- Shadow realm/book infrastructure and monster shadow-book drops still exist even though shadow power should not be player-facing.
- `Summon Undead` and `Curse Weapon` / `Curse Armour` scrolls remain player-usable object definitions.

### Important constraints

Do not delete shared evil mechanics blindly. Monsters, traps, curses, artifacts, save compatibility, Borg code, object knowledge, tiles, and tests may still reference old tvals, projections, timed effects, and internal identifiers. Prefer data classification, access gates, warnings, and targeted replacement before deleting parser-visible entries or enum-backed mechanics.

## Intended Implementation Shape

Start with a narrow corrupt-item foundation: classify corrupt objects, warn before wielding or activating them, and test that the most important corrupt artifacts are no longer treated as safe power. Then convert direct player evil powers into either clean replacements or corrupt mechanics. Keep enemy-only evil and hostile corruption effects intact. Each slice should be small enough to build and test independently.

## Red/Green TDD Slices

### Slice 1: Corrupt Item Classification

#### Red - Write tests first, no implementation code yet

- Test file: add `src/tests/object/corruption.c` and include it from the object test suite.
- What the test proves: the One Ring, Morgoth crown, Hammer of the Underworld, and Morgul ego weapons are classified as corrupt when loaded from game data.
- Assertion strategy: data-aware lookup through public object/artifact/ego data structures after test initialization, not string-only grep.
- Existing tests to rewrite: none.
- Follow-up mini-cycles:
  - First test: artifact lookup marks `The One Ring` corrupt.
  - Second test, after green: Morgoth crown and Hammer of the Underworld are corrupt.
  - Third test, after green: `of Morgul` ego is corrupt.

Run:

```sh
cmake --build build -t run-unittest-object-corruption -j2
```

Expected red: no corrupt classification exists yet.

**Hard gate: Do not proceed to Green until the first behavior test fails for the expected reason.**

#### Green - Make the red test pass, minimum change only

- Source files: likely `lib/gamedata/artifact.txt`, `lib/gamedata/ego_item.txt`, parser support in the relevant object/artifact/ego parser, and shared object metadata structs.
- What to change: add a small `corrupt` data marker or equivalent parsed flag for artifacts/egos/kinds, then mark only the agreed first set.
- Constraint: do not add corruption consequences yet; this slice only classifies.
- Decisions satisfied: Q1, Q9, Q12.
- Next mini-cycle: return to Red for each additional corrupt object family.

Run the targeted object corruption test after each mini-cycle.

#### Refactor - Clean up while keeping tests green

- Consolidate corrupt checks behind one public helper, for example `object_is_corrupt()` or the closest local naming pattern.
- Keep separate: warnings, activation behavior, and failure endings.

### Slice 2: Warning Before Equipping Corrupt Gear

#### Red - Write tests first, no implementation code yet

- Test file: add or extend `src/tests/player/inven-wield.c`.
- What the test proves: attempting to wield or wear a known corrupt item routes through a warning/confirmation path instead of normal silent equip.
- Assertion strategy: public inventory/wield command behavior or the narrowest existing equip API that reports confirmation requirements.
- Existing tests to rewrite: any test that assumes the One Ring or Morgul weapons equip as ordinary gear.
- Follow-up mini-cycles:
  - First test: corrupt wearable requires confirmation.
  - Second test, after green: non-corrupt cursed wearable follows the existing ordinary curse path.

Run:

```sh
cmake --build build -t run-unittest-player-inven-wield -j2
```

Expected red: corrupt gear is currently treated like ordinary equipment.

**Hard gate: observe the warning-path test fail before changing wield code.**

#### Green - Make the red test pass, minimum change only

- Source files: likely `src/player-util.c`, `src/obj-gear.c`, `src/cmd-obj.c`, or the UI command layer that handles wield confirmation.
- What to change: add a corrupt-equipment warning gate that is separate from ordinary curse handling.
- Constraint: do not block all equipping yet unless the red test requires it; this slice establishes warning and deliberate choice.
- Decisions satisfied: Q2, Q3, Q9.
- Next mini-cycle: return to Red for ordinary cursed gear distinction.

Run the targeted test and then the player test suite.

#### Refactor - Clean up while keeping tests green

- Keep warning text centralized so later activation and victory checks can reuse consistent language.

### Slice 3: Warning Before Activating Corrupt Power

#### Red - Write tests first, no implementation code yet

- Test file: add or extend an object activation test under `src/tests/object` if one exists; otherwise add a focused command/effect test for activation confirmation.
- What the test proves: activating a corrupt artifact with an evil activation requires explicit warning/confirmation.
- Assertion strategy: command/activation path result, not parser internals.
- Existing tests to rewrite: none unless existing activation tests assume silent activation.
- Follow-up mini-cycles:
  - First test: corrupt activation requires warning.
  - Second test, after green: clean artifact activation does not require corrupt warning.

Run the targeted test; if no target exists yet, add the new test target to `CMakeLists.txt` and run it.

Expected red: activation has no corrupt warning gate.

**Hard gate: observe the corrupt activation test fail before editing activation code.**

#### Green - Make the red test pass, minimum change only

- Source files: likely `src/obj-use.c`, `src/cmd-obj.c`, or the activation command handler.
- What to change: gate corrupt artifact activations through the same warning vocabulary as corrupt equipment.
- Constraint: do not implement escalating corruption yet.
- Decisions satisfied: Q2, Q3, Q9, Q12.
- Next mini-cycle: return to Red for clean activation behavior.

Run targeted activation/object tests.

#### Refactor - Clean up while keeping tests green

- Extract a small shared corrupt-use confirmation helper if both equip and activation gates duplicate logic.

### Slice 4: First Corruption State Consequence

#### Red - Write tests first, no implementation code yet

- Test file: add `src/tests/player/corruption.c`.
- What the test proves: confirmed use of a corrupt artifact increases a persistent character-bound corruption counter or status.
- Assertion strategy: player state after public corrupt-use pathway; verify state survives normal recalculation and is separate from ordinary curses.
- Existing tests to rewrite: none.
- Follow-up mini-cycles:
  - First test: one confirmed corrupt use increments corruption.
  - Second test, after green: ordinary cursed gear does not increment corruption.
  - Third test, after green: corruption persists through save/load if the repo has a practical save test harness.

Run:

```sh
cmake --build build -t run-unittest-player-corruption -j2
```

Expected red: no player corruption state exists.

**Hard gate: observe the state test fail before adding player fields or save data.**

#### Green - Make the red test pass, minimum change only

- Source files: likely `src/player.h`, player save/load code, corrupt-use command handlers, and any player cleanup/init path.
- What to change: add the smallest durable corruption state needed by the test and increment it only on confirmed corrupt use.
- Constraint: do not add multiple tiers, endings, or broad UI yet.
- Decisions satisfied: Q4, Q5, Q10.
- Next mini-cycle: return to Red for ordinary cursed gear separation and save persistence.

Run targeted player corruption tests after each mini-cycle.

#### Refactor - Clean up while keeping tests green

- Encapsulate corruption mutation in a helper such as `player_inc_corruption()` if direct field writes spread beyond one file.

### Slice 5: Escalating Corruption Effects

#### Red - Write tests first, no implementation code yet

- Test file: extend `src/tests/player/corruption.c`.
- What the test proves: higher corruption tiers produce escalating hostile effects and never produce clean sustainable power.
- Assertion strategy: deterministic player state transitions at explicit corruption thresholds.
- Existing tests to rewrite: none.
- Follow-up mini-cycles:
  - First test: low corruption produces warning/status only.
  - Second test, after green: middle corruption applies a hostile consequence such as aggravation, drain, impaired recovery, or sticky binding.
  - Third test, after green: deep corruption marks the character doomed or blocks heroic victory.

Run the player corruption target after each mini-cycle.

Expected red: corruption counter has no consequences.

**Hard gate: each threshold test must fail before implementing that threshold.**

#### Green - Make the red test pass, minimum change only

- Source files: player corruption helper, player property calculation, status display, and victory eligibility code as needed by each mini-cycle.
- What to change: add one threshold consequence at a time.
- Constraint: consequences must be hostile and legible; do not provide corruption-only buffs.
- Decisions satisfied: Q2, Q3, Q4, Q5, Q10, Q11.
- Next mini-cycle: return to Red for the next threshold.

Run targeted tests and inspect player-facing messages.

#### Refactor - Clean up while keeping tests green

- Keep threshold data readable and avoid scattering magic numbers across unrelated systems.

### Slice 6: Convert Player Drain Life Devices

#### Red - Write tests first, no implementation code yet

- Test file: add or extend `src/tests/object/info.c` or a data validation test under `src/tests/object`.
- What the test proves: ordinary player devices no longer expose `Drain Life` / `MON_DRAIN` as safe player power.
- Assertion strategy: data lookup for wands/rods/activations confirms either clean replacement effects or corrupt classification.
- Existing tests to rewrite: any test that treats player `MON_DRAIN` devices as normal allowed objects.
- Follow-up mini-cycles:
  - First test: `wand:Drain Life` is absent, renamed to a clean equivalent, or marked corrupt.
  - Second test, after green: `rod:Drain Life` follows the same policy.
  - Third test, after green: artifact `DRAIN_LIFE*` activations are corrupt-gated, not safe.

Run the targeted object test.

Expected red: `Drain Life` is currently ordinary player-accessible power.

**Hard gate: observe the first device-policy test fail before editing object data.**

#### Green - Make the red test pass, minimum change only

- Source files: `lib/gamedata/object.txt`, `lib/gamedata/activation.txt`, possibly artifact entries using `DRAIN_LIFE*`.
- What to change: replace ordinary devices with clean damage/control equivalents, or mark corrupt artifact activations as corrupt-use paths.
- Constraint: do not remove monster `MON_DRAIN` or enemy nether/life-drain behavior.
- Decisions satisfied: Q6, Q7.
- Next mini-cycle: return to Red for rods and artifact activations.

Run targeted object tests and parser tests.

#### Refactor - Clean up while keeping tests green

- Update player-facing names/descriptions so clean replacements do not imply life siphoning, souls, undeath, or occult power.

### Slice 7: Remove Vampiric and Bloodlust Player Power

#### Red - Write tests first, no implementation code yet

- Test file: add or extend `src/tests/game/vanguard.c`, `src/tests/parse/ptimed.c`, or a focused data policy test.
- What the test proves: no playable class, player shape, or normal player object can grant `ATT_VAMP` or `BLOODLUST`.
- Assertion strategy: data scan through parsed class/shape/object definitions where practical; grep-style check only as a secondary gate.
- Existing tests to rewrite: any test that expects vampire form, werewolf form, or bloodlust as player power.
- Follow-up mini-cycles:
  - First test: no class spell grants `BLOODLUST`.
  - Second test, after green: no player shape grants `ATT_VAMP`.
  - Third test, after green: vampire form is unavailable or converted to enemy-only data.

Run targeted tests.

Expected red: `shape:vampire` grants `ATT_VAMP`, and `BLOODLUST` remains available as a player timed effect.

**Hard gate: observe one forbidden-access test fail before editing class/shape data.**

#### Green - Make the red test pass, minimum change only

- Source files: `lib/gamedata/class.txt`, `lib/gamedata/shape.txt`, `lib/gamedata/player_timed.txt` only if text cleanup is required.
- What to change: remove player access to `ATT_VAMP` and `BLOODLUST`; replace vampire with a clean stealth/speed form and werewolf with a clean nature/melee form if the gameplay niche is needed.
- Constraint: do not delete enum/timed-effect plumbing until save and enemy references are audited.
- Decisions satisfied: Q6, Q8.
- Next mini-cycle: return to Red for each remaining forbidden access path.

Run targeted tests and `alltests`.

#### Refactor - Clean up while keeping tests green

- Keep replacement descriptions grounded in nature, courage, discipline, or heroic resolve.

### Slice 8: Shadow Player Infrastructure Access Gate

#### Red - Write tests first, no implementation code yet

- Test file: add or extend store/object/birth tests as appropriate.
- What the test proves: shadow books and shadow realm content are not available as player progression.
- Assertion strategy: no town stock/buy access, no playable class uses `shadow`, and shadow-book drops are either removed, converted, or classified as corrupt/hostile clutter.
- Existing tests to rewrite: any test expecting normal player shadow books.
- Follow-up mini-cycles:
  - First test: no playable class has `realm:shadow`.
  - Second test, after green: no town store stocks or buys shadow books.
  - Third test, after green: monster shadow-book drops are removed or reclassified according to chosen policy.

Run targeted birth/store/object tests.

Expected red: shadow book tval and drops remain reachable as ordinary drops.

**Hard gate: observe the first shadow-access test fail before editing data.**

#### Green - Make the red test pass, minimum change only

- Source files: `lib/gamedata/class.txt`, `lib/gamedata/store.txt`, `lib/gamedata/monster.txt`, possibly `lib/gamedata/object_base.txt` only after compatibility review.
- What to change: remove player progression access to shadow books while preserving parser stability.
- Constraint: do not delete `TV_SHADOW_BOOK` or shadow realm parser entries unless a compatibility audit proves safe.
- Decisions satisfied: Q6.
- Next mini-cycle: return to Red for store and monster drops.

Run targeted tests and parser/alltests.

#### Refactor - Clean up while keeping tests green

- Leave comments or follow-up notes where compatibility plumbing remains intentionally.

### Slice 9: Summon Undead and Curse Scroll Policy

#### Red - Write tests first, no implementation code yet

- Test file: add or extend object data policy tests.
- What the test proves: `Summon Undead`, `Curse Weapon`, and `Curse Armour` are not normal useful player consumables.
- Assertion strategy: object data lookup confirms removal, hostile/trap classification, zero normal allocation, or corrupt warning path.
- Existing tests to rewrite: any object test that assumes these are safe scrolls.
- Follow-up mini-cycles:
  - First test: `Summon Undead` is not a normal player-beneficial scroll.
  - Second test, after green: curse scrolls are hostile/junk/corrupt, not tools.

Run targeted object tests.

Expected red: scrolls currently exist as player-use objects.

**Hard gate: observe the first scroll-policy test fail before editing object data.**

#### Green - Make the red test pass, minimum change only

- Source file: `lib/gamedata/object.txt`.
- What to change: remove normal allocation, convert to trap/hostile flavor, or mark corrupt with warnings and consequences.
- Constraint: keep enemy summoning spells and chest/trap summoning intact.
- Decisions satisfied: Q6, Q9.
- Next mini-cycle: return to Red for curse scrolls.

Run targeted object tests and parser/alltests.

#### Refactor - Clean up while keeping tests green

- Clarify descriptions so scrolls read as dangerous artifacts or hostile mistakes, not tactical tools.

### Slice 10: Heroic Victory Gate and Corruption Failure Ending

#### Red - Write tests first, no implementation code yet

- Test file: add or extend a game/victory test under `src/tests/game` if practical.
- What the test proves: a deeply corrupted character cannot receive an untainted heroic victory and can trigger a unique corruption failure state.
- Assertion strategy: controlled player state and victory/failure function outcome; avoid full dungeon simulation.
- Existing tests to rewrite: any victory test that assumes all living characters can win regardless of corruption.
- Follow-up mini-cycles:
  - First test: deep corruption blocks heroic victory.
  - Second test, after green: deep corruption produces a distinct failure message/reason.
  - Third test, after green: uncorrupted victory path is unchanged.

Run targeted game tests.

Expected red: victory logic does not know about corruption.

**Hard gate: observe the victory-gate test fail before editing victory logic.**

#### Green - Make the red test pass, minimum change only

- Source files: victory handling, death/failure reason handling, score entry if needed.
- What to change: add corruption-aware victory/failure classification.
- Constraint: do not make ordinary curses or low corruption block victory.
- Decisions satisfied: Q10, Q11.
- Next mini-cycle: return to Red for message and uncorrupted control path.

Run targeted game tests and relevant score/death tests.

#### Refactor - Clean up while keeping tests green

- Keep corruption failure text separate from normal death and from Morgoth victory messaging.

### Slice 11: Documentation and Ubiquitous Language

#### Red - Write tests first, no implementation code yet

- Test file: content verification via `rg` plus any docs test target available.
- What the test proves: player-facing docs explain corruption as hostile temptation and no docs recommend forbidden player powers as normal options.
- Assertion strategy: content checks over `docs`, `lib/help`, and `UBIQUITOUS_LANGUAGE.md`.
- Existing tests to rewrite: stale docs that present old powers as available.
- Follow-up mini-cycles:
  - First check: add `Corruption Mechanic` to ubiquitous language.
  - Second check, after green: docs mention warnings and consequences.
  - Third check, after green: stale player-facing `Drain Life`, vampire, bloodlust, and shadow power recommendations are gone.

Run:

```sh
rg -n "Corruption Mechanic" UBIQUITOUS_LANGUAGE.md docs lib/help
rg -n "vampire form|Bloodlust|Drain Life|shadow book|shadow ritual|Summon Undead" docs lib/help lib/gamedata/class.txt
```

Expected red: the new term is missing and stale player-facing references may remain.

**Hard gate: observe the content checks fail or produce stale hits before editing docs.**

#### Green - Make the red test pass, minimum change only

- Source files: `UBIQUITOUS_LANGUAGE.md`, `docs/birth.rst`, `docs/dungeon.rst`, relevant help files.
- What to change: document corruption policy and remove/rewrite stale player advice.
- Constraint: enemy-only evil references may remain when clearly antagonistic.
- Decisions satisfied: Q1-Q12.
- Next mini-cycle: return to Red for stale reference checks.

Run the content checks again.

#### Refactor - Clean up while keeping tests green

- Keep language precise: corruption is not a class path, not a clean heroic power, and not a safe optimization.

## Verification

### Local verification sequence

1. For each slice, run the targeted unit or data test named in that slice.
2. Run parser-focused tests after gamedata edits:

   ```sh
   cmake --build build -t allunittests -j2
   ```

3. Run end-to-end test frontend coverage:

   ```sh
   cmake --build build -t alltests -j2
   ```

4. Run the normal build:

   ```sh
   cmake --build build -j2
   ```

5. Run whitespace and patch hygiene:

   ```sh
   git diff --check
   ```

6. For player-facing gameplay changes, use `$heroband-playtest` with a written `TEST_CONTRACT.md` before any direct GCU/tmux gameplay pass.

### Subagent verification passes

#### Test verifier pass

Use `$heroband-test-quality-verifier` on:

- `src/tests/object/corruption.c`
- `src/tests/player/corruption.c`
- Any modified birth, object, game, or save/load tests

Prompt focus:

Check that tests prove player-visible moral restrictions and corruption behavior through real data paths, not reward-hacked string checks or setup that bypasses the behavior under test. Verify that enemy-only evil content is not accidentally treated as a failure.

#### Moral policy audit

Review:

- `lib/gamedata/class.txt`
- `lib/gamedata/object.txt`
- `lib/gamedata/artifact.txt`
- `lib/gamedata/ego_item.txt`
- `lib/gamedata/shape.txt`
- `lib/gamedata/player_timed.txt`
- `docs`
- `lib/help`

Prompt focus:

Classify every remaining hit for blood, shadow, nether, soul, vampire, demon, undead, curse, Morgul, Morgoth, and drain as either enemy-only, hostile corruption, harmless atmosphere, or player-accessible power. Flag any player-accessible evil power that remains safe, sustainable, or beneficial.

## Acceptance Criteria

1. The One Ring, Morgoth crown, Hammer of the Underworld, and Morgul ego weapons are classified as corrupt and are not treated as normal safe equipment.
2. Equipping or activating corrupt gear produces an explicit warning/confirmation path.
3. Confirmed corrupt use increments durable character corruption separate from ordinary curses.
4. Corruption consequences escalate and can eventually block heroic victory or trigger a unique failure ending.
5. Ordinary cursed gear remains a permitted risk and does not automatically count as deep corruption.
6. Player-use `Drain Life` devices are removed, cleanly replaced, or corrupt-gated; monster drain remains intact.
7. No playable class, shape, or normal object grants safe `ATT_VAMP` or `BLOODLUST`.
8. Vampire form is removed or enemy-only; werewolf form is replaced with a clean nature/melee form if its gameplay niche remains.
9. Shadow books and shadow realm infrastructure are not available as player progression.
10. `Summon Undead` and curse scrolls are not normal player-beneficial tools.
11. Heroic victory requires resisting, rejecting, destroying, or surviving corruption without being mastered by it.
12. Documentation and ubiquitous language describe corruption as hostile temptation, not a heroic power source.

## Implementation Checklist

### Slice 1: Corrupt Item Classification

- [ ] RED: add first object corruption test for the One Ring.
- [ ] RED: run targeted test and confirm failure.
- [ ] GREEN: add minimum corrupt classification support and mark the One Ring.
- [ ] GREEN: run targeted test and confirm pass.
- [ ] RED/GREEN: repeat for Morgoth artifacts.
- [ ] RED/GREEN: repeat for Morgul ego weapons.
- [ ] REFACTOR: centralize corrupt object query and rerun tests.

### Slice 2: Warning Before Equipping Corrupt Gear

- [ ] RED: add corrupt equip warning test.
- [ ] RED: run targeted wield test and confirm failure.
- [ ] GREEN: add minimum warning/confirmation gate.
- [ ] GREEN: run targeted wield test and confirm pass.
- [ ] RED/GREEN: verify ordinary cursed gear remains distinct.
- [ ] REFACTOR: centralize warning text if duplicated.

### Slice 3: Warning Before Activating Corrupt Power

- [ ] RED: add corrupt activation warning test.
- [ ] RED: run targeted activation/object test and confirm failure.
- [ ] GREEN: add minimum activation warning gate.
- [ ] GREEN: run targeted test and confirm pass.
- [ ] RED/GREEN: verify clean activations are unchanged.
- [ ] REFACTOR: share confirmation helper if useful.

### Slice 4: First Corruption State Consequence

- [ ] RED: add player corruption increment test.
- [ ] RED: run targeted player corruption test and confirm failure.
- [ ] GREEN: add minimum player corruption state and increment path.
- [ ] GREEN: run targeted test and confirm pass.
- [ ] RED/GREEN: verify ordinary curses do not increment corruption.
- [ ] RED/GREEN: add save/load persistence only if practical in current harness.
- [ ] REFACTOR: centralize corruption mutation.

### Slice 5: Escalating Corruption Effects

- [ ] RED: add low-threshold corruption test.
- [ ] GREEN: add minimum low-threshold behavior.
- [ ] RED/GREEN: add middle-threshold hostile consequence.
- [ ] RED/GREEN: add deep corruption doomed state.
- [ ] REFACTOR: make thresholds readable and rerun tests.

### Slice 6: Convert Player Drain Life Devices

- [x] RED: add wand `Drain Life` policy test.
- [x] GREEN: replace, remove, or corrupt-gate wand use.
- [x] RED/GREEN: repeat for rod `Drain Life`.
- [x] RED/GREEN: repeat for artifact `DRAIN_LIFE*` activations.
- [x] REFACTOR: clean player-facing descriptions.

### Slice 7: Remove Vampiric and Bloodlust Player Power

- [x] RED: add no-class-`BLOODLUST` access test.
- [x] GREEN: remove class/object access to `BLOODLUST`.
- [x] RED/GREEN: add no-player-`ATT_VAMP` shape test.
- [x] RED/GREEN: replace vampire/werewolf forms.
- [x] REFACTOR: clean descriptions and rerun alltests.

### Slice 8: Shadow Player Infrastructure Access Gate

- [x] RED: add no-playable-shadow-realm test.
- [x] GREEN: remove playable shadow realm use.
- [x] RED/GREEN: add no-town-shadow-book access test.
- [x] RED/GREEN: remove or reclassify monster shadow-book drops.
- [x] REFACTOR: document compatibility plumbing that remains.

### Slice 9: Summon Undead and Curse Scroll Policy

- [x] RED: add `Summon Undead` object policy test.
- [x] GREEN: remove, hostile-classify, or corrupt-gate it.
- [x] RED/GREEN: repeat for `Curse Weapon` and `Curse Armour`.
- [x] REFACTOR: clarify dangerous descriptions.

### Slice 10: Heroic Victory Gate and Corruption Failure Ending

- [x] RED: add deep-corruption victory-block test.
- [x] GREEN: add minimum victory gate.
- [x] RED/GREEN: add unique failure reason/message.
- [x] RED/GREEN: verify uncorrupted victory path unchanged.
- [x] REFACTOR: keep failure text separate from ordinary death.

### Slice 11: Documentation and Ubiquitous Language

- [x] RED: run term/stale-reference checks and capture failures.
- [x] GREEN: add `Corruption Mechanic` to `UBIQUITOUS_LANGUAGE.md`.
- [x] RED/GREEN: update docs/help for corrupt warnings and consequences.
- [x] RED/GREEN: remove stale player-facing evil power recommendations.
- [x] REFACTOR: tighten moral terminology.

## Notes and Follow-Ups

- A later plan can design richer corruption content: purification quests, Ring destruction, sanctuary cleansing, corruption dreams, or deep-floor scenario saves.
- Do not implement passive low-health reward mechanics as corruption or Vanguard features without a separate TDD plan; those risk incentivizing intentional self-harm.
- Preserve internal enum and parser stability unless a compatibility audit proves deletion is safe.
