
#autodetect system
ifeq (${TARGET}, AUTO)
	ifeq ($(OS),Windows_NT)
		ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			MACHINE = AMD64
		endif
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			MACHINE = IA32
		endif
		TARGET = MINGW32
	endif
endif

ifeq (${TARGET}, MINGW32)

SYSTEM          ?= Windows
RELEASE         ?= MinGW
DISTRIBUTION    ?= Windows
MACHINE         ?= IA32
LIBS            ?= -lm -lstdc++ -lgcc
CROSS           ?= /usr/lib/mxe/usr/bin/i686-w64-mingw32.static-
COMP            ?= ${CROSS}gcc
PKGS            ?= gtk+-2.0 gtk+-win32-2.0 gtkglext-1.0 gtksourceview-2.0 lua
INSTALL_PATH    ?= Windows/CAMmill
TESTFILE        ?= share/doc/cammill/examples/test-minimal.dxf
EXTRA_OBJS      ?= src/icons.res
BINARY          ?= bin/cammill.exe

src/icons.rc: ./share/cammill/icons/icon.ico
	echo "id ICON \"./share/cammill/icons/icon.ico\"" > src/icons.rc

src/icons.res: src/icons.rc
	$(CROSS)windres src/icons.rc -O coff -o src/icons.res

peinstall: pinstall
	mkdir -p ${PKG_INSTALL_PATH}/${INSTALL_PATH}/
	cp -p share/${PROGRAM}/icons/icon.ico ${PKG_INSTALL_PATH}/${INSTALL_PATH}/icon.ico
	cp -p LICENSE.txt ${PKG_INSTALL_PATH}/${INSTALL_PATH}/LICENSE.txt

package: peinstall
	(cd ${PKG_INSTALL_PATH}/${INSTALL_PATH} ; tclsh ../../../../../utils/create-win-installer.tclsh > installer.nsis)
	(cd ${PKG_INSTALL_PATH}/${INSTALL_PATH} ; makensis installer.nsis)
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${PKG_INSTALL_PATH}/${INSTALL_PATH}/installer.exe packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.exe
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.exe"
	@echo "##"

test: ${BINARY}
	wine ${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
