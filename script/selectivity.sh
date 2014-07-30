#!/bin/bash

#compile selectivity.cpp : g++ -g -O2 selectivity.cpp -o selectivity

BIN=/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/selectivity
WORKDIR=/home/xzou2/sigmod_2012_2GB/
SEL=("0.001" "0.01" "0.1" "1" "10" )

VARS=("uvel" "vvel" "temp" "wvel" )

LBS_0=(50 13 6.8E2 45)  # enumerate the lower bounds for all variables
HBS_0=(69 23 7.4E2 48)

LBS_1=(50 -8.9 6.9E2 -10)  # enumerate the lower bounds for all variables
HBS_1=(64 19 7.5E2 -11)

LBS_2=(42 3 7.5E2 10)  # enumerate the lower bounds for all variables
HBS_2=(80 5 1.4E3 14)

LBS_3=(21 -15 7.2E2 6.3)  # enumerate the lower bounds for all variables
HBS_3=(95 27 7.5E2 15)

LBS_4=(53 -5.7 6.8E2 3.7)  # enumerate the lower bounds for all variables
HBS_4=(93 4.2 1.9E3 5.7)

WHERE="" #fastbit where clause '--var=temp --start-range=419 --end-range=500 -var=vv --start-range=33 --end-range=66'

echo "4 var"
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
		WHERE=$WHERE" "${VARS[j]}" "$lb" "$hb
	done

	$BIN $nval ${WORKDIR} $WHERE  
	#clean
	WHERE=""
	echo 
done


echo "3 var"
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

WHERE="" #fastbit where clause '--var=temp --start-range=419 --end-range=500 -var=vv --start-range=33 --end-range=66'

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
		WHERE=$WHERE" "${VARS[j]}" "$lb" "$hb
	done

	$BIN $nval ${WORKDIR} $WHERE  
	#clean
	WHERE=""
	echo 
done

echo "2 var"

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

WHERE="" #alacrity where clause 'temp 400 440 uvel 10 300 vvel 50 300 wvel -166 0'

	#go through every selectivity 
for ((i=0; i<${#SEL[@]}; i++ )) ; do
	
	nval=${#VARS[@]} #number of variables
	#assemble where clause for a number of variables
	for ((j=0; j<$nval; j++)) ; do
		lb="$(eval echo '${LBS_'$i'[${j}]}')"
		hb="$(eval echo '${HBS_'$i'[${j}]}')"
		WHERE=$WHERE" "${VARS[j]}" "$lb" "$hb
	done

	$BIN $nval ${WORKDIR} $WHERE  
	#clean
	WHERE=""
done

