
#autodetect system
ifeq (${TARGET}, AUTO)
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
VERSION          ?= $(shell test -d .git && git describe --tags --match "v*" | sed "s|^v||g" | sed "s|-test-*|t|g" | cut -d"-" -f1 | sed "s|[a-zA-Z].*||g")


depends:
	pacman -Syy
	pacman-key --refresh-keys
	for PKG in mesa-libgl gtk2 gtkglext gtksourceview2 git freeglut pkg-config lua51 make clang gcc libunistring glib2 do yes | pacman -S $PKG || true	done

pkgbuild_archlinux:
	@echo "generate pkgbuild"
	@echo "# Maintainer: ${MAINTAINER_NAME} <${MAINTAINER_EMAIL}>" > ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "pkgname=${PROGRAM}" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "pkgver=`echo ${VERSION} | sed "s|[a-zA-Z].*||g"`" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "pkgrel=${VRELEASE}" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "pkgdesc=\"${COMMENT}\"" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "arch=('${MACHINE}')" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "url=" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "license=('GPL')" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "groups=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "depends=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "makedepends=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "optdepends=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "provides=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "conflicts=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "replaces=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "backup=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "options=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "install=" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "changelog=" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "source=" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "noextract=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "sha256sums=()" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "build() {" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "  echo -n" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "}" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "package() {" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "  cd .." >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "  cp -a \"\$$pkgname-\$$pkgver\"/* \"\$$pkgdir/\"" >> ${PKG_INSTALL_PATH}/../PKGBUILD
	@echo "}" >> ${PKG_INSTALL_PATH}/../PKGBUILD

package: peinstall_unix pkgbuild_archlinux
	(cd ${PKG_INSTALL_PATH}/../ ; makepkg --asroot -e)
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${PKG_INSTALL_PATH}/../${PROGRAM}-${VERSION}-${VRELEASE}-${MACHINE}.pkg.tar.xz packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}-${VERSION}-${VRELEASE}-${MACHINE}.pkg.tar.xz
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}-${VERSION}-${VRELEASE}-${MACHINE}.pkg.tar.xz"
	@echo "##"

test: binary
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
