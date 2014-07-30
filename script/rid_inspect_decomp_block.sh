#!/bin/bash
source /home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting.sh
LOG3=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/rid_inspect_decomp_block.log
SEL=5   #5 selectivities
echo "2 var" >> ${LOG3}
nval=2  #2 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	$RIDMALBIN $nval ${ALCDATADIR} ${WHERES2VAR[i]}  >> ${LOG3} #--base-dir=$ALCDATADIR $FWHERE
	echo >> ${LOG3}
done

echo "3 var" >> ${LOG3}
nval=3  #3 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	$RIDMALBIN $nval ${ALCDATADIR} ${WHERES3VAR[i]}  >> ${LOG3} #--base-dir=$ALCDATADIR $FWHERE
	echo >> ${LOG3}
done

echo "4 var" >> ${LOG3}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	$RIDMALBIN $nval ${ALCDATADIR} ${WHERES4VAR[i]}  >> ${LOG3} #--base-dir=$ALCDATADIR $FWHERE
	echo >> ${LOG3}
done
