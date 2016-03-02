/*
 * multiquery.c
 *
 *  Created on: Jan 19, 2013
 *      Author: xczou
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>
//#include <openssl/md5.h>


#include <alacrity.h>
#include <uniquery.h>
#include <ALUtil.h>

// Internal includes under tools/
#include <trycatch.h>

double dclock(void) {
	struct timeval tv;
	gettimeofday(&tv, 0);

	return (double) tv.tv_sec + (double) tv.tv_usec * 1e-6;
}

double bit_construct_time = 0, io_time= 0, decode_time = 0, bit_inter_time = 0, bit_recover_time = 0, setup_time = 0;
uint64_t total_result_count = 0;

//unsigned char md5val[MD5_DIGEST_LENGTH + 1];

typedef enum {
	VAL_R, REGION_R, VAL_REGION_R
} retrieval_type;

typedef struct {
	ALIndex index;
	rid_t * rid_list;
	uint64_t num_rids; // # of rid elements
	bin_id_t num_touched_bins; // # of bins
	int64_t *bin_lbs; // bin lower bound
	int64_t *bin_hbs; // bin high bound
} rid_intersect_t;

//micro_idx structure to record the position of a RID and its value
typedef struct {
	int64_t pos;
	rid_t rid;
} micro_idx_t;

typedef uint32_t g_rid_t;  // when the rid is recovered to global id, uint32_t causes overflow


uint64_t PRECALED[64] ={
		0x01,0x02,0x04,0x08,
		0x10,0x20,0x40,0x80,
		0x100,0x200,0x400,0x800,
		0x1000,0x2000,0x4000,0x8000,
		0x10000,0x20000,0x40000,0x80000,
		0x100000,0x200000,0x400000,0x800000,
		0x1000000,0x2000000,0x4000000,0x8000000,
		0x10000000,0x20000000,0x40000000,0x80000000,
		0x100000000,0x200000000,0x400000000,0x800000000,
		0x1000000000,0x2000000000,0x4000000000,0x8000000000,
		0x10000000000,0x20000000000,0x40000000000,0x80000000000,
		0x100000000000,0x200000000000,0x400000000000,0x800000000000,
		0x1000000000000,0x2000000000000,0x4000000000000,0x8000000000000,
		0x10000000000000,0x20000000000000,0x40000000000000,0x80000000000000,
		0x100000000000000,0x200000000000000,0x400000000000000,0x800000000000000,
		0x1000000000000000,0x2000000000000000,0x4000000000000000,0x8000000000000000
};

static uint8_t bits_in_char [256] =
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
       B6(0), B6(1), B6(1), B6(2)
};


/****** NOT USED now *********/
/*#define BITMASK(b)  PRECALED[((b) & 0x3F )]
//#define BITMASK(b) 1LL << ((b) & 0x3F)  //remainder
#define BITSLOT(b) ((b) >> 6)  // divided by 64
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
//#define BITTEST(a, b) ( BITMASK(b))*/
#define BITNSLOTS64(nb) ((nb + 64 - 1) / 64)



void create_lookup(unsigned char set_bit_count[],
		unsigned char set_bit_position[][16]) {
	memset(set_bit_count, 0, 256);
	for (int i = 0; i < 65536; i++) {
//		set_bit_count[i] = __builtin_popcount(i); // total bit 1 for value i
		set_bit_count[i] =  bits_in_char [i & 0xff]
					                           +  bits_in_char [(i >>  8) & 0xff]
					                           +  bits_in_char [(i >> 16) & 0xff]
					                           +  bits_in_char [(i >> 24) & 0xff]
					                           ;
		unsigned short int temp = i;
		int counter = set_bit_count[i] - 1;
		for (int j = 15; j >= 0; j--) {
			unsigned int temp1 = temp >> j & 0x0001;
			if (temp1 == 1) {
				set_bit_position[i][counter--] = j;
			}
		}

	}
}

