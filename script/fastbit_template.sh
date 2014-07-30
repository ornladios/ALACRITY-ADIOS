#!/bin/bash
#if [ $# -ne 4 ] ; then
#   echo "usage : ALAC/FASTBIT 3 ${WORKDIR} $WHERECLS"
   #/home/xzou2/fastbit/data  /home/xzou2/alac/data" 
#   exit 0
#fi
#source /ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/script/env_setting_sith.sh
############  setting on sith ########################
#export FASTBIT_PATH=/ccs/home/xzou2/build/fastbit-1.3.5-h
#export LD_LIBRARY_PATH=${FASTBIT_PATH}/lib:${LD_LIBRARY_PATH}


WHERES4VAR=(
"uvel 665 674 vvel 826 840 temp 11 14 wvel 871 878" 
"uvel 665 669 vvel 68 82 temp 11 14 wvel 76 81" 
"uvel 661 677 vvel 792 805 temp 13 28 wvel 837 846" 
"uvel 666 680 vvel 79 801 temp 11 36 wvel 814 824" 
"uvel 665 673 vvel 68 817 temp 11 14 wvel 76 129")

WHERES3VAR=(
"uvel 661 663 vvel 82 88 temp 12 14" 
"uvel 661 663 vvel 82 746 temp 12 14 " 
"uvel 661 667 vvel 82 789 temp 12 14" 
"uvel 663 665 vvel 95 785 temp 13 14" 
"uvel 659 665 vvel 68 788 temp 13 14")

WHERES2VAR=(
"uvel 649 651 vvel 801 802" 
"uvel 663 665 vvel 805 806" 
"uvel 658 664 vvel 790 792" 
"uvel 653 665 vvel 51 66" 
"uvel 643 665 vvel 95 773"
)

SEL=(0.001 0.01 0.1 1 10)

if [ $# -ne 4 ] ; then
   echo "usage : needs 4 arguments"
   #/home/xzou2/fastbit/data  /home/xzou2/alac/data" 
   exit 0
fi

# $1: the command ".query" location
# $2: the number of variables
# $3: selectivty indexing 
# $4: output log location

source ~/.bashrc
cd $1

WHEREC="$(eval echo '${WHERES'$2'VAR[$3]}')"
WHICHSEL="$(eval echo '${SEL[$3]}')"
#echo $1 $2 $3 $WHEREC
#echo "./query wah $2 $WHEREC >> ${4}/${2}vars.${WHICHSEL}.wah.log"

./query wah $2 $WHEREC >> ${4}/${2}vars.${WHICHSEL}.wah.log


if [ 1 -eq 0 ] ; then 
for btype in  wah;                                                                                                                                                              
do                                                                                                                                                                              
                                                                                                                                       
    ./query ${btype} 2 uvel 649 651 vvel 801 802                           >> $2/2vars.0.001.${btype}.log                                                                          
    ./query ${btype} 2 uvel 663 665 vvel 805 806                           >> $2/2vars.0.01.${btype}.log                                                                           
    ./query ${btype} 2 uvel 658 664 vvel 790 792                           >> $2/2vars.0.1.${btype}.log                                                                            
    ./query ${btype} 2 uvel 653 665 vvel 51 66                             >> $2/2vars.1.${btype}.log                                                                              
    ./query ${btype} 2 uvel 643 665 vvel 95 773                            >> $2/2vars.10.${btype}.log                                                                             
                                                                                                                                                                              
   ./query ${btype} 3 uvel 661 663 vvel 82 88 temp 12 14                  >> $2/3vars.0.001.${btype}.log                                                                          
   ./query ${btype} 3 uvel 661 663 vvel 82 746 temp 12 14                 >> $2/3vars.0.01.${btype}.log                                                                                                                                                
    ./query ${btype} 3 uvel 661 667 vvel 82 789 temp 12 14                 >> $2/3vars.0.1.${btype}.log                                                                            
    ./query ${btype} 3 uvel 663 665 vvel 95 785 temp 13 14                 >> $2/3vars.1.${btype}.log                                                                              
    ./query ${btype} 3 uvel 659 665 vvel 68 788 temp 13 14                 >> $2/3vars.10.${btype}.log                                                                             

    ./query ${btype} 4 uvel 665 674 vvel 826 840 temp 11 14 wvel 871 878   >> $2/4vars.0.001.${btype}.log                                                                          
    ./query ${btype} 4 uvel 665 669 vvel 68 82 temp 11 14 wvel 76 81       >> $2/4vars.0.01.${btype}.log                                                                           
    ./query ${btype} 4 uvel 661 677 vvel 792 805 temp 13 28 wvel 837 846   >> $2/4vars.0.1.${btype}.log                                                                            
    ./query ${btype} 4 uvel 666 680 vvel 79 801 temp 11 36 wvel 814 824    >> $2/4vars.1.${btype}.log                                                                              
    ./query ${btype} 4 uvel 665 673 vvel 68 817 temp 11 14 wvel 76 129     >> $2/4vars.10.${btype}.log                                                                             
                                                                                                                                                                                
done
fi
