# General Marshal Implementation Plan
## Marshal of the West redesign for General powers

**Based on spec commit:** `2ccf8d9ca` - `Update General marshal design specs`

> **Status: COMPLETED** - Spec-driven from `specs/player.md`,
> `specs/monsters-combat.md`, and `specs/UBIQUITOUS_LANGUAGE.md` version 1.1.0.
> Implementation and verification completed on 2026-05-25.

## Overview

Implement the General's Marshal of the West direction: level-tiered living
temporary allies, formation effects that imply supporting soldiers without
creating extra monster instances, replacement of teleport-style General mobility
with Fighting Withdrawal, and a temporary Marshal's Banner banner zone. Preserve
the existing General compatibility slot and Angband command infrastructure while
removing player-facing project names and literal teleportation from the General's
class identity.

## Spec Delta To Implement

1. General temporary ally tier is determined by player level, not dungeon depth.
2. General temporary ally races must be explicit living soldier races with
   in-world names, not player-visible project labels like "Heroband infantry".
3. General formation effects may describe supporting soldiers but must not create
   additional controllable monsters, monster turns, drops, corpses, experience,
   or monster-lore sources.
4. Formation effects must be usable at reduced strength without a temporary ally
   and stronger when the player controls a valid matching temporary ally.
5. Arrow Volley is a control-first formation effect with modest area missile
   damage and stronger disruption when an archer-line temporary ally is active.
6. Fighting Withdrawal replaces Tactical Withdrawal's teleport identity with
   ordinary action energy, defensive benefit, no literal teleport, and optional
   legal non-teleport movement later.
7. Glorious Charge is an offensive commitment power, not an escape tool; it gives
   short heroic combat benefits and may disrupt nearby non-unique enemies, with
   stronger effect when a melee-line temporary ally is active.
8. Marshal's Banner creates a fixed-location temporary banner zone with authored
   radius and duration, player/ally morale benefits, possible enemy disruption,
   and expiration cleanup with no object, terrain, corpse, or permanent map
   residue.
9. Banner zones and formation state must clear on expiration and level
   transition.
10. All player-facing General text must use tactics, morale, lawful command,
    formations, living temporary allies, and banner-led battlefield control.

## Current Code State

### Already Correct

- `lib/gamedata/class.txt` already defines General as the player-facing class in
  the old Necromancer class slot, using the `tactics` realm and field-command
  books.
- `effect_handler_CALL_ALLY()` in `src/effect-handler-general.c` already creates
  a living monster, marks it with `MFLAG_CALLED_ALLY`, mirrors command duration
  between `TMD_COMMAND` and `MON_TMD_COMMAND`, wakes it, tracks health, and
  avoids the ordinary `SUMMON` handler.
- `src/mon-timed.c` already removes a called ally on command expiry with the
  "falls back to their unit" message.
- The current temporary ally races have `rarity:0` and `experience:0`, so the
  first draft already avoids ordinary allocation and experience rewards.

### Out Of Alignment

- `lib/gamedata/monster.txt` exposes `Heroband infantry` and `Heroband archer`
  as player-visible monster race names, violating the new in-world naming rule.
- `effect_handler_CALL_ALLY()` selects only two fixed race names by subtype and
  does not scale by player level.
- `lib/gamedata/class.txt` still has `Tactical Withdrawal` using `effect:TELEPORT`
  and presenting teleport as the General's mobility identity.
- General has no formation effect model yet; existing later powers are ordinary
  timed effects, bolt status, and teleport.
- There is no banner-zone state, expiration path, level-transition cleanup, or
  area disruption model for Marshal's Banner.
- There are no targeted unit tests for General ally tiering, formation
  no-monster-instance behavior, Fighting Withdrawal, Glorious Charge, or banner
  zones.

### Important Constraints

Do not delete shared teleport effects, monster timed effects, command mode, or
enemy-only evil systems. The implementation should add General-specific heroic
effects and data while leaving shared Angband mechanics available to enemies,
items, and other classes where appropriate. Keep parser-visible class slot order
stable and avoid changing save IDs unless a separate save/load compatibility
slice is added.

## Intended Implementation Shape

Start data-first and helper-first. Add explicit General soldier tiers in
`monster.txt`, then teach `CALL_ALLY` to choose a tier from player level. Next,
remove General teleport identity from class data with the simplest defensive
Fighting Withdrawal behavior. Then add a small formation-effect helper that can
apply monster disruption and player timed benefits without placing monsters.
Use that helper for Arrow Volley and Glorious Charge. Add banner-zone state only
after formation behavior exists, so Marshal's Banner can reuse the same
non-monster tactical effects. Finish with player-facing text/docs and direct GCU
playtest evidence.

## Red/Green TDD Slices

