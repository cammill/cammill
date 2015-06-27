
ifeq (${TARGET}, NONE)
	ifeq ($(OS),Linux)
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),Linux)
			ifneq ("$(wildcard /etc/debian_version)","")
				TARGET = DEBIAN
			endif
		endif
	endif
endif

ifeq (${TARGET}, DEBIAN)

SYSTEM           ?= $(shell uname -s)
MACHINE          ?= $(shell dpkg --print-architecture)
RELEASE          ?= $(shell lsb_release -s -r)
DISTRIBUTION     ?= $(shell lsb_release -s -i | tr " " "_")
PKGS             ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua5.1
LIBS             ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lstdc++ -lXext -lXi -lxcb -lXau -lXdmcp -lgcc -lc
INSTALL_PATH     ?= /usr


gprof:
	gcc -pg -o ${BINARY} ${OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}
	@echo "./${BINARY}"
	@echo "gprof ${BINARY} gmon.out"

depends:
	apt-get install clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev libg3d-dev lintian

peinstall: pinstall
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/man/man1/
	help2man ./${BINARY} -N -n "${COMMENT}" | gzip -n -9 > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/man/man1/${PROGRAM}.1.gz

	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/
	install -m 0644 README.md ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/README

	echo "It was downloaded from https://github.com/${PROGRAM}" > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "Copyright 2014 - 2015 by Oliver Dippel <oliver@multixmedia.org>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "Copyright 2014 - 2015 by McUles mcules@fpv-club.de" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "Copyright 2014 - 2015 by Jakob Flierl <@koppi>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "Copyright 2014 - 2015 by Carlo <onekk>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "License:" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "This program is free software; you can redistribute it and/or modify it" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "under the terms of the GNU General Public License as published by the" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "Free Software Foundation; either version 2, or (at your option) any" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "later version." >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "On Debian systems, the complete text of the GNU General Public License" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "can be found in the file /usr/share/common-licenses/GPL-3" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright

	git log | gzip -n -9 > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/changelog.gz
	chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/changelog.gz

	mkdir -p ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications
	echo "[Desktop Entry]" > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Version=${VERSION}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Type=Application" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Name=${PROGNAME}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Comment=${COMMENT}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "TryExec=${PROGRAM}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Exec=${PROGRAM} %F" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Icon=${PROGRAM}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Keywords=cam;cnc;gcode;dxf;" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "Terminal=false" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop

	mkdir -p ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/pixmaps
	install -m 0644 share/${PROGRAM}/icons/icon_128.png ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/pixmaps/${PROGRAM}.png
	chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/pixmaps/${PROGRAM}.png

	echo "${PROGRAM} (${VERSION}) unstable; urgency=low\n\n  * Git Release.\n  * take a look in to changelog.gz\n\n -- ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>  `date -R`\n" | gzip -n -9 > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/changelog.Debian.gz
	chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/changelog.Debian.gz

package: peinstall
	mkdir -p ${PKG_INSTALL_PATH}/DEBIAN/
	(for F in `find ${PKG_INSTALL_PATH} -type f | grep -v "^${PKG_INSTALL_PATH}/DEBIAN/"`; do md5sum "$$F" | sed "s| ${PKG_INSTALL_PATH}/| |g"; done) >> ${PKG_INSTALL_PATH}/DEBIAN/md5sums
	#(for F in material.tbl tool.tbl postprocessor.lua posts/* ; do echo "${INSTALL_PATH}/$$F" ; done) >> ${PKG_INSTALL_PATH}/DEBIAN/conffiles
	echo "Package: ${PROGRAM}" > ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Source: ${PROGRAM}" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Version: ${VERSION}" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Architecture: `dpkg --print-architecture`" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Maintainer: ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Depends: libc6, libgtksourceview2.0-0, libgtkglext1, liblua5.1-0" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Section: graphics" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Priority: optional" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Homepage: ${HOMEPAGE}" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	echo "Description: ${COMMENT}" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	cat DESCRIPTION | grep ".." | sed "s|^| |g" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	chmod -R -s ${PKG_INSTALL_PATH}/DEBIAN/
	chown -R root:root ${PKG_INSTALL_PATH}/DEBIAN/
	chmod -R 0755 ${PKG_INSTALL_PATH}/DEBIAN/
	chmod 0644 ${PKG_INSTALL_PATH}/DEBIAN/control
	chmod 0644 ${PKG_INSTALL_PATH}/DEBIAN/md5sums

	dpkg-deb --build ${PKG_INSTALL_PATH}
	lintian -I --show-overrides ${PKG_INSTALL_PATH}.deb || true
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${PKG_INSTALL_PATH}.deb packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.deb
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.deb"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
