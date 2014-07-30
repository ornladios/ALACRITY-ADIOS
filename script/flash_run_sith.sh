#!/bin/bash



MERGE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_merge
EPFD=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_decode_bmap
RPFD=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_hybrid_decode
SEP=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery



function is_in_qsub() {
    hostname | grep -v login
}

function subtest() {

    if ! is_in_qsub; then
        rm -rf $TMP_WORK_DIR
        mkdir -p $TMP_WORK_DIR
        cp -r ${DATA_DIR} $TMP_WORK_DIR
        echo "Settling..."
        sleep 2s  # Wait for the copy to settle
        echo "qsub..."
	#echo "$COMMAND $nval $TMP_WORK_DIR $WHERECLS"
	$QSUB -x $TMP_SCRIPT $COMMAND $nval $TMP_WORK_DIR $WHERECLS >> $LOG_FILE
        echo "Settling..."
        sleep 1s # Settle again
    else
        echo "Must be run on a login node"
        exit 1     
    fi
}

if hostname | grep -q sith; then
  QSUB="qsub -I -lwalltime=1:0:0,nodes=1:ppn=32 "
elif hostname | grep -q lens; then
  QSUB="qsub -I -ACSC025EWK -q comp_mem -lwalltime=1:0:0,nodes=1:ppn=16 "
fi


TMP_WORK_DIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/  # MODIFY
LOG_FILE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/flash_2var_sith.log #  MODIFY
TMP_SCRIPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/template.sh  # copy your own template.sh

DATA_DIR=""
WHERECLS=""
COMMAND=""
iter=10
SEL=5  #5 selectivities
#test ALACRITY



DATA_DIR_BASE="/tmp/work/zgong/work/intersection/data/ii_index/flash/1part_2_1G/pfd/*" # pdf 
COMMAND=$MERGE
echo "merge" >> ${LOG_FILE}
nval=2  
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done


DATA_DIR_BASE="/tmp/work/zgong/work/intersection/data/ii_index/flash/1part_2_1G/pfd/*" 
COMMAND=$SEP

echo "sep" >> ${LOG_FILE}
nval=2  
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done



DATA_DIR_BASE="/tmp/work/zgong/work/intersection/data/ii_index/flash/1part_2_1G/pfd_epd/*" 
COMMAND=$EPFD

echo "epfd" >> ${LOG_FILE}
nval=2  
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done




DATA_DIR_BASE="/tmp/work/zgong/work/intersection/data/ii_index/flash/1part_2_1G/pfd_rle/*" 
COMMAND=$RPFD

echo "rpfd" >> ${LOG_FILE}
nval=2  
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done