### Slice 1: Level-Tiered Temporary Ally Races

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/effects/general.c`
- Build registration: add the new test file to `CMakeLists.txt` and
  `src/tests/effects/suite.mk` as part of the Red test-file change.
- What the test proves: `CALL_ALLY` selects explicit in-world soldier race names
  by player level for infantry and archer calls.
- Assertion strategy: call a small exposed helper such as
  `general_ally_race_name(subtype, player_level)` or
  `general_ally_race(subtype, player_level)`, then assert exact tier names and
  `experience == 0`.
- Existing tests to rewrite: none.
- Follow-up mini-cycles:
  - Cycle A: level 1 infantry selects the low-tier infantry race.
  - Cycle B: a high-level infantry call selects a stronger infantry tier.
  - Cycle C: archer calls follow the same tier boundaries.
  - Cycle D: every temporary ally tier uses an in-world name and no name begins
    with `Heroband `.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
```

Expected red: the helper does not exist or returns the current fixed
`Heroband infantry` / `Heroband archer` names.

#### Green - Make the red test pass, minimum change only

- Source files: `src/effect-handler-general.c`,
  `lib/gamedata/monster.txt`, and any local header needed for the test helper.
- What to change: add explicit living soldier tier races, replace hard-coded
  `Heroband infantry` / `Heroband archer` lookup with level-based tier lookup,
  and keep `CALL_ALLY`'s existing command setup unchanged.
- Constraint: do not add extra allies, new command ownership, drops, or
  experience rewards.
- Spec delta satisfied: items 1 and 2.
- Next mini-cycle: after helper tests pass, add one effect-level test that
  invokes `CALL_ALLY` in a small arena and asserts exactly one called ally exists
  with mirrored command timers.

Run the same unit target after each mini-cycle and confirm green.

#### Refactor - Clean up while keeping tests green

- Extract tier thresholds into one local table if the first green step used
  duplicated switch logic.
- Keep separate: ordinary monster allocation and General temporary ally
  selection.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
```

### Slice 2: Fighting Withdrawal Replaces Teleport Identity

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/effects/general.c`
- What the test proves: no General class spell named Fighting Withdrawal or
  Tactical Withdrawal uses `EF_TELEPORT`, and casting the implemented effect
  grants a defensive benefit without changing the player's location.
- Assertion strategy: data-aware lookup of General spells from parsed
  `class.txt`, plus effect-handler execution in a small arena checking
  `player->grid` and relevant timed effects.
- Existing tests to rewrite: any test that expects General `Tactical Withdrawal`
  to teleport, if one is discovered.
- Follow-up mini-cycles:
  - Cycle A: class data test fails while `Tactical Withdrawal` still has
    `effect:TELEPORT`.
  - Cycle B: behavior test fails until the new effect applies defensive timed
    state and leaves location unchanged.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
```

Expected red: General spell data still exposes `EF_TELEPORT`.

#### Green - Make the red test pass, minimum change only

- Source files: `lib/gamedata/class.txt`, `src/list-effects.h`,
  `src/effect-handler-general.c`, and effect-info code only if required for
  player-visible descriptions.
- What to change: rename `Tactical Withdrawal` to `Fighting Withdrawal`, replace
  `effect:TELEPORT` with a General-specific defensive effect or existing clean
  timed effects, and ensure it consumes ordinary spell/action flow without
  teleporting.
- Constraint: do not implement pathfinding-based repositioning in this slice;
  the spec allows legal non-teleport movement later.
- Spec delta satisfied: item 6.

Run the same unit target and confirm green.

#### Refactor - Clean up while keeping tests green

- None unless a General helper becomes duplicated.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
```

### Slice 3: Formation Effect Foundation And Arrow Volley

#### Red - Write tests first, no implementation code yet

- Test files: `src/tests/effects/general.c` and, if monster timed resistance is
  easier to isolate there, `src/tests/monster/monster.c`.
- What the test proves: Arrow Volley applies only authored formation effects,
  creates no additional monster instances, produces no commandable actor, and
  has stronger control when an archer-line temporary ally is active.
- Assertion strategy: set up a small arena with two enemy monsters, count live
  monsters before/after, check `MON_TMD_SLOW` or the chosen disruption status,
  assert player command ownership is unchanged except for the preexisting
  archer ally, and assert no floor drops are created.
- Existing tests to rewrite: none.
- Follow-up mini-cycles:
  - Cycle A: formation helper applies reduced solo disruption and no monster
    creation.
  - Cycle B: with an active archer-line temporary ally, Arrow Volley applies a
    stronger duration or wider radius.
  - Cycle C: unique or resistant monsters obey existing monster timed-effect
    resistance behavior.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
