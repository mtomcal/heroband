# AGENTS.md

Repository guidance for AI coding agents working on Heroband.

<!-- TREE-HASH: 2d220a98a78c0e67eadb035f6593dfe21023dcad90824f87a24f9c2c929b7528 -->

## Project Rule

Heroband is a morally heroic fork of Angband 4.2.

The central design rule is: the player may fight evil, but may not wield evil.

Do not give playable characters power from demons, devils, evil spirits, necromancy, soul pacts, blood magic, dark rituals, forbidden occultism, or morally corrupt shadow/dark power. Enemy-only evil content may remain when it is clearly antagonistic.

Prefer small, buildable patches. Preserve Angband behavior unless it conflicts with Heroband's moral design constraints.

## Plan Completion Gate

When the user asks to implement an approved plan, treat the plan as an end-to-end contract. Work through every unchecked slice unless the user explicitly narrows the scope or a real blocker prevents progress. Update the plan checklist as slices complete, run the required deterministic tests and Heroband playtest/verifier gates, and do not hand back after only one slice with remaining unchecked work unless asked to pause.

If an approved plan includes subagent verification passes, ask the user whether
subagents are authorized before starting implementation or before the first
subagent-dependent gate. Use a direct prompt such as: `This plan includes
subagent verification passes. Are subagents authorized for this implementation
run?` If subagents are not authorized, convert those gates into local/manual
review steps or ask the user to revise the plan.

## Plan Lifecycle

Use one active implementation plan per feature. Plans should state their status
in the header (`PLANNING`, `IMPLEMENTING`, `PAUSED`, `COMPLETED`,
`SUPERSEDED`, or `ARCHIVED`) and name any plan they replace. When implementation
finishes, update the checklist and either mark the plan completed, archive it,
or summarize the durable outcome into specs/docs so root-level plan files do not
accumulate without clear ownership.

## Specs

Heroband has a first-pass brownfield spec suite in `specs/`. Before making any
code change, inspect the relevant spec and update the spec first when the
intended behavior, contract, terminology, or moral-access rule changes. Use
`specs/PLAN.md` as historical extraction context, not as the primary source of
truth for current behavior.

For current gameplay behavior, class mechanics, save/load rules, build profiles,
and terminology, prefer `specs/` as the source of truth. Use recent git history
to understand what changed most recently, for example `git log --oneline -n 20
-- specs src lib/gamedata scripts` and focused `git show` calls for relevant
commits.

When touching command dispatch, timed effects, command mode, or energy use,
start from `specs/turn-engine.md` before broadening guidance in this file.
When adding or changing domain terms in any spec, check
`specs/UBIQUITOUS_LANGUAGE.md` and use `$ubiquitous-language` for glossary
updates so overloaded terms and aliases stay consistent.

## Repo-local Skills

Use `$heroband-create-plan` from `.agents/skills/heroband-create-plan/SKILL.md`
when creating or updating Heroband implementation plans. It layers spec-first
planning, moral-access classification, scenario-save playtest matrices, subagent
authorization gates, and plan lifecycle cleanup onto the shared TDD plan shape.

Use `$tdd` from the shared skills before making source changes for bug fixes,
regression fixes, behavior changes, or implementation tasks. This applies even
when the bug is discovered during investigation or live play. Diagnose and
stabilize the live situation first when needed, then enter a red/green/refactor
cycle before editing implementation: write or update the smallest behavior test,
run it and observe the failure, implement the minimal fix, and rerun the test to
green.

Use `$heroband-playtest` from `.agents/skills/heroband-playtest/SKILL.md` for player-facing gameplay, terminal UI, birth flow, class power, store, inventory, spell, save/load, or Heroband moral-restriction changes. That skill requires a written test contract before tmux gameplay begins, then validates with deterministic tests plus a direct GCU/tmux gameplay pass when appropriate.

Use `$heroband-play` from `.agents/skills/heroband-play/SKILL.md` when the user asks to compile or launch a playable Heroband build for hands-on use. It builds the GCU frontend through `scripts/heroband-play`, launches with an isolated save-preserving profile, and avoids raw default-save autoload failures such as `Broken savefile`.

