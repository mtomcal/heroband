# Test System

Version: 1.0.0

## Overview

The test system must provide repeatable evidence for Heroband behavior through
deterministic unit tests, scripted full-game tests, optional coverage reporting,
and project-specific player-facing validation workflows. Tests must protect both
classic Angband mechanics and Heroband's central design rule: the player may
fight evil, but may not wield evil.

Unit tests validate focused engine behavior. Scripted full-game tests validate
startup, prompts, birth flow, command handling, and other player-visible flows
through the test front end. Direct gameplay passes are required when automation
alone cannot prove a player-facing interaction.

## Dependencies

- The build system must be able to compile the core engine and test programs.
- Unit tests depend on the shared unit-test harness.
- Scripted full-game tests depend on a game executable with the test front end
  enabled.
- Test data paths depend on the selected installation profile.
- Optional coverage depends on supported compiler instrumentation and report
  tooling.
- Direct gameplay validation depends on a terminal front end and an isolated
  playtest session.
- Moral-restriction test quality checks depend on the Heroband-specific
  verifier workflow.

## Parameters

| Parameter | Required Value | Rationale |
| --- | --- | --- |
| Unit-test output contract | A final suite summary containing passed and total counts | The test runners aggregate results by parsing a stable suite summary. |
| Unit-test success threshold | All tests in every executed suite must pass | Partial pass results can hide regressions in shared engine behavior. |
| Scripted full-game success threshold | Every discovered scripted case must pass | End-to-end scripts represent player-visible contracts and must not be treated as advisory. |
| Verbose mode default | off unless requested | Normal validation should produce concise summaries while preserving an opt-in diagnostic mode. |
| Quiet mode behavior | summary only | Continuous validation needs a low-noise mode for large test runs. |
| Force-path mode default | off | Most pre-install tests must use isolated build data rather than installed data. |
| Force-path override | explicit option or non-empty environment setting | Maintainers need a way to verify installed runtime data without changing test binaries. |
| Test working directory | build output for self-contained builds; source distribution root for install-profile builds | Tests must find runtime data before installation across all supported profiles. |
| Scripted test matcher | optional executable matcher, otherwise byte-for-byte comparison | Some scripted outputs need specialized comparison while simple cases should remain exact. |
| Scripted test state reset | remove prior per-user test state before each scripted case | Scripted tests must not pass or fail because of residue from previous runs. |
| Direct gameplay prerequisite | written test contract | Player-facing manual validation needs declared invariants and evidence before interaction begins. |
| Test-quality verifier scope | gameplay, class-power, birth, store, save and load, scenario-save, and moral-restriction tests | These areas are vulnerable to weak tests that bypass the visible behavior under review. |

## Data Structures

- Unit-test suite: a named executable containing setup, ordered test functions,
  teardown, and a final pass-count summary.
- Unit-test result: suite name, passed count, total count, process status, and
  optional verbose diagnostic output.
- Scripted full-game case: input transcript, expected output, optional custom
  matcher, and last-run output.
- Scripted run result: case name, matcher result, generated output, and pass or
  fail status.
- Test run summary: aggregate passed count, aggregate total count, and process
  exit status.
- Test path mode: default isolated runtime data or forced installed runtime
  data.
- Direct gameplay contract: invariant list, procedure, and evidence plan.
- Scenario save fixture: temporary generated save state for a focused gameplay
  situation.

## Behavior

TS-B1. Every unit-test suite must run setup before test functions and teardown
after test functions.

Test scenarios: TS-T1.

TS-B2. A unit-test suite must return failure when setup fails, teardown fails,
the process dies, or the final summary is malformed.

Test scenarios: TS-T2, TS-T3.

TS-B3. Unit-test runners must aggregate all suite pass counts and must return
failure when any suite fails.

Test scenarios: TS-T4.

TS-B4. Verbose mode must pass through detailed suite output; quiet mode must
suppress per-suite details and retain the aggregate result.

Test scenarios: TS-T5.

TS-B5. Force-path mode must make tests use the same runtime data paths as the
game executable rather than the alternate pre-install test paths.

Test scenarios: TS-T6.

TS-B6. The all-unit-tests build target must build required unit-test programs
before running them.

Test scenarios: TS-T7.

TS-B7. The all-tests build target must run all unit tests and, when the test
front end can execute, all scripted full-game tests.

Test scenarios: TS-T8.

TS-B8. Scripted full-game cases must feed their input transcript to the test
front end and compare generated output to expected output.

Test scenarios: TS-T9.

TS-B9. A scripted full-game case with an executable matcher must use that
matcher instead of exact output comparison.

Test scenarios: TS-T10.

TS-B10. Scripted full-game runs must clear prior per-user test state before
each case.

Test scenarios: TS-T11.

