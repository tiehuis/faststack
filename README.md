FastStack
=========

A customisable tetris game aimed at line-race modes.

![SDL Frontend](/res/sdl-example.png)

This is largely adapted from LockJaw. Many existing features have been removed
such as support for the DS/GBA and block connections amongst others. This was
done primarily because this is effectively a rewrite using LockJaw as a basis
on how to structure the project, but also because they aren't part of the
feature set I want to use.

The internal code is altered so that game logic is not tied to the draw phase
and the internal tick rate is configurable.

I want to keep this simple and a good foundation to build on top of, but
reduced in scope enough that the focus is clear. This isn't an end-all
emulator like NullpoMino.

Another important reason for this is that I want to play 40L mode, but
NullpoMino currently lags immensely and massively slows down other programs
on my current java version. This version uses no dynamically allocated
variables in the core engine so should be almost as quick as it can be.
Maybe in the future I'll even adapt it to run on an embedded device?

Also, there weren't any good serious terminal based tetris games. They usually
offered really poor input choices in my experience.

Frontends
---------

The current frontend uses the SDL2 library. There is an in-progress terminal
frontend which is assumes linux and vt100 but otherwise doesn't have any other
dependencies.

The GUI will never be fully-featured, instead it is much more likely that
things such as keybindings and options are configured through an INI file.
If it remains lean and quick there is little reason to configure it any other
way.

See the specific README files for details.

 * [SDL2](./src/frontend/SDL2/README.md)
 * [Terminal](./src/frontend/terminal/README.md)

Contributing
------------

I'd like to keep this fairly active. If you are reading this let me know if you
have any ideas and/or requests and I may think about implementing them.

License
-------

GPLv3 licensed. I'm usually an MIT sort of person, but Lockjaw was GPLv2
(or later) and for a simple application, it fits okay anyway.
