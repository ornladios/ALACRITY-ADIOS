#!/bin/bash


source /home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting.sh


VARS=("uvel" "vvel" "temp" )

#element is for temp wvel vvel uvel potential
LBS_0=(47 -2.8 7.5E2)  # enumerate the lower bounds for all variables
HBS_0=(48 3.2 7.6E2)

LBS_1=(22 -28 7.5E2)  # enumerate the lower bounds for all variables
HBS_1=(43 2.3E2 7.6E2)

LBS_2=(42 -4.9 7.3E2)  # enumerate the lower bounds for all variables
HBS_2=(54 4.4 7.4E2)

LBS_3=(33 -13 7.5E2)  # enumerate the lower bounds for all variables
HBS_3=(66 7.1 7.6E2)

LBS_4=(38 -8.6 7.5E2)  # enumerate the lower bounds for all variables
HBS_4=(49 20 7.6E2)

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
		AWHERE=$AWHERE" "${VARS[j]}" "$lb" "$hb
	done

	$FBIN $nval ${FASTDATADIR} $FWHERE  
	$MALCBIN $nval ${ALCDATADIR} $AWHERE 
	#clean
	FWHERE=""
	AWHERE=""
done

#	echo 

#done
