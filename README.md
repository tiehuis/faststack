faststack
=========

<p align="center">
    <img src="https://i.imgur.com/dCFmGih.png">
</p>

A simple customizable tetris variant.

This is primarily targeted at fast line-race play. If you want a more
feature-filled game, I would suggest [NullpoMino](https://github.com/nullpomino/nullpomino)
instead.

Motivation
----------

 - NullpoMino doesn't run on linux for me.
 - No decent terminal tetris games that target fast 40-line play.
 - Wanted something to do.

Installation
------------

This requires [`meson`](http://mesonbuild.com/Getting-meson.html) to build along
with any C99 compiler. Some frontends have additional requirements.

```
meson build
cd build
ninja
```

### Switching Frontends

```
cd build
mesonconf -Dfrontend=sdl # or -Dfrontend=terminal
ninja
```

NOTE: Some frontends have individual dependencies not listed here. See their
subdirectories for details.

Structure
---------

The rough project structure is as follows:

 - [`src/engine`](./src/engine)
    Contains the core tetris engine code that does the logic/work. This is
    independent of any graphical code.

 - `src/frontend`
    - [`terminal`](./src/frontend/terminal)
        Contains a frontend which renders directly to a linux terminal.

    - [`SDL2`](./src/frontend/SDL2)
        Contains an SDL2 graphical interface.

    - [`kernel`](./src/frontend/kernel)
        Contains a complete operating system base and a minimal frontend which
        can run the faststack engine for iX86/x86_64 systems.

Contributing
------------

If you have any ideas or requests, feel free to create an issue.

License
-------

GPLv3 licensed. See the [README](./README.md).

The core engine is based vary loosely in structure on the old `lockjaw`
game, which was licensed under the GPLv2 or later. Mostly zero-code is shared,
however some design similarities may be noticeable.
