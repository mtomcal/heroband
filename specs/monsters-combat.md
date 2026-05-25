# Monsters, Command Mode, AI, and Combat

Version: 1.0.0

## Overview

This specification defines monster lifecycle, monster selection and placement, monster state, monster AI, monster command control, monster-versus-player combat, monster-versus-monster combat, spell and breath attacks, monster lore, and Heroband moral boundaries for enemy-only evil.

Monsters may be evil, corrupt, demonic, undead, summoners, spellcasters, or servants of Morgoth when they are antagonists. Such enemy-only evil content is allowed and supports the good-against-evil atmosphere. The restriction is on player benefit: monster systems must not provide playable characters with forbidden evil power.

## Dependencies

This system depends on:

- Data parsing and gamedata for monster races, bases, flags, blows, blow effects, blow methods, monster spells, pain messages, projections, summons, timed monster effects, and allocation constants.
- Core game engine for dungeon chunks, grids, terrain, traps, line of sight, flow, scent, light, messages, events, turn processing, and level transitions.
- Turn engine and command dispatch for player and monster energy, command-mode remapping, forced turns, and command validity.
- Player, birth, classes, and timed effects for player level, stealth, saving throws, armor class, timed statuses, command mode, General allies, Vanguard taunts, and moral access gates.
- Objects, equipment, stores, and corruption gates for monster-held objects, floor object pickup and destruction, slays, object breakage, player resistances, and corruption distinction.
- Save/load and runtime user state for persistent monsters, monster lore, unique limits, and command-state cleanup.
- Front ends and terminal/UI input for targeting, visible messages, monster recall, monster lists, health tracking, and command-mode input.
- Documentation and player-facing help for attack descriptions, monster memory, timed status explanations, and Heroband moral terminology.

## Parameters

