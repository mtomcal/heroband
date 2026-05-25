---
name: heroband-playtest
description: Repo-only Heroband workflow for validating Angband terminal gameplay with deterministic tests plus tmux-driven GCU play sessions. Use when changing player-facing gameplay, birth flow, terminal UI, class powers, stores, inventory, spells, save/load behavior, or Heroband moral restrictions and an agent needs an end-to-end feedback loop close to a human playing the game.
metadata:
  short-description: Validate Heroband with tests and tmux gameplay
---

# Heroband Playtest

Use this repo-local skill to close the loop on Heroband changes with both automated tests and direct terminal gameplay. Heroband is a morally heroic fork of Angband: the player may fight evil, but may not wield evil.

## Required Flow

1. Define the test contract before launching tmux.
2. Run deterministic validation where it applies.
3. Run direct gameplay in a GCU tmux session for player-facing or gameplay changes.
4. Compare observed behavior to the contract.
5. Promote repeatable discoveries into automated tests when practical.

## Test Contract Gate

Do not start a direct gameplay session until you have written a test contract. Save it in a temporary state directory or another clearly named artifact path.

The contract must include:

- `Invariant`: the behavior that must remain true.
- `Correct Looks Like`: observable pass criteria.
- `Steps`: exact gameplay actions to perform.
- `Evidence Plan`: what pane captures, messages, or screen states will prove the result.

Example:

```text
Invariant: The player cannot select Necromancer or Blackguard during birth.
Correct Looks Like: The class list excludes both names, navigation maps displayed rows to allowed class IDs, and accepted birth creates the selected allowed class.
Steps:
1. Launch isolated GCU session in tmux.
2. Start a new character.
3. Inspect the class choice screen.
4. Navigate through the class list.
5. Select an allowed class.
6. Verify the resulting character class.
7. Quit cleanly.
Evidence Plan: Capture the class screen, the accepted class result, and the quit prompt.
```

## Validation Layers

Use the smallest set that honestly validates the change.

- Deterministic build: `cmake --build build -j2`
- Full automated tests: `cmake --build build -t alltests -j2`
- Unit tests: `cmake --build build -t run-unittest-<path-with-dashes>`
- Scripted game tests: add or update `tests/.../input` and `output` when behavior fits `-mtest`.
- Direct gameplay: use tmux and the GCU frontend for terminal UI, menus, prompts, save/load, play feel, and gameplay scenarios.

For player-facing or gameplay changes, direct gameplay is required unless GCU cannot be configured. If GCU is unavailable, say exactly why and fall back to deterministic tests.

## Direct Gameplay Tiers

- Tier 2, town/dungeon smoke: create or load a character, enter gameplay, move, inspect relevant inventory/equipment/spell/help/store commands, and verify messages and screen state.
- Tier 3, targeted scenario: set up the specific mechanic under test, then exercise the changed class, power, spell, store behavior, item, combat interaction, save/load case, or moral restriction.
- Tier 4, scenario save: generate or load an isolated save fixture that places a character at a chosen depth or hard-floor situation, then exercise mechanics that cannot be judged from town or early-dungeon play.

Use tier 2 by default for player-facing changes. Use tier 3 for mechanics and moral restrictions. Use tier 4 when the behavior depends on depth, monster pressure, equipment, learned powers, save/load continuity, or other state that would be slow or unreliable to set up by hand.

## Scenario Save Fixtures

Scenario saves are temporary playtest artifacts, not checked-in user saves. Keep them under the playtest state directory and launch them with the same isolated `user`, `save`, `panic`, and `archive` paths used by `start`.

When a test needs a high-level, hard-floor, class-power, equipment, store, spellbook, monster-pressure, or save/load-continuity situation, prefer a generated scenario save over manual grinding or hand-driven wizard setup. The save must then be loaded through normal GCU gameplay so the final pass still observes the human-visible game, not only a unit harness.

Use this setup ladder before touching source solely for a playtest:

1. Use an existing scenario helper through `scripts/heroband-playtest prepare-scenario`.
2. Use reliable GCU/debug/wizard setup when it can be verified by captured game state.
3. Add or extend a reusable scenario helper when the need will recur.
4. Only temporarily edit source as a last resort. Revert those setup-only edits before final handoff unless the scenario generator itself is intentionally being committed.

Scenario generation should be generic. It must support any class and level combination needed by the contract, not only the current feature under development. A scenario setup should record:

- character race, class, level, stats, equipment, inventory, and learned powers
- dungeon depth, relevant terrain, monsters, and starting position
- the mechanic under test and the expected risk pressure
- whether the save should be reused for observation only or regenerated each run

Prefer existing test, debug, wizard, or save/load hooks when they can create the fixture cleanly. If no hook exists, document the missing setup capability before adding new engine support. The wrapper command `scripts/heroband-playtest prepare-scenario <preset>` should create the isolated save and write a manifest next to it so the direct gameplay pass can report exactly what state was loaded.

The scenario command should accept parameters or named presets for:

- race and class
- character level, experience, stats, hit points, mana, and gold
- dungeon depth and optional seed
- inventory, equipment, books, and learned spells/orders/powers
- terrain fixtures, starting position, and nearby monsters
- expected pressure and verification notes

The generated manifest should include the exact scenario inputs, the save path, the executable/build used, any seed or setup commands, the expected invariant, and which GCU actions must be performed after loading.

If a generated save cannot be produced yet, use the closest deterministic `src/tests/game/...` or `tests/...` scenario to exercise the behavior and explicitly report the missing save-generation hook. Do not describe that fallback as equivalent to a loaded-save GCU playtest.

## GCU Build

