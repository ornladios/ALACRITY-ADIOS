/*
 * c-interface.h
 *
 *  Created on: Sep 12, 2012
 *      Author: David A. Boyuka II
 */

#include <stdint.h>
#include <stdbool.h>

#ifndef C_INTERFACE_H_
#define C_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PRINT_ERROR_RETURN_FAIL(...) \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
    return false

#define PRINT_WARNING_RETURN_FAIL(...) \
	fprintf(stdout, __VA_ARGS__); \
	fprintf(stdout, "\n"); \
    return false


//####################ORIGINAL PFORDETLA ######################################################//
/*
 * PForDelta encoding
 * input: a list of integers
 * output: a stream of bytes encoded in the pfordelta format
 */
int encode_rids(const uint32_t *input /*input a list of integers*/, uint32_t inputCount /*input integer number*/
		         , char *output /*output bit stream*/, uint64_t *outputLength /*output bit stream length*/);

/**
 * PForDelta decoding
 * input: a bit stream
 * output: a list of integers
 */
int decode_rids(const char *input /*bit stream*/, uint64_t inputLength/*bit stream length*/
		        , uint32_t *output /*integer list*/, uint32_t *outputCount/*integer list size*/);

/*
 * This function is not recommended, as it modifies the PForDelta bit stream
 */
int update_rids(char *input, uint64_t inputLength, int32_t rid_offset);
/*
 * An estimated function that returns the how much memory needs to hold the encoded PForDelta bit stream.
 * This is often called before the encode_rids function so the caller can know how much memory should be allocated in advance
 */
uint32_t get_sufficient_buffer_capacity(uint32_t bin_length);
//####################END OF ORIGINAL PFORDETLA ######################################################//



//####################COMPONENT 1 == PFORDETLA WITH RUNLENGTH AND EXPANSION ######################################################//
/*
 * PForDelta decoding function.
 * Different from the decode_rids, it outputs a bitmap, rather than a list of integers.
 */
int decode_rids_set_bmap(const char *input, uint64_t inputLength,void **bmap);

/*
 * this hybrid function encodes the data by two approaches.
 * For b (minimum bits of encoding most of integers) determined to be 1, we use the Run Length Encoding.
 * For other bs, we turn to use the original encode function
  */
int hybrid_encode_rids(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength);

/*
 * Special encoding for b=1, & pfordelta decode for b!=1
 * for the efficient memory usage purpose, we hold data in bitmap, therefore,
 * there is no need to keep decompressed data in memory.
 */
int rle_decode_rids(const char *input, uint64_t inputLength,void **bmap);

/***************FOLLOWING IS EXPANSION + RUN Length PFD METHOD *************************/

/*
 * Now, we have combine the expansion and the run length, which deals the b=1 and b ~= 1 cases, respectively
 * The codes is just two branches from the expansion and run length
 */
int exapnd_runlength_encode_rids(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength);

int expand_runlength_decode_rids(const char *input, uint64_t inputLength, void **bmap);


/***************FOLLOWING IS EXPANSION PFD METHOD *************************/
/*
 * Basic idea is instead of using b bits, we are using b+n bits which eliminates exception value
 * `n` value is wisely computed
 * at this moment, we look at the case b = 9, and expand 9 to 11
 * which will drop around 60% and 25% of rounds without exception for variable (vvel, wvel) and (temp, uvel) respectively
 */
int expand_encode_rids(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength);

/*
 * signature changed:
 * for memory efficiency it does not need recover data in a uint32_t array format
 * instead, we have bitmap to represent the recovered data
 */
int expand_decode_rids(const char *input, uint64_t inputLength, void **bmap);


/*
 * Sufficient buffer allocated for skipping encoding
 */

uint64_t get_sufficient_skipping_buffer_size(uint32_t bin_length);

/*
 * return pfordelta block size, usual case is 128
 */
uint32_t pfordelta_block_size();



//#############COMPONENT 2 == ADIOS PFORDETLA ######################################################//
/* PForDelta functions tailored for ALACRITY in ADIOS
 * The main difference is there is an additional selection box in the input
 */

