[![Gitter chat](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/OpenSWE1R)       [![Travis build Status](https://travis-ci.org/OpenSWE1R/openswe1r.svg?branch=master)](https://travis-ci.org/OpenSWE1R/openswe1r)       [![AppVeyor Build status](https://ci.appveyor.com/api/projects/status/92s5hpto3kvn8sx3/branch/master?svg=true)](https://ci.appveyor.com/project/JayFoxRox82949/openswe1r/branch/master)

---

### OpenSWE1R
*An Open-Source port of the 1999 Game ["Star Wars: Episode 1 Racer"](https://en.wikipedia.org/wiki/Star_Wars_Episode_I:_Racer)*

![Screenshot](https://i.imgur.com/oFsQUXc.png)

### Details

This project is a mix of console emulation and something like [WINE](https://www.winehq.org/).
It implements the subset of Windows functions used by the game.
OpenSWE1R also contains x86 CPU emulation.
The original games code is replaced by open-source code which is patched into the game at runtime.
This is similar to the approach used by [OpenRCT2](https://openrct2.website/), but more emphasis is put on emulation and platform independence.

Currently, not all functions are implemented in the open-source portion.
Therefore, this project also still relies on the original games code.
Even if not all code is patched, the game will still work cross-platform due to the API and CPU abstraction.

You will still need the original game for it's art assets: levels, sounds, ...

### Goals

The goal is to eventually replace all of the original code with an open-source equivalent.
The code is supposed to be clean, accurate and portable.
It's not a goal to fix any bugs or modify the games behaviour.
If you want to do such things, they should stay in a fork.

An exception is platform-independence and improvements to the overall code quality.
Some compromises are acceptable.
Please talk to the maintainers if you have a specific case.

It's possible that support for other versions (Mac / Dreamcast / N64) will be added later.
For now, the goal is to support the x86 Windows Version of the game.

### Requirements

**System**

* OpenGL 3.3 Core

**Toolchain**

* [git](https://git-scm.com/)
* [CMake](https://cmake.org/)
* C11 toolchain

**Libraries**

* [Unicorn-Engine](http://www.unicorn-engine.org/)
* [SDL2](https://www.libsdl.org/)
* [GLEW](http://glew.sourceforge.net/)
* [DevIL](http://openil.sourceforge.net/)

### Building

From your desired project folder, run:

```
git clone https://github.com/OpenSWE1R/openswe1r.git
cd openswe1r
mkdir build
cd build
cmake ..
make
```

### Running

The `openswe1r` binary contained in the build directory is a drop-in replacement for `swep1rcr.exe`.
Copy it to your game folder and run it.

### Development

Development happens on GitHub at https://github.com/OpenSWE1R/openswe1r
You can help by reviewing other peoples Pull-Requests or sending your own after forking.

---

**© 2017 OpenSWE1R Maintainers**

Licensed under GPL version 2 or later.
