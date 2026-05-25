# Core Engine Specification

Version: 1.0.0

## Overview

The Heroband core engine MUST coordinate initialized game data, active player
state, current dungeon state, monsters, world upkeep, events, messages, level
transitions, and the main game loop.

The engine MUST remain front-end agnostic. It MAY emit events and messages, but
input collection, terminal drawing, sound playback, and store or birth
presentation belong to front-end or feature-specific systems.

The engine MUST preserve classic Angband turn, energy, dungeon, monster, object,
message, and save and load behavior unless player-accessible behavior conflicts
with Heroband's central moral rule. The player may fight evil but MUST NOT gain
power from demons, devils, evil spirits, necromancy, soul pacts, blood magic,
dark rituals, forbidden occultism, or morally corrupt shadow power.

## Dependencies

The core engine depends on Build And Test for executable configuration, Data
Parsing And Gamedata for initialized constants and runtime records, Player for
player state and timed effects, Monsters And Combat for monster turns, Objects
And Corruption for inventory and terrain interactions, Save Load for character
state persistence, and Frontends UI for command collection and event handling.

The core engine provides loop progression, world state, event signaling,
message dispatch, energy progression, and level-transition boundaries to Turn
Engine, Player, Monsters And Combat, Objects And Corruption, Save Load,
Frontends UI, and Test System.

## Parameters

| Parameter | Requirement | Rationale |
| --- | --- | --- |
| Movement energy | A normal player action MUST consume the configured movement-energy baseline unless the command is explicitly free or partial. | The baseline is the shared unit for player actions, monster turns, waiting, pickup, stores, and command-loop progress. |
| Speed-to-energy table | The engine MUST convert speed into per-turn energy with a bounded table covering the supported speed range. | Table lookup preserves classic speed behavior and avoids recalculating a nonlinear curve during the game loop. |
| World-upkeep cadence | World upkeep MUST run on the configured recurring turn cadence after monster processing and before the player receives new energy. | Poison, cuts, food, regeneration, timed effects, stores, monster generation, light, scent, and delayed movement need deterministic periodic processing. |
| Day length | Town daylight, nightfall, and ambient sound timing MUST be derived from the configured day length. | Town illumination and ambient feedback must stay synchronized with the global turn counter. |
| Store restock period | Store restock bookkeeping while the player is in the dungeon MUST use the configured store-turn period. | Store changes should advance with world time without revealing dungeon-unsafe knowledge menu details before returning to town. |
| Ambient sound quarter-day cadence | Ambient dungeon or town sound SHOULD play at quarter-day intervals and on level changes. | Periodic ambience gives front ends sound hooks without coupling sound playback to gameplay logic. |
| Distant monster generation chance | World upkeep MUST use the configured chance to attempt distant monster generation. | Monster pressure is a core pacing parameter and must remain data-driven. |
| Monster-list compaction thresholds | The engine MUST compact monster lists when approaching capacity or when the active list has excessive holes. | Stable long-running play requires bounded monster storage and efficient iteration. |
| Minimum new-level player energy | On entering a non-arena level, the player MUST have at least one movement-energy unit available unless a loaded save already has more. | Level entry must be playable immediately while preserving higher saved energy values. |
| Message history capacity | The message system MUST retain a bounded recent-message history and drop the oldest message when full. | Long sessions need useful recall without unbounded memory growth. |
| Repeated-message count limit | Consecutive identical messages of the same type MUST stack by count until the count representation is saturated. | Repeated feedback should be compact while still reflecting repeated events. |
| Generated-level flag | A level transition MUST be requested through explicit engine state and resolved at the controlled transition point in the loop. | Deferred transition prevents mid-action systems from observing partially changed dungeon state. |

## Data Structures

The core world state MUST include a global turn counter, day counter, world-level
list, current dungeon chunk, chunk archive for active generated chunks, current
player, character-generated flag, character-in-dungeon flag, and deterministic
seeds for persistent randomized content.

The player upkeep state MUST carry pending update, notice, redraw, playing,
energy-use, dropping, generate-level, arena-level, target, health, inventory,
and command-related flags used by the loop and front ends.

The event system MUST maintain per-event handler lists. Each signaled event
MUST synchronously dispatch to registered handlers with event-type-specific
data or no data.

