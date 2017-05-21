faststack-SDL
=============

<p align="center">
    <img src="https://i.imgur.com/8NilljB.png">
</p>

Platforms: Linux, Windows, OSX (any with SDL2 support)

This is built atop the SDL2 library. This is a popular cross-platform library
for abstracted access to the hardware. This version should run on all major
platforms.

Installation
------------

You will require the SDL2 and SDL2_ttf libraries on your system to dynamically
link against. This are found in most repositories.

```
meson build # Only on first run
cd build
mesonconf -Dfrontend=sdl
ninja
```