- Monster allocation base probability is 100 divided by rarity, multiplied by one plus monster level divided by 10. Rationale: rarer monsters should appear less often while higher-level monsters of the same rarity remain represented at their native depth.
- Out-of-depth monster generation has a one-in-25 chance outside town and may raise generated depth by up to the lesser of one quarter of generated depth plus 2 or the configured out-of-depth amount. Rationale: occasional danger spikes are part of Angband pacing but must remain bounded.
- Monster allocation must exclude ordinary dungeon monsters from town and town-only monsters from ordinary dungeon generation unless a special generation effect changes the requested level. Rationale: town and dungeon populations serve different gameplay roles.
- Seasonal monsters may appear only during their authored seasonal window. Rationale: special ambient content should not affect ordinary generation balance year-round.
- Unique monsters may have at most one live instance and must obey their maximum count. Rationale: uniqueness is a player-visible contract and a save/lore compatibility constraint.
- Force-depth monsters must not be generated above their native depth. Rationale: some encounters are explicitly depth-gated for balance and narrative pacing.
- Monster visual range and ranged spell range use the same 20-grid practical horizon as player ranged interaction. Rationale: targeting, spell projection, and command sight loss need a consistent spatial limit.
- Monsters flee up to 5 grids beyond sight range. Rationale: fleeing monsters should try to leave the player's immediate awareness without requiring unbounded pathing.
- Nearby monsters within 5 grids do not continue fleeing solely from range preference. Rationale: close engagement should remain tactically active and avoid excessive retreat loops.
- Grouped monsters may remain up to 5 grids from related group members during generation and behavior. Rationale: packs should feel coordinated without collapsing into a single stack.
- Monster movement energy cost is 100 energy. Rationale: monster turns must share the same baseline turn cost as player movement.
- Monster hit points regenerate every 100 game turns. Rationale: regeneration should matter in extended encounters without changing every individual action.
- Monster base regeneration is one percent of maximum hit points per regeneration interval, with a minimum of 1 and doubled for regenerating races. Rationale: large monsters recover proportionally while weak monsters still heal at a visible minimum.
- Monster multiplication is blocked once the level reaches the configured breeder cap and is slowed by the configured multiplication rate of 8 in crowded adjacent areas. Rationale: breeders must be threatening but must not flood the level uncontrollably.
- Monster spells may be attempted only within 20 grids and with a projectable path. Rationale: ranged attacks must remain spatially legible and respect terrain.
- Monster spell frequency and innate frequency are percentage chances authored per race. Rationale: spell cadence is a race identity and balance parameter.
- Taunt halves monster spell-casting chance. Rationale: taunt should pull enemies toward direct engagement without fully disabling ranged-capable monsters.
- A monster at its preferred range doubles its spell-casting chance. Rationale: positioning should matter and ranged monsters should benefit from achieving their intended combat spacing.
- Non-stupid monsters must not use bolt spells without a clean projectile path and must not summon when no valid summon space exists. Rationale: basic intelligence should avoid obviously wasted actions.
- Smart, badly wounded monsters may ignore ordinary damage spells half the time when considering desperate options. Rationale: intelligent monsters should sometimes prefer survival or special actions over routine damage.
- Non-innate monster spell base failure is 25 percent minus one quarter of spell power plus 3, with fear adding 20 percent and confusion or disenchantment adding 50 percent each. Rationale: status effects should visibly degrade monster spell reliability while powerful casters remain more capable.
- Stun reduces monster melee accuracy and damage by 25 percent. Rationale: stun should weaken direct offense without fully preventing action.
- Stunned monsters have a one-in-10 chance to miss their turn. Rationale: stun should sometimes deny action while remaining less absolute than hold or command.
- Confusion applies repeated 40 percent hit reduction steps to monster spell accuracy by confusion grade. Rationale: stronger confusion should increasingly impair ranged precision.
- Confusion creates at least a 30 percent erratic movement chance, increasing by grade. Rationale: confused monsters should visibly stagger, with stronger confusion worsening reliability.
- Monster timed-effect saving throw chance is the lesser of 90 percent and monster level plus the positive remainder of 25 minus half the incoming duration. Rationale: high-level monsters and short-duration effects should be harder to impose, while the cap preserves possible success.
- Unique monsters that pass the monster timed-effect saving throw receive a second successful-resistance check. Rationale: unique monsters should be noticeably harder to disable.
- Newly applied monster timed effects last at least 2 turns unless they are being cleared. Rationale: successful effects should be observable and not vanish immediately from rounding or tiny durations.
- Monster timed effects cap at their authored maximum, commonly 50 turns for stun, confusion, slow, haste, hold, command, disenchantment, and shapechange, and 10000 turns for sleep and fear. Rationale: statuses need strong upper bounds for save stability and balance.
- Glyph breaking succeeds when a random value up to glyph hardness is less than monster level; glyph hardness is 550. Rationale: wards should stop most monsters but allow powerful monsters to break through occasionally.
- Door bashing against locked doors compares monster hit points divided by 10 against door power. Rationale: stronger monsters should be more capable of forcing locks.
- Hit calculations must always preserve a 12 percent automatic hit chance and a 5 percent automatic miss chance, with effective to-hit floored at 9. Rationale: combat should avoid absolute certainty while preserving armor and skill relevance.
- Monster melee base to-hit is three times monster level, with level floored at 1, plus blow-effect power. Rationale: deeper monsters and stronger blow effects should be more accurate.
- Monster spell base to-hit is three times monster level, with level floored at 1, plus spell hit power. Rationale: spell accuracy should scale consistently with monster depth and authored spell precision.
- Armor damage reduction caps effective armor at 240 and reduces physical damage by armor divided by 400. Rationale: armor should reduce melee damage substantially but not indefinitely.
- Monster critical blows require at least 95 percent of maximum possible damage and either at least 20 damage or a damage-percent chance for weaker blows. Rationale: cuts and stuns from monster blows should occur on exceptional hits, not routine low rolls.
- Breath damage is current monster hit points divided by the projection divisor and capped by the projection's damage cap. Rationale: wounded breathers should become less dangerous while elemental caps preserve balance.
- Called ally command duration and ordinary command duration must be mirrored between player state and monster state. Rationale: command mode must have one authoritative gameplay duration from the player's perspective.

## Data Structures

