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
Missing Features: see [#6](https://github.com/cammill/cammill/issues/6)

Homepage: http://www.multixmedia.org/cammill/

### Compile under Linux (Debian Wheezy/Jessie)

```bash
apt-get install clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev libg3d-dev
```

```bash
cd /usr/src
git clone https://github.com/cammill/cammill.git
cd cammill
make
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
make LIBS="-framework OpenGL -framework GLUT -lm -lpthread -lstdc++ -lc" PKGS="gtk+-2.0 gtkglext-1.0 gtksourceview-2.0 lua"
./cammill test.dxf
```

### Cross-Compile Windows (32 Bit) unter Linux

Compile and install [mxe](http://mxe.cc/):
```bash
cd /usr/src
git clone https://github.com/mxe/mxe.git
cd mxe
make MXE_TARGETS='i686-w64-mingw32.static' gcc gtk2 lua gtkglext gtksourceview freeglut
export PATH=`pwd`/usr/bin:$PATH
```
Cross-compile ```cammill.exe```:
```
cd /usr/src
git clone https://github.com/cammill/cammill.git
cd cammill
make PROGRAM=cammill.exe LIBS="-lm -lstdc++ -lgcc" CROSS=i686-w64-mingw32.static- COMP=i686-w64-mingw32.static-gcc PKGS="gtk+-2.0 gtk+-win32-2.0 gtkglext-1.0 gtksourceview-2.0 lua"
```

### IRC

[irc.freenode.org](http://www.freenode.org/) #cammill
