#/bin/bash

l=timing_encode_s3d.log
TMP1=$l".tmp"
TMP2=$l".tmp2"
F_OUT=$l".out"
 sed  '/^qsub.*$/d' $l> $TMP1
tr -d "\015" < $TMP1 > $TMP2
sed '/^\/ccs\/home.*$/d' $TMP2 > $TMP1
sed '/^Encoding.*$/d' $TMP1 > $TMP2
sed '/^$/d' $TMP2 > $F_OUT

rm -f $TMP1 $TMP2




