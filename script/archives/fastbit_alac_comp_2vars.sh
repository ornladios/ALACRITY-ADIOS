#!/bin/bash

if [ $# -ne 2 ] ; then
   echo "usage : ./fastbit_alac_comp.sh ./fastbit_query ./build/bin/multiquery "
   #/home/xzou2/fastbit/data  /home/xzou2/alac/data" 
   exit 0
fi

export FASTBIT_PATH=/ccs/home/xzou2/build/fastbit-1.3.5
export LD_LIBRARY_PATH=${FASTBIT_PATH}/lib:${LD_LIBRARY_PATH}



LOGS=("2vars_fastbit_results.csv" "2vars_alac_results.csv")

for ((K=0; K<${#LOGS[@]}; K++ )) ; do
   if [ -f ${LOGS[K]} ] ; then
     #echo "delete log file ${LOGS[K]} and create it again" 
     rm -rf ${LOGS[K]}
     echo "" > ${LOGS[K]}
   fi
done

#During to bash upgrade on my OS, the array declaration (ARRY=(0 3 4))is not working anymore. 
FBIN=$1
MALCBIN=$2
#BASEDIR=$3
#DATADIR=$4

# ./fastbit_query --base-dir=/home/xzou2/fastbit/indexing/tempIndex/FastBit/P2/S3D/ --var=temp --start-range=419 --end-range=500 -var=vv --start-range 
#Note: --base-dir should point to directory that has -part.txt file

#./build/bin/multiquery 4 ~/data/sigmod/ temp 400 440 uvel 10 300 vvel 50 300 wvel -166 0


#SELECTIVITY ARRAY, the value in this array is not usefully, just sign of percentages
#the indice of each element is used for looking for array which has 'LBS_x' and 'HBS_x' forms.
#the indice value will replace the 'x' part in 'LBS_x' and 'HBS_x' form
#unit in pecentage

#SEL=("0.1" "0.2" "0.5" "1.0");
SEL=("0.001" "0.01" ) #"0.1" "1" "10" )

#VARS=("temp" "wvel" "vvel" "uvel" "potential")
#VARS=("temp" "vv")

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

# the results are to generate figure which has selectivity as x-axis,  and each figure is for fixed number of variables
#for ((v=0; v<${#VARS[@]}; v++ )) ; do   

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
		#FWHERE=$FWHERE" --VAR="${VARS[j]}"  --start-range="$lb" --end-rang="$hb
		AWHERE=$AWHERE" "${VARS[j]}" "$lb" "$hb
	done

	WORKDIR=/lustre/widow0/scratch/xzou2/tmp/
	#run 10  times
		for ((k=9;k<11; k++ )); do
			rm -rf ${WORKDIR}*
			BASEDIR="/lustre/widow0/scratch/xzou2/fastbit"${k}"/uvelindex/FastBit/P2/S3D/*"
			cp -R ${BASEDIR} ${WORKDIR}
			$FBIN $nval ${WORKDIR} $FWHERE   >> ${LOGS[0]} #--base-dir=$BASEDIR $FWHERE
			
			rm -rf ${WORKDIR}*
			DATADIR="/lustre/widow0/scratch/xzou2/alacrity_index"${k}"/*"
			cp -R ${DATADIR} ${WORKDIR}
			$MALCBIN $nval ${WORKDIR} $AWHERE  >> ${LOGS[1]}
		done
	
	echo "" >> ${LOGS[0]}
	echo "" >> ${LOGS[1]}
	
	#DATADIR=/home/xzou2/data/alac_4part_2GB/
	#$MALCBIN $nval ${DATADIR} $AWHERE

	#clean
	FWHERE=""
	AWHERE=""
done

#	echo 

#done
