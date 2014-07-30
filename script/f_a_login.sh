#!/bin/bash
WORKDIR=/lustre/widow2/scratch/xzou2/widow0-20130305/tmp/

FBIN=/ccs/home/xzou2/alac_fastbit_comp/fastbit-alac-benchmark/query_code/fastbit_query
MALCBIN=/ccs/home/xzou2/alac_fastbit_comp/alac_multi_engine/build/bin/multiquery

for ((k=1;k<3; k++ )); do
		rm -rf ${WORKDIR}*
		BASEDIR="/lustre/widow2/scratch/xzou2/widow0-20130305/fastbit"${k}"/uvelindex/FastBit/P2/S3D/*"
		cp -R ${BASEDIR} ${WORKDIR}
		
    	qsub -lnodes=1:ppn=16 ./fastbit_2var.sh 
    	
    	qsub -lnodes=1:ppn=16 ./fastbit_3var.sh 
    	
    	qsub -lnodes=1:ppn=16 ./fastbit_4var.sh 

		
		rm -rf ${WORKDIR}*
		DATADIR="/lustre/widow2/scratch/xzou2/widow0-20130305/alacrity_index"${k}"/*"
		cp -R ${DATADIR} ${WORKDIR}

    	qsub -lnodes=1:ppn=16 ./alac_2var.sh 
    	
    	qsub -lnodes=1:ppn=16 ./alac_3var.sh 
    	
    	qsub -lnodes=1:ppn=16 ./alac_4var.sh 
done
