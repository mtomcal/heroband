# Player, Birth, Classes, and Timed Effects

Version: 1.1.0

## Overview

This specification defines the player lifecycle from character creation through active play, including race and class selection, starting attributes, class access, timed player effects, level progression, and Heroband moral restrictions.

The player is the only playable character. The system must preserve classic Angband-style character progression while enforcing Heroband's central rule: the player may fight evil, but may not wield evil. Enemy-only evil remains valid world content, but player-accessible demonic, necromantic, blood, corrupt shadow, soul-pact, and forbidden occult power must be blocked, removed, or replaced by clean heroic alternatives.

## Dependencies

This system depends on:

- Data parsing and gamedata for race, class, realm, spell, timed-effect, ability, body, history, starting equipment, and constants definitions.
- Core game engine for world initialization, turn lifecycle, event signaling, level changes, object knowledge, stores, and quest state.
- Turn engine and command dispatch for birth commands, ordinary player commands, forced upkeep turns, energy use, repeated commands, and command-mode remapping.
- Monsters, command mode, AI, and combat for temporary living allies, commanded-monster state, formation effects, and banner zones.
- Objects, equipment, stores, and corruption gates for starting kits, item knowledge, equipment slots, corrupt object warnings, and object-derived player properties.
- Save/load and runtime user state for quickstart data, character names, persisted options, lore, and compatibility with legacy internal identifiers.
- Front ends and terminal/UI input for birth menus, prompts, display, help, and command acquisition.
- Documentation and player-facing help for class descriptions, race/class tables, spell descriptions, and Heroband moral terminology.

## Parameters

- Player maximum level is 50. Rationale: the existing progression table and class titles are balanced around a long but finite Angband-style advancement curve.
- Player maximum experience is capped by the engine-wide maximum. Rationale: experience arithmetic must not overflow and level adjustment must be deterministic after gains, drains, and save/load.
- Player stat floor is 3 and the normal birth point-buy range is 10 through 18 before race and class modifiers. Rationale: this preserves Angband's stat scale while allowing meaningful race/class differentiation.
- Maximum natural stat after restoration and increase is 18/100. Rationale: this preserves the expected Angband stat ceiling for intrinsic player growth while leaving equipment and temporary effects as separate modifiers.
- Birth point-buy budget is 20 points. Rationale: this approximates desirable results from the older roller while keeping point-based birth fast and fair.
- Birth point-buy costs are free through base 10, cost 1 point for each increase from 11 through 16, cost 2 points for base 17, and cost 4 points for base 18. Rationale: early customization should be cheap, while top-end intrinsic stats should require a meaningful tradeoff.
- Unspent birth points increase starting gold by 50 gold each. Rationale: players who accept weaker intrinsic stats should receive a modest compensating resource without exceeding ordinary starting-kit balance.
- Starting gold baseline is 600 before starting equipment value and unspent-point adjustments. Rationale: new characters need enough value to begin play, and starting equipment should count against that value.
- Player food capacity scale is 100 turns per one percent of food capacity. Rationale: hunger grades and food constants must be expressible in compact timed-effect storage while still supporting long exploration.
- Player sight range is 20 grids. Rationale: sight, targeting, monster awareness, and command loss must share a stable visual horizon.
- Player spell and missile range is 20 grids. Rationale: ranged player actions and monster ranged interactions should use a common practical combat limit.
- Player movement energy cost is 100 energy. Rationale: ordinary turns, forced turns, monster processing, and command-mode actions need a single baseline cost for turn progression.
- Birth menu selection must maintain explicit visible-choice to canonical-choice mapping. Rationale: hidden or forbidden classes must not shift user-visible menu rows into the wrong saved class identity.
- Random birth class selection must select only playable classes. Rationale: random creation must never bypass moral access gates.
- Timed effect durations and grade maxima must fit in signed 16-bit storage, with nonnegative lower bounds. Rationale: player timed state is compact and must remain stable across parsing, play, and save/load.
- Timed effect grade maxima must be strictly ascending after the implicit off grade. Rationale: grade lookup must be deterministic and status transitions must not produce ambiguous messages.
- A nonstacking timed effect must not extend while already active. Rationale: effects such as paralysis should not be indefinitely extended by repeated applications that are meant to be blocked.
- General ally call duration is determined by the relevant field-command dice expression. Rationale: ally command time must scale by the authored tactical power rather than by monster summoning rules.
- General temporary ally tier is determined by player level, not dungeon depth. Rationale: the General's command authority should scale with character progression rather than creating out-of-depth soldier spikes.
- General formation effects scale from player level expressions and may become stronger when a temporary ally is active. Rationale: late General power should come from leadership, tactics, and morale without requiring multiple controllable allies.
- General banner zones have authored radius and duration and remain fixed to their planted dungeon location until expiration. Rationale: Marshal's Banner should provide area control without becoming teleportation, terrain mutation, or a permanent object.
- Vanguard drill durations and blow counts are determined by their authored dice expressions and player-level expressions. Rationale: martial powers should scale predictably with level while remaining clean heroic tactics.

