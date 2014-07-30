#!/bin/bash
#THIS script is used for summarize the md5sum value of each query results for variant of ALACRITY
source /home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting.sh
LOG=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/crctness.log
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd_rle/
SEL=5   #5 selectivities
COMMAND=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_hybrid_decode


echo "4 var" >> ${LOG}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	$COMMAND $nval ${DATADIR} ${WHERES4VAR[i]}  >> ${LOG} #--base-dir=$DATADIR $FWHERE
	echo >> ${LOG}
done

echo "3 var" >> ${LOG}
nval=3  #3 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	$COMMAND $nval ${DATADIR} ${WHERES3VAR[i]}  >> ${LOG} #--base-dir=$DATADIR $FWHERE
	echo >> ${LOG}
done


echo "2 var" >> ${LOG}
nval=2  #2 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	$COMMAND $nval ${DATADIR} ${WHERES2VAR[i]}  >> ${LOG} #--base-dir=$DATADIR $FWHERE
	echo >> ${LOG}
done

