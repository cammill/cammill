
#TARGETS: DEFAULT, FREEBSD, MINGW32, OSX, OPENBSD
TARGET ?= DEFAULT

#autodetect system
ifeq (${TARGET}, DEFAULT)
	ifeq ($(OS),Windows_NT)
		TARGET = MINGW32
		ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			CPU = AMD64
		endif
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			CPU = IA32
		endif
	else
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S),Linux)
			TARGET = DEFAULT
		endif
		ifeq ($(UNAME_S),FreeBSD)
			TARGET = FREEBSD
		endif
		ifeq ($(UNAME_S),OpenBSD)
			TARGET = OPENBSD
		endif
		ifeq ($(UNAME_S),NetBSD)
			TARGET = NETBSD
		endif
		ifeq ($(UNAME_S),Darwin)
			TARGET = OSX
		endif
		UNAME_P := $(shell uname -p)
		ifeq ($(UNAME_P),x86_64)
			CPU = AMD64
		endif
		ifneq ($(filter %86,$(UNAME_P)),)
			CPU = IA32
		endif
		ifneq ($(filter arm%,$(UNAME_P)),)
			CPU = ARM
		endif
	endif
endif



ifeq (${TARGET}, MINGW32)
	PROGRAM         ?= cammill.exe
	LIBS            ?= -lm -lstdc++ -lgcc
	CROSS           ?= i686-w64-mingw32.static-
	COMP            ?= ${CROSS}gcc
	PKGS            ?= gtk+-2.0 gtk+-win32-2.0 gtkglext-1.0 gtksourceview-2.0 lua
	INSTALL_PATH    ?= packages/windows/CAMmill
endif

ifeq (${TARGET}, OSX)
	CFLAGS          += "-Wno-deprecated"
	LIBS            ?= -framework OpenGL -framework GLUT -lm -lpthread -lstdc++ -lc
	PKGS            ?= gtk+-2.0 gtkglext-1.0 gtksourceview-2.0 lua
    PKG_CONFIG_PATH ?= /opt/X11/lib/pkgconfig
	INSTALL_PATH    ?= packages/osx/CAMmill
endif

ifeq (${TARGET}, FREEBSD)
	COMP            ?= clang
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua-5.1
endif

ifeq (${TARGET}, NETBSD)
	COMP            ?= gcc
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua-5.1
endif

ifeq (${TARGET}, OPENBSD)
	COMP            ?= gcc
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua51
endif

COMP       ?= $(CROSS)clang
PKG_CONFIG ?= $(CROSS)pkg-config
VERSION    ?= 0.9

HERSHEY_FONTS_DIR = ./
PROGRAM ?= cammill
INSTALL_PATH ?= /opt/${PROGRAM}

MAINTAINER_NAME  ?= Oliver Dippel
MAINTAINER_EMAIL ?= oliver@multixmedia.org

LIBS   ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lstdc++ -lXext -lXi -lxcb -lXau -lXdmcp -lgcc -lc
CFLAGS += -I./ -I./src
CFLAGS += "-DHERSHEY_FONTS_DIR=\"./\""
CFLAGS += -ggdb -Wall -Wno-unknown-pragmas -O3

OBJS = src/main.o src/pocket.o src/calc.o src/hersheyfont.o src/postprocessor.o src/setup.o src/dxf.o src/font.o src/texture.o src/os-hacks.o

# GTK+2.0 and LUA5.1
PKGS ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua5.1
CFLAGS += "-DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"
CFLAGS += "-DGSEAL_ENABLE"
CFLAGS += "-DVERSION=\"${VERSION}\""

# LIBG3D
#PKGS += libg3d
#CFLAGS += "-DUSE_G3D"

# VNC-1.0
#PKGS += gtk-vnc-1.0
#CFLAGS += "-DUSE_VNC"

# WEBKIT-1.0
#PKGS += webkit-1.0 
#CFLAGS += "-DUSE_WEBKIT"

ALL_LIBS = $(LIBS) $(PKGS:%=`$(PKG_CONFIG) % --libs`)
CFLAGS += $(PKGS:%=`$(PKG_CONFIG) % --cflags`)

LANGS += de
LANGS += it
LANGS += fr