## Data Structures

- Player record: stores identity, race, class, shape, body, level, experience, depth, gold, stats, hit points, spell points, timed effects, equipment, inventory, quiver, object knowledge, options, history, command state, and upkeep flags.
- Race definition: contributes stat adjustments, skills, hit die, experience factor, infravision, age, height, weight, history, innate object properties, innate player flags, and innate element values.
- Class definition: contributes stat adjustments, skills, hit die, experience factor, maximum attacks, minimum weapon weight, strength multiplier, titles, starting equipment, books, spells, flags, and realms.
- Birth state: tracks selected race and class, rolled or point-buy stats, point costs, point budget, previous roll, quickstart candidate, name, history, physical traits, and starting gold.
- Body definition: defines equipment slots available to the player after birth, derived from race.
- Spell realm and book definitions: define what classes can study and cast, including tactical books used by Heroband heroic classes.
- Timed effect definition: defines effect name, display grades, messages, message type, failure conditions, temporary resistances, temporary brands or slays, object-flag synonyms, transition effects, lower bounds, and nonstacking behavior.
- Player upkeep record: accumulates pending redraws, recalculations, command working state, energy use, inventory/quiver arrays, tracked health target, and level-transition requests.
- Player knowledge record: stores object, rune, flavor, artifact, monster, and innate knowledge visible to the player.
- Temporary ally state: uses ordinary monster state plus a player command timer and an ally marker to represent a living soldier currently under battlefield command.
- Formation state: uses timed tactical effects, projectable effects, or anchored area state to represent supporting soldiers without creating additional controllable monsters.
- Banner zone state: stores the location, radius, duration, and tactical effects of an active Marshal's Banner.

## Behavior