cmake --build build -t run-unittest-monster-monster -j2
```

Expected red: there is no Arrow Volley formation effect or helper.

#### Green - Make the red test pass, minimum change only

- Source files: `src/list-effects.h`, `src/effect-handler-general.c`,
  `lib/gamedata/class.txt`, and a small header if tests need direct access to a
  pure helper.
- What to change: add a General formation effect path that scans an authored
  radius, applies modest missile damage only if chosen for the first green, and
  applies monster control statuses through ordinary timed-effect APIs.
- Constraint: do not place supporting soldiers as monsters, do not create drops
  or corpses, and do not alter autonomous monster processing.
- Spec delta satisfied: items 3, 4, and 5.

Run targeted unit tests after each mini-cycle and confirm green.

#### Refactor - Clean up while keeping tests green

- Extract ally-presence classification: no ally, archer-line ally, melee-line
  ally.
- Keep separate: formation effect state and command-mode ownership.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
cmake --build build -t run-unittest-monster-monster -j2
```

### Slice 4: Glorious Charge Offensive Formation

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/effects/general.c`
- What the test proves: Glorious Charge grants short heroic combat benefits,
  disrupts nearby non-unique enemies, improves with a melee-line temporary ally,
  and does not move the player as an escape or teleport.
- Assertion strategy: arena setup with adjacent and out-of-radius enemies,
  before/after player grid equality, timed-effect assertions for player benefits,
  and monster timed-effect assertions for affected enemies.
- Existing tests to rewrite: none.
- Follow-up mini-cycles:
  - Cycle A: solo Glorious Charge grants short player benefits and no movement.
  - Cycle B: nearby non-unique enemies are disrupted; out-of-radius enemies are
    unchanged.
  - Cycle C: a melee-line ally strengthens the benefit or disruption.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
```

Expected red: the Glorious Charge spell/effect does not exist.

#### Green - Make the red test pass, minimum change only

- Source files: `lib/gamedata/class.txt`, `src/effect-handler-general.c`, and
  `src/list-effects.h` if a new effect enum is needed.
- What to change: author Glorious Charge as a formation/offensive commitment
  effect using clean timed player benefits and monster disruption.
- Constraint: no teleport, no level transition, no extra controlled monsters.
- Spec delta satisfied: item 7.

Run the same unit target and confirm green.

#### Refactor - Clean up while keeping tests green

- Reuse the Slice 3 formation helper instead of branching a second area scanner.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
```

### Slice 5: Marshal's Banner Zone State

#### Red - Write tests first, no implementation code yet

- Test files: `src/tests/player/timed.c` or `src/tests/effects/general.c`; use
  the one with the cleaner setup for player turn/expiration state.
- What the test proves: Marshal's Banner creates one fixed banner zone with
  authored radius and duration, applies player/ally benefits only while actors
  are inside it, disrupts monsters inside but not outside, and expires without
  object or terrain residue.
- Assertion strategy: inspect explicit banner-zone state, player grid movement
  relative to the fixed origin, monster timed effects inside/outside radius, and
  terrain/object counts before/after expiration.
- Existing tests to rewrite: none.
- Follow-up mini-cycles:
  - Cycle A: planting a banner records origin, radius, and duration.
  - Cycle B: benefits apply in radius and stop outside radius.
  - Cycle C: monsters inside radius receive authored disruption; outside monsters
    do not.
  - Cycle D: expiration clears state and leaves no terrain/object residue.
  - Cycle E: level transition clears banner-zone and formation state.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
cmake --build build -t run-unittest-player-timed -j2
```

Expected red: there is no banner-zone state or Marshal's Banner effect.

#### Green - Make the red test pass, minimum change only

- Source files: `src/player.h`, `src/player-timed.c` or the appropriate world
  upkeep module, `src/effect-handler-general.c`, `src/generate.c` or level
  transition cleanup only if needed, and `lib/gamedata/class.txt`.
- What to change: add one active banner-zone state to player or upkeep state,
  create it from Marshal's Banner, tick and expire it through ordinary upkeep,
  clear it on level departure, and apply the same formation-style effects inside
  its fixed radius.
- Constraint: represent the banner as state, not a terrain feature, object,
  monster, teleport anchor, or permanent map occupant.
- Spec delta satisfied: items 8 and 9.

Run targeted tests after each mini-cycle and confirm green.

#### Refactor - Clean up while keeping tests green

- Keep banner-zone state serialization out of the first green unless tests prove
  save/load behavior is required for the same patch. If persistence is required,
  split it into a new save/load slice before docs.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
