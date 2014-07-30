#!/bin/bash
#./build/bin/alac encode -p268435456E -x -edouble -s16 /tmp/work/xzou2/widow0-20130305/sigmod_2GB/temp.bin /lustre/widow2/scratch/xzou2/widow0-20130305/tmp_out/temp
#echo "$1 $2 $3 $4 $5 "
CMD="${1} encode -p${2}E -${3} -edouble -s16 ${4} ${5}"
echo "$CMD"
$CMD

