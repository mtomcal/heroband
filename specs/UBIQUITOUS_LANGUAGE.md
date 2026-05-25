# Ubiquitous Language

Version: 1.0.0

## Overview

Heroband is a morally heroic Angband-family dungeon adventure. The player may
fight evil, but may not wield evil. This glossary defines the canonical terms
used by the specification suite so requirements distinguish allowed heroic
gameplay from forbidden player-accessible corrupt power.

## Dependencies

This language spec is the root vocabulary for all other specs. The moral access
terms are required by the player, monster, object, data, documentation, and test
specs. The turn, command, and energy terms are required by the core engine and
turn engine specs.

## Parameters

- Moral access category: every suspiciously evil name, mechanic, item, power, or
  text reference MUST be classified as player-accessible, enemy-only, harmless,
  or ambiguous before it is changed or accepted. Rationale: Heroband preserves
  evil enemies and atmosphere while forbidding evil player power.
- Compatibility plumbing: legacy internal names MAY remain when they are not
  visible to players and do not grant forbidden power. Rationale: stable data
  and save identifiers avoid unnecessary breakage in a brownfield fork.
- Specification status: a spec marked version 1.0.0 is considered fully
  authored and reviewed against the extraction plan. Rationale: maintainers need
  a clear completion marker for brownfield extraction.

## Data Structures

### Actors And Content

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Player** | The human-controlled character and their directly controlled choices, inventory, class, powers, equipment, and saved state. | Character when discussing monsters too, hero when precision matters |
| **Monster** | A non-player creature that may oppose, ignore, flee, or temporarily obey the player. | Enemy when neutral or commanded creatures are included |
| **Object** | An item that can exist in the dungeon, stores, home, inventory, quiver, or equipment. | Item when identity, generation, or save state matters |
| **Artifact** | A unique object identity with persistent lore, properties, and optional corruption status. | Unique item |
| **Store** | A town service with stock, owner rules, buying rules, and player-visible commerce. | Shop when referring to saved stock state |
| **Home** | The player's store-like storage location that preserves possessions without making corrupt use safe. | Store when moral consequence is the point |
| **Save** | Persisted runtime state for a character, including compatibility fields and data-derived identities. | Savefile when discussing gameplay state |

### Player Access And Morality

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Moral Rule** | The Heroband rule that the player may fight evil but may not wield evil. | Theme, flavor, vibe |
| **Player-Accessible Power** | Any benefit the player can choose, equip, cast, activate, buy, learn, or otherwise exploit. | Player-facing text, available content |
| **Enemy-Only Evil** | Evil content that remains antagonistic, dangerous, atmospheric, or lore-only and cannot become a player benefit. | Forbidden content |
| **Forbidden Power** | Player-accessible power from demonic, devilish, evil-spirit, necromantic, soul-pact, blood-magic, dark-ritual, forbidden occult, corrupt shadow, life-drain, or curse-benefit sources. | Dark power, shadow power, evil magic |
| **Clean Heroic Power** | Player-accessible power sourced from courage, discipline, tactics, lawful command, craft, endurance, mercy, healing, light, music, nature, protection, morale, or heroic resolve. | Whitewashed dark power |
| **Corruption** | A character-bound moral-risk state produced by confirmed corrupt object use. | Curse, evil, taint, corruption gate |
| **Corrupt Object** | An object whose artifact or ego identity is explicitly marked as morally corrupt. | Cursed item, evil item |
| **Ordinary Curse** | A harmful object affliction that does not by itself record moral corruption. | Corruption |
| **Ambiguous Content** | Content whose moral access category cannot be determined without tracing whether the player can benefit from it. | Safe content, forbidden content |

### Classes And Power Sources

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **General** | A playable heroic class using tactics, morale, field commands, and temporary living allies. | Necromancer, summoner |
| **Vanguard** | A playable heroic class using courage, discipline, armor mastery, battlefield drills, and heroic resolve. | Blackguard, blood warrior |
| **Class Slot** | A stable compatibility identity that may differ from the player-facing class name. | Class when discussing visible choices |
| **Realm** | A grouped source of learnable powers or books for classes. | Book type when discussing power source |
| **Temporary Ally** | A living soldier temporarily directed by the player through command mode. | Summon, minion, spirit, undead |

