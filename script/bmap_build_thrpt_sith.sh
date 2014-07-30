#!/bin/bash
source /ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting_sith.sh

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
TMP_WORK_DIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/

############ NEED TO BE CHANGED #################
iter=5
#DATASETS=temp
LOG_FILE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/bmap_build_thrpt_s3d_gts_flash.log
DATA_DIR="" # point to the encoded path 
############ NEED TO BE CHANGED #################
#test ALACRITY
COMMAND=$BTHRPT

echo "S3D" >> ${LOG_FILE}

for DATASETS in uvel vvel temp wvel ; do 

echo $DATASETS >> ${LOG_FILE}

	echo "SIMPLE BMAP" >> ${LOG_FILE}
	METHOD=1 
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_pfd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}



	echo "MERGED BMAP" >> ${LOG_FILE}
	METHOD=2
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_pfd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}


	echo "EXPANSION BMAP" >> ${LOG_FILE}
	METHOD=3
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_epd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}



	echo "RLE BMAP" >> ${LOG_FILE}
	METHOD=4
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_rle_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}


done






echo "FLASH" >> ${LOG_FILE}
for DATASETS in gamc vely ; do 

echo $DATASETS >> ${LOG_FILE}

	echo "SIMPLE BMAP" >> ${LOG_FILE}
	METHOD=1 
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/flash/pfd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}



	echo "MERGED BMAP" >> ${LOG_FILE}
	METHOD=2
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/flash/pfd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}


	echo "EXPANSION BMAP" >> ${LOG_FILE}
	METHOD=3
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/flash/epd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}



	echo "RLE BMAP" >> ${LOG_FILE}
	METHOD=4
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/flash/rle_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}


done








echo "GTS" >> ${LOG_FILE}
for DATASETS in potential ; do 

echo $DATASETS >> ${LOG_FILE}

	echo "SIMPLE BMAP" >> ${LOG_FILE}
	METHOD=1 
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/gts/pfd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}



	echo "MERGED BMAP" >> ${LOG_FILE}
	METHOD=2
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/gts/pfd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}


	echo "EXPANSION BMAP" >> ${LOG_FILE}
	METHOD=3
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/gts/epd_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}



	echo "RLE BMAP" >> ${LOG_FILE}
	METHOD=4
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/gts/rle_cii"${k}"/*"
		WHERECLS=${DATASETS}
		subtest
	done
	echo >> ${LOG_FILE}


done