cmake --build build -t run-unittest-player-timed -j2
```

### Slice 6: Player-Facing Text, Help, And Moral Audit

#### Red - Write tests first, no implementation code yet

- Test file: add or update a focused moral-access/data audit test, preferably
  `src/tests/player/birth.c` if class text helpers are already available or
  `src/tests/effects/general.c` if spell descriptions are easier to inspect.
- What the test proves: General spell names/descriptions use clean heroic
  language and no General spell exposes teleport, summon, necromancy, spirit,
  undead, demon, blood, occult, corrupt shadow, or project-label ally names.
- Assertion strategy: data-aware class/spell traversal plus negative assertions
  against forbidden terms in General player-accessible names and descriptions.
- Existing tests to rewrite: any test fixture that still expects `Tactical
  Withdrawal`, `Banner of Courage` as a simple timed buff, or `Heroband
  infantry/archer` as player-visible names.
- Follow-up mini-cycles:
  - Cycle A: class spell names and descriptions pass the moral vocabulary audit.
  - Cycle B: docs/help/customize references match implemented General powers.
  - Cycle C: enemy-only evil references remain allowed and are not deleted as
    part of this audit.

Run:

```sh
cmake --build build -t run-unittest-effects-general -j2
cmake --build build -t run-unittest-player-birth -j2
```

Expected red: existing General spell data still includes `Tactical Withdrawal`
with teleport behavior and old temporary ally names.

#### Green - Make the red test pass, minimum change only

- Source files: `lib/gamedata/class.txt`, `lib/gamedata/monster.txt`,
  `docs/birth.rst`, `docs/hacking/heroband.rst`, `lib/help` files if they
  mention General powers, and any scripted test fixtures that display these
  names.
- What to change: update General spell names/descriptions, ally race names,
  docs, and help to match the implemented Marshal of the West mechanics.
- Constraint: do not remove enemy-only demons, undead, nether, curses, or evil
  monster content because this slice is about General player access only.
- Spec delta satisfied: item 10.

Run targeted tests and confirm green.

#### Refactor - Clean up while keeping tests green

- None expected.

Run:

```sh
git diff --check
```

## Verification

### Local Verification Sequence

1. Run targeted unit tests after each slice:

```sh
cmake --build build -t run-unittest-effects-general -j2
cmake --build build -t run-unittest-monster-monster -j2
cmake --build build -t run-unittest-player-timed -j2
cmake --build build -t run-unittest-player-birth -j2
```

2. Build the game:

```sh
cmake --build build -j2
```

3. Run the full automated suite:

```sh
cmake --build build -t alltests -j2
```

4. Run whitespace/check patch gate:

```sh
git diff --check
```

5. Create scenario-save fixtures before direct gameplay. Use
   `scripts/heroband-playtest prepare-scenario` if it supports the required
   state; otherwise add or extend a reusable scenario helper as part of the
   implementation. Do not hand-build one-off checked-in save files.

6. Run Heroband GCU direct gameplay using `$heroband-playtest` against the
   scenario-save matrix below. Every run must have a written test contract,
   saved scenario manifest, pane captures before and after each power use, and a
   stopped tmux session unless intentionally left open for debugging.

### Scenario-Save Playtest Matrix

Scenario saves are validation fixtures, not source artifacts. Generate them
under a temporary state directory and record the exact helper command, build
path, seed, save path, and manifest path in the final evidence report.

Each scenario manifest must include:

- General race, class, level, experience, stats, hit points, mana, gold, learned
  General powers, and carried books.
- Dungeon depth, seed, starting grid, terrain layout, nearby monsters, and
  whether monsters are awake, unique, resistant, grouped, or out of line of
  sight.
- Expected temporary ally tier for infantry and archer calls at that level.
- Expected formation mode: solo, archer-line ally present, melee-line ally
  present, or no valid ally.
- Expected banner-zone radius, duration, origin grid, in-radius actors, and
  out-of-radius actors.
- Exact GCU actions to take and exact player-visible messages or screen states
  that prove success.

#### Scenario A: Level 1 Town/Dungeon Smoke

- Save name: `general-l01-smoke`.
- Character: General level 1, starting kit, Field Commands known or available.
- Depth: town or dungeon level 1 with open floor around the player.
- Purpose: prove birth-visible General identity, low-tier `Call Infantry`, no
  project-label soldier name, mirrored command mode, release/expiry message, and
  no drops after fallback.
- Required GCU evidence: class/power screen showing General field commands,
  message for the infantry arrival, monster list or visible map showing one
  living ally, command release or expiry message, post-expiry grid with no corpse
  or floor item.

#### Scenario B: Level 7 Archer Tier And Early Withdrawal

- Save name: `general-l07-archer-withdrawal`.
- Character: General level 7 with `Call Archer` and `Fighting Withdrawal`.
- Depth: dungeon level 5 to 10 with a corridor, nearby melee enemy, and at least
  one legal retreat-adjacent floor square.
- Purpose: prove archer-line temporary ally selection, Fighting Withdrawal's
  defensive benefit, ordinary action cadence, and no teleport-style relocation.
- Required GCU evidence: archer arrival message with in-world race name,
  before/after player position capture for Fighting Withdrawal, timed-effect or
  status display evidence for the defensive benefit, and no sudden movement to a
  nonadjacent or random location.

#### Scenario C: Mid-Level Formation Control

- Save name: `general-l15-arrow-volley`.
- Character: General level 15 with `Arrow Volley`, once solo and once with an
  active archer-line temporary ally.
- Depth: dungeon level 15 to 20 with three non-unique enemies: two inside the
  authored formation radius and one outside it.
- Purpose: prove Arrow Volley is control-first, has modest damage, disrupts or
  slows in-radius enemies, improves with archer-line ally, and creates no extra
  monsters.
- Required GCU evidence: monster count before/after, visible messages for arrow
  support or enemy disruption, in-radius enemies slowed/disrupted or visibly
  affected, out-of-radius enemy unaffected, no new commandable actor beyond the
  preexisting temporary ally.

#### Scenario D: High-Level Charge And Banner

- Save name: `general-l30-charge-banner`.
- Character: General level 30 with `Glorious Charge` and `Marshal's Banner`.
- Depth: dungeon level 30 to 35 with a melee-line temporary ally setup,
  clustered non-unique enemies, one unique or high-resistance enemy nearby, and a
  clear landmark grid for banner origin checks.
