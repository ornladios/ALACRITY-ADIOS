#!/bin/bash
source /home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting.sh
LOG1=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/decode_bmap.log
LOG2=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/debmap.log
LOG3=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/measure_decode_comp.log
LOG4=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/inpect_decode_b.log
#go through every selectivity 
SEL=5   #5 selectivities
iter=10
##############NORMAL ALACRITY -- BITMAP CONSTRUCTION AFTER PFORDELTA DECODE #####################
#echo "4 var" >> ${LOG1}
#nval=4  #4 variables 
#for ((i=0; i<${SEL}; i++ )) ; do
#	
#	for ((k=0;k<${iter}; k++ )); do
#		$MALCBIN $nval ${ALCDATADIR} ${WHERES4VAR[i]}  >> ${LOG1} #--base-dir=$ALCDATADIR $FWHERE
#	done
#	echo >> ${LOG1}
#done
#
#echo "3 var" >> ${LOG1}
#nval=3  #3 variables 
#for ((i=0; i<${SEL}; i++ )) ; do
#	
#	for ((k=0;k<${iter}; k++ )); do
#		$MALCBIN $nval ${ALCDATADIR} ${WHERES3VAR[i]}  >> ${LOG1} #--base-dir=$ALCDATADIR $FWHERE
#	done	
#	echo >> ${LOG1}
#done
#
#echo "2 var" >> ${LOG1}
#nval=2  #2 variables 
#for ((i=0; i<${SEL}; i++ )) ; do
#	for ((k=0;k<${iter}; k++ )); do
#		$MALCBIN $nval ${ALCDATADIR} ${WHERES2VAR[i]}  >> ${LOG1} #--base-dir=$ALCDATADIR $FWHERE
#	done	
#	echo >> ${LOG1}
#done
#
#
###############SPEEDUP ALACRITY -- BITMAP CONSTRUCTION WHILE PFORDELTA DECODING #####################
#
#echo "4 var" >> ${LOG2}
#nval=4  #4 variables 
#for ((i=0; i<${SEL}; i++ )) ; do
#	
#	for ((k=0;k<${iter}; k++ )); do
#		$NEWMALCBIN $nval ${ALCDATADIR} ${WHERES4VAR[i]}  >> ${LOG2} #--base-dir=$ALCDATADIR $FWHERE
#	done
#	echo >> ${LOG2}
#done
#
#echo "3 var" >> ${LOG2}
#nval=3  #3 variables 
#for ((i=0; i<${SEL}; i++ )) ; do
#	
#	for ((k=0;k<${iter}; k++ )); do
#		$NEWMALCBIN $nval ${ALCDATADIR} ${WHERES3VAR[i]}  >> ${LOG2} #--base-dir=$ALCDATADIR $FWHERE
#	done	
#	echo >> ${LOG2}
#done
#
#echo "2 var" >> ${LOG2}
#nval=2  #2 variables 
#for ((i=0; i<${SEL}; i++ )) ; do
#	for ((k=0;k<${iter}; k++ )); do
#		$NEWMALCBIN $nval ${ALCDATADIR} ${WHERES2VAR[i]}  >> ${LOG2} #--base-dir=$ALCDATADIR $FWHERE
#	done	
#	echo >> ${LOG2}
#done

##############SPEEDUP ALACRITY -- BITMAP CONSTRUCTION WHILE PFORDELTA DECODING AND NO EXCEPTION PATH FOR B=9 #####################

echo "4 var" >> ${LOG3}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=0;k<${iter}; k++ )); do
		$NEWMALCBIN $nval ${ALCDATADIR} ${WHERES4VAR[i]}  >> ${LOG3} #--base-dir=$ALCDATADIR $FWHERE
	done
	echo >> ${LOG3}
done

echo "3 var" >> ${LOG3}
nval=3  #3 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=0;k<${iter}; k++ )); do
		$NEWMALCBIN $nval ${ALCDATADIR} ${WHERES3VAR[i]}  >> ${LOG3} #--base-dir=$ALCDATADIR $FWHERE
	done	
	echo >> ${LOG3}
done

echo "2 var" >> ${LOG3}
nval=2  #2 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	for ((k=0;k<${iter}; k++ )); do
		$NEWMALCBIN $nval ${ALCDATADIR} ${WHERES2VAR[i]}  >> ${LOG3} #--base-dir=$ALCDATADIR $FWHERE
	done	
	echo >> ${LOG3}
done



#########################INPSECT value of b used during decode#######################################
itr=1
echo "4 var" >> ${LOG4}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=0;k<${iter}; k++ )); do
		$INSPMALCBIN $nval ${ALCDATADIR} ${WHERES4VAR[i]}  >> ${LOG4} #--base-dir=$ALCDATADIR $FWHERE
	done
	echo >> ${LOG4}
done

echo "3 var" >> ${LOG4}
nval=3  #3 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=0;k<${iter}; k++ )); do
		$INSPMALCBIN $nval ${ALCDATADIR} ${WHERES3VAR[i]}  >> ${LOG4} #--base-dir=$ALCDATADIR $FWHERE
	done	
	echo >> ${LOG4}
done

echo "2 var" >> ${LOG4}
nval=2  #2 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	for ((k=0;k<${iter}; k++ )); do
		$INSPMALCBIN $nval ${ALCDATADIR} ${WHERES2VAR[i]}  >> ${LOG4} #--base-dir=$ALCDATADIR $FWHERE
	done	
	echo >> ${LOG4}
done
