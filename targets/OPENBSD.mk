
#autodetect system
ifeq (${TARGET}, NONE)
	ifeq ($(OS),Windows_NT)
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),OpenBSD)
			TARGET = OPENBSD
		endif
	endif
endif

ifeq (${TARGET}, OPENBSD)

SYSTEM           ?= $(shell uname -s)
MACHINE          ?= $(shell uname -m)
RELEASE          ?= $(shell uname -r | cut -d"-" -f1)
DISTRIBUTION     ?= ${SYSTEM}
COMP             ?= gcc
PKGS             ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua51
INSTALL_PATH     ?= /usr


depends:
	pkg_add git gcc gmake freeglut gtk+ gtksourceview gtkglext lua

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

contents:
	cat DESCRIPTION >> ${PKG_INSTALL_PATH}/+DESC
	echo "@comment ${COMMENT}" > ${PKG_INSTALL_PATH}/+CONTENTS
	echo "@name ${PROGRAM}-${VERSION}" >> ${PKG_INSTALL_PATH}/+CONTENTS
	echo "@arch ${MACHINE}" >> ${PKG_INSTALL_PATH}/+CONTENTS
	echo "+DESC" >> ${PKG_INSTALL_PATH}/+CONTENTS
	#echo "@sha 2hdzVfdHx5FWZ5A7gEOrE1uKtNiAWKMo3yuNnXuZAEQ=" >> ${PKG_INSTALL_PATH}/+CONTENTS
	#echo "@size 552" >> ${PKG_INSTALL_PATH}/+CONTENTS
	#echo "@depend devel/p5-WeakRef:p5-WeakRef-*:p5-WeakRef-0.01p4" >> ${PKG_INSTALL_PATH}/+CONTENTS
	echo "@cwd ${PKG_INSTALL_PATH}" >> ${PKG_INSTALL_PATH}/+CONTENTS
	(for F in `find ${PKG_INSTALL_PATH} -type f | grep -v "+"` ; do echo "$$F" | sed "s|^${PKG_INSTALL_PATH}/||g" ; echo "@sha `sha256 $$F | cut -d" " -f4`"; echo "@size `stat -f %z $$F`"; echo "@ts `stat -f %m $$F`"; done) >> ${PKG_INSTALL_PATH}/+CONTENTS

package: peinstall contents
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	tar -C ${PKG_INSTALL_PATH}/ -czpPf packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.tgz +CONTENTS +DESC ${BINARY} bin share
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${MACHINE}.tgz"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
