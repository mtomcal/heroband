# Vanguard Heroic Resolve Implementation Plan
## Last Line Champion redesign for Vanguard powers

> **Status: PLANNING** - Supersedes the earlier Vanguard replacement plan in
> this file. Spec-driven from `specs/player.md` version 1.3.0,
> `specs/save-load.md` version 1.1.0, and `specs/UBIQUITOUS_LANGUAGE.md`
> version 1.2.0.

## Overview

Implement the Vanguard's Last Line Champion direction: a visible Heroic Resolve
Meter that rises from qualifying enemy pressure, gains additional strength from
the live Last Stand Tier at low hit points, passively scales combat/defense
orders without being spent, and uses bounded non-magical Armor Mastery to
improve defensive orders. Preserve the existing Vanguard class slot, aggressive
armored melee bruiser niche, and enemy-only evil content. Do not reintroduce
Blackguard, bloodlust, rage, pain empowerment, self-harm rewards, corrupt
shadow, life-drain, curse-benefit, demonic, necromantic, soul-pact, or forbidden
occult player power.

## Spec Delta To Implement

1. Vanguard is the Last Line Champion: hostile pressure reveals discipline and
   endurance, not rage, blood hunger, pain empowerment, darkness, self-harm, or
   evil empowerment.
2. Vanguard has one player-visible Heroic Resolve Meter with named tiers such
   as Steady, Tested, Resolute, Unbroken, and Last Stand.
3. The meter combines two separately testable contributors: Resolve Pressure
   from qualifying enemy pressure and Last Stand Tier from current hit point
   percentage.
4. Qualifying enemy pressure is tied to hostile combat. Hostile monster damage
   qualifies; ongoing damage qualifies only when caused by a hostile source or
   when active combat context remains valid.
5. Self-damage, starvation, safe rest attrition, friendly fire, gear abuse,
   corrupt object use, ordinary curses, and farmable non-hostile damage must not
   grant Resolve Pressure.
6. Active combat for the first implementation is visible hostile monster or
   recent qualifying hostile damage.
7. Resolve Pressure decays quickly outside active combat, more slowly during
   active combat, and clears on level transition or safe rest-to-full.
8. Healing immediately reduces Last Stand Tier if hit points cross a threshold
   but does not erase valid Resolve Pressure while active combat remains.
9. Vanguard orders scale passively from the Heroic Resolve Meter and do not
   spend or decrement resolve.
10. Utility orders, currently Assess the Field and Shatter Stone, remain
    predictable and do not scale from resolve.
11. Heroic Resolve Meter tiers grant modest passive benefits and passively scale
    combat, defense, and control orders.
12. Armor Mastery enhances defensive orders when using a shield or heavy armor,
    but is not required for the meter and must not reward cursed or corrupt
    gear.
13. Save/load conditionally preserves Resolve Pressure only while active hostile
    context remains valid; Last Stand Tier is always recalculated from current
    hit points.
14. Mid-level and deep scenario-save GCU coverage is required for hostile
    pressure, low-health tiering, order scaling, save/load continuity, and moral
    language.

## Current Code State

### Already Correct

- `lib/gamedata/class.txt` already exposes the playable class as `Vanguard`
  using the `tactics` realm and clean order names.
- `src/tests/game/vanguard.c` already verifies the current order list, tactics
  realm, clean order names/descriptions, clean defensive buffs, and no
  `TMD_BLOODLUST` or `TMD_ATT_VAMP` grants.
- `src/tests/player/corruption.c` already has broad player-accessible checks for
  no bloodlust grants, no vampirism shape grants, and no playable shadow realm.
- `tests/birth/Hu-Va` already covers scripted Human Vanguard birth.
- Existing General work provides a model for spec-first mechanics, deterministic
  tests, scenario-save GCU evidence, and moral-language audits.

### Out Of Alignment

- Vanguard has no Heroic Resolve Meter state, display, tier calculation, or
  decay behavior.
- There is no tracked distinction between qualifying enemy pressure and
  nonqualifying damage.
