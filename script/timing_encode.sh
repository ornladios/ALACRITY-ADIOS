#!/bin/bash
# this script is used for timing the encoding method, 
# encoding method now include RPFD, and EPFD
source /ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting_sith.sh

function is_in_qsub2() {
    hostname | grep -v login
}

function suttest2() {

    if ! is_in_qsub2; then
        rm -f $TMP_DS
#        mkdir -p $TMP_WORK_DIR
#        cp -r ${DATA_DIR} $TMP_DS
	# we are not execute exactly on one dataset , abandon the dir folder 
        cp  ${DATA_DIR} $TMP_DS
        echo "Settling..."
        sleep 2s  # Wait for the copy to settle
        echo "qsub..."
	# this strange calling is because we can not add 'x' on the qsub command 
	$QSUB -x $TMP_SCRIPT $COMMAND $DS_ELM_SIZE $EN_MTD $TMP_DS $TMP_OUT_DIR >> $LOG_FILE
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

TMP_SCRIPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/encode_template.sh

DATA_DIR=""
WHERECLS=""
COMMAND=""
COMMAND=${ENCODE}
#####################need to been changed #########################
iter=5

DS_ELM_SIZE=275869696  #number of element for FLASH
DATA_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/flash/"
TMP_DS=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/flash
TMP_OUT_DIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp_out/flash
LOG_FILE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/timing_encode_flash.log


for ds in "gamc.bin" "vely.bin"   ; do 

echo $ds >> ${LOG_FILE}

	DATA_DIR=$DATA_BASE$ds
	EN_MTD="x"  # ENCODE METHOD x = PFD, h = RPFD, r = EPFD
	echo "PFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}

	EN_MTD="h" 
	echo "RPFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}


	EN_MTD="r" 
	echo "EPFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}

done


DS_ELM_SIZE=268435456 #number of element in the dataset 
DATA_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/sigmod_2GB/"
TMP_DS=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/s3d
TMP_OUT_DIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp_out/s3d
LOG_FILE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/timing_encode_s3d.log
####################end of to be changed ##########################################

for ds in "uvel.bin" "vvel.bin" "temp.bin" "wvel.bin"   ; do 

echo $ds >> ${LOG_FILE}

	DATA_DIR=$DATA_BASE$ds
	EN_MTD="x"  # ENCODE METHOD x = PFD, h = RPFD, r = EPFD
	echo "PFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}

	EN_MTD="h" 
	echo "RPFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}


	EN_MTD="r" 
	echo "EPFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}

done



DS_ELM_SIZE=268435456 #number of element in the dataset  => 2.0GB
DATA_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/gts/"
TMP_DS=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/gts
TMP_OUT_DIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp_out/gts
LOG_FILE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/timing_encode_gts.log
####################end of to be changed ##########################################

for ds in "potential.bin" ; do 

echo $ds >> ${LOG_FILE}

	DATA_DIR=$DATA_BASE$ds
	EN_MTD="x"  # ENCODE METHOD x = PFD, h = RPFD, r = EPFD
	echo "PFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}

	EN_MTD="h" 
	echo "RPFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}


	EN_MTD="r" 
	echo "EPFD" >> ${LOG_FILE}
	for ((k=1;k<=${iter}; k++ )); do
		suttest2
	done
	echo >> ${LOG_FILE}

done










