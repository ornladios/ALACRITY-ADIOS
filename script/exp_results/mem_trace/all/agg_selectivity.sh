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


############    WAH  ##############################################
CSVS13=("wah_4var_.001.csv" "wah_4var_.01.csv" "wah_4var_.1.csv" "wah_4var_1.csv" "wah_4var_10.csv" )
CSVS14=("wah_3var_.001.csv" "wah_3var_.01.csv" "wah_3var_.1.csv" "wah_3var_1.csv" "wah_3var_10.csv" )
CSVS15=("wah_2var_.001.csv" "wah_2var_.01.csv" "wah_2var_.1.csv" "wah_2var_1.csv" "wah_2var_10.csv" )


############    EWAH  ##############################################
CSVS16=("ewah_4var_.001.csv" "ewah_4var_.01.csv" "ewah_4var_.1.csv" "ewah_4var_1.csv" "ewah_4var_10.csv" )
CSVS17=("ewah_3var_.001.csv" "ewah_3var_.01.csv" "ewah_3var_.1.csv" "ewah_3var_1.csv" "ewah_3var_10.csv" )
CSVS18=("ewah_2var_.001.csv" "ewah_2var_.01.csv" "ewah_2var_.1.csv" "ewah_2var_1.csv" "ewah_2var_10.csv" )

VAR4_1=("rle_4var_.001.csv" "epd_4var_.001.csv" "merge_4var_.001.csv" "sep_4var_.001.csv" "wah_4var_.001.csv" "ewah_4var_.001.csv")
VAR4_2=("rle_4var_.01.csv" "epd_4var_.01.csv" "merge_4var_.01.csv" "sep_4var_.01.csv" "wah_4var_.01.csv" "ewah_4var_.01.csv")
VAR4_3=("rle_4var_.1.csv" "epd_4var_.1.csv" "merge_4var_.1.csv" "sep_4var_.1.csv" "wah_4var_.1.csv" "ewah_4var_.1.csv")
VAR4_4=("rle_4var_1.csv" "epd_4var_1.csv" "merge_4var_1.csv" "sep_4var_1.csv" "wah_4var_1.csv" "ewah_4var_1.csv")
VAR4_5=("rle_4var_10.csv" "epd_4var_10.csv" "merge_4var_10.csv" "sep_4var_10.csv" "wah_4var_10.csv" "ewah_4var_10.csv")



VAR3_1=("rle_3var_.001.csv" "epd_3var_.001.csv" "merge_3var_.001.csv" "sep_3var_.001.csv" "wah_3var_.001.csv" "ewah_3var_.001.csv")
VAR3_2=("rle_3var_.01.csv" "epd_3var_.01.csv" "merge_3var_.01.csv" "sep_3var_.01.csv" "wah_3var_.01.csv" "ewah_3var_.01.csv")
VAR3_3=("rle_3var_.1.csv" "epd_3var_.1.csv" "merge_3var_.1.csv" "sep_3var_.1.csv" "wah_3var_.1.csv" "ewah_3var_.1.csv")
VAR3_4=("rle_3var_1.csv" "epd_3var_1.csv" "merge_3var_1.csv" "sep_3var_1.csv" "wah_3var_1.csv" "ewah_3var_1.csv")
VAR3_5=("rle_3var_10.csv" "epd_3var_10.csv" "merge_3var_10.csv" "sep_3var_10.csv" "wah_3var_10.csv" "ewah_3var_10.csv")


VAR2_1=("rle_2var_.001.csv" "epd_2var_.001.csv" "merge_2var_.001.csv" "sep_2var_.001.csv" "wah_2var_.001.csv" "ewah_2var_.001.csv")
VAR2_2=("rle_2var_.01.csv" "epd_2var_.01.csv" "merge_2var_.01.csv" "sep_2var_.01.csv" "wah_2var_.01.csv" "ewah_2var_.01.csv")
VAR2_3=("rle_2var_.1.csv" "epd_2var_.1.csv" "merge_2var_.1.csv" "sep_2var_.1.csv" "wah_2var_.1.csv" "ewah_2var_.1.csv")
VAR2_4=("rle_2var_1.csv" "epd_2var_1.csv" "merge_2var_1.csv" "sep_2var_1.csv" "wah_2var_1.csv" "ewah_2var_1.csv")
VAR2_5=("rle_2var_10.csv" "epd_2var_10.csv" "merge_2var_10.csv" "sep_2var_10.csv" "wah_2var_10.csv" "ewah_2var_10.csv")