- Current Vanguard orders are clean but mostly static; they do not scale from
  resolve or armor/shield state.
- The current `Last Stand` is an active clean buff, not the combined high-tier
  expression of enemy pressure plus low hit points.
- Save/load has no Vanguard Resolve Pressure fields or conditional persistence.
- There is no scenario-save helper or manifest for mid/deep Vanguard pressure.

### Important Constraints

Keep parser-visible class order and class slot IDs stable. Do not delete shared
`TMD_BLOODLUST`, `TMD_ATT_VAMP`, shadow book, nether, curse, drain-life, or
enemy-only evil infrastructure as part of this plan. This work is about
Vanguard player-accessible power only. Any suspicious evil reference must be
classified as player-accessible, enemy-only, harmless, or ambiguous before it is
changed.

## Moral Access Classification

- **Player-accessible clean power**: Heroic Resolve Meter, Resolve Pressure,
  Last Stand Tier, Armor Mastery, Vanguard order scaling, shield/heavy-armor
  defensive enhancement.
- **Forbidden player-accessible power**: bloodlust, rage-as-power, self-harm
  reward, pain empowerment, corrupt shadow, life-drain, curse-benefit, demonic,
  necromantic, soul-pact, forbidden occult, or corrupt-object fueled resolve.
- **Enemy-only evil to preserve**: hostile nether, curses, demons, undead,
  corrupt artifacts, Morgul weapons, monster drain/life attacks, and enemy dark
  flavor that cannot become a player benefit.
- **Ambiguous content requiring classification during implementation**:
  traps/terrain damage, poison/cut ticks, friendly fire, cursed equipment,
  corrupt equipment, and any existing damage callback that does not expose a
  clear hostile source.

## Intended Implementation Shape

Build the resolve engine before broadly changing orders. Start with pure helper
tests for tier calculation and qualifying damage. Add player state only after
the behavior contract is red. Then add display/status integration, decay,
healing, level-transition cleanup, and save/load. Scale a small set of orders
first (`Stand Firm`, `Staggering Blow`, `Last Stand`) to prove the pattern
before widening to the remaining combat/defense/control orders. Add Armor
Mastery after baseline scaling exists. Finish with scenario-save generation,
GCU playtests, docs/help text, and moral audit.

## Red/Green TDD Slices

### Slice 1: Resolve Tier Model

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/game/vanguard.c` or a new focused
  `src/tests/player/vanguard.c` if the helper belongs near player state.
- What the test proves: Heroic Resolve Meter effective tier is derived from
  Resolve Pressure plus Last Stand Tier, with named display tiers.
- Assertion strategy: call pure helpers with pressure charge counts and hit
  point percentages; assert exact tier names and effective tier ordering.
- Mini-cycles:
  - Steady at full HP with no pressure.
  - Tested/Resolute from increasing pressure charges.
  - Unbroken/Last Stand from low HP plus pressure.
  - Healing across HP thresholds lowers only the Last Stand contribution.

Run:

```sh
cmake --build build -t run-unittest-game-vanguard -j2
```

Expected red: helper/state does not exist.

#### Green - Make the red test pass, minimum change only

- Source files: likely `src/player.c`, `src/player.h`, `src/player-timed.c`, or
  a new `src/player-vanguard.c`/`.h` if the local pattern supports it.
- What to change: add a small helper for tier derivation and named tier lookup.
- Constraint: no damage hooks, display, save/load, or order scaling yet.

#### Refactor - Clean up while keeping tests green

- Keep helper names tied to Vanguard/Heroic Resolve, not generic damage.
- Keep Last Stand Tier derived from current HP; do not store it as durable state.

### Slice 2: Qualifying Enemy Pressure And Abuse Rejection

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/game/vanguard.c` or `src/tests/player/vanguard.c`.
- What the test proves: hostile monster damage grants Resolve Pressure, while
  nonqualifying damage does not.
- Assertion strategy: use a small arena with a hostile monster and direct damage
  source metadata where available. Add direct helper tests if the first engine
  hook cannot yet identify source cleanly.
