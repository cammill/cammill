#!/bin/sh
#
#

APP_PATH="$1"
BINARY="$2"

otool -L ${APP_PATH}/Contents/MacOS/${BINARY} | awk '{print $1}' | grep "\.dylib$" | grep -v "^/usr/lib/" | grep -v "^/System/" | while read LIB
do
#	echo "$LIB"
	LIBNAME="`echo "$LIB" | sed "s|.*/||g"`"
	cp -v $LIB ${APP_PATH}/Contents/MacOS/lib
	install_name_tool -change "$LIB" @executable_path/lib/$LIBNAME ${APP_PATH}/Contents/MacOS/${BINARY}
done
N=0
while test "$N" -lt "30"
do
#	echo $N
	ls ${APP_PATH}/Contents/MacOS/lib/*.dylib | while read LIBFILE
	do
#		echo "## $LIBFILE ##"
		chmod +w $LIBFILE
		otool -L $LIBFILE | awk '{print $1}' | grep "\.dylib$" | grep -v "@executable_path" | grep "^/opt/" | grep -v "^/System/" | while read LIB
		do
#			echo "	#### $LIB ####"
			LIBNAME="`echo "$LIB" | sed "s|.*/||g"`"
			if ! test -e ${APP_PATH}/Contents/MacOS/lib/$LIBNAME
			then
				cp -v $LIB ${APP_PATH}/Contents/MacOS/lib/$LIBNAME
			fi
			install_name_tool -change "$LIB" @executable_path/lib/$LIBNAME $LIBFILE
		done
	done
	N="`expr $N + 1`"
done