- Monster race definition: stores native level, rarity, experience, armor class, average hit points, speed, light, hearing, smell, spell power, spell frequencies, flags, blows, spell flags, drop behavior, group behavior, and race-specific messages.
- Monster instance: stores current race, original race when shapechanged, position, hit points, maximum hit points, speed, energy, distance to player, target, group data, timed effects, held objects, temporary flags, and awareness state.
- Monster allocation table: stores depth-sorted race entries with base, prepared, and final probabilities for generation.
- Monster lore: stores what the player has learned about a race, including flags, spell flags, blow observations, casts, kills, deaths, drops, wakes, ignores, and other visible behavior.
- Monster timed effect: stores name, whether a saving throw applies, stacking mode, resistance flag, maximum timer, and messages for beginning, ending, and increase.
- Blow method: defines melee presentation and whether a blow can inflict cuts, stuns, or miss messages.
- Blow effect: defines melee effect semantics, hit power, damage behavior, status effects, and special handler behavior.
- Monster spell: defines spell type, hit power, messages, lore text, save message, effects, damage, breath status, innate status, and targeting behavior.
- Target state: stores whether a monster is targeting the player, a decoy, or another monster.
- Group state: stores leader and role information for packs, escorts, and bodyguards.
- Flow state: stores shared dungeon noise and scent values used by monsters to track the player when direct sight is unavailable.
- Commanded monster state: combines a monster command timer, player command timer, target fields, and optional called-ally marker.

## Behavior

