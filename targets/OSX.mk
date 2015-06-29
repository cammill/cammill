
#autodetect system
ifeq (${TARGET}, AUTO)
	ifeq ($(OS),Windows_NT)
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),Darwin)
			TARGET = OSX
		endif
	endif
endif

ifeq (${TARGET}, OSX)

SYSTEM           ?= $(shell uname -s)
MACHINE          ?= $(shell uname -m)
RELEASE          ?= $(shell sw_vers -productVersion)
DISTRIBUTION     ?= $(shell sw_vers -productName | tr -d" ")
LIBS             ?= -framework OpenGL -framework GLUT -lm -lpthread -lstdc++ -lc
PKGS             ?= gtk+-2.0 gtkglext-1.0 gtksourceview-2.0 lua
INSTALL_PATH     ?= OSX/CAMmill
STRIP_CMD        ?= strip


depends:
	xcode-select --install
	ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
	brew update
	brew install Caskroom/cask/xquartz
	brew update
	brew install gtkglext
	brew install gtksourceview
	brew install lua
	brew install gettext
	brew link gettext --force

package: pinstall
	sh utils/osx-app.sh ${BINARY} ${VERSION} ${PKG_INSTALL_PATH}
	mkdir -p packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/
	mv ${PROGRAM}.dmg packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.dmg
	@echo "##"
	@echo "## packages/${DISTRIBUTION}/${RELEASE}/${MACHINE}/${PROGRAM}_${VERSION}-${VRELEASE}_${MACHINE}.dmg"
	@echo "##"

test: ${BINARY}
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