/* Following function is particular for the case in which the data candidate check requires
 * any compressed form to been decoded into RIDs
 * */
//BitRun
int adios_runlength_decode_rid(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount);

//BitExp
int adios_expand_decode_rid(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount);

//BitRun-BitExp
int adios_expand_runlength_decode_rid(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount);

//ASSUME the data is compressed by PForDelta (no bitrun, bitexp etc.), the output is a delta list, rather than a original rid list
//this detal list has the first element as frame of reference, and the rest values are deltas
int adios_decode_deltas(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount);

/*
 * Given a selection box and indexes, output a bitmap, the scope of which represents the selection box
 * */
uint32_t decode_rids_to_selbox(
						bool isPGContained /*1: PG space is fully contained in the selection box, 0: intersected*/
						, const char *input
						, uint64_t inputLength
						, uint64_t *srcstart //PG region dimension
						, uint64_t *srccount
						, uint64_t *deststart //region dimension of the Selection box
						, uint64_t *destcount
						, int dim
						,void **bmap);

uint32_t runlength_decode_rids_to_selbox(bool isPGContained /*1: PG space is fully contained in the selection box, 0: intersected*/
, const char *input, uint64_t inputLength, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount, int dim, void **bitmap);

//#######################END OF ADIOS PFORDELTA################################################# //




/*
 * Discarded  functions
 *
 *int print_decode_stat(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount, void **bmap
		,uint64_t b_stat[], uint64_t exp_stat[]);

 *int print_encode_stat(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount,
		uint64_t b_stat[], uint64_t exp_stat[] );

 *int adaptive_encode_rids(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength);

 *int adpative_decode_rids(const char *input, uint64_t inputLength,void **bmap);



 * Format of output buffer
 * [total # of input element] | [# of blocks] | [first level index] | [compress block pointer (bytes offset)] | [ every compressed block]
 *  uint32_t                  |  uint32_t     | rid_t == uint32_t   |  uint64_t                               |
 *  every compressed block length is computed in a such way, comp_blk_length[i] = comp_blk_ptr[i+1] - comp_blk_ptr[i] // shift one block
 *  we have extra block pointer here
 *
 *int encode_skipping (const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength);


 * decode part of blocks and meanwhile setting the bitmap given by a raw RID value

 *int decode_skipping_partially_set_bmap(const char *input, const uint64_t inputLength, const uint32_t rid, void **bmap );

 * decode entire skipping structure, this is used by smallest RID list
 *int decode_skipping_fully(const char *input, const uint64_t inputLength, uint32_t *output, uint32_t *outputCount);

 * original decoding method: given by a raw rid value, decompress block
 * return:
 *   2 : rid is not found
 *   1 : rid is found, desired block is decompressed
 *   0 : decompression error
 *int decode_skipping_partially(const char *input, const uint64_t inputLength, const uint32_t rid, uint32_t *output, uint32_t *outputCount);

 * decode_skipping_partially function is divided into two functions (xx_search_block & xx_decompress_block)
 * in order to allow caller to implement caching
 * the caller will call this xx_search_block function, and will cache this returned block number.
 * the caller will also call xx_decompress_block function, and cache the decompressed block.
 * return: 0 means not found / decompress unsuccessfully
 *         1 means we are good
 *int decode_skipping_partially_search_block(const char *input, const uint64_t inputLength, const uint32_t rid, int64_t *block_idx );
 *int decode_skipping_partially_decompress_block(
		const char *input, const uint64_t inputLength, const int64_t block_idx, uint32_t *output, uint32_t *outputCount);

 * customized binary search
 * looking for a position that the key value locates the interval of [position , position + 1]
 *_Bool binary_search_for_position(const raw_rid_t *set, int64_t low, int64_t high, raw_rid_t key,
		int64_t *key_pos);

 * Sufficient buffer allocated for skipping encoding
 *uint64_t get_sufficient_skipping_buffer_size(uint32_t bin_length);
 */



#ifdef __cplusplus
}
#endif

#endif /* C_INTERFACE_H_ */
