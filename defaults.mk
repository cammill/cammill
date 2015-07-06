
PROGRAM     ?= cammill
PROGNAME    ?= CAMmill
COMMENT     ?= 2D CAM-Tool (DXF to GCODE)
HOMEPAGE    ?= http://www.multixmedia.org/${PROGRAM}/
MAINTAINER_NAME  ?= Oliver Dippel
MAINTAINER_EMAIL ?= oliver@multixmedia.org

VERSION     ?= $(shell test -d .git && git describe --tags --match "v*" | sed "s|^v||g" | sed "s|-test-*|t|g" | cut -d"-" -f1)
VRELEASE    ?= $(shell test -d .git && git describe --tags --match "v*" | sed "s|^v||g" | sed "s|-test-*|t|g" | cut -d"-" -f2)

ifeq ("$(VERSION)","")
VERSION      = 0.9
endif
ifeq ("$(VRELEASE)","")
VRELEASE     = 1
endif

BINARY     ?= bin/${PROGRAM}
COMP       ?= $(CROSS)clang
PKG_CONFIG ?= $(CROSS)pkg-config

TESTFILE = share/doc/cammill/examples/test-minimal.dxf
HERSHEY_FONTS_DIR = ./
INSTALL_PATH ?= /usr/local
PKG_INSTALL_PATH = build/${DISTRIBUTION}/${PROGRAM}-${VERSION}

STRIP_CMD ?= strip --remove-section=.comment --remove-section=.note

LIBS   ?= -lGL -lglut -lGLU -lX11 -lm -lpthread -lstdc++ -lXext -lXi -lxcb -lXau -lXdmcp -lgcc -lc
CFLAGS += -I./ -I./src
CFLAGS += "-DHERSHEY_FONTS_DIR=\"./\""
CFLAGS += -ggdb -Wall -Wno-unknown-pragmas -O3
CFLAGS += "-DTARGET=\"${TARGET}\""
CFLAGS += "-DSYSTEM=\"${SYSTEM}\""
CFLAGS += "-DMACHINE=\"${MACHINE}\""
CFLAGS += "-DVERSION=\"${VERSION}\""
CFLAGS += "-DVRELEASE=\"${VRELEASE}\""
CFLAGS += "-DTARGET_${TARGET}=1"
CFLAGS += "-DSYSTEM_${SYSTEM}=1"
CFLAGS += "-DMACHINE_${MACHINE}=1"

# debian hardening flags
ifeq (${TARGET}, DEBIAN)
LIBS += -Wl,-z,relro,-z,now
CFLAGS += -D_FORTIFY_SOURCE=2
CFLAGS += -Wformat -Werror=format-security
#CFLAGS += -fstack-protector-strong
endif

OBJS = src/main.o src/pocket.o src/calc.o src/hersheyfont.o src/postprocessor.o src/setup.o src/dxf.o src/font.o src/texture.o src/os-hacks.o

# GTK+2.0 and LUA5.1
PKGS   ?= gtk+-2.0 gtkglext-x11-1.0 gtksourceview-2.0 lua5.1
CFLAGS += "-DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"
CFLAGS += "-DGSEAL_ENABLE"

# LIBG3D
#PKGS += libg3d
#CFLAGS += "-DUSE_G3D"

# VNC-1.0
#PKGS += gtk-vnc-1.0
#CFLAGS += "-DUSE_VNC"

# WEBKIT-1.0
#PKGS += webkit-1.0 
#CFLAGS += "-DUSE_WEBKIT"

ALL_LIBS = $(LIBS) $(PKGS:%=`$(PKG_CONFIG) % --libs`)
CFLAGS += $(PKGS:%=`$(PKG_CONFIG) % --cflags`)

LANGS ?= $(shell ls po/*.po | sed "s|^po/||g" | cut -d"." -f1)
LANG_MO = $(foreach PO,$(LANGS), share/locale/$(PO)/LC_MESSAGES/${PROGRAM}.mo)
PO_MERGE = $(foreach PO,$(LANGS),msgmerge --no-fuzzy-matching --width=512 --backup=none --previous --update po/$(PO).po lang.pot\;)
PO_SED = $(foreach PO,$(LANGS),sed -i \'s/^.~ //g\' po/$(PO).po\;)

