
#autodetect system
ifeq (${TARGET}, AUTO)
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

contents_openbsd:
	@echo "generate contents"
	@cat DESCRIPTION >> ${PKG_INSTALL_PATH}/+DESC
	@echo "@comment ${COMMENT}" > ${PKG_INSTALL_PATH}/+CONTENTS
	@echo "@name ${PROGRAM}-${VERSION}-${VRELEASE}" >> ${PKG_INSTALL_PATH}/+CONTENTS
	@echo "@arch ${MACHINE}" >> ${PKG_INSTALL_PATH}/+CONTENTS
	@echo "+DESC" >> ${PKG_INSTALL_PATH}/+CONTENTS
	#@echo "@sha 2hdzVfdHx5FWZ5A7gEOrE1uKtNiAWKMo3yuNnXuZAEQ=" >> ${PKG_INSTALL_PATH}/+CONTENTS
	#@echo "@size 552" >> ${PKG_INSTALL_PATH}/+CONTENTS
	#@echo "@depend devel/p5-WeakRef:p5-WeakRef-*:p5-WeakRef-0.01p4" >> ${PKG_INSTALL_PATH}/+CONTENTS
	@echo "@cwd ${PKG_INSTALL_PATH}" >> ${PKG_INSTALL_PATH}/+CONTENTS
	@(for F in `find ${PKG_INSTALL_PATH} -type f | grep -v "+"` ; do echo "$$F" | sed "s|^${PKG_INSTALL_PATH}/||g" ; echo "@sha `sha256 $$F | cut -d" " -f4`"; echo "@size `stat -f %z $$F`"; echo "@ts `stat -f %m $$F`"; done) >> ${PKG_INSTALL_PATH}/+CONTENTS

package: peinstall_unix contents_openbsd
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	tar -C ${PKG_INSTALL_PATH}/ -czpPf packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.tgz +CONTENTS +DESC ${BINARY} bin share
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.tgz"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