Use a separate build directory for terminal playtests so the normal `build` validation path stays intact:

```sh
cmake -G Ninja -B build-gcu-test -DSUPPORT_GCU_FRONTEND=ON -DSUPPORT_TEST_FRONTEND=ON
cmake --build build-gcu-test -j2
```

The helper scripts below perform this setup when needed.

## Helper Scripts

Prefer the repo wrapper for normal use:

```sh
scripts/heroband-playtest start --contract "$STATE/TEST_CONTRACT.md"
scripts/heroband-playtest capture --state-dir "$STATE"
scripts/heroband-playtest send --state-dir "$STATE" Space
scripts/heroband-playtest stop --state-dir "$STATE"
```

All scripts live relative to this skill:

- `scripts/heroband-playtest prepare-scenario corruption --state-dir "$STATE"`: generate a corruption scenario save and manifest using the repo scenario helper.
- `scripts/start-playtest.sh`: require a contract, configure/build `build-gcu-test`, create isolated state, and launch Angband in tmux.
- `scripts/capture-playtest.sh`: capture the pane and append a transcript.
- `scripts/send-playtest-key.sh`: send one or more tmux keys, then immediately capture the pane.
- `scripts/stop-playtest.sh`: stop the tmux session, optionally preserving artifacts.

The start wrapper keeps runtime writes isolated by redirecting Angband's
`user`, `save`, `panic`, and `archive` directories into the state directory.
If GCU dependencies are missing, install the Debian/Ubuntu baseline with:
`sudo apt-get install -y pkg-config libncurses-dev tmux ninja-build`.

Typical use:

```sh
STATE="$(mktemp -d /tmp/heroband-playtest.XXXXXX)"
cat > "$STATE/TEST_CONTRACT.md" <<'EOF'
Invariant: ...
Correct Looks Like: ...
Steps:
1. ...
Evidence Plan: ...
EOF

scripts/heroband-playtest start \
  --state-dir "$STATE" \
  --contract "$STATE/TEST_CONTRACT.md"

scripts/heroband-playtest capture --state-dir "$STATE"
scripts/heroband-playtest send --state-dir "$STATE" Space
```

## Manual Tmux Recipe

If you do not use the scripts, preserve the same mechanics:

```sh
STATE="$(mktemp -d /tmp/heroband-playtest.XXXXXX)"
SESSION="heroband-playtest-$(date +%Y%m%d-%H%M%S)"
HOME="$STATE" TERM=xterm-256color tmux new-session -d -s "$SESSION" -c "$PWD" \
  "./build-gcu-test/game/angband -mgcu"
tmux capture-pane -p -t "$SESSION" | tail -n 40
tmux send-keys -t "$SESSION" Space
tmux send-keys -t "$SESSION" Enter
```

Always inspect with `tmux capture-pane` after sending keys. Seeing a key command in your shell history is not evidence that the game handled it.

When possible, use `scripts/heroband-playtest send` instead of raw `tmux send-keys`; it sends the key and immediately captures the resulting pane state.

## Tmux Control Keys and Chords

Control keys in GCU must be verified by game state, not by assuming tmux accepted the key name. `tmux send-keys` supports named keys such as `C-a`, `C-w`, `Escape`, and `C-m`, and supports literal input with `-l`, but the target application, terminal mode, and Angband keymaps still determine what happens.

For wizard/debug setup or any control-key sequence:

1. Send the named key through the wrapper when possible, for example `scripts/heroband-playtest send --state-dir "$STATE" C-a`.
2. Capture immediately and verify that the expected prompt, mode, message, or screen state appeared.
3. If the key did not work, try direct tmux send-keys against the session from `playtest.env`.
4. If Angband keymaps may be involved, try bypassing keymaps with `\` before the command.
5. For escape-like input, try both `Escape` and `C-[`; terminals commonly treat control-left-bracket as escape.
6. For submit/return-like input, try both `Enter` and `C-m`.
7. For wizard setup, prefer launch-time wizard mode (`-w`) or a generated scenario save when that is cleaner than interactive debug commands.
8. If a chord remains unreliable, stop using hand-driven setup for that contract and switch to a generated scenario save or deterministic scenario test. Report the limitation and preserved transcript path.

Borrow the verification discipline from `$tmux-agent-orchestration`: seeing text or sending a key is not proof. Inspect the pane and confirm the target application changed state.

## Evidence Standard

Report direct gameplay evidence in the final answer:

- build and test commands run
- launch command and isolated state path
- invariant tested
- gameplay steps completed
- captured screen/message summary
- pass/fail against the contract
- whether the tmux session was stopped or left open for debugging

For Heroband moral changes, explicitly state whether the player could access forbidden player powers: demons, devils, evil spirits, necromancy, soul pacts, blood magic, dark rituals, forbidden occultism, or corrupt shadow/dark power.

## Promotion Rule

If a tmux session reveals or validates behavior that should not regress, promote it:

- Use `tests/...` when `src/main-test.c` can express the scenario.
- Use `src/tests/...` when parser or core logic can be tested directly.
- Keep a transcript only when automation is not yet practical, and identify the missing hook.

If a tmux session reveals a bug that leads to source changes, load `$tdd`
before editing implementation. Create or update the smallest deterministic
regression first, run it and record the red failure, implement the minimal fix,
then rerun and record the green pass. Continue with the GCU playtest after the
deterministic regression is green, unless the bug cannot be expressed outside
GCU; in that case, state the missing hook and preserve the transcript.

For new or changed gameplay/class/moral-restriction tests, run `$heroband-test-quality-verifier` before final handoff when available. The verifier should check that deterministic tests do not merely reward-hack the implementation and that GCU evidence still covers the player-visible path.
