/*
 * ALRIDTranslate.c
 *
 *  Created on: Sep 27, 2012
 *      Author: David A. Boyuka II
 */

#include <alacrity-types.h>
#include <alacrity-rid-compress.h>
#include <alacrity-serialization.h>
#include <ALUtil.h>

ALError ALTranslateRIDs(const ALMetadata *metadata,
                        ALIndex *index,
                        int32_t rid_offset) {
    switch (metadata->indexMeta.indexForm) {
    case ALInvertedIndex:
    {
        rid_t *indexPtr = (rid_t*)*index;
        rid_t *indexEndPtr = indexPtr + metadata->partitionLength;
        for (; indexPtr != indexEndPtr; indexPtr++)
            *indexPtr += rid_offset;
        break;
    }

    case ALCompressedInvertedIndex:
    {
        bin_id_t bin;
        uint64_t *compBinOffsets = metadata->indexMeta.u.ciim.indexBinStartOffsets;
        for (bin = 0; bin < metadata->binLayout.numBins; bin++) {
            ALTranslateCompressedRIDs(*index + compBinOffsets[bin],
                                      compBinOffsets[bin+1] - compBinOffsets[bin],
                                      rid_offset);
        }
        break;
    }

    case ALCompressionIndex:
    default:
        eprintf("[%s] Error: Index form $d is not supported for RID translation at this time\n",
                __FUNCTION__, metadata->indexMeta.indexForm);
        return ALErrorSomething;
    }

    return ALErrorNone;
}
