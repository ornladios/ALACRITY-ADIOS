#!/bin/bash

DATA_DIR_BASE="/tmp/proj/csc025/xzou2/"

IIDATAOUT=${DATA_DIR_BASE}"/alac/ii/1part-2GB1/"
CIIDATAOUT=${DATA_DIR_BASE}"/alac/pfd/1part-2GB1/"
EXPDATAOUT=${DATA_DIR_BASE}"/alac/epfd/1part-2GB1/"
RLEDATAOUT=${DATA_DIR_BASE}"/alac/rpfd/1part-2GB1/"
#SKIPDATAOUT=${DATA_DIR_BASE}"/alac/skip/1part-2GB1/"
dup=5
for M in ii pfd epfd rpfd ; do 
   
   for ((i=2;i<=${dup};i++)); do 
      pathB=${DATA_DIR_BASE}"/alac/"${M}"/1part-2GB"
      path=${pathB}${i}
      mkdir -p ${path}
     cp ${pathB}1/* ${path}/
     #echo "cp ${pathB}1/* ${path}/"
   done
done
