
#TARGETS: DEBIAN, FREEBSD, MINGW32, NETBSD, OSX, OPENBSD, SUSE
TARGET ?= NONE

all: binary

include targets/*.mk
include defaults.mk

binary: info ${BINARY}

info:
	@echo ""
	@echo "Info:"
	@echo "	TARGET          ${TARGET}"
	@echo "	SYSTEM          ${SYSTEM}"
	@echo "	MACHINE         ${MACHINE}"
	@echo "	RELEASE         ${RELEASE}"
	@echo "	DISTRIBUTION    ${DISTRIBUTION}"
	@echo "	BINARY          ${BINARY}"
	@echo "	VERSION         ${VERSION}"
	@echo "	PKGS:           ${PKGS}"
	@echo "	INSTALL_PATH    ${INSTALL_PATH}"
	@echo ""

updatepo:
	xgettext -k_ src/*.c -o lang.pot
	@echo ${PO_MERGE}
	@echo ${PO_MERGE} | sh
	@echo ${PO_SED}
	@echo ${PO_SED} | sh
	rm -rf lang.pot

lang:
	@echo ${PO_MKDIR}
	@echo ${PO_MKDIR} | sh
	@echo ${PO_MSGFMT}
	@echo ${PO_MSGFMT} | sh

${BINARY}: ${OBJS}
	mkdir -p bin
	$(COMP) -o ${BINARY} ${OBJS} ${ALL_LIBS} ${INCLUDES} ${CFLAGS}

%.o: %.c
		$(COMP) -c $(CFLAGS) ${INCLUDES} $< -o $@

clean:
	rm -rf ${OBJS}
	rm -rf ${BINARY}

install: ${BINARY}
	strip --remove-section=.comment --remove-section=.note ${BINARY}
	install -m 0755 -d ${INSTALL_PATH}
	install -m 0755 -d ${INSTALL_PATH}/bin
	install -m 0755 -d ${INSTALL_PATH}/lib/cammill
	install -m 0755 -d ${INSTALL_PATH}/lib/cammill/posts
	install -m 0755 -d ${INSTALL_PATH}/share/cammill
	install -m 0755 -d ${INSTALL_PATH}/share/cammill/icons
	install -m 0755 -d ${INSTALL_PATH}/share/cammill/textures
	install -m 0755 -d ${INSTALL_PATH}/share/cammill/fonts
	install -m 0755 -d ${INSTALL_PATH}/share/doc/cammill
	install -m 0755 -d ${INSTALL_PATH}/share/doc/cammill/examples
	install -m 0755 ${BINARY} ${INSTALL_PATH}/${BINARY}
	install -m 0644 lib/cammill/posts/*.scpost ${INSTALL_PATH}/lib/cammill/posts/
	install -m 0644 lib/cammill/*.lua ${INSTALL_PATH}/lib/cammill/
	install -m 0644 share/cammill/*.tbl ${INSTALL_PATH}/share/cammill/
	install -m 0644 share/cammill/icons/*.png ${INSTALL_PATH}/share/cammill/icons/
	install -m 0644 share/cammill/icons/*.icns ${INSTALL_PATH}/share/cammill/icons/
	install -m 0644 share/cammill/icons/*.svg ${INSTALL_PATH}/share/cammill/icons/
	install -m 0644 share/cammill/icons/*.ico ${INSTALL_PATH}/share/cammill/icons/
	install -m 0644 share/cammill/textures/*.bmp ${INSTALL_PATH}/share/cammill/textures/
	install -m 0644 share/cammill/fonts/*.jhf ${INSTALL_PATH}/share/cammill/fonts/
	install -m 0644 share/doc/cammill/examples/*.dxf ${INSTALL_PATH}/share/doc/cammill/examples/

pinstall: ${BINARY}
	strip --remove-section=.comment --remove-section=.note ${BINARY}
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/bin
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib/${PROGRAM}
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib/${PROGRAM}/posts
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/textures
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/fonts
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}
	install -m 0755 -d ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/examples
	install -m 0755 ${BINARY} ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${BINARY}
	install -m 0644 lib/${PROGRAM}/posts/*.scpost ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib/${PROGRAM}/posts/
	install -m 0644 lib/${PROGRAM}/*.lua ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib/${PROGRAM}/
	install -m 0644 share/${PROGRAM}/*.tbl ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/
	install -m 0644 share/${PROGRAM}/icons/*.png ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons/
	install -m 0644 share/${PROGRAM}/icons/*.icns ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons/
	install -m 0644 share/${PROGRAM}/icons/*.svg ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons/
	install -m 0644 share/${PROGRAM}/icons/*.ico ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/icons/
	install -m 0644 share/${PROGRAM}/textures/*.bmp ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/textures/
	install -m 0644 share/${PROGRAM}/fonts/*.jhf ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/${PROGRAM}/fonts/
	install -m 0644 share/doc/${PROGRAM}/examples/*.dxf ${PKG_INSTALL_PATH}/${INSTALL_PATH}/share/doc/${PROGRAM}/examples/

doc: ${BINARY}
	test -e ${PROGRAM}.github.io && sh utils/help2html.sh ./${BINARY} > ${PROGRAM}.github.io/de/cmdline.html || true
	test -e ${PROGRAM}.wiki/ && sh utils/help2md.sh ./${BINARY} > ${PROGRAM}.wiki/Commandline-Arguments.md || true

check:
	cppcheck --error-exitcode=1 --quiet src/

pull:
	git pull --rebase
	test -e ${PROGRAM}.github.io && (cd ${PROGRAM}.github.io/ ; git pull --rebase) || true
	test -e ${PROGRAM}.wiki/ && (cd ${PROGRAM}.wiki/ ; git pull --rebase) || true