### Turns, Commands, And State

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Player Intent Command** | A command originating from player input, keymaps, menus, scripts, or repeated input. | Command when forced upkeep is possible |
| **Queued Command** | An engine command stored for gameplay processing with context, arguments, and repeat/background metadata. | Input |
| **Forced Upkeep Command** | An internally queued command that spends the player's turn for paralysis, knockout, sleep, or similar upkeep. | Player command, monster command |
| **Monster Order** | A player-directed action for a commanded monster while command mode is active. | Command when player intent is meant |
| **Command Mode** | The player state for directing one commanded monster or ally. | Command dispatch, command menu |
| **Energy Use** | The amount of actor energy consumed by an action that advances time. | Turn cost when no energy is spent |
| **No-Energy Loop** | A repeated gameplay prompt or command-processing cycle where expected forced progress does not spend energy. | Terminal lag, more prompt issue |
| **Timed Effect** | A temporary condition on the player or monster that can change capabilities, action choice, messages, or combat outcomes. | Status when parser identity matters |

### Character Lifecycle

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Birth** | The player-visible character creation lifecycle that selects race, class, stats, name, history, options, and starting kit. | New game when discussing menus and state |
| **Quickstart** | A birth shortcut that reuses a complete previous playable character template. | Random birth, restart |
| **Starting Kit** | The class-derived initial equipment and supplies granted during birth. | Loot, store stock |
| **Character History** | The recorded ancestry, personal description, and milestones attached to a player. | Lore when referring to monster memory |
| **Winner State** | The persistent state that records whether the player achieved a clean final victory. | Victory when corruption consequences matter |

### World And Engine State

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Dungeon Chunk** | The active generated level area containing terrain, objects, monsters, traps, light, scent, and noise. | Level when generated state matters |
| **Level Transition** | The controlled engine process for leaving one dungeon chunk and preparing another. | Teleport, recall, stairs |
| **World Upkeep** | Periodic engine processing for time-based dungeon, player, monster, object, store, and delayed-movement effects. | Turn when actor action is meant |
| **Message History** | The bounded record of recent player-visible messages. | Log when event delivery matters |
| **Game Event** | A synchronous engine signal that lets front ends and subsystems react to state changes. | Message, callback |
| **Runtime Data Bundle** | The read-only data, assets, preferences, and writable user-state directories needed by an executable. | Install tree, source data |
| **Runtime User State** | Generated per-user saves, panic saves, scores, preferences, archives, lore, and similar play artifacts. | Source files, gamedata |
| **Panic Save** | A recovery save written after a severe fault or signal path. | Normal save, scenario save |

### Knowledge And Memory

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Player Knowledge** | The player's discovered object, rune, flavor, artifact, spell, and monster information. | Omniscient state |
| **Monster Lore** | The player's accumulated memory of observable monster race behavior. | Monster data, character history |
| **Object Knowledge** | The player's discovered identity and properties for object kinds, artifacts, egos, runes, and flavors. | Object state |
| **Hint** | A short player-facing gameplay tip loaded as data. | Manual page |

### Objects And Commerce

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Object Kind** | The reusable base template from which object instances are generated. | Object when referring to a concrete item |
| **Object Instance** | A concrete object in the dungeon, pack, quiver, equipment, store, or home. | Object kind |
| **Ego Item** | A modifier identity that can add properties and corruption status to eligible object instances. | Artifact, enchantment |
| **Inventory Container** | A pack, quiver, equipment body, floor pile, store stock, known stock, or home stock holding object instances. | Inventory when stores or floor are included |
| **Store Stock** | The object instances currently held for sale or storage by a store-like service. | Loot, allocation |