Use `$heroband-test-quality-verifier` from `.agents/skills/heroband-test-quality-verifier/SKILL.md` when adding or changing gameplay, class-power, birth, store, save/load, scenario-save, or moral-restriction tests. This repo-specific verifier checks for reward-hacking, setup that bypasses the behavior under test, and weak moral-compliance assertions.

Use `$heroband-release` from `.agents/skills/heroband-release/SKILL.md` when generating, tagging, packaging, uploading, or publishing a Heroband release. It records the release workflow availability check, manual artifact fallback, documentation build dependency on `sphinx-better-theme`, source/Linux/checksum artifact validation, and GitHub release publication steps.

Preferred wrapper:

```sh
scripts/heroband-playtest start --contract /path/to/TEST_CONTRACT.md
scripts/heroband-playtest capture --state-dir /tmp/heroband-playtest.xxxxxx
scripts/heroband-playtest send --state-dir /tmp/heroband-playtest.xxxxxx Space
scripts/heroband-playtest stop --state-dir /tmp/heroband-playtest.xxxxxx
```

## Map

<!-- TREE-START -->
```
.
|-- docs
|   |-- _static
|   |-- _templates
|   `-- hacking
|-- lib
|   |-- customize
|   |-- fonts
|   |-- gamedata
|   |-- help
|   |-- icons
|   |-- panic
|   |-- save
|   |-- scores
|   |-- screens
|   |-- sounds
|   |-- tiles
|   |   |-- adam-bolt
|   |   |-- gervais
|   |   |-- nomad
|   |   |-- old
|   |   `-- shockbolt
|   `-- user
|       |-- archive
|       |-- panic
|       |-- save
|       `-- scores
|-- m4
|-- mk
|-- screenshots
|-- scripts
|-- specs
|-- src
|   |-- borg
|   |-- cmake
|   |   |-- macros
|   |   |-- modules
|   |   `-- scripts
|   |-- cocoa
|   |   |-- Base.lproj
|   |   `-- en.lproj
|   |-- doc
|   |-- nds
|   |-- sdl2
|   |-- stats
|   |-- tests
|   |   |-- artifact
|   |   |-- cave
|   |   |-- command
|   |   |-- effects
|   |   |-- game
|   |   |-- message
|   |   |-- monster
|   |   |-- object
|   |   |-- parse
|   |   |-- player
|   |   |-- trivial
|   |   |-- z-dice
|   |   |-- z-expression
|   |   |-- z-file
|   |   |-- z-quark
|   |   |-- z-queue
|   |   |-- z-textblock
|   |   |-- z-util
|   |   `-- z-virt
|   `-- win
|       |-- dll
|       |-- include
|       |   `-- libpng12
|       |-- lib
|       `-- vs2019
|-- tests
|   |-- birth
|   |   |-- Dw-Pa
|   |   |-- Hu-Ge
|   |   |-- Hu-Va
|   |   |-- Hu-Wa
|   |   |-- new-game-0
|   |   `-- new-game-1
|   `-- trivial
|       `-- matcher
|-- toolchains
`-- utils

83 directories
```
<!-- TREE-END -->

## Modules

### `src`

- **Purpose**: Core C game engine, UI, front ends, parsers, player/monster/object systems, effects, generation, save/load, and command handling.
- **Owns**: Top-level `*.c`/`*.h`, `src/borg`, `src/cmake`, `src/cocoa`, `src/nds`, `src/sdl2`, `src/stats`, `src/win`.
- **Depends on**: Data definitions in `lib/gamedata`, preferences/assets in `lib`, build configuration in `CMakeLists.txt`.
- **Rules**: Keep shared mechanics available for monsters/enemy content unless the task specifically removes them. For player-facing moral restrictions, prefer gating player access before deleting shared code or data. Maintain parser and class ID stability unless save/data compatibility has been explicitly audited.
- **Entry points**: `src/main.c`, front-end mains such as `src/main-x11.c`, `src/main-gcu.c`, `src/main-test.c`, and the `OurCoreLib` / `OurExecutable` targets in `CMakeLists.txt`.

### `lib/gamedata`

