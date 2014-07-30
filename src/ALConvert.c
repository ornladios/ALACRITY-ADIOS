#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "alacrity.h"
#include <alacrity-rid-compress.h>
#include "ALUtil.h"

static ALError convertIIToCII(ALMetadata *metadata,
                              ALIndex *indexPtr, ALIndexForm toindexForm ) {
    const ALBinLayout const *bl = &metadata->binLayout;
    const int elemsize = metadata->elementSize;
    const int sigbytes = sigBytesCeil(metadata);
    const int insigbytes = insigBytesCeil(metadata);
    const rid_t * input_index = (rid_t*)*indexPtr;

    bin_offset_t binLen;
    uint64_t binCompressedLen;

    uint64_t new_index_wcsize = 0;
    for (bin_id_t bin = 0; bin < bl->numBins; bin++) {
        binLen = bl->binStartOffsets[bin + 1] - bl->binStartOffsets[bin];
        if (toindexForm == ALCompressedInvertedIndex || toindexForm == ALCompressedHybridInvertedIndex
        		|| toindexForm == ALCompressedExpansionII
        		|| toindexForm == ALCompressedMixInvertedIndex ){
			new_index_wcsize += ALGetMaxCompressedRIDLength(binLen);
        }else if (toindexForm == ALCompressedSkipInvertedIndex){
        	new_index_wcsize += ALSkippingSufficientBufferSize(binLen);
        }else {
        	printf("not support index form \n");
        	return ALErrorSomething;
        }
    }
    char *output_index = malloc(new_index_wcsize);

    const rid_t *inputCurPtr = input_index;
    char *outputCurPtr = *indexPtr = output_index;

    // Set up new metadata
    uint64_t *indexBinStartOffsets = (uint64_t*)malloc((bl->numBins + 1) * sizeof(uint64_t));

    // Now compress each bin in turn
    for (bin_id_t bin = 0; bin < bl->numBins; bin++) {
        binLen = bl->binStartOffsets[bin + 1] - bl->binStartOffsets[bin];

        // compressRIDs can work in place
        // for toindexForm == ALCompressedInvertedIndex || toindexForm == ALCompressedHybridInvertedIndex
        binCompressedLen = ALGetMaxCompressedRIDLength(binLen);

        if (toindexForm == ALCompressedSkipInvertedIndex) {
        	binCompressedLen = ALSkippingSufficientBufferSize(binLen);
        }else {
        	binCompressedLen = binLen * sizeof(rid_t); // Max compressed length
        }
        if (toindexForm == ALCompressedInvertedIndex)
        	ALCompressRIDs(inputCurPtr, binLen, outputCurPtr, &binCompressedLen);
        else if (toindexForm == ALCompressedHybridInvertedIndex)
        /**********Switch to hybrid compress method**************/
			ALHybridCompressRIDs(inputCurPtr, binLen, outputCurPtr, &binCompressedLen);
        else if (toindexForm == ALCompressedSkipInvertedIndex){
	//	printf("skipping conversion \n");
        	ALSkippingCompressRIDs(inputCurPtr, binLen, outputCurPtr, &binCompressedLen);
        }else if (toindexForm == ALCompressedExpansionII){
        	// expansion PFD, EPFD
        	ALExpandCompressRIDs(inputCurPtr, binLen, outputCurPtr, &binCompressedLen);

        }else if (toindexForm == ALCompressedMixInvertedIndex ){
        	//EPFD + RPFD
            ALERPFDCompressRIDs(inputCurPtr, binLen, outputCurPtr, &binCompressedLen);
        }



        indexBinStartOffsets[bin] = outputCurPtr - output_index;
        inputCurPtr += binLen;
        outputCurPtr += binCompressedLen;
    }

    // Insert new metadata
    indexBinStartOffsets[bl->numBins] = outputCurPtr - output_index;
    metadata->indexMeta.u.ciim.indexBinStartOffsets = indexBinStartOffsets;
    metadata->indexMeta.indexForm = toindexForm;

    assert(new_index_wcsize >= outputCurPtr - output_index); // Ensure we didn't overrun
    output_index = realloc(output_index, outputCurPtr - output_index); // Shrink the index down to max size
    FREE(input_index);

    return ALErrorNone;
}


