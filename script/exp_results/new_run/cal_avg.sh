#!/bin/bash

TMP1="t1.log"
TMP2="t2.log"

NUM=20
#for log_p in `ls *.log_p` ; do
#	sed -i 's/^[0-9] var.*$//g' $log_p 
#	sed '/^$/d' $log_p > $log_p"_p"
#for log_p in "naive.log" "simple.log"  "epfd.log" "rpfd.log" "erpfd.log" ; do
#for log_p in "rpfd.log" ; do
#for log_p in "epfd.log" ; do
for log_p in "mixindex.log" ; do
	cat $log_p"_p_p" | awk -v avg=$NUM '
	BEGIN {
	   bmapbuild=0;
	   decode=0;
	   io=0;
	   inter=0;
	   brecover=0;
	   total=0;
	   line = 0;
	}
	{
	   bmapbuild += $1;
	   decode +=$2;
	   io +=$3;
	   inter +=$4;
	   brecover +=$5;
	   total +=$6;
	   line +=1 ;
	   if (line % avg == 0) {
		   print bmapbuild/line","decode/line","io/line","inter/line","brecover/line","total/line; 
	   	   bmapbuild=0;
		   decode=0;
		   io=0;
		   inter=0;
		   brecover=0;
		   total=0;
		   line = 0;
	   }
	}
	END {
	}
	' > $log_p"_avg"
done



