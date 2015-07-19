
ifeq (${TARGET}, AUTO)
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

docker-build:
	docker build -t cammill .

docker-run:
	docker run -ti --rm -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev/dri:/dev/dri -v /dev/video0:/dev/video0 -v $$HOME/.Xauthority:/home/cammill/.Xauthority --net=host cammill

gprof:
	gcc -pg -o ${BINARY} ${OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}
	@echo "./${BINARY}"
	@echo "gprof ${BINARY} gmon.out"

depends:
	apt-get install clang libgtkglext1-dev libgtksourceview2.0-dev liblua5.1-0-dev freeglut3-dev libglu1-mesa-dev libgtk2.0-dev libgvnc-1.0-dev libg3d-dev lintian

changelog_debian:
	@echo "generate debian changelog"
	@echo "${PROGRAM} (${VERSION}) unstable; urgency=low\n\n  * Git Release.\n  * take a look in to changelog.gz\n\n -- ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>  `date -R`\n" | gzip -n -9 > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/changelog.Debian.gz
	@chmod 0644 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/changelog.Debian.gz

package: peinstall_unix changelog_debian debian_control
	dpkg-deb --build ${PKG_INSTALL_PATH}
	lintian -I --show-overrides ${PKG_INSTALL_PATH}.deb || true
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${PKG_INSTALL_PATH}.deb packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.deb
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.deb"
	@echo "##"

package_install:
	dpkg -i packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.deb

test: binary
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