- Purpose: prove Glorious Charge is an offensive commitment effect, banner-zone
  creation, in-radius morale benefits, in-radius enemy disruption, unique or
  resistance handling, and fixed-location banner behavior.
- Required evidence: GCU before/after player-visible evidence for Glorious
  Charge, short heroic benefit visible after charge, nearby non-unique
  disruption, unique or resistant enemy pressure not trivially disabled, and
  banner placement message; deterministic tests prove exact inside/outside
  banner-radius benefits.

#### Scenario E: Deep Pressure And Cleanup

- Save name: `general-l45-banner-cleanup`.
- Character: General level 45 with all General powers learned and enough mana or
  resources to exercise them repeatedly.
- Depth: dungeon level 45 to 60 with mixed melee/ranged pressure, one narrow
  blocked space case, and a nearby stair or controlled level-transition route.
- Purpose: prove high-tier ally selection, blocked ally/formation/banner failure
  messages, banner expiration cleanup, level-transition cleanup, and no stale
  command or banner state after leaving the level.
- Required evidence: GCU high-tier soldier arrival messages, release/cleanup,
  active banner state, level transition after active banner or ally state, and
  post-transition capture showing no stale banner benefits or controlled monster
  routing; deterministic tests prove banner expiration and no terrain/object
  residue.

#### Scenario F: Moral Regression Audit Save

- Save name: `general-moral-regression`.
- Character: General level 35 with all implemented Marshal powers and a nearby
  selection of enemy-only evil monsters or corrupt objects that are not part of
  General power.
- Depth: any controlled test depth with stable visibility.
- Purpose: prove General player powers remain clean heroic power while enemy-only
  evil remains antagonistic and separate.
- Required GCU evidence: General power descriptions/messages use tactics,
  morale, lawful command, living soldiers, formations, or banner terminology;
  no player-accessible demon, undead, spirit, blood, necromancy, occult, corrupt
  shadow, life-drain, curse-benefit, or summon-style power appears through the
  General commands.

### Subagent Verification Passes

#### Heroband test-quality verifier

Use `$heroband-test-quality-verifier` on:

- `src/tests/effects/general.c`
- `src/tests/monster/monster.c`
- `src/tests/player/timed.c`
- the GCU playtest contract and transcript

Prompt focus:

`Review the General Marshal tests for reward-hacking and moral-access gaps.
Identify any tests that would still pass if General powers created extra
commandable monsters, used teleport as the mobility identity, leaked project
names like "Heroband infantry", or granted forbidden demonic, necromantic,
blood, occult, corrupt shadow, or summon-style player power. Check whether the
GCU scenario-save matrix proves player-visible behavior rather than only
internal state, and verify each scenario manifest records level, depth, terrain,
monsters, learned powers, expected ally tier, banner radius/duration, and exact
GCU evidence.`

#### Gameplay pre-mortem pass

Use a generic/default review pass after tests are green.

Prompt focus:

`Perform a pre-mortem on the General Marshal implementation. Assume all unit
tests pass but the feature ships with a bad gameplay experience. Find likely
failure modes around ally tier balance, banner-zone cleanup, formation messages,
monster disruption on uniques, command-mode ownership, save/load or level
transition state, scenario-save setup drift, and player confusion between
temporary allies and formations.`

## Acceptance Criteria

1. `Call Infantry` and `Call Archer` choose explicit living soldier race tiers
   from player level and never expose `Heroband ` project-label race names to the
   player.
2. Temporary allies remain one commandable living soldier at a time, with mirrored
   player and monster command timers, no drops, no corpses, no experience reward,
   and clean fallback on expiry.
3. No General player-accessible spell uses `EF_TELEPORT` or player-facing
   teleport identity for class mobility.
4. Arrow Volley and Glorious Charge operate as formation effects: tactical
   benefits and monster disruption without extra monster instances, AI turns,
   commandable actors, drops, corpses, or lore sources.
5. Formation effects are weaker without a matching temporary ally and stronger
   with a valid archer-line or melee-line temporary ally.
