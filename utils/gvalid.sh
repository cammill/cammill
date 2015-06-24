#!/bin/sh
#
#

if ! cat "$1" | tr -d "\r" | grep -v " *(.*)$" | grep -s -q "\<G[0-9]*[0-9]\>"
then
	echo "ERROR(no G codes found)"
	exit 1
fi
if ! cat "$1" | tr -d "\r" | grep -v " *(.*)$" | grep -s -q "\<X[0-9]*[0-9\.]\>"
then
	echo "ERROR(no X cords found)"
	exit 1
fi
if ! cat "$1" | tr -d "\r" | grep -v " *(.*)$" | grep -s -q "\<Y[0-9]*[0-9\.]\>"
then
	echo "ERROR(no Y cords found)"
	exit 1
fi
if ! cat "$1" | tr -d "\r" | grep -v " *(.*)$" | grep -s -q "\<M[0-9]*[0-9]\>"
then
	echo "ERROR(no M codes found)"
	exit 1
fi
if ! cat "$1" | tr -d "\r" | grep -v " *(.*)$" | grep -s -q "\<F[0-9]*[0-9]\>"
then
	echo "ERROR(no F codes found)"
	exit 1
fi
if cat "$1" | tr -d "\r" | grep -v " *(.*)$" | grep -s -q -i "nan"
then
	echo "ERROR('nan' found)"
	exit 1
fi
if cat "$1" | tr -d "\r" | grep -v " *(.*)$" | grep -s -q -i ","
then
	echo "ERROR(',' found)"
	exit 1
fi
if cat "$1" | tr -d "\r" | grep -v " *(.*)$" | tr " " "\n" | grep -v "^G[0-9]*[0-9]$" | grep -v "^F[0-9][0-9]*$" | grep -v "^[XYZIJ]-*[0-9][0-9\.]*$" | grep -v "^[MT][0-9]*[1-9]$" | grep -v "^S[0-9][0-9]*$" | grep "."
then
	echo "ERROR(unknown codes found)"
	exit 1 
fi

echo "PASSED"
exit 0