- Monster race allocation must include only races with nonzero rarity, must sort by native depth, and must preserve each race's allocation identity. Test scenario: M-GEN-001.
- Monster generation must apply prepared restrictions, town and dungeon legality, seasonal gating, unique limits, force-depth rules, out-of-depth boosts, and deeper-of-several selection. Test scenario: M-GEN-002.
- Monster deletion must remove the monster from its grid, groups, target tracking, health tracking, light updates, held objects, and command state. Test scenario: M-LIFE-001.
- Monster visibility and awareness must update when camouflaged or mimicking monsters act, cast, attack, are revealed, or are deleted. Test scenario: M-LIFE-002.
- Monster processing must scan live monsters once per game turn, ignore already handled monsters, respect minimum energy, add speed-derived energy, spend movement energy before acting, skip mimics waiting in disguise, and update monster visibility afterward. Test scenario: M-TURN-001.
- Monster regeneration must occur on the configured interval and must update the tracked health display when relevant. Test scenario: M-TURN-002.
- Sleeping monsters must not act, must have sleep reduced by player stealth, aggravation, distance, and local noise, and must update lore when observed. Test scenario: M-TURN-003.
- Held or commanded monsters must skip their autonomous turns. Test scenario: M-TURN-004.
- Stunned monsters must decrement stun and may miss a turn by chance. Test scenario: M-TURN-005.
- Fear, confusion, slow, haste, disenchantment, hold, command, shapechange, and sleep timers must decrement according to their status rules and must obey resistance, stacking, notification, and maximum-duration rules. Test scenario: M-TIMED-001.
- Monster timed effects must learn visible resistance flags and must give unique monsters enhanced resistance. Test scenario: M-TIMED-002.
- Command expiration must clear stale monster-versus-monster targets, and called allies must fall back to their unit when their command ends. Test scenario: M-COMMAND-001.
- Commanded monsters must perform only supported commanded actions: move or attack, stand still, drop a held object, cast a random spell at a chosen monster target, or release command. Test scenario: M-COMMAND-002.
- Invalid commanded actions must report valid options or the specific failure reason and must not spend player energy. Test scenario: M-COMMAND-003.
- A commanded monster move must obey immobility, terrain, doors, wall-passing, wall-destruction, monster collision, and monster-versus-monster attack rules. Test scenario: M-COMMAND-004.
- A commanded monster spell must require a selected monster target and an available spell, then use ordinary monster spell effects and lore updates. Test scenario: M-COMMAND-005.
- Monster AI must first resolve web restrictions, then rouse group members, then attempt multiplication, then attempt ranged attacks, then choose movement or staggering. Test scenario: M-AI-001.
- Monsters in webs must pass, destroy, clear, or remain stuck according to their abilities, with clearing consuming the turn. Test scenario: M-AI-002.
- Monster multiplication must require available adjacent space, must be blocked for unique shapes, must respect breeder caps and arena restrictions, must preserve revealed camouflage state for offspring, and must consume the monster turn only on success. Test scenario: M-AI-003.
- Monster active state must be set by direct wall-passing proximity, injury, player view, hearing, smell, or terrain damage; otherwise the monster becomes passive. Test scenario: M-AI-004.
- Monster movement must prefer direct pursuit when possible, otherwise group tracking, retained tracking goals, flow by noise or scent, or random movement. Test scenario: M-AI-005.
- Afraid monsters must seek safety or move away, except nearby pressure may force engagement. Test scenario: M-AI-006.
- Group monsters may lure from corridors and may try to surround the player when the player is visible. Test scenario: M-AI-007.
- Bodyguards must prioritize staying near their leader and may attack the player when that also improves positioning. Test scenario: M-AI-008.
- Monsters must avoid damaging terrain unless resistant, except confused movement may stumble. Test scenario: M-AI-009.
- Monsters must handle walls and doors according to pass, smash, destroy, open, and bash abilities; confused moves may fail, message, and lightly stun the monster. Test scenario: M-AI-010.
- Monsters must attempt to break wards before entering warded grids, and failure must block movement without consuming unrelated extra effects. Test scenario: M-AI-011.
- Stronger monsters may push past or trample weaker non-unique monsters when their race abilities allow it. Test scenario: M-AI-012.
- Monsters may pick up or destroy floor objects according to race flags, must skip gold and mimicked objects, must fail safely on artifacts or harmful objects, and must message observable actions. Test scenario: M-AI-013.
- Monster ranged attacks must require spell chance, range, projectable path, visibility rules for non-player targets, and at least one usable spell after filtering. Test scenario: M-RANGED-001.
- Taunted monsters must be less likely to cast and more inclined toward direct engagement. Test scenario: M-RANGED-002.
- Non-stupid monsters must filter useless healing, haste, teleport-to, lash, spit, blocked bolts, impossible summons, and known resisted effects. Test scenario: M-RANGED-003.
- Monster spell choice must respect innate versus non-innate categories for the current attempt. Test scenario: M-RANGED-004.
- Non-innate monster spells may fail from spell failure rate; innate attacks must not use that failure check. Test scenario: M-RANGED-005.
- Monster spell messages must render seen, unseen, miss, target, pronoun, and race-specific forms without exposing impossible observations. Test scenario: M-RANGED-006.
- Monster spells with hit value 100 must always hit, hit value 0 must always miss, and other hit values must use the ordinary hit check against the player or monster target. Test scenario: M-RANGED-007.
- Monster spell saving throws must apply only when the spell defines a save message and the target is the player; a successful save must learn relevant protective runes. Test scenario: M-RANGED-008.
- Monster breath damage must depend on current hit points and projection caps, while non-breath spell damage must come from authored spell effects. Test scenario: M-RANGED-009.
- Monster melee against the player must stop when the monster cannot blow, the player dies, the level changes, the player moves away from the original grid, or no more blows remain. Test scenario: M-MELEE-001.
- Monster melee must use hit chance against player armor, disturb on hits, apply protection from evil repulsion when eligible, roll damage, reduce damage and accuracy when stunned, apply blow effects, and update lore. Test scenario: M-MELEE-002.
- Monster melee cuts and stuns must arise only from eligible critical blows and only one of cut or stun may be applied from a blow that could do both. Test scenario: M-MELEE-003.
- Monster melee misses must message only when visible and the blow method supports miss messages. Test scenario: M-MELEE-004.
- Monster-versus-monster melee must use target monster armor, ordinary blow effects, stun application where relevant, target movement/death termination, blink behavior, and lore updates. Test scenario: M-MELEE-005.
- Hit-and-run blink behavior must message when observable and teleport the attacking monster after the blow sequence. Test scenario: M-MELEE-006.
- Player armor class must reduce physical damage to the player but monster armor class must affect only chance to hit, not damage received. Test scenario: M-MELEE-007.
- Monster lore must update only for observable or otherwise knowable behavior and must saturate counters rather than overflow. Test scenario: M-LORE-001.
- Heroband must allow enemy-only demons, undead, corruption, evil summons, Morgoth, Sauron, and hostile dark powers as antagonistic content. Test scenario: M-MORAL-001.
- Monster or ally systems must not convert enemy-only evil into player-accessible demonic, undead, blood, corrupt shadow, soul-pact, necromantic, or forbidden occult power. Test scenario: M-MORAL-002.
- General allies must be living soldiers under temporary command and must not use corpse, undead, spirit, demonic, or occult summoning semantics. Test scenario: M-MORAL-003.