6. Marshal's Banner creates one fixed-location banner zone with authored radius
   and duration, applies only in that area, disrupts only in-area monsters, and
   expires or clears on level transition without object or terrain residue.
7. Player-facing General names, descriptions, help, docs, and messages use clean
   heroic sources: tactics, morale, lawful command, discipline, living soldiers,
   formations, and banner-led battlefield control.
8. Enemy-only evil content remains available as antagonistic content and is not
   deleted merely because it is evil-themed.
9. Scenario saves `general-l01-smoke`, `general-l07-archer-withdrawal`,
   `general-l15-arrow-volley`, `general-l30-charge-banner`,
   `general-l45-banner-cleanup`, and `general-moral-regression` are generated
   under temporary state directories with manifests and are exercised through
   GCU captures.
10. The scenario-save manifests record level, depth, seed, terrain, monsters,
    learned powers, expected ally tier, expected formation mode, expected banner
    zone, and exact evidence steps.
11. All quality gates pass: targeted unit tests, `cmake --build build -j2`,
    `cmake --build build -t alltests -j2`, `git diff --check`,
    `$heroband-test-quality-verifier`, and `$heroband-playtest`.

## Implementation Checklist

- [x] **Slice 1 / Cycle A: low-tier infantry** - Create
  `src/tests/effects/general.c`, register it, and write the first ally-tier test.
- [x] **Slice 1 / Cycle A: RED** - Run
  `cmake --build build -t run-unittest-effects-general -j2`; observe missing
  helper or stale `Heroband infantry` failure.
- [x] **Slice 1 / Cycle A: GREEN** - Add level-tier helper and low-tier
  infantry data in `src/effect-handler-general.c` and `lib/gamedata/monster.txt`.
- [x] **Slice 1 / Cycle A: GREEN** - Rerun the same target and observe pass.
- [x] **Slice 1 / Cycles B-D** - Add high-tier infantry, archer tier, and
  no-project-label tests one at a time; run red, implement minimum green, rerun
  green after each.
- [x] **Slice 1: REFACTOR** - Consolidate tier lookup table if needed and rerun
  `run-unittest-effects-general`.

- [x] **Slice 2 / Cycle A: no General teleport data** - Add class-data test for
  Fighting Withdrawal and no `EF_TELEPORT`.
- [x] **Slice 2 / Cycle A: RED** - Run `run-unittest-effects-general`; observe
  stale `Tactical Withdrawal` / `EF_TELEPORT` failure.
- [x] **Slice 2 / Cycle A: GREEN** - Update `lib/gamedata/class.txt` and add the
  minimal non-teleport defensive effect path.
- [x] **Slice 2 / Cycle A: GREEN** - Rerun target and observe pass.
- [x] **Slice 2 / Cycle B: behavior** - Add no-location-change and defensive
  benefit test; run red, implement green, rerun green.
- [x] **Slice 2: REFACTOR** - None expected; rerun `run-unittest-effects-general`.

- [x] **Slice 3 / Cycle A: solo Arrow Volley** - Add formation no-extra-monster
  test.
- [x] **Slice 3 / Cycle A: RED** - Run `run-unittest-effects-general`; observe
  missing formation effect failure.
- [x] **Slice 3 / Cycle A: GREEN** - Add minimal formation helper and Arrow
  Volley data/effect.
- [x] **Slice 3 / Cycle A: GREEN** - Rerun target and observe pass.
- [x] **Slice 3 / Cycles B-C** - Add archer-ally stronger-control and resistance
  tests one at a time; run red, implement green, rerun green after each.
- [x] **Slice 3: REFACTOR** - Extract ally-presence classification and rerun
  `run-unittest-effects-general` plus `run-unittest-monster-monster`.

- [x] **Slice 4 / Cycle A: solo Glorious Charge** - Add benefit/no-teleport test.
- [x] **Slice 4 / Cycle A: RED** - Run `run-unittest-effects-general`; observe
  missing spell/effect failure.
- [x] **Slice 4 / Cycle A: GREEN** - Add Glorious Charge data and reuse the
  formation helper.
- [x] **Slice 4 / Cycle A: GREEN** - Rerun target and observe pass.
- [x] **Slice 4 / Cycles B-C** - Add nearby enemy disruption and melee-ally
  stronger-effect tests one at a time; run red, implement green, rerun green.
- [x] **Slice 4: REFACTOR** - Consolidate formation area scanning and rerun
  `run-unittest-effects-general`.

- [x] **Slice 5 / Cycle A: banner zone creation** - Add banner origin/radius/
  duration state test.
- [x] **Slice 5 / Cycle A: RED** - Run `run-unittest-effects-general` and
  `run-unittest-player-timed`; observe missing banner state failure.
- [x] **Slice 5 / Cycle A: GREEN** - Add one active banner-zone state and
  Marshal's Banner effect.