- Mini-cycles:
  - Hostile monster melee/spell damage grants one pressure charge.
  - Repeated qualifying damage respects a cap.
  - Self-damage does not grant charges.
  - Starvation/safe rest attrition does not grant charges.
  - Corrupt object use and ordinary curse harm do not grant charges.

Run:

```sh
cmake --build build -t run-unittest-game-vanguard -j2
cmake --build build -t run-unittest-player-corruption -j2
```

Expected red: no Resolve Pressure state or source classification exists.

#### Green - Make the red test pass, minimum change only

- Source files: damage handling near `take_hit()` call sites, `player.h`, and
  local Vanguard helper files as needed.
- What to change: add qualifying-source classification and pressure charge
  increment only for active Vanguard characters.
- Constraint: do not make all damage grant resolve; do not alter non-Vanguard
  damage behavior.

#### Refactor - Clean up while keeping tests green

- Consolidate qualification logic so future poison/cut/trap decisions are
  explicit rather than scattered across call sites.

### Slice 3: Active Combat, Decay, Healing, And Cleanup

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/game/vanguard.c` and, if cleanup belongs elsewhere,
  `src/tests/game/basic.c` or `src/tests/player/timed.c`.
- What the test proves: pressure decays slowly in active combat, faster outside
  active combat, clears on level transition or safe rest-to-full, and healing
  affects only Last Stand Tier.
- Assertion strategy: deterministic turn advancement with visible hostile
  monster present/absent, controlled HP changes, and level-transition cleanup.
- Mini-cycles:
  - Visible hostile monster keeps active combat true.
  - Recent qualifying hostile damage keeps active combat true for N turns.
  - Out-of-combat decay is faster than in-combat decay.
  - Safe rest-to-full clears pressure.
  - Level transition clears pressure.

Run:

```sh
cmake --build build -t run-unittest-game-vanguard -j2
cmake --build build -t run-unittest-player-timed -j2
```

Expected red: no decay/cleanup behavior exists.

#### Green - Make the red test pass, minimum change only

- Source files: `src/game-world.c`, level-transition cleanup path, rest handling
  if needed, and Vanguard helper state.
- What to change: tick pressure state through ordinary world/player upkeep and
  clear it at safe boundaries.
- Constraint: decay must not damage, slow, confuse, coerce commands, or emulate
  bloodlust side effects.

#### Refactor - Clean up while keeping tests green

- Keep active-combat definition to visible hostile monster or recent qualifying
  hostile damage for the first pass.

### Slice 4: Player-Visible Meter And Documentation Hooks

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/game/vanguard.c` if UI entry helpers are testable, or a
  small parser/UI-entry test if existing patterns fit.
- What the test proves: the player-visible status names are produced from the
  effective resolve tier.
- Assertion strategy: assert exact display strings for Steady, Tested, Resolute,
  Unbroken, and Last Stand.

Run:

```sh
cmake --build build -t run-unittest-game-vanguard -j2
```

Expected red: no meter display/status exists.

#### Green - Make the red test pass, minimum change only

- Source files: `src/ui-entry.c`, player status rendering, and docs/help files
  after behavior is green.
- What to change: expose one concise Heroic Resolve Meter status. Do not expose
  internal components unless needed for debug/test output.
- Constraint: player-facing text must avoid bloodlust, rage, pain empowerment,
  self-harm, shadow, nether, life-drain, curse-benefit, demonic, necromantic,
  soul-pact, or occult wording.

#### Refactor - Clean up while keeping tests green

- Keep names and tier thresholds centralized.

