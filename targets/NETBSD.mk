
#autodetect system
ifeq (${TARGET}, NONE)
	ifeq ($(OS),Windows_NT)
	else
		SYSTEM ?= $(shell uname -s)
		ifeq ($(SYSTEM),NetBSD)
			TARGET = NETBSD
		endif
	endif
endif

ifeq (${TARGET}, NETBSD)

SYSTEM          ?= $(shell uname -s)
MACHINE         ?= $(shell uname -m)
RELEASE         ?= $(shell uname -r | cut -d"-" -f1)
DISTRIBUTION    ?= ${SYSTEM}
COMP            ?= gcc
PKGS            ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua-5.1
INSTALL_PATH    ?= /usr

test: ${BINARY}
	./${BINARY} -bm 1 ${TESTFILE} > test.ngc
	sh utils/gvalid.sh test.ngc
	rm -rf test.ngc

endif
