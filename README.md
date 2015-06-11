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
git clone https://github.com/cammill/cammill.git
cd cammill
make
./cammill test.dxf
```

### Compile under Mac OS X

```bash
port selfupdate
port upgrade outdated
port install gtkglext
port install gtksourceview2
port install lua
```

```bash
git clone https://github.com/cammill/cammill.git
cd cammill
make -f Makefile.osx
./cammill test.dxf
```

### Cross-Compile Windows (32 Bit) unter Linux

Compile and install [mxe](http://mxe.cc/):
```bash
cd /opt
git clone https://github.com/mxe/mxe.git
cd mxe
make MXE_TARGETS='i686-w64-mingw32.static' gcc gtk2 lua gtkglext gtksourceview freeglut
export PATH=`pwd`/usr/bin:$PATH
```
Cross-compile ```cammill.exe```:
```
cd /opt
git clone https://github.com/cammill/cammill.git
cd cammill
make PROGRAM=cammill.exe LIBS="-lm -lstdc++ -lgcc" CROSS=i686-w64-mingw32.static- COMP=i686-w64-mingw32.static-gcc PKGS="gtk+-2.0 gtk+-win32-2.0 gtkglext-1.0 gtksourceview-2.0 lua"
```

### IRC

[irc.freenode.org](http://www.freenode.org/) #cammill