## Error Handling

- Monster generation must return no monster when no legal allocation entry remains after restrictions.
- Missing or invalid monster race, spell, blow, timed effect, projection, or summon data must fail data loading rather than create undefined combat behavior.
- A monster with no usable ranged spell after filtering must skip ranged attack and continue to movement.
- A monster spell with missing required visible, invisible, or miss message must report a bug message rather than producing malformed text.
- A commanded monster action without a commanded monster is invalid state and must be guarded by command-mode ownership.
- Commanded casting without a selected monster target must report that no target monster was selected and must not spend energy.
- Commanded casting by a monster with no spells must report that the monster has no spells and must not spend energy.
- Commanded movement into blocked terrain must report that the way is blocked and must not spend energy unless a valid attack or terrain interaction occurred.
- Attempting to move an immobile commanded monster must report that the monster cannot move and must not spend energy.
- Releasing command must clear both player and monster command timers.
- Deleting a commanded monster must clear player command state.
- Failed timed-effect application due to resistance must not mutate the monster timer.
- Shapechange reversion failure is fatal because it indicates corrupted monster identity state.
- Monster object pickup must leave artifacts and harmful objects safely in place when pickup or destruction is not allowed.
- Monster processing must stop promptly when the player dies or a level transition begins.

## Implementation Notes

- Keep monster evil as enemy-only unless a specific mechanic grants the player a benefit from that evil.
- Prefer gating player access over deleting monster races, monster spells, or summon categories used by enemies.
- Keep monster allocation, race identity, lore identity, and unique identity stable for compatibility.
- Treat command mode as a player command overlay that temporarily directs one monster; autonomous monster turns must skip commanded monsters.
- Called allies are temporary living units and should not drop rewards when their duty ends.
- Keep monster AI deterministic enough for tests by isolating random gates behind scenario-controlled seeds where possible.
- Monster lore should reflect what the player could observe or infer, not omniscient implementation state.
- Status-effect descriptions in documentation must match actual timed-effect effects, resistance, duration caps, and stacking rules.
- Combat calculations should share hit, armor, and damage rules between player-facing descriptions and tests.

## Test Scenarios