### Monsters And Combat

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Monster Race** | The reusable definition for a monster type, including depth, rarity, flags, blows, spells, drops, and messages. | Monster when referring to one live creature |
| **Monster Instance** | A live monster on a dungeon chunk with current position, health, energy, targets, objects, and timed effects. | Monster race |
| **Monster Allocation** | The depth-weighted process for selecting legal monster races for generation. | Spawn when legality and rarity matter |
| **Monster Spell** | A monster-authored ranged, breath, summoning, utility, or hostile effect. | Player spell |
| **Blow** | One monster melee method and effect pair within an attack sequence. | Hit when the attack can miss |
| **Group State** | The pack, escort, bodyguard, or leader relationship that shapes monster generation and movement. | Team, faction |

### Front End And Documentation

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Front End** | A platform or terminal presentation layer that translates events and renders shared terminal state. | UI when gameplay legality is meant |
| **Terminal** | The platform-independent grid used by front ends and shared UI code. | Console when graphical front ends are included |
| **UI Event** | A normalized keyboard, mouse, resize, escape, empty, or disconnect event. | Command |
| **Keymap** | A user-defined input trigger and action sequence scoped to a command keyset. | Macro when prompt suppression or keyset scope matters |
| **Menu** | A selectable UI surface that maps input events into explicit choices or cancellation. | Prompt when multiple choices are shown |
| **Message Prompt** | The continuation prompt that requires acknowledgement before more message text is shown. | More prompt, terminal pause |
| **Manual Page** | A structured player-facing or maintainer-facing documentation topic. | Help file |
| **In-Game Help** | Compact browsable help text available from inside play. | Manual |
| **Developer Note** | Maintainer-facing guidance about rules, compatibility, validation, or workflow. | Player manual |

### Validation And Release

| Term | Definition | Aliases To Avoid |
| --- | --- | --- |
| **Deterministic Test** | An automated test that should produce repeatable pass or fail evidence without manual play. | Playtest |
| **Scripted Full-Game Test** | A test-front-end case that feeds input and compares generated player-visible output. | Unit test |
| **Direct Gameplay Pass** | A human-like terminal play session used to validate player-visible behavior. | Manual test when contract evidence is required |
| **Test Contract** | A written invariant, procedure, and evidence plan for direct gameplay validation. | Checklist |
| **Scenario Save** | A temporary generated save fixture for a focused gameplay situation. | Normal save, panic save |
| **Release Manifest** | The release record of version, artifacts, checksums, platform support, documentation status, and validation status. | Release notes |
| **Source Archive** | A distributable source package with tracked content and required generated configuration support. | Checkout |
| **Playable Archive** | A platform package containing an executable, runtime data, assets, documentation, and initialized user-state directories. | Source archive |
| **Checksum Manifest** | The release file listing integrity checks for every downloadable artifact. | Release manifest |
| **Build Identity** | The version string derived from a tag or source snapshot and shown by built artifacts. | Release version when build source state matters |

### Relationships

- A **Player** has exactly one visible playable class, but that class may use a
  legacy **Class Slot** for compatibility.
- **General** and **Vanguard** are **Clean Heroic Power** classes and must not
  expose their legacy class-slot aliases as playable identities.
- **Player-Accessible Power** must be either **Clean Heroic Power** or explicit
  **Corruption** with warning and consequence.
- **Enemy-Only Evil** may use monsters, spells, curses, traps, lore, and hostile
  effects, but it must not become **Player-Accessible Power**.
- A **Corrupt Object** can produce **Corruption** only through confirmed player
  use, not through passive existence, storage in the **Home**, or ordinary
  inspection.
- A **Forced Upkeep Command** may share infrastructure with a **Queued Command**,
  but it is not a **Player Intent Command** and must not become a **Monster
  Order**.
- **Command Mode** controls at most one **Monster** or **Temporary Ally** from
  the **Player** perspective.
- **Energy Use** is the turn-progress contract connecting the turn engine, core
  loop, player state, and monster processing.
- **Birth** creates one **Player** with a visible class, **Starting Kit**,
  **Character History**, and initial **Player Knowledge**.
- **World Upkeep** acts on the active **Dungeon Chunk** and may request a
  **Level Transition** rather than mutating arbitrary level state directly.