The message system MUST maintain a newest-first bounded message history. Each
message MUST include text, type, repeat count, and links to newer and older
messages. Message type colors MUST be configurable independently of stored
message text.

The current dungeon chunk MUST be the authoritative terrain, object, monster,
trap, scent, and light state for engine processing. Level-generation requests
MUST replace or prepare this state only through the engine's level-transition
path.

## Behavior

1. Engine initialization MUST initialize game constants, modules, gamedata,
   display-neutral lists, messages, options, stores, player support, generation
   support, and random number generation before play begins.

   Test scenario: A setup test installs initialization-status handlers,
   initializes the engine, and verifies player creation and level preparation
   can use parsed constants, classes, objects, terrain, and monsters.

2. Engine cleanup MUST release active chunks, monsters, gamedata-owned runtime
   structures, event handlers, messages, and current cave state.

   Test scenario: A reset test cleans up after an active dungeon, initializes
   again, loads a saved character, and verifies no stale cave or handler state
   prevents play.

3. The play loop MUST start or load a character, enter gameplay, collect a
   front-end command, and run the engine loop until another command is needed,
   play stops, the player dies, or a new game is requested.

   Test scenario: A scripted gameplay test creates a character, enters a level,
   runs commands through the command queue, and verifies the loop returns when
   command input is needed again.

4. Player processing MUST repeatedly handle pending updates, refresh events,
   inventory overflow, forced incapacitation commands, command repeat events,
   and queued commands until an energy-consuming command occurs or no command
   remains.

   Test scenario: A command-loop test queues a forced wait while command mode is
   active and verifies the command consumes movement energy.

5. Player command cleanup MUST subtract used energy, increment total energy,
   apply terrain damage, refresh hallucination and monster visibility state,
   clear per-turn monster show flags, clear drop status, and process pending
   updates and redraws.

   Test scenario: A movement test runs a normal movement command and verifies
   player energy decreases, the command has progressed the loop, and visible
   state updates are applied.

6. The engine MUST process monsters with more energy than the player before
   allowing another player action at the same energy threshold.

   Test scenario: A deterministic combat test gives a fast monster enough
   energy to act before the player and verifies monster processing precedes the
   next player command.

7. The engine MUST process remaining monsters, reset monster readiness, process
   periodic world upkeep, grant player energy from speed, and increment the
   turn counter when no immediate player action remains.

   Test scenario: A loop-progression test runs from a low-energy player state
   and verifies turn count, player energy gain, and world upkeep cadence.

8. World upkeep MUST handle monster-list compaction, ambient sound timing,
   town daylight changes, deferred store restock bookkeeping, light-affecting
   player states, distant monster generation, damage-over-time, timed healing,
   Black Breath effects, food digestion, fainting, starvation, hit point and
   mana regeneration, timed-effect countdown, light updates, scent and noise,
   experience-drain equipment, object recharge, delayed object knowledge,
   trap timeouts, recall, and deep descent.

   Test scenario: World-upkeep tests seed individual timed states and delayed
   movement counters, advance to the upkeep cadence, and verify each effect
   updates player, world, message, and transition state as specified.

9. Level transition requests MUST flush unsafe command state, leave the old
   level cleanly, prepare the destination, run new-level housekeeping, refresh
   player state and display events, and clear the generated-level flag.

   Test scenario: A stairs test queues descent from a valid level, runs the
   loop, and verifies player depth changes, new-level state exists, messages
   are flushed, and the player has enough energy to act.

10. New-level housekeeping MUST disturb the player, clear target and health
    tracking outside arena levels, update maximum player and dungeon progress,
    flush messages, signal new-level display, update player calculations,
    refresh the display, announce feelings when applicable, search nearby
    terrain, and enforce minimum energy.

    Test scenario: A new-level test prepares a level, invokes new-level
    housekeeping, and verifies cave existence, player hit points, food state,
    display events, and minimum energy.

11. Delayed recall and deep descent MUST disturb the player, clear pending
    commands when necessary, signal level-change messages, and request level
    generation rather than directly mutating arbitrary dungeon state.

    Test scenario: A delayed-movement test sets recall or descent counters,
    advances world upkeep, and verifies command queue flushing, message type,
    target depth, and generated-level request.

