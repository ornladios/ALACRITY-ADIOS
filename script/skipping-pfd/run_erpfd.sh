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

TMP_WORK_DIR=/lustre/atlas/proj-shared/csc025/ncsu/xzou2/tmp/
LOG_DIR=/lustre/atlas/proj-shared/csc025/ncsu/xzou2/exp_results/
TMP_SCRIPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/template.sh

DATA_DIR=""
WHERECLS=""
COMMAND=""
iter=10
SEL=2

DATA_DIR_BASES="/lustre/atlas/proj-shared/csc025/ncsu/xzou2/alac/erpfd/"

WHERES3VAR=("vvel -168 -108 uvel -224 112 temp 416 2176" "vvel -168 -76 uvel -224 -16 temp 416 480")


DATA_DIR=("1part-2GB")
COMMANDS=($ERPFD)
LOG_FILES=("erpfd-skipping.log" )


for ((n=0; n<${#DATA_DIR[@]}; n++ )) ; do 
	DATA_DIR_BASE=${DATA_DIR_BASES}${DATA_DIR[n]}
	LOG_FILE=${LOG_DIR}${LOG_FILES[n]}
	COMMAND=${COMMANDS[n]}
	for v in  3  ; do 
		nval=$v
		echo "$COMMAND $LOG_FILE $DATA_DIR_BASE $nval"
		echo "$nval var" >> ${LOG_FILE}
		for ((i=0; i<${SEL}; i++ )) ; do	
			for ((k=1;k<=${iter}; k++ )); do
				DATA_DIR=${DATA_DIR_BASE}${k}"/*"
				WHERECLS=${i}
				subtest
			done
			echo >> ${LOG_FILE}
		done
	done
done


