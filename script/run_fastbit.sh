#!/bin/bash
source /ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting_sith.sh

function is_in_qsub() {
    hostname | grep -v login
}

function subtest() {

    if ! is_in_qsub; then
        rm -rf $TMP_WORK_DIR
        mkdir -p $TMP_WORK_DIR
        cp  ${DATA_DIR}/* $TMP_WORK_DIR
        echo "Settling..."
        sleep 2s  # Wait for the copy to settle
        echo "qsub..."
	#echo "$COMMAND $nval $TMP_WORK_DIR $WHERECLS"
	$QSUB -x $TMP_SCRIPT $TMP_WORK_DIR $V $i $LOG_DIR >> $LOG_FILE
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

TMP_WORK_DIR=/lustre/atlas/proj-shared/csc025/ncsu/xzou2/tmp/
LOG_DIR=/lustre/atlas/proj-shared/csc025/ncsu/xzou2/exp_results/wah
TMP_SCRIPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/fastbit_template.sh


iter=10

#test ALACRITY

DATA_DIR_BASES="/lustre/atlas/proj-shared/csc025/ncsu/xzou2/alac/wah/"

LOG_FILE=${LOG_DIR}/wah.log



SEL=5	



for ((n=1; n<=${iter}; n++ )) ; do 
	DATA_DIR=${DATA_DIR_BASES}${n}/
	for V in 2 3 4 ; do
        	for ((i=0; i<${SEL}; i++ )) ; do	
			#./fastbit_template.sh $TMP_WORK_DIR $V $i $LOG_DIR
			subtest
		done
        done
#	echo $DATA_DIR

done

