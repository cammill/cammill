
#TARGETS: AUTO, DEBIAN, FREEBSD, MINGW32, NETBSD, OSX, OPENBSD, SUSE
TARGET ?= AUTO

all: binary

include targets/*.mk
include defaults.mk

binary: info ${BINARY}

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
	@echo "	COMP            ${COMP}"
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

${BINARY}: ${OBJS} ${EXTRA_OBJS}
	@echo "linking ${BINARY}"
	@mkdir -p bin
	@$(COMP) -o ${BINARY} ${OBJS} ${EXTRA_OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}

%.o: %.c
	@echo "compile $< -> $@"
	@$(COMP) -c $(CFLAGS) ${INCLUDES} $< -o $@

clean:
	rm -rf ${OBJS}
	rm -rf ${BINARY}

install: ${BINARY}
	@echo "install files"
	@${STRIP_CMD} ${BINARY}
	@install -m 0755 -d ${INSTALL_PATH}
	@install -m 0755 -d ${INSTALL_PATH}/bin
	@install -m 0755 -d ${INSTALL_PATH}/lib/cammill
	@install -m 0755 -d ${INSTALL_PATH}/lib/cammill/posts
	@install -m 0755 -d ${INSTALL_PATH}/share/cammill
	@install -m 0755 -d ${INSTALL_PATH}/share/cammill/icons
	@install -m 0755 -d ${INSTALL_PATH}/share/cammill/textures
	@install -m 0755 -d ${INSTALL_PATH}/share/cammill/fonts
	@install -m 0755 -d ${INSTALL_PATH}/share/doc/cammill
	@install -m 0755 -d ${INSTALL_PATH}/share/doc/cammill/examples
	@install -m 0755 ${BINARY} ${INSTALL_PATH}/${BINARY}
	@install -m 0644 lib/cammill/posts/*.scpost ${INSTALL_PATH}/lib/cammill/posts/
	@install -m 0644 lib/cammill/*.lua ${INSTALL_PATH}/lib/cammill/
	@install -m 0644 share/cammill/*.tbl ${INSTALL_PATH}/share/cammill/
	@install -m 0644 share/cammill/icons/*.png ${INSTALL_PATH}/share/cammill/icons/
	@install -m 0644 share/cammill/icons/*.icns ${INSTALL_PATH}/share/cammill/icons/
	@install -m 0644 share/cammill/icons/*.svg ${INSTALL_PATH}/share/cammill/icons/
	@install -m 0644 share/cammill/icons/*.ico ${INSTALL_PATH}/share/cammill/icons/
	@install -m 0644 share/cammill/textures/*.bmp ${INSTALL_PATH}/share/cammill/textures/
	@install -m 0644 share/cammill/fonts/*.jhf ${INSTALL_PATH}/share/cammill/fonts/
	@install -m 0644 share/doc/cammill/examples/*.dxf ${INSTALL_PATH}/share/doc/cammill/examples/

pinstall: ${BINARY}
	@echo "install package files"
	@${STRIP_CMD} ${BINARY}
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/bin
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib/${PROGRAM}
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib/${PROGRAM}/posts
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/textures
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/fonts
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}
	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/examples
	@install -m 0755 ${BINARY} ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${BINARY}
	@install -m 0644 lib/${PROGRAM}/posts/*.scpost ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib/${PROGRAM}/posts/
	@install -m 0644 lib/${PROGRAM}/*.lua ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib/${PROGRAM}/
	@install -m 0644 share/${PROGRAM}/*.tbl ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/
	@install -m 0644 share/${PROGRAM}/icons/*.png ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons/
	@install -m 0644 share/${PROGRAM}/icons/*.icns ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons/
	@install -m 0644 share/${PROGRAM}/icons/*.svg ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons/
	@install -m 0644 share/${PROGRAM}/icons/*.ico ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons/
	@install -m 0644 share/${PROGRAM}/textures/*.bmp ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/textures/
	@install -m 0644 share/${PROGRAM}/fonts/*.jhf ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/fonts/
	@install -m 0644 share/doc/${PROGRAM}/examples/*.dxf ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/examples/

peinstall_unix: pinstall
	@echo "install unix extra files"

	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/man/man1/
	@help2man ./${BINARY} -N -n "${COMMENT}" | gzip -n -9 > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/man/man1/${PROGRAM}.1.gz

	@install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/
	@install -m 0644 README.md ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/README

	@echo "It was downloaded from https://github.com/${PROGRAM}" > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "Copyright 2014 - 2015 by Oliver Dippel <oliver@multixmedia.org>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "Copyright 2014 - 2015 by McUles mcules@fpv-club.de" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "Copyright 2014 - 2015 by Jakob Flierl <@koppi>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "Copyright 2014 - 2015 by Carlo <onekk>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "License:" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "This program is free software; you can redistribute it and/or modify it" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "under the terms of the GNU General Public License as published by the" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "Free Software Foundation; either version 2, or (at your option) any" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "later version." >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "On Debian systems, the complete text of the GNU General Public License" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "can be found in the file /usr/share/common-licenses/GPL-3" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright
	@chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/copyright

	@git log | gzip -n -9 > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/changelog.gz
	@chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/changelog.gz

	@mkdir -p ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications
	@echo "[Desktop Entry]" > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Version=${VERSION}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Type=Application" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Name=${PROGNAME}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Comment=${COMMENT}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "TryExec=${PROGRAM}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Exec=${PROGRAM} %F" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Icon=${PROGRAM}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Categories=Graphics;2DGraphics;Engineering;GTK;" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Keywords=cam;cnc;gcode;dxf;" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "Terminal=false" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop
	@chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/applications/${PROGRAM}.desktop

	@mkdir -p ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/pixmaps
	@install -m 0644 share/${PROGRAM}/icons/icon_128.png ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/pixmaps/${PROGRAM}.png
	@chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/pixmaps/${PROGRAM}.png

rpmspec:
	@echo "generate rpmsec"
	@install -m 0644 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/
	@echo "Summary: ${COMMENT}" > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "Name: ${PROGRAM}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "Version: ${VERSION}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "Release: ${VRELEASE}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "License: GPL" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "Group: Utilities/System" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "BuildRoot: %{_tmppath}/%{name}-root" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "Requires: bash" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "Source0: ${PROGRAM}-%{version}.tar.gz" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "%description" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@cat desc.txt | grep ".." | sed "s|^| |g" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "%prep" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "%setup" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "%build" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "%install" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "rm -rf \$${RPM_BUILD_ROOT}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "mkdir -p \$${RPM_BUILD_ROOT}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "cp -a * \$${RPM_BUILD_ROOT}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "%clean" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "rm -rf \$${RPM_BUILD_ROOT}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "%files" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "/usr/bin/${PROGRAM}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@(for F in `find ${PKG_INSTALL_PATH} -type f`; do echo "$$F" | sed "s|${PKG_INSTALL_PATH}||g"; done) >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec
	@echo "" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec

debian_control:
	@echo "generate debian-control"
	@mkdir -p ${PKG_INSTALL_PATH}/DEBIAN/
	@(for F in `find ${PKG_INSTALL_PATH} -type f | grep -v "^${PKG_INSTALL_PATH}/DEBIAN/"`; do md5sum "$$F" | sed "s| ${PKG_INSTALL_PATH}/| |g"; done) >> ${PKG_INSTALL_PATH}/DEBIAN/md5sums
	@echo "Package: ${PROGRAM}" > ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Source: ${PROGRAM}" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Version: ${VERSION}-${VRELEASE}" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Architecture: `dpkg --print-architecture`" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Installed-Size: `du -sk ${PKG_INSTALL_PATH}|awk '{ print $$1 }'`" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Maintainer: ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Depends: libc6, libgtksourceview2.0-0, libgtkglext1, liblua5.1-0" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Section: graphics" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Priority: optional" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Homepage: ${HOMEPAGE}" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@echo "Description: ${COMMENT}" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@cat DESCRIPTION | grep ".." | sed "s|^| |g" >> ${PKG_INSTALL_PATH}/DEBIAN/control
	@chmod -R -s ${PKG_INSTALL_PATH}/DEBIAN/
	@chown -R root:root ${PKG_INSTALL_PATH}/DEBIAN/
	@chmod -R 0755 ${PKG_INSTALL_PATH}/DEBIAN/
	@chmod 0644 ${PKG_INSTALL_PATH}/DEBIAN/control
	@chmod 0644 ${PKG_INSTALL_PATH}/DEBIAN/md5sums

doc: ${BINARY}
	test -e ${PROGRAM}.github.io && sh utils/help2html.sh ./${BINARY} > ${PROGRAM}.github.io/de/cmdline.html || true
	test -e ${PROGRAM}.wiki/ && sh utils/help2md.sh ./${BINARY} > ${PROGRAM}.wiki/Commandline-Arguments.md || true

check:
	cppcheck --error-exitcode=1 --quiet src/

pull:
	git pull --rebase
	test -e ${PROGRAM}.github.io && (cd ${PROGRAM}.github.io/ ; git pull --rebase) || true
	test -e ${PROGRAM}.wiki/ && (cd ${PROGRAM}.wiki/ ; git pull --rebase) || true