### Slice 5: Passive Benefits And First Order Scaling

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/game/vanguard.c`.
- What the test proves: resolve gives modest passive benefits and scales a small
  first set of orders without being spent.
- First set: `Stand Firm`, `Staggering Blow`, and `Last Stand`.
- Assertion strategy: compare no-resolve and high-resolve state for passive
  bonuses, order durations/status effects, and unchanged pressure charges after
  casting.
- Mini-cycles:
  - Passive defensive/martial bonus appears at named tiers.
  - `Stand Firm` scales from resolve.
  - `Staggering Blow` scales from resolve.
  - `Last Stand` scales from resolve and low HP.
  - Casting orders does not spend resolve.

Run:

```sh
cmake --build build -t run-unittest-game-vanguard -j2
```

Expected red: orders are static and no passive bonuses exist.

#### Green - Make the red test pass, minimum change only

- Source files: player bonus calculation, spell/effect handlers used by Vanguard
  orders, and `lib/gamedata/class.txt` only if data expressions need adjustment.
- What to change: thread effective resolve tier into the relevant Vanguard order
  behavior.
- Constraint: no broad rebalance. Keep numbers modest and authored around
  pressure identity, not burst damage.

#### Refactor - Clean up while keeping tests green

- Extract shared order-scaling helper before widening to more orders.

### Slice 6: Remaining Combat/Defense/Control Order Scaling

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/game/vanguard.c`.
- What the test proves: combat, defense, and control orders scale; utility
  orders do not.
- Scale candidates: `Whirlwind Attack`, `Breakthrough`, `Combat Discipline`,
  `Horn of Defiance`, `Defend the Weak`, `Unbroken`, `Forceful Blow`, and
  `Brace for Impact`.
- Non-scale utility: `Assess the Field`, `Shatter Stone`.

Run:

```sh
cmake --build build -t run-unittest-game-vanguard -j2
```

Expected red: remaining orders do not yet use resolve.

#### Green - Make the red test pass, minimum change only

- Source files: Vanguard order effects and class data as needed.
- What to change: apply the shared scaling helper to remaining eligible orders.
- Constraint: no fear-lock dominance from `Horn of Defiance` or taunt-lock
  dominance from `Defend the Weak`; scaling must stay bounded.

#### Refactor - Clean up while keeping tests green

- Keep utility-order invariants close to the scaling tests.

### Slice 7: Armor Mastery Defensive Enhancements

#### Red - Write tests first, no implementation code yet

- Test file: `src/tests/game/vanguard.c` or object/player tests if equipment
  setup is easier there.
- What the test proves: shield/heavy armor enhance defensive orders only, while
  no shield/light armor still allows ordinary resolve behavior.
- Assertion strategy: compare defensive order durations/effects with shield,
  heavy armor, no shield, light armor, cursed equipment, and corrupt equipment.
- Defensive orders: `Stand Firm`, `Unbroken`, `Brace for Impact`, and maybe
  `Defend the Weak`.

Run:

```sh
cmake --build build -t run-unittest-game-vanguard -j2
cmake --build build -t run-unittest-object-corruption -j2
```

Expected red: armor/shield state does not affect defensive orders.

#### Green - Make the red test pass, minimum change only

- Source files: equipment classification helper, Vanguard order scaling helper,
  and corruption/curse checks if needed.
- What to change: add bounded non-magical armor/shield enhancement.
- Constraint: do not reward cursed gear or corrupt gear; Armor Mastery is an
  enhancer, never a hard requirement.

#### Refactor - Clean up while keeping tests green

- Keep armor category logic shared and explicit.

### Slice 8: Save/Load Conditional Persistence

#### Red - Write tests first, no implementation code yet

- Test file: existing save/load test harness if available, otherwise add a
  focused game/player persistence test following local save/load patterns.
- What the test proves: Resolve Pressure conditionally persists only while
  active hostile context remains valid; Last Stand Tier is recalculated from HP.
- Mini-cycles:
  - Combat save/load keeps pressure when hostile context remains valid.
  - Safe-context save/load clears pressure.
  - Low HP after load derives Last Stand Tier from current HP.
  - Loaded missing/old save state defaults to no banked pressure.

Run:

```sh
cmake --build build -t alltests -j2
```

Expected red: no save fields or load migration exist.

#### Green - Make the red test pass, minimum change only

- Source files: `src/save.c`, `src/load.c`, `src/player.h`, and any block
  version migration code.
- What to change: persist only the minimum Resolve Pressure state needed, then
  validate or clear it after load.
- Constraint: no durable Last Stand Tier field; no precharged safe-context
  resolve.

