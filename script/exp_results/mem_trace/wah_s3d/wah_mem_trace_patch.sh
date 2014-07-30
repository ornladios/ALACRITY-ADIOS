#!/bin/bash
#THIS Script is to re-generate .csv file in order to change the epd_xx to ewah_xx name.
LOG_BASE=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/mem_trace/wah_s3d/
SEL=5

LOGS=("epd_4var_.001.out" "epd_4var_.01.out" "epd_4var_.1.out" "epd_4var_1.out" "epd_4var_10.out" )
CSVS=("ewah_4var_.001.csv" "ewah_4var_.01.csv" "ewah_4var_.1.csv" "ewah_4var_1.csv" "ewah_4var_10.csv" )
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done


LOGS=("epd_3var_.001.out" "epd_3var_.01.out" "epd_3var_.1.out" "epd_3var_1.out" "epd_3var_10.out" )
CSVS=("ewah_3var_.001.csv" "ewah_3var_.01.csv" "ewah_3var_.1.csv" "ewah_3var_1.csv" "ewah_3var_10.csv" )
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done



LOGS=("epd_2var_.001.out" "epd_2var_.01.out" "epd_2var_.1.out" "epd_2var_1.out" "epd_2var_10.out" )
CSVS=("ewah_2var_.001.csv" "ewah_2var_.01.csv" "ewah_2var_.1.csv" "ewah_2var_1.csv" "ewah_2var_10.csv" )
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	CSV=${LOG_BASE}${CSVS[i]}
	ms_print ${OUTF} | grep -P "^\s+\d+\s+[\d|,]+.*0$"  | awk '{ print  $3}' | sed 's/,//g'   | awk -F, '{print $1/1024/1024}' > ${CSV}
done









