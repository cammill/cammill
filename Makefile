
#TARGETS: DEBIAN, FREEBSD, MINGW32, NETBSD, OSX, OPENBSD, SUSE
TARGET ?= NONE

#autodetect system
ifeq (${TARGET}, NONE)
	ifeq ($(OS),Windows_NT)
		ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			MACHINE = AMD64
		endif
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			MACHINE = IA32
		endif
		TARGET = MINGW32
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),Linux)
			ifneq ("$(wildcard /etc/SuSE-release)","")
				TARGET = SUSE
			endif
			ifneq ("$(wildcard /etc/redhat-release)","")
				TARGET = REDHAT
			endif
			ifneq ("$(wildcard /etc/fedora-release)","")
				TARGET = FEDORA
			endif
			ifneq ("$(wildcard /etc/centos-release)","")
				TARGET = CENTOS
			endif
			ifneq ("$(wildcard /etc/debian_version)","")
				TARGET = DEBIAN
			endif
			ifneq ("$(wildcard /etc/arch-release)","")
				TARGET = ARCHLINUX
			endif
		endif
		ifeq ($(SYSTEM),FreeBSD)
			TARGET = FREEBSD
		endif
		ifeq ($(SYSTEM),OpenBSD)
			TARGET = OPENBSD
		endif
		ifeq ($(SYSTEM),NetBSD)
			TARGET = NETBSD
		endif
		ifeq ($(SYSTEM),Darwin)
			TARGET = OSX
		endif
	endif
endif

ifeq (${TARGET}, MINGW32)
	SYSTEM          ?= Windows
	RELEASE         ?= MinGW
	DISTRIBUTION    ?= Windows
	MACHINE         ?= IA32
	BINARY          ?= cammill.exe
	LIBS            ?= -lm -lstdc++ -lgcc
	CROSS           ?= /usr/lib/mxe/usr/bin/i686-w64-mingw32.static-
	COMP            ?= ${CROSS}gcc
	PKGS            ?= gtk+-2.0 gtk+-win32-2.0 gtkglext-1.0 gtksourceview-2.0 lua
	INSTALL_PATH    ?= build/windows/CAMmill
endif
ifeq (${TARGET}, OSX)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell uname -m)
	RELEASE         ?= $(shell lsb_release -s -r)
	DISTRIBUTION    ?= $(shell lsb_release -s -i | tr " " "_")
	CFLAGS          += "-Wno-deprecated"
	LIBS            ?= -framework OpenGL -framework GLUT -lm -lpthread -lstdc++ -lc
	PKGS            ?= gtk+-2.0 gtkglext-1.0 gtksourceview-2.0 lua
    PKG_CONFIG_PATH ?= /opt/X11/lib/pkgconfig
	INSTALL_PATH    ?= build/osx/CAMmill
endif
ifeq (${TARGET}, DEBIAN)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell dpkg --print-architecture)
	RELEASE         ?= $(shell lsb_release -s -r)
	DISTRIBUTION    ?= $(shell lsb_release -s -i | tr " " "_")
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua5.1
	LIBS            ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lstdc++ -lXext -lXi -lxcb -lXau -lXdmcp -lgcc -lc
endif
ifeq (${TARGET}, SUSE)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell uname -m)
	RELEASE         ?= $(shell lsb_release -s -r)
	DISTRIBUTION    ?= $(shell lsb_release -s -i | tr " " "_")
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua
	LIBS            ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lstdc++ -lXext -lxcb -lXau -lgcc -lc
endif
ifeq (${TARGET}, FEDORA)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell uname -m)
	RELEASE         ?= $(shell lsb_release -s -r)
	DISTRIBUTION    ?= $(shell lsb_release -s -i | tr " " "_")
	COMP            ?= gcc
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua
	LIBS            ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lXext -lxcb -lXau -lgcc -lc
endif
ifeq (${TARGET}, CENTOS)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell uname -m)
	RELEASE         ?= $(shell lsb_release -s -r)
	DISTRIBUTION    ?= $(shell lsb_release -s -i | tr " " "_")
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua
	LIBS            ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lXext -lxcb -lXau -lgcc -lc
endif
ifeq (${TARGET}, FREEBSD)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell uname -m)
	RELEASE         ?= $(shell uname -r | cut -d"-" -f1)
	DISTRIBUTION    ?= ${SYSTEM}
	COMP            ?= clang
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua-5.1
endif
ifeq (${TARGET}, NETBSD)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell uname -m)
	RELEASE         ?= $(shell uname -r | cut -d"-" -f1)
	DISTRIBUTION    ?= ${SYSTEM}
	COMP            ?= gcc
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua-5.1
endif
ifeq (${TARGET}, OPENBSD)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell uname -m)
	RELEASE         ?= $(shell uname -r | cut -d"-" -f1)
	DISTRIBUTION    ?= ${SYSTEM}
	COMP            ?= gcc
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua51
	INSTALL_PATH    ?= /usr/local/cammill
