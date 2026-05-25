# Objects, Equipment, Stores, and Corruption Gates

Version: 1.0.0

## Overview

Objects are the player-facing representation of equipment, consumables,
ammunition, magical devices, activatable artifacts, store stock, home storage,
and dungeon treasure. Heroband preserves ordinary Angband object play while
enforcing the rule that the player may fight evil but may not wield evil.

Corruption is a character-bound moral failure state caused by confirmed use of
objects explicitly marked as corrupt. It is not the same as an ordinary curse,
ordinary item danger, enemy-only evil, or flavor text. Corrupt object power may
remain discoverable as danger, but player benefit from it must be warned,
recorded, and given hostile consequences.

## Dependencies

This specification depends on these systems:

- Data parsing provides object kinds, artifact definitions, ego definitions,
  store definitions, effects, flags, curses, activation metadata, and global
  tuning constants.
- Core game engine provides dungeon generation, object placement, command
  execution, energy accounting, messages, and quest completion.
- Player lifecycle provides inventory, equipment, quiver, timed effects,
  derived bonuses, victory state, and persistent character state.
- Save and load preserves carried objects, equipment, stores, artifacts,
  object knowledge, player corruption, and generated dungeon objects.
- Front ends and UI provide item selection, confirmation prompts, store menus,
  inventory views, object inspection, and message acknowledgement.
- Documentation and help explain player-visible commands, stores, object use,
  curses, corruption, and Heroband moral terminology.

## Parameters

- Move energy: 100 energy units define a full ordinary turn. Rationale: object
  commands that consume half or full turns need one shared baseline so object
  handling stays consistent with movement, combat, and the turn engine.
- Equipment change energy: one half of move energy for taking off, dropping, and
  equivalent inventory manipulation commands. Rationale: changing gear is
  intentionally meaningful in tactical time without being as expensive as a full
  movement turn.
- Pack slots: 23 discrete carried-item slots. Rationale: this preserves classic
  inventory pressure and matches the screen layout assumption that a full pack
  can be displayed with a header on the minimum terminal height.
- Quiver slots: 10 discrete ammunition slots. Rationale: ammunition needs
  separate capacity so ranged play is practical without consuming all pack
  slots.
- Missiles per quiver slot: 40. Rationale: ammunition stacks must be large
  enough for sustained ranged play while still requiring inventory management.
- Floor stack limit: 23 discrete objects per grid. Rationale: the limit keeps
  object pile display bounded and prevents a single grid from exceeding the
  minimum terminal display model.
- Store inventory slots: 24 discrete store entries. Rationale: stores and home
  storage need bounded inventories for menu readability, save stability, and
  predictable stock turnover.
- Store turnover interval: 1000 game turns. Rationale: stores should refresh as
  time passes, but not so often that players can easily churn stock without
  meaningful dungeon play.
- Store owner shuffle chance: one chance in 25 per day. Rationale: shopkeeper
  turnover should be rare flavor and economy variation, not a routine source of
  price manipulation.
- Store magic level: 5. Rationale: normal town store stock may include modest
  magical quality without competing with dungeon rewards.
- Corrupt use increment: 1 corruption point for each confirmed corrupt wield,
  wear, or successful corrupt activation. Rationale: corruption records discrete
  player choices rather than passive exposure.
- Hostile attention corruption threshold: 5 corruption points. Rationale: early
  corrupt use should be recoverable in play, while repeated corrupt choices must
  produce visible danger.
- Doomed corruption threshold: 10 corruption points. Rationale: deep repeated
  corrupt use must be enough to deny a clean heroic victory.
- Corruption maximum: the unsigned character-state maximum. Rationale:
  corruption accumulation must saturate rather than wrap around and erase moral
  consequence.

## Data Structures

- Object kind: the base template for an object, including type, subtype,
  display, generation level, weight, value, combat values, effects, flags,
  curses, allocation frequency, and descriptions.
- Object instance: a concrete item in the dungeon, pack, quiver, equipment,
  store, or home. It may reference an object kind, artifact, ego, known state,
  charges, timeout, stack count, origin, inscriptions, curses, and other
  mutable play state.
- Artifact: a unique object identity that can add properties, activation, lore,
  and a corrupt marker.
- Ego item: a modifier identity that can add properties and a corrupt marker to
  eligible object instances.
- Corrupt marker: a data-level classification on an artifact or ego item. An
  object is corrupt when either attached identity is corrupt.
- Character corruption: a persistent integer on the player recording confirmed
  corrupt object use.
- Inventory containers: pack, quiver, equipment body, floor pile, store stock,
  known store stock, and home stock. Each container has distinct selection and
  capacity behavior.
- Store: a town service with owner records, accepted buy categories, always-stock
  entries, normal stock entries, stock capacity, turnover amount, and current
  inventory.
- Home: a store-like player storage container with no purchase price and no
  moral laundering of corrupt objects.

## Behavior

- B1. Object generation must use parsed allocation and store stock rules to make
  ordinary objects available without exposing forbidden player-beneficial evil
  power through normal allocation. Test scenarios: T1, T2, T3.
- B2. Enemy-only evil, hostile lore, slays against evil creatures, and ordinary
  curses may remain when they do not grant the player corrupt power. Test
  scenarios: T4, T5.
- B3. Artifacts and ego items marked corrupt must make any object carrying them
  corrupt. Clean artifacts, clean egos, and unmarked objects must not be treated
  as corrupt. Test scenarios: T6, T7.
