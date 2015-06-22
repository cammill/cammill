
#TARGETS: DEFAULT, MINGW32, OSX, OPENBSD
TARGET ?= DEFAULT

ifeq (${TARGET}, MINGW32)
	PROGRAM         ?= cammill.exe
	LIBS            ?= -lm -lstdc++ -lgcc
	CROSS           ?= i686-w64-mingw32.static-
	COMP            ?= ${CROSS}gcc
	PKGS            ?= gtk+-2.0 gtk+-win32-2.0 gtkglext-1.0 gtksourceview-2.0 lua
	INSTALL_PATH    ?= Windows/CamMill
endif

ifeq (${TARGET}, OSX)
	LIBS            ?= -framework OpenGL -framework GLUT -lm -lpthread -lstdc++ -lc
	PKGS            ?= gtk+-2.0 gtkglext-1.0 gtksourceview-2.0 lua
    PKG_CONFIG_PATH ?= /opt/X11/lib/pkgconfig
	INSTALL_PATH    ?= OSX/CamMill
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

LIBS   ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lstdc++ -lXext -lXi -lxcb -lXau -lXdmcp -lgcc -lc
CFLAGS += -I./
CFLAGS += "-DHERSHEY_FONTS_DIR=\"./\""
CFLAGS += -ggdb -Wall -Wno-unknown-pragmas -O3

OBJS = main.o pocket.o calc.o hersheyfont.o postprocessor.o setup.o dxf.o font.o texture.o os-hacks.o

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


all: lang ${PROGRAM}

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
	mkdir -p ${INSTALL_PATH}/doc
	cp -p doc/* ${INSTALL_PATH}/doc
	cp -p GPLv3.txt material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf ${INSTALL_PATH}/

ifeq (${TARGET}, MINGW32)

win_installer: install
	(cd ${INSTALL_PATH} ; tclsh ../../utils/create-win-installer.tclsh > installer.nsis)
	cp -p icons/icon.ico ${INSTALL_PATH}/icon.ico
	(cd ${INSTALL_PATH} ; makensis installer.nsis)
	mv ${INSTALL_PATH}/installer.exe Windows/

endif
ifeq (${TARGET}, OSX)

osx_app: install
	sh utils/osx-app.sh ${PROGRAM} ${VERSION} ${INSTALL_PATH}

endif
ifeq (${TARGET}, DEFAULT)

gprof:
	gcc -pg -o ${PROGRAM} ${OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}
	@echo "./${PROGRAM}"
	@echo "gprof ${PROGRAM} gmon.out"

deb: ${PROGRAM}
	rm -rf debian-package
	mkdir -p debian-package${INSTALL_PATH}
	cp -p ${PROGRAM} debian-package${INSTALL_PATH}/${PROGRAM}
	chmod 755 debian-package${INSTALL_PATH}/${PROGRAM}
	mkdir -p debian-package${INSTALL_PATH}/posts
	cp -p posts/* debian-package${INSTALL_PATH}/posts
	mkdir -p debian-package${INSTALL_PATH}/textures
	cp -p textures/* debian-package${INSTALL_PATH}/textures
	mkdir -p debian-package${INSTALL_PATH}/icons
	cp -p icons/* debian-package${INSTALL_PATH}/icons
	mkdir -p debian-package${INSTALL_PATH}/fonts
	cp -p fonts/* debian-package${INSTALL_PATH}/fonts
	cp -p material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf debian-package${INSTALL_PATH}/
	mkdir -p debian-package/usr/bin
	ln -sf ${INSTALL_PATH}/${PROGRAM} debian-package/usr/bin/${PROGRAM}
	mkdir -p debian-package/usr/share/man/man1/
	cat utils/man.1 | gzip -9 > debian-package/usr/share/man/man1/${PROGRAM}.1.gz
	mkdir -p debian-package/usr/share/doc/${PROGRAM}/
	cp -p README.md debian-package/usr/share/doc/${PROGRAM}/README
	cp -p doc debian-package/usr/share/doc/${PROGRAM}/doc
	cp -p GPLv3.txt debian-package/usr/share/doc/${PROGRAM}/copyright
	cp -p GPLv3.txt debian-package/usr/share/doc/${PROGRAM}/GPLv3.txt
	git log | gzip -9 > debian-package/usr/share/doc/${PROGRAM}/changelog.gz
	git log | gzip -9 > debian-package/usr/share/doc/${PROGRAM}/changelog.Debian.gz 
	mkdir -p debian-package/DEBIAN/
	(for F in material.tbl tool.tbl postprocessor.lua posts/* ; do echo "${INSTALL_PATH}/$$F" ; done) >> debian-package/DEBIAN/conffiles
	echo "Package: ${PROGRAM}" > debian-package/DEBIAN/control
	echo "Source: ${PROGRAM}" >> debian-package/DEBIAN/control
	echo "Version: $(VERSION)-`date +%s`" >> debian-package/DEBIAN/control
	echo "Architecture: `dpkg --print-architecture`" >> debian-package/DEBIAN/control
	echo "Maintainer: Oliver Dippel <oliver@multixmedia.org>" >> debian-package/DEBIAN/control
	echo "Depends: libgtksourceview2.0-0, libgtkglext1, liblua5.1-0" >> debian-package/DEBIAN/control
	echo "Section: graphics" >> debian-package/DEBIAN/control
	echo "Priority: optional" >> debian-package/DEBIAN/control
	echo "Description: 2D CAM-Tool (DXF to GCODE)" >> debian-package/DEBIAN/control
	echo " 2D CAM-Tool for Linux, Windows and Mac OS X" >> debian-package/DEBIAN/control
	echo "Homepage: http://www.multixmedia.org/cammill/" >> debian-package/DEBIAN/control
	chmod -R -s debian-package/ -R
	chmod 0755 debian-package/DEBIAN/ -R
	dpkg-deb --build debian-package
	mv debian-package.deb ${PROGRAM}_$(VERSION)-`date +%s`_`dpkg --print-architecture`.deb

endif
