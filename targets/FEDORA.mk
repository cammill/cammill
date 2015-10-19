
#autodetect system
ifeq (${TARGET}, AUTO)
	ifeq ($(OS),Windows_NT)
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),Linux)
			ifneq ("$(wildcard /etc/fedora-release)","")
				TARGET = FEDORA
			endif
		endif
	endif
endif

ifeq (${TARGET}, FEDORA)

SYSTEM           ?= $(shell uname -s)
MACHINE          ?= $(shell uname -m)
RELEASE          ?= $(shell lsb_release -s -r)
DISTRIBUTION     ?= $(shell lsb_release -s -i | tr " " "_")
COMP             ?= gcc
PKGS             ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua
LIBS             ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lXext -lxcb -lXau -lgcc -lc
INSTALL_PATH     ?= /usr
RPMBASEDIR       ?= ~/rpmbuild

docker-build:
	docker build -t cammill docker/${TARGET}

docker-run:
	docker run -ti --rm -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev/dri:/dev/dri -v /dev/video0:/dev/video0 -v $$HOME/.Xauthority:/home/cammill/.Xauthority --net=host cammill

depends:
	yum install gtkglext-devel gtksourceview2-devel lua-devel freeglut-devel make clang gcc gtk+-devel rpm-build git

package: peinstall_unix rpmspec
	mkdir -p ${RPMBASEDIR}/SPECS
	cp -a ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/${PROGRAM}.spec ${RPMBASEDIR}/SPECS/${PROGRAM}.spec
	mkdir -p ${RPMBASEDIR}/SOURCES
	(cd ${PKG_INSTALL_PATH} ; tar czpf ${RPMBASEDIR}/SOURCES/${PROGRAM}-${VERSION}.tar.gz ../${PROGRAM}-${VERSION})

	rpmbuild --bb ${RPMBASEDIR}/SPECS/${PROGRAM}.spec

	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${RPMBASEDIR}/RPMS/${MACHINE}/${PROGRAM}-${VERSION}-${VRELEASE}.${MACHINE}.rpm packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.rpm
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