- B4. Corrupt objects must prompt for explicit confirmation before wielding,
  wearing, or activation. A denied prompt must leave equipment, timeout, energy,
  and character corruption unchanged. Test scenarios: T8, T9.
- B5. Confirmed corrupt wielding or wearing must equip the object according to
  normal equipment rules and increment character corruption once. Test
  scenarios: T10, T11.
- B6. Confirmed corrupt activation must increment character corruption only when
  the activation succeeds and places the object on recharge. Failed, denied, or
  still-charging activations must not increment corruption. Test scenarios: T12,
  T13.
- B7. Ordinary cursed items must keep ordinary curse behavior and must not
  increment corruption merely because a curse is present. Test scenarios: T14.
- B8. Character corruption at the hostile attention threshold must add hostile
  attention to the player state. Below that threshold it must not. Test
  scenarios: T15, T16.
- B9. Character corruption at the doomed threshold must mark the character as
  deeply corrupted and must deny clean victory after the final quest, using a
  corruption-specific failure reason. Test scenarios: T17, T18, T19.
- B10. Inventory and equipment movement must preserve object identity, stack
  splitting, pack capacity, floor overflow, equipment slot rules, and ring-slot
  replacement behavior. Test scenarios: T20, T21, T22.
- B11. Stores must buy only their accepted categories and must not stock or buy
  player-forbidden book lines or forbidden corrupt player powers as ordinary
  commerce. Test scenarios: T23, T24.
- B12. The home may store player items but must not make corrupt use morally
  safe or remove corruption consequences. Test scenarios: T25.
- B13. Object inspection, knowledge, equipment comparison, and store listings
  must expose useful player information without hiding corruption warnings or
  presenting corrupt power as heroic. Test scenarios: T26.
- B14. Save and load must preserve corrupt object identity, store state,
  equipment state, and character corruption. Test scenarios: T27.

## Error Handling

- Invalid object selections must cancel the command without consuming energy or
  changing corruption.
- If no eligible item exists for a requested object command, the UI must report
  that absence and leave game state unchanged.
- If an equipped item cannot be removed, replacement must be refused before the
  new item is equipped.
- If the player denies a warning inscription or corrupt-object confirmation, the
  command must stop before irreversible object movement or corrupt activation.
- If an activatable object is still charging, activation must be refused before
  confirmation or corruption accounting.
- If a device has no charges or cannot possibly succeed, it must report the
  failure and must not consume corrupt-use credit.
- If a store or home is full, incoming stock must obey existing stacking,
  capacity, and overflow rules rather than silently deleting player property.
- Corruption accumulation must saturate at its maximum representable value.

## Implementation Notes

- Prefer data-level access control for player-facing object availability when
  the parser and save compatibility remain stable.
- Do not remove enemy-only evil objects or lore merely because the theme is
  evil. Classify each case as player-accessible, enemy-only, ambient, harmless,
  or ambiguous before changing behavior.
- Do not rename forbidden object power into heroic language while preserving the
  forbidden mechanic.
- Keep corrupt classification attached to object identities, not to name
  matching in UI prompts.
- Keep confirmation and corruption accounting close to the player action that
  creates benefit. Passive possession, floor visibility, and store display do
  not by themselves cause corruption.
- Store and home behavior must remain compatible with existing save data and
  object indexes.
- When changing object availability, search object data, artifact data, ego
  data, activations, stores, drops, tests, help, and docs for related
  player-facing references.

## Test Scenarios

- T1. A normal allocation scan finds no normally generated player devices that
  expose life-draining power.
- T2. Forbidden scrolls exist for compatibility but have no normal allocation.
- T3. Normally allocated objects do not grant vampirism or bloodlust.
- T4. Slays against demons or undead remain usable as heroic opposition to evil.
- T5. An ordinary curse can be worn without increasing character corruption.
- T6. A known corrupt artifact is classified as corrupt.
- T7. A clean artifact, clean ego, and plain object are classified as clean.
- T8. Denying corrupt wield leaves the item unequipped and corruption unchanged.
- T9. Denying corrupt activation leaves timeout, energy, and corruption
  unchanged.
- T10. Confirming corrupt wield equips the item and increments corruption once.
- T11. Replacing equipment with a corrupt item observes normal removal
  restrictions before equipping.
- T12. Confirming successful corrupt activation places the item on recharge and
  increments corruption once.
- T13. Attempting to activate a charging corrupt item does not increment
  corruption.
- T14. Wielding an ordinary cursed item increments no corruption.
- T15. Corruption below hostile attention does not add hostile attention.
- T16. Corruption at hostile attention adds hostile attention.
- T17. Corruption at doomed threshold is reported as deeply corrupted.
- T18. A deeply corrupted character who completes the final quest does not become
  a clean winner.
- T19. The final-quest corruption failure has a distinct death reason.
- T20. Wielding from a stack splits only the equipped item.
- T21. Replacing gear with a full pack places overflow according to normal pack
  and floor rules.
- T22. Ring replacement asks for the ring slot when both ring slots are occupied.
- T23. Town stores do not stock forbidden shadow or necromantic player books.
- T24. Stores do not buy forbidden player book lines as normal commerce.
- T25. Retrieving a corrupt item from home and using it still triggers normal
  corrupt-use confirmation and consequences.
- T26. Object inspection of a corrupt item shows enough identity to support an
  informed confirmation decision.
- T27. A saved and reloaded game preserves corruption count, corrupt equipment,
  and store inventory state.

## Changelog

- 1.0.0: Authored full brownfield behavior specification for objects,
  equipment, stores, and corruption gates.
