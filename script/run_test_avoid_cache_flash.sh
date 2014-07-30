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

TMP_WORK_DIR=$ALAC_DIR/../tmp/
LOG_FILE=$ALAC_DIR/../result/epd_bmap.log
TMP_SCRIPT=$ALAC_DIR/script/template_flash.sh

DATA_DIR=""
WHERECLS=""
COMMAND=""
iter=10
SEL=5  #5 selectivities
#test ALACRITY

#DATA_DIR_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_epd_cii"
#DATA_DIR_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/alacrity_2GB_index_cii"
#DATA_DIR_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_hybrid_cii"${k}"/*"




#DATA_DIR_BASE="$ALAC_DIR/../data/ii_index/flash/1part_2_1G/pfd/"
#COMMAND=$SEP

#DATA_DIR_BASE="$ALAC_DIR/../data/ii_index/flash/1part_2_1G/pfd/"
#COMMAND=$MERGE

#DATA_DIR_BASE="$ALAC_DIR/../data/ii_index/flash/1part_2_1G/pfd_epd/"
#COMMAND=$EPFD

#DATA_DIR_BASE="$ALAC_DIR/../data/ii_index/flash/1part_2_1G/pfd_rle/"
#COMMAND=$RPFD

DATA_DIR_BASE="$ALAC_DIR/../data/ii_index/flash/1part_2_1G/raw/"
COMMAND=$RAW




echo "2 var" >> ${LOG_FILE}
nval=2  #2 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done



##########################################################################################

if [ 1 -eq 0 ]; then 
echo "3 var" >> ${LOG_FILE}
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done

echo "2 var" >> ${LOG_FILE}
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done
fi

if [ 1 -eq 0 ]; then 

LOG_FILE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/fastbit.log
COMMAND=$FBIN

DATA_DIR_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/fastbit"
echo "4 var" >> ${LOG_FILE}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/uvelindex/FastBit/P2/S3D/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done

#if [ 1 -eq 0 ]; then 
echo "3 var" >> ${LOG_FILE}
nval=3  #3 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/uvelindex/FastBit/P2/S3D/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done


echo "2 var" >> ${LOG_FILE}
nval=2  #2 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/uvelindex/FastBit/P2/S3D/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done

fi