#### Refactor - Clean up while keeping tests green

- Keep compatibility defaults explicit for old saves.

### Slice 9: Scenario Save Helper And GCU Coverage

#### Red - Write test contract/scenario support first

- Add or extend `scripts/heroband-playtest prepare-scenario` support for
  Vanguard pressure scenarios if no generic scenario helper exists.
- What the deterministic support proves: scenario manifests can generate Human
  Vanguard saves at the required level, HP, equipment, depth, learned powers,
  and monster pressure.
- Expected red: current helper likely lacks Vanguard resolve scenario presets.

Run helper checks and targeted deterministic tests before GCU.

#### Green - Make scenario setup pass

- Source/scripts: scenario helper and manifests only; avoid engine shortcuts that
  bypass normal loaded-save GCU observation.
- Constraint: scenario helper must be reusable, not hard-coded only for one
  final transcript.

#### Refactor - Clean up while keeping tests green

- Archive failed/retry state outside active evidence roots.
- Require manifests to name exact GCU actions and expected visible states.

## Scenario-Save Playtest Matrix

### Scenario A: Birth And Low-Level Smoke

- Save name: `vanguard-l01-birth-smoke`.
- Character: Human, Vanguard, level 1, starting kit, Vanguard Drills available.
- Depth: town and first dungeon entry.
- Learned powers/inventory: starting Vanguard book and basic equipment.
- Monster setup: none for birth; optional weak hostile after dungeon entry.
- Expected mode: class is Vanguard, Blackguard absent, no Heroic Resolve from
  safe context.
- Purpose: prove birth-visible identity and no stale forbidden player-facing
  language.
- Required GCU evidence: class list showing Vanguard, character screen showing
  Vanguard, starting inventory/order list, no Blackguard/Bloodlust/shadow/nether
  player-power language.
- Cleanup/reporting: stop session, preserve contract and transcript.

### Scenario B: Mid-Level Enemy Pressure

- Save name: `vanguard-l20-enemy-pressure`.
- Character: Human, Vanguard, level 20-25, moderate HP, shield and heavy armor.
- Depth: dungeon level 20-25, open room plus corridor.
- Learned powers/inventory: `Stand Firm`, `Breakthrough`, `Staggering Blow`,
  `Defend the Weak`, enough SP/items for repeated observation.
- Monster setup: two melee non-unique enemies and one ranged/spell enemy, awake,
  visible, near enough to damage the player.
- Expected mode: Resolve advances from qualifying hostile damage, visible meter
  tier changes, orders scale, no resolve from safe waiting after combat ends.
- Purpose: prove the core meter loop under realistic mid-level pressure.
- Required GCU evidence: before/after HP and resolve tier, hostile attack
  messages, `Stand Firm`/`Staggering Blow` visible effect messages, decay after
  enemies are gone or out of active combat.
- Cleanup/reporting: manifest, transcript, deterministic test citations for
  exact charge counts and decay rates.

### Scenario C: Mid-Level Abuse Rejection

- Save name: `vanguard-l20-abuse-rejection`.
- Character: Human, Vanguard, level 20-25, shield and heavy armor.
- Depth: controlled dungeon or town-safe context with no visible hostile monster.
- Learned powers/inventory: clean healing items, one cursed non-corrupt item if
  setup can safely provide it, no corrupt object required unless testing prompt
  denial.
- Monster setup: none visible during abuse checks.
- Expected mode: self/attrition/cursed-gear contexts do not build Resolve
  Pressure.
- Purpose: prove the mechanic does not reward self-harm, safe attrition, or
  curse/corruption loops.
- Required GCU evidence: safe damage or attrition setup screen, unchanged meter
  after nonqualifying damage, no misleading resolve message.
- Cleanup/reporting: deterministic tests carry exact no-charge assertions; GCU
  only proves player-visible status/message behavior.

### Scenario D: Deep Last Stand Pressure

- Save name: `vanguard-l40-last-stand`.
- Character: Human, Vanguard, level 38-42, low HP below the chosen Last Stand
  threshold, shield and heavy armor.
