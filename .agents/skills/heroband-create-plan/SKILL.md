---
name: heroband-create-plan
description: Create Heroband implementation plans with spec-first TDD slices, moral-access checks, scenario-save playtest matrices, subagent authorization gates, and plan lifecycle cleanup. Use when planning Heroband feature work, bug fixes, class powers, gameplay changes, save/load behavior, UI/birth flow, moral restrictions, or when the user asks to create or update an implementation plan in this repo.
metadata:
  short-description: Create Heroband implementation plans
---

# Heroband Create Plan

Use this repo-local skill instead of shared `create-plan` for Heroband work. Keep the shared red/green/refactor structure, but
add Heroband's spec, moral-access, playtest, subagent, and plan-lifecycle gates.

## Required Flow

1. Read the relevant `specs/` files before source inspection. If intended
   behavior changes, plan the spec update before implementation.
2. Read `AGENTS.md`, relevant existing plans, and current code/data paths.
3. Build a spec-driven plan with red/green/refactor slices and explicit commands.
4. Add Heroband moral-access checks for any player-accessible power, text, item,
   class, spell, ally, store, activation, or save path.
5. Add scenario-save playtest coverage when behavior depends on level, depth,
   learned powers, class state, terrain, monsters, stores, save/load continuity,
   or player-visible terminal behavior.
6. Add a subagent authorization gate whenever verifier, pre-mortem, or other
   subagent passes are listed.
7. Add plan lifecycle instructions so the plan does not become stale clutter.

## Plan Sections To Add

Start from the shared `create-plan` structure, then include these Heroband
sections when applicable:

- `Spec Delta To Implement`: cite exact spec files and commit or version.
- `Moral Access Classification`: classify suspicious content as
  player-accessible, enemy-only, harmless, or ambiguous.
- `Scenario-Save Playtest Matrix`: use the template from `$heroband-playtest`
  for gameplay/class/UI/save work.
- `Subagent Authorization Gate`: name each subagent pass and require user
  authorization before implementation begins or before the first subagent gate.
- `Plan Lifecycle`: state whether this is the active plan, what it supersedes,
  and what should be archived or summarized when complete.

## Scenario-Save Requirements

For class powers, gameplay mechanics, save/load, deep-floor behavior, or terminal
UI, do not write a generic "run playtest" gate. Include named scenarios with:

- save name and purpose
- character race/class/level/depth
- learned powers, equipment, inventory, HP/SP/gold
- terrain, monsters, monster state, and pressure
- expected ally tier, formation mode, banner zone, or moral-access state
- exact GCU actions and required captured evidence
- manifest fields and cleanup expectations

If scenario generation support is missing, add a slice to extend
`scripts/heroband-playtest prepare-scenario` or explicitly mark the missing hook
as a blocker/risk. Do not treat wizard-only setup as equivalent to a loaded
scenario-save GCU pass.

## Subagent Authorization Gate

When the plan includes `$heroband-test-quality-verifier`, pre-mortem reviews, or
any subagent work, include this gate before implementation checklist items:

```md
## Subagent Authorization Gate

This plan includes subagent verification passes:
- <subagent or skill>: <purpose>

Before beginning implementation, ask:
`This plan includes subagent verification passes. Are subagents authorized for
this implementation run?`

If subagents are not authorized, convert those gates into local/manual review
steps or ask the user to revise the plan.
```

## Plan Lifecycle

Plans are working contracts, not permanent root-level clutter.

- One active implementation plan per feature.
- Put status in the plan header: `PLANNING`, `IMPLEMENTING`, `PAUSED`,
  `COMPLETED`, `SUPERSEDED`, or `ARCHIVED`.
- When a plan supersedes another plan, name the old plan and the replacement.
- When implementation completes, update the checklist, summarize durable results
  into specs/docs when needed, then archive or mark the plan completed.
- Treat `specs/PLAN.md` as historical spec-extraction context, not an active
  implementation plan.

## Verification

Every Heroband plan must end with targeted tests, build, full tests,
`git diff --check`, relevant playtest gates, and authorized verifier/pre-mortem
gates. The checklist must mirror red/green/refactor slices and list each
scenario-save contract, generation step, GCU run, capture review, and report
step separately.
