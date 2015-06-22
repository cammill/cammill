2D CAM-Tool for Linux, Windows and Mac OS X

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
Missing Features: see [#6](https://github.com/cammill/cammill/issues/6), help [translate](https://crowdin.com/project/cammill).

Homepage: http://www.multixmedia.org/cammill/

### Compile under Linux (Debian Wheezy/Jessie)

```bash
apt-get install clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev libg3d-dev
```

```bash
cd /usr/src
git clone https://github.com/cammill/cammill.git
cd cammill
make all install
./cammill test.dxf
```

### Compile under Mac OS X

```bash
export PKG_CONFIG_PATH=/opt/X11/lib/pkgconfig
brew update
brew install gtkglext
brew install gtksourceview
brew install lua
brew install gettext
brew link gettext --force
```

```bash
git clone https://github.com/cammill/cammill.git
cd cammill
make TARGET=OSX clean all
./cammill test.dxf
```

### Cross-Compile Windows (32 Bit) under Linux

Compile and install [mxe](http://mxe.cc/):
```bash
cd /usr/src
git clone https://github.com/mxe/mxe.git
cd mxe
make MXE_TARGETS='i686-w64-mingw32.static' gcc gtk2 lua gtkglext gtksourceview freeglut
```
Cross-compile ```cammill.exe```:
```
cd /usr/src
git clone https://github.com/cammill/cammill.git
cd cammill
make TARGET=MINGW32 CROSS=/usr/src/mxe/usr/bin/i686-w64-mingw32.static- clean all 
wine cammill.exe test-minimal.dxf
```
generate installer-package ```cammill-installer.exe```:
```
make TARGET=MINGW32 CROSS=/usr/src/mxe/usr/bin/i686-w64-mingw32.static- package 
wine Windows/cammill-installer.exe
```

## Compile under FreeBSD (10.0)

```bash
pkg install git gmake pkgconf gettext freeglut gtkglext gtksourceview2 lua51
git clone https://github.com/cammill/cammill
cd cammill/
gmake TARGET=FREEBSD clean all
./cammill test.dxf
```
generate installer-package ```cammill-freebsd-0.9.tgz```:
```
make TARGET=FREEBSD package 
pkg install cammill-freebsd-0.9.tgz
```

## Compile under OpenBSD (5.7)

```bash
pkg_add git gcc gmake freeglut gtk+ gtksourceview gtkglext lua
git clone https://github.com/cammill/cammill
cd cammill/
gmake TARGET=OPENBSD clean all
./cammill test.dxf
```

### IRC

[irc.freenode.org](http://www.freenode.org/) #cammill