int compfunc (const void *theone, const void *theotherone)
{
  rid_t * first = (rid_t*)theone;
  rid_t * second = (rid_t*) theotherone;

  return (*first - *second);
}


/*
 * value/region retrieval
 * value constraint with AND operator
 */
_Bool doMultiQueryValueConstraint(const int varnums, char filebases[][1024],
		const double *lbs, const double *hbs, char varnames[][1024]) {

	double setup_s;
	setup_s = dclock();

	if (varnums <= 0)
		return false;

	ALStore *stores = (ALStore *) malloc(varnums * sizeof(ALStore));

	ALQueryEngine *engines = (ALQueryEngine *) malloc(
			varnums * sizeof(ALQueryEngine));

	ALUnivariateQuery *queries = (ALUnivariateQuery *) malloc(
			varnums * sizeof(ALUnivariateQuery));

	ALUnivariateQueryResult *results = (ALUnivariateQueryResult *) malloc(
			varnums * sizeof(ALUnivariateQueryResult));
	/*
	 * Query initialization
	 */
	int x = 0;
	for (; x < varnums; x++) {

		TRY(tc1)
		{
			ALError err = ALStoreOpenPOSIX(&stores[x], filebases[x], "r",
					false);
			if (err != ALErrorNone)
				THROW(tc1, 1,
						"Could not open ALACRITY store %s for reading", filebases[x]);

			ALQueryEngineInit(&engines[x], &stores[x], true);
			/*load partition meta info */
			ensureMetadataReady(&engines[x]);

		}
		 CATCH(tc1){
		 IF_EL(1):
		 eprintf(EMSG);
		 eprintf("\n");
		}ENDTRYCATCH

	}


/*
 * do query for partitions
 */
//	ALQueryEngine *alqe = queries[0].qe;
	ALQueryEngine *alqe = &(engines[0]);


	bin_id_t *start_bins = (bin_id_t *) malloc(sizeof(bin_id_t) * varnums);
	bin_id_t *end_bins = (bin_id_t *) malloc(sizeof(bin_id_t) * varnums);

	ALPartitionStore *partitions = (ALPartitionStore *) malloc(
			sizeof(ALPartitionStore) * varnums);

    /* pre-calculated  bitmap */
	uint64_t psize = alqe->gmeta->partition_size;
	unsigned char set_bit_count[65536];
	unsigned char set_bit_position[65536][16];
	create_lookup(set_bit_count, set_bit_position);
	//bitmap final result for each partition
	uint64_t *p_bitmap = (uint64_t *) malloc(
				BITNSLOTS64(psize) * sizeof(uint64_t));
	/* pre-calculated  bitmap */


	double bit_construct_s ,  bit_inter_s,  bit_recover_s,  decode_s;
	double io_s, io_e;
	//iterate every partitions

	uint64_t current_part_num = 0; // partition number
//	uint64_t g_total = 0;


	setup_time = setup_time + (dclock() - setup_s);


	while (!ALStoreEOF(alqe->store)) {

		setup_s = dclock();
		/*
		 * region retrieval
		 */
		memset(start_bins, 0, sizeof(bin_id_t) * varnums);
		memset(end_bins, 0, sizeof(bin_id_t) * varnums);
		memset(partitions, 0, sizeof(ALPartitionStore) * varnums);

		/* bitmap reset, set all bit are 1s (-1ULL) has all bits are 1*/
		memset(p_bitmap, -1ULL, BITNSLOTS64(psize) * sizeof(uint64_t));
		uint64_t *tmp_bitmap = (uint64_t *) malloc(
					BITNSLOTS64(psize) * sizeof(uint64_t));
		/*bitmap */

		int j = 0;
		for (; j < varnums; j++) {
			/* Open partition & load meta */
			ALStoreOpenPartition(&stores[j], &partitions[j], true);
			getPartitionMetadata(&engines[j], &partitions[j]);

			if ( (*(engines[j].metadatas))->datatype == DATATYPE_FLOAT64){
					ALQueryEngineStartUnivariateDoubleQuery(&engines[j], lbs[j], hbs[j],
						VALUE_RETRIEVAL_QUERY_TYPE, &queries[j]);
				}else if ((*(engines[j].metadatas))->datatype == DATATYPE_FLOAT32){
					ALQueryEngineStartUnivariateFloatQuery(&engines[j], lbs[j], hbs[j],
										VALUE_RETRIEVAL_QUERY_TYPE, &queries[j]);
				}else {
					printf("[%d] is not supportted query data type \n", (*(engines[j].metadatas))->datatype);
					return false;
			}



		}

		_Bool empty_index = false;
		int reorder = -1;
		int i = 0; // reading index

		setup_time = setup_time + (dclock() - setup_s);

		for (; i < varnums; i++) {


			setup_s = dclock();


			memset(tmp_bitmap, 0, BITNSLOTS64(psize) * sizeof(uint64_t));

			const ALMetadata * const meta =
					engines[i].metadatas[partitions[i].partition_num];
			const ALBinLayout * const bl = &meta->binLayout;
			// First, find which bins are touched by the query (all elements in the query range will
			// fall into these bins, however not all elements in these bins fall into the query range).
			// the start and end bin is left-inclusive, and right-exclusive.
			// so, end_bin actually is not in range
			_Bool are_bins_touched = findBinRange1C(meta, &queries[i],
					&start_bins[i], &end_bins[i]);

			if ( !are_bins_touched) {
				empty_index = true;
				printf("no bin is touched \n");
				break;
			}else{

			uint64_t rid_num = bl->binStartOffsets[end_bins[i]]
									- bl->binStartOffsets[start_bins[i]];
			ALIndex r_index =NULL;

			setup_time = setup_time + (dclock() - setup_s);


			io_s = dclock();
			 ALPartitionStoreReadIndexBins(&partitions[i], meta, start_bins[i], end_bins[i], &r_index);
			 io_time = io_time + (dclock()-io_s);

			 decode_s = dclock();
    		 const ALIndexForm oldForm = meta->indexMeta.indexForm;
    		 //printf("old form: %d, brbe[%d], bitrun[%d], bitexp[%d], pfd[%d] \n"
    		 //	 ,oldForm , ALCompressedMixInvertedIndex, ALCompressedHybridInvertedIndex,ALCompressedExpansionII ,ALCompressedInvertedIndex);
			 if (oldForm == ALCompressedMixInvertedIndex   /*bit-run & bit-exp*/
					 || oldForm == ALCompressedHybridInvertedIndex  /* RUN-LENGTH Bit-Run */
					 || oldForm == ALCompressedExpansionII   /*bit-exp */
					 || oldForm == ALCompressedInvertedIndex /* pfd */ ){

//				 	 printf(" used index form %d \n", oldForm);
					 ALIndex* indexPtr = &r_index;
					 bin_id_t lo_bin = start_bins[i];
					 bin_id_t hi_bin = end_bins[i];
					 const ALBinLayout const *bl = &meta->binLayout;
					 const bin_offset_t lo_bin_off = bl->binStartOffsets[lo_bin];
					 const bin_offset_t hi_bin_off = bl->binStartOffsets[hi_bin];
					 const bin_offset_t outputCount = hi_bin_off - lo_bin_off;

					 const char * input_index = (char*)*indexPtr;
					 const char *inputCurPtr = input_index;
					 const uint64_t *compBinStartOffs = meta->indexMeta.u.ciim.indexBinStartOffsets;
					 uint64_t binCompressedLen;

					 // Now compress each bin in turn
					 if (oldForm == ALCompressedMixInvertedIndex ) { //BRBE
						 for (bin_id_t bin = lo_bin; bin < hi_bin; bin++) {
							 binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];
							 ALERPFDDecompressRIDs_set_bmap(inputCurPtr, binCompressedLen,  &tmp_bitmap );
							 inputCurPtr += binCompressedLen;
						 }
					 }else  if (oldForm == ALCompressedHybridInvertedIndex ){ // run-length Bit-Run
						 for (bin_id_t bin = lo_bin; bin < hi_bin; bin++) {
							 binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];
							 ALRLEDecompressRIDs(inputCurPtr, binCompressedLen,  &tmp_bitmap );
							 inputCurPtr += binCompressedLen;
						 }
					 }else if  (oldForm == ALCompressedExpansionII ){ // expanding Bit-Exp
							 for (bin_id_t bin = lo_bin; bin < hi_bin; bin++) {
								 binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];
								 ALExpandDecompressRIDs_set_bmap(inputCurPtr, binCompressedLen,&tmp_bitmap );
								 inputCurPtr += binCompressedLen;
							 }
					 }else if  (oldForm == ALCompressedInvertedIndex ){ // pfd
						 for (bin_id_t bin = lo_bin; bin < hi_bin; bin++) {
							 binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];
							 ALDecompressRIDs_set_bmap(inputCurPtr, binCompressedLen,&tmp_bitmap );
							 inputCurPtr += binCompressedLen;
						 }
					 }


					 FREE(input_index);

			 }else {
				 printf("Expect the index is ALCompressedMixInvertedIndex[%d, %d, %d], but I got [%d]\n ", ALCompressedMixInvertedIndex, ALCompressedHybridInvertedIndex, ALCompressedExpansionII, oldForm);

				 return false; 
			 }

				 decode_time = decode_time + (dclock()- decode_s);

				bit_inter_s = dclock();
				int k = 0;
				for (; k < BITNSLOTS64(psize); k++) {
					p_bitmap[k] = p_bitmap[k] & tmp_bitmap[k];
				}
				bit_inter_time = bit_inter_time + (dclock() - bit_inter_s);

			}
		}


		/************check total number by counting the binary 1 number *******/
		bit_recover_s = dclock();
		total_result_count =0;
		int kk = 0;
		for (; kk < BITNSLOTS64(psize); kk++) {

			 uint64_t count =  bits_in_char [p_bitmap[kk] & 0xff]
			                           +  bits_in_char [(p_bitmap[kk] >>  8) & 0xff]
			                           +  bits_in_char [(p_bitmap[kk] >> 16) & 0xff]
			                           +  bits_in_char [(p_bitmap[kk] >> 24) & 0xff]
			                           +  bits_in_char [(p_bitmap[kk] >> 32) & 0xff]
			                           +  bits_in_char [(p_bitmap[kk] >> 40) & 0xff]
			                           +  bits_in_char [(p_bitmap[kk] >> 48) & 0xff]
			                           +  bits_in_char [(p_bitmap[kk] >> 56) & 0xff]
			                           ;
			 total_result_count += count;

		}
		/************END *******/


		/* recover region from bitmap*/
		if (total_result_count > 0) {

			uint32_t part_offset = current_part_num * psize;

			g_rid_t * recovered_rids = (g_rid_t *) malloc(sizeof(g_rid_t)*total_result_count);
			uint32_t rcount =0;
			uint32_t reconstct_rid ;
			int k = 0;
			for (; k < BITNSLOTS64(psize); k++) {
					uint64_t offset_long_int = k * 64; // original index offset
					// 2 bytes (unsigned short int)  = 16 bits
					// 4 bytes (unsigned long int )= 64 bit
					uint16_t * temp = (uint16_t *) &p_bitmap[k];
					uint64_t offset;
					for (int j = 0; j < 4; j++) {
						offset = offset_long_int + j * 16; // here, 16 is used because temp is 16bits (unsigned short int) pointer
						// set_bit_count for each 2 bytes, the number of 1
						/*
						 * *******|               64 bits                 | => final_result_bitmap []
						 * *******| 16 bits | 16 bits | 16 bits | 16 bits | => temp[]
						 */
						for (int m = 0; m < set_bit_count[temp[j]]; m++) {
							/*uint32_t reconstct_rid = offset + set_bit_position[temp[j]][m]
														 +  part_offset;*/ // global
							reconstct_rid = offset+ set_bit_position[temp[j]][m];
							recovered_rids[rcount ++] = reconstct_rid;

						}
					}
			}

			/*********VERIFY correctness by calculate MD5SUM value ************/
			/*unsigned char md5val[MD5_DIGEST_LENGTH + 1];

			MD5 ((unsigned char*) (&recovered_rids[0]), rcount * sizeof (rid_t), md5val);
			for (uint8_t i = 0; i < MD5_DIGEST_LENGTH; i ++) printf("%02x", md5val[i]);
			printf ("\n");*/

			free(recovered_rids);
		}
		bit_recover_time = bit_recover_time + (dclock() - bit_recover_s);


		int k = 0;
		for (; k < varnums; k++) {
			ALPartitionStoreClose(&partitions[k]);
		}

		current_part_num ++;

	}


	free(partitions);
	free(stores);
	free(engines);
	free(queries);
	free(results);
	free(start_bins);
	free(end_bins);
	return true;
}

