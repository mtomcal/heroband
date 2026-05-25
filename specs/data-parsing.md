# Data Parsing And Gamedata Specification

Version: 1.0.0

## Overview

Heroband MUST load its rule data from text gamedata into validated runtime
tables before normal play, tests, save loading, birth, level generation, and
user-interface display rely on those tables.

The data parsing system MUST preserve classic Angband data-driven behavior
unless a player-accessible rule conflicts with Heroband's central moral rule:
the player may fight evil, but may not wield evil. Enemy-only evil content MAY
remain when the resulting runtime data cannot grant the player forbidden power.

The parser layer MUST provide a shared directive grammar for all gamedata
parsers. Domain parsers MUST translate those directives into strongly typed
runtime records, reject malformed or incompatible records, and leave enough
error context for maintainers to find the failing directive.

## Dependencies

Data parsing depends on the build and test system to provide runtime data,
locale behavior, and deterministic parser tests.

Data parsing provides runtime tables and constants to the Core Engine, Player,
Monsters And Combat, Objects And Corruption, Save Load, Frontends UI, and
Documentation Help specs.

Data parsing MUST complete before systems that require world levels, terrain,
objects, monsters, player races, player classes, timed effects, stores, effects,
message types, or generation profiles can run.

## Parameters

| Parameter | Requirement | Rationale |
| --- | --- | --- |
| Parser error report limit | The parser MUST cap reported parse diagnostics by a configurable non-negative limit, where zero means unlimited reports. | Large data files can produce many follow-on errors after one bad directive; a bounded default keeps diagnostics usable while allowing exhaustive audits when needed. |
| Directive separator | Gamedata directives MUST use a stable field separator between directive name and field values. | A single delimiter allows common parsing, predictable tests, and straightforward hand-editing of data files. |
| Comment and blank-line handling | Blank lines and leading-whitespace comments MUST be ignored. | Maintainers need readable data files with comments and spacing that do not affect runtime behavior. |
| Field types | The shared parser MUST support signed integers, unsigned integers, symbols, full-line strings, random values, and single display characters. | Existing gamedata needs numeric tuning, symbolic references, free text, dice-like values, and visual glyphs without each domain parser reimplementing those conversions. |
| Optional field ordering | Optional fields MUST appear only after all required fields in a directive definition. | This keeps directive parsing unambiguous and prevents later mandatory values from shifting when an optional field is omitted. |
| Full-line string position | A full-line string field MUST be the final field in its directive definition. | Free text can contain the normal separator, so later fields would be ambiguous. |
| User override precedence | A valid user-provided data file MUST take precedence over the standard gamedata file with the same logical name. | Customization is an established behavior, but it must still pass the same validation as standard data. |
| Missing required data file policy | A missing required standard data file MUST stop initialization. | Runtime systems cannot safely operate with partially initialized constants, classes, terrain, monsters, objects, or effects. |
| First-error preservation | When a file has parse errors, the parser MUST preserve the first error as the returned error state while reporting later errors up to the configured limit. | The first error is usually the root cause; later diagnostics are useful context but must not hide the initial failure. |
| Runtime table finalization | Domain parsers MUST finish by converting temporary parse chains into stable lookup structures and indexes. | Gameplay systems require stable references, efficient lookup, and save-compatible identities after initialization. |

## Data Structures

The shared parser state MUST contain registered directive handlers, typed values
for the current line, a caller-owned private context pointer, and the latest
error state.

Each domain parser MUST expose a lifecycle with initialization, file loading,
finalization, and cleanup. Initialization registers directive formats and
creates temporary state. Loading feeds lines through the shared parser.
Finalization validates cross-record references and installs runtime structures.
Cleanup releases all parser-owned runtime allocations.

Runtime gamedata structures MUST include stable records for world levels,
terrain, player properties, object bases, slays, brands, monster bases, summons,
curses, shapes, objects, activations, ego items, histories, bodies, player
races, realms, player classes, artifacts, timed effects, blow methods, blow
effects, monster spells, monsters, pits, lore, traps, quests, flavors, hints,
and random names.

Record-oriented parsers MUST require a record header before record-specific
directives. Directives that modify a current record MUST reject input when no
current record exists.

Runtime constants MUST be held in a single initialized constants structure.
Named constant directives MUST map only recognized labels to known fields.

## Behavior

1. Shared directive parsing MUST ignore empty lines and comments.

   Test scenario: A parser test feeds blank lines and comments before and after
   valid directives and verifies that no record state changes or errors result.

2. Shared directive parsing MUST reject an undefined directive.

   Test scenario: A parser test sends a directive name that has not been
   registered and verifies an undefined-directive error with the failing name in
   parser state.

3. Directive registration MUST reject malformed formats, unknown field types,
   mandatory fields after optional fields, and fields after a full-line string.

   Test scenario: Parser registration tests attempt each invalid format and
   verify registration fails before any data is parsed.

4. Typed fields MUST parse according to their declared type.

   Test scenario: Parser tests parse signed integers, unsigned integers,
   symbols, full-line strings containing separators, random values, and display
   characters, then verify the handler receives the expected typed values.

