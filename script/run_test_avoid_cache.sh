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
LOG_DIR=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/skipping_query/
TMP_SCRIPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/template.sh

DATA_DIR=""
WHERECLS=""
COMMAND=""
iter=1
SEL=5  #5 selectivities
#test ALACRITY

DATA_DIR_BASES="/lustre/widow2/scratch/xzou2/widow0-20130305/"
#DATA_DIR_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/alacrity_2GB_index_cii"
#DATA_DIR_BASE="/lustre/widow2/scratch/xzou2/widow0-20130305/alac_2GB_hybrid_cii"${k}"/*"


#DATA_DIR=("alac_2GB_ii" "alac_2GB_pfd_cii" "alac_2GB_pfd_cii" "alac_2GB_epd_cii" "alac_2GB_rle_cii")
#COMMANDS=($SEP $SEP $MERGE $EPFD $RPFD)
#LOG_FILES=("naive.log" "simple.log" "merged.log" "epfd.log" "rpfd.log")


DATA_DIR=("epfd_model")
COMMANDS=($EPFD)
LOG_FILES=("epfd_model.log" )


for ((n=0; n<${#DATA_DIR[@]}; n++ )) ; do 
	DATA_DIR_BASE=${DATA_DIR_BASES}${DATA_DIR[n]}
	LOG_FILE=${LOG_DIR}${LOG_FILES[n]}
	COMMAND=${COMMANDS[n]}
	for v in 4 3 2 ; do 
		nval=$v
		echo "$COMMAND $LOG_FILE $DATA_DIR_BASE $nval"
		if [ 1 -eq 0 ] ; then
		echo "$nval var" >> ${LOG_FILE}
		for ((i=0; i<${SEL}; i++ )) ; do	
			for ((k=1;k<=${iter}; k++ )); do
				DATA_DIR=${DATA_DIR_BASE}${k}"/*"
				WHERECLS=${i}
				subtest
			done
			echo >> ${LOG_FILE}
		done
		fi
	done
done

if [ 1 -eq 0 ] ; then 
echo "4 var" >> ${LOG_FILE}
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	
	for ((k=1;k<=${iter}; k++ )); do
		DATA_DIR=${DATA_DIR_BASE}${k}"/*"
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

