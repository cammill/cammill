
#autodetect system
ifeq (${TARGET}, AUTO)
	ifeq ($(OS),Windows_NT)
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),Linux)
			ifneq ("$(wildcard /etc/centos-release)","")
				TARGET = CENTOS
			endif
		endif
	endif
endif

ifeq (${TARGET}, CENTOS)

SYSTEM           ?= $(shell uname -s)
MACHINE          ?= $(shell uname -m)
RELEASE          ?= $(shell lsb_release -s -r)
DISTRIBUTION     ?= $(shell lsb_release -s -i | tr " " "_")
PKGS             ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua
LIBS             ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lXext -lxcb -lXau -lgcc -lc
COMP             ?= gcc
INSTALL_PATH     ?= /usr
RPMBASEDIR       ?= ~/rpmbuild


depends:
	yum install gtkglext-devel lua-devel freeglut-devel make gcc gtk2-devel rpm-build git
	rpm --import http://winswitch.org/gpg.asc
	cd /etc/yum.repos.d/
	curl -O https://winswitch.org/downloads/CentOS/winswitch.repo
	yum install gtkglext-devel

package: peinstall_unix rpmspec
	mkdir -p ${RPMBASEDIR}/SPECS
	cp -a ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec ${RPMBASEDIR}/SPECS/${PROGRAM}.spec
	mkdir -p ${RPMBASEDIR}/SOURCES
	(cd ${PKG_INSTALL_PATH} ; tar czpf ${RPMBASEDIR}/SOURCES/${PROGRAM}-${VERSION}.tar.gz ../${PROGRAM}-${VERSION})

	rpmbuild --bb ${RPMBASEDIR}/SPECS/${PROGRAM}.spec

	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${RPMBASEDIR}/RPMS/*/${PROGRAM}-${VERSION}-${VRELEASE}.*.rpm packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.rpm
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.rpm"
	@echo "##"

package_install:
	rpm -i packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.rpm

test: binary
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