5. Unsigned numeric fields MUST reject negative input.

   Test scenario: A parser test parses a negative value into an unsigned field
   and verifies a numeric parse error.

6. Random-value fields MUST accept the supported fixed, dice, bonus, and maximum
   forms and MUST reject empty, malformed, overflowed, misplaced, or incomplete
   forms.

   Test scenario: Random-value parser tests enumerate valid and invalid forms
   and verify both the produced components and the rejected errors.

7. Single-character fields MUST accept exactly one display character in the
   active character encoding and MUST reject longer fields.

   Test scenario: Character parser tests parse one ordinary glyph, one
   supported multibyte glyph when the locale allows it, and overlong glyph
   fields.

8. Optional fields MUST be detectable by domain handlers.

   Test scenario: A parser test registers a directive with a trailing optional
   field, parses records with and without that field, and verifies the handler
   can distinguish both cases.

9. Domain parsers MUST reject record-member directives that occur before a
   record header.

   Test scenario: Player-class and monster parser tests feed every member
   directive before the name directive and verify missing-record-header errors.

10. Domain parsers MUST reject references to unknown domain symbols.

    Test scenario: Monster parser tests reference an unknown monster base, and
    constant parser tests reference unknown constant labels or message names.

11. Runtime constants MUST reject negative values for fields that require
    non-negative tuning values.

    Test scenario: Constants parser tests feed negative values into each
    non-negative constant group and verify invalid-value errors.

12. Initialization MUST parse gamedata in dependency order.

    Test scenario: An initialization test registers status-event handlers,
    initializes arrays, and verifies later systems can construct a character and
    level using parsed classes, objects, terrain, monsters, and constants.

13. User override files MUST be parsed with the same grammar and validation as
    standard gamedata.

    Test scenario: A data-loading test places an override file in the user data
    location, verifies it is selected before the standard file, and verifies a
    malformed override fails with normal parser diagnostics.

14. Parser cleanup MUST release both temporary parse state and installed
    runtime allocations owned by each domain parser.

    Test scenario: A parser lifecycle test initializes, parses, finalizes, and
    cleans up representative parsers under leak-checking or repeated
    initialization.

15. Moral-access validation MUST classify evil-themed gamedata by access path
    before changing it.

    Test scenario: A moral-audit test for a player-accessible class, spell,
    object activation, store item, or timed benefit verifies forbidden power is
    absent while enemy-only monsters, curses, and hostile effects remain
    available as antagonistic content.

## Error Handling

Malformed directive formats MUST fail registration before parsing begins.

Malformed data lines MUST return the first parse error for the file while
logging later parse errors up to the configured limit.

Missing required standard data MUST stop initialization with a clear fatal
message.

Domain parsers MUST return explicit errors for missing record headers,
undefined labels, invalid values, invalid flags, invalid message names, invalid
effects, invalid object or monster references, and invalid dice or expression
forms.

Domain parser finalization MUST fail when records cannot be converted into
stable runtime structures.

Parser getter misuse for absent required values is a programmer error; domain
handlers MUST use optional-field presence checks before reading optional values.

## Implementation Notes

Specs MUST describe gamedata behavior, not source layout. Implementations MAY
keep legacy internal identifiers for save and parser stability when the
player-facing meaning has been replaced with morally clean Heroband content.

Data edits are preferred over engine edits when removing or replacing
player-accessible forbidden content, but shared enemy-only content MUST NOT be
deleted merely because its name or theme is evil.

Parser tests SHOULD remain data-aware and should verify canonical records or
symbols rather than menu order or incidental file order.

When adding a new gamedata directive, maintainers SHOULD add directive parsing
tests, missing-header tests when the directive belongs to a record, invalid
reference tests, finalization tests, and at least one runtime behavior test in
the consuming system.

## Test Scenarios

1. Shared parser accepts blank lines, comments, valid required fields, valid
   optional fields, and full-line strings containing separators.
2. Shared parser rejects invalid registration formats, undefined directives,
   missing fields, invalid signed and unsigned numbers, invalid random values,
   and overlong character fields.
3. Constants parsing accepts every recognized tuning label, rejects unknown
   labels, rejects negative non-negative constants, and validates message names
   used by critical-hit levels.
4. Player-class parsing rejects member directives before the class header,
   creates a clean empty class record from a class name, and fills stats,
   skills, titles, equipment, books, spells, effects, and descriptions only
   through valid directives.
5. Monster parsing rejects member directives before the monster header,
   resolves known bases, blows, drops, flags, spells, friends, mimics, shapes,
   and alternate messages, and rejects unknown references.
6. Full initialization parses every required gamedata group in dependency order
   and emits initialization status events.
7. Cleanup after initialization can be followed by another initialization
   without stale runtime structures.
8. A moral-access audit confirms player-accessible records do not grant
   demonic, devilish, evil-spirit, necromantic, soul-pact, blood-magic, dark
   ritual, forbidden occult, or corrupt shadow power, while enemy-only evil
   records remain usable by hostile systems.

## Changelog

### 1.0.0

Initial fully authored brownfield specification for Heroband data parsing and
gamedata loading.
