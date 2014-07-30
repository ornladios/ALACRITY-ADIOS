#!bin/bash
########## Purpose of this file ###############
# tihs file is to calcuate when 'b' is expanded to `n`, how 
# many times of pfordelta detla we can gain without any exception 
##########
# format of each trace in file 
# [1/9] [\d+]{32} 
# 1/9 stands for b value
# [\d+]{32} stands for number of exception encoding bit distribution for each pfordetla delta encode. 
# We expect no exception value when using `n` bit encoding, which means the rest of number should be 0 in each trace line
#Case study, we have following cases that find out the number of pfordetla encoding with no exception value when 
# expanding b to 2, 3, and 4 respectively.
#grep -P '^1 0 (\d+ ){1}(0 ){30}$' uvel_exp_dist.txt | awk '{if ($3 > 1) print $0}'  | wc -l
#grep -P '^1 0 (\d+ ){2}(0 ){29}$' uvel_exp_dist.txt | awk '{if ($4 > 1) print $0}'  | wc -l
#grep -P '^1 0 (\d+ ){3}(0 ){28}$' uvel_exp_dist.txt | awk '{if ($5 > 0) print $0}'  | wc -l

#TROUBLE WITH the for loop !!!!!!!!!!
# we start b=2, which is third column 
#for ((b=2; b<=32; b++ )) ; do
#	COL=`expr $b + 1`
#	grep -P '^1 0 (\d+ ){`expr $b - 1`}(0 ){`expr 32 - $b`}$' uvel_exp_dist.txt | awk '{if ("$"${COL} > 1) print $0}'  | wc -l
#done

#"$(eval echo '${LBS_'$i'[${j}]}')"

FILES=("exp_results/vely_dist.txt" "exp_results/gmac_dist.txt" "exp_results/potential_dist.txt")

for ((i=0; i<${#FILES[@]}; i++ )) ; do

echo "${FILES[i]} 1 bits"
grep -P '^1 0 (\d+ ){1}(0 ){30}$' ${FILES[i]} | awk '{if ($3 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){2}(0 ){29}$' ${FILES[i]} | awk '{if ($4 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){3}(0 ){28}$' ${FILES[i]} | awk '{if ($5 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){4}(0 ){27}$' ${FILES[i]} | awk '{if ($6 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){5}(0 ){26}$' ${FILES[i]} | awk '{if ($7 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){6}(0 ){25}$' ${FILES[i]} | awk '{if ($8 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){7}(0 ){24}$' ${FILES[i]} | awk '{if ($9 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){8}(0 ){23}$' ${FILES[i]} | awk '{if ($10 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){9}(0 ){22}$' ${FILES[i]} | awk '{if ($11 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){10}(0 ){21}$' ${FILES[i]} | awk '{if ($12 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){11}(0 ){20}$' ${FILES[i]} | awk '{if ($13 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){12}(0 ){19}$' ${FILES[i]} | awk '{if ($14 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){13}(0 ){18}$' ${FILES[i]} | awk '{if ($15 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){14}(0 ){17}$' ${FILES[i]} | awk '{if ($16 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){15}(0 ){16}$' ${FILES[i]} | awk '{if ($17 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){16}(0 ){15}$' ${FILES[i]} | awk '{if ($18 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){17}(0 ){14}$' ${FILES[i]} | awk '{if ($19 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){18}(0 ){13}$' ${FILES[i]} | awk '{if ($20 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){19}(0 ){12}$' ${FILES[i]} | awk '{if ($21 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){20}(0 ){11}$' ${FILES[i]} | awk '{if ($22 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){21}(0 ){10}$' ${FILES[i]} | awk '{if ($23 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){22}(0 ){9}$' ${FILES[i]} | awk '{if ($24 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){23}(0 ){8}$' ${FILES[i]} | awk '{if ($25 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){24}(0 ){7}$' ${FILES[i]} | awk '{if ($26 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){25}(0 ){6}$' ${FILES[i]} | awk '{if ($27 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){26}(0 ){5}$' ${FILES[i]} | awk '{if ($28 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){27}(0 ){4}$' ${FILES[i]} | awk '{if ($29 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){28}(0 ){3}$' ${FILES[i]} | awk '{if ($30 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){29}(0 ){2}$' ${FILES[i]} | awk '{if ($31 > 0) print $0}'  | wc -l
grep -P '^1 0 (\d+ ){30}(0 ){1}$' ${FILES[i]} | awk '{if ($32 > 0) print $0}'  | wc -l


echo "${FILES[i]} 9 bits"
grep -P '^9 (0 ){9}(\d+ ){1}(0 ){22}$' ${FILES[i]} | awk '{if ($11 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){10}(\d+ ){1}(0 ){21}$' ${FILES[i]} | awk '{if ($12 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){11}(\d+ ){1}(0 ){20}$' ${FILES[i]} | awk '{if ($13 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){12}(\d+ ){1}(0 ){19}$' ${FILES[i]} | awk '{if ($14 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){13}(\d+ ){1}(0 ){18}$' ${FILES[i]} | awk '{if ($15 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){14}(\d+ ){1}(0 ){17}$' ${FILES[i]} | awk '{if ($16 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){15}(\d+ ){1}(0 ){16}$' ${FILES[i]} | awk '{if ($17 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){16}(\d+ ){1}(0 ){15}$' ${FILES[i]} | awk '{if ($18 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){17}(\d+ ){1}(0 ){14}$' ${FILES[i]} | awk '{if ($19 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){18}(\d+ ){1}(0 ){13}$' ${FILES[i]} | awk '{if ($20 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){19}(\d+ ){1}(0 ){12}$' ${FILES[i]} | awk '{if ($21 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){20}(\d+ ){1}(0 ){11}$' ${FILES[i]} | awk '{if ($22 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){21}(\d+ ){1}(0 ){10}$' ${FILES[i]} | awk '{if ($23 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){22}(\d+ ){1}(0 ){9}$' ${FILES[i]} | awk '{if ($24 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){23}(\d+ ){1}(0 ){8}$' ${FILES[i]} | awk '{if ($25 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){24}(\d+ ){1}(0 ){7}$' ${FILES[i]} | awk '{if ($26 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){25}(\d+ ){1}(0 ){6}$' ${FILES[i]} | awk '{if ($27 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){26}(\d+ ){1}(0 ){5}$' ${FILES[i]} | awk '{if ($28 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){27}(\d+ ){1}(0 ){4}$' ${FILES[i]} | awk '{if ($29 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){28}(\d+ ){1}(0 ){3}$' ${FILES[i]} | awk '{if ($30 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){29}(\d+ ){1}(0 ){2}$' ${FILES[i]} | awk '{if ($31 > 0) print $0}'  | wc -l
grep -P '^9 (0 ){30}(\d+ ){1}(0 ){1}$' ${FILES[i]} | awk '{if ($32 > 0) print $0}'  | wc -l
done
