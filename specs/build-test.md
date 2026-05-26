# Build And Test System

Version: 1.1.0

## Overview

The build and test system must produce playable Heroband executables, optional
front ends, optional documentation, installation layouts, unit-test programs,
scripted game-test runs, and coverage reports from the same source tree. It
must preserve classic Angband portability while supporting Heroband validation
workflows for deterministic tests and player-facing moral restrictions.

The default developer path is a self-contained build that copies the runtime
data next to the executable. Shared and read-only installation profiles are
supported for system installs. Test execution must remain available without
requiring a full installation step.

## Dependencies

- A C compiler with C99 support is required.
- CMake is the preferred build orchestrator.
- Autotools and GNU make remain supported for source-distribution and legacy
  build workflows.
- Sphinx is optional and is required only when building the manual.
- Front-end dependencies are optional and depend on the selected front end or
  sound backend.
- SQLite development libraries are optional and are required only for the
  statistics front end.
- Coverage tooling is optional and is required only for coverage targets.
- Cross-compilation may require an emulator before executable tests can run.

## Parameters

| Parameter | Required Value | Rationale |
| --- | --- | --- |
| Minimum CMake version | 3.7 | The project declares this as the oldest supported CMake baseline for configuration. |
| Preferred command-style CMake version | 3.13 or newer | The documented developer commands rely on modern source and build directory flags. |
| C language standard | C99 | The core engine and test programs are configured to compile as C99 across targets. |
| Default non-Windows front end | X11 | A graphical front end is enabled by default when no other non-Windows front end is chosen. |
| Default Windows front end | Native Windows | Windows builds default to the native client when no other graphical front end is chosen. |
| Default installation profile | Self-contained | Developer builds must run directly from the build output without installing system-wide files. |
| Shared install group | games | The shared install profile needs a default group for controlled access to centralized saves and scores. |
| Default Borg support | enabled | The build preserves Angband's automated-player support unless maintainers explicitly disable it. |
| Default Borg high-score eligibility | disabled | Automated characters must not enter normal high-score records by default. |
| Default spoiler front end | enabled except on Windows | Spoiler generation is useful on Unix-like builds but incompatible with the native Windows front-end model. |
| Default test front end | disabled | The test front end is specialized validation machinery and must be enabled deliberately. |
| Default documentation build | disabled unless requested | Documentation generation requires optional tooling and should not block ordinary compilation. |
| Default coverage build | disabled unless requested | Coverage instrumentation changes build flags and is only valid when supported tooling exists. |
| Parallel validation jobs | 2 for local Heroband gate runs | The local project guidance uses two jobs as a conservative default that reduces load while retaining speed. |
| Canonical deterministic build root | `build` | Keeps normal compilation and automated tests in one predictable ignored directory. |
| Canonical GCU build root | `build-gcu-test` | Keeps terminal playtest and human playable builds separate from deterministic validation while still enabling the test frontend. |
| Canonical release build root | `build-release` | Keeps release configuration and artifacts separate from development and GCU builds. |

## Data Structures

- Build configuration: the selected generator, compiler, front ends, sound
  backends, installation profile, documentation option, coverage option, and
  feature toggles.
- Front-end set: zero or more non-conflicting front ends that become available
  in a compiled executable.
- Installation profile: one of self-contained, shared, or read-only, with
  associated runtime data ownership and writable-state rules.
- Target graph: named build actions for the executable, manual, all unit tests,
  all deterministic tests, coverage reset, coverage report, and installation.
- Runtime data bundle: the read-only data, configurable preferences, optional
  assets, and writable user-state directories needed by the executable.
- Build identity: a version string derived from source-control tags or a
  stamped source snapshot.
- Build root profile: a local ignored build directory with a documented role,
  expected CMake front-end options, and cleanup status.

## Behavior

BT-B1. The build system must configure exactly one installation profile unless
the user has explicitly selected a valid profile.

Test scenarios: BT-T1, BT-T2.

BT-B2. The self-contained profile must place the executable and runtime data in
a movable output tree that can run without a system install.

Test scenarios: BT-T3, BT-T11.

BT-B3. Shared and read-only profiles must hardwire their runtime paths at
configuration time and must not rely on destination-directory relocation during
installation.

Test scenarios: BT-T4.

BT-B4. Shared installs must protect centralized writable state through group
ownership and elevated executable permissions.

Test scenarios: BT-T5.

BT-B5. Read-only installs must keep mutable player state in per-user storage
rather than in the read-only installation tree.

Test scenarios: BT-T6.

BT-B6. The native Windows front end must disable front ends and sound backends
that cannot coexist with its entry-point and resource model.

Test scenarios: BT-T7.

BT-B7. SDL and SDL2 support must be mutually exclusive for both front ends and
sound.

Test scenarios: BT-T8.

BT-B8. Enabling the statistics front end must imply statistics backend support.

Test scenarios: BT-T9.

BT-B9. Enabling documentation must build the manual only when Sphinx is
available or fail with a clear documentation-tooling error.

Test scenarios: BT-T10.

BT-B10. Enabling coverage must instrument supported targets and expose reset,
test, and report actions as a single coverage workflow.

Test scenarios: BT-T12.