endif
ifeq (${TARGET}, ARCHLINUX)
	SYSTEM          ?= $(shell uname -s)
	MACHINE         ?= $(shell uname -m)
	RELEASE         ?= $(shell uname -r | cut -d"-" -f1)
	DISTRIBUTION    ?= ${SYSTEM}
	COMP            ?= clang
	PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua5.1
	INSTALL_PATH    ?= /usr/lib/cammill
endif


PROGRAM    ?= cammill
PROGNAME   ?= CAMmill
COMMENT    ?= 2D CAM-Tool (DXF to GCODE)
VERSION    ?= 0.9
HOMEPAGE   ?= http://www.multixmedia.org/${PROGRAM}/
MAINTAINER_NAME  ?= Oliver Dippel
MAINTAINER_EMAIL ?= oliver@multixmedia.org

BINARY     ?= ${PROGRAM}
COMP       ?= $(CROSS)clang
PKG_CONFIG ?= $(CROSS)pkg-config


HERSHEY_FONTS_DIR = ./
INSTALL_PATH ?= /usr/lib/${BINARY}

LIBS   ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lstdc++ -lXext -lXi -lxcb -lXau -lXdmcp -lgcc -lc
CFLAGS += -I./ -I./src
CFLAGS += "-DHERSHEY_FONTS_DIR=\"./\""
CFLAGS += -ggdb -Wall -Wno-unknown-pragmas -O3

# debian hardening flags
ifeq (${TARGET}, DEBIAN)
LIBS += -Wl,-z,relro,-z,now
CFLAGS += -D_FORTIFY_SOURCE=2
CFLAGS += -Wformat -Werror=format-security
#CFLAGS += -fstack-protector-strong
endif

OBJS = src/main.o src/pocket.o src/calc.o src/hersheyfont.o src/postprocessor.o src/setup.o src/dxf.o src/font.o src/texture.o src/os-hacks.o

