FastStack
=========

<p align="center">
    <img src="http://i.imgur.com/M9RXMpH.gif"/>
</p>

A heavily customisable speed-oriented puzzle game.

This is primarily intended for line-race modes at the current stage. If you
want a more feature-filled game, I would suggest
[Nullpomino](https://github.com/nullpomino/nullpomino) instead.

Motivation
----------

Nullpomino has a few problems on Linux with new Java versions so I needed
another lightweight game as an alternative. Another reason was that there are
no decent terminal versions.

See the specific README files for details about the different frontends.

 * [SDL2](./src/frontend/SDL2/README.md)
 * [Terminal](./src/frontend/terminal/README.md)

Goals
-----

 * The core engine should be as lean as possible. No dynamic memory allocation
   if possible, and strictly conforming to C99 with no external dependencies.

 * The main focus is on line-race modes. Other modes could be added in the
   future, but it is important to limit the scope for the moment.

Contributing
------------

If you have any ideas or requests, feel free to create an issue.

License
-------

GPLv3 licensed. See the [README](./README.md).
