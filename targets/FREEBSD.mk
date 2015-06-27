
#autodetect system
ifeq (${TARGET}, NONE)
	ifeq ($(OS),Windows_NT)
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),FreeBSD)
			TARGET = FREEBSD
		endif
	endif
endif

ifeq (${TARGET}, FREEBSD)

SYSTEM           ?= $(shell uname -s)
MACHINE          ?= $(shell uname -m)
RELEASE          ?= $(shell uname -r | cut -d"-" -f1)
DISTRIBUTION     ?= ${SYSTEM}
COMP             ?= clang
PKGS             ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua-5.1
INSTALL_PATH     ?= /usr


depends:
	pkg install git gmake pkgconf gettext freeglut gtkglext gtksourceview2 lua51

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

manifest:
	echo "name: ${BINARY}" > ${PKG_INSTALL_PATH}/+MANIFEST
	echo "version: ${VERSION}_0" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "origin: graphics" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "comment: ${COMMENT}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "arch: ${MACHINE}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "www: ${HOMEPAGE}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "maintainer: ${MAINTAINER_EMAIL}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "prefix: /opt" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "licenselogic: or" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "licenses: [GPL3]" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "flatsize: `du -sck ${PKG_INSTALL_PATH}/ | tail -n1 | awk '{print $$1}'`" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#echo "users: [USER1, USER2]" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#echo "groups: [GROUP1, GROUP2]" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#echo "options: { OPT1: off, OPT2: on }" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "desc: |-" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo " 2D CAM-Tool for Linux, Windows and Mac OS X" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#echo "desc: {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#cat DESCRIPTION | sed "s|^| |g" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#echo "}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "categories: [graphics]" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#echo "deps:" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#echo "  libiconv: {origin: converters/libiconv, version: 1.13.1_2}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#echo "  perl: {origin: lang/perl5.12, version: 5.12.4 }" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#freeglut gtkglext gtksourceview2 lua51
	echo "files: {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	(for F in `find ${PKG_INSTALL_PATH} -type f | grep -v "+"` ; do echo "  `echo $$F | sed "s|^${PKG_INSTALL_PATH}||g"`: \"`sha256 $$F | cut -d" " -f4`\"" ; done) >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "scripts: {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "  pre-install:  {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "    #!/bin/sh" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "    echo pre-install" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "  }" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "  post-install:  {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "    #!/bin/sh" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "    echo post-install" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "  }" >> ${PKG_INSTALL_PATH}/+MANIFEST
	echo "}" >> ${PKG_INSTALL_PATH}/+MANIFEST

package: peinstall manifest
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	tar -C ${PKG_INSTALL_PATH}/ -czpPf packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.tgz +MANIFEST `echo ${INSTALL_PATH} | sed "s|^/||g"`
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.tgz"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
