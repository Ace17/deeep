# Deeep

Author: Sebastien Alaiwan

Demo
----

An online demo is available here:

http://code.alaiwan.org/games/deeep

<p align="center"><img src="doc/screenshot.jpg" width="50%"></p>

Description
-----------

This is a demo of a platform-independent C++14 game project.
It uses SDL2, and OpenGL ES 3.0.

It's meant as an example of a platformer/metroidvania game
which can be compiled to native code, or, using Emscripten, to Javascript,
and maybe one day to WebAssembly.

The code doesn't contain any reference/dependency to Emscripten, except in the
entry-point file, where the main loop function gets passed to Emscripten.

This code also shows how to isolate your game logic code (doors, switches,
powerups, bullets, ..) from your I/O code (display, audio, input).

Directory structure
-------------------

```
bin:            output directory for architecture-specific executable binaries.
res:            output directory for game resources (e.g. sounds, music, sprites, tilesets).
assets:         source files for game resources.
src:            source files for the game logic (agnostic to the engine implementation).
engine/src:     I/O code (=engine), mostly game-agnostic.
engine/include: interfaces for communication between the game logic and the I/O code. Also contains shared low-level utilities (e.g Vector2f, Span, etc.).
./check:        main check script. Call this to build native and asmjs versions and to launch the unit tests.
```


Build
-----

Requirements:
```
* libsdl2-dev
```

It can be compiled to native code using your native compiler (gcc or clang):

```
$ make
```

The binaries will be generated to a 'bin' directory
(This can be overriden using the BIN makefile variable).

It can also be compiled to WebAssembly, using the Emscripten SDK.

```
$ source /path/to/your/emsdk_env.sh
$ ./scripts/deliver
```

Run the game
------------

Just run the following command:

```
$ bin/rel/game.exe
```

Convert screenshot to PNG
-------------------------

```
ffmpeg -f rawvideo -pixel_format rgba -video_size 768x768 -framerate 25 -i screenshot.rgba -y screenshot.png
```

Thanks to:
----------

- llexandro from DevianArt (Textures)