- Depth: dungeon level 40-45 with controlled hostile pressure and escape route.
- Learned powers/inventory: `Unbroken`, `Last Stand`, `Forceful Blow`, `Brace
  for Impact`, clean healing source.
- Monster setup: durable non-unique melee enemy, one resistant/unique-like enemy
  if available, awake and visible.
- Expected mode: Last Stand Tier contributes to visible meter, healing lowers
  only the HP-derived contribution, pressure charges remain while active combat
  persists, orders scale without spending resolve.
- Purpose: prove the high-pressure fantasy without self-harm or bloodlust.
- Required GCU evidence: low HP visible state, resolve tier, `Last Stand` and
  `Brace for Impact` messages/status, healing across threshold and updated tier,
  no forced attacks or bloodlust-style coercion.
- Cleanup/reporting: cite deterministic tests for exact HP threshold and charge
  persistence behavior.

### Scenario E: Save/Load Continuity

- Save name: `vanguard-l35-resolve-save-load`.
- Character: Human, Vanguard, level 35, active Resolve Pressure, medium-low HP.
- Depth: dungeon level 35 with one visible hostile monster.
- Learned powers/inventory: enough to inspect meter before and after save/load.
- Monster setup: one visible hostile monster for valid combat-context reload and
  a second safe-context variant with no hostile context.
- Expected mode: combat save/load conditionally preserves pressure; safe load
  clears pressure; Last Stand Tier recalculates from current HP in both cases.
- Purpose: prove save/load semantics match `specs/save-load.md`.
- Required GCU evidence: pre-save meter, post-load meter in combat context,
  safe-context post-load meter, and no banked pre-buff in safety.
- Cleanup/reporting: manifest includes save path, hostile context, HP, resolve
  inputs, and exact load actions.

## Subagent Authorization Gate

This plan includes subagent verification passes:

- `$heroband-test-quality-verifier`: audit Vanguard gameplay, moral restriction,
  save/load, and scenario-save tests for weak assertions and reward-hacking.

Before beginning implementation, ask:

`This plan includes subagent verification passes. Are subagents authorized for this implementation run?`

If subagents are not authorized, convert those gates into local/manual review
steps or ask the user to revise the plan.

## Verification

### Local Verification Sequence

Run targeted tests after each slice:

```sh
cmake --build build -t run-unittest-game-vanguard -j2
cmake --build build -t run-unittest-player-corruption -j2
cmake --build build -t run-unittest-object-corruption -j2
cmake --build build -t run-unittest-player-timed -j2
```

Run build and full tests:

```sh
cmake --build build -j2
cmake --build build -t alltests -j2
```

Run patch and moral-audit gates:

```sh
git diff --check
rg -n "Blackguard|Bloodlust|TMD_BLOODLUST|ATT_VAMP|life-drain|life drain|shadow|nether|curse-benefit|demon|necrom|occult|rage|self-harm|pain empowerment" \
  docs lib/help lib/customize lib/gamedata/class.txt src/tests tests src
```

Classify all remaining hits as player-accessible, enemy-only, harmless,
compatibility plumbing, or ambiguous before handoff.

### Direct Gameplay Pass

Use `$heroband-playtest` for all scenarios above. Every GCU run must have:

- written `TEST_CONTRACT.md`
- scenario manifest
- clean state directory
- pane captures before and after each key action
- transcript citations for player-visible claims
- deterministic test citations for hidden mechanics
- stopped tmux session
- `scripts/heroband-playtest validate-evidence` where scenario manifests are
  present

## Acceptance Criteria

1. Heroic Resolve Meter has visible named tiers and combines Resolve Pressure
   with live Last Stand Tier.
2. Hostile monster pressure builds Resolve Pressure; self-harm, safe attrition,
   corrupt object use, curse benefit, and non-hostile loops do not.
3. Pressure decays correctly in and out of combat and clears on level transition
   or safe rest-to-full.
4. Healing lowers only the low-HP Last Stand contribution.
5. Vanguard orders scale passively from resolve and do not spend it.
6. Utility orders remain deterministic and do not scale from resolve.
7. Armor Mastery enhances defensive orders with shield/heavy armor and never
   rewards cursed or corrupt gear.