TS-B11. Cross-compiled tests must run only when an emulator is configured; they
must otherwise avoid false success.

Test scenarios: TS-T12.

TS-B12. Coverage workflow must reset old coverage data, run deterministic
tests, then produce per-file reports and an aggregate summary.

Test scenarios: TS-T13.

TS-B13. New behavior changes must add or update the smallest deterministic test
that observes the behavior when practical.

Test scenarios: TS-T14.

TS-B14. Player-facing gameplay, birth, store, inventory, spell, save and load, and
moral-restriction changes must receive a direct gameplay pass when deterministic
tests cannot fully prove the interaction.

Test scenarios: TS-T15.

TS-B15. Direct gameplay passes must begin with a written test contract and must
record evidence against the stated invariants.

Test scenarios: TS-T16.

TS-B16. Tests for Heroband moral restrictions must assert player-visible access
or denial of forbidden power, not merely renamed text or internal identifiers.

Test scenarios: TS-T17.

TS-B17. Repeatable direct-play discoveries should be promoted to deterministic
tests when the unit-test layer or scripted front end can express them.

Test scenarios: TS-T18.

## Error Handling

- Unit-test setup failure must stop that suite and report a setup error.
- Unit-test teardown failure must report failure even if all test functions
  passed.
- A unit-test process that exits unexpectedly must count as at least one failed
  result.
- Malformed unit-test output must fail the aggregate runner.
- Scripted test argument errors must print usage and fail before running.
- Missing scripted test directories must fail the scripted runner.
- Scripted matcher failure must fail that case.
- Exact output mismatch must fail that case and retain last-run output for
  diagnosis.
- Cross-compiled tests without an emulator must not be counted as executed
  successful tests.
- Coverage setup must be unavailable when required instrumentation is not
  supported.
- Direct gameplay without a test contract is invalid evidence for required
  Heroband player-facing validation.

## Implementation Notes

- Prefer deterministic tests for regressions, parser behavior, command
  dispatch, data loading, moral gates, and save and load contracts.
- Use scripted full-game tests for prompts, startup flow, birth flow, and
  command sequences that are visible through the test front end.
- Use direct gameplay when terminal state, timing, or player-visible workflows
  cannot be captured reliably by deterministic tests alone.
- Keep test setup honest: do not bypass the behavior being asserted.
- Keep moral-compliance assertions about player access and gameplay effect, not
  just strings.
- Use scenario saves as temporary focused setup when deep-floor or high-level
  behavior would otherwise be slow to reach.
- Retain generated last-run output as diagnostic evidence, not as a source of
  truth.
- Do not let environment-specific installed data silently affect ordinary
  pre-install tests.

## Test Scenarios

TS-T1. Run a suite with visible setup and teardown effects; verify setup occurs
before tests and teardown occurs after tests.

TS-T2. Force suite setup to fail; verify the suite exits with failure and no
test functions are treated as passed.

TS-T3. Force suite teardown to fail after passing test functions; verify the
suite still fails.

TS-T4. Run multiple suites with one failing suite; verify the aggregate runner
returns failure.

TS-T5. Run the aggregate unit-test runner in verbose and quiet modes; verify
diagnostic detail changes while the final result remains equivalent.

TS-T6. Run a path-sensitive unit test with and without force-path mode; verify
the selected runtime data source changes as expected.

TS-T7. Invoke a single unit-test target from a clean build; verify the target
builds its test program before executing it.

TS-T8. Enable the test front end and run all tests; verify both unit-test and
scripted full-game results affect the final status.

TS-T9. Run a scripted full-game case with exact expected output; verify matching
output passes and changed output fails.

TS-T10. Run a scripted full-game case with a custom matcher; verify the matcher
decides pass or fail.

TS-T11. Create residual per-user test state before a scripted case; verify the
runner removes it before the case executes.

TS-T12. Cross-compile without an emulator; verify runnable test execution is
skipped or unavailable rather than reported as passed.

TS-T13. Run the coverage workflow on a supported toolchain; verify stale data is
cleared, tests run, and coverage summaries are generated.

TS-T14. Change a deterministic behavior and add a focused failing test first;
verify the test fails before the fix and passes after the fix.

TS-T15. Change a player-facing terminal interaction; verify deterministic tests
run and a direct gameplay pass covers the visible workflow.

TS-T16. Start a direct gameplay pass; verify it has declared invariants,
procedure, and evidence plan before interaction.

TS-T17. Add a moral-restriction test; verify it proves forbidden player power is
unavailable or punished through visible behavior.

TS-T18. Reproduce a direct-play bug deterministically; verify a unit or scripted
test is added before closing the regression.

## Changelog

- 1.0.0: Fully authored test-system specification from existing unit-test
  harnesses, scripted test runners, build targets, and Heroband validation
  guidance.