void process_query(char * argv[]) {
	int varnums;

	int arg_cnt = 1;

	sscanf(argv[arg_cnt++], "%d", &varnums);

	char path[varnums][1024];

	char varnames[varnums][1024];

	char input_folder_path[1024];
	// NEED '/' at end
	strcpy(input_folder_path, argv[arg_cnt++]);

	double lb[varnums], hb[varnums];

	double start_value, end_value;
	int i = 0;
	char *vname = NULL;
	for (; i < varnums; i++) {
		strcpy(path[i], input_folder_path);
		vname = argv[arg_cnt++];
		strcpy(varnames[i], vname);
		strcat(path[i], vname);
		sscanf(argv[arg_cnt++], "%lg", &lb[i]);
		sscanf(argv[arg_cnt++], "%lg", &hb[i]);
	}


	double s  =dclock();
	doMultiQueryValueConstraint(varnums, path, lb, hb,varnames);

	//printf("%9.3lf\t%9.3lf\t%9.3lf\t%9.3lf\t%9.3lf\t%9.3lf\t%9.3lf\n"
		//		,setup_time,bit_construct_time, decode_time, io_time, bit_inter_time, bit_recover_time,dclock()-s);

	//so werid problem, decode time is always 0, this is a workaround approach, also, remove the bit_construct_time from output, its always 0
	double totalt = dclock()-s;
	printf("%9.3lf\t%9.3lf\t%9.3lf\t%9.3lf\t%9.3lf\t%9.3lf\n"
				,setup_time, totalt - (setup_time + io_time + bit_inter_time + bit_recover_time), io_time, bit_inter_time, bit_recover_time, totalt);

}

