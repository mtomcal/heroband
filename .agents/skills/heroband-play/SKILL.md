---
name: heroband-play
description: Repo-only Heroband workflow for compiling and launching a human-playable terminal build with a clean, save-preserving GCU profile. Use when the user asks to compile a playable version, run Heroband, start a local game, let them play with a change, or avoid broken default savefile friction.
metadata:
  short-description: Launch playable Heroband safely
---

# Heroband Play

Use this repo-local skill when the user wants a playable build for hands-on
experimentation, not formal GCU evidence. The goal is to get the user into the
game with one safe command.

## Required Flow

1. Build or reuse the GCU build through `scripts/heroband-play`.
2. Launch with an isolated play profile instead of the raw default save path.
3. Preserve compatible user saves. Never delete, overwrite, or reset a save to
   recover from a broken launch unless the user explicitly asks.
4. Report the exact command and profile/save path the user can reuse.

## Command

Default launch:

```sh
scripts/heroband-play
```

Named save:

```sh
scripts/heroband-play --save-name NAME
```

Fresh alternate profile:

```sh
scripts/heroband-play --profile .play/heroband-gcu-alt --save-name NAME
```

## Save Rules

- The default profile is `.play/heroband-gcu`.
- The default save name is `playtest`.
- Existing compatible saves in the profile are loaded normally.
- If a save is incompatible with the current Heroband build, keep it intact and
  tell the user its path. Suggest a different `--save-name` or `--profile` for a
  clean run.
- Do not launch the raw executable as `build-gcu-test/game/angband -mgcu` for
  user play. That can auto-load stale default saves such as `lib/save/$USER` and
  fail with `Broken savefile`.

## Distinction From Playtesting

This skill is for human playable launches. It does not replace
`$heroband-playtest`, which is still required for implementation validation,
scenario contracts, tmux captures, evidence validation, and verifier review.
