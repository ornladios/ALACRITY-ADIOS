#/bin/bash

if [ $# -lt 1 ] ; then
	echo "input path to the log file"
	exit
fi
F=$1
l=$(basename $F)
TMP1=$l".tmp"
TMP2=$l".tmp2"
F_OUT=$l".out"
 sed  '/^qsub.*$/d' $F> $TMP1
tr -d "\015" < $TMP1 > $TMP2
sed '/^\/ccs\/home.*$/d' $TMP2 > $TMP1
sed '/^Encoding.*$/d' $TMP1 > $TMP2
sed '/^$/d' $TMP2 > $F_OUT

rm -f $TMP1 $TMP2




