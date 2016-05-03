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
#include <inttypes.h>
//#include <openssl/md5.h>


#include <alacrity.h>
#include <trycatch.h>
#include "../src/include/alacrity-uniquery.h"
#include "../src/include/alacrity-util.h"
#include "../src/include/alacrity-rid-compress.h"
#include <../src/uniquery/helpers.h>

double dclock(void) {
	struct timeval tv;
	gettimeofday(&tv, 0);

	return (double) tv.tv_sec + (double) tv.tv_usec * 1e-6;
}

double bit_construct_time = 0, io_time= 0, decode_time = 0, bit_inter_time = 0,
		bit_to_rids_time = 0, setup_time = 0, candidate_check_time = 0, total_query_time = 0,
		lob_read_reconstruct_orig_time = 0, store_close_time = 0;
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

typedef struct{
	double index_read ;
	double decode;
	double candidate_check;
	double lob_read_reconstruct_orig;
}perf_t; // only for the performance profile

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


void setBitsinBitMap(rid_t rid, uint64_t *bitmap, uint32_t bitmap_len){
	uint32_t word = (uint32_t) (rid >> 6);
	assert(word <= bitmap_len);
	bitmap[word] |= (1LL << (rid & 0x3F));
}


void candidateCheck(const ALMetadata * meta, bin_id_t  low_bin, bin_id_t hi_bin, ALPartitionStore * ps,  const ALIndexForm oldForm , double lb, double hb, uint32_t bitmap_length,
		uint64_t * tmp_bitmap /*OUT*/, char ** inputCurPtr /*OUT*/, int v , perf_t* index_decode_candidate){
//timer_start("candidateCheck");
double candidate_time = 0, d_time = 0;
double start = dclock();
	const ALBinLayout * const bl = &meta->binLayout;
	 const uint64_t *compBinStartOffs = meta->indexMeta.u.ciim.indexBinStartOffsets;
	 uint64_t binCompressedLen;

	// low boundary bin, compressed byte offset
	binCompressedLen = compBinStartOffs[hi_bin] - compBinStartOffs[low_bin];
	ALData lowBinData; // recovered data from the low bin
	uint32_t binDataNum = bl->binStartOffsets[hi_bin] - bl->binStartOffsets[low_bin]; // lowbinData number and rid number
	//1. read low-order-bytes and reconstruct the data
	readAndReconstituteData(ps, meta, low_bin , hi_bin, false/*read all data*/, &lowBinData);
candidate_time = dclock() - start;
index_decode_candidate[v].lob_read_reconstruct_orig += candidate_time;
lob_read_reconstruct_orig_time += (candidate_time);
//timer_stop("candidateCheck");

//timer_start("decode");
start = dclock();
	//2. read rids and decode them
	uint32_t *decodedRids = (uint32_t * ) malloc(sizeof(uint32_t) * binDataNum);
	if (oldForm == ALCompressedMixInvertedIndex ) { //BRBE
		 binCompressedLen = compBinStartOffs[hi_bin] - compBinStartOffs[low_bin];
		 ALERPFDDecompressRIDs((*inputCurPtr), binCompressedLen, decodedRids,  &binDataNum);
		 (*inputCurPtr) += binCompressedLen; // todo: double check
	 }else {
		 fprintf(stderr, "compressed index format [%d] is not supported \n",oldForm );
	 }
d_time = dclock() - start;
index_decode_candidate[v].decode += d_time;
decode_time += d_time;
//timer_stop("decode");

//timer_start("candidateCheck");
start = dclock();
	//3. do candidate check
	uint32_t c = 0 ;
	if (meta->elementSize == sizeof(uint64_t) ) { // double
		double * origDataD = (double *) lowBinData;
		for ( c = 0;  c < binDataNum ; ++c ){
			if ( origDataD[c] >= lb && origDataD[c] < hb) {
				setBitsinBitMap(decodedRids[c], tmp_bitmap, bitmap_length);
			}
		}
	}else if ( meta->elementSize == sizeof(uint32_t) ) {// float
		// todo: convert to lowBinData to float *
	}

	FREE(lowBinData);
candidate_time = dclock() - start;
index_decode_candidate[v].candidate_check += candidate_time;
candidate_check_time += (candidate_time);
//timer_stop("candidateCheck");
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
			ALQueryEnsureMetadataReady(&engines[x]);

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
	ALQueryEngine *alqe = &(engines[0]);

	bin_id_t *start_bins = (bin_id_t *) malloc(sizeof(bin_id_t) * varnums);
	bin_id_t *end_bins = (bin_id_t *) malloc(sizeof(bin_id_t) * varnums);

	ALPartitionStore *partitions = (ALPartitionStore *) malloc(sizeof(ALPartitionStore) * varnums);

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
	double io_s, io_e, index_time = 0;
	//iterate every partitions

	uint64_t current_part_num = 0; // partition number

setup_time += (dclock() - setup_s);

perf_t* index_decode_candidate = (perf_t *) malloc(sizeof(perf_t) * varnums); // index read, decode, and candidate check time
int v = 0;
for ( ; v < varnums; v ++ ) {
	index_decode_candidate[v].index_read = 0;
	index_decode_candidate[v].candidate_check = 0;
	index_decode_candidate[v].decode = 0;
}

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
		uint64_t *var_tmp_bitmap = (uint64_t *) malloc(
					BITNSLOTS64(psize) * sizeof(uint64_t)); // a temporary bitmap for each variable
		/*bitmap */

setup_time += (dclock() - setup_s);

		_Bool are_bins_touched = true; // it needs all variables' partition to have bins touched
		int i = 0;
		for (; i < varnums ; i++) {

setup_s = dclock();
			/* Open partition & load meta */
			ALStoreOpenPartition(&stores[i], &partitions[i], true);
			ALQueryGetPartitionMetadata(&engines[i], &partitions[i]);


			// if previous variable constraint doesn't touch any bins, this variable constraint doesn't need to do the process,
			// it only needs to proceed the `cur_partition` variable, which the ALStoreOpenPartition did
			if ( !are_bins_touched ) {
double ss = dclock();
				ALPartitionStoreClose(&partitions[i]);
store_close_time += (dclock() - ss);
				continue;
			}


			if ( (*(engines[i].metadatas))->datatype == DATATYPE_FLOAT64){
					ALQueryEngineStartUnivariateDoubleQuery(&engines[i], lbs[i], hbs[i],
						VALUE_RETRIEVAL_QUERY_TYPE, &queries[i]);
				}else if ((*(engines[i].metadatas))->datatype == DATATYPE_FLOAT32){
					ALQueryEngineStartUnivariateFloatQuery(&engines[i], lbs[i], hbs[i],
										VALUE_RETRIEVAL_QUERY_TYPE, &queries[i]);
				}else {
					printf("[%d] is not supportted query data type \n", (*(engines[i].metadatas))->datatype);
					return false;
			}

			memset(var_tmp_bitmap, 0, BITNSLOTS64(psize) * sizeof(uint64_t));

			const ALMetadata * const meta = engines[i].metadatas[partitions[i].partition_num];
			const ALBinLayout * const bl = &meta->binLayout;
			// First, find which bins are touched by the query (all elements in the query range will
			// fall into these bins, however not all elements in these bins fall into the query range).
			// the start and end bin is left-inclusive, and right-exclusive.
			// so, end_bin actually is not in range
			are_bins_touched = findBinRange1C(meta, &queries[i], &start_bins[i], &end_bins[i]);

/*if (varnums == 2 || (varnums ==5  && (i == 1 || i == 2)) ) {
	printf("var[%d]: %s [%"PRIu64"] [%"PRIu64"] [%"PRIu32"], [%"PRIu32" : %"PRIu32" : %"PRIu32"]\n", i,  varnames[i],
			stores[i].global_meta.num_partitions, stores[i].cur_partition, bl->numBins, end_bins[i] - start_bins[i], start_bins[i], end_bins[i]);
}*/

setup_time += (dclock() - setup_s);

			if (are_bins_touched) {

			uint64_t rid_num = bl->binStartOffsets[end_bins[i]] - bl->binStartOffsets[start_bins[i]];

			ALIndex r_index =NULL;

io_s = dclock();
//timer_start("index_read");
			 ALPartitionStoreReadIndexBins(&partitions[i], meta, start_bins[i], end_bins[i], &r_index);
//timer_stop("index_read");
index_time = dclock()-io_s;
index_decode_candidate[i].index_read += index_time;
io_time = io_time + index_time;

    		 const ALIndexForm oldForm = meta->indexMeta.indexForm;
    		 //printf("old form: %d, brbe[%d], bitrun[%d], bitexp[%d], pfd[%d] \n"
    		 //	 ,oldForm , ALCompressedMixInvertedIndex, ALCompressedHybridInvertedIndex,ALCompressedExpansionII ,ALCompressedInvertedIndex);
			 if (oldForm == ALCompressedMixInvertedIndex   /*bit-run & bit-exp*/
					 || oldForm == ALCompressedHybridInvertedIndex  /* RUN-LENGTH Bit-Run */
					 || oldForm == ALCompressedExpansionII   /*bit-exp */
					 || oldForm == ALCompressedInvertedIndex /* pfd */ ){

//				 	 printf(" used index form %d \n", oldForm);
					 ALIndex* indexPtr = &r_index;
					 bin_id_t low_bin = start_bins[i];
					 bin_id_t hi_bin = end_bins[i];

					 const ALBinLayout* const bl = &meta->binLayout;
					 const bin_offset_t lo_bin_off = bl->binStartOffsets[low_bin];
					 const bin_offset_t hi_bin_off = bl->binStartOffsets[hi_bin];
					 const bin_offset_t outputCount = hi_bin_off - lo_bin_off;

					 const char * input_index = (char*)*indexPtr;
					 char *inputCurPtr = (char*)*indexPtr; // not const as we change this pointer in candidateCheck() subroutine
					 const uint64_t *compBinStartOffs = meta->indexMeta.u.ciim.indexBinStartOffsets;
					 uint64_t binCompressedLen;


					 if ( hi_bin - low_bin > 2 ) { // only check the boundary bins
						 // candidate check on low_bin
						 candidateCheck(meta, low_bin, low_bin + 1, &partitions[i], oldForm, lbs[i], hbs[i],BITNSLOTS64(psize) , var_tmp_bitmap,  &inputCurPtr, i , index_decode_candidate);

//timer_start("decode");
double de_time = 0;
decode_s = dclock();
						 // Now compress each bin in turn
						 if (oldForm == ALCompressedMixInvertedIndex ) { //BRBE
							 for (bin_id_t bin = low_bin + 1; bin < hi_bin -1 ; bin++) {
								 binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];
								 ALERPFDDecompressRIDs_set_bmap(inputCurPtr, binCompressedLen, (void **)&var_tmp_bitmap );
								 inputCurPtr += binCompressedLen;
							 }
						 }else  if (oldForm == ALCompressedHybridInvertedIndex ){ // run-length Bit-Run
							 for (bin_id_t bin = low_bin + 1; bin < hi_bin -1; bin++) {
								 binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];
								 ALRLEDecompressRIDs(inputCurPtr, binCompressedLen, (void **)&var_tmp_bitmap );
								 inputCurPtr += binCompressedLen;
							 }
						 }else if  (oldForm == ALCompressedExpansionII ){ // expanding Bit-Exp
								 for (bin_id_t bin = low_bin + 1; bin < hi_bin - 1; bin++) {
									 binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];
									 ALExpandDecompressRIDs_set_bmap(inputCurPtr, binCompressedLen, (void **)&var_tmp_bitmap );
									 inputCurPtr += binCompressedLen;
								 }
						 }else if  (oldForm == ALCompressedInvertedIndex ){ // pfd
							 for (bin_id_t bin = low_bin + 1; bin < hi_bin - 1 ; bin++) {
								 binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];
								 ALDecompressRIDs_set_bmap(inputCurPtr, binCompressedLen, (void **)&var_tmp_bitmap );
								 inputCurPtr += binCompressedLen;
							 }
						 }
de_time = dclock() - decode_s;
index_decode_candidate[i].decode += de_time;
decode_time += de_time;
//timer_stop("decode");
						 candidateCheck(meta, hi_bin -1, hi_bin , &partitions[i], oldForm, lbs[i], hbs[i],BITNSLOTS64(psize) , var_tmp_bitmap,  &inputCurPtr, i , index_decode_candidate);
					 }else {
						 // for 1 or 2 bins touched, we need to check all RIDs
						 candidateCheck(meta, low_bin, hi_bin, &partitions[i], oldForm, lbs[i], hbs[i],BITNSLOTS64(psize) , var_tmp_bitmap,  &inputCurPtr, i , index_decode_candidate);
					 }

					 FREE(input_index);

			 }else {
				 printf("Expect the index is ALCompressedMixInvertedIndex[%d, %d, %d], but I got [%d]\n ", ALCompressedMixInvertedIndex, ALCompressedHybridInvertedIndex, ALCompressedExpansionII, oldForm);

				 return false; 
			 }

				 //decode_time = decode_time + (dclock()- decode_s);