8. Save/load conditionally preserves Resolve Pressure only in valid hostile
   context and recalculates Last Stand Tier from HP.
9. Mid-level and deep scenario-save GCU playtests produce clean evidence.
10. Player-facing language remains clean heroic Vanguard language.
11. Build, full automated tests, `git diff --check`, moral audit, playtest
    evidence validation, and authorized test-quality verification pass.

## Implementation Checklist

### Slice 1: Resolve Tier Model

- [ ] RED: add tier helper tests.
- [ ] RED: run `run-unittest-game-vanguard` and observe failure.
- [ ] GREEN: implement tier derivation and tier names.
- [ ] GREEN: rerun targeted test.
- [ ] REFACTOR: centralize tier thresholds/names.

### Slice 2: Qualifying Enemy Pressure

- [ ] RED: add hostile damage and nonqualifying damage tests.
- [ ] RED: observe no pressure state/source classification failure.
- [ ] GREEN: implement pressure charge gain and abuse rejection.
- [ ] GREEN: rerun targeted tests.
- [ ] REFACTOR: consolidate qualification logic.

### Slice 3: Decay, Healing, Cleanup

- [ ] RED: add active-combat, decay, healing, rest, and level-transition tests.
- [ ] GREEN: implement upkeep/cleanup behavior.
- [ ] GREEN: rerun targeted tests.
- [ ] REFACTOR: keep active-combat definition explicit.

### Slice 4: Visible Meter

- [ ] RED: add display/status tests.
- [ ] GREEN: expose Heroic Resolve Meter status.
- [ ] GREEN: rerun targeted tests.
- [ ] REFACTOR: keep internal components hidden from normal UI.

### Slice 5: First Order Scaling

- [ ] RED: add passive and first-order scaling tests.
- [ ] GREEN: scale `Stand Firm`, `Staggering Blow`, and `Last Stand`.
- [ ] GREEN: verify orders do not spend resolve.
- [ ] REFACTOR: extract shared scaling helper.

### Slice 6: Remaining Order Scaling

- [ ] RED: add remaining order scaling and utility non-scaling tests.
- [ ] GREEN: scale eligible combat/defense/control orders.
- [ ] GREEN: rerun targeted tests.
- [ ] REFACTOR: keep utility invariants near scaling tests.

### Slice 7: Armor Mastery

- [ ] RED: add shield/heavy armor/light/no shield/cursed/corrupt gear tests.
- [ ] GREEN: implement bounded defensive Armor Mastery enhancement.
- [ ] GREEN: rerun targeted tests.
- [ ] REFACTOR: centralize armor category checks.

### Slice 8: Save/Load

- [ ] RED: add conditional persistence tests.
- [ ] GREEN: implement save/load fields and compatibility defaults.
- [ ] GREEN: rerun `alltests`.
- [ ] REFACTOR: keep Last Stand Tier derived.

### Slice 9: Scenario Saves And GCU

- [ ] RED: add/extend Vanguard scenario-save helper support.
- [ ] GREEN: generate scenario manifests for Scenarios A-E.
- [ ] GREEN: run GCU playtests with contracts and clean evidence roots.
- [ ] GREEN: validate evidence where manifests are present.

### Final Gates

- [ ] `cmake --build build -j2`
- [ ] `cmake --build build -t alltests -j2`
- [ ] `git diff --check`
- [ ] Moral-reference audit with classifications.
- [ ] `$heroband-test-quality-verifier` if subagents are authorized, otherwise
      documented local/manual test-quality review.
- [ ] Mark this plan `COMPLETED` or archive/summarize durable outcomes after
      implementation.

## Plan Lifecycle

This is the active Vanguard implementation plan and supersedes the earlier
Vanguard replacement plan that used this same file. When implementation
finishes, update the checklist, summarize durable behavior into specs/docs, and
mark this file `COMPLETED` or archive it so root-level plan files do not
accumulate stale planning state.
