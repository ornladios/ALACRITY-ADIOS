#!/bin/bash

source /ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting.sh

VARS=("uvel" "vvel") # "temp" "wvel" )

#element is for temp wvel vvel uvel potential
LBS_0=(57 5.3)  # enumerate the lower bounds for all variables
HBS_0=(58 5.4)

LBS_1=(25 -3.0)  # enumerate the lower bounds for all variables
HBS_1=(54 -2.9)

LBS_2=(37 2.8)  # enumerate the lower bounds for all variables
HBS_2=(47 2.9)

LBS_3=(19 11)  # enumerate the lower bounds for all variables
HBS_3=(1.3E2 23)

LBS_4=(29 -18)  # enumerate the lower bounds for all variables
HBS_4=(59 -14)

FWHERE="" #fastbit where clause '--var=temp --start-range=419 --end-range=500 -var=vv --start-range=33 --end-range=66'
AWHERE="" #alacrity where clause 'temp 400 440 uvel 10 300 vvel 50 300 wvel -166 0'

	#go through every selectivity 
for ((i=0; i<${#SEL[@]}; i++ )) ; do
	
	nval=${#VARS[@]} #number of variables
	#assemble where clause for a number of variables
	for ((j=0; j<$nval; j++)) ; do
		#echo '${LBS_'$i'[${j}]}' is to resemble the expression that accesses the element in array : ${LBS_1[2]}
		#eval to get value for string ${LBS_1[2]}, result will be like 3
		#"$(3)" will get 3 and assign 3 to variable lb
		lb="$(eval echo '${LBS_'$i'[${j}]}')"
		hb="$(eval echo '${HBS_'$i'[${j}]}')"
		FWHERE=$FWHERE" "${VARS[j]}" "$lb" "$hb
	done

	$FBIN $nval ${WORKDIR} $FWHERE  
	#clean
	FWHERE=""
done

#	echo 

#done
