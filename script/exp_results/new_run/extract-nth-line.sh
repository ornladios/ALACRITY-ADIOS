#!/bin/bash

SEL=(0.001 0.01 0.1 1 10)
#VARS=(4 3 2)
VARS=(4 3)

#for F in "naive.log_p" "simple.log_p"  "epfd.log_p" "rpfd.log_p" "erpfd.log_p" ; do
for F in "mixindex.log_p" ; do
#for F in  "epfd.log_p"  ; do  
I=2 #first line has data 
	for ((V=0; V<${#VARS[@]}; V++ )) ; do 
		for ((S=0; S<${#SEL[@]}; S++ )) ; do 
			#sed -n "${I}~18p" $F >> $F"_p"
			sed -n "${I}~12p" $F >> $F"_p" # only for mixindex case, which has 4, and 3 varables results
			I=$((I+1))
		done
		I=$((I+1))
	done
done
