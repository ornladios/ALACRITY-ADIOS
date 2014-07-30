#!/bin/bash
#THIS script is used for collect the memory usage for sep_bitmap, rle_bitmap, and exp_bitmap  programs
source /home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting.sh
LOG_BASE=/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/exp_results/cache_miss/alac/
SEL=4 #5 selectivities
COMMAND="valgrind --tool=cachegrind --log-file="

############    RLE_PFD  ##############################################
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd_rle/
PRM=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_hybrid_decode

LOGS=("4var.0.001.rle.log" "4var.0.01.rle.log" "4var.0.1.rle.log" "4var.1.rle.log" "4var.10.rle.log" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES4VAR[i]} 
done


LOGS=("3var.0.001.rle.log" "3var.0.01.rle.log" "3var.0.1.rle.log" "3var.1.rle.log" "3var.10.rle.log" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES3VAR[i]} 
done



LOGS=("2var.0.001.rle.log" "2var.0.01.rle.log" "2var.0.1.rle.log" "2var.1.rle.log" "2var.10.rle.log" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES2VAR[i]} 
done


############    EPD_PFD  ##############################################
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd_epd/
PRM=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_decode_bmap

LOGS=("4var.0.001.epd.log" "4var.0.01.epd.log" "4var.0.1.epd.log" "4var.1.epd.log" "4var.10.epd.log" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES4VAR[i]} 
done

LOGS=("3var.0.001.epd.log" "3var.0.01.epd.log" "3var.0.1.epd.log" "3var.1.epd.log" "3var.10.epd.log" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES3VAR[i]} 
done

LOGS=("2var.0.001.epd.log" "2var.0.01.epd.log" "2var.0.1.epd.log" "2var.1.epd.log" "2var.10.epd.log" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES2VAR[i]} 
done





############    MERGE_PFD  ##############################################
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd/
PRM=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_merge

LOGS=("4var.0.001.merge.log" "4var.0.01.merge.log" "4var.0.1.merge.log" "4var.1.merge.log" "4var.10.merge.log" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES4VAR[i]} 
done


LOGS=("3var.0.001.merge.log" "3var.0.01.merge.log" "3var.0.1.merge.log" "3var.1.merge.log" "3var.10.merge.log" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES3VAR[i]} 
done


LOGS=("2var.0.001.merge.log" "2var.0.01.merge.log" "2var.0.1.merge.log" "2var.1.merge.log" "2var.10.merge.log" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES2VAR[i]} 
done







############    SEP_PFD  ##############################################
DATADIR=/home/xzou2/data/ii_index/s3d/1part_2G/pfd/
PRM=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery

LOGS=("4var.0.001.sep.log" "4var.0.01.sep.log" "4var.0.1.sep.log" "4var.1.sep.log" "4var.10.sep.log" )
nval=4  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES4VAR[i]} 
	
done


LOGS=("3var.0.001.sep.log" "3var.0.01.sep.log" "3var.0.1.sep.log" "3var.1.sep.log" "3var.10.sep.log" )
nval=3  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES3VAR[i]} 
	
done


LOGS=("2var.0.001.sep.log" "2var.0.01.sep.log" "2var.0.1.sep.log" "2var.1.sep.log" "2var.10.sep.log" )
nval=2  #4 variables 
for ((i=0; i<${SEL}; i++ )) ; do
	OUTF=${LOG_BASE}${LOGS[i]}
	
	$COMMAND${OUTF} $PRM $nval ${DATADIR} ${WHERES2VAR[i]} 
	
done