/*ONLY region retrieval available now */

int main(int argc, char **argv) {

	if (argc < 6) {
		printf("<Number of variables>\n");
		printf("<Input folder path>\n");
		printf(
				"For number of variables in query : <name of variables> <Start range> <End range>\n");
		printf("<name of variables> (As they appear in compressed files..)\n");
		printf("<Start range> : start value for range query \n");
		printf("<End range> : start value for range query \n");
		printf(
				"Sample usage : ./build/bin/multiquery 2 /home/xzou2/alacrity/indexing/ temp 350 450 vv 250 350");
		return 1;
	}

//	char rfile[1024] = "alac_results.csv";

	process_query(argv);

//	FILE * f = fopen(rfile, "a");
//	if (!f) {
//		printf("can not open file %s \n", rfile);
//	}
//
//	fprintf(f, "%9.3lf\n", e_time - s_time);

//	fclose(f);

	/*


	 printf("multi-query \n");

	 const int varnums = 2;
	 char path[2][1024];
	 strcpy(path[0], "/home/xzou2/alacrity/indexing/temp");
	 strcpy(path[1], "/home/xzou2/alacrity/indexing/vv");
	 const double lb[2] = { 350, 250 };
	 const double hb[2] = { 450, 350 };
	 doMultiQueryValueConstraint(varnums, path, lb, hb, VAL_REGION_R);*/
}
