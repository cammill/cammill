
#autodetect system
ifeq (${TARGET}, NONE)
	ifeq ($(OS),Windows_NT)
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),Linux)
			ifneq ("$(wildcard /etc/arch-release)","")
				TARGET = ARCHLINUX
			endif
		endif
	endif
endif

ifeq (${TARGET}, ARCHLINUX)

SYSTEM           ?= $(shell uname -s)
MACHINE          ?= $(shell uname -m)
RELEASE          ?= $(shell uname -r | cut -d"-" -f1)
DISTRIBUTION     ?= Arch-Linux
COMP             ?= clang
PKGS             ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua5.1
INSTALL_PATH     ?= /usr


depends:
	pacman -Syy
	pacman-key --refresh-keys
	for PKG in mesa-libgl gtk2 gtkglext gtksourceview2 git freeglut pkg-config lua51 make clang gcc libunistring glib2 do yes | pacman -S $PKG || true	done

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

pkgbuild:
	echo "# Maintainer: ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>" > ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "pkgname=${PROGRAM}" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "pkgver=${VERSION}" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "pkgrel=1" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "pkgdesc=\"${COMMENT}\"" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "arch=('${MACHINE}')" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "url=" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "license=('GPL')" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "groups=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "depends=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "makedepends=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "optdepends=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "provides=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "conflicts=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "replaces=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "backup=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "options=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "install=" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "changelog=" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "source=" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "noextract=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "sha256sums=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "build() {" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "  echo -n" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "}" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "package() {" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "  cd .." >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "  cp -a \"\$$pkgname-\$$pkgver\"/* \"\$$pkgdir/\"" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	echo "}" >> ${PKG_INSTALL_PATH}/../PKGBUILD

package: peinstall pkgbuild
	(cd ${PKG_INSTALL_PATH}/../ ; makepkg --asroot -e)
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${PKG_INSTALL_PATH}/../${PROGRAM}-${VERSION}-1-${MACHINE}.pkg.tar.xz packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}-${VERSION}-1-${MACHINE}.pkg.tar.xz
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}-${VERSION}-1-${MACHINE}.pkg.tar.xz"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
