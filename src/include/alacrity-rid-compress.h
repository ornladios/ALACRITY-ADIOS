/*
 * alacrity-rid-compress.h
 *
 *  Created on: Sep 9, 2012
 *      Author: David A. Boyuka II
 */

#ifndef ALACRITY_RID_COMPRESS_H_
#define ALACRITY_RID_COMPRESS_H_



#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*
 * ADIOS RID decompression. RID compression is through index building
 * 1: PG space is fully contained in the selection box, 0: intersected
 */
ALError ALDecompressDeltas(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount);

uint32_t ALDecompressRIDtoSelBox(bool isPGContained
		, const char *input
		, uint64_t inputLength
		, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount
		, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount
		, int dim
		,void **bmap);


/*
 * EXPASION compression and de-compression
 */

ALError ALExpandCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength) ;

ALError ALExpandDecompressRIDs_set_bmap(const char *input, uint64_t inputLength,void **bmap);
/*
 * END OF EXPASION
 */

ALError ALHybridCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength);

ALError ALRLEDecompressRIDs(const char *input, uint64_t inputLength,  void **bmap);

// NOTE: input and output may overlap, but if so, input <= output
// NOTE: outputLength should hold the maximum output buffer length
ALError ALCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength);

/************ EPFD + RPFD METHOD ****************/
ALError ALERPFDCompressRIDs(const uint32_t *input, uint32_t inputCount, char *output, uint64_t *outputLength);
ALError ALERPFDDecompressRIDs(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount);
ALError ALERPFDDecompressRIDs_set_bmap(const char *input, uint64_t inputLength,void **bmap);
/************ end of EPFD + RPFD METHOD ****************/

// NOTE: outputCount should hold the maximum output buffer count
ALError ALDecompressRIDs(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount);

ALError ALInpsectEncodedRIDs(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount
		, uint64_t b_stat[], uint64_t exp_stat[]);

ALError ALDecompressRIDs_inspect_decode_bmap(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount, void **bmap
		, uint64_t b_stat[], uint64_t exp_stat[]);


ALError ALDecompressRIDs_inspect_consecutive_binary_stat(
		const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount
		, uint64_t b_stat[], uint64_t exp_stat[]);
//
ALError ALDecompressRIDs_set_bmap(const char *input, uint64_t inputLength, void **bmap);

uint64_t ALGetMaxCompressedRIDLength(uint32_t inputCount);

ALError ALTranslateCompressedRIDs(char *input, uint64_t inputLength, int32_t rid_offset);

#ifdef __cplusplus
}
#endif


#endif /* ALACRITY_RID_COMPRESS_H_ */
