#!/bin/bash
#A Aggregation script which merges all results into one csv file 
# each file only has one column data
############    RLE_PFD  ##############################################

CSVS1=("rle_4var_.001.csv" "rle_4var_.01.csv" "rle_4var_.1.csv" "rle_4var_1.csv" "rle_4var_10.csv" )


CSVS2=("rle_3var_.001.csv" "rle_3var_.01.csv" "rle_3var_.1.csv" "rle_3var_1.csv" "rle_3var_10.csv" )



CSVS3=("rle_2var_.001.csv" "rle_2var_.01.csv" "rle_2var_.1.csv" "rle_2var_1.csv" "rle_2var_10.csv" )
############    EPD_PFD  ##############################################

CSVS4=("epd_4var_.001.csv" "epd_4var_.01.csv" "epd_4var_.1.csv" "epd_4var_1.csv" "epd_4var_10.csv" )

CSVS5=("epd_3var_.001.csv" "epd_3var_.01.csv" "epd_3var_.1.csv" "epd_3var_1.csv" "epd_3var_10.csv" )

CSVS6=("epd_2var_.001.csv" "epd_2var_.01.csv" "epd_2var_.1.csv" "epd_2var_1.csv" "epd_2var_10.csv" )


############    MERGE_PFD  ##############################################


CSVS7=("merge_4var_.001.csv" "merge_4var_.01.csv" "merge_4var_.1.csv" "merge_4var_1.csv" "merge_4var_10.csv" )

CSVS8=("merge_3var_.001.csv" "merge_3var_.01.csv" "merge_3var_.1.csv" "merge_3var_1.csv" "merge_3var_10.csv" )

CSVS9=("merge_2var_.001.csv" "merge_2var_.01.csv" "merge_2var_.1.csv" "merge_2var_1.csv" "merge_2var_10.csv" )




############    SEP_PFD  ##############################################


CSVS10=("sep_4var_.001.csv" "sep_4var_.01.csv" "sep_4var_.1.csv" "sep_4var_1.csv" "sep_4var_10.csv" )

CSVS11=("sep_3var_.001.csv" "sep_3var_.01.csv" "sep_3var_.1.csv" "sep_3var_1.csv" "sep_3var_10.csv" )

CSVS12=("sep_2var_.001.csv" "sep_2var_.01.csv" "sep_2var_.1.csv" "sep_2var_1.csv" "sep_2var_10.csv" )




FILES="${CSVS1[*]} ${CSVS2[*]} ${CSVS3[*]} ${CSVS4[*]} ${CSVS5[*]} ${CSVS6[*]} ${CSVS7[*]} ${CSVS8[*]} ${CSVS9[*]} ${CSVS10[*]} ${CSVS11[*]} ${CSVS12[*]}"

TOFILE="alac_s3d.csv"

echo " mereg all files ${FILES} to one file ${TOFILE} "

paste -d"," ${FILES} > ${TOFILE}