//timer_start("bit_op");
			 bit_inter_s = dclock();
			 uint64_t k = 0;
				for (; k < BITNSLOTS64(psize); k++) {
					p_bitmap[k] = p_bitmap[k] & var_tmp_bitmap[k];
				}
//timer_stop("bit_op");
bit_inter_time += (dclock() - bit_inter_s);

			} // if are_bin_touched

double s = dclock();
			ALPartitionStoreClose(&partitions[i]);
store_close_time += (dclock() - s);

		}// end of for variable loop


		/************check total number by counting the binary 1 number *******/
bit_recover_s = dclock();
//timer_start("bits_to_rids");

		uint64_t pcount  = 0, count = 0;
		if ( are_bins_touched ) {
			uint64_t kk = 0;
			for (; kk < BITNSLOTS64(psize); kk++) {

					count =  bits_in_char [p_bitmap[kk] & 0xff]
										   +  bits_in_char [(p_bitmap[kk] >>  8) & 0xff]
										   +  bits_in_char [(p_bitmap[kk] >> 16) & 0xff]
										   +  bits_in_char [(p_bitmap[kk] >> 24) & 0xff]
										   +  bits_in_char [(p_bitmap[kk] >> 32) & 0xff]
										   +  bits_in_char [(p_bitmap[kk] >> 40) & 0xff]
										   +  bits_in_char [(p_bitmap[kk] >> 48) & 0xff]
										   +  bits_in_char [(p_bitmap[kk] >> 56) & 0xff]
										   ;
				 pcount += count;
			}
			total_result_count += pcount;
//printf("partition [%"PRIu32"]: [%"PRIu32" : %"PRIu64"]\n", current_part_num, pcount, total_result_count);
			/************END *******/
			if (pcount > 0) {  /* recover RIDs from bitmap*/

				uint32_t part_offset = current_part_num * psize;

				// g_rid_t * recovered_rids = (g_rid_t *) malloc(sizeof(g_rid_t)* pcount);
				uint32_t rcount =0;
				uint32_t reconstct_rid ;
				uint64_t k = 0;
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
							//	recovered_rids[rcount ++] = reconstct_rid;

							}
						}
				}

				/*********VERIFY correctness by calculate MD5SUM value ************/
				/*unsigned char md5val[MD5_DIGEST_LENGTH + 1];

				MD5 ((unsigned char*) (&recovered_rids[0]), rcount * sizeof (rid_t), md5val);
				for (uint8_t i = 0; i < MD5_DIGEST_LENGTH; i ++) printf("%02x", md5val[i]);
				printf ("\n");*/

				// free(recovered_rids);
			}
		} // end if are_bins_touched