- Character creation must proceed as ordered birth commands that always leave the in-progress character in a playable state after each accepted choice. Test scenario: P-BIRTH-001.
- A new character must default to the first available race and class when no valid quickstart character exists. Test scenario: P-BIRTH-002.
- Quickstart may be offered only when the previous character has complete birth data and a playable class. Test scenario: P-BIRTH-003.
- Quickstart must increment a valid trailing Roman numeral name suffix when possible and must report failure without corrupting the name when it cannot. Test scenario: P-BIRTH-004.
- Race selection must update race, history, physical traits, stat generation, hit points, experience factor, innate properties, and displayed birth values. Test scenario: P-BIRTH-005.
- Class selection must reject unplayable classes with a clear Heroband availability message and must leave the previous valid class in place. Test scenario: P-BIRTH-006.
- The old Necromancer and Blackguard player-facing classes must be unplayable. Test scenario: P-MORAL-001.
- General and Vanguard must be playable heroic replacements, even if compatibility plumbing still uses legacy internal class slots. Test scenario: P-MORAL-002.
- Player-facing class powers must not grant demonic, necromantic, blood, corrupt shadow, soul-pact, or forbidden occult power. Test scenario: P-MORAL-003.
- Enemy-only evil, including evil monsters and evil monster powers, may remain as antagonistic content and must not be removed merely because it is evil-themed. Test scenario: P-MORAL-004.
- General powers must use clean heroic tactics, morale, field command, formations, living temporary allies, and banner-led battlefield control. Test scenario: P-GENERAL-001.
- General powers must not use literal teleportation as the class mobility identity; orderly movement and survival should be expressed through Fighting Withdrawal, defensive timed effects, monster disruption, or ally cover. Test scenario: P-GENERAL-002.
- Birth menus must display only playable classes and must submit canonical class choices, not visible row numbers. Test scenario: P-BIRTH-007.
- Random class completion must select only playable classes and must submit the selected canonical class identity. Test scenario: P-BIRTH-008.
- Point-based birth must start all intrinsic stats at base 10, spend from the point budget, disallow invalid stat choices, disallow buying above base 18, disallow selling below base 10, and recalculate displayed derived values after accepted changes. Test scenario: P-BIRTH-009.
- Automatic point allocation must prioritize strength, useful dexterity breakpoints, spell stat when relevant, constitution, and remaining useful secondary stats according to class role. Test scenario: P-BIRTH-010.
- Standard rolling must produce valid stats, preserve previous rolls for one-step retrieval, lock out point buying and selling while using rolled stats, and recalculate visible values. Test scenario: P-BIRTH-011.
- Accepting a character must roll final hit points, clear old history, record the quest start, initialize body slots, initialize spells, initialize stores and dungeon state, initialize object/flavor/rune knowledge from options, apply starting equipment, mark the character alive and generated, and leave birth mode. Test scenario: P-BIRTH-012.
- Starting equipment must be drawn from class definitions, filtered by birth options, marked as birth-origin, made known or aware as appropriate, charged against starting gold, carried, and auto-wielded when possible. Test scenario: P-BIRTH-013.
- Player level must adjust upward or downward from current experience using race and class experience factor, restore core stats on level gain, record level-gain history when verbose, and never exceed the level cap. Test scenario: P-PROGRESS-001.
- Player stat increases must respect the natural cap, update current and maximum stat values, and schedule bonus recalculation. Test scenario: P-PROGRESS-002.
- Player stat decreases must not reduce below the stat floor, may be temporary or permanent, and must schedule redraw and bonus recalculation only when values change. Test scenario: P-PROGRESS-003.
- Timed effects must clamp to their lower and upper bounds, ignore exact no-op changes, choose the correct grade, and return whether the player was notified. Test scenario: P-TIMED-001.
- Timed effect grade increases must issue the grade increase message even when normal notification is suppressed. Test scenario: P-TIMED-002.
- Timed effect decreases and clearing must issue recovery or grade-down messages only when requested or when the effect fully ends, except where grade messages require notification. Test scenario: P-TIMED-003.
- Timed effects that duplicate already-known permanent or equipped properties must suppress redundant messages while still updating state when appropriate. Test scenario: P-TIMED-004.
- Timed effects may be resisted by object flags, player flags, elemental resistance, elemental vulnerability, or other active timed effects, and monster-origin effects must update monster learning when resisted. Test scenario: P-TIMED-005.
- Nonstacking timed effects must refuse duration increases while already active, including increases that bypass ordinary failure checks. Test scenario: P-TIMED-006.
- Timed effect start and end transitions must execute their authored transition effects in order. Test scenario: P-TIMED-007.
- Player command mode must maintain exactly one currently commanded ally or monster from the player's perspective. Test scenario: P-COMMAND-001.
- Calling a General ally must create the appropriate player-level tier of living infantry or archer ally near the player when space allows, wake it, mark it as command-controlled, set both player and ally command timers, track its health, and report the ally's arrival. Test scenario: P-COMMAND-002.
- Calling a General ally must fail without consuming command control if the player is already directing an ally, if no valid ally subtype exists, if the ally race is unavailable, if no soldier can reach the player, or if the arena blocks assistance. Test scenario: P-COMMAND-003.
- Command mode must release the controlled monster when the player uses the release command, when the controlled monster leaves line of sight, when the monster is deleted, when its command timer expires, or when the player leaves the level. Test scenario: P-COMMAND-004.
- A called ally whose command expires must leave play as falling back to its unit, not as a corpse, hostile monster, or loot source. Test scenario: P-COMMAND-005.
- While command mode is active, ordinary player intent must be remapped to commanded-monster actions, but forced upkeep commands such as paralysis and knockout sleep must still consume the player's turn. Test scenario: P-COMMAND-006.
- Commanded-monster invalid actions must report why they failed and must not consume player energy. Test scenario: P-COMMAND-007.
- Formation effects must remain usable without a temporary ally at reduced strength when authored that way, and must apply their stronger ally-present mode only when the player currently controls a valid temporary ally. Test scenario: P-GENERAL-003.
- Formation effects may represent multiple supporting soldiers in messages or spell descriptions, but must not create additional controlled monsters, corpses, drops, or experience sources. Test scenario: P-GENERAL-004.
- Arrow Volley must be a control-first formation effect: it may deal modest area missile damage, but its primary scaling value is slowing, disrupting, or pressuring enemies, with improved effect when an archer-line temporary ally is active. Test scenario: P-GENERAL-005.
- Fighting Withdrawal must replace teleport-style General mobility with defensive tactical withdrawal: it spends ordinary action energy, grants defensive benefit, may reposition the player by legal non-teleport movement when implemented, and may improve ally cover when a temporary ally is active. Test scenario: P-GENERAL-006.
- Glorious Charge must be an offensive commitment power, not an escape tool: it grants short heroic combat benefits and may disrupt nearby non-unique enemies, with improved effect when a melee-line temporary ally is active. Test scenario: P-GENERAL-007.
- Marshal's Banner must create a temporary banner zone with authored radius and duration. The zone must grant morale or formation benefits to the player and temporary ally within it, may disrupt enemies in its area, and must expire without leaving an object, terrain feature, corpse, or permanent map state. Test scenario: P-GENERAL-008.

