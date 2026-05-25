# Documentation and Player-Facing Help

Version: 1.0.0

## Overview

Documentation and help explain Heroband to players and maintainers. They include
the manual, in-game help, command summaries, customization guidance, hints,
developer notes, release-facing pages, and player-visible terminology.

Documentation must match actual playable behavior. Inherited Angband material
may remain when still accurate, but Heroband-specific moral rules override
upstream wording wherever player choices, classes, powers, corrupt objects,
stores, commands, or victory consequences differ.

## Dependencies

This specification depends on these systems:

- Data parsing provides command names, object names, class names, spell names,
  hints, symbols, stores, effects, options, and preference syntax.
- Core game engine provides command behavior, dungeon rules, stores, combat,
  victory, death, messages, and runtime state.
- Player lifecycle provides birth choices, class descriptions, options,
  corruption, timed effects, winner state, and save behavior.
- Objects and stores provide item categories, corruption warnings, curse
  behavior, object commands, inventory behavior, store behavior, and home
  behavior.
- Save and load provide save commands, panic-save limitations, runtime
  directories, preference output, lore memory, and save selection behavior.
- Front ends and UI provide command keysets, prompts, help browsing, symbols,
  customization menus, keymaps, colors, tiles, fonts, subwindows, and
  front-end-specific settings.
- Build and release systems provide manual generation, packaged help content,
  public release notes, and downloadable documentation.

## Parameters

- Manual depth: the manual may use multi-page structured chapters. Rationale:
  player education requires more context than the compact in-game help browser
  can provide.
- In-game help scope: short command and symbol references plus a pointer to the
  public manual. Rationale: in-game help must remain fast to browse during play.
- Command keysets: original and roguelike keysets must both be documented.
  Rationale: both input modes are supported and players need accurate mappings.
- Help browser page advance: one full page, one half page, and one line motions
  are documented. Rationale: long help text must be navigable without leaving
  the game.
- Help browser search controls: case sensitivity toggle, search, line jump,
  file jump, and highlight controls are documented. Rationale: players need to
  find commands quickly from the keyboard.
- Preference auto-load order: global window settings, race preferences, class
  preferences, then character-name preferences. Rationale: later files must be
  able to specialize earlier defaults.
- User preference persistence scope: options are save-linked, front-end details
  are front-end-owned, and other customizations are preference-file-owned.
  Rationale: documentation must prevent players from expecting all settings to
  persist through the same mechanism.
- Moral terminology source: the central Heroband rule is the canonical wording.
  Rationale: all docs and help must reinforce the same distinction between
  fighting evil and wielding evil.
- Release documentation version: public release pages must name the current
  packaged release. Rationale: users need to match downloads, manual content,
  and checksums.

## Data Structures

- Manual page: a structured player-facing or maintainer-facing topic with
  headings, links, and generated indexes.
- In-game help file: compact browsable text with menu entries and navigation
  commands.
- Command summary: a keyset-specific mapping from keys to command labels.
- Symbol summary: a reference from display symbols to entity categories.
- Hint: a short gameplay tip loaded as data and shown through gameplay systems.
- Preference file: a text customization source for visuals, keymaps, colors,
  messages, sounds, subwindows, inscriptions, and conditional includes.
- Front-end settings record: platform-specific saved presentation settings.
- Public release page: user-facing release and download information.
- Developer note: maintainer-facing explanation of Heroband rules, validation
  expectations, and compatibility constraints.
- Terminology entry: a player-facing canonical term, forbidden alias,
  compatibility note, or moral-access category.

## Behavior

- B1. Documentation must state the central Heroband rule consistently: the
  player may fight evil but may not wield evil. Test scenarios: T1.
- B2. Documentation must distinguish player-accessible corrupt power from
  enemy-only evil, ambient lore, harmless names, and compatibility plumbing.
  Test scenarios: T2, T3.
- B3. Player-facing class documentation must describe General and Vanguard as
  clean heroic classes and must not present old forbidden class identities as
  playable choices. Test scenarios: T4.
- B4. Documentation for corruption must explain warnings, confirmed corrupt use,
  character-bound corruption, hostile consequences, and doomed victory. Test
  scenarios: T5.
- B5. Object, dungeon, and command documentation must describe ordinary curses
  separately from corruption. Test scenarios: T6.
- B6. Command documentation and in-game command summaries must document both
  supported keysets accurately, including save, quit, help, keymap bypass,
  object use, equipment, inventory, quiver, targeting, and message controls.
  Test scenarios: T7, T8.
- B7. Help documentation must explain continuation prompt acknowledgement and
  automatic prompt clearing consistently with UI behavior. Test scenarios: T9.
