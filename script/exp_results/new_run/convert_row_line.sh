#!/bin/bash

OUT="tmp"


for ((I=1;I<=15;I++)); do 

   for F in "naive_avg.csv" "simple_avg.csv"  "epfd_avg.csv" "rpfd_avg.csv" "erpfd_avg.csv" ; do
	   sed -n "${I}~${I}P" $F |  sed 's/,/\n/g' > $OUT/$F.$I
   done
done


K=1
for ((I=1;I<=5;I++)); do 
	for F in "naive_avg.csv" "simple_avg.csv"  "epfd_avg.csv" "rpfd_avg.csv" "erpfd_avg.csv" ; do
	      JF[$K]=$F.$I
	      K=$((K+1))

	done
done
echo ${JF[@]}
#paste ${JF[@]} -d ","
#4var

#	paste naive_avg.csv.1 simple_avg.csv.1  epfd_avg.csv.1  rpfd_avg.csv.1 erpfd_avg.csv.1 naive_avg.csv.2 simple_avg.csv.2  epfd_avg.csv.2 rpfd_avg.csv.2 erpfd_avg.csv.2 naive_avg.csv.3 simple_avg.csv.3  epfd_avg.csv.3  rpfd_avg.csv.3 erpfd_avg.csv.3 naive_avg.csv.4 simple_avg.csv.4 epfd_avg.csv.4  rpfd_avg.csv.4 erpfd_avg.csv.5 naive_avg.csv.1 simple_avg.csv.1  epfd_avg.csv.1  rpfd_avg.csv.1 erpfd_avg.csv.1


