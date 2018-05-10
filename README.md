[![Gitter chat](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/OpenSWE1R)       [![CLA assistant](https://cla-assistant.io/readme/badge/OpenSWE1R/openswe1r)](https://cla-assistant.io/OpenSWE1R/openswe1r)       [![Travis build Status](https://travis-ci.org/OpenSWE1R/openswe1r.svg?branch=master)](https://travis-ci.org/OpenSWE1R/openswe1r)       [![AppVeyor Build status](https://ci.appveyor.com/api/projects/status/92s5hpto3kvn8sx3/branch/master?svg=true)](https://ci.appveyor.com/project/JayFoxRox82949/openswe1r/branch/master)

---

### OpenSWE1R
*An Open-Source port of the 1999 Game ["Star Wars: Episode 1 Racer"](https://en.wikipedia.org/wiki/Star_Wars_Episode_I:_Racer)*

![Screenshot of OpenSWE1R running on Linux](https://i.imgur.com/LAqHPgh.png)

### Details

This project is a mix of console emulation and something like [WINE](https://www.winehq.org/).
It implements the subset of Windows functions used by the game.
OpenSWE1R also contains x86 CPU emulation.
The original games code is replaced by open-source code which is patched into the game at runtime.
This is similar to the approach used by [OpenRCT2](https://openrct2.website/), but more emphasis is put on emulation and platform independence.

Currently, not all functions are implemented in the open-source portion.
Therefore, this project also still relies on the original games binary.
Even if not all code is patched, the game will still work cross-platform due to the API and CPU abstraction.

You will still need the original game for its art assets: levels, sounds, ...

### Goals

The goal is to eventually replace all of the original code with an open-source equivalent.
The code is supposed to be clean, accurate and portable.

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
* [OpenAL](https://www.openal.org/)

### Getting Started

Read our ["Getting Started" guide on the wiki](https://github.com/OpenSWE1R/openswe1r/wiki/Getting-Started) to learn how to build and run OpenSWE1R.

### Development

Development happens on GitHub at https://github.com/OpenSWE1R/openswe1r
You can help by reviewing or submitting Pull-Requests.

If you want to contribute, you'll have to [sign our Contributor License Agreement (CLA)](https://cla-assistant.io/OpenSWE1R/openswe1r).
The CLA allows us to easily switch to other [licenses the FSF classifies as Free Software License](https://www.gnu.org/licenses/license-list.html) and which are [approved by the OSI as Open Source licenses](https://opensource.org/licenses), if the need should ever arise ([more information](https://github.com/OpenSWE1R/openswe1r/pull/95)).

---

**© 2017 - 2018 OpenSWE1R Maintainers**

Source code licensed under GPLv2 or any later version.
Binaries which link against the default Unicorn-Engine backend must be licensed under GPLv2.

OpenSWE1R is not affiliated with, endorsed by, or sponsored by The Walt Disney Company, Twenty-First Century Fox, the games original developers, publishers or any of their affiliates or subsidiaries.
All product and company names are trademarks™ or registered® trademarks of their respective holders. Use of them does not imply any affiliation with or endorsement by them.

Reverse engineering of the original software is done to achieve interoperability with other computing platforms.
In the process, excerpts of the reverse engineered source code might be shown for educational purposes.

No copyright infringement is intended at any stage during development of OpenSWE1R.
