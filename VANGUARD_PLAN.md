# Vanguard Replacement Implementation Plan
## Clean heroic replacement for playable Blackguard

> **Status: PLANNING** - 10 grill decisions resolved. Ready for TDD implementation.

## Overview

Replace the player-accessible Blackguard class with Vanguard, a heavily armored frontline champion powered by courage, discipline, armor mastery, battlefield training, and heroic resolve. The first patch must make Vanguard selectable in birth, remove player access to Blackguard's blood/shadow/unholy mechanics, update player-facing documentation, remove town shop access to shadow books, and narrowly recast enemy blackguards as fallen vanguards without redesigning monster behavior.

## Decisions

| # | Question | Decision | Source |
|---|---|---|---|
| Q1 | What replaces old `Werewolf Form`? | `Unbroken`, a timed active stance. | Grill session |
| Q2 | May `Unbroken` include speed? | Yes, modest `FAST` plus courage/protection is acceptable if self-harm mechanics are removed. | Grill session |
| Q3 | Is real passive `Last Stand` in first patch? | No. Defer passive low-HP thresholds until C behavior can be tested carefully. | Grill session |
| Q4 | What occupies the old `Bloodlust` slot? | A clean active `Last Stand` order using safe effects, not `TMD_BLOODLUST`. | Grill session |
| Q5 | Does Vanguard keep orders/books? | Yes. Preserve bruiser-with-powers niche via clean martial orders. | Grill session |
| Q6 | Add a new field-manual item type? | No. Use existing book infrastructure and the existing `tactics` realm. | Grill session |
| Q7 | Keep shadow books in town shops? | No. Remove normal town shop stock/buy access if no playable class uses them. | Grill session |
| Q8 | Delete enemy blackguards? | No. Recast/rename them as enemy-only fallen counterparts. | Grill session |
| Q9 | Enemy naming? | `blackguard` -> `fallen vanguard`; `troll blackguard` -> `fallen troll vanguard`. | Grill session |
| Q10 | Update tile preference labels? | Yes, rename simple monster-name mappings; do not redraw assets. | Grill session |

## Current Code State

### Already correct

- Birth UI maps visible class rows to canonical class IDs in `src/ui-birth.c`; this avoids hidden-class menu drift.
- `tactics` realm already exists in `lib/gamedata/realm.txt` with clean "issue/order/field manual" language.
- General already demonstrates clean tactical book usage through existing `magic book` infrastructure.
- Enemy-only evil content exists throughout the data and can remain when clearly antagonistic.

### Out of alignment

- `lib/gamedata/class.txt` still defines the old player class as `Blackguard`, with shadow books, rituals, cruel titles, `Bloodlust`, `Unholy Reprieve`, `Werewolf Form`, `Venom`, and `COMBAT_REGEN`.
- `src/player-class.c` currently gates `Blackguard` out of birth; after the rename it must allow `Vanguard` while still rejecting any lingering `Blackguard` data.
- `lib/gamedata/player_timed.txt` still has player-facing `BLOODLUST` messages. The first patch should remove Vanguard access to this timed effect rather than renaming the timed-effect symbol.
- `PF_COMBAT_REGEN` rewards HP loss with mana and mana degeneration with healing in `src/player-util.c`, `src/player-spell.c`, and `src/player-attack.c`; Vanguard must not use it in the first patch.
- `lib/gamedata/store.txt` stocks and buys `shadow book` in town.
- `lib/gamedata/monster.txt` and tile preference files still expose enemy `blackguard` names.
- `docs/birth.rst`, Borg class-name expectations, and birth tests do not know about Vanguard.

### Important constraints

Keep class order and class slot IDs stable. Do not delete `TMD_BLOODLUST`, `TV_SHADOW_BOOK`, shadow-book object base data, or shared dark/nether/curse effects in the first patch, because they may be referenced by saves, monsters, tiles, Borg, object knowledge, or tests. The first patch should remove player access and player-facing town availability, not globally delete shared infrastructure.

## Intended Implementation Shape

Make a narrow data-first patch. Rename the playable class slot from `Blackguard` to `Vanguard`, switch it to `tactics` orders using existing book infrastructure, replace forbidden spells with clean martial/tactical equivalents backed by existing effects, remove `COMBAT_REGEN` and `IMPAIR_HP`, update birth availability, docs, tests, Borg expectations, shop stock, enemy names, and tile preference labels. Defer the real low-health passive `Last Stand` to a later C-tested mechanic.

## Red/Green TDD Slices