- M-GEN-001: Build allocation from mixed rarity and zero-rarity races and verify only legal races appear with expected depth ordering and probabilities.
- M-GEN-002: Generate monsters under town, dungeon, seasonal, unique, force-depth, restricted, and out-of-depth conditions and verify legal outcomes.
- M-LIFE-001: Delete ordinary, visible, light-emitting, health-tracked, target-tracked, object-holding, grouped, and commanded monsters and verify all state cleanup.
- M-LIFE-002: Make camouflaged and mimicking monsters act, attack, cast, be revealed, and be deleted; verify awareness and display updates.
- M-TURN-001: Process monsters with low energy, enough energy, handled flags, different speeds, and mimicking state; verify one action opportunity and energy accounting.
- M-TURN-002: Damage monsters before a regeneration interval and verify normal and fast regeneration plus health redraw.
- M-TURN-003: Process sleeping monsters under stealth, aggravation, distance noise, and visible observation; verify wake reduction and lore.
- M-TURN-004: Process held and commanded monsters and verify they skip autonomous actions.
- M-TURN-005: Process stunned monsters repeatedly with deterministic randomness and verify timer decrement and skipped turns.
- M-TIMED-001: Apply, increase, decrease, clear, over-cap, and resisted monster timed effects; verify stacking, caps, messages, and timer values.
- M-TIMED-002: Apply monster statuses to visible resistant monsters and unique monsters; verify lore and enhanced resistance.
- M-COMMAND-001: Let a commanded monster timer expire while targeting another monster and verify target clearing and called-ally fallback behavior.
- M-COMMAND-002: Issue each supported commanded action and verify action-specific behavior.
- M-COMMAND-003: Issue invalid commanded actions and verify messages and zero player energy use.
- M-COMMAND-004: Command movement through passable terrain, blocked terrain, permanent wall, destructible wall, closed door, locked door, occupied grid, and target monster.
- M-COMMAND-005: Command a spell with and without target and with spell-capable and spellless monsters; verify effects and lore.
- M-AI-001: Place an active monster with options to multiply, cast, and move; verify priority order.
- M-AI-002: Place webbed monsters with pass-web, wall-passing, wall-destroying, web-clearing, and no relevant abilities; verify turn outcomes.
- M-AI-003: Attempt multiplication for ordinary breeders, unique shapes, crowded areas, arena levels, no-space areas, and revealed camouflaged breeders.
- M-AI-004: Evaluate active state from wall-passing proximity, injury, view, hearing, smell, terrain damage, and none of those.
- M-AI-005: Test pursuit with direct sight, sound, scent, group tracker, retained tracking, and no information.
- M-AI-006: Test afraid monster safety seeking, failed safety seeking, and nearby forced engagement.
- M-AI-007: Test group lure behavior in corridors and surround behavior in open spaces.
- M-AI-008: Test bodyguard positioning relative to leader and player.
- M-AI-009: Test terrain avoidance for resistant and nonresistant monsters, including confused stumbling.
- M-AI-010: Test wall, door, locked door, and secret door handling for each relevant movement ability and confused movement.
- M-AI-011: Test ward breaking by weak and powerful monsters under deterministic random values.
- M-AI-012: Test pushing and trampling weaker monsters, equal monsters, stronger monsters, and unique blockers.
- M-AI-013: Test pickup and destruction with gold, ordinary objects, artifacts, harmful slay objects, ignored objects, and mimicked objects.
- M-RANGED-001: Test ranged attacks at valid range, beyond range, blocked path, unseen target, decoy target, and no usable spells.
- M-RANGED-002: Compare spell-cast frequency while taunted, at preferred range, and outside preferred range.
- M-RANGED-003: Verify filtering for full-health healing, no injured kin, active haste, adjacent teleport-to, distant lash or spit, blocked bolts, impossible summons, and known resisted effects.
- M-RANGED-004: Verify innate and non-innate spell attempts select only from the allowed category.
- M-RANGED-005: Verify non-innate spell failure under fear, confusion, disenchantment, and innate no-failure behavior.
- M-RANGED-006: Verify spell messages for visible, invisible, miss, target-monster, pronoun, alternate race-specific, and suppressed message cases.
- M-RANGED-007: Verify spell hit behavior for always-hit, always-miss, player target, and monster target.
- M-RANGED-008: Verify player saving throw success blocks effects and teaches relevant protective knowledge.
- M-RANGED-009: Verify breath damage from full and wounded monsters and non-breath damage from authored spell effects.
- M-MELEE-001: Test melee termination from no-blow flag, player death, level transition, player movement, and exhausted blows.
- M-MELEE-002: Test melee hit, miss, protection from evil, stunned attacker, damage effect, and lore update.
- M-MELEE-003: Test critical blow thresholds for cuts and stuns and verify only one applies when both are possible.
- M-MELEE-004: Test visible and nonvisible miss messaging for methods with and without miss text.
- M-MELEE-005: Test monster-versus-monster attacks for hit chance, damage, stun, target death, target movement, blink, and lore.
- M-MELEE-006: Test hit-and-run effects against player and monster targets and verify observable message plus teleport.
- M-MELEE-007: Compare player armor damage reduction and monster armor hit-only behavior.
- M-LORE-001: Observe attacks, spells, movement flags, drops, wakes, deaths, and kills until counters saturate; verify no overflow and no unobserved omniscience.
- M-MORAL-001: Verify enemy demons, undead, evil summons, corrupt forces, Morgoth, Sauron, and hostile dark powers remain available as antagonists.
- M-MORAL-002: Audit monster-derived player benefits and verify none grant forbidden evil power.
- M-MORAL-003: Call General allies and verify they are living temporary soldiers, leave cleanly, and are described without forbidden summoning semantics.

## Changelog

- 1.0.0: Fully authored brownfield specification for monsters, command mode, AI, combat, lore, and Heroband enemy-only evil boundaries.