# GTK+2.0 and LUA5.1
PKGS   ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua5.1
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
PO_MSGFMT = $(foreach PO,$(LANGS),msgfmt po/$(PO).po -o intl/$(PO)_$(shell echo $(PO) | tr "a-z" "A-Z").UTF-8/LC_MESSAGES/${BINARY}.mo\;)
PO_MERGE = $(foreach PO,$(LANGS),msgmerge --no-fuzzy-matching --width=512 --backup=none --previous --update po/$(PO).po lang.pot\;)
PO_SED = $(foreach PO,$(LANGS),sed -i \'s/^.~ //g\' po/$(PO).po\;)

ifeq (${TARGET}, DEBIAN)

all: info lang ${BINARY}

else

all: info ${BINARY}

endif

info:
	@echo ""
	@echo "Info:"
	@echo "	TARGET          ${TARGET}"
	@echo "	SYSTEM          ${SYSTEM}"
	@echo "	MACHINE         ${MACHINE}"
	@echo "	RELEASE         ${RELEASE}"
	@echo "	DISTRIBUTION    ${DISTRIBUTION}"
	@echo "	BINARY          ${BINARY}"
	@echo "	VERSION         ${VERSION}"
	@echo "	PKGS:           ${PKGS}"
	@echo "	INSTALL_PATH    ${INSTALL_PATH}"
	@echo ""

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

${BINARY}: ${OBJS}
		$(COMP) -o ${BINARY} ${OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}

%.o: %.c
		$(COMP) -c $(CFLAGS) ${INCLUDES} $< -o $@

clean:
	rm -rf ${OBJS}
	rm -rf ${BINARY}

install: ${BINARY}
	mkdir -p ${INSTALL_PATH}
	cp ${BINARY} ${INSTALL_PATH}/${BINARY}
	chmod 755 ${INSTALL_PATH}/${BINARY}
	mkdir -p ${INSTALL_PATH}/posts
	cp -p posts/* ${INSTALL_PATH}/posts
	mkdir -p ${INSTALL_PATH}/textures
	cp -p textures/* ${INSTALL_PATH}/textures
	mkdir -p ${INSTALL_PATH}/icons
	cp -p icons/* ${INSTALL_PATH}/icons
	mkdir -p ${INSTALL_PATH}/fonts
	cp -p fonts/* ${INSTALL_PATH}/fonts
	cp -p LICENSE.txt material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf ${INSTALL_PATH}/
	chown -R root:root ${INSTALL_PATH}/


ifeq (${TARGET}, MINGW32)

package: install
	(cd ${INSTALL_PATH} ; tclsh ../../../utils/create-win-installer.tclsh > installer.nsis)
	cp -p icons/icon.ico ${INSTALL_PATH}/icon.ico
	(cd ${INSTALL_PATH} ; makensis installer.nsis)
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${INSTALL_PATH}/installer.exe packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.exe
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.exe"
	@echo "##"

test: ${BINARY}
	wine ${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, OSX)

package:
	strip ${BINARY}
	mkdir -p ${INSTALL_PATH}/Contents/${TARGET}
	cp ${BINARY} ${INSTALL_PATH}/Contents/${TARGET}/${BINARY}
	chmod 755 ${INSTALL_PATH}/Contents/${TARGET}/${BINARY}
	mkdir -p ${INSTALL_PATH}/Contents/${TARGET}/posts
	cp -p posts/* ${INSTALL_PATH}/Contents/${TARGET}/posts
	mkdir -p ${INSTALL_PATH}/Contents/${TARGET}/textures
	cp -p textures/* ${INSTALL_PATH}/Contents/${TARGET}/textures
	mkdir -p ${INSTALL_PATH}/Contents/${TARGET}/icons
	cp -p icons/* ${INSTALL_PATH}/Contents/${TARGET}/icons
	mkdir -p ${INSTALL_PATH}/Contents/${TARGET}/fonts
	cp -p fonts/* ${INSTALL_PATH}/Contents/${TARGET}/fonts
	cp -p LICENSE.txt material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf ${INSTALL_PATH}/Contents/${TARGET}/

	sh utils/osx-app.sh ${BINARY} ${VERSION} ${INSTALL_PATH}
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${PROGRAM}.dmg packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.dmg
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.dmg"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, SUSE)

depends:
	zypper install libgtk-2_0-0 libgtkglext-x11-1_0-0 libgtksourceview-2_0-0 git freeglut-devel lua51-devel make clang gtk2-devel gtkglext-devel gtksourceview2-devel gcc rpm-build

package: ${BINARY}
	strip --remove-section=.comment --remove-section=.note ${BINARY}
	rm -rf build/${DISTRIBUTION}/${PROGRAM}-${VERSION}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}
	cp -p ${BINARY} build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/${BINARY}
	chmod 755 build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/${BINARY}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/posts
	cp -p posts/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/posts
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/textures
	cp -p textures/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/textures
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/icons
	cp -p icons/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/icons
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/fonts
	cp -p fonts/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/fonts
	cp -p material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/bin
	ln -sf ../lib/${BINARY}/${BINARY} build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/bin/${BINARY}

	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications
	echo "[Desktop Entry]" > build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Version=${VERSION}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Type=Application" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Name=${PROGNAME}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Comment=${COMMENT}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "TryExec=${BINARY}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Exec=${BINARY} %F" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Icon=${BINARY}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Keywords=cam;cnc;gcode;dxf;" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Terminal=false" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/pixmaps
	cp -p icons/icon_128.png build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/pixmaps/${BINARY}.png

	echo "Summary: ${COMMENT}" > build/${DISTRIBUTION}/${BINARY}.spec
	echo "Name: ${BINARY}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Version: ${VERSION}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Release: 1" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "License: GPL" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Group: Utilities/System" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "BuildRoot: %{_tmppath}/%{name}-root" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Requires: bash" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Source0: ${BINARY}-%{version}.tar.gz" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%description" >> build/${DISTRIBUTION}/${BINARY}.spec
	cat desc.txt | grep ".." | sed "s|^| |g" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%prep" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%setup" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%build" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%install" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "rm -rf \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "mkdir -p \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "cp -a * \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%clean" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "rm -rf \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%files" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "/usr/bin/${BINARY}" >> build/${DISTRIBUTION}/${BINARY}.spec
	(for F in `find build/${DISTRIBUTION}/${PROGRAM}-${VERSION} -type f`; do echo "$$F" | sed "s|build/${DISTRIBUTION}/${PROGRAM}-${VERSION}||g"; done) >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	cp -a build/${DISTRIBUTION}/${BINARY}.spec /usr/src/packages/SPECS/${BINARY}.spec
	(cd build/${DISTRIBUTION} ; tar czpf /usr/src/packages/SOURCES/${PROGRAM}-${VERSION}.tar.gz ${PROGRAM}-${VERSION})
	rpmbuild --bb /usr/src/packages/SPECS/${BINARY}.spec
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv /usr/src/packages/RPMS/${MACHINE}/${PROGRAM}-${VERSION}-1.${MACHINE}.rpm packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-1_${MACHINE}.rpm
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-1_${MACHINE}.rpm"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, FEDORA)

depends:
	yum install gtkglext-devel gtksourceview2-devel lua-devel freeglut-devel make clang gcc gtk+-devel rpm-build git

package: ${BINARY}
	strip --remove-section=.comment --remove-section=.note ${BINARY}
	rm -rf build/${DISTRIBUTION}/${PROGRAM}-${VERSION}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}
	cp -p ${BINARY} build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/${BINARY}
	chmod 755 build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/${BINARY}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/posts
	cp -p posts/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/posts
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/textures
	cp -p textures/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/textures
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/icons
	cp -p icons/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/icons
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/fonts
	cp -p fonts/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/fonts
	cp -p material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/bin
	ln -sf ../lib/${BINARY}/${BINARY} build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/bin/${BINARY}

	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications
	echo "[Desktop Entry]" > build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Version=${VERSION}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Type=Application" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Name=${PROGNAME}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Comment=${COMMENT}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "TryExec=${BINARY}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Exec=${BINARY} %F" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Icon=${BINARY}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Keywords=cam;cnc;gcode;dxf;" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Terminal=false" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/pixmaps
	cp -p icons/icon_128.png build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/pixmaps/${BINARY}.png

	echo "Summary: ${COMMENT}" > build/${DISTRIBUTION}/${BINARY}.spec
	echo "Name: ${BINARY}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Version: ${VERSION}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Release: 1" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "License: GPL" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Group: Utilities/System" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "BuildRoot: %{_tmppath}/%{name}-root" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Requires: bash" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Source0: ${BINARY}-%{version}.tar.gz" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%description" >> build/${DISTRIBUTION}/${BINARY}.spec
	cat desc.txt | grep ".." | sed "s|^| |g" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%prep" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%setup" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%build" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%install" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "rm -rf \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "mkdir -p \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "cp -a * \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%clean" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "rm -rf \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%files" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "/usr/bin/${BINARY}" >> build/${DISTRIBUTION}/${BINARY}.spec
	(for F in `find build/${DISTRIBUTION}/${PROGRAM}-${VERSION} -type f`; do echo "$$F" | sed "s|build/${DISTRIBUTION}/${PROGRAM}-${VERSION}||g"; done) >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	mkdir -p ~/rpmbuild/SOURCES
	mkdir -p ~/rpmbuild/SPECS
	cp -a build/${DISTRIBUTION}/${BINARY}.spec ~/rpmbuild/SPECS/${BINARY}.spec
	(cd build/${DISTRIBUTION} ; tar czpf ~/rpmbuild/SOURCES/${PROGRAM}-${VERSION}.tar.gz ${PROGRAM}-${VERSION})
	rpmbuild --bb ~/rpmbuild/SPECS/${BINARY}.spec
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ~/rpmbuild/RPMS/${MACHINE}/${PROGRAM}-${VERSION}-1.${MACHINE}.rpm packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-1_${MACHINE}.rpm
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-1_${MACHINE}.rpm"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, CENTOS)

depends:
	yum install gtkglext-devel lua-devel freeglut-devel make gcc gtk2-devel rpm-build git
	rpm --import http://winswitch.org/gpg.asc
	cd /etc/yum.repos.d/
	curl -O https://winswitch.org/downloads/CentOS/winswitch.repo
	yum install gtkglext-devel

package: ${BINARY}
	strip --remove-section=.comment --remove-section=.note ${BINARY}
	rm -rf build/${DISTRIBUTION}/${PROGRAM}-${VERSION}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}
	cp -p ${BINARY} build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/${BINARY}
	chmod 755 build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/${BINARY}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/posts
	cp -p posts/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/posts
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/textures
	cp -p textures/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/textures
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/icons
	cp -p icons/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/icons
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/fonts
	cp -p fonts/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/fonts
	cp -p material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/bin
	ln -sf ../lib/${BINARY}/${BINARY} build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/bin/${BINARY}

	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications
	echo "[Desktop Entry]" > build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Version=${VERSION}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Type=Application" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Name=${PROGNAME}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Comment=${COMMENT}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "TryExec=${BINARY}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Exec=${BINARY} %F" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Icon=${BINARY}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Keywords=cam;cnc;gcode;dxf;" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Terminal=false" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/pixmaps
	cp -p icons/icon_128.png build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/pixmaps/${BINARY}.png

	echo "Summary: ${COMMENT}" > build/${DISTRIBUTION}/${BINARY}.spec
	echo "Name: ${BINARY}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Version: ${VERSION}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Release: 1" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "License: GPL" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Group: Utilities/System" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "BuildRoot: %{_tmppath}/%{name}-root" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Requires: bash" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "Source0: ${BINARY}-%{version}.tar.gz" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%description" >> build/${DISTRIBUTION}/${BINARY}.spec
	cat desc.txt | grep ".." | sed "s|^| |g" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%prep" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%setup" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%build" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%install" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "rm -rf \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "mkdir -p \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "cp -a * \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%clean" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "rm -rf \$${RPM_BUILD_ROOT}" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "%files" >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "/usr/bin/${BINARY}" >> build/${DISTRIBUTION}/${BINARY}.spec
	(for F in `find build/${DISTRIBUTION}/${PROGRAM}-${VERSION} -type f`; do echo "$$F" | sed "s|build/${DISTRIBUTION}/${PROGRAM}-${VERSION}||g"; done) >> build/${DISTRIBUTION}/${BINARY}.spec
	echo "" >> build/${DISTRIBUTION}/${BINARY}.spec
	mkdir -p ~/rpmbuild/SOURCES
	mkdir -p ~/rpmbuild/SPECS
	cp -a build/${DISTRIBUTION}/${BINARY}.spec ~/rpmbuild/SPECS/${BINARY}.spec
	(cd build/${DISTRIBUTION} ; tar czpf ~/rpmbuild/SOURCES/${PROGRAM}-${VERSION}.tar.gz ${PROGRAM}-${VERSION})
	rpmbuild --bb ~/rpmbuild/SPECS/${BINARY}.spec
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ~/rpmbuild/RPMS/${MACHINE}/${PROGRAM}-${VERSION}-1.${MACHINE}.rpm packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-1_${MACHINE}.rpm
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-1_${MACHINE}.rpm"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, DEBIAN)

gprof:
	gcc -pg -o ${BINARY} ${OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}
	@echo "./${BINARY}"
	@echo "gprof ${BINARY} gmon.out"

depends:
	apt-get install clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev libg3d-dev lintian

package: ${BINARY}
	strip --remove-section=.comment --remove-section=.note ${BINARY}
	rm -rf build/${DISTRIBUTION}
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}
	cp -p ${BINARY} build/${DISTRIBUTION}${INSTALL_PATH}/${BINARY}
	chmod 755 build/${DISTRIBUTION}${INSTALL_PATH}/${BINARY}
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}/posts
	cp -p posts/* build/${DISTRIBUTION}${INSTALL_PATH}/posts
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}/textures
	cp -p textures/* build/${DISTRIBUTION}${INSTALL_PATH}/textures
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}/icons
	cp -p icons/* build/${DISTRIBUTION}${INSTALL_PATH}/icons
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}/fonts
	cp -p fonts/* build/${DISTRIBUTION}${INSTALL_PATH}/fonts
	cp -p material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf build/${DISTRIBUTION}${INSTALL_PATH}/
	mkdir -p build/${DISTRIBUTION}/usr/bin

	ln -sf ../lib/${BINARY}/${BINARY} build/${DISTRIBUTION}/usr/bin/${BINARY}

	mkdir -p build/${DISTRIBUTION}/usr/share/man/man1/
	help2man ./${BINARY} -N -n "${COMMENT}" | gzip -n -9 > build/${DISTRIBUTION}/usr/share/man/man1/${BINARY}.1.gz
	mkdir -p build/${DISTRIBUTION}/usr/share/doc/${BINARY}/
	cp -p README.md build/${DISTRIBUTION}/usr/share/doc/${BINARY}/README

	echo "It was downloaded from https://github.com/${PROGRAM}" > build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "Copyright 2014 - 2015 by Oliver Dippel <oliver@multixmedia.org>" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "Copyright 2014 - 2015 by McUles mcules@fpv-club.de" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "Copyright 2014 - 2015 by Jakob Flierl <@koppi>" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "Copyright 2014 - 2015 by Carlo <onekk>" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "License:" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "This program is free software; you can redistribute it and/or modify it" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "under the terms of the GNU General Public License as published by the" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "Free Software Foundation; either version 2, or (at your option) any" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "later version." >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "On Debian systems, the complete text of the GNU General Public License" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "can be found in the file /usr/share/common-licenses/GPL-3" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright
	echo "" >> build/${DISTRIBUTION}/usr/share/doc/${BINARY}/copyright

	git log | gzip -n -9 > build/${DISTRIBUTION}/usr/share/doc/${BINARY}/changelog.gz
	echo "${BINARY} (${VERSION}) unstable; urgency=low\n\n  * Git Release.\n  * take a look in to changelog.gz\n\n -- ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>  `date -R`\n" | gzip -n -9 > build/${DISTRIBUTION}/usr/share/doc/${BINARY}/changelog.Debian.gz

	mkdir -p build/${DISTRIBUTION}/usr/share/applications
	echo "[Desktop Entry]" > build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Version=${VERSION}" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Type=Application" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Name=${PROGNAME}" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Comment=${COMMENT}" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "TryExec=${BINARY}" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Exec=${BINARY} %F" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Icon=${BINARY}" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Keywords=cam;cnc;gcode;dxf;" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "Terminal=false" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	echo "" >> build/${DISTRIBUTION}/usr/share/applications/${BINARY}.desktop
	mkdir -p build/${DISTRIBUTION}/usr/share/pixmaps
	cp -p icons/icon_128.png build/${DISTRIBUTION}/usr/share/pixmaps/${BINARY}.png

	mkdir -p build/${DISTRIBUTION}/DEBIAN/
	(for F in `find build/${DISTRIBUTION} -type f | grep -v "^build/${DISTRIBUTION}/DEBIAN/"`; do md5sum "$$F" | sed "s| build/${DISTRIBUTION}/| |g"; done) >> build/${DISTRIBUTION}/DEBIAN/md5sums
	#(for F in material.tbl tool.tbl postprocessor.lua posts/* ; do echo "${INSTALL_PATH}/$$F" ; done) >> build/${DISTRIBUTION}/DEBIAN/conffiles
	echo "Package: ${BINARY}" > build/${DISTRIBUTION}/DEBIAN/control
	echo "Source: ${BINARY}" >> build/${DISTRIBUTION}/DEBIAN/control
	echo "Version: ${VERSION}" >> build/${DISTRIBUTION}/DEBIAN/control
	echo "Architecture: `dpkg --print-architecture`" >> build/${DISTRIBUTION}/DEBIAN/control
	echo "Maintainer: ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>" >> build/${DISTRIBUTION}/DEBIAN/control
	echo "Depends: libc6, libgtksourceview2.0-0, libgtkglext1, liblua5.1-0" >> build/${DISTRIBUTION}/DEBIAN/control
	echo "Section: graphics" >> build/${DISTRIBUTION}/DEBIAN/control
	echo "Priority: optional" >> build/${DISTRIBUTION}/DEBIAN/control
	echo "Homepage: ${HOMEPAGE}" >> build/${DISTRIBUTION}/DEBIAN/control
	echo "Description: ${COMMENT}" >> build/${DISTRIBUTION}/DEBIAN/control
	cat desc.txt | grep ".." | sed "s|^| |g" >> build/${DISTRIBUTION}/DEBIAN/control
	# bad hack, please use install command
	chown -R root:root build/${DISTRIBUTION}/
	chmod 0755 build/${DISTRIBUTION}/
	chmod 0644 build/${DISTRIBUTION}/usr/share/doc/${PROGRAM}/*
	chmod 0644 build/${DISTRIBUTION}/usr/share/man/man1/${PROGRAM}.1.gz
	chmod 0644 build/${DISTRIBUTION}/usr/share/pixmaps/${PROGRAM}.png
	chmod 0644 build/${DISTRIBUTION}/usr/share/applications/${PROGRAM}.desktop
	chmod 0644 build/${DISTRIBUTION}/usr/share/doc/${PROGRAM}/*
	chmod 0644 build/${DISTRIBUTION}${INSTALL_PATH}/posts/*
	chmod 0644 build/${DISTRIBUTION}${INSTALL_PATH}/textures/*
	chmod 0644 build/${DISTRIBUTION}${INSTALL_PATH}/icons/*
	chmod 0644 build/${DISTRIBUTION}${INSTALL_PATH}/fonts/*
	chmod 0644 build/${DISTRIBUTION}${INSTALL_PATH}/material.tbl
	chmod 0644 build/${DISTRIBUTION}${INSTALL_PATH}/tool.tbl
	chmod 0644 build/${DISTRIBUTION}${INSTALL_PATH}/postprocessor.lua
	chmod 0644 build/${DISTRIBUTION}${INSTALL_PATH}/*.dxf

	chmod -R -s build/${DISTRIBUTION}/
	chown -R root:root build/${DISTRIBUTION}/DEBIAN/
	chmod -R 0755 build/${DISTRIBUTION}/DEBIAN/
	chmod 0644 build/${DISTRIBUTION}/DEBIAN/control
	chmod 0644 build/${DISTRIBUTION}/DEBIAN/md5sums
	#chmod 0644 build/${DISTRIBUTION}/DEBIAN/conffiles

	dpkg-deb --build build/${DISTRIBUTION}
	lintian -I --show-overrides build/${DISTRIBUTION}.deb || true
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv build/${DISTRIBUTION}.deb packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.deb
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.deb"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, OPENBSD)

depends:
	pkg_add git gcc gmake freeglut gtk+ gtksourceview gtkglext lua

package: ${BINARY}
	strip ${BINARY}
	rm -rf build/${DISTRIBUTION}
	mkdir -p build/${DISTRIBUTION}

	mkdir -p build/${DISTRIBUTION}/${PROGRAM}
	cp ${BINARY} build/${DISTRIBUTION}/${PROGRAM}/${BINARY}
	chmod 755 build/${DISTRIBUTION}/${PROGRAM}/${BINARY}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}/posts
	cp -p posts/* build/${DISTRIBUTION}/${PROGRAM}/posts
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}/textures
	cp -p textures/* build/${DISTRIBUTION}/${PROGRAM}/textures
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}/icons
	cp -p icons/* build/${DISTRIBUTION}/${PROGRAM}/icons
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}/fonts
	cp -p fonts/* build/${DISTRIBUTION}/${PROGRAM}/fonts
	cp -p LICENSE.txt material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf build/${DISTRIBUTION}/${PROGRAM}/
	mkdir -p build/${DISTRIBUTION}/bin/
	ln -sf ${INSTALL_PATH}/${BINARY} build/${DISTRIBUTION}/bin/${BINARY}

	mkdir -p build/${DISTRIBUTION}/share/applications
	echo "[Desktop Entry]" > build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Version=${VERSION}" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Type=Application" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Name=${PROGNAME}" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Comment=${COMMENT}" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "TryExec=${BINARY}" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Exec=${BINARY} %F" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Icon=${BINARY}" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Keywords=cam;cnc;gcode;dxf;" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "Terminal=false" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	echo "" >> build/${DISTRIBUTION}/share/applications/${BINARY}.desktop
	mkdir -p build/${DISTRIBUTION}/share/pixmaps
	cp -p icons/icon_128.png build/${DISTRIBUTION}/share/pixmaps/${BINARY}.png

	cat desc.txt >> build/${DISTRIBUTION}/+DESC
	echo "@comment ${COMMENT}" > build/${DISTRIBUTION}/+CONTENTS
	echo "@name ${PROGRAM}-${VERSION}" >> build/${DISTRIBUTION}/+CONTENTS
	echo "@arch ${MACHINE}" >> build/${DISTRIBUTION}/+CONTENTS
	echo "+DESC" >> build/${DISTRIBUTION}/+CONTENTS
	#echo "@sha 2hdzVfdHx5FWZ5A7gEOrE1uKtNiAWKMo3yuNnXuZAEQ=" >> build/${DISTRIBUTION}/+CONTENTS
	#echo "@size 552" >> build/${DISTRIBUTION}/+CONTENTS
	#echo "@depend devel/p5-WeakRef:p5-WeakRef-*:p5-WeakRef-0.01p4" >> build/${DISTRIBUTION}/+CONTENTS
	echo "@cwd /usr/local" >> build/${DISTRIBUTION}/+CONTENTS
	(for F in `find build/${DISTRIBUTION} -type f | grep -v "+"` ; do echo "$$F" | sed "s|^build/${DISTRIBUTION}/||g" ; echo "@sha `sha256 $$F | cut -d" " -f4`"; echo "@size `stat -f %z $$F`"; echo "@ts `stat -f %m $$F`"; done) >> build/${DISTRIBUTION}/+CONTENTS

	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	tar -C build/${DISTRIBUTION}/ -czpPf packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.tgz +CONTENTS +DESC ${BINARY} bin share
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.tgz"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, FREEBSD)

depends:
	pkg install git gmake pkgconf gettext freeglut gtkglext gtksourceview2 lua51

package: ${BINARY}
	strip ${BINARY}
	rm -rf build/${DISTRIBUTION}
	mkdir -p build/${DISTRIBUTION}
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}
	cp ${BINARY} build/${DISTRIBUTION}${INSTALL_PATH}/${BINARY}
	chmod 755 build/${DISTRIBUTION}${INSTALL_PATH}/${BINARY}
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}/posts
	cp -p posts/* build/${DISTRIBUTION}${INSTALL_PATH}/posts
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}/textures
	cp -p textures/* build/${DISTRIBUTION}${INSTALL_PATH}/textures
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}/icons
	cp -p icons/* build/${DISTRIBUTION}${INSTALL_PATH}/icons
	mkdir -p build/${DISTRIBUTION}${INSTALL_PATH}/fonts
	cp -p fonts/* build/${DISTRIBUTION}${INSTALL_PATH}/fonts
	cp -p LICENSE.txt material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf build/${DISTRIBUTION}${INSTALL_PATH}/
	mkdir -p build/${DISTRIBUTION}/usr/local/bin/
	ln -sf ${INSTALL_PATH}/${BINARY} build/${DISTRIBUTION}/usr/local/bin/${BINARY}

	mkdir -p build/${DISTRIBUTION}/usr/local/share/applications
	echo "[Desktop Entry]" > build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Version=${VERSION}" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Type=Application" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Name=${PROGNAME}" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Comment=${COMMENT}" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "TryExec=${BINARY}" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Exec=${BINARY} %F" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Icon=${BINARY}" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Keywords=cam;cnc;gcode;dxf;" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "Terminal=false" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	echo "" >> build/${DISTRIBUTION}/usr/local/share/applications/${BINARY}.desktop
	mkdir -p build/${DISTRIBUTION}/usr/local/share/pixmaps
	cp -p icons/icon_128.png build/${DISTRIBUTION}/usr/local/share/pixmaps/${BINARY}.png

	echo "name: ${BINARY}" > build/${DISTRIBUTION}/+MANIFEST
	echo "version: ${VERSION}_0" >> build/${DISTRIBUTION}/+MANIFEST
	echo "origin: graphics" >> build/${DISTRIBUTION}/+MANIFEST
	echo "comment: ${COMMENT}" >> build/${DISTRIBUTION}/+MANIFEST
	echo "arch: ${MACHINE}" >> build/${DISTRIBUTION}/+MANIFEST
	echo "www: ${HOMEPAGE}" >> build/${DISTRIBUTION}/+MANIFEST
	echo "maintainer: ${MAINTAINER_EMAIL}" >> build/${DISTRIBUTION}/+MANIFEST
	echo "prefix: /opt" >> build/${DISTRIBUTION}/+MANIFEST
	echo "licenselogic: or" >> build/${DISTRIBUTION}/+MANIFEST
	echo "licenses: [GPL3]" >> build/${DISTRIBUTION}/+MANIFEST
	echo "flatsize: `du -sck build/${DISTRIBUTION}/ | tail -n1 | awk '{print $$1}'`" >> build/${DISTRIBUTION}/+MANIFEST
	#echo "users: [USER1, USER2]" >> build/${DISTRIBUTION}/+MANIFEST
	#echo "groups: [GROUP1, GROUP2]" >> build/${DISTRIBUTION}/+MANIFEST
	#echo "options: { OPT1: off, OPT2: on }" >> build/${DISTRIBUTION}/+MANIFEST
	echo "desc: |-" >> build/${DISTRIBUTION}/+MANIFEST
	echo " 2D CAM-Tool for Linux, Windows and Mac OS X" >> build/${DISTRIBUTION}/+MANIFEST
	#echo "desc: {" >> build/${DISTRIBUTION}/+MANIFEST
	#cat desc.txt | sed "s|^| |g" >> build/${DISTRIBUTION}/+MANIFEST
	#echo "}" >> build/${DISTRIBUTION}/+MANIFEST
	echo "categories: [graphics]" >> build/${DISTRIBUTION}/+MANIFEST
	#echo "deps:" >> build/${DISTRIBUTION}/+MANIFEST
	#echo "  libiconv: {origin: converters/libiconv, version: 1.13.1_2}" >> build/${DISTRIBUTION}/+MANIFEST
	#echo "  perl: {origin: lang/perl5.12, version: 5.12.4 }" >> build/${DISTRIBUTION}/+MANIFEST
	#freeglut gtkglext gtksourceview2 lua51
	echo "files: {" >> build/${DISTRIBUTION}/+MANIFEST
	(for F in `find build/${DISTRIBUTION} -type f | grep -v "+"` ; do echo "  `echo $$F | sed "s|^build/${DISTRIBUTION}||g"`: \"`sha256 $$F | cut -d" " -f4`\"" ; done) >> build/${DISTRIBUTION}/+MANIFEST
	echo "}" >> build/${DISTRIBUTION}/+MANIFEST
	echo "scripts: {" >> build/${DISTRIBUTION}/+MANIFEST
	echo "  pre-install:  {" >> build/${DISTRIBUTION}/+MANIFEST
	echo "    #!/bin/sh" >> build/${DISTRIBUTION}/+MANIFEST
	echo "    echo pre-install" >> build/${DISTRIBUTION}/+MANIFEST
	echo "  }" >> build/${DISTRIBUTION}/+MANIFEST
	echo "  post-install:  {" >> build/${DISTRIBUTION}/+MANIFEST
	echo "    #!/bin/sh" >> build/${DISTRIBUTION}/+MANIFEST
	echo "    echo post-install" >> build/${DISTRIBUTION}/+MANIFEST
	echo "  }" >> build/${DISTRIBUTION}/+MANIFEST
	echo "}" >> build/${DISTRIBUTION}/+MANIFEST
	mkdir -p build/${DISTRIBUTION}/${MACHINE}/${VERSION}/
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	tar -s "|.${INSTALL_PATH}|${INSTALL_PATH}|" -s "|./usr/local|/usr/local|" -C build/${DISTRIBUTION}/ -czpPf packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.tgz +MANIFEST .${INSTALL_PATH} ./usr/local/share ./usr/local/bin/${BINARY}
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.tgz"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
ifeq (${TARGET}, ARCHLINUX)

depends:
	pacman -Syy
	pacman-key --refresh-keys
	for PKG in mesa-libgl gtk2 gtkglext gtksourceview2 git freeglut pkg-config lua51 make clang gcc libunistring glib2 do yes | pacman -S $PKG || true	done

package: ${BINARY}
	strip --remove-section=.comment --remove-section=.note ${BINARY}
	rm -rf build/${DISTRIBUTION}/${PROGRAM}-${VERSION}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}
	cp -p ${BINARY} build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/${BINARY}
	chmod 755 build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/${BINARY}
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/posts
	cp -p posts/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/posts
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/textures
	cp -p textures/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/textures
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/icons
	cp -p icons/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/icons
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/fonts
	cp -p fonts/* build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/fonts
	cp -p material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf test-minimal.dxf build/${DISTRIBUTION}/${PROGRAM}-${VERSION}${INSTALL_PATH}/
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/bin
	ln -sf ../lib/${BINARY}/${BINARY} build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/bin/${BINARY}

	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications
	echo "[Desktop Entry]" > build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Version=${VERSION}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Type=Application" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Name=${PROGNAME}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Comment=${COMMENT}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "TryExec=${BINARY}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Exec=${BINARY} %F" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Icon=${BINARY}" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Keywords=cam;cnc;gcode;dxf;" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "Terminal=false" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	echo "" >> build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/applications/${BINARY}.desktop
	mkdir -p build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/pixmaps
	cp -p icons/icon_128.png build/${DISTRIBUTION}/${PROGRAM}-${VERSION}/usr/share/pixmaps/${BINARY}.png

	echo "# Maintainer: ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>" > build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "pkgname=${PROGRAM}" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "pkgver=${VERSION}" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "pkgrel=1" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "pkgdesc="${COMMENT}"" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "arch=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "url=" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "license=('GPL')" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "groups=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "depends=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "makedepends=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "optdepends=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "provides=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "conflicts=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "replaces=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "backup=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "options=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "install=" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "changelog=" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "source=" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "noextract=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "sha256sums=()" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "build() {" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "}" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "package() {" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "  cd \"\$$pkgname-\$$pkgver\"" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "  cp -a * \"\$$pkgdir/\"" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO
	echo "}" >> build/${DISTRIBUTION}/${BINARY}.PKGINFO

	(cd build/${DISTRIBUTION} ; tar czpf /usr/src/packages/SOURCES/${PROGRAM}-${VERSION}.tar.gz ${PROGRAM}-${VERSION})
	makepkg -i /usr/src/packages/SPECS/${BINARY}.spec
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv /usr/src/packages/RPMS/${MACHINE}/${PROGRAM}-${VERSION}-1.${MACHINE}.rpm packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-1_${MACHINE}.rpm
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-1_${MACHINE}.rpm"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 test-minimal.dxf > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif

doc: ${BINARY}
	test -e ${PROGRAM}.github.io && sh utils/help2html.sh ./${BINARY} > ${PROGRAM}.github.io/de/cmdline.html || true
	test -e ${PROGRAM}.wiki/ && sh utils/help2md.sh ./${BINARY} > ${PROGRAM}.wiki/Commandline-Arguments.md || true

check:
	cppcheck --error-exitcode=1 --quiet src/

pull:
	git pull --rebase
	test -e ${PROGRAM}.github.io && (cd ${PROGRAM}.github.io/ ; git pull --rebase) || true
	test -e ${PROGRAM}.wiki/ && (cd ${PROGRAM}.wiki/ ; git pull --rebase) || true




