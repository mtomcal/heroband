# Front Ends and Terminal UI Input

Version: 1.0.0

## Overview

Front ends translate platform-specific windows, terminals, keyboard events,
mouse events, colors, fonts, graphics, sounds, resize events, suspend events,
and disconnect events into the shared terminal and UI contracts used by the
game. Terminal UI input translates those events into prompts, menus, command
requests, target selection, item selection, help browsing, message prompts, and
game commands.

Front ends own presentation mechanics. They must not own gameplay rules,
Heroband moral policy, command energy semantics, save compatibility, or object
availability. Shared UI code may ask questions and collect player intent, but
the gameplay systems decide whether an action is legal and what state changes.

## Dependencies

This specification depends on these systems:

- Build configuration selects available front ends and optional sound or
  graphics support.
- Core game engine provides command requests, game-loop state, messages, events,
  redraw flags, panel state, and save or close hooks.
- Turn engine owns command energy, repeated command cancellation, forced turns,
  and command-mode remapping.
- Player lifecycle provides status, map position, target state, equipment,
  inventory, options, and moral warnings.
- Objects and stores provide item lists, store menus, object prompts,
  confirmation requirements, and corruption warnings.
- Save and load provide save selection, panic-save prompts, save progress, and
  disconnect close behavior.
- Documentation and help provide command summaries, option descriptions, keymap
  guidance, in-game help text, and front-end customization guidance.

## Parameters

- Minimum primary terminal size: 80 columns by 24 rows. Rationale: core map,
  message, prompt, inventory, and help displays assume this classic roguelike
  minimum.
- Maximum terminal grid coordinate value: 255 in each dimension. Rationale:
  terminal buffers store bounded grid coordinates and front ends must not report
  larger logical sizes to shared terminal code.
- Maximum curses terminal windows: 6. Rationale: the terminal front end must
  offer useful subwindows while remaining manageable on a single text display.
- Comfortable subwindow size: 40 columns by 5 rows. Rationale: default
  secondary terminal panes need enough room for compact lists without consuming
  the whole display.
- Main-screen message prompt reserve: 8 columns. Rationale: top-line message
  wrapping must leave room for the continuation prompt.
- Input scan delay unit: 10 milliseconds. Rationale: macro and keymap scanning
  needs short waits that are responsive without busy-waiting.
- Message maximum render length: 1000 characters. Rationale: player messages
  should be bounded before display splitting and prompt handling.
- Keymap auto-more delimiters: one delimiter disables message prompts within a
  keymap and a matching delimiter reenables them. Rationale: scripted keymaps
  need a controlled way to prevent message prompts from consuming later keys.
- More-prompt automatic clear option: when enabled, message prompts do not wait
  for input. Rationale: players who prefer fast play can accept the risk of
  missing message pacing.
- Interrupt warning count: four user interrupts warn about unsafe exit, and
  five escalates. Rationale: UI signal handling must distinguish accidental
  input from deliberate termination.

## Data Structures

- Terminal: a platform-independent grid with requested contents, displayed
  contents, optional saved contents, cursor state, size, flags, and platform
  hooks.
- Front-end terminal instance: a platform window or terminal pane attached to a
  shared terminal.
- UI event: a normalized keyboard, mouse, button, escape, empty, or disconnect
  event.
- Keypress: the command-oriented representation of a keyboard event, including
  modifiers and special keys.
- Keymap: a user-defined trigger and action sequence scoped to the active
  command keyset.
- Input hooks: shared prompt functions for strings, quantities, confirmations,
  directions, points, items, spells, curses, effects, panels, and break checks.
- Menu: a selectable UI surface that maps keyboard, mouse, escape, and selection
  events into explicit choices.
- Screen save stack: temporary saved terminal contents used by menus, dialogs,
  and contextual views.
- Message line: the top-line message display state, including whether a pending
  prompt must be acknowledged.

## Behavior

- B1. Front ends must initialize one primary terminal and any supported
  subwindows, then route all gameplay input through the primary terminal. Test
  scenarios: T1, T2.
- B2. Front ends must translate platform-specific key, mouse, resize, color,
  font, and graphics events into shared terminal or UI events. Test scenarios:
  T3, T4.
- B3. Shared input must activate the primary terminal while waiting for gameplay
  input, restore the previous terminal afterward, and preserve cursor state.
  Test scenarios: T5.
- B4. Pending input flush requests must be delayed until the next input read,
  then clear queued keypresses and keymap expansion state. Test scenarios: T6.
- B5. Backquote input must be normalized to escape for systems where escape is
  unavailable or inconvenient. Test scenarios: T7.
- B6. Keyboard, mouse, button, escape, and disconnect events must be normalized
  to command-facing keypresses where a command prompt requires a keypress. Test
  scenarios: T8, T9.
- B7. Confirmation prompts must return true only for an affirmative answer and
  false for negative, escape, disconnect, or unavailable prompt hooks. Test
  scenarios: T10.
