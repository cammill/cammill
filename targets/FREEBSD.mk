
#autodetect system
ifeq (${TARGET}, AUTO)
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

manifest_freebsd:
	@echo "generate manifest"
	@echo "name: ${BINARY}" > ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "version: ${VERSION}_${VRELEASE}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "origin: graphics" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "comment: ${COMMENT}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "arch: ${MACHINE}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "www: ${HOMEPAGE}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "maintainer: ${MAINTAINER_EMAIL}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "prefix: /opt" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "licenselogic: or" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "licenses: [GPL3]" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "flatsize: `du -sck ${PKG_INSTALL_PATH}/ | tail -n1 | awk '{print $$1}'`" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@echo "users: [USER1, USER2]" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@echo "groups: [GROUP1, GROUP2]" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@echo "options: { OPT1: off, OPT2: on }" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "desc: |-" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo " 2D CAM-Tool for Linux, Windows and Mac OS X" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@echo "desc: {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@cat DESCRIPTION | sed "s|^| |g" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@echo "}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "categories: [graphics]" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@echo "deps:" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@echo "  libiconv: {origin: converters/libiconv, version: 1.13.1_2}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@echo "  perl: {origin: lang/perl5.12, version: 5.12.4 }" >> ${PKG_INSTALL_PATH}/+MANIFEST
	#@freeglut gtkglext gtksourceview2 lua51
	@echo "files: {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@(for F in `find ${PKG_INSTALL_PATH} -type f | grep -v "+"` ; do echo "  `echo $$F | sed "s|^${PKG_INSTALL_PATH}||g"`: \"`sha256 $$F | cut -d" " -f4`\"" ; done) >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "}" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "scripts: {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "  pre-install:  {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "    #!/bin/sh" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "    echo pre-install" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "  }" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "  post-install:  {" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "    #!/bin/sh" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "    echo post-install" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "  }" >> ${PKG_INSTALL_PATH}/+MANIFEST
	@echo "}" >> ${PKG_INSTALL_PATH}/+MANIFEST

package: peinstall_unix manifest_freebsd
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	tar -C ${PKG_INSTALL_PATH}/ -czpPf packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${VRELEASE}_${MACHINE}.tgz +MANIFEST `echo ${INSTALL_PATH} | sed "s|^/||g"`
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}_${VRELEASE}_${MACHINE}.tgz"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
