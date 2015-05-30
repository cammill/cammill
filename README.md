2D CAM-Tool for Linux and OSX

[![Build Status](https://travis-ci.org/cammill/cammill.svg?branch=master)](https://travis-ci.org/cammill/cammill)

Features:
* Automatic Offsets (Inside / Outside)
* Normal and Climb milling
* Rotary-Axis Support
* Laser-Mode (inc. Offsets)
* Tool/Material calculations for feedrate and tool-speed
* Holding-Tab's
* Support for milling MTEXT from dxf
* LinuxCNC-Tooltable import
* Setup-Presets
* Post-Processor (with lua-script / compatible with sheetcam-tng)
* Running under Linux and Mac-OSX (and maybe other Unix-Like Systems)
* .....
Missing Features: see [#6](https://github.com/cammill/cammill/issues/6)

Homepage: http://www.multixmedia.org/cammill/

### Compile under Linux (Debian Wheezy/Jessie)

```bash
$ apt-get install clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev
```

```bash
$ git clone https://github.com/cammill/cammill.git
$ cd cammill
$ make
$ ./cammill test.dxf
```

### Compile under Mac-OSX

```bash
$ port selfupdate
$ port upgrade outdated
$ port install gtkglext
$ port install gtksourceview2
$ port install lua
```

```bash
$ git clone https://github.com/multigcs/cammill.git
$ cd cammill
$ make -f Makefile.osx
$ ./cammill test.dxf
```