bit_to_rids_time += (dclock() - bit_recover_s);
//timer_stop("bits_to_rids");

	current_part_num ++;

	}// end of while partitionStore loop

printf("==Start Var Specifics\n");
x = 0;
for ( ; x < varnums; x ++ ){
	printf("==var[%9.8lf, %9.8lf]: %s  index_read: %9.3lf   decode: %9.3lf  "
	        "lob_read_reconstruct_orig: %9.3lf  candidate_check: %9.3lf\n",
	        lbs[x], hbs[x], varnames[x],
			index_decode_candidate[x].index_read, index_decode_candidate[x].decode,
			index_decode_candidate[x].lob_read_reconstruct_orig, index_decode_candidate[x].candidate_check);
}
printf("==End Var Specifics\n");
	free(index_decode_candidate);

	free(partitions);
	free(stores);
	free(engines);
	free(queries);
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

//	timer_init();
double s = dclock();
//timer_start("total");
	doMultiQueryValueConstraint(varnums, path, lb, hb,varnames);
total_query_time += (dclock() - s);
//timer_stop("total");

printf("total: %9.3lf    index_read: %9.3lf    decode: %9.3lf  lob_read_reconstruct_orig: %9.3lf   candidate_check: %9.3lf    bit_op: %9.3lf     bits_to_rids: %9.3lf   setup: %9.3lf   partition_close: %9.3lf   total_result_number: %"PRIu64"\n",
		total_query_time, io_time, decode_time, lob_read_reconstruct_orig_time, candidate_check_time, bit_inter_time, bit_to_rids_time, setup_time, store_close_time, total_result_count);
/*	printf("total: %9.3lf    index_read: %9.3lf    decode: %9.3lf   candidate_check: %9.3lf    bit_op: %9.3lf     bits_to_rids: %9.3lf\n",
			timer_get_total_interval("total"),	timer_get_total_interval("index_read"), timer_get_total_interval("decode"),
			timer_get_total_interval("candidateCheck"), timer_get_total_interval("bit_op"), timer_get_total_interval("bits_to_rids")); */

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

	process_query(argv);


}
