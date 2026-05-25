# Parameters

Version: 1.0.0

## Overview

This spec collects shared tunable values, limits, timings, and compatibility
constants that other specs rely on. Values may be data-defined or engine-defined,
but requirements here describe their required behavior rather than their storage
location.

## Dependencies

This spec depends on the vocabulary in [UBIQUITOUS_LANGUAGE.md]. It is used by
the build, data, core engine, turn engine, player, monster, object, save, UI,
test, documentation, and release specs.

## Parameters

- Full movement energy: the baseline cost for a normal player or monster action
  is 100 energy units. Rationale: one full action must be comparable across
  movement, waiting, forced sleep, and many object or spell uses.
- Half movement energy: selected equipment, inventory, and refill actions may
  cost 50 energy units. Rationale: short manipulations should advance time while
  remaining cheaper than a full move.
- Three-quarter spell energy: selected fast spellcasting effects may reduce a
  spell action to 75 energy units. Rationale: speed-modifying class mechanics
  need a consistent fractional action cost.
- Command queue capacity: the command queue holds a bounded number of pending
  commands, currently 20 slots. Rationale: queued input and background commands
  must be finite and safe to flush.
- Speed energy table range: actor speed is converted through a bounded table of
  200 entries. Rationale: speed-to-energy conversion must be deterministic and
  fast during turn processing.
- Maximum effective speed gain: high speed approaches an asymptotic gain near
  50 percent of normal action cost. Rationale: speed should remain valuable
  without allowing unlimited action multiplication.
- Day cycle: day and night are determined from a configurable day length in
  turns, with daytime occupying the first half of the cycle. Rationale: town and
  time-of-day behavior require deterministic phase boundaries.
- Maximum dungeon depth: dungeon traversal is bounded by the configured world
  depth. Rationale: level changes, save validation, and quest placement need a
  stable maximum.
- Visual sight range: line of sight and local detection are bounded by a maximum
  sight range. Rationale: visibility, monster knowledge, and detection effects
  must share the same spatial assumptions.
- Projectile and spell range: ranged effects are bounded by a maximum range.
  Rationale: targeting, spell descriptions, and combat balance must agree.
- Pack size: inventory capacity is configurable and includes one extra working
  slot for inventory processing. Rationale: object pickup, combine, reorder, and
  save/load behavior need a stable capacity model.
- Quiver size and slot size: quiver inventory is separately bounded by slot count
  and per-slot missile count. Rationale: ammunition handling needs predictable
  capacity distinct from ordinary pack space.
- Store count: the number of stores is data-defined and must match save/load
  expectations. Rationale: store identities and saved inventories depend on
  stable counts.
- Store turnover interval: stores refresh stock after a configurable number of
  turns. Rationale: store economy behavior must be deterministic enough for
  tests and saves.
- Store owner shuffle chance: store owners may change according to a configurable
  daily chance. Rationale: owner changes are part of the inherited economy but
  must remain bounded.
- Normal store magic level: ordinary store stock generation uses a configured
  item-magic level. Rationale: store inventory quality should be tuned without
  changing store logic.
- Paralysis maximum grade: paralysis has a non-stacking maximum grade that
  includes knockout. Rationale: repeated paralysis should not grow unbounded and
  forced-turn handling should remain predictable.
- Bloodlust grades: bloodlust has multiple increasing grades up to an insatiable
  maximum. Rationale: legacy and corrupt timed effects need explicit boundaries
  for moral-access review.
- Scenario save isolation: generated playtest saves must use isolated runtime
  directories. Rationale: verification must not mutate a user's live saves,
  archive, panic saves, or scores.

## Data Structures

- Constant registry: the loaded set of numeric limits used by the game engine,
  parsers, inventory, stores, world, and UI.
- Timed-effect grade table: named severity ranges and messages for temporary
  player and monster effects.
- Energy table: a deterministic mapping from actor speed to energy gained per
  game turn.
- World-depth table: ordered level names and up/down transitions.
- Store configuration: store count, inventory capacity, owner behavior, and
  turnover timing.
- Inventory configuration: pack, quiver, and quiver-slot capacities.

## Behavior

- Shared parameters MUST be loaded before systems that allocate player upkeep,
  store arrays, world levels, timed effects, or parser-owned tables.
- Requirements that use action cost SHOULD express it relative to full movement
  energy unless the exact numeric value is part of compatibility.
- A full forced upkeep turn MUST consume full movement energy.
- Invalid or canceled commands MUST NOT spend energy merely because they entered
  the queue.
- Speed conversion MUST be deterministic for players and monsters and MUST use
  the same baseline movement energy.
- Save/load validation MUST reject or repair values that exceed configured world
  or table bounds when safe repair is possible.
- Store and inventory limits MUST be treated as compatibility-sensitive because
  saved objects and UI menus depend on them.
- Parameters that affect player access to suspicious powers MUST be reviewed
  through the moral-access categories in [UBIQUITOUS_LANGUAGE.md].

## Error Handling

- If a required parameter cannot be loaded, dependent systems MUST fail
  initialization rather than run with silent defaults.
- If a saved store count differs from the current configured store count, loading
  MUST report the mismatch and avoid corrupting active store state.
- If a level depth is outside the configured world range, loading or traversal
  MUST reject or normalize it according to save compatibility rules.
- If a command queue push would exceed capacity, the push MUST fail without
  corrupting queued commands.
- If a moral-risk parameter makes forbidden player power accessible, the
  parameter value is invalid for Heroband even if it was valid upstream.

## Implementation Notes

These parameter requirements intentionally do not prescribe whether a value comes
from generated constants, text data, compile-time definitions, or save metadata.
The observable contract is that all dependent systems agree on the value during
one game session and preserve compatibility across saves where required.

## Test Scenarios

- Energy baseline: execute a normal wait or forced sleep action and verify it
  spends full movement energy.
- Fractional action cost: perform equipment or refill actions that are intended
  to be shorter than a full move and verify they spend the documented fraction.
- Queue capacity: fill the command queue to capacity, verify the next push fails,
  then pop commands and verify ordering remains intact.
- Speed conversion: compare player and monster energy gain at slow, normal, and
  fast speeds and verify both use the same conversion rule.
- Store compatibility: load a save whose store count matches current data and
  verify all stores retain owner and stock state; load a mismatched fixture and
  verify the mismatch is reported.
- Inventory capacity: fill pack and quiver to their configured limits and verify
  overflow is rejected or redirected through normal item handling.
- Moral parameter audit: verify bloodlust, corrupt shadow, life-drain, curse
  benefit, demonic, necromantic, and occult parameters are not reachable as clean
  player class benefits.

## Changelog

- 1.0.0: Initial shared parameter catalogue extracted from command processing,
  energy accounting, world timing, inventory, stores, timed effects, and
  Heroband validation guidance.
