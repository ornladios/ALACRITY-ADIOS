#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <alacrity.h>
#include <alacrity-rid-compress.h>
#include "include/alacrity-util.h"

static ALError ALDecodeWithCompressedIndex(const ALPartitionData *input,
                                           void            *output,
                                           uint64_t        *outputCount);

static ALError ALDecodeWithInvertedIndex(const ALPartitionData *input,
                                         void            *output,
                                         uint64_t        *outputCount);

static ALError ALDecodeWithCompressedInvertedIndex(const ALPartitionData *input,
                                                   void            *output,
                                                   uint64_t        *outputCount);

/*
 * Returns how many bytes decoding the given ALACRITY partition will produce
 */
uint64_t ALGetDecodeLength(const ALPartitionData *input) {
    return input->metadata.partitionLength * input->metadata.elementSize;
}

/*
 * Convert ALACRITY form data back to raw data
 * Assumptions:
 *   - calling application (or library) takes
 *     care of (de)-linearization, contiguity
 */
ALError ALDecode(const ALPartitionData *input,
                 void            *output,
                 uint64_t        *outputCount) {
    switch (input->metadata.indexMeta.indexForm) {
    case ALCompressionIndex:
        return ALDecodeWithCompressedIndex(input, output, outputCount);
    case ALInvertedIndex:
        return ALDecodeWithInvertedIndex(input, output, outputCount);
    case ALCompressedInvertedIndex:
        return ALDecodeWithCompressedInvertedIndex(input, output, outputCount);
    default:
        fprintf(stderr, "%s: Error: unknown ALACRITY index form %d found during decoding\n", __FUNCTION__, input->metadata.indexMeta.indexForm);
        return ALErrorSomething; // TODO: Error code
    }
}

static ALError ALDecodeWithCompressedIndex(const ALPartitionData *input,
                                           void            *output,
                                           uint64_t        *outputCount) {
	*outputCount = 0;
    return ALErrorNone;
}

// Naive implementation, a better one would attempt better cache coherence by using some sort of block-based merging perhaps
static ALError ALDecodeWithInvertedIndex(const ALPartitionData *input,
                                         void            *output,
                                         uint64_t        *outputCount) {

    const ALBinLayout* const bl = &input->metadata.binLayout;
    const int elemsize = input->metadata.elementSize;
    const int sigbits = input->metadata.significantBits;
    const int sigbytes = alacrity_util_sigBytesCeil(&input->metadata);
    const int insigbytes = alacrity_util_insigBytesCeil(&input->metadata);

    char *dataptr = (char*)input->data;
    bin_offset_t *binvalptr = bl->binValues;
    const rid_t *indexptr = (rid_t*)input->index;;

    bin_id_t binStartOff = 0;
    low_order_bytes_t low;
    high_order_bytes_t hi;
    for (bin_id_t bin = 0; bin < bl->numBins; bin++) {
        const bin_offset_t binEndOff = bl->binStartOffsets[bin + 1];
        hi = *binvalptr;

        for (bin_offset_t elempos = binStartOff; elempos < binEndOff; elempos++) {
            GET_BUFFER_ELEMENT(low, dataptr, insigbytes);

            rid_t rid = *indexptr;

            REJOIN_DATUM_BITS((char*)output + rid * elemsize, elemsize, sigbits, hi, low);

            dataptr += insigbytes;
            indexptr++;

        }

        binStartOff = binEndOff;
        binvalptr ++;
    }

    *outputCount = input->metadata.partitionLength;//ALGetDecodeLength(input);

    return ALErrorNone;
}

// TODO: Optimize to use less memory by decompressing the inverted index a
//       block at a time, using each block and discarding before getting the
//       next, as opposed to decompressing the whole bin, as is done currently.
static ALError ALDecodeWithCompressedInvertedIndex(const ALPartitionData *input,
                                                   void            *output,
                                                   uint64_t        *outputCount) {

    const ALBinLayout* const bl = &input->metadata.binLayout;
    const ALCompressedInvertedIndexMetadata* const ciim = &input->metadata.indexMeta.u.ciim;
    const int elemsize = input->metadata.elementSize;
    const int sigbits = input->metadata.significantBits;
    const int sigbytes = (input->metadata.significantBits + 0x07) >> 3;
    const int insigbytes = ((elemsize << 3) - input->metadata.significantBits + 0x07) >> 3; // & 0x07 = last 3 bits

    assert(input->metadata.indexMeta.indexForm == ALCompressedInvertedIndex);

    char *dataPtr = (char*)input->data;
    char *compressedIndexPtr = (char*)input->index;
    rid_t *indexPtr;

    bin_offset_t decompressBinBufLen = 0;
    rid_t *decompressedBinBuf = NULL;

    bin_offset_t decompressBinLen = 0;
    uint64_t compressedBinLen;

    bin_id_t binStartOff = 0, binEndOff, binLen;
    low_order_bytes_t low;
    high_order_bytes_t hi;
    for (bin_id_t bin = 0; bin < bl->numBins; bin++) {
        binEndOff = bl->binStartOffsets[bin + 1];
        binLen = binEndOff - binStartOff;

        // First decompress index bin, since we need the RIDs to reconstitue the data
        // Allocate memory for the decompressed index
        if (decompressBinBufLen < binLen) {
            FREE(decompressedBinBuf);
            decompressBinBufLen = binLen;
            decompressedBinBuf = (rid_t *)malloc(decompressBinBufLen * sizeof(rid_t));
        }

		assert (decompressedBinBuf != 0);
        // Perform the decompression
        compressedBinLen = ciim->indexBinStartOffsets[bin + 1] - ciim->indexBinStartOffsets[bin];
        ALDecompressRIDs(compressedIndexPtr, compressedBinLen, decompressedBinBuf, &decompressBinLen);
        assert(decompressBinLen == binLen);

        // Update pointers
        compressedIndexPtr += compressedBinLen;
        indexPtr = decompressedBinBuf;

        // Now, continue decoding as before, but using the decompressed index
        // buffer rather than the original input
        hi = bl->binValues [bin];
        for (bin_offset_t elempos = binStartOff; elempos < binEndOff; elempos++) {
            GET_BUFFER_ELEMENT(low, dataPtr, insigbytes);

            rid_t rid = *indexPtr;

            REJOIN_DATUM_BITS((char*)output + rid * elemsize, elemsize, sigbits, hi, low);
            // REJOIN_DATUM((char*)output + rid, elemsize, sigbytes, hi, low);

            dataPtr += insigbytes;
            indexPtr++;
        }
        binStartOff = binEndOff;
    }

    *outputCount = input->metadata.partitionLength;//ALGetDecodeLength(input);

    return ALErrorNone;
}
