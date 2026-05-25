==========================
Frequently Asked Questions
==========================

The best way to get answers to your questions is to check the Heroband GitHub project or the public manual. For inherited gameplay questions, the upstream Angband forum can still be useful.

.. contents:: Contents
   :local:

Issues and problems
-------------------

How do I report a bug?
~~~~~~~~~~~~~~~~~~~~~~

Open an issue on the Heroband GitHub project.

Bug reports should include:

* your current operating system (e.g. Windows 10)
* what version the problem appeared in
* the best steps you can figure out to reproduce the bug.

Savefiles that show the problem might be requested, because they help tracking bugs down.

Dark monsters are hard to see
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Fix (reduce) the alpha on your screen, or use the "Interact with colors" screen under the options (``=``) menu.  Navigate to the ``8`` using ``n`` and increase the color intensity with r(ed)/g(reen)/b(lue).

.. _x11-fonts:

How do I avoid the "Couldn't load the requested X11 font (10x20)" message?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The X11 frontend requires *legacy bitmap fonts*. Many modern systems no longer install those fonts by default, or install them in locations that X11 does not search automatically.

Common ways to resolve this include:

* Install legacy X11 bitmap font packages provided by your operating system (package names vary by distribution).
* Explicitly select an installed bitmap font by setting the ``ANGBAND_X11_FONT`` environment variable.
* Use a different frontend (SDL, SDL2, Windows, macOS) that does not rely on X11 bitmap fonts.

On Linux, the required font is typically provided as ``10x20.pcf.gz``. If your distribution allows searching packages by installed files, look for a package that provides that file. Common examples include:

* Debian/Ubuntu and derivatives: ``xfonts-base``
* Red Hat–derived distributions: ``xorg-x11-fonts-misc``
* Arch Linux: ``xorg-fonts-misc``

Depending on the distribution and configuration, installing these packages may not be sufficient if X11 is not configured to search the installed font paths.

If none of the above works, open an issue with your OS, distribution, frontend, and Heroband version.

Is there a way to disable that thing that pops up when you hit the enter key?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Go into the options menu, choose "Edit keymaps", then "Create a keymap".  Press Enter at the "Key" prompt and a single space as the "Action".

And then you'll probably want to choose to "Save keymaps to a file", and either choose the file name so that it is one automatically loaded when a character is loaded or combine the contents of the saved file with one of the automatically loaded preference files. That allows the change to stay in effect the next time you load the game.

This just replaces the default action of Enter with a "do nothing but don't tell me about help" action. If you want to keep the menu available, say on the 'Tab' key, you can also remap the Tab keypress to the ``\n`` action.


Development
-----------

What are the current plans for the game?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Heroband plans are tracked in the GitHub project and in the fork-specific development notes.

How do I suggest an idea/feature?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Open an issue or discussion on GitHub.  If the idea fits Heroband's direction,
it can be discussed there and turned into a focused implementation plan.

Some suggestions may not fit Heroband. In particular, player-facing power from demons, undead, blood magic, shadow magic, necromancy, soul pacts, curse benefits, or occult ritual conflicts with the fork rule: the player may fight evil, but may not wield evil.

How do I get a copy of the source code?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Go to the GitHub_ page, where you can find the bleeding edge as well as all previous versions.

How do I compile the game?
~~~~~~~~~~~~~~~~~~~~~~~~~~

Please see the :doc:`compiling section of the manual <hacking/compiling>`.

How do I contribute to the game?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You have two options:

* Write your patch and submit it as a pull request on GitHub.
* Open a GitHub issue with a focused bug report, test case, or proposal.

All contributions are accepted as dual-licenced with both the Angband and GPLv2 licences.

There are contribution guidelines in CONTRIBUTING.md in the top level directory of the source code.

Bug fixes and Heroband-aligned improvements are welcome. Gameplay changes should preserve Angband behavior unless it conflicts with Heroband's moral design rule.

Documentation, test cases, playtest reports, tiles, and release packaging improvements are also useful contributions.


.. _GitHub: https://github.com/mtomcal/heroband/
.. _Angband forum: https://angband.live/forums/
