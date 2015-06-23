#!/bin/sh
#
#


CMD=$1


$CMD --help | while read LINE
do
	if echo "$LINE" | grep -s -q "^ *-"
	then
		if test "$START" = ""
		then
			START=1
cat <<EOF

| Argument      | Type  | Name     | Comment  |
| ------------- |-------|----------|---------|
EOF
		fi
		P1="`echo $LINE | tr "\t" " " | sed "s|  *| |g" | cut -d" " -f1`"
		P2="`echo $LINE | tr "\t" " " | sed "s|  *| |g" | cut -d" " -f2`"
		P3="`echo $LINE | tr "\t" " " | sed "s|  *| |g" | cut -d" " -f3- | sed "s| (.*||g"`"
		P4="`echo $LINE | tr "\t" " " | sed "s|  *| |g" | cut -d" " -f3- | sed "s|.* (||g" | sed "s|)||g"`"
		echo "| $P1 | $P2 |  $P3 | $P4 |"
	else
		echo "$LINE"
	fi

done

cat <<EOF

EOF