PO_MKDIR = mkdir -p $(foreach PO,$(LANGS),intl/$(PO)_$(shell echo $(PO) | tr "a-z" "A-Z").UTF-8/LC_MESSAGES)
PO_MSGFMT = $(foreach PO,$(LANGS),msgfmt po/$(PO).po -o intl/$(PO)_$(shell echo $(PO) | tr "a-z" "A-Z").UTF-8/LC_MESSAGES/${PROGRAM}.mo\;)
PO_MERGE = $(foreach PO,$(LANGS),msgmerge --no-fuzzy-matching --width=512 --backup=none --previous --update po/$(PO).po lang.pot\;)
PO_SED = $(foreach PO,$(LANGS),sed -i \'s/^.~ //g\' po/$(PO).po\;)


all: lang ${PROGRAM}

updatepo:
	xgettext -k_ src/*.c -o lang.pot
	@echo ${PO_MERGE}
	@echo ${PO_MERGE} | sh
	@echo ${PO_SED}
	@echo ${PO_SED} | sh
	rm -rf lang.pot

lang:
	@echo ${PO_MKDIR}
	@echo ${PO_MKDIR} | sh
	@echo ${PO_MSGFMT}
	@echo ${PO_MSGFMT} | sh

${PROGRAM}: ${OBJS}
		$(COMP) -o ${PROGRAM} ${OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}

%.o: %.c
		$(COMP) -c $(CFLAGS) ${INCLUDES} $< -o $@

clean:
	rm -rf ${OBJS}
	rm -rf ${PROGRAM}

install: ${PROGRAM}
	mkdir -p ${INSTALL_PATH}
	cp ${PROGRAM} ${INSTALL_PATH}/${PROGRAM}
	chmod 755 ${INSTALL_PATH}/${PROGRAM}
	mkdir -p ${INSTALL_PATH}/posts
	cp -p posts/* ${INSTALL_PATH}/posts
	mkdir -p ${INSTALL_PATH}/textures
	cp -p textures/* ${INSTALL_PATH}/textures
	mkdir -p ${INSTALL_PATH}/icons
	cp -p icons/* ${INSTALL_PATH}/icons
	mkdir -p ${INSTALL_PATH}/fonts
	cp -p fonts/* ${INSTALL_PATH}/fonts
	cp -p LICENSE.txt material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf ${INSTALL_PATH}/

ifeq (${TARGET}, MINGW32)

package: install
	(cd ${INSTALL_PATH} ; tclsh ../../../utils/create-win-installer.tclsh > installer.nsis)
	cp -p icons/icon.ico ${INSTALL_PATH}/icon.ico
	(cd ${INSTALL_PATH} ; makensis installer.nsis)
	rm -rf packages/windows/*.exe
	mv ${INSTALL_PATH}/installer.exe packages/cammill-installer.exe
	@echo "##"
	@echo "## packages/cammill-installer.exe"
	@echo "##"

test: ${PROGRAM}
	wine ${PROGRAM} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, OSX)

package:
	mkdir -p ${INSTALL_PATH}/Contents/MacOS
	cp ${PROGRAM} ${INSTALL_PATH}/Contents/MacOS/${PROGRAM}
	chmod 755 ${INSTALL_PATH}/Contents/MacOS/${PROGRAM}
	mkdir -p ${INSTALL_PATH}/Contents/MacOS/posts
	cp -p posts/* ${INSTALL_PATH}/Contents/MacOS/posts
	mkdir -p ${INSTALL_PATH}/Contents/MacOS/textures
	cp -p textures/* ${INSTALL_PATH}/Contents/MacOS/textures
	mkdir -p ${INSTALL_PATH}/Contents/MacOS/icons
	cp -p icons/* ${INSTALL_PATH}/Contents/MacOS/icons
	mkdir -p ${INSTALL_PATH}/Contents/MacOS/fonts
	cp -p fonts/* ${INSTALL_PATH}/Contents/MacOS/fonts
	cp -p LICENSE.txt material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf ${INSTALL_PATH}/Contents/MacOS/

	sh utils/osx-app.sh ${PROGRAM} ${VERSION} ${INSTALL_PATH}
	mv cammill.dmg packages/cammill.dmg
	@echo "##"
	@echo "## packages/cammill.dmg"
	@echo "##"

test: ${PROGRAM}
	./${PROGRAM} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, DEFAULT)

gprof:
	gcc -pg -o ${PROGRAM} ${OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}
	@echo "./${PROGRAM}"
	@echo "gprof ${PROGRAM} gmon.out"

depends:
	apt-get install clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev libg3d-dev

package: ${PROGRAM}
	rm -rf packages/debian
	mkdir -p packages/debian${INSTALL_PATH}
	cp -p ${PROGRAM} packages/debian${INSTALL_PATH}/${PROGRAM}
	chmod 755 packages/debian${INSTALL_PATH}/${PROGRAM}
	mkdir -p packages/debian${INSTALL_PATH}/posts
	cp -p posts/* packages/debian${INSTALL_PATH}/posts
	mkdir -p packages/debian${INSTALL_PATH}/textures
	cp -p textures/* packages/debian${INSTALL_PATH}/textures
	mkdir -p packages/debian${INSTALL_PATH}/icons
	cp -p icons/* packages/debian${INSTALL_PATH}/icons
	mkdir -p packages/debian${INSTALL_PATH}/fonts
	cp -p fonts/* packages/debian${INSTALL_PATH}/fonts
	cp -p material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf packages/debian${INSTALL_PATH}/
	mkdir -p packages/debian/usr/bin
	ln -sf ${INSTALL_PATH}/${PROGRAM} packages/debian/usr/bin/${PROGRAM}
	mkdir -p packages/debian/usr/share/man/man1/
	help2man ./${PROGRAM} | gzip -9 > packages/debian/usr/share/man/man1/${PROGRAM}.1.gz
	mkdir -p packages/debian/usr/share/doc/${PROGRAM}/
	cp -p README.md packages/debian/usr/share/doc/${PROGRAM}/README
	cp -p LICENSE.txt packages/debian/usr/share/doc/${PROGRAM}/copyright
	cp -p LICENSE.txt packages/debian/usr/share/doc/${PROGRAM}/LICENSE.txt
	git log | gzip -9 > packages/debian/usr/share/doc/${PROGRAM}/changelog.gz
	git log | gzip -9 > packages/debian/usr/share/doc/${PROGRAM}/changelog.Debian.gz 
	mkdir -p packages/debian/DEBIAN/
	(for F in material.tbl tool.tbl postprocessor.lua posts/* ; do echo "${INSTALL_PATH}/$$F" ; done) >> packages/debian/DEBIAN/conffiles
	echo "Package: ${PROGRAM}" > packages/debian/DEBIAN/control
	echo "Source: ${PROGRAM}" >> packages/debian/DEBIAN/control
	echo "Version: $(VERSION)-`date +%s`" >> packages/debian/DEBIAN/control
	echo "Architecture: `dpkg --print-architecture`" >> packages/debian/DEBIAN/control
	echo "Maintainer: ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>" >> packages/debian/DEBIAN/control
	echo "Depends: libgtksourceview2.0-0, libgtkglext1, liblua5.1-0" >> packages/debian/DEBIAN/control
	echo "Section: graphics" >> packages/debian/DEBIAN/control
	echo "Priority: optional" >> packages/debian/DEBIAN/control
	echo "Description: 2D CAM-Tool (DXF to GCODE)" >> packages/debian/DEBIAN/control
	echo " 2D CAM-Tool for Linux, Windows and Mac OS X" >> packages/debian/DEBIAN/control
	echo "Homepage: http://www.multixmedia.org/cammill/" >> packages/debian/DEBIAN/control
	chmod -R -s packages/debian/ -R
	chmod 0755 packages/debian/DEBIAN/ -R
	dpkg-deb --build packages/debian
	cp packages/debian.deb packages/${PROGRAM}.deb
	mv packages/debian.deb packages/${PROGRAM}_$(VERSION)_`dpkg --print-architecture`.deb
	@echo "##"
	@echo "## packages/${PROGRAM}_$(VERSION)_`dpkg --print-architecture`.deb"
	@echo "##"

test: ${PROGRAM}
	./${PROGRAM} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, OPENBSD)

depends:
	pkg_add git gcc gmake freeglut gtk+ gtksourceview gtkglext lua

test: ${PROGRAM}
	./${PROGRAM} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, FREEBSD)

depends:
	pkg install git gmake pkgconf gettext freeglut gtkglext gtksourceview2 lua51

package: ${PROGRAM}
	rm -rf packages/freebsd
	mkdir -p packages/freebsd

	mkdir -p packages/freebsd${INSTALL_PATH}
	cp ${PROGRAM} packages/freebsd${INSTALL_PATH}/${PROGRAM}
	chmod 755 packages/freebsd${INSTALL_PATH}/${PROGRAM}
	mkdir -p packages/freebsd${INSTALL_PATH}/posts
	cp -p posts/* packages/freebsd${INSTALL_PATH}/posts
	mkdir -p packages/freebsd${INSTALL_PATH}/textures
	cp -p textures/* packages/freebsd${INSTALL_PATH}/textures
	mkdir -p packages/freebsd${INSTALL_PATH}/icons
	cp -p icons/* packages/freebsd${INSTALL_PATH}/icons
	mkdir -p packages/freebsd${INSTALL_PATH}/fonts
	cp -p fonts/* packages/freebsd${INSTALL_PATH}/fonts
	cp -p LICENSE.txt material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf packages/freebsd${INSTALL_PATH}/
	mkdir -p packages/freebsd/usr/local/bin/
	ln -sf ${INSTALL_PATH}/${PROGRAM} packages/freebsd/usr/local/bin/${PROGRAM}

	echo "name: ${PROGRAM}" > packages/freebsd/+MANIFEST
	echo "version: ${VERSION}_0" >> packages/freebsd/+MANIFEST
	echo "origin: graphics" >> packages/freebsd/+MANIFEST
	echo "comment: 2D CAM-Tool (DXF to GCODE)" >> packages/freebsd/+MANIFEST
	echo "arch: i386" >> packages/freebsd/+MANIFEST
	echo "www: http://www.multixmedia.org/cammill/" >> packages/freebsd/+MANIFEST
	echo "maintainer: ${MAINTAINER_EMAIL}" >> packages/freebsd/+MANIFEST
	echo "prefix: /opt" >> packages/freebsd/+MANIFEST
	echo "licenselogic: or" >> packages/freebsd/+MANIFEST
	echo "licenses: [GPL3]" >> packages/freebsd/+MANIFEST
	echo "flatsize: `du -sck packages/freebsd/ | tail -n1 | awk '{print $$1}'`" >> packages/freebsd/+MANIFEST
	#echo "users: [USER1, USER2]" >> packages/freebsd/+MANIFEST
	#echo "groups: [GROUP1, GROUP2]" >> packages/freebsd/+MANIFEST
	#echo "options: { OPT1: off, OPT2: on }" >> packages/freebsd/+MANIFEST
	echo "desc: |-" >> packages/freebsd/+MANIFEST
	echo "  2D CAM-Tool for Linux, Windows and Mac OS X" >> packages/freebsd/+MANIFEST
	echo "categories: [graphics]" >> packages/freebsd/+MANIFEST
	#echo "deps:" >> packages/freebsd/+MANIFEST
	#echo "  libiconv: {origin: converters/libiconv, version: 1.13.1_2}" >> packages/freebsd/+MANIFEST
	#echo "  perl: {origin: lang/perl5.12, version: 5.12.4 }" >> packages/freebsd/+MANIFEST
	#freeglut gtkglext gtksourceview2 lua51
	echo "files: {" >> packages/freebsd/+MANIFEST
	(for F in `find packages/freebsd${INSTALL_PATH} packages/freebsd/usr/local/bin/${PROGRAM} -type f` ; do echo "  `echo $$F | sed "s|^packages/freebsd||g"`: \"`sha256 $$F | cut -d" " -f4`\"" ; done) >> packages/freebsd/+MANIFEST
	echo "}" >> packages/freebsd/+MANIFEST
	echo "scripts: {" >> packages/freebsd/+MANIFEST
	echo "  pre-install:  {" >> packages/freebsd/+MANIFEST
	echo "    #!/bin/sh" >> packages/freebsd/+MANIFEST
	echo "    echo pre-install" >> packages/freebsd/+MANIFEST
	echo "  }" >> packages/freebsd/+MANIFEST
	echo "  post-install:  {" >> packages/freebsd/+MANIFEST
	echo "    #!/bin/sh" >> packages/freebsd/+MANIFEST
	echo "    echo post-install" >> packages/freebsd/+MANIFEST
	echo "  }" >> packages/freebsd/+MANIFEST
	echo "}" >> packages/freebsd/+MANIFEST
	tar -s "|.${INSTALL_PATH}|${INSTALL_PATH}|" -s "|./usr/local|/usr/local|" -C packages/freebsd/ -czvpPf packages/cammill-freebsd-${VERSION}.tgz +MANIFEST .${INSTALL_PATH} ./usr/local/bin/${PROGRAM}
	@echo "##"
	@echo "## packages/cammill-freebsd-${VERSION}.tgz"
	@echo "##"

test: ${PROGRAM}
	./${PROGRAM} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif

doc: ${PROGRAM}
	test -e cammill.github.io && sh utils/help2html.sh ./${PROGRAM} > cammill.github.io/de/cmdline.html || true
	test -e cammill.wiki/ && sh utils/help2md.sh ./${PROGRAM} > cammill.wiki/Commandline-Arguments.md || true

pull:
	git pull --rebase
	test -e cammill.github.io && (cd cammill.github.io/ ; git pull --rebase) || true
	test -e cammill.wiki/ && (cd cammill.wiki/ ; git pull --rebase) || true