- B8. Customization documentation must explain preference files, automatic load
  order, keymaps, visuals, colors, message colors, sounds, subwindows, and
  front-end-owned settings. Test scenarios: T10, T11.
- B9. Front-end documentation must keep platform-specific font, tile, window,
  fullscreen, menu, and quit behavior separate from core gameplay rules. Test
  scenarios: T12.
- B10. Save documentation must describe save-and-quit, save-without-quit,
  retirement, dead-character saves, savefile troubleshooting, and panic-save
  limitations accurately. Test scenarios: T13.
- B11. Store documentation must describe town stores, stock turnover, home
  storage, buying, selling, no-selling behavior, and black-market behavior
  consistently with gameplay. Test scenarios: T14.
- B12. Hints and compact help must not advertise removed or forbidden
  player-facing powers. Test scenarios: T15.
- B13. Developer documentation must tell maintainers to validate Heroband moral
  edits with focused tests and playtest gates when touching player-facing
  gameplay, saves, stores, inventory, spells, class powers, or moral
  restrictions. Test scenarios: T16.
- B14. Release-facing documentation must match packaged artifacts, supported
  build routes, public manual location, and checksum instructions. Test
  scenarios: T17.
- B15. Documentation changes must be made in the same behavioral patch when
  player-visible commands, classes, powers, objects, stores, corruption,
  front-end controls, save behavior, or release packaging change. Test
  scenarios: T18.

## Error Handling

- If documentation conflicts with behavior, behavior is the source for
  extraction and documentation must be corrected.
- If inherited Angband wording advertises forbidden player power, it must be
  removed, rewritten, or explicitly marked as not available in Heroband.
- If a term has an internal compatibility meaning and a player-facing meaning,
  documentation must expose only the player-facing meaning unless explaining
  compatibility to maintainers.
- If a help entry points to missing or obsolete behavior, the entry must be
  removed or redirected to current behavior.
- If front-end instructions vary by platform, documentation must identify the
  platform-specific scope rather than implying a universal UI.
- If a public release page names downloadable assets, the version and asset
  names must match the actual release being described.
- If a generated manual is stale, source documentation remains authoritative
  until the manual is rebuilt.

## Implementation Notes

- Keep player docs concise and practical; put deeper implementation context in
  developer notes.
- Use Heroband class and power names in player-facing documentation. Mention old
  internal slot names only as compatibility details for maintainers or advanced
  notes.
- Do not treat a string search for evil words as sufficient evidence. Determine
  whether content is player-accessible, enemy-only, ambient, harmless, or
  ambiguous.
- Prefer updating command summaries, in-game help, manual pages, hints, and
  customization guidance together when behavior changes.
- Avoid documenting removed classes, forbidden spell realms, shadow books,
  necromancy, demonic power, bloodlust, curse-benefit play, or corrupt power as
  player options.
- Public help may link to the hosted manual, but the packaged in-game help must
  remain useful when offline.
- Generated documentation output should be rebuilt by the release process rather
  than hand-edited as source truth.

## Test Scenarios

- T1. The central Heroband rule appears in the player-facing introduction.
- T2. Enemy-only evil is documented as allowed opposition rather than removed
  content.
- T3. Compatibility notes explain legacy internal identifiers without presenting
  them as player choices.
- T4. General and Vanguard descriptions use clean heroic sources and exclude
  forbidden power sources.
- T5. Corruption documentation mentions warning, confirmed use, consequence, and
  doomed victory.
- T6. Curse documentation distinguishes ordinary curses from corruption.
- T7. Original keyset command summary contains current object, save, help, and
  inventory commands.
- T8. Roguelike keyset command summary contains current object, save, help, and
  inventory commands.
- T9. Message prompt documentation matches acknowledgement and automatic clear
  behavior.
- T10. Keymap documentation explains creation, nonrecursion, keyset scope,
  prompt suppression, and persistence.
- T11. Preference documentation explains automatic load order and persistence
  scope.
- T12. Front-end customization documentation describes platform-specific
  presentation settings without changing gameplay rules.
- T13. Save command documentation describes saving, quitting, retiring, and dead
  save behavior.
- T14. Store documentation explains stock turnover, home storage, buying, and
  black-market behavior.
- T15. Hints and compact help contain no removed forbidden player-power
  recommendations.
- T16. Developer notes for Heroband moral changes mention deterministic tests
  and direct playtest or test-quality gates when applicable.
- T17. Release documentation names the same release version, artifacts, and
  checksum workflow as the packaged release.
- T18. A behavior change to corrupt object use updates the manual and compact
  help in the same change when player-facing text is affected.

## Changelog

- 1.0.0: Authored full brownfield behavior specification for documentation,
  in-game help, customization guidance, hints, release-facing pages, and
  Heroband terminology.
