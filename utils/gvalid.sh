#!/bin/sh
#
#

if ! cat "$1" | grep -v " *(.*)$" | grep -s -q "^G[0-9]*[0-9]$"
then
	echo "ERROR(no G codes found)"
	exit 1
fi
if ! cat "$1" | grep -v " *(.*)$" | grep -s -q "^M[0-9]*[0-9]$"
then
	echo "ERROR(no M codes found)"
	exit 1
fi
if ! cat "$1" | grep -v " *(.*)$" | grep -s -q "^F[0-9]*[0-9]$"
then
	echo "ERROR(no F codes found)"
	exit 1
fi
if cat "$1" | grep -v " *(.*)$" | grep -s -q -i "nan"
then
	echo "ERROR('nan' found)"
	exit 1
fi
if cat "$1" | grep -v " *(.*)$" | grep -s -q -i ","
then
	echo "ERROR(',' found)"
	exit 1
fi
(for CMD in `cat "$1" | grep -v " *(.*)$"`
do
	echo $CMD | grep -v "^G[0-9]*[0-9]$\|^F[0-9][0-9]*$\|^[XYZIJ]-*[0-9][0-9\.]*$\|^[MT][0-9]*[1-9]$\|^S[0-9][0-9]*$"
done) | grep "." && echo "ERROR(unknown codes found)" && exit 1 

echo "PASSED"
exit 0