- **Player Knowledge** includes **Monster Lore** and **Object Knowledge**, but it
  is not the same as authored monster or object data.
- **Object Instances** are created from **Object Kinds** and may be modified by
  **Artifacts** or **Ego Items**.
- **Monster Instances** are created from **Monster Races** through **Monster
  Allocation** unless a special effect or scenario places them directly.
- A **Front End** consumes **UI Events**, renders a **Terminal**, and must not
  decide whether **Player-Accessible Power** is legal.
- A **Scripted Full-Game Test** is a **Deterministic Test**; a **Direct Gameplay
  Pass** requires a **Test Contract**.
- A **Scenario Save** is validation setup, while a **Panic Save** is recovery
  state.
- A **Release Manifest** names **Source Archives**, **Playable Archives**, and a
  **Checksum Manifest** that all share the same **Build Identity**.

### Example Dialogue

> **Dev:** "When **Command Mode** is active, should every command become a
> **Monster Order**?"
>
> **Domain expert:** "No. Only **Player Intent Commands** are remapped. A
> **Forced Upkeep Command** from paralysis or knockout still spends the
> **Player** turn."
>
> **Dev:** "So an unchanged continuation prompt with unchanged **Energy Use** is
> not just a front-end problem?"
>
> **Domain expert:** "Correct. That is a **No-Energy Loop** in the turn engine."
>
> **Dev:** "For the **General**, can a called soldier use the old summoning
> semantics if the text is renamed?"
>
> **Domain expert:** "No. A **Temporary Ally** is a living soldier and **Clean
> Heroic Power**; renamed necromancy would still be **Forbidden Power**."
>
> **Dev:** "Can I use a **Scenario Save** as proof for the release?"
>
> **Domain expert:** "Only as validation evidence. The **Release Manifest**
> still needs final **Playable Archives**, a **Source Archive**, and a
> **Checksum Manifest** that match the **Build Identity**."

### Flagged Ambiguities

- "Command" is overloaded across **Player Intent Command**, **Queued Command**,
  **Forced Upkeep Command**, **Monster Order**, and menu actions; specs must use
  the specific term when remapping or energy is involved.
- "Command mode" can sound like generic command dispatch; use **Command Mode**
  only for directing one commanded monster or ally.
- "Turn" can mean player action, monster action, forced upkeep, energy tick, or
  loop iteration; specs must say which actor and whether **Energy Use** occurs.
- "Evil" can mean **Enemy-Only Evil** or **Forbidden Power**; never remove or
  block content solely because the word sounds evil.
- "Corruption" is not an **Ordinary Curse**; corruption is moral consequence
  from confirmed corrupt use, while ordinary curses are item afflictions.
- "Class" can mean visible playable class or internal **Class Slot**; use
  **Class Slot** for compatibility plumbing and class name for player-facing
  identity.
- "Summon" is not acceptable for **Temporary Ally** behavior unless the mechanic
  is genuinely ordinary assistance rather than undead, demonic, spirit, occult,
  or necromantic power.
- "Knowledge" can mean **Player Knowledge**, **Monster Lore**, **Object
  Knowledge**, or authored data; use the specific term when save/load or tests
  assert what the player has learned.
- "Level" can mean player level, dungeon depth, world level, or **Dungeon
  Chunk**; use **Level Transition** when describing movement between chunks.
- "Save" can mean normal **Save**, **Scenario Save**, or **Panic Save**; use the
  specific term because their ownership and safety rules differ.
- "Front end" and UI are often conflated; use **Front End** for platform
  rendering/event translation and **UI Event**, **Menu**, or **Message Prompt**
  for shared input behavior.
- "Archive" can mean **Source Archive**, **Playable Archive**, user archive, or
  panic archive; release requirements must name the package type explicitly.
- "Test" can mean **Deterministic Test**, **Scripted Full-Game Test**, **Direct
  Gameplay Pass**, or **Test Contract**; validation reports must state which
  evidence layer was used.

## Behavior

- Heroband MUST preserve classic dungeon exploration, combat, character growth,
  equipment, stores, monsters, artifacts, and terminal/front-end play unless a
  feature conflicts with the moral rule.
