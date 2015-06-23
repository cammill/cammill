#!/bin/sh
#
#


CMD=$1

cat <<EOF
<HTML>
<HEAD>
<META charset="utf-8"/>
<STYLE>
	body {
		background-color: #000000;
		background-image: url('bg.png');
		color: #FFFFFF;
	}
</STYLE>
</HEAD>
<BODY>
<IMG src="logo.png">
<HR>
<H2>Commandline-Options</H2>
EOF

$CMD --help | while read LINE
do
	if echo "$LINE" | grep -s -q "^ *-"
	then
		P1="`echo $LINE | tr "\t" " " | sed "s|  *| |g" | cut -d" " -f1`"
		P2="`echo $LINE | tr "\t" " " | sed "s|  *| |g" | cut -d" " -f2`"
		P3="`echo $LINE | tr "\t" " " | sed "s|  *| |g" | cut -d" " -f3-`"
		echo "<BLOCKQUOTE><LI>$P1: $P3 ($P2)</BLOCKQUOTE>"
	else
		echo "$LINE<BR>"
	fi

done

cat <<EOF
<BR>
<BR>
<BR>
<BR>
<BR>
</BODY>
</HTML>
EOF

if $CMD --help | grep "^ *-[a-z]" | awk '{print $1}' | sort | uniq -d | grep -s -q "."
then
	echo "WARNING: double used arguments:" >&2
	for ARG in `$CMD --help | grep "^ *-[a-z]" | awk '{print $1}' | sort | uniq -d`
	do
		echo " $ARG" >&2
		$CMD --help | grep "^ *$ARG" | sed "s|^|  |g" >&2
	done
fi