- **Purpose**: Data-driven game definitions for classes, races, monsters, objects, spells, effects, realms, stores, terrain, curses, hints, and parser-fed tables.
- **Owns**: Text data consumed by C parsers at startup.
- **Depends on**: Parser code in `src/parse*`, initialization in `src/init.c`, gameplay systems throughout `src`.
- **Rules**: Prefer data edits when removing player-facing content, but do not remove entries that shared parser code, monsters, stores, drops, or save/load assumptions still reference. For Heroband, classify each evil-themed entry as player-accessible, enemy-only, harmless, or ambiguous before changing it.
- **Entry points**: Data files loaded during game initialization; parser tests under `src/tests/parse`.

### `lib/customize`, `lib/help`, and Assets

- **Purpose**: User preferences, help text, visual/audio assets, screens, tiles, fonts, and runtime user data directories.
- **Owns**: `lib/customize`, `lib/help`, `lib/fonts`, `lib/icons`, `lib/screens`, `lib/sounds`, `lib/tiles`, `lib/user`.
- **Depends on**: UI and preference loading code in `src`.
- **Rules**: Keep player-facing text consistent with playable content. Do not document removed classes or powers as available. Do not edit generated user save/archive/panic/score files unless the task is explicitly about runtime state.
- **Entry points**: Preference loading, help browser, tile/font/sound front-end loading.

### `docs`

- **Purpose**: Sphinx documentation for player manual and developer-facing notes.
- **Owns**: `docs/*.rst`, `docs/hacking`, Sphinx static/template folders.
- **Depends on**: Current gameplay rules and player-facing features.
- **Rules**: Documentation must match actual playable behavior. Historical notes may mention old Angband behavior, but manuals and class descriptions must not present removed Heroband powers as player choices.
- **Entry points**: Sphinx build via CMake when `BUILD_DOC` is enabled and Sphinx is available.

### `src/tests`

- **Purpose**: C unit tests for parsers, player logic, objects, monsters, effects, utility libraries, and low-level systems.
- **Owns**: Unit test source files listed in `ANGBAND_TEST_CASE_SOURCES` in `CMakeLists.txt`.
- **Depends on**: `OurCoreLib`, `OurUnitTestLib`, test utilities in `src/tests/test-utils.c`.
- **Rules**: Add or update focused tests when behavior changes cross parser, birth, effects, or player/object/monster boundaries. Keep tests data-aware rather than relying on menu row numbers when class IDs or visible lists can diverge.
- **Entry points**: `cmake --build build -t allunittests -j2` or `cmake --build build -t alltests -j2`.

### `tests`

- **Purpose**: End-to-end test scripts and fixtures run through the test front end.
- **Owns**: `tests/birth`, `tests/trivial`, `tests/run-test`, root `run-tests`.
- **Depends on**: `SUPPORT_TEST_FRONTEND=ON`, `src/main-test.c`, built executable and copied runtime data.
- **Rules**: Update these tests when birth flow, UI prompts, startup flow, or command scripting changes. These tests exercise the built game, so keep them aligned with player-visible behavior.
- **Entry points**: `cmake --build build -t alltests -j2`.

### Build System

- **Purpose**: CMake, legacy make fragments, autotools support, scripts, and toolchains.
- **Owns**: `CMakeLists.txt`, `src/cmake`, `mk`, `m4`, `scripts`, `toolchains`, platform makefiles.
- **Depends on**: Compiler, platform front-end dependencies, optional Sphinx, optional SDL/X11/ncurses/statistics dependencies.
- **Rules**: Keep build changes minimal and cross-platform. Do not assume top-level `make` works before configuration. CMake is the preferred validation path for this workspace.
- **Entry points**: `cmake -G Ninja -B build -DSUPPORT_TEST_FRONTEND=ON`, `cmake --build build -j2`, `cmake --build build -t alltests -j2`.

## Dependency Rules

Architectural boundaries and forbidden shortcuts:

