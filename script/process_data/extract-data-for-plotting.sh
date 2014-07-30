#!/bin/bash
echo "query,method,bmap,decode,io,intersection,ridrecovery,total" >> data.csv
B=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/skipping_query


   for m in naive simple merged epfd rpfd ; do
       log=${B}/${m}.log.out
       if [ $m == embed ] || [ $m == epfd ] ; then
       #if [ $m == embed ] ; then
	       rep=20
	       echo "$m : $rep"
       fi
        cat ${log} | sed 1d | awk -v repNum=$rep '
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
	   if (line % repNum == 0) {
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
	}' > ${log}".avg"


        cat ${log}".avg" | awk -F"," -v mi=${m} '{print "query" NR "," mi "," $1 "," $2 "," $3 "," $4 "," $5 "," $6}' >> data.csv

   done