12. The event system MUST synchronously dispatch events to all registered
    handlers for the event type and MUST support removing individual handlers,
    all handlers of a type, handler sets, and every handler.

    Test scenario: Event tests register multiple handlers, signal message,
    refresh, and level-display events, remove handlers by each removal mode, and
    verify only expected handlers run.

13. Messages MUST be added to history and signaled as events. Typed messages
    MUST also signal sound events when sound is enabled. Bell events MUST use
    the bell message type.

    Test scenario: Message tests add generic and typed messages, verify history
    order, repeat counts, message type, color lookup, event delivery, sound
    lookup, and bell signaling.

14. Consecutive identical messages with the same type MUST stack instead of
    creating a new history entry until the repeat count is saturated.

    Test scenario: Message tests add the same message repeatedly and verify the
    newest message count increases while history length remains stable until
    saturation.

15. The message history MUST discard oldest entries when capacity is exceeded.

    Test scenario: Message tests fill the history beyond capacity and verify
    newest messages remain available and oldest messages are gone.

16. Core processing MUST preserve Heroband moral boundaries: enemy-only evil
    world effects may harm or oppose the player, but core upkeep MUST NOT grant
    forbidden evil power as a player benefit.

    Test scenario: A moral-regression test advances upkeep with evil-themed
    hostile states active and verifies they harm, constrain, or oppose the
    player rather than creating a beneficial player power source.

## Error Handling

Invalid game modes MUST stop with a fatal error before gameplay begins.

Broken save loading MUST stop or retry through the calling game-selection flow
rather than entering the dungeon with partial state.

The engine loop MUST return to input collection when no queued command consumes
energy. A repeated prompt with unchanged energy after an internal forced command
is a loop bug.

If the player dies, stops playing, or requests level generation, monster and
world processing MUST stop at the next checked boundary.

When the monster list approaches capacity or has too many inactive slots, the
engine MUST compact it rather than allowing unbounded growth or invalid
iteration.

Message functions MUST tolerate calls before message initialization by dropping
the message rather than dereferencing missing message storage.

Event dispatch assumes registered handlers are valid. Removing handlers during
cleanup MUST leave no dangling registrations across reinitialization.

## Implementation Notes

The core engine SHOULD communicate with front ends through events and messages
instead of direct terminal calls. UI-specific prompting and rendering belong
outside core loop rules.

The engine SHOULD treat the command queue and energy-use field as turn-engine
contracts. Forced upkeep commands, incapacitation commands, and player-entered
commands must remain distinguishable by the command-dispatch rules.

The world-upkeep cadence is intentionally coarse. New per-turn effects SHOULD
state whether they belong to player command cleanup, monster processing,
periodic world upkeep, or level-transition housekeeping.

Moral restrictions SHOULD be enforced at the player-access boundary before
shared enemy-only mechanics are removed. The same effect family may be valid as
hostile content and invalid as a player benefit.

## Test Scenarios

1. Initialize, create a simple character, prepare a level, verify dungeon state,
   hit points, food state, and save compatibility.
2. Clean up and reinitialize after active play, then load the saved character
   and verify no stale runtime state remains.
3. Queue stair descent from a valid dungeon state and verify depth changes
   through level-transition handling.
4. Move away from and back to stairs, handling the possibility of monster
   interference, then verify descent still works when the player is on stairs.
5. Drop an inventory item, auto-pick it up, and verify object pile and inventory
   state after the loop.
6. Drop food, eat from the floor, and verify object pile counts after the
   command consumes energy.
7. Process forced waiting while command mode is active and verify movement
   energy is consumed.
8. Fill the message history, stack repeated messages, redefine message colors,
   look up message names and sound names, and verify events are emitted.
9. Advance world upkeep with poison, cuts, hunger, healing, Black Breath, object
   recharge, trap timeouts, recall, and deep descent, verifying messages,
   player state, and generated-level requests.
10. Run a moral regression where hostile evil effects remain harmful or
    antagonistic and no player-accessible demonic, necromantic, blood, occult,
    corrupt shadow, or soul-pact benefit is produced.

## Changelog

### 1.0.0

Initial fully authored brownfield specification for Heroband core engine
responsibilities, loop behavior, world upkeep, events, messages, and moral
boundaries.
