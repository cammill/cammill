
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
DISTRIBUTION     ?= $(shell sw_vers -productName | sed "s|  *||g")
LIBS             ?= -framework OpenGL -framework GLUT -lm -lpthread -lstdc++ -lc
PKGS             ?= gtk+-2.0 gtkglext-1.0 gtksourceview-2.0 lua
INSTALL_PATH     ?= OSX/CAMmill.app/MacOS
STRIP_CMD        ?= strip


dmg_back = dmg-background.png
dmg_width = 522
dmg_height = 361
dmg_topleft_x = 200
dmg_topleft_y = 200
dmg_bottomright_x = $(shell expr ${dmg_topleft_x} + ${dmg_width})
dmg_bottomright_y = $(shell expr ${dmg_topleft_y} + ${dmg_height})


peinstall_osx:
	mkdir -p ${PKG_INSTALL_PATH}/${INSTALL_PATH}/
	mkdir -p ${PKG_INSTALL_PATH}/${INSTALL_PATH}/lib
	mkdir -p ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Resources/

	cp -a ${BINARY} ${PKG_INSTALL_PATH}/${INSTALL_PATH}/

	@echo "generate start-script"
	@echo "#!/bin/bash" > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${PROGRAM}
	@echo "cd \"\`dirname \$$0\`/bin\"" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${PROGRAM}
	@echo "./${PROGRAM}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${PROGRAM}
	chmod 755 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${PROGRAM}

	@echo "#!/bin/bash" > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${PROGNAME}
	@echo "cd \"\`dirname \$$0\`/bin\"" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${PROGNAME}
	@echo "./${PROGRAM}" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${PROGNAME}
	chmod 755 ${PKG_INSTALL_PATH}/${INSTALL_PATH}/${PROGNAME}
	
	@echo "generate Info.plist"
	@echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "<plist version=\"1.0\">" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "<dict>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>CFBundleGetInfoString</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <string>${PROGNAME}</string>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>CFBundleExecutable</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <string>${PROGRAM}</string>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>CFBundleIdentifier</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <string>org.multixmedia.www</string>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>CFBundleName</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <string>${PROGNAME}</string>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>CFBundleIconFile</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <string>${PROGRAM}.icns</string>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>CFBundleShortVersionString</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <string>${VERSION}</string>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>CFBundleInfoDictionaryVersion</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <string>6.0</string>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>CFBundlePackageType</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <string>APPL</string>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>IFMajorVersion</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <integer>0</integer>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <key>IFMinorVersion</key>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "  <integer>1</integer>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "</dict>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist
	@echo "</plist>" >> ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Info.plist

	cp share/cammill/icons/icon.icns ${PKG_INSTALL_PATH}/${INSTALL_PATH}/../Contents/Resources/${PROGRAM}.icns

	cp ./utils/dmg-background.png dmg-background.png

	umount /Volumes/${PROGNAME} 2>/dev/null || true
	rm -rf ${PROGNAME}.temp.dmg ${PROGNAME}.dmg
	hdiutil create -srcfolder "${PKG_INSTALL_PATH}/${INSTALL_PATH}/../" -volname "${PROGNAME}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size 50M ${PROGNAME}.temp.dmg

	hdiutil attach -readwrite -noverify -noautoopen "${PROGNAME}.temp.dmg" | egrep '^/dev/' | sed 1q | awk '{print $$1}' > device.osx

	mkdir -p /Volumes/${PROGNAME}/.background
	cp share/cammill/icons/logo.png /Volumes/${PROGNAME}/.background/icon.png
	cp utils/dmg-background.png /Volumes/${PROGNAME}/.background/dmg-background.png

	ln -s /Applications /Volumes/${PROGNAME}/Applications

	@echo "generate osascript"
	@echo "   tell application \"Finder\"" > script.osa
	@echo "     tell disk \"${PROGNAME}\"" >> script.osa
	@echo "           open" >> script.osa
	@echo "           set current view of container window to icon view" >> script.osa
	@echo "           set toolbar visible of container window to false" >> script.osa
	@echo "           set statusbar visible of container window to false" >> script.osa
	@echo "           set the bounds of container window to {${dmg_topleft_x}, ${dmg_topleft_y}, ${dmg_bottomright_x}, ${dmg_bottomright_y}}" >> script.osa
	@echo "           set theViewOptions to the icon view options of container window" >> script.osa
	@echo "           set arrangement of theViewOptions to not arranged" >> script.osa
	@echo "           set icon size of theViewOptions to 104" >> script.osa
	@echo "           set background picture of theViewOptions to file \".background:${dmg_back}\"" >> script.osa
	@echo "           set position of item \"${PROGNAME}\" of container window to {120, 180}" >> script.osa
	@echo "           set position of item \"Applications\" of container window to {400, 180}" >> script.osa
	@echo "           close" >> script.osa
	@echo "           open" >> script.osa
	@echo "           update without registering applications" >> script.osa
	@echo "           delay 5" >> script.osa
	@echo "           eject" >> script.osa
	@echo "     end tell" >> script.osa
	@echo "   end tell" >> script.osa
	cat script.osa | osascript

	sync
	hdiutil detach $(shell cat device.osx) 2>/dev/null || true
	hdiutil convert "${PROGNAME}.temp.dmg" -format UDZO -imagekey zlib-level=9 -o "${PROGNAME}"
	rm -f ${PROGNAME}.temp.dmg

	rm -rf dmg-background.png

	#rm -rf script.osa device.osx



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

package: pinstall peinstall_osx
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
