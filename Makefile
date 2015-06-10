
HERSHEY_FONTS_DIR = ./
COMP = clang
PROGRAM = cammill
INSTALL_PATH = /opt/${PROGRAM}

LIBS += -lGL -lglut -lGLU -lX11 -lm -lpthread -lstdc++ -lXext -ldl -lXi -lxcb -lXau -lXdmcp -lgcc -lc
CFLAGS += -I./
CFLAGS += "-DHERSHEY_FONTS_DIR=\"./\""
CFLAGS += -ggdb -Wno-int-to-void-pointer-cast -Wall -Wno-unknown-pragmas -O3

OBJS = main.o pocket.o calc.o hersheyfont.o postprocessor.o setup.o dxf.o font.o texture.o

# GTK+2.0
PKGS += gtk+-2.0
PKGS += gtkglext-x11-1.0
PKGS += gtksourceview-2.0
CFLAGS += "-DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"
CFLAGS += "-DGSEAL_ENABLE"

# LUA-5.1
PKGS += lua5.1

# LIBG3D
PKGS += libg3d
CFLAGS += "-DUSE_G3D"

# VNC-1.0
#PKGS += gtk-vnc-1.0
#CFLAGS += "-DUSE_VNC"

# WEBKIT-1.0
#PKGS += webkit-1.0 
#CFLAGS += "-DUSE_WEBKIT"

LIBS += $(PKGS:%=`pkg-config % --libs`)
CFLAGS += $(PKGS:%=`pkg-config % --cflags`)

LANGS += de
LANGS += it
LANGS += fr

PO_MKDIR = mkdir -p $(foreach PO,$(LANGS),intl/$(PO)_$(shell echo $(PO) | tr "a-z" "A-Z").UTF-8/LC_MESSAGES)
PO_MSGFMT = $(foreach PO,$(LANGS),msgfmt po/$(PO).po -o intl/$(PO)_$(shell echo $(PO) | tr "a-z" "A-Z").UTF-8/LC_MESSAGES/${PROGRAM}.mo\;)


all: lang ${PROGRAM}

lang:
	@echo ${PO_MKDIR}
	@echo ${PO_MKDIR} | sh
	@echo ${PO_MSGFMT}
	@echo ${PO_MSGFMT} | sh

${PROGRAM}: ${OBJS}
	$(COMP) -o ${PROGRAM} ${OBJS} ${LIBS} ${INCLUDES} ${CFLAGS}

%.o: %.c
	$(COMP) -c $(CFLAGS) ${INCLUDES} $< -o $@

gprof:
	gcc -pg -o ${PROGRAM} ${OBJS} ${LIBS} ${INCLUDES} ${CFLAGS}
	@echo "./${PROGRAM}"
	@echo "gprof ${PROGRAM} gmon.out"

clean:
	rm -rf ${OBJS}
	rm -rf ${PROGRAM}

install: ${PROGRAM}
	mkdir -p ${INSTALL_PATH}
	cp ${PROGRAM} ${INSTALL_PATH}/${PROGRAM}
	chmod 755 ${INSTALL_PATH}/${PROGRAM}
	mkdir -p ${INSTALL_PATH}/posts
	cp -a posts/* ${INSTALL_PATH}/posts
	mkdir -p ${INSTALL_PATH}/textures
	cp -a textures/* ${INSTALL_PATH}/textures
	mkdir -p ${INSTALL_PATH}/icons
	cp -a icons/* ${INSTALL_PATH}/icons
	mkdir -p ${INSTALL_PATH}/fonts
	cp -a fonts/* ${INSTALL_PATH}/fonts
	mkdir -p ${INSTALL_PATH}/doc
	cp -a doc/* ${INSTALL_PATH}/doc
	cp -a material.tbl postprocessor.lua tool.tbl cammill.dxf test.dxf ${INSTALL_PATH}/

