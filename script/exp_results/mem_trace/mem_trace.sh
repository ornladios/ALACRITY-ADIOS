#!/bin/bash
#THIS script is used for collect the memory usage for sep_bitmap, rle_bitmap, and exp_bitmap  programs
source /home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting.sh
LOG_BASE=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/mem_trace/
SEL=1   #5 selectivities
COMMAND="valgrind --tool=massif --massif-out-file="

############    RLE_PFD  ##############################################
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd_rle/
PRM=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_hybrid_decode

LOGS=("rle_4var_.001.log" "rle_4var_.01.log" "rle_4var_.1.log" "rle_4var_1.log" "rle_4var_10.log" )
CSVS=("rle_4var_.001.csv" "rle_4var_.01.csv" "rle_4var_.1.csv" "rle_4var_1.csv" "rle_4var_10.csv" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES4VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


LOGS=("rle_3var_.001.log" "rle_3var_.01.log" "rle_3var_.1.log" "rle_3var_1.log" "rle_3var_10.log" )
CSVS=("rle_3var_.001.csv" "rle_3var_.01.csv" "rle_3var_.1.csv" "rle_3var_1.csv" "rle_3var_10.csv" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES3VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done



LOGS=("rle_2var_.001.log" "rle_2var_.01.log" "rle_2var_.1.log" "rle_2var_1.log" "rle_2var_10.log" )
CSVS=("rle_2var_.001.csv" "rle_2var_.01.csv" "rle_2var_.1.csv" "rle_2var_1.csv" "rle_2var_10.csv" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES2VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


############    EPD_PFD  ##############################################
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd_epd/
PRM=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_decode_bmap

LOGS=("epd_4var_.001.log" "epd_4var_.01.log" "epd_4var_.1.log" "epd_4var_1.log" "epd_4var_10.log" )
CSVS=("epd_4var_.001.csv" "epd_4var_.01.csv" "epd_4var_.1.csv" "epd_4var_1.csv" "epd_4var_10.csv" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES4VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


LOGS=("epd_3var_.001.log" "epd_3var_.01.log" "epd_3var_.1.log" "epd_3var_1.log" "epd_3var_10.log" )
CSVS=("epd_3var_.001.csv" "epd_3var_.01.csv" "epd_3var_.1.csv" "epd_3var_1.csv" "epd_3var_10.csv" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES3VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done



LOGS=("epd_2var_.001.log" "epd_2var_.01.log" "epd_2var_.1.log" "epd_2var_1.log" "epd_2var_10.log" )
CSVS=("epd_2var_.001.csv" "epd_2var_.01.csv" "epd_2var_.1.csv" "epd_2var_1.csv" "epd_2var_10.csv" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES2VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done





############    MERGE_PFD  ##############################################
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd/
PRM=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_merge


LOGS=("merge_4var_.001.log" "merge_4var_.01.log" "merge_4var_.1.log" "merge_4var_1.log" "merge_4var_10.log" )
CSVS=("merge_4var_.001.csv" "merge_4var_.01.csv" "merge_4var_.1.csv" "merge_4var_1.csv" "merge_4var_10.csv" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES4VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


LOGS=("merge_3var_.001.log" "merge_3var_.01.log" "merge_3var_.1.log" "merge_3var_1.log" "merge_3var_10.log" )
CSVS=("merge_3var_.001.csv" "merge_3var_.01.csv" "merge_3var_.1.csv" "merge_3var_1.csv" "merge_3var_10.csv" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES3VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done



LOGS=("merge_2var_.001.log" "merge_2var_.01.log" "merge_2var_.1.log" "merge_2var_1.log" "merge_2var_10.log" )
CSVS=("merge_2var_.001.csv" "merge_2var_.01.csv" "merge_2var_.1.csv" "merge_2var_1.csv" "merge_2var_10.csv" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES2VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done







############    SEP_PFD  ##############################################
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd/
PRM=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery


LOGS=("sep_4var_.001.log" "sep_4var_.01.log" "sep_4var_.1.log" "sep_4var_1.log" "sep_4var_10.log" )
CSVS=("sep_4var_.001.csv" "sep_4var_.01.csv" "sep_4var_.1.csv" "sep_4var_1.csv" "sep_4var_10.csv" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES4VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


LOGS=("sep_3var_.001.log" "sep_3var_.01.log" "sep_3var_.1.log" "sep_3var_1.log" "sep_3var_10.log" )
CSVS=("sep_3var_.001.csv" "sep_3var_.01.csv" "sep_3var_.1.csv" "sep_3var_1.csv" "sep_3var_10.csv" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES3VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done



LOGS=("sep_2var_.001.log" "sep_2var_.01.log" "sep_2var_.1.log" "sep_2var_1.log" "sep_2var_10.log" )
CSVS=("sep_2var_.001.csv" "sep_2var_.01.csv" "sep_2var_.1.csv" "sep_2var_1.csv" "sep_2var_10.csv" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES2VAR[i]} 
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done