- [x] **Slice 5 / Cycle A: GREEN** - Rerun targeted tests and observe pass.
- [x] **Slice 5 / Cycles B-E** - Add in-radius benefits, monster disruption,
  expiration cleanup, and level-transition cleanup tests one at a time; run red,
  implement green, rerun green after each.
- [x] **Slice 5: REFACTOR** - Keep banner and formation helpers explicit; rerun
  targeted tests.

- [x] **Slice 6 / Cycle A: General vocabulary audit** - Add data-aware moral
  language test for General powers.
- [x] **Slice 6 / Cycle A: RED** - Run `run-unittest-effects-general` and observe
  stale text/name failure.
- [x] **Slice 6 / Cycle A: GREEN** - Update class data, monster names, docs, help,
  and fixtures.
- [x] **Slice 6 / Cycle A: GREEN** - Rerun targeted tests and observe pass.
- [x] **Slice 6: REFACTOR** - None expected; run `git diff --check`.

- [x] **Verification: targeted tests** - Run all targeted unit targets listed in
  the Local Verification Sequence.
- [x] **Verification: build** - Run `cmake --build build -j2`.
- [x] **Verification: full tests** - Run `cmake --build build -t alltests -j2`.
- [x] **Verification: diff check** - Run `git diff --check`.
- [x] **Verification: test quality** - Run `$heroband-test-quality-verifier` with
  the focused prompt above and address findings.
- [x] **Verification: scenario helper** - Confirm
  `scripts/heroband-playtest prepare-scenario` can generate parameterized
  General saves for class, level, depth, learned powers, terrain, monsters, and
  expected evidence; extend the helper if it cannot.
- [x] **Verification: Scenario A contract/save** - Write the contract and
  generate `general-l01-smoke` with manifest.
- [x] **Verification: Scenario A GCU** - Run `$heroband-playtest`, capture
  General identity, low-tier infantry arrival, release/expiry, and no-drop
  evidence, then stop the session.
- [x] **Verification: Scenario B contract/save** - Write the contract and
  generate `general-l07-archer-withdrawal` with manifest.
- [x] **Verification: Scenario B GCU** - Capture archer tier, Fighting
  Withdrawal defensive benefit, ordinary action cadence, and no teleport-style
  relocation.
- [x] **Verification: Scenario C contract/save** - Write the contract and
  generate `general-l15-arrow-volley` with manifest.
- [x] **Verification: Scenario C GCU** - Capture archer-line Arrow Volley
  player-visible behavior and pair it with deterministic solo/archer-line,
  in/out radius, modest damage, and no-extra-commandable-monster assertions.
- [x] **Verification: Scenario D contract/save** - Write the contract and
  generate `general-l30-charge-banner` with manifest.
- [x] **Verification: Scenario D GCU** - Capture Glorious Charge commitment,
  banner placement, morale benefits, enemy disruption, and unique pressure,
  paired with deterministic in-zone/out-of-zone and unique/resistance handling
  assertions.
- [x] **Verification: Scenario E contract/save** - Write the contract and
  generate `general-l45-banner-cleanup` with manifest.
- [x] **Verification: Scenario E GCU** - Capture high-tier ally selection,
  banner state, release behavior, and post-transition cleanup, paired with
  deterministic banner expiration and stale command/banner assertions.
- [x] **Verification: Scenario F contract/save** - Write the contract and
  generate `general-moral-regression` with manifest.
- [x] **Verification: Scenario F GCU** - Capture clean heroic General power
  language and verify enemy-only evil remains separate from General player
  powers.
- [x] **Verification: playtest report** - Summarize all scenario save paths,
  manifests, tmux state directories, captures, pass/fail outcomes, and addressed
  findings.

## Implementation Evidence

- 2026-05-25: `cmake --build build -t run-unittest-effects-general -j2`
  passed, `effects/general finished: 20/20 passed`.
- 2026-05-25: `cmake --build build -t run-unittest-game-basic -j2`
  passed, `game/basic finished: 9/9 passed`.
- 2026-05-25: `cmake --build build -t run-unittest-monster-monster -j2`
  passed, `monster/monster finished: 2/2 passed`.
- 2026-05-25: `cmake --build build -t run-unittest-player-timed -j2`
  passed, `player/timed finished: 14/14 passed`.
- 2026-05-25: `cmake --build build -t run-unittest-player-birth -j2`
  passed, `player/birth finished: 2/2 passed`.
- 2026-05-25: `cmake --build build -j2` passed.
- 2026-05-25: `cmake --build build -t alltests -j2` passed with unit
  summary `Total: 989/989 passed (100.0%)`; scripted tests passed
  `Total: 7/7`.