- B8. Message output must share short messages on the top line, split long
  messages, show continuation prompts when needed, and clear the top line after
  acknowledgement. Test scenarios: T11, T12.
- B9. Continuation prompts must wait for acknowledgement unless automatic
  clearing is enabled globally or temporarily inside a keymap. Test scenarios:
  T13, T14.
- B10. Keymaps must expand as nonrecursive input sequences, remain scoped to the
  command keyset used when created, and support bypassing keymaps for the next
  underlying command. Test scenarios: T15, T16, T17.
- B11. Menus must handle movement, selection, escape, mouse selection, and mouse
  cancellation without returning ambiguous choices. Test scenarios: T18.
- B12. Screen save and restore must allow menus, help, object inspection, and
  contextual displays to return to the prior game screen. Test scenarios: T19.
- B13. Front-end customization may control fonts, tiles, colors, subwindow
  visibility, layout, sound, and platform-specific settings, but must not change
  gameplay legality. Test scenarios: T20.
- B14. Save, quit, close-window, disconnect, suspend, and severe-signal flows
  must integrate with save/load behavior and must not continue prompting for
  ordinary gameplay input after disconnection. Test scenarios: T21, T22.
- B15. UI text for Heroband-specific classes, corrupt objects, and forbidden
  power must match current gameplay and must not present corrupt power as a
  normal heroic option. Test scenarios: T23.

## Error Handling

- If a front end cannot provide the minimum primary terminal size, it must refuse
  startup or prevent play rather than run with an incoherent display.
- If input scanning times out, the input request must return an empty event
  rather than blocking forever.
- If a disconnect event occurs while waiting for a key, it must be normalized to
  cancellation for prompt code and to orderly close behavior for gameplay code.
- If an escape event occurs in a prompt or menu, it must cancel the current
  prompt or menu without selecting a normal action.
- If a mouse event is used where only a keypress is accepted, primary click may
  select and secondary click or disconnect must cancel.
- If a message is too long to render safely, it must be ignored or truncated by
  bounded display behavior rather than overflowing terminal state.
- If a menu returns escape, callers must not read a stale cursor as a valid
  selection.
- If front-end-specific preference files are missing or invalid, the front end
  must fall back to usable defaults where possible.

## Implementation Notes

- Keep platform-specific code at the boundary. Shared UI should use terminal and
  event abstractions rather than platform APIs.
- Do not put gameplay rule checks into front-end event conversion.
- Keep message-prompt behavior separate from turn-engine progress. A repeated
  unchanged continuation prompt with unchanged energy is a turn-engine bug
  signal, not a front-end feature.
- Treat keymap expansion as user input intent only. Internally queued upkeep or
  forced-turn commands must not be remapped by UI keymaps.
- Preserve legacy command keysets and bypass behavior so existing player
  preference files remain useful.
- Front-end settings are runtime preferences and should not be used as source
  truth for game behavior.
- Heroband moral wording belongs in shared prompts, help, and docs rather than
  only in one front end.

## Test Scenarios

- T1. The terminal front end starts with a primary terminal at or above the
  minimum dimensions.
- T2. A graphical front end can show a primary terminal and optional subwindows.
- T3. A printable key from a platform event reaches shared input as a keyboard
  event.
- T4. A resize event causes terminal redraw or resize behavior without losing
  pending game state.
- T5. Reading input from a subwindow context temporarily activates the primary
  terminal and restores the previous terminal.
- T6. A queued input flush clears pending keymap expansion before the next read.
- T7. Backquote is interpreted as escape.
- T8. A disconnect while waiting for a keypress produces cancellation.
- T9. A primary mouse click at a keypress prompt is treated as selection.
- T10. A corrupt-object confirmation denied with escape returns false.
- T11. Multiple short messages share the top line until a continuation prompt is
  needed.
- T12. A long message is split before display overflow.
- T13. A continuation prompt waits for acknowledgement when automatic clearing
  is off.
- T14. A keymap-delimited sequence clears continuation prompts without consuming
  later keymap actions.
- T15. A keymap action containing its own trigger does not recursively invoke the
  keymap.
- T16. Keymaps created under one command keyset are not applied as if created
  under the other keyset.
- T17. The keymap bypass command sends the next underlying command directly.
- T18. Menu escape cancels without returning a stale selection.
- T19. Opening and closing help restores the prior game screen.
- T20. Changing tiles or fonts affects presentation only, not object legality or
  command outcomes.
- T21. Closing a front-end window during play triggers save-and-close behavior.
- T22. Disconnect stops ordinary gameplay prompts and enters close handling.
- T23. A corrupt object prompt uses Heroband corruption terminology consistently
  across front ends.

## Changelog

- 1.0.0: Authored full brownfield behavior specification for front ends,
  terminal UI input, prompts, menus, keymaps, and presentation boundaries.
