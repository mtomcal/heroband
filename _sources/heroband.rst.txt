========
Heroband
========

Heroband is a morally heroic fork of Angband 4.2.6.  The central rule is:
the player may fight evil, but may not wield evil.

Demons, undead, corruption, Morgoth, Sauron, and other evil forces remain as
enemies and dangers.  Enemy-only evil is part of the good-against-evil
atmosphere.  Player-facing corrupt power is removed, blocked, or replaced with
clean heroic alternatives.

Heroic Classes
==============

General
  The General replaces the old Necromancer player slot.  Generals lead through
  discipline, morale, and battlefield command.  Their field manuals teach
  orders that call temporary living soldiers, rally courage, strengthen a shield
  wall, expose enemy openings, and withdraw in good order.  They do not draw
  power from spirits, undeath, blood, shadow, demons, or occult ritual.

Vanguard
  The Vanguard replaces the old Blackguard player slot.  Vanguards are armored
  frontline champions who rely on courage, discipline, armor mastery,
  battlefield drills, and heroic resolve.  Their powers are martial orders and
  clean tactics, not bloodlust, shadow magic, demonic power, curse benefits, or
  forbidden arts.

Corruption
==========

Some evil artifacts and Morgul weapons are corrupt.  Heroband warns before you
equip or activate corrupt power.  If you confirm that use, corruption is
recorded on the character, can bring hostile consequences, and can doom a final
victory.

Corruption is not a class path, a safe optimization, or a renamed heroic power.
It is a moral failure state attached to dangerous objects.

Downloads
=========

The current public prerelease is `Heroband 0.1.0`_.

Linux x86_64
  Download ``heroband-0.1.0-linux-x86_64.tar.gz``, extract it, and run
  ``./angband`` from the extracted directory.  The archive includes the
  executable, runtime data, and the built HTML manual.

Source package
  Download ``heroband-0.1.0-source.tar.gz`` to build from source.  The archive
  includes generated autotools files, including ``configure`` and
  ``src/autoconf.h.in``, along with the CMake build files.

Checksums
  Download ``heroband-0.1.0-checksums.txt`` and verify the release assets with
  ``sha256sum -c heroband-0.1.0-checksums.txt``.

.. _Heroband 0.1.0: https://github.com/mtomcal/heroband/releases/tag/heroband-0.1.0

Compatibility Notes
===================

Some internal class-slot identifiers still use old Angband names for parser,
save, and data compatibility.  Those identifiers are implementation plumbing
unless they expose forbidden player-facing content.
