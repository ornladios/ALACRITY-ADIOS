#!/bin/bash
#THIS script is used for collect the memory usage for sep_bitmap, rle_bitmap, and exp_bitmap  programs
source /home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting.sh
LOG_BASE=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/mem_trace/wah_s3d/
SEL=1   #5 selectivities
COMMAND="valgrind --tool=massif --massif-out-file="

############    WAH  ##############################################

PRM="/home/xzou2/wah_integration/icde/benchmark/intersection/query wah"

LOGS=("wah_4var_.001.out" "wah_4var_.01.out" "wah_4var_.1.out" "wah_4var_1.out" "wah_4var_10.out" )
CSVS=("wah_4var_.001.csv" "wah_4var_.01.csv" "wah_4var_.1.csv" "wah_4var_1.csv" "wah_4var_10.csv" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${BINRANGE4VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


LOGS=("wah_3var_.001.out" "wah_3var_.01.out" "wah_3var_.1.out" "wah_3var_1.out" "wah_3var_10.out" )
CSVS=("wah_3var_.001.csv" "wah_3var_.01.csv" "wah_3var_.1.csv" "wah_3var_1.csv" "wah_3var_10.csv" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${BINRANGE3VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done



LOGS=("wah_2var_.001.out" "wah_2var_.01.out" "wah_2var_.1.out" "wah_2var_1.out" "wah_2var_10.out" )
CSVS=("wah_2var_.001.csv" "wah_2var_.01.csv" "wah_2var_.1.csv" "wah_2var_1.csv" "wah_2var_10.csv" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${BINRANGE2VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


############    EWAH  ##############################################

PRM="/home/xzou2/wah_integration/icde/benchmark/intersection/query ewah"

LOGS=("ewah_4var_.001.out" "ewah_4var_.01.out" "ewah_4var_.1.out" "ewah_4var_1.out" "ewah_4var_10.out" )
CSVS=("ewah_4var_.001.csv" "ewah_4var_.01.csv" "ewah_4var_.1.csv" "ewah_4var_1.csv" "ewah_4var_10.csv" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${BINRANGE4VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


LOGS=("ewah_3var_.001.out" "ewah_3var_.01.out" "ewah_3var_.1.out" "ewah_3var_1.out" "ewah_3var_10.out" )
CSVS=("ewah_3var_.001.csv" "ewah_3var_.01.csv" "ewah_3var_.1.csv" "ewah_3var_1.csv" "ewah_3var_10.csv" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${BINRANGE3VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done



LOGS=("ewah_2var_.001.out" "ewah_2var_.01.out" "ewah_2var_.1.out" "ewah_2var_1.out" "ewah_2var_10.out" )
CSVS=("ewah_2var_.001.csv" "ewah_2var_.01.csv" "ewah_2var_.1.csv" "ewah_2var_1.csv" "ewah_2var_10.csv" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${BINRANGE2VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done









