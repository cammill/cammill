# CAMmill [![travis](https://travis-ci.org/cammill/cammill.svg?branch=master)](https://travis-ci.org/cammill/cammill)

2D CAM tool for Linux/BSD, Windows and Mac OS X

![license](https://img.shields.io/github/license/cammill/cammill.svg)
[![release](https://img.shields.io/github/release/cammill/cammill.svg)](https://github.com/cammill/cammill/releases)
[![issues](https://img.shields.io/github/issues/cammill/cammill.svg)](https://github.com/cammill/cammill/issues)

Features:
* Automatic Offsets (Inside / Outside)
* Normal and Climb milling
* Rotary-Axis Support
* Laser-Mode (inc. Offsets)
* Tool/Material calculations for feedrate and tool-speed
* Holding-Tab's
* Support for milling MTEXT from dxf
* LinuxCNC tool table import
* Setup-Presets
* Post-Processor (with Lua script / compatible with [SheetCam-tng](http://www.sheetcam.com/))

Missing Features: see [#6](https://github.com/cammill/cammill/issues/6), please help [translate](https://crowdin.com/project/cammill).

Homepage: http://www.multixmedia.org/cammill/

### Building for Mac OS X
Prerequisites:
* XCode, including XCode command-line tools.

#### Mac OS X (10.9)

(assumes [Homebrew](http://brew.sh/) is already installed)
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

### Building for Linux/BSD

#### Debian (Wheezy/Jessie)

```bash
apt-get install clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev libg3d-dev help2man
```

```bash
cd /usr/src
git clone https://github.com/cammill/cammill.git
cd cammill
make clean all
./cammill test.dxf
```
Generate installer-package ```cammill_0.9_i386.deb```:
```
make package 
dpkg -i packages/cammill_0.9_i386.deb
```

#### FreeBSD (10.0)

```bash
pkg install git gmake pkgconf gettext freeglut gtkglext gtksourceview2 lua51
git clone https://github.com/cammill/cammill
cd cammill/
gmake TARGET=FREEBSD clean all
./cammill test.dxf
```
Generate installer-package ```cammill-freebsd-0.9.tgz```:
```
gmake TARGET=FREEBSD package 
pkg install packages/cammill-freebsd-0.9.tgz
```

#### NetBSD (6.1)

```bash
pkgin install git gcc49 freeglut gtksourceview gtkglext lua51
git clone https://github.com/cammill/cammill
cd cammill/
gmake TARGET=NETBSD clean all
./cammill test.dxf
```

#### OpenBSD (5.7)

```bash
pkg_add git gcc gmake freeglut gtk+ gtksourceview gtkglext lua
git clone https://github.com/cammill/cammill
cd cammill/
gmake TARGET=OPENBSD clean all
./cammill test.dxf
```

### Building for Windows

#### Cross-Compile Windows (32 Bit) under Linux

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
Generate installer-package ```cammill-installer.exe```:
```
make TARGET=MINGW32 CROSS=/usr/src/mxe/usr/bin/i686-w64-mingw32.static- package 
wine packages/cammill-installer.exe
```

### Compilation

If you had problems compiling from source, raise a new issue in the [issue tracker on the github page](https://github.com/cammill/cammill/issues).

### Contact

#### IRC
[#cammill](http://webchat.freenode.net?nick=webchat_user&channels=%23cammill&prompt=1&uio=MTE9MjM20f) (FreeNode)

#### Facebook
[https://www.facebook.com/cammill2d](https://www.facebook.com/cammill2d)
