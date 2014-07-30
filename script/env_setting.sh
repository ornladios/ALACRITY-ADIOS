#!/bin/bash

############  setting on sith ########################
#export FASTBIT_PATH=/ccs/home/xzou2/build/fastbit-1.3.5-h
#export LD_LIBRARY_PATH=${FASTBIT_PATH}/lib:${LD_LIBRARY_PATH}

#WORKDIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/
#FBIN=/ccs/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/print_fastbit_index_size
#MALCBIN=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery
#INDEXBIN=/ccs/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/print_fastbit_index_size
#MALCBINNOPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_nopt
#SEL=("0.001" "0.01" "0.1" "1" "10" )
############  setting on sith ########################


############  setting on server3 ########################
ALCDATADIR=/home/xzou2/data/alac_1part_2GB_comp/
FASTDATADIR=/home/xzou2/fastbit/indexing_2GB/temp/FastBit/P2R/S3D/
MALINDEXBIN=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_index_size
FINDEXBIN=/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/eval_fastbit_index_size
FBIN=/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/print_fastbit_index_size

MALCBIN=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery
NEWMALCBIN=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_decode_bmap
INSPMALCBIN=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_inspect_decode
BLKMALBIN=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_block_overlap
RIDMALBIN=/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_rid_decomp_block

#SEL=("0.001" "0.01" "0.1" "1" "10" )
############  setting on server3 ########################
#where clause is ordered by selectivity from 0.001% to 10%
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
#"vvel -19 0"


#query bin boundaries, order by selectivity from 0 .001% to 10%
BINRANGE4VAR=(
"uvel 665 674 vvel 826 840 temp 11 14 wvel 871 878" 
"uvel 665 669 vvel 68 82 temp 11 14 wvel 76 81" 
"uvel 661 677 vvel 792 805 temp 13 28 wvel 837 846" 
"uvel 666 680 vvel 79 801 temp 11 36 wvel 814 824" 
"uvel 665 673 vvel 68 817 temp 11 14 wvel 76 129" )
BINRANGE3VAR=(
"uvel 661 663 vvel 82 88 temp 12 14" 
"uvel 661 663 vvel 82 746 temp 12 14"
"uvel 661 667 vvel 82 789 temp 12 14" 
"uvel 663 665 vvel 95 785 temp 13 14" 
"uvel 659 665 vvel 68 788 temp 13 14")
BINRANGE2VAR=(
"uvel 649 651 vvel 801 802" 
"uvel 663 665 vvel 805 806" 
"uvel 658 664 vvel 790 792" 
"uvel 653 665 vvel 51 66" 
"uvel 643 665 vvel 95 773" 
)

