/*
 * ALRIDCompression.c
 *
 *  Created on: Sep 9, 2012
 *      Author: David A. Boyuka II
 */

#include <alacrity.h>
#include <alacrity-rid-compress.h>
#include <pfordelta-c-interface.h>
#include "include/alacrity-util.h"



ALError ALDecompressDeltas(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount) {
    int ret = adios_decode_deltas(input, inputLength, output, outputCount);
    return ret ? ALErrorNone : ALErrorSomething;
}

/*
 * 1: PG space is fully contained in the selection box, 0: intersected
 */
uint32_t ALDecompressRIDtoSelBox(bool isPGContained
		, const char *input
		, uint64_t inputLength
		, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount
		, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount
		, int dim
		,void **bmap){
	return decode_rids_to_selbox(isPGContained,input,inputLength
			, srcstart, srccount, deststart, destcount
			, dim, bmap);
}

/************ EPFD + RPFD METHOD ****************/


// NOTE: outputCount should hold the maximum output buffer count
// output RID list
ALError ALERPFDDecompressRIDs(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount) {
	int ret = adios_expand_runlength_decode_rid(input, inputLength, output, outputCount);
    return ret ? ALErrorNone : ALErrorSomething;
}


ALError ALERPFDCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength) {
	int ret = exapnd_runlength_encode_rids(input, inputCount, output, outputLength);
	return ret ? ALErrorNone : ALErrorSomething;
}

ALError ALERPFDDecompressRIDs_set_bmap(const char *input, uint64_t inputLength,void **bmap) {
    int ret = expand_runlength_decode_rids(input, inputLength, bmap);
    return ret ? ALErrorNone : ALErrorSomething;
}


/************ END OF EPFD + RPFD METHOD ****************/



/************ EXPANSION METHOD ****************/

ALError ALExpandCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength) {
    int ret = expand_encode_rids(input, inputCount, output, outputLength);
    return ret ? ALErrorNone : ALErrorSomething;
}
ALError ALExpandDecompressRIDs_set_bmap(const char *input, uint64_t inputLength,void **bmap) {
    int ret = expand_decode_rids(input, inputLength, bmap);
    return ret ? ALErrorNone : ALErrorSomething;
}


/************** FOLLOWING IS SKIPPING METHOD************************** */
/*
uint64_t ALSkippingSufficientBufferSize(uint32_t inputCount) {
	return get_sufficient_skipping_buffer_size(inputCount);
}


int ALSkippingPartiallyDecompressBlock(	const char *input, const uint64_t inputLength, const int64_t block_idx, uint32_t *output, uint32_t *outputCount){
	return decode_skipping_partially_decompress_block(input, inputLength, block_idx, output, outputCount);
}


int ALSkippingPartiallySearchBlock(const char *input, const uint64_t inputLength, const uint32_t rid, int64_t *block_idx) {
	return decode_skipping_partially_search_block(input,inputLength, rid, block_idx);

}
ALError ALSkippingCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength) {
	int ret =encode_skipping (input, inputCount, output, outputLength);
	return ret ? ALErrorNone : ALErrorSomething;
}

//int decode_skipping_partially_set_bmap(const char *input, const uint64_t inputLength, const uint32_t rid, void **bmap );

ALError ALSkippingDecompressRIDs(const char *input, const uint64_t inputLength, uint32_t *output, uint32_t *outputCount){

	int ret =  decode_skipping_fully(input, inputLength, output, outputCount);
	return ret ? ALErrorNone : ALErrorSomething;
}

*/
/*
 * original decoding method: given by a raw rid value, decompress block
 * return:
 *   2 : rid is not found
 *   1 : rid is found, desired block is decompressed
 *   0 : decompression error
 */
/*int ALSkippingPartiallyDecompress(const char *input, const uint64_t inputLength, const uint32_t rid, uint32_t *output, uint32_t *outputCount){
	return decode_skipping_partially(input,  inputLength, rid, output, outputCount);
}*/

uint32_t ALPForDetlaBlockSize() {
	return pfordelta_block_size();
}

/****************END OF SKIPPING **************************/



ALError ALHybridCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength) {
    int ret = hybrid_encode_rids(input, inputCount, output, outputLength);
    return ret ? ALErrorNone : ALErrorSomething;
}

ALError ALRLEDecompressRIDs(const char *input, uint64_t inputLength, void **bmap){
	int ret = rle_decode_rids(input, inputLength, bmap);
	    return ret ? ALErrorNone : ALErrorSomething;
}


// NOTE: input and output may overlap, but if so, input <= output
// NOTE: outputLength should hold the maximum output buffer length
ALError ALCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength) {
    int ret = encode_rids(input, inputCount, output, outputLength);
    return ret ? ALErrorNone : ALErrorSomething;
}

// NOTE: outputCount should hold the maximum output buffer count
ALError ALDecompressRIDs(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount) {
    int ret = decode_rids(input, inputLength, output, outputCount);
    return ret ? ALErrorNone : ALErrorSomething;
}

/*
ALError ALInpsectEncodedRIDs(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount
		, uint64_t b_stat[], uint64_t exp_stat[]) {
    int ret = print_encode_stat(input, inputLength, output, outputCount,b_stat, exp_stat);
    return ret ? ALErrorNone : ALErrorSomething;
}


ALError ALDecompressRIDs_inspect_decode_bmap(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount, void **bmap
		, uint64_t b_stat[], uint64_t exp_stat[]) {
    int ret = print_decode_stat(input, inputLength, output, outputCount,bmap,b_stat, exp_stat);
    return ret ? ALErrorNone : ALErrorSomething;
}


ALError ALDecompressRIDs_inspect_consecutive_binary_stat(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount
		, uint64_t b_stat[], uint64_t exp_stat[]) {
    int ret = print_consecutive_binary_stat(input, inputLength, output, outputCount,b_stat, exp_stat);
    return ret ? ALErrorNone : ALErrorSomething;
}
*/

ALError ALDecompressRIDs_set_bmap(const char *input, uint64_t inputLength, void **bmap) {
    int ret = decode_rids_set_bmap(input, inputLength,bmap);
    return ret ? ALErrorNone : ALErrorSomething;
}

uint64_t ALGetMaxCompressedRIDLength(uint32_t inputCount) {
    return get_sufficient_buffer_capacity(inputCount);
}


ALError ALTranslateCompressedRIDs(char *input, uint64_t inputLength, int32_t rid_offset) {
    int ret = update_rids(input, inputLength, rid_offset);
    return ret ? ALErrorNone : ALErrorSomething;
}