BT-B11. The all-tests target must include all unit tests and, when possible,
scripted full-game tests through the test front end.

Test scenarios: BT-T13.

BT-B12. Cross-compiled builds must not pretend to run executable tests unless
an emulator is configured.

Test scenarios: BT-T14.

BT-B13. Build identity must come from source-control description when available
and from a snapshot stamp when source-control metadata is unavailable.

Test scenarios: BT-T15.

BT-B14. Heroband validation builds must support deterministic tests before any
player-facing gameplay change is considered complete.

Test scenarios: BT-T13, BT-T16.

BT-B15. Build options must preserve the central design rule by enabling tests
that can detect player-accessible evil power regressions.

Test scenarios: BT-T16.

BT-B16. Local Heroband development should use canonical build roots for the
common workflows: `build` for deterministic validation, `build-gcu-test` for
GCU playtest and human playable terminal launches, and `build-release` for
release packaging. Other `build*` directories are local scratch or legacy roots
unless a workflow explicitly documents them.

Test scenarios: BT-T17.

BT-B17. Build-root hygiene tooling must be inspection-first. It may list roots,
roles, sizes, front-end flags, and timestamps, but must not delete build
directories unless the user explicitly requests cleanup.

Test scenarios: BT-T18.

## Error Handling

- Conflicting installation profiles must stop configuration with an explicit
  conflict message.
- Shared or read-only installation profiles must reject incompatible native
  Windows front-end combinations.
- SDL and SDL2 conflicts must stop configuration rather than choosing one
  silently.
- Native Windows front-end conflicts may be resolved by disabling incompatible
  optional front ends with warnings.
- Missing optional dependencies must disable their dependent features unless
  the user explicitly requested a feature that cannot be built.
- Missing Sphinx must produce a documentation-specific failure only when a
  documentation build is required.
- Unsupported coverage tooling must make coverage unavailable rather than
  producing partial reports.
- Cross-compiled test runs without an emulator must create no false-positive
  test success.
- Installation ownership or permission failures in shared installs must fail
  the install step.
- Misconfigured test entries must fail configuration instead of creating broken
  test targets.

## Implementation Notes

- Prefer the CMake workflow for local Heroband development and validation.
- Keep legacy autotools support usable for source-release users and downstream
  packagers.
- Add new feature flags as explicit configuration options with conservative
  defaults.
- Keep platform-specific front-end logic isolated from core build rules.
- Keep self-contained builds movable by copying all required runtime data into
  the output tree.
- Do not require generated files, local dependency paths, or user-state data to
  be committed.
- Treat internal legacy names as compatibility plumbing unless they surface as
  player-facing behavior.
- A build-only change may be validated by compilation, but a behavior change
  requires deterministic tests and, when player-facing, the Heroband gameplay
  validation workflow.
- Use `scripts/heroband-build-roots` to inspect local build-root sprawl before
  deciding whether a directory is canonical, scratch, legacy, or safe to clean.

## Test Scenarios

BT-T1. Configure with no installation profile selected; verify the
self-contained profile is selected.

BT-T2. Configure with multiple mutually exclusive installation profiles; verify
configuration fails before build generation completes.

BT-T3. Build the default self-contained profile; verify the executable starts
from the generated output tree and can find its runtime data.

BT-T4. Configure a shared or read-only profile with custom install locations;
verify runtime locations are fixed by configuration rather than by install-time
relocation.

BT-T5. Install the shared profile; verify the executable and writable state use
the configured group and permissions.

BT-T6. Install the read-only profile; verify player saves and scores are written
to per-user state.

BT-T7. Configure the native Windows front end with incompatible optional front
ends; verify the incompatible options are disabled or rejected according to
their compatibility rule.

BT-T8. Configure SDL and SDL2 together; verify configuration fails.

BT-T9. Configure the statistics front end; verify the statistics backend is
also enabled.

BT-T10. Request a manual build without Sphinx; verify the build reports the
missing documentation tool rather than failing later with missing files.

BT-T11. Rebuild a self-contained output after a runtime-data edit; verify the
output tree receives the updated data.

BT-T12. Configure coverage on a supported compiler and run the coverage
workflow; verify reset, test execution, and report generation all occur.

BT-T13. Enable the test front end and run all deterministic tests; verify unit
tests and scripted full-game tests both contribute to the final result.

BT-T14. Cross-compile without an emulator; verify runnable test targets do not
claim successful execution.

BT-T15. Build from a tagged checkout, a dirty checkout, and a source snapshot;
verify the build identity reflects each source state.

BT-T16. Run Heroband moral-regression tests after a player-facing change; verify
forbidden player-accessible evil power remains blocked or absent.

BT-T17. Inspect local CMake build roots and verify `build`, `build-gcu-test`,
and `build-release` have documented roles while other `build*` roots are
reported as scratch or legacy unless a workflow claims them.

BT-T18. Run build-root hygiene tooling and verify it reports directory metadata
without deleting or modifying build outputs.

## Changelog

- 1.1.0: Added canonical local build-root profiles and non-destructive
  build-root hygiene requirements.
- 1.0.0: Fully authored build and test system specification from existing
  build files, developer documentation, test harnesses, and release guidance.