## Error Handling

- Invalid race names or class names supplied by noninteractive creation must fail the creation request rather than creating a partially initialized player.
- Unplayable class selection must report that the class is not available in Heroband and must preserve the previous valid player state.
- Birth menus must tolerate missing race or class records by skipping invalid entries and never submitting a nonexistent canonical choice.
- Random name generation must abort with an explicit bug message if it cannot find an unused save name after repeated attempts.
- Invalid timed effect names, flags, grades, colors, messages, expressions, brands, slays, and lower bounds must be rejected during data loading.
- Setting a timed effect to its current value must be a no-op and must not disturb the player.
- Attempting to increase a resisted timed effect must fail cleanly and may teach relevant resistance knowledge.
- Attempting to call an ally in blocked situations must report the specific player-facing reason and must not leave stale command state.
- Attempting to create a formation or banner zone in a blocked situation must report the specific player-facing reason and must not leave stale area state.
- Deleting or losing a commanded monster must clear player command state.
- Expiring a banner zone must clear its area state even if the player, temporary ally, or monsters have left the area.
- Leaving a level must clear command state before post-level processing continues.
- Leaving a level must clear banner zones and formation state before post-level processing continues.
- Forced player upkeep while command mode is active must not enter a no-energy loop.

## Implementation Notes

- Preserve legacy internal identifiers when needed for parser and save compatibility, but never expose forbidden player-facing class names or powers as playable choices.
- Prefer data-level replacement of forbidden player powers with clean heroic equivalents when that preserves parser and save stability.
- Do not remove enemy-only evil content as part of player access gating.
- Keep birth UI choices decoupled from storage identifiers so hidden classes cannot corrupt selection.
- Keep timed effect definitions data-driven, but treat their names and ordering as compatibility-sensitive.
- Treat command mode as a temporary control overlay, not as a second player character.
- Treat called General allies as living soldiers under command, not summoned spirits, undead, demons, or conjured monsters.
- Use in-world military names for General ally monster races rather than project labels in player-visible names.
- Prefer distinct monster race tiers for General temporary allies over hidden runtime mutation of a single race's stats.
- Treat formations as tactical effects and messages, not fake map monsters. If a formation appears on the map, it must use a real inspectable mechanic with explicit rules.
- Treat Marshal's Banner as a temporary battlefield zone before considering a real object or terrain implementation.
- Player-facing documentation, help, class tables, spell descriptions, and tests must remain consistent with playable behavior.

## Test Scenarios

