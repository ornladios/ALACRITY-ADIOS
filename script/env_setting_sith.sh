#!/bin/bash

############  setting on sith ########################
export FASTBIT_PATH=/ccs/home/xzou2/build/fastbit-1.3.5-h
export LD_LIBRARY_PATH=${FASTBIT_PATH}/lib:${LD_LIBRARY_PATH}

#WORKDIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/
FBIN=/ccs/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/print_fastbit_index_size
MALCBIN=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_decode_bmap

SKIP=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_skip
SKIPCACHE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_skip_cache

#Bitmap construction throughput
BTHRPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/bit_build_thrpt

#expansion pfordelta encoding , multiquery_decode_bmap swtich to expansion pfd 
EMAL=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_decode_bmap


MERGE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_merge
EPFD=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_decode_bmap
RPFD=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_hybrid_decode
SEP=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery  #SIMPLE & Naive


ERPFD=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_erpfd

ENCODE=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/alac



#INDEXBIN=/ccs/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/print_fastbit_index_size
#MALCBINNOPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_nopt
#SEL=("0.001" "0.01" "0.1" "1" "10" )
############  setting on sith ########################


############  setting on server3 ########################
#ALCDATADIR=/home/xzou2/data/alac_1part_2GB_comp/
#FASTDATADIR=/home/xzou2/fastbit/indexing_2GB/temp/FastBit/P2R/S3D/
#MALINDEXBIN=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_index_size
#FINDEXBIN=/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/eval_fastbit_index_size


#FBIN=/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/print_fastbit_index_size
#SEL=("0.001" "0.01" "0.1" "1" "10" )
############  setting on server3 ########################

WHERES4VAR=(
"uvel 50 69 vvel 13 23 temp 6.8E2 7.4E2 wvel 45 56" 
"uvel 50 57 vvel -8.9 -5 temp 6.9E2 7.4E2 wvel -10 -8" 
"uvel 42 80 vvel 3 5 temp 7.5E2 14E2 wvel 10 14" 
"uvel 53 93 vvel -5.7 4.2 temp 6.8E2 19E2 wvel 3.7 5.7" 
"uvel 50 66 vvel -8.8 8 temp 6.9E2 7.6E2 wvel -10 -1")

WHERES3VAR=(
"uvel 42 44 vvel -4.9 -3.8 temp 7.3E2 7.4E2" 
"uvel 42 45 vvel -4.9 0.4 temp 7.3E2 7.4E2" 
"uvel 42 52 vvel -4.9 2.6 temp 7.3E2 7.4E2" 
"uvel 47 48 vvel -2.8 2 temp 7.5E2 7.6E2" 
"uvel 38 49 vvel -8.6 2.4 temp 7.5E2 7.6E2")

WHERES2VAR=(
"uvel 25 26 vvel 4.3 4.4" 
"uvel 47 48 vvel 5.3 5.4" 
"uvel 37 46 vvel 2.8 2.9" 
"uvel 29 48 vvel -18 -10" 
"uvel 19 49 vvel -2.8 1.3"
)

WHERES1VAR=(
"vvel -17 -16.9965"
"vvel -19 -18.95"
"vvel -19 -18.5"
"vvel -19 -15.8"
"vvel -19 -8"
)
