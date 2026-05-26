# Save, Load, and Runtime User State

Version: 1.1.0

## Overview

Save and load preserve the player, world, dungeon, object, monster, store,
quest, knowledge, message, option, and history state needed to resume a game.
Runtime user state covers generated saves, panic saves, score-related state,
preference output, lore memory, archives, and per-user files created while
playing.

Heroband save/load behavior must preserve Angband compatibility where practical
while retaining Heroband-specific moral state such as corruption and class-slot
compatibility plumbing. A save file is player state, not documentation; old
internal identifiers may remain when needed for compatibility, but loading them
must not expose forbidden player-facing power.

## Dependencies

This specification depends on these systems:

- Build and runtime initialization provide writable user, save, panic, archive,
  and score locations.
- Data parsing initializes stable object, artifact, ego, monster, terrain,
  class, race, effect, option, and store indexes before save data is interpreted.
- Core game engine owns world generation, level entry, game loop state, and
  cleanup after death or quit.
- Player lifecycle owns birth, death, winner state, corruption, timed effects,
  energy, inventory, equipment, and persistent options.
- Objects and stores own carried objects, floor objects, store inventories,
  known stock, object knowledge, artifacts, and ignore settings.
- Front ends and UI own save selection, panic-save prompts, save progress
  messages, disconnect handling, and confirmation prompts.
- Documentation and help explain save commands, panic-save limitations, runtime
  preferences, and user-state locations.

## Parameters

- Save header length: 8 bytes. Rationale: a compact fixed header lets the loader
  reject non-save files before interpreting block data.
- Save variant marker length: 4 bytes. Rationale: variant identification must be
  separated from the generic save marker so incompatible save families are not
  silently loaded.
- Block name length: 16 bytes. Rationale: fixed-size block names allow simple
  block scanning and bounded loader lookup.
- Block version length: 4 bytes. Rationale: each block type controls its own
  compatibility window without needing a single whole-file version.
- Block size length: 4 bytes. Rationale: block readers must know exactly how
  much data to read or skip.
- Block checksum length: 4 bytes. Rationale: block data corruption must be
  detected before partial state is trusted.
- Block padding alignment: 4 bytes. Rationale: aligned block data keeps reading
  consistent across platforms and block sizes.
- Initial save buffer size: 1024 bytes. Rationale: small blocks can be written
  without immediate reallocation while keeping memory usage modest.
- Save buffer growth increment: 1024 bytes. Rationale: larger blocks grow
  predictably without excessive allocation churn.
- Save path buffer capacity: 1024 characters. Rationale: generated save and
  panic paths must fit common platform path lengths while staying bounded.
- Save description capacity: 120 characters. Rationale: save-selection menus
  need a short bounded description rather than loading the full save.
- Panic save selection rule: use a panic save only when it exists and is newer
  than the normal save, and only after user confirmation. Rationale: panic saves
  are recovery artifacts and should not override a normal save silently.
- Interrupt escalation count: four user interrupts warn about unsafe exit, and
  five escalate to forced exit behavior. Rationale: accidental interrupts should
  be recoverable, while repeated interrupts indicate intent or a stuck process.
- Corruption save width: 32-bit unsigned character state. Rationale: corruption
  must survive save/load, support saturation, and remain compact.
- Vanguard resolve pressure save policy: pressure charges may persist only when
  the saved or loaded state still has valid active-combat context. Rationale:
  Heroic Resolve is enemy-pressure state, not a banked pre-buff.

## Data Structures

- Save file: a header followed by named, versioned, checksummed blocks.
- Save block: one independently versioned state section with a loader and saver
  contract.
- Description block: a short summary used by save selection without loading the
  whole game.
- Player block: persistent player identity, status, options, energy, timed
  effects, turns, corruption, Vanguard resolve pressure state, death status,
  and related character state.
- Gear and object blocks: carried, equipped, floor, and known object state.
- Store block: town store and home inventories, owners, and known stock state.
- Dungeon and chunk blocks: current level, generated level data, traps,
  monsters, objects, and persistent level information.
- Knowledge blocks: monster memory, object memory, ignore settings, artifacts,
  messages, player spells, quests, and history.
- Runtime paths: per-user locations for normal saves, panic saves, archives,
  preferences, lore, scores, and generated support files.
- Panic save: a recovery save derived from the normal save name and stored in
  the panic-save location.

## Behavior

- B1. Saving must write a complete temporary save, then atomically replace the
  prior normal save where the platform allows. Test scenarios: T1, T2.
- B2. A failed save must delete incomplete temporary output and must not mark the
  character as saved. Test scenarios: T3.
- B3. Saving must preserve the prior normal save until the replacement save has
  been written successfully. Test scenarios: T4.
- B4. Loading must reject files with an invalid save header before reading game
  blocks. Test scenarios: T5.
- B5. Loading must reject unknown block names, unsupported block versions,
  malformed block headers, short block reads, and block checksum or loader
  failures. Test scenarios: T6, T7, T8.
- B6. Save selection must display valid available saves and descriptions, offer
  a new game when allowed, and retry selection after an unusable save. Test
  scenarios: T9, T10.
- B7. Starting a game must load an existing living character when possible and
  must start birth when no living character is loaded or a new game is
  requested. Test scenarios: T11, T12.
