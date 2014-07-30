#!/bin/bash

ALAC_DIR=/tmp/work/zgong/work/intersection/alac_multi_engine/

source $ALAC_DIR/script/env_setting_flash_sith.sh

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
	$QSUB -x $COMMAND $METHOD $TMP_WORK_DIR $WHERECLS >> $LOG_FILE
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





WHERECLS=""
COMMAND=""
TMP_WORK_DIR=$ALAC_DIR/../tmp/

############ NEED TO BE CHANGED #################
iter=10
DATASETS=pot
LOG_FILE=$ALAC_DIR/../result/bmap_build_thrpt.log
DATA_DIR="" # point to the encoded path 
############ NEED TO BE CHANGED #################
#test ALACRITY

COMMAND=$BTHRPT
echo "SIMPLE BMAP" >> ${LOG_FILE}
METHOD=1 
for ((k=1;k<=${iter}; k++ )); do
	DATA_DIR="/tmp/work/zgong/work/intersection/data/ii_index/gts/1part_2_1G/pfd/"${k}"/*"
	WHERECLS=${DATASETS}
	subtest
done
echo >> ${LOG_FILE}



echo "MERGED BMAP" >> ${LOG_FILE}
METHOD=2
for ((k=1;k<=${iter}; k++ )); do
	DATA_DIR="/tmp/work/zgong/work/intersection/data/ii_index/gts/1part_2_1G/pfd/"${k}"/*"
	WHERECLS=${DATASETS}
	subtest
done
echo >> ${LOG_FILE}


echo "EXPANSION BMAP" >> ${LOG_FILE}
METHOD=3
for ((k=1;k<=${iter}; k++ )); do
	DATA_DIR="/tmp/work/zgong/work/intersection/data/ii_index/gts/1part_2_1G/pfd_epd/"${k}"/*"
	WHERECLS=${DATASETS}
	subtest
done
echo >> ${LOG_FILE}



echo "RLE BMAP" >> ${LOG_FILE}
METHOD=4
for ((k=1;k<=${iter}; k++ )); do
	DATA_DIR="/tmp/work/zgong/work/intersection/data/ii_index/gts/1part_2_1G/pfd_rle/"${k}"/*"
	WHERECLS=${DATASETS}
	subtest
done
echo >> ${LOG_FILE}



