#!/bin/bash
source /ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting_sith.sh
LOG=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/alac.log

#go through every selectivity 
SEL=5   #5 selectivities

echo "4 var" >> ${LOG}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<11; k++ )); do
		BASEDIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alacrity_2GB_index_cii"${k}"/"
		$MALCBIN $nval ${BASEDIR} ${WHERES4VAR[i]}   >> ${LOG} #--base-dir=$BASEDIR $FWHERE
	done
	echo >> ${LOG}
done

echo "3 var" >> ${LOG}
nval=3  #3 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<11; k++ )); do
		BASEDIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alacrity_2GB_index_cii"${k}"/"
		$MALCBIN $nval ${BASEDIR} ${WHERES3VAR[i]}   >> ${LOG} #--base-dir=$BASEDIR $FWHERE
	done	
	echo >> ${LOG}
done

echo "2 var" >> ${LOG}
nval=2  #2 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<11; k++ )); do
		BASEDIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alacrity_2GB_index_cii"${k}"/"
		$MALCBIN $nval ${BASEDIR} ${WHERES2VAR[i]}   >> ${LOG} #--base-dir=$BASEDIR $FWHERE
	done	
	echo >> ${LOG}
done

echo "1 var" >> ${LOG}
nval=1  #1 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<11; k++ )); do
		BASEDIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alacrity_2GB_index_cii"${k}"/"
		$MALCBIN $nval ${BASEDIR} ${WHERES1VAR[i]}   >> ${LOG} #--base-dir=$BASEDIR $FWHERE
	done	
	echo >> ${LOG}
done
#VARS=("uvel" "vvel" "temp" "wvel" )

#LBS_0=(50 13 6.8E2 45)  # enumerate the lower bounds for all variables
#HBS_0=(69 23 7.4E2 48)

#LBS_1=(50 -8.9 6.9E2 -10)  # enumerate the lower bounds for all variables
#HBS_1=(64 19 7.5E2 -11)

#LBS_2=(42 3 7.5E2 10)  # enumerate the lower bounds for all variables
#HBS_2=(80 5 1.4E3 14)

#LBS_3=(21 -15 7.2E2 6.3)  # enumerate the lower bounds for all variables
#HBS_3=(95 27 7.5E2 15)

#LBS_4=(53 -5.7 6.8E2 3.7)  # enumerate the lower bounds for all variables
#HBS_4=(93 4.2 1.9E3 5.7)

#FWHERE="" #fastbit where clause '--var=temp --start-range=419 --end-range=500 -var=vv --start-range=33 --end-range=66'
#AWHERE="" #alacrity where clause 'temp 400 440 uvel 10 300 vvel 50 300 wvel -166 0'


	#nval=${#VARS[@]} #number of variables
	#assemble where clause for a number of variables
	#for ((j=0; j<$nval; j++)) ; do
		#echo '${LBS_'$i'[${j}]}' is to resemble the expression that accesses the element in array : ${LBS_1[2]}
		#eval to get value for string ${LBS_1[2]}, result will be like 3
		#"$(3)" will get 3 and assign 3 to variable lb
	#	lb="$(eval echo '${LBS_'$i'[${j}]}')"
	#	hb="$(eval echo '${HBS_'$i'[${j}]}')"
	#	FWHERE=$FWHERE" "${VARS[j]}" "$lb" "$hb
	#	AWHERE=$AWHERE" "${VARS[j]}" "$lb" "$hb
	#done

	#$FBIN $nval ${FASTDATADIR} $FWHERE  
	#$MALCBIN $nval ${ALCDATADIR} $AWHERE 
	#clean
	#FWHERE=""
	#AWHERE=""
