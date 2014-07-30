#!/bin/bash
WORKDIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/

#FBIN=/ccs/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/fastbit_query
MALCBIN=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery
MALCBINNOPT=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery_nopt

for ((k=1;k<6; k++ )); do
		
	rm -rf ${WORKDIR}*
	DATADIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alacrity_index"${k}"/*"
	cp -R ${DATADIR} ${WORKDIR}

    	#qsub -lnodes=1:ppn=16 ./alac_2var.sh 
    	
    	qsub -lnodes=1:ppn=16 ./alac_3var.sh 
	qsub -lnodes=1:ppn=16 ./malac_nopt_3var.sh 
    	
    	#qsub -lnodes=1:ppn=16 ./alac_4var.sh 
done