######EVIL WAY TO DO THIS! HATE SHELL SCRIPT #############


paste -d","  rle_4var_.001.csv epd_4var_.001.csv merge_4var_.001.csv sep_4var_.001.csv wah_4var_.001.csv ewah_4var_.001.csv >  4_.001.csv 
paste -d"," rle_4var_.01.csv epd_4var_.01.csv merge_4var_.01.csv sep_4var_.01.csv wah_4var_.01.csv ewah_4var_.01.csv > 4_.01.csv
paste -d"," rle_4var_.1.csv epd_4var_.1.csv merge_4var_.1.csv sep_4var_.1.csv wah_4var_.1.csv ewah_4var_.1.csv > 4_.1.csv
paste -d"," rle_4var_1.csv epd_4var_1.csv merge_4var_1.csv sep_4var_1.csv wah_4var_1.csv ewah_4var_1.csv > 4_1.csv
paste -d"," rle_4var_10.csv epd_4var_10.csv merge_4var_10.csv sep_4var_10.csv wah_4var_10.csv ewah_4var_10.csv > 4_10.csv



paste -d"," rle_3var_.001.csv epd_3var_.001.csv merge_3var_.001.csv sep_3var_.001.csv wah_3var_.001.csv ewah_3var_.001.csv > 3_.001.csv
paste -d"," rle_3var_.01.csv epd_3var_.01.csv merge_3var_.01.csv sep_3var_.01.csv wah_3var_.01.csv ewah_3var_.01.csv > 3_.01.csv
paste -d"," rle_3var_.1.csv epd_3var_.1.csv merge_3var_.1.csv sep_3var_.1.csv wah_3var_.1.csv ewah_3var_.1.csv > 3_.1.csv
paste -d"," rle_3var_1.csv epd_3var_1.csv merge_3var_1.csv sep_3var_1.csv wah_3var_1.csv ewah_3var_1.csv > 3_1.csv
paste -d"," rle_3var_10.csv epd_3var_10.csv merge_3var_10.csv sep_3var_10.csv wah_3var_10.csv ewah_3var_10.csv > 3_10.csv


paste -d"," rle_2var_.001.csv epd_2var_.001.csv merge_2var_.001.csv sep_2var_.001.csv wah_2var_.001.csv ewah_2var_.001.csv > 2_.001.csv
paste -d"," rle_2var_.01.csv epd_2var_.01.csv merge_2var_.01.csv sep_2var_.01.csv wah_2var_.01.csv ewah_2var_.01.csv > 2_.01.csv
paste -d"," rle_2var_.1.csv epd_2var_.1.csv merge_2var_.1.csv sep_2var_.1.csv wah_2var_.1.csv ewah_2var_.1.csv > 2_.1.csv
paste -d"," rle_2var_1.csv epd_2var_1.csv merge_2var_1.csv sep_2var_1.csv wah_2var_1.csv ewah_2var_1.csv > 2_1.csv
paste -d"," rle_2var_10.csv epd_2var_10.csv merge_2var_10.csv sep_2var_10.csv wah_2var_10.csv ewah_2var_10.csv > 2_10.csv




sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 4_.001.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 4_.01.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 4_.1.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 4_1.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 4_10.csv 


sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 3_.001.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 3_.01.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 3_.1.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 3_1.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 3_10.csv 


sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 2_.001.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 2_.01.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 2_.1.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 2_1.csv 
sed -i '1s/^/rle, epd, merge, sep, wah, ewah \n/' 2_10.csv 





#FILES="${CSVS1[*]} ${CSVS2[*]} ${CSVS3[*]} ${CSVS4[*]} ${CSVS5[*]} ${CSVS6[*]} ${CSVS7[*]} ${CSVS8[*]} ${CSVS9[*]} ${CSVS10[*]} ${CSVS11[*]} ${CSVS12[*]}"

#TOFILE="alac_s3d.csv"

#echo " mereg all files ${FILES} to one file ${TOFILE} "

#paste -d"," ${FILES} > ${TOFILE}