- The player MUST NOT gain power from demons, devils, evil spirits,
  necromancy, soul pacts, blood magic, dark rituals, forbidden occultism,
  corrupt shadow or dark power, curse benefits, life-drain, or similar morally
  corrupt sources.
- Enemy-only evil MAY remain when it is clearly antagonistic, dangerous, or
  atmospheric rather than a player benefit.
- Harmless names MAY remain when they are ordinary language or historical flavor
  and do not imply a forbidden power source available to the player.
- Ambiguous content MUST be reviewed by access path. If a player can benefit
  from it, it must be removed, blocked, replaced with a clean heroic source, or
  guarded as corrupt with warning and consequence.
- General is the canonical player-facing class that uses tactics, morale, field
  commands, and temporary living allies.
- Vanguard is the canonical player-facing class that uses courage, discipline,
  armor mastery, battlefield drills, and heroic resolve.
- General and Vanguard MAY retain legacy class-slot plumbing internally, but
  player-facing names, help, menus, books, powers, stores, and tests MUST use
  their Heroband identities.
- Corruption is a moral-risk state produced by confirmed corrupt choices. It
  MUST remain a warning and consequence system, not a class feature,
  optimization path, or power economy.
- Clean heroic power sources include courage, discipline, tactics, lawful
  command, craft, endurance, mercy, healing, light, music, nature, protection,
  morale, and heroic resolve.
- Command means one of four different concepts depending on context: player
  input intent, a queued engine command, a monster-control order, or a UI command
  menu action. Specs MUST qualify the meaning when confusion is possible.
- Command mode means the player is directing a commanded monster. It MUST NOT be
  confused with general command dispatch or input handling.
- Turn means one of player action, monster action, energy-processing step, or
  game-loop iteration. Specs MUST identify which turn type is being constrained.
- Energy use means the cost consumed by an action to advance actor time. A
  command that performs no game action SHOULD use no energy; a forced upkeep
  turn that intentionally advances time MUST use energy.
- A repeated unchanged acknowledgement prompt with unchanged player energy is a
  gameplay-loop bug signal when the player is expected to be spending forced
  turns.

## Error Handling

- If a moral-access category cannot be determined, the content is ambiguous and
  must not be promoted as player-accessible until reviewed.
- If player-facing text names a removed or forbidden class, power, realm, or
  benefit, the text is stale even when internal compatibility names remain.
- If a legacy compatibility name leaks into a player-facing menu, help page,
  saved character display, or test fixture, that is a vocabulary regression.
- If two specs use the same term with different meanings, this glossary must be
  updated before the downstream requirements are considered stable.

## Implementation Notes

Specs should describe behavior without binding requirements to a particular
source file or parser routine. Implementation may retain stable identifiers,
data order, save fields, and shared evil systems when player access is properly
gated.

Moral replacement must change the actual mechanic and power source, not only the
visible name. A renamed forbidden mechanic is still forbidden.

## Test Scenarios

- Moral classification audit: given an evil-themed entry, classify it as
  player-accessible, enemy-only, harmless, or ambiguous, then verify the chosen
  treatment matches the access path.
- Legacy slot leak check: create or inspect a General or Vanguard character and
  verify player-facing menus, books, help, dumps, and scripted tests use the
  Heroband names.
- Forbidden power regression: search player-accessible class powers, books,
  stores, activations, equipment benefits, and help text for demon, necromancy,
  blood, soul, occult, corrupt shadow, life-drain, and curse-benefit access.
- Enemy-only preservation: verify demon, undead, curse, nether, and evil monster
  content remains allowed when it is only an enemy attack, monster category, trap,
  lore threat, or corrupt danger.
- Term collision review: when adding a command, turn, or energy requirement,
  verify the spec qualifies whether it refers to input, queue, monster order,
  player action, monster action, or loop progress.

## Changelog

- 1.0.0: Initial glossary extracted from the Heroband design rule, current class
  replacement notes, corruption guidance, and active command/turn terminology.
