# Heroband

<p align="center">
  <img src="screenshots/title.png" width="425"/>
  <img src="screenshots/game.png" width="425"/>
</p>

Heroband is a morally heroic fork of Angband 4.2.6. It is a graphical dungeon
adventure game that uses textual characters to represent the walls and floors of
a dungeon and the inhabitants therein, in the vein of games like NetHack and
Rogue. If you need help in-game, press `?`.

The central Heroband rule is: the player may fight evil, but may not wield evil.
Demons, undead, corruption, Morgoth, Sauron, and other evil forces remain as
enemies and dangers. Player-facing corrupt power is removed, blocked, or replaced
with clean heroic alternatives.

Recent Heroband changes include:

- **General:** replaces the old Necromancer player slot with discipline, morale,
  field commands, and temporary living allies.
- **Vanguard:** replaces the old Blackguard player slot with courage, armor
  mastery, battlefield drills, and heroic resolve.
- **Corruption gates:** corrupt artifacts and Morgul weapons warn before use,
  record corruption when confirmed, can bring hostile consequences, and can
  invalidate a final victory.

## Downloads

The current prerelease is
[Heroband 0.1.0](https://github.com/mtomcal/heroband/releases/tag/heroband-0.1.0).

- **Linux x86_64:** download `heroband-0.1.0-linux-x86_64.tar.gz`, extract it,
  and run `./angband` from the extracted directory.
- **Source package:** download `heroband-0.1.0-source.tar.gz`. This archive
  includes generated autotools files (`configure` and `src/autoconf.h.in`) as
  well as the CMake build files.
- **Checksums:** download `heroband-0.1.0-checksums.txt` and verify with
  `sha256sum -c heroband-0.1.0-checksums.txt`.

- **How to Play:** [The Heroband Manual](https://mtomcal.github.io/heroband/)
- **Compile it yourself:** [Building from source](https://mtomcal.github.io/heroband/hacking/compiling.html)
- **Developer workflow:** [Heroband development notes](https://mtomcal.github.io/heroband/hacking/heroband.html)
- **Upstream project:** [Angband](https://angband.github.io/angband/)

Heroband builds on decades of Angband development. See the manual and source
history for upstream credits and license details.
