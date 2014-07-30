#!/bin/bash
#if [ $# -ne 4 ] ; then
#   echo "usage : ALAC/FASTBIT 3 ${WORKDIR} $WHERECLS"
   #/home/xzou2/fastbit/data  /home/xzou2/alac/data" 
#   exit 0
#fi
source /tmp/work/zgong/work/intersection/alac_multi_engine/script/env_setting_flash_sith.sh
############  setting on sith ########################
#export FASTBIT_PATH=/ccs/home/xzou2/build/fastbit-1.3.5-h
#export LD_LIBRARY_PATH=${FASTBIT_PATH}/lib:${LD_LIBRARY_PATH}

#eval $@
WHEREC="$(eval echo '${WHERES'$2'VAR[$4]}')"
echo $1 $2 $3 $WHEREC
$1 $2 $3 $WHEREC 

#$1 $2 $3 $4 