- B8. If a newer panic save exists for the selected normal save, the player must
  be asked whether to use it. Older panic saves must be removed as stale. Test
  scenarios: T13, T14.
- B9. Panic-save creation after a severe fault must write to the panic-save
  location, report success or failure, and then exit through the signal path.
  Test scenarios: T15, T16.
- B10. User-initiated save must disturb the player, flush pending messages,
  handle pending updates, report progress, save the game, then save subwindow
  preferences and monster memory. Test scenarios: T17.
- B11. Closing a live game must protect against user interruption while saving,
  retry failed saves when interactive prompting is allowed, and then offer score
  prediction only when the terminal is mapped and not disconnecting. Test
  scenarios: T18, T19.
- B12. Closing a dead game must display death knowledge when possible and save a
  dead character, retrying failed saves only when prompting is available. Test
  scenarios: T20.
- B13. Disconnect handling must stop normal play and close the game in an orderly
  way rather than continuing to request player commands. Test scenarios: T21.
- B14. Save/load must preserve Heroband corruption across a full cleanup,
  reinitialization, and reload cycle. Test scenarios: T22.
- B15. Save/load must conditionally preserve Vanguard resolve pressure when a
  game is saved during active hostile pressure, must clear or rapidly decay that
  pressure in safe contexts, and must always recalculate the Last Stand Tier
  from current hit points after load. Test scenarios: T23, T24.
- B16. Runtime user files are generated state and must not be treated as source
  truth for gameplay rules. Test scenarios: T25.

## Error Handling

- If the save directory cannot be opened during save selection, the game must
  stop with a clear error rather than continuing with an unknown save target.
- If a selected save is unusable in selection mode, the game must return to
  selection and mark the previous choice as unusable.
- If a forced load mode receives an unusable save, the game must stop with a
  broken-save error.
- If no save file can be opened, loading must fail with a clear message.
- If a save block cannot be read by the current loader set, loading must fail
  rather than silently discarding required state.
- If save replacement fails after moving the old save aside, the old save must
  be restored where possible.
- If panic-save naming fails because the derived name cannot fit or paths are
  invalid, panic saving must fail cleanly rather than write to an ambiguous
  location.
- Signal handlers must avoid complex unsafe work before deciding whether to
  request orderly shutdown, warn, panic-save, or re-raise the signal.

## Implementation Notes

- Treat block versions as compatibility contracts. A compatibility-breaking
  state change requires a new block version and a loader for each supported
  version.
- Keep generated runtime files out of source-control decisions unless the task
  is explicitly about packaged runtime defaults.
- Load static data before interpreting save indexes.
- Preserve internal class-slot and object identity compatibility unless a
  deliberate migration has been specified and tested.
- Panic saves are recovery files, not normal saves. They should be offered
  conservatively and never silently preferred over a newer normal save.
- Save and close paths should protect critical sections from user-initiated
  interruption to avoid lock, score, or partial-save damage.
- Do not remove corruption, forbidden-access state, or moral consequences during
  load migration.
- Do not treat saved Vanguard resolve pressure as durable power. If active
  combat cannot be validated after load, clear pressure charges and leave the
  visible Heroic Resolve Meter to be derived from current hit points and future
  enemy pressure.
- Keep Last Stand Tier derived rather than saved as an independent durable
  value.

## Test Scenarios

- T1. A new game can be saved and the resulting save exists.
- T2. A save operation writes a replacement save through a temporary file.
- T3. Simulated save failure leaves the character unsaved and removes temporary
  output.
- T4. Existing save data remains recoverable if replacement cannot be completed.
- T5. A file with an invalid header is rejected as not a save.
- T6. A save with an unsupported block version is rejected.
- T7. A save with a malformed block header is rejected.
- T8. A save with block data that cannot be loaded is rejected with the block
  identified.
- T9. Save selection lists usable saves with descriptions when available.
- T10. Save selection retries after a selected save cannot be loaded.
- T11. Loading a living character resumes game state rather than entering birth.
- T12. Loading a dead character or requesting a new game enters birth.
- T13. A newer panic save triggers a user confirmation prompt before use.
- T14. An older panic save is removed and not offered.
- T15. A severe fault with unsaved generated character state attempts a panic
  save and reports success.
- T16. A severe fault with no valid panic target reports panic-save failure.
- T17. User save reports progress, saves game state, saves window preferences,
  and saves monster memory.
- T18. Closing a live game retries failed saves when interactive prompting is
  allowed.
- T19. Closing while disconnecting does not wait for score-prediction input.
- T20. Closing a dead game saves the dead character state.
- T21. A disconnect signal stops command input and proceeds to orderly close.
- T22. A saved game with nonzero corruption reloads with the same corruption.
- T23. A Vanguard saved during active hostile pressure reloads with valid
  resolve pressure only when hostile combat context remains valid.
- T24. A Vanguard saved or loaded in town, safe rest, after level transition, or
  without active hostile context reloads without banked resolve pressure, while
  Last Stand Tier is recalculated from current hit points.
- T25. Deleting generated lore or preference output does not change core
  gameplay rules after reinitialization.

## Changelog

- 1.1.0: Added conditional save/load requirements for Vanguard Heroic Resolve
  pressure and derived Last Stand Tier.
- 1.0.0: Authored full brownfield behavior specification for save, load, panic
  recovery, and runtime user state.