### Slice 1: Birth Playability Contract

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/player/birth.c`
- What the test proves: `Vanguard` is playable, `Necromancer` and stale `Blackguard` are not playable, and `NULL` remains rejected.
- Assertion strategy: direct calls to public `player_class_is_playable()`.
- Existing tests to rewrite: update `test_forbidden_classes_unplayable` into a broader moral availability test.
- Follow-up mini-cycles: none.

Run:

```sh
cmake --build build -t run-unittest-player-birth -j2
```

Expected red: the new `Vanguard` assertion fails until the class gate is updated.

#### Green - Make the red test pass, minimum change only

- Source files: `src/player-class.c`, `lib/gamedata/class.txt`.
- What to change: rename `name:Blackguard` to `name:Vanguard`; update `player_class_is_playable()` so only `Necromancer` and stale `Blackguard` are rejected by name.
- Constraint: do not alter class order, class count, or class IDs.
- Decisions satisfied: Q5.
- Next mini-cycle: move to Slice 2.

Run the same unit test and confirm green.

#### Refactor - Clean up while keeping tests green

- Adjust the comment in `player_class_is_playable()` to mention Vanguard replacing the old Blackguard slot.
- Keep separate: no spell list cleanup in this slice.

### Slice 2: Clean Vanguard Class Data

#### Red - Write tests first, no implementation code yet

- Test file: add or update a parser/data test under `src/tests/parse` if an existing class parser test is available; otherwise add an end-to-end birth fixture in `tests/birth/Hu-Va`.
- What the test proves: Human Vanguard can be generated through the test frontend and reports `player-class: Vanguard`.
- Assertion strategy: public `-mtest` birth command/output, not parser internals.
- Existing tests to rewrite: none; keep existing Human General fixture.
- Follow-up mini-cycles:
  - Add a second assertion path, if practical, that rejected `Blackguard` input does not generate a playable Blackguard.

Run:

```sh
cmake --build build -t alltests -j2
```

Expected red: `Human Vanguard` does not birth correctly until class data and availability are complete.

#### Green - Make the red test pass, minimum change only

- Source file: `lib/gamedata/class.txt`.
- What to change:
  - Replace Blackguard titles with clean Vanguard ranks.
  - Remove `obj-flags:IMPAIR_HP`.
  - Remove `COMBAT_REGEN` from player flags; keep `CHOOSE_SPELLS | SHIELD_BASH`.
  - Switch books from `shadow book` / `shadow` to existing `magic book` / `tactics` style.
  - Replace player-facing spell names/descriptions:
    - `Seek Battle` -> `Assess the Field`
    - `Berserk Strength` -> `Stand Firm` or another clean courage/protection order
    - `Leap into Battle` -> `Breakthrough`
    - `Grim Purpose` -> `Combat Discipline`
    - `Maim Foe` -> `Staggering Blow`
    - `Howl of the Damned` -> `Horn of Defiance`
    - `Relentless Taunting` -> `Hold the Line` or `Defend the Weak`
    - `Werewolf Form` -> `Unbroken`
    - `Bloodlust` -> `Last Stand`
  - Remove player access to `Venom`, `Unholy Reprieve`, `Werewolf Form` shapechange, and `TMD_BLOODLUST`.
  - Keep only existing safe effects for first patch: `HERO`, `SHIELD`, `FAST`, `BLESSED`, `FREE_ACT`, `OPP_CONF`, `SWEEP`, `MOVE_ATTACK`, `MELEE_BLOWS:MON_STUN`, `MELEE_BLOWS:FORCE`, detection, and wall/rubble utility.
- Constraint: do not add new effects or timed-effect enum entries in this slice.
- Decisions satisfied: Q1, Q2, Q3, Q4, Q5, Q6.
- Next mini-cycle: if parser/test frontend fails because book names are missing object definitions, add only the minimum `book-graphics`/`book-properties` lines matching General's pattern.

Run the targeted birth fixture, then `alltests`.

#### Refactor - Clean up while keeping tests green

- Ensure all Vanguard descriptions use courage, discipline, armor mastery, duty, endurance, and battlefield training.
- Keep separate: do not implement passive low-HP thresholds.

### Slice 3: Remove Town Shadow Book Access

#### Red - Write tests first, no implementation code yet

- Test file: add a focused parser/store test if one exists for `store.txt`; otherwise add this as a deterministic grep-style validation script only if the repo already accepts such checks.
- What the test proves: town store data no longer has `always:shadow book` or `buy:shadow book`.
- Assertion strategy: data-level assertion against `lib/gamedata/store.txt`.
- Existing tests to rewrite: none.

Run the targeted test or `alltests`; observe failure while shadow-book stock remains.

#### Green - Make the red test pass, minimum change only

- Source file: `lib/gamedata/store.txt`.
- What to change: remove `always:shadow book` and `buy:shadow book` from the Magic Shop.
- Constraint: do not delete `shadow book` object base, tval, monster drops, tiles, or object knowledge entries.
- Decisions satisfied: Q7.

Run the targeted test and `alltests`.

#### Refactor - Clean up while keeping tests green

- None expected.

### Slice 4: Documentation and Player-Facing Text

#### Red - Write tests first, no implementation code yet

- Test file: add or update a documentation/content check if existing tooling supports it; otherwise use a scripted `rg` verification as the red contract.
- What the test proves: docs include `Vanguard` and no player class docs present Blackguard as playable.
- Assertion strategy: content check over `docs/birth.rst` and player-facing help/customize files.
- Existing tests to rewrite: none.

Run:

```sh
rg -n "Blackguard|Bloodlust|Unholy Reprieve|Werewolf Form|Howl of the Damned|Fear and Torment|Deadly Powers|Into the Shadows" docs lib/help lib/customize lib/gamedata/class.txt
```

Expected red: forbidden player-facing strings remain until docs/data cleanup is complete.

#### Green - Make the red test pass, minimum change only

- Source files: `docs/birth.rst`, `lib/gamedata/class.txt`, any help/customize file found by the red check.
- What to change: add a Vanguard class description using the approved class fantasy and remove stale playable Blackguard references.
- Constraint: enemy-only dark/evil docs are not in scope unless they present player access.
- Decisions satisfied: Q1-Q6.

Run the `rg` check again and inspect any remaining hits for classification.

#### Refactor - Clean up while keeping tests green

- Tighten wording so `Last Stand` does not imply self-harm, pain empowerment, rage, blood, darkness, or occult power.

### Slice 5: Enemy Recast and Tile Name Mappings

#### Red - Write tests first, no implementation code yet

- Test file: add a data content check or document a mandatory `rg` verification.
- What the test proves: monster data and tile preference labels no longer use enemy monster names `blackguard` or `troll blackguard`.
- Assertion strategy: data string search, then parser/alltests.
- Existing tests to rewrite: none.

Run:

```sh
rg -n "name:blackguard|name:troll blackguard|friends:.*blackguard|monster:blackguard|monster:troll blackguard" lib/gamedata lib/tiles
```

Expected red: current enemy names and friend references still exist.

#### Green - Make the red test pass, minimum change only

- Source files: `lib/gamedata/monster.txt`, `lib/tiles/**/graf-*.prf`, `lib/tiles/gervais/xtra-dvg.prf` if class-name mappings apply.
- What to change:
  - `name:blackguard` -> `name:fallen vanguard`
  - `name:troll blackguard` -> `name:fallen troll vanguard`
  - friend references to those names accordingly.
  - tile `monster:` labels accordingly.
- Constraint: do not change monster stats, spells, drops, levels, friends quantity, or art assets.
- Decisions satisfied: Q8, Q9, Q10.

Run parser/alltests.

#### Refactor - Clean up while keeping tests green

- If descriptions call them simply blackguards, adjust only direct wording needed to fit fallen-vanguard naming.

### Slice 6: Borg and Compatibility References

#### Red - Write tests first, no implementation code yet

- Test file: use existing build/alltests as the red gate; add a unit test only if Borg initialization has an existing test harness.
- What the test proves: build does not fail from stale `Blackguard` class-name expectations or old spell rating names.
- Assertion strategy: compile and deterministic tests.
- Existing tests to rewrite: none.

Run:

```sh
cmake --build build -j2
```

Expected red after earlier slices: stale Borg expectations may fail build/runtime initialization checks.

#### Green - Make the red test pass, minimum change only

- Source files: `src/borg/borg-init.c`, `src/borg/borg-magic.c`, `src/borg/borg.txt`, related Borg files found by `rg`.
- What to change:
  - Update class-name expectation from `Blackguard` to `Vanguard`.
  - Update spell-rating strings to the new Vanguard order names.
  - Update comments and formula text labels where they refer to the playable class.
- Constraint: do not rebalance Borg strategy beyond names needed for build/test stability.
- Decisions satisfied: Q5.

Run build again.

#### Refactor - Clean up while keeping tests green

- Leave `CLASS_BLACKGUARD` numeric constant names if changing them would create unnecessary compatibility churn; treat as internal legacy plumbing for this patch.

## Verification

### Local verification sequence

1. Run targeted player birth unit test:
   ```sh
   cmake --build build -t run-unittest-player-birth -j2
   ```
2. Run targeted scripted birth fixture for Human Vanguard after adding it:
   ```sh
   cmake --build build -t alltests -j2
   ```
3. Run full build:
   ```sh
   cmake --build build -j2
   ```
4. Run full automated tests:
   ```sh
   cmake --build build -t alltests -j2
   ```
5. Check whitespace:
   ```sh
   git diff --check
   ```
6. Run final moral-reference audit:
   ```sh
   rg -n "Blackguard|Bloodlust|Unholy Reprieve|Werewolf Form|Howl of the Damned|Fear and Torment|Deadly Powers|Into the Shadows|shadow book|TMD_BLOODLUST|COMBAT_REGEN" docs lib/help lib/customize lib/gamedata/class.txt src/tests tests src/borg
   ```
   Classify any remaining hits before handoff.

### Direct Gameplay Pass

Because this changes player-facing birth, class powers, spell/order lists, stores, and moral restrictions, use `$heroband-playtest` after deterministic tests are green.

Write a test contract before launching GCU:

```text
Invariant: Vanguard is selectable during birth and has no player-accessible evil power.
Correct Looks Like: The class list includes Vanguard, excludes Blackguard and Necromancer, Vanguard starts with clean tactical books/orders, shadow books are not sold in town, and order names contain no blood/shadow/unholy/werewolf framing.
Steps:
1. Launch isolated GCU session.
2. Start a new Human Vanguard.
3. Inspect the class selection screen.
4. Enter gameplay and inspect starting inventory.
5. Browse Vanguard orders.
6. Visit Magic Shop and inspect book stock.
7. Quit cleanly.
Evidence Plan: Capture class screen, character summary, inventory, order list, Magic Shop stock, and quit prompt.
```

Use:

```sh
STATE="$(mktemp -d /tmp/heroband-playtest.XXXXXX)"
scripts/heroband-playtest start --state-dir "$STATE" --contract "$STATE/TEST_CONTRACT.md"
scripts/heroband-playtest capture --state-dir "$STATE"
scripts/heroband-playtest stop --state-dir "$STATE"
```

## Acceptance Criteria

1. New characters can select `Vanguard` through normal and scripted birth.
2. `Blackguard` is not selectable as a playable class by name.
3. Vanguard class data has no player-accessible shadow, bloodlust, unholy, werewolf, occult, demonic, curse-benefit, nether, or life-drain mechanics.
4. Vanguard powers are framed as courage, discipline, armor mastery, battlefield training, and heroic resolve.
5. `Last Stand` exists only as a clean active order in the first patch and does not use `TMD_BLOODLUST`.
6. `Unbroken` is a clean timed stance and does not shapechange.
7. Vanguard does not use `PF_COMBAT_REGEN` or `IMPAIR_HP`.
8. Town stores no longer stock or buy `shadow book`.
9. Enemy blackguard names are recast as `fallen vanguard` / `fallen troll vanguard` with mechanics preserved.
10. Tile preference labels are updated for renamed enemy monsters without art churn.
11. Documentation presents Vanguard as playable and does not present Blackguard as a player class.
12. Build, automated tests, whitespace check, moral-reference audit, and direct GCU birth/store/order playtest pass or have clearly reported residual risks.

## Implementation Checklist

### Slice 1

- [ ] RED: update `src/tests/player/birth.c` for Vanguard playable / Blackguard forbidden.
- [ ] RED: run targeted birth unit test and observe failure.
- [ ] GREEN: rename class slot and update `player_class_is_playable()`.
- [ ] GREEN: rerun targeted birth unit test.
- [ ] REFACTOR: update class-gate comment.

### Slice 2

- [ ] RED: add scripted Human Vanguard birth fixture or class parser coverage.
- [ ] RED: run tests and observe failure.
- [ ] GREEN: clean Vanguard class data, books, flags, titles, and order list.
- [ ] GREEN: rerun targeted fixture and `alltests`.
- [ ] REFACTOR: clean descriptions and remove first-patch forbidden player-facing terms.

### Slice 3

- [ ] RED: add store-data check or run documented `rg` contract.
- [ ] RED: observe shadow-book shop stock failure.
- [ ] GREEN: remove town shop `shadow book` stock/buy lines.
- [ ] GREEN: rerun store check and `alltests`.
- [ ] REFACTOR: none.

### Slice 4

- [ ] RED: run documentation/player-facing moral reference check.
- [ ] RED: observe stale playable Blackguard or forbidden ability text.
- [ ] GREEN: update `docs/birth.rst` and any help/customize hits.
- [ ] GREEN: rerun moral reference check.
- [ ] REFACTOR: tighten `Last Stand` and `Unbroken` wording.

### Slice 5

- [ ] RED: run enemy blackguard/tile label search.
- [ ] RED: observe old enemy names.
- [ ] GREEN: rename monster names, friend references, and tile labels only.
- [ ] GREEN: run parser/alltests.
- [ ] REFACTOR: adjust direct monster descriptions only if needed.

### Slice 6

- [ ] RED: build after prior slices and capture stale Borg failures.
- [ ] GREEN: update Borg class-name and spell-rating references.
- [ ] GREEN: rerun build and alltests.
- [ ] REFACTOR: leave numeric legacy constants unless tests force change.

### Final Gates

- [ ] `cmake --build build -j2`
- [ ] `cmake --build build -t alltests -j2`
- [ ] `git diff --check`
- [ ] Final `rg` moral-reference audit with classifications for remaining hits.
- [ ] `$heroband-playtest` contract plus GCU birth/order/store pass.