- P-BIRTH-001: Step through birth one command at a time and verify that after each accepted race, class, stat, name, or history command the character has a race, class, stats, hit points, and displayable derived values.
- P-BIRTH-002: Start birth with no quickstart candidate and verify the default choices are valid playable records.
- P-BIRTH-003: Start birth from a complete playable previous character and from a previous forbidden class; verify quickstart is offered only for the playable case.
- P-BIRTH-004: Quickstart characters with valid, invalid, absent, and too-long Roman numeral suffixes; verify successful increment or explicit failure without name corruption.
- P-BIRTH-005: Select multiple races and verify stat modifiers, skills, physical traits, history, hit die, experience factor, and innate properties update.
- P-BIRTH-006: Attempt direct class selection of forbidden classes and verify rejection, message, and unchanged valid class.
- P-BIRTH-007: Open the class menu and verify forbidden classes are absent while every displayed class selects its own canonical identity.
- P-BIRTH-008: Use random completion repeatedly under deterministic seeds and verify no forbidden class is selected.
- P-BIRTH-009: Buy, sell, overbuy, oversell, and use invalid stat choices; verify point totals, stat values, gold, and derived values.
- P-BIRTH-010: Generate automatic point-buy stats for martial, caster, and hybrid classes and verify the allocation follows role priorities without exceeding budget.
- P-BIRTH-011: Use the standard roller, reroll, retrieve previous roll, and attempt point-buy commands afterward; verify rolled stats and lockout behavior.
- P-BIRTH-012: Accept a character and verify generated state, starting history, body slots, spells, stores, object knowledge, equipment, alive status, and exit from birth.
- P-BIRTH-013: Toggle starting-kit and recall birth options and verify starting equipment, known state, gold deduction, carrying, and auto-wield behavior.
- P-MORAL-001: Verify Necromancer and Blackguard are not playable by UI, random birth, and noninteractive creation paths.
- P-MORAL-002: Verify General and Vanguard are playable and documented as heroic classes.
- P-MORAL-003: Audit player spells and class descriptions for forbidden power sources and verify any replacements use discipline, courage, command, tactics, healing, light, or other clean heroic sources.
- P-MORAL-004: Verify evil monsters, undead, demons, and enemy spell summons can remain enemy-only without granting player access to their evil power.
- P-GENERAL-001: Audit General spells, descriptions, books, help, and messages and verify they use tactics, morale, lawful command, formations, living temporary allies, or banner-led battlefield control.
- P-GENERAL-002: Inspect General mobility powers and verify they do not use literal teleportation as the General class identity.
- P-PROGRESS-001: Gain and lose experience around level thresholds and verify level, maximum level, stat restoration, history, and redraw behavior.
- P-PROGRESS-002: Increase stats below 18, between 18 and 18/99, at 18/99, and at 18/100; verify current and maximum stat results.
- P-PROGRESS-003: Temporarily and permanently drain stats at low, normal, and high values; verify floors, maximum changes, and recalculation flags.
- P-TIMED-001: Set timed effects to negative, zero, current, within-grade, above-grade, and above-maximum values and verify clamping and notification return.
- P-TIMED-002: Increase effects across grade boundaries with notification disabled and verify grade messages still occur.
- P-TIMED-003: Decrease and clear effects with notification enabled and disabled and verify recovery messages and disturbance rules.
- P-TIMED-004: Apply temporary properties already supplied by known equipment or innate state and verify redundant messages are suppressed.
- P-TIMED-005: Apply timed effects blocked by object flags, player flags, resistances, vulnerabilities, and other timed effects; verify failure and learning behavior.
- P-TIMED-006: Reapply an active nonstacking effect and verify duration is unchanged.
- P-TIMED-007: Define start and end transition effects in data and verify they execute once and in order.
- P-COMMAND-001: Enter command mode and verify only one commanded monster is tracked for player command routing.
- P-COMMAND-002: Cast General ally calls with space available and verify living ally creation, command timers, health tracking, and arrival message.
- P-COMMAND-003: Attempt ally calls while already commanding, in an arena, with invalid subtype, missing ally race, and blocked spaces; verify clean failure.
- P-COMMAND-004: Release command mode by release command, line-of-sight break, monster deletion, timer expiry, and level departure; verify both player and monster command state clear.
- P-COMMAND-005: Let a called ally expire and verify it falls back, disappears, and leaves no player-beneficial corpse or drops.
- P-COMMAND-006: While command mode is active, process ordinary movement as monster command and forced sleep as player energy use.
- P-COMMAND-007: While command mode is active, attempt invalid movement, casting without target, casting without spells, dropping without held objects, and invalid commands; verify messages and no energy use.
- P-GENERAL-003: Use a formation with and without a temporary ally and verify reduced solo behavior and stronger ally-present behavior.
- P-GENERAL-004: Use formation powers and verify no additional controllable monsters, drops, corpses, or experience sources are created.
- P-GENERAL-005: Use Arrow Volley against multiple enemies with and without an archer-line temporary ally and verify modest area damage plus control-first disruption.
- P-GENERAL-006: Use Fighting Withdrawal when retreat space exists and when blocked; verify action energy, defensive benefit, no literal teleport, and clean fallback behavior.
- P-GENERAL-007: Use Glorious Charge while advancing into danger and verify short heroic benefits, enemy disruption, and no escape-style teleport behavior.
- P-GENERAL-008: Plant Marshal's Banner and verify banner zone radius, duration, player and ally benefits, enemy disruption, expiration cleanup, and no object or terrain residue.

## Changelog

- 1.1.0: Added the planned Marshal of the West direction for General: level-tiered
  temporary allies, formation effects, Fighting Withdrawal, Glorious Charge, and
  Marshal's Banner banner zones.
- 1.0.0: Fully authored brownfield specification for player lifecycle, birth, playable classes, timed effects, and command-mode player restrictions.
