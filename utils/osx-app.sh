#!/bin/sh
#
#


EXE="$1"
INSTALL_PATH="$3"
UTILS_PATH="utils"


## work-around ##
cp $UTILS_PATH/dmg-background.png dmg-background.png

applicationName="$1"
version="$2"
dmg_back="dmg-background.png"
dmg_width=522
dmg_height=361
dmg_topleft_x=200
dmg_topleft_y=200
dmg_bottomright_x=`expr $dmg_topleft_x + $dmg_width`
dmg_bottomright_y=`expr $dmg_topleft_y + $dmg_height`

echo "## create structure ##"
mkdir -p ${INSTALL_PATH}/Contents/Resources
mkdir -p ${INSTALL_PATH}/Contents/MacOS/lib
cp "$EXE" ${INSTALL_PATH}/Contents/MacOS/${applicationName}.bin

echo "#!/bin/bash" > ${INSTALL_PATH}/Contents/MacOS/${applicationName}
echo "cd \`dirname \$0\`" >> ${INSTALL_PATH}/Contents/MacOS/${applicationName}
echo "./${applicationName}.bin" >> ${INSTALL_PATH}/Contents/MacOS/${applicationName}
chmod 755 ${INSTALL_PATH}/Contents/MacOS/${applicationName}

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > ${INSTALL_PATH}/Contents/Info.plist
echo "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" >> ${INSTALL_PATH}/Contents/Info.plist
echo "<plist version=\"1.0\">" >> ${INSTALL_PATH}/Contents/Info.plist
echo "<dict>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>CFBundleGetInfoString</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <string>${applicationName}</string>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>CFBundleExecutable</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <string>${applicationName}</string>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>CFBundleIdentifier</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <string>org.multixmedia.www</string>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>CFBundleName</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <string>${applicationName}</string>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>CFBundleIconFile</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <string>${applicationName}.icns</string>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>CFBundleShortVersionString</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <string>${version}</string>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>CFBundleInfoDictionaryVersion</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <string>6.0</string>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>CFBundlePackageType</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <string>APPL</string>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>IFMajorVersion</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <integer>0</integer>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <key>IFMinorVersion</key>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "  <integer>1</integer>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "</dict>" >> ${INSTALL_PATH}/Contents/Info.plist
echo "</plist>" >> ${INSTALL_PATH}/Contents/Info.plist


echo "## copy libs/depends ##"
otool -L ${INSTALL_PATH}/Contents/MacOS/${applicationName}.bin | awk '{print $1}' | grep "\.dylib$" | grep -v "^/usr/lib/" | grep -v "^/System/" | while read LIB
do
#	echo "$LIB"
	LIBNAME="`echo "$LIB" | sed "s|.*/||g"`"
	cp -v $LIB ${INSTALL_PATH}/Contents/MacOS/lib
	install_name_tool -change "$LIB" @executable_path/lib/$LIBNAME ${INSTALL_PATH}/Contents/MacOS/${applicationName}.bin
done
N=0
while test "$N" -lt "30"
do
#	echo $N
	ls ${INSTALL_PATH}/Contents/MacOS/lib/*.dylib | while read LIBFILE
	do
#		echo "## $LIBFILE ##"
		chmod +w $LIBFILE
		otool -L $LIBFILE | awk '{print $1}' | grep "\.dylib$" | grep -v "@executable_path" | grep "^/opt/" | grep -v "^/System/" | while read LIB
		do
#			echo "	#### $LIB ####"
			LIBNAME="`echo "$LIB" | sed "s|.*/||g"`"
			if ! test -e ${INSTALL_PATH}/Contents/MacOS/lib/$LIBNAME
			then
				cp -v $LIB ${INSTALL_PATH}/Contents/MacOS/lib/$LIBNAME
			fi
			install_name_tool -change "$LIB" @executable_path/lib/$LIBNAME $LIBFILE
		done
	done
	N="`expr $N + 1`"
done

echo "## create/copy icons ##"
cp $UTILS_PATH/../icons/icon.icns ${INSTALL_PATH}/Contents/Resources/${applicationName}.icns

echo "## creating dmg-image ##"
umount /Volumes/${applicationName} 2>/dev/null
rm -rf ${applicationName}.temp.dmg ${applicationName}.dmg
hdiutil create -srcfolder "${INSTALL_PATH}" -volname "${applicationName}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size 50M ${applicationName}.temp.dmg

echo "## mounting dmg-image ##"
device=$(hdiutil attach -readwrite -noverify -noautoopen "${applicationName}.temp.dmg" | egrep '^/dev/' | sed 1q | awk '{print $1}')


if ! test -d /Volumes/${applicationName}
then
	echo "ERROR - generating/mounting dmg-image: ${applicationName}.temp.dmg -> /Volumes/${applicationName}"
	rm -rf dmg-background.png
	exit 1
fi

echo "## copy background-image ##"
mkdir -p /Volumes/${applicationName}/.background
cp icons/logo.png /Volumes/${applicationName}/.background/icon.png
cp $UTILS_PATH/dmg-background.png /Volumes/${applicationName}/.background/dmg-background.png

echo "## configure dmg-image ##"
ln -s /Applications /Volumes/${applicationName}/Applications
echo '
   tell application "Finder"
     tell disk "'${applicationName}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {'${dmg_topleft_x}', '${dmg_topleft_y}', '${dmg_bottomright_x}', '${dmg_bottomright_y}'}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 104
           set background picture of theViewOptions to file ".background:'${dmg_back}'"
           set position of item "'${applicationName}'" of container window to {120, 180}
           set position of item "'Applications'" of container window to {400, 180}
           close
           open
           update without registering applications
           delay 5
           eject
     end tell
   end tell
' | osascript

echo "## unmounting/compressing dmg-image ##"
sync
hdiutil detach ${device} 2>/dev/null
hdiutil convert "${applicationName}.temp.dmg" -format UDZO -imagekey zlib-level=9 -o "${applicationName}"
rm -f ${applicationName}.temp.dmg

rm -rf dmg-background.png

