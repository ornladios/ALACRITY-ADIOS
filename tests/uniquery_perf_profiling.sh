#!/bin/sh
#merge path nblocks output
function merge() 
{

   for i in `seq 0 $2`
     do
     cat $1.${i} >> $3
   done
}

if [ $# -ne 3 ] ; then
   echo "usage : ./unquery_perf_profiling.sh ~/data ~/alacrity_index 5" 
   exit 0
fi

EXECDIR=./alacrity-rid-comp/bin

LOG="alout.txt"

LOG2="alidx.txt"

if [ -f ${LOG} ]; then
   rm -f ${LOG}
fi
  echo "" > ${LOG}
  
if [ -f ${LOG2} ]; then
   rm -f ${LOG2}
fi
  echo "" > ${LOG2}  
FOLDERS=("temp" "wvel" "vvel" "uvel" )


CONSTRAINTS=(750 755 4 5 5 6 8 9)  #low & high value pair for each variable 
#PSIZE=(2000 2000000)
PSIZE=(2000)
#./unquery_perf_profiling.sh raw_data_path index_output_path num_blocks 
# i.e ./unquery_perf_profiling.sh ~/data ~/alacrity_index 5 
for ((J=0; J<${#PSIZE[@]}; J++ )) ; do
    NUMELEM=${PSIZE[J]}         # # of elements in each partition
    echo " partition size is ${NUMELEM} " >> ${LOG}	
	for ((I=0; I<${#FOLDERS[@]}; I++ )) ; do
		 VARPATH=$1"/"${FOLDERS[I]} # concatenate string i.e ~/data/temp


		 INDEXPATH=$2"/"${FOLDERS[I]} # i.e ~/alacrity_index/temp 
		 INDEXPREFIX=$INDEXPATH"/"${FOLDERS[I]} #i.e ~/alacrity_index/temp/temp

		 if [ ! -d ${INDEXPATH} ]; then  # if index path doesn't exist, create it 
		    echo "${INDEXPATH} NOT EXIST, Create it!"
		    mkdir -p ${INDEXPATH}
		 fi

		     BINFILE=${VARPATH}"/"${FOLDERS[I]}".bin"  # i.e ~/data/temp/temp.bin
		
		if ( -f ${BINFILE} ) ; then 
		   rm -f ${BINFILE}
		fi

		     #merge file  , $3 is # of block
		     for j in `seq 0 $3`
		        do
		          cat ${VARPATH}"/block."${j} >> ${BINFILE}
		     done
		     #merge ${VARPATH}"/block" $3 ${BINFILE}

	
		 ${EXECDIR}"/alac" encode -p${NUMELEM}E -i -e8 -s16 ${BINFILE} ${INDEXPREFIX} >> ${LOG2}

		 echo "profile ${FOLDERS[I]}" >> ${LOG}

		 ${EXECDIR}"/test-uniquery-bi" ${INDEXPREFIX} ${CONSTRAINTS[I*2]} ${CONSTRAINTS[I*2+1]} 0 >> ${LOG}

	done
done
