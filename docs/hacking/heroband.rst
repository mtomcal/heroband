======================
Heroband Development
======================

Heroband keeps Angband's core structure and compatibility where practical, but
adds a moral design rule for player-facing content: the player may fight evil,
but may not wield evil.

When changing gameplay, first classify suspicious content as player-accessible,
enemy-only, harmless, or ambiguous.  Enemy-only evil can remain.  Player-facing
power from demons, undead, blood, shadow, necromancy, soul pacts, curse
benefits, or occult ritual must be removed, blocked, or replaced with a clean
heroic source.

Recent Compatibility Notes
==========================

General and Vanguard use old Angband class slots internally so parser and save
compatibility stay stable.  Treat legacy internal names as plumbing unless they
surface in player-facing text or mechanics.

Corrupt artifacts and Morgul weapons are intentionally still present as dangers.
They warn before use, record corruption when confirmed, can trigger hostile
consequences, and can prevent a clean final victory.  Do not turn corruption
into a class feature or optimization path.

Validation Workflow
===================

For ordinary buildable checkpoints, use the configured CMake build and tests:

.. code-block:: sh

   cmake --build build -j2
   cmake --build build -t alltests -j2

For player-facing gameplay, birth flow, class powers, stores, inventory, spells,
save/load, or moral-restriction changes, use the repo-local Heroband playtest
workflow.  Start with a written test contract, then run deterministic tests and
a terminal playtest when appropriate.

Preferred wrapper commands:

.. code-block:: sh

   scripts/heroband-playtest start --contract /path/to/TEST_CONTRACT.md
   scripts/heroband-playtest capture --state-dir /tmp/heroband-playtest.xxxxxx
   scripts/heroband-playtest send --state-dir /tmp/heroband-playtest.xxxxxx Space
   scripts/heroband-playtest stop --state-dir /tmp/heroband-playtest.xxxxxx

When adding or changing gameplay, class-power, birth, store, save/load,
scenario-save, or moral-restriction tests, also run the Heroband test-quality
verifier.  It checks that tests exercise player-visible behavior instead of
bypassing setup, relying on weak assertions, or only checking renamed text.

Scenario Saves
==============

The ``scripts/heroband-corruption-scenario-save`` helper creates focused
scenario saves for corruption checks.  Use scenario saves to exercise real
equipment, activation, consequence, and victory behavior when unit tests alone
would be too indirect.

Release Packaging
=================

Heroband releases are published from GitHub Releases.  Heroband 0.3.1 includes
a Linux x86_64 playable archive, a source archive, and SHA-256 checksums.

The attached source archive is prepared from tracked files and then runs
``./autogen.sh`` before packaging.  That means the release source tarball
contains generated autotools files such as ``configure`` and
``src/autoconf.h.in`` and does not require release users to have ``aclocal`` for
the initial configure-based build.  Maintainers who regenerate that source
archive need automake and autoconf installed:

.. code-block:: sh

   sudo apt install automake autoconf

After replacing any release asset, also regenerate and replace the checksum
manifest so it matches the downloadable files:

.. code-block:: sh

   sha256sum heroband-0.3.1-source.tar.gz \
     heroband-0.3.1-linux-x86_64.tar.gz > heroband-0.3.1-checksums.txt