- **Data before source when feasible**: If a Heroband restriction can be enforced safely in `lib/gamedata` or player-facing text, prefer that before changing core engine behavior.
- **Do not delete shared evil content blindly**: Demons, undead, curses, Morgoth, Sauron, and corruption may remain as enemy-only content. Remove or replace player-accessible evil power, not all evil references.
- **Birth UI must map displayed choices to actual IDs**: If filtering classes/races/options, keep an explicit displayed-choice-to-data-ID mapping. Angband data order and linked-list order may not match visible menu order.
- **Parser stability matters**: Data files are parsed into indexed structures used throughout C. Removing entries can shift IDs or break tests; gate access first unless the compatibility impact has been audited.
- **Docs must follow behavior**: Whenever player choices, classes, spells, or commands change, update `docs`, `lib/help`, `lib/customize`, and hints in the same patch when practical.
- **Front ends are optional**: Changes in common UI or core systems must not assume one front end. The test front end is the validation path for automated behavior.

## Anti-patterns

Mistakes to avoid:

- **Pattern**: Renaming evil player powers without changing the mechanic.
  - **Why wrong**: Heroband forbids whitewashed necromancy, demonic pacts, blood sacrifice, occult ritual, and corrupt shadow power.
  - **Right way**: Replace the power source and mechanic with clean heroic equivalents, such as discipline, courage, craft, lawful command, healing, light, music, tactics, nature, or heroic resolve.

- **Pattern**: Removing monster or object data because a string sounds evil.
  - **Why wrong**: Enemy-only evil is allowed and is part of the good-vs-evil atmosphere.
  - **Right way**: Determine whether the content is player-beneficial, enemy-only, ambient lore, or ambiguous before editing.

- **Pattern**: Filtering a visible menu by list position and returning that position as the data ID.
  - **Why wrong**: It can silently select the wrong class or race when hidden entries exist.
  - **Right way**: Maintain a mapping from visible row to canonical data ID and use the canonical ID for selection.

- **Pattern**: Making a large thematic rewrite before a buildable checkpoint.
  - **Why wrong**: Angband has many data/code cross-references; wide changes are hard to validate.
  - **Right way**: Make narrow patches, build after each, and leave explicit follow-up lists for remaining references.

## Coding Principles

- **Test Methodology**: Use `$tdd` for source changes and preserve red/green evidence for each bug fix or behavior change. For buildable checkpoints, run `cmake --build build -j2` and `cmake --build build -t alltests -j2` when the test front end is configured. Use `git diff --check` before handing off code changes.
- **Design Principles**: Preserve classic Angband gameplay unless it conflicts with Heroband's moral constraints. Replace forbidden player powers with genuinely clean mechanics, not euphemisms.
- **Code Organization**: Keep C source changes close to the owning subsystem. Birth/class selection belongs in `player-birth`, `player-class`, and `ui-birth`; parser/data changes belong with the related `lib/gamedata` file and parser tests.
- **Error Handling**: Prefer explicit rejection messages for unavailable player choices. Do not allow invalid class/race IDs to flow deeper into birth or save initialization.
- **Mutation Rules**: Do not revert user changes. Do not edit generated build outputs, local dependency caches, or runtime user files unless specifically asked.
- **Naming Conventions**: Follow existing Angband C naming and file organization. New Heroband replacements should use morally clear names that identify the actual clean power source.
- **Review Gates**: For Heroband moral edits, search code, gamedata, docs, help, birth UI, tests, and customization files for old class/power names before finalizing.

## Local Validation Notes

In this workspace, full validation was performed with local non-root dependencies and Ninja:

```sh
cmake -G Ninja -B build -DSUPPORT_TEST_FRONTEND=ON
cmake --build build -j2
cmake --build build -t alltests -j2
```

If a machine uses local, non-system dependencies, keep those paths outside committed files or pass them through environment variables / CMake cache options locally. The `build` directory and local dependency caches should not be committed.

For canonical build-root roles and local build sprawl inspection, use
`specs/build-test.md` and `scripts/heroband-build-roots`.

## Current Context

Do not use this file as a gameplay ledger. Current Heroband behavior belongs in
`specs/`, with completed implementation plans and recent git history as
supporting context.

## Appendix

The structure scan was produced by:

The structure scan was produced with the `create-agents-md` skill's `detect-structure.sh --json` helper.

The detector reported no package manager and limited dependency inference, so this file intentionally uses repository-specific CMake and Angband/Heroband knowledge rather than treating every subdirectory as a separate module.