static ALError convertCIIToII(ALMetadata *metadata,
                              ALIndex *indexPtr,
                              bin_id_t lo_bin,
                              bin_id_t hi_bin,
                              _Bool updateMeta) {
    const ALBinLayout const *bl = &metadata->binLayout;
    const bin_offset_t lo_bin_off = bl->binStartOffsets[lo_bin];
    const bin_offset_t hi_bin_off = bl->binStartOffsets[hi_bin];
    const bin_offset_t outputCount = hi_bin_off - lo_bin_off;

    const char * input_index = (char*)*indexPtr;
    rid_t *output_index = malloc(outputCount * sizeof(rid_t));

    const char *inputCurPtr = input_index;
    rid_t *outputCurPtr = output_index;
    *indexPtr = (ALIndex)output_index;

    const uint64_t *compBinStartOffs = metadata->indexMeta.u.ciim.indexBinStartOffsets;

    bin_offset_t binLen;
    uint64_t binCompressedLen;

    // Now compress each bin in turn
    for (bin_id_t bin = lo_bin; bin < hi_bin; bin++) {
        binLen = bl->binStartOffsets[bin + 1] - bl->binStartOffsets[bin];
        binCompressedLen = compBinStartOffs[bin + 1] - compBinStartOffs[bin];

        ALDecompressRIDs(inputCurPtr, binCompressedLen, outputCurPtr, &binLen);

        assert(binLen == bl->binStartOffsets[bin + 1] - bl->binStartOffsets[bin]);

        inputCurPtr += binCompressedLen;
        outputCurPtr += binLen;
    }

    if (updateMeta) {
        // Insert new metadata
        free(metadata->indexMeta.u.ciim.indexBinStartOffsets);
        metadata->indexMeta.indexForm = ALInvertedIndex;
    }

    FREE(input_index);
    return ALErrorNone;
}

ALError ALConvertIndexForm(ALMetadata *metadata,
                           ALIndex *index,
                           ALIndexForm newForm) {
    const ALIndexForm oldForm = metadata->indexMeta.indexForm;

    if (oldForm == ALInvertedIndex &&
        newForm >= ALCompressedInvertedIndex) {

    	return convertIIToCII(metadata, index, newForm);

    } /*else if  (oldForm == ALInvertedIndex &&
            newForm == ALCompressedHybridInvertedIndex) {

    	return  convertIIToHII(metadata, index);

    } else if (oldForm == ALInvertedIndex &&
            newForm == ALCompressedSkipInvertedIndex) {

    	return convertIIToKII(metadata,index);

    } else if (oldForm == ALInvertedIndex &&
            newForm == ALCompressedMixInvertedIndex) {

    	return convertIIToMII(metadata,index);

    }*/ else if (oldForm == ALCompressedInvertedIndex &&
               newForm == ALInvertedIndex) {
       return convertCIIToII(metadata, index, 0, metadata->binLayout.numBins, true);
    } else {
        eprintf("ERROR: Conversion from index form %d to %d is not supported at this time\n",
                oldForm, newForm);
        return ALErrorSomething;
    }
}

// Converts a contiguous range of bins of the index to a new form.
// Currently only CII->II is supported, because:
// 1. Compression index to/from [compressed] inverted index requires the full index
// 2. II -> CII requires new metadata to describe, and since we can't edit the metadata,
//    this can't be done
ALError ALConvertPartialIndexForm(const ALMetadata *metadata,
                                  ALIndex *partialIndex,
                                  ALIndexForm newForm,
                                  bin_id_t lo_bin,
                                  bin_id_t hi_bin) {
    const ALIndexForm oldForm = metadata->indexMeta.indexForm;

    if (oldForm == ALCompressedInvertedIndex &&
        newForm == ALInvertedIndex) {
        return convertCIIToII(metadata, partialIndex, lo_bin, hi_bin, false);
    } else {
        eprintf("ERROR: Conversion from index form %d to %d is not supported at this time\n",
                oldForm, newForm);
        return ALErrorSomething;
    }
}



