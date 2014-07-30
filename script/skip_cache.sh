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

TMP_WORK_DIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/
LOG_FILE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/alac_skip.log
TMP_SCRIPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/template.sh

DATA_DIR=""
WHERECLS=""
COMMAND=""
iter=1
SEL=3  #3 selectivities
#test ALACRITY

COMMAND=$SKIP
echo "4 var" >> ${LOG_FILE}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_skip_cii"${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done

#if [ 1 -eq 0 ]; then 
echo "3 var" >> ${LOG_FILE}
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_skip_cii"${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done

echo "2 var" >> ${LOG_FILE}
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_skip_cii"${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done
#fi

COMMAND=$SKIPCACHE

echo "4 var" >> ${LOG_FILE}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_skip_cii"${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done

#if [ 1 -eq 0 ]; then 
echo "3 var" >> ${LOG_FILE}
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_skip_cii"${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done

echo "2 var" >> ${LOG_FILE}
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_skip_cii"${k}"/*"
		WHERECLS=${i}
		subtest
	done
	echo >> ${LOG_FILE}
done