- 2026-05-25: A later full `cmake --build build -t alltests -j2` run exited
  successfully with scripted tests `Total: 7/7`; the aggregate unit summary
  repeated the known intermittent `game/vanguard` line at `988/989`, and
  isolated `cmake --build build -t run-unittest-game-vanguard -j2` passed
  `game/vanguard finished: 1/1 passed`.
- 2026-05-25: `git diff --check` passed.
- 2026-05-25: `cmake --build build-gcu-test -j2` passed for the GCU
  playtest executable.
- 2026-05-25: Final subagent verifier pass returned PASS after checking the
  six active scenario directories, top-level playtest report, manifests,
  deterministic tests, and the GCU/deterministic evidence split.

### Playtest Evidence

- 2026-05-25: Added `scripts/heroband-general-scenario-save` and extended
  `scripts/heroband-playtest prepare-scenario` with the General presets
  `general-l01-smoke`, `general-l07-archer-withdrawal`,
  `general-l15-arrow-volley`, `general-l30-charge-banner`,
  `general-l45-banner-cleanup`, and `general-moral-regression`.
- 2026-05-25: Fixed GCU/tmux capture startup by launching the playtest session
  with a sufficiently large pane and adding `--save-name` support for generated
  scenario saves.
- 2026-05-25: Tightened `scripts/heroband-general-scenario-save` so required
  nearby fixture monsters fail generation if they cannot be placed, and so
  manifests record exact stats, equipment/inventory, learned powers, banner
  state, RNG-seed status, and nearby actors.
- 2026-05-25: Generated and captured the final General scenario matrix under
  `/tmp/heroband-general-playtest-20260525-191504`; the top-level report is
  `/tmp/heroband-general-playtest-20260525-191504/PLAYTEST_REPORT.md`.
- Scenario A evidence:
  `/tmp/heroband-general-playtest-20260525-191504/general-l01-smoke`.
  The transcript captures a level-1 Human General calling a Westfold footman,
  command-mode control, release/fallback, and no-longer-controlled messages;
  deterministic coverage verifies no temporary ally drops.
- Scenario B evidence:
  `/tmp/heroband-general-playtest-20260525-191504/general-l07-archer-withdrawal`.
  The transcript captures a level-7 Human General with both books, live
  Fighting Withdrawal `Shield`/`Blssd` state, and the level-7 archer fixture;
  deterministic coverage verifies no teleport-style relocation.
- Scenario C evidence:
  `/tmp/heroband-general-playtest-20260525-191504/general-l15-arrow-volley`.
  The transcript captures a level-15 Human General calling an Ithilien bowman
  and then casting Arrow Volley while command mode remains active; deterministic
  coverage verifies solo/archer-line strength, radius behavior, modest damage,
  and no extra commandable monsters.
- Scenario D evidence:
  `/tmp/heroband-general-playtest-20260525-191504/general-l30-charge-banner`.
  The manifest includes required nearby kobolds and Grip; the transcript
  captures Glorious Charge, Marshal's Banner, Hero/Blssd morale state, and
  slowed nearby enemies while Grip remains a visible pressure actor;
  deterministic coverage verifies fixed banner radius, in-zone/out-of-zone
  benefits, and unique/resistance handling.
- Scenario E evidence:
  `/tmp/heroband-general-playtest-20260525-191504/general-l45-banner-cleanup`.
  The transcript captures a high-tier Gondor captain ally, release cleanup,
  banner morale state, and a focused down-stair transition to level 46 with no
  post-transition `Hero`/`Blssd`/`Cmd` status; deterministic coverage verifies
  banner expiration and no stale command/banner state across level transition.
- Scenario F evidence:
  `/tmp/heroband-general-playtest-20260525-191504/general-moral-regression`.
  The manifest includes enemy-only skeleton/quasit fixtures; the transcript
  captures clean General Battlefield Tactics names without demonic,
  necromantic, blood, occult, corrupt shadow, or summon-style player power.
- The terminal transcript is line-wrapped and sometimes merges message fragments
  with status text. Player-visible loading/menu/action evidence is recorded in
  each scenario's `transcript.txt` and `capture-*.txt`; exact radius,
  no-extra-monster, no-teleport, no-drop, and stale-state invariants are covered
  by the deterministic tests listed above.

## References

- `specs/player.md`
- `specs/monsters-combat.md`
- `specs/UBIQUITOUS_LANGUAGE.md`
- `lib/gamedata/class.txt`
- `lib/gamedata/monster.txt`
- `src/effect-handler-general.c`
- `src/list-effects.h`
- `src/player.h`
- `src/player-timed.c`
- `src/mon-timed.c`
- `src/tests/effects/suite.mk`
- `src/tests/effects/general.c`
- `src/tests/monster/monster.c`
- `src/tests/player/timed.c`
- `docs/birth.rst`
- `docs/hacking/heroband.rst`
- `scripts/heroband-playtest`
- `.agents/skills/heroband-playtest/SKILL.md`
- `.agents/skills/heroband-test-quality-verifier/SKILL.md`
