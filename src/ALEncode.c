#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Needed for memcpy

#include <assert.h>
#include <alacrity.h>
#include <ALUtil.h>

#include "alacrity-serialization-debug.h"

typedef enum {
    ALList,        //array of unique counts
    ALBitmap,      //bitmap array (with array of unique counts)
    ALCompBitmap,  //compressed bitmap array (with array of unique counts)
    ALMap          //map data structure, high-order byte key, count value
} ALUniqueEncodeMethod;

static void determineBinLayout(const ALEncoderConfig    *config,
                               const void               *input,
                               uint64_t                 inputCount,

                               ALBinLayout              *binLayout,
                               _Bool                    buildLookupTable,
                               ALBinLookupTable         *invertedLookupTable);

static void initMetadataAndAllocateBuffers(ALPartitionData *part, const ALEncoderConfig *config, uint64_t inputCount, const ALBinLayout *binLayout);

static ALError encodeWithCompressionIndex(ALPartitionData *part, const ALEncoderConfig *config, const void *input, uint64_t inputCount, const ALBinLookupTable *binLookupTable);
static ALError encodeWithInvertedIndex(ALPartitionData *part, const ALEncoderConfig *config, const void *input, uint64_t inputCount, const ALBinLookupTable *binLookupTable);

ALError ALBuildBinLayout (const ALEncoderConfig    *config,
                        const void               *input,
                        uint64_t                inputCount,
                        ALBinLookupTable *binLookupTable,
                        ALPartitionData         *output)
{
    // First, scan the original data to determine the bin sizes and offsets, and to produce a value-to-bin lookup table if significantBytes is small enough
    _Bool buildLookupTable = true; //config->significantBytes <= 2;
    ALBinLayout binLayout;
    determineBinLayout(config, input, inputCount, &binLayout, buildLookupTable, binLookupTable);

    // Next initialize the output partition, including metadata and data/index buffers
    initMetadataAndAllocateBuffers(output, config, inputCount, &binLayout);

    return ALErrorNone;
}

ALError ALBuildInvertedIndexFromLayout (const ALEncoderConfig    *config,
                                        const void               *input,
                                        uint64_t                inputCount,
                                        ALBinLookupTable *binLookupTable,
                                        ALPartitionData         *output)
{
    _Bool buildLookupTable = true;
    return encodeWithInvertedIndex(output, config, input, inputCount, binLookupTable);
}

/* Perform compression/index phase of ALACRITY
 * Assumptions:
 *   - calling application (or library) takes
 *     care of linearization, contiguity
 */
ALError ALEncode(const ALEncoderConfig    *config,
                 const void               *input,
                 uint64_t                inputCount,
                 ALPartitionData         *output) {

    // First, scan the original data to determine the bin sizes and offsets, and to produce a value-to-bin lookup table if significantBytes is small enough
    _Bool buildLookupTable = true;//config->significantBytes <= 2;
    ALBinLayout binLayout;
    ALBinLookupTable binLookupTable;
    determineBinLayout(config, input, inputCount, &binLayout, buildLookupTable, &binLookupTable);

    // Next initialize the output partition, including metadata and data/index buffers
    initMetadataAndAllocateBuffers(output, config, inputCount, &binLayout);

    // Now, scan the original data again and convert to ALACRITY format
    ALError err;
    if (config->indexForm == ALCompressionIndex) {
        dbprintf("Encoding with compressed index...\n");
        err = encodeWithCompressionIndex(output, config, input, inputCount, buildLookupTable ? &binLookupTable : NULL);
    } else if (config->indexForm == ALInvertedIndex) {
        dbprintf("Encoding with inverted index...\n");
        err = encodeWithInvertedIndex(output, config, input, inputCount, buildLookupTable ? &binLookupTable : NULL);
    } else if (config->indexForm == ALCompressedInvertedIndex) {
        eprintf("ERROR: Encoding with an index form of ALCompressedInvertedIndex is not supported right now."
                "Instead, encode with inverted index, then compress the index separately using ALCompressInvertedIndex()\n");
        err = ALErrorSomething;
    } else {
        // Unsupported right now
        // return ALErrorUnsupportedIndexForm
    }

    FREE(binLookupTable);
    return err;
}

void determineBinLayout(const ALEncoderConfig    *config,
                        const void                *input,
                        uint64_t                inputCount,

                        ALBinLayout                *binLayout,
                        _Bool                    buildLookupTable,
                        ALBinLookupTable        *invertedLookupTable) {

    const uint64_t num_possible_bins = 1ULL << (config->significantBits);
    const void *endptr = (char*)input + inputCount * config->elementSize;
    const char elemsize = config->elementSize;
    const char sigbits = config->significantBits;
    const char sigbytes = (sigbits + 0x07) >> 3;

    // Sriram
    dbprintf ("Number of possible bins: %llu and therefore sigbytes = %d\n", num_possible_bins, sigbytes);

    if (buildLookupTable) {
        dbprintf("Building an inverted lookup table...\n");
        // Use calloc to CLEAR the memory first (how did this never trigger a bug until now?)
        bin_offset_t *countTable = calloc(num_possible_bins, sizeof(bin_offset_t)); // countTable[hibytes] = count, sizeof(bin_offset_t) == max possible values in a bin

        //*invertedLookupTable = malloc(num_possible_bins * config->significantBytes); // invertedLookupTable[hibytes] = binID

        // Scan data and count instances of each high-order bytes value
        high_order_bytes_t hi = 0;
        low_order_bytes_t lo = 0;
        bin_id_t numBins = 0;
        for (const char *dataptr = input; dataptr != endptr; dataptr += elemsize) {
            SPLIT_DATUM_BITS(dataptr, elemsize, sigbits, hi, lo);
            countTable[hi]++;
            if (countTable[hi] == 1) {
                numBins++;
            }
        }

        dbprintf("%lu bins detected\n", numBins);
        binLayout->numBins = numBins;
        binLayout->binValues = malloc(numBins * sizeof (bin_id_t));
        binLayout->binStartOffsets = malloc((numBins + 1) * sizeof(bin_offset_t));

        *invertedLookupTable = (ALBinLookupTable)countTable; // Reuse the count table as an inverted lookup table. Possible since sizeof(bin_id_t) <= sizeof(partition_length_t) and the lists are read/written sequentially

        // Scan the count table and construct the bin layout, as well as the inverted lookup table
        bin_id_t curBinID = 0;
        bin_offset_t curEndOffset = 0;

        binLayout->binStartOffsets [0] = 0;
        high_order_bytes_t possibleBinValue = 0;

        FOR_BIN_VALUE_IN_1C_ORDER(possibleBinValue, config->significantBits, {
        //for (possibleBinValue = 0; possibleBinValue < num_possible_bins; possibleBinValue ++) {
            if (countTable[possibleBinValue] == 0) continue;
            curEndOffset += countTable[possibleBinValue];

            binLayout->binValues [curBinID] = possibleBinValue;
            binLayout->binStartOffsets[curBinID + 1] = curEndOffset;
            (*invertedLookupTable)[possibleBinValue] = curBinID++; // Overwrites countTable[possibleBinValue], but that's OK, we're done with this position
        });
    } else {
        // TODO: Implement
        *invertedLookupTable = NULL;

        // Find unique values
        // Sort them (or keep them sorted)
        // Bin ID = position in list
        // Count how many for each, as well
        // Set up bin layout
    }
}

void initMetadataAndAllocateBuffers(ALPartitionData *part, const ALEncoderConfig *config, uint64_t inputCount, const ALBinLayout *binLayout) {
    const char sigbytes = (config->significantBits + 0x07) >> 3;
    const char elemsize = config->elementSize;
    const char insigbytes = ((elemsize << 3) - config->significantBits + 0x07) >> 3;
    ALMetadata *meta = &part->metadata;

    part->data = malloc(inputCount * insigbytes);
    if (config->indexForm == ALCompressionIndex)
        part->index = malloc(inputCount * sigbytes);
    else if (config->indexForm == ALInvertedIndex)
        part->index = malloc(inputCount * sizeof(rid_t));
    else
        fprintf(stderr, "Unknown index form %d\n", config->indexForm);

    part->ownsBuffers = true;

    meta->binLayout = *binLayout;
    meta->elementSize = elemsize;
    meta->indexMeta.indexForm = config->indexForm;
    meta->globalOffset = 0;
    meta->partitionLength = inputCount;
    meta->significantBits = config->significantBits;
    meta->datatype = config->datatype;
    meta->endianness = detectEndianness();
}

static inline bin_id_t lookupBinID(high_order_bytes_t value, const ALBinLookupTable *binLookupTable) {
    return (*binLookupTable)[value];
}

// Sriram
ALError encodeWithInvertedIndex(ALPartitionData *part, const ALEncoderConfig *config, const void *input, uint64_t inputCount, const ALBinLookupTable *binLookupTable) {
    // Local constants for brevity/efficiency below
    const char sigbits = config->significantBits;
    const char sigbytes = (sigbits + 0x07) >> 3;
    const char elemsize = config->elementSize;
    const char insigbytes = ((elemsize << 3) - sigbits + 0x07) >> 3;
    const ALBinLayout *binLayout = &part->metadata.binLayout;
    const void *endptr = (char*)input + inputCount * elemsize;

    rid_t rID = 0;
    bin_id_t binID = 0;

    // Set up a pointer to the current position within the index
    rid_t *invertedIndex = (rid_t*)part->index;
    const char *dataptr = 0;

    // Now, iterate over all elements of the raw data
    high_order_bytes_t hi;
    low_order_bytes_t lo;

    // Set up bin offset table. The binStartOffsets from the bin layout is what we want
    //
    // As we process the data, these offsets will be incremented, and at the end, this list should be exactly
    // equal to the "binEndOffsets"
    bin_offset_t *binCurOffsets = (bin_offset_t *) malloc (sizeof (bin_offset_t) * binLayout->numBins);
    memcpy (binCurOffsets, binLayout->binStartOffsets, sizeof (bin_offset_t) * binLayout->numBins);

    for (dataptr = input, rID = 0; dataptr != endptr; dataptr += elemsize, rID ++) {

        // Split the datum into hi- and low-order bytes
        SPLIT_DATUM_BITS(dataptr, elemsize, sigbits, hi, lo);

        // Look up the bin ID corresponding to the hi-order bytes, as well as the current fill offset for that bin
        bin_id_t binID = lookupBinID(hi, binLookupTable);

        bin_offset_t binOffset = binCurOffsets[binID]; // Get and increment
        binCurOffsets[binID] ++;

        // Copy the low-order bytes to the data buffer
        SET_ITH_BUFFER_ELEMENT(part->data, binOffset, lo, insigbytes);

        //VARLEN_TYPE_TO_MEM(lo, part->data + (binOffset * insigbytes), insigbytes);

        // Copy the bin ID to the index buffer
        //SET_ITH_BUFFER_ELEMENT(part->index, binOffset, rID, sizeof (rid_t));
        //*invertedIndexPtr++ = rID;
        invertedIndex[binOffset] = rID;

        dbprintf("High/low parts: %08lx/%016llx, binID %lu, rID %lu\n", hi, lo, binID, rID);
    }

    FREE(binCurOffsets);

    return ALErrorNone;
}

ALError encodeWithCompressionIndex(    ALPartitionData *part, const ALEncoderConfig *config,
                                    const void *input, uint64_t inputCount, const ALBinLookupTable *binLookupTable) {
    // Local constants for brevity/efficiency below
    const char elemsize = config->elementSize;
    const char sigbits = config->significantBits;
    const char insigbits = (elemsize << 3) - sigbits;

    const char sigbytes = (sigbits + 0x07) >> 3;
    const char insigbytes = (insigbits + 0x07) >> 3;

    const ALBinLayout *binLayout = &part->metadata.binLayout;
    const void *endptr = (char*)input + inputCount * elemsize;

    dbprintf("Breaking %d byte elements into %d/%d\n", elemsize, sigbits, insigbits);

    // Set up bin offset table. The binEndOffsets from the bin layout is almost what we want, but instead of
    // offsets to the ends of the bins, we need offsets to their starts. Therefore we do a "right-shift" copy,
    // such that binCurOffsets[0] = 0 and binCurOffsets[i+1] = binEndOffsets[i].
    //
    // As we process the data, these offsets will be incremented, and at the end, this list should be exactly
    // equal to binEndOffsets.
    bin_offset_t *binCurOffsets = (bin_offset_t *) malloc (sizeof (bin_offset_t) * (binLayout->numBins + 1));
    memcpy (binCurOffsets, binLayout->binStartOffsets, sizeof (bin_offset_t) * (binLayout->numBins + 1));

    // Set up a pointer to the current position within the index
    char *compressedIndexPtr = part->index;

    // Now, iterate over all elements of the raw data
    high_order_bytes_t hi;
    low_order_bytes_t lo;

    for (const char *dataptr = input; dataptr != endptr; dataptr += elemsize) {
        // Split the datum into hi- and low-order bytes
        SPLIT_DATUM_BITS(dataptr, elemsize, sigbits, hi, lo);

        // Look up the bin ID corresponding to the hi-order bytes, as well as the current fill offset for that bin
        bin_id_t binID = lookupBinID(hi, binLookupTable);
        bin_offset_t binOffset = binCurOffsets[binID]++; // Get and increment

        // Copy the low-order bytes to the data buffer
        SET_ITH_BUFFER_ELEMENT(part->data, binOffset, lo, insigbytes);
        //VARLEN_TYPE_TO_MEM(lo, part->data + (binOffset * insigbytes), insigbytes);

        // Copy the bin ID to the index buffer
        SET_BUFFER_ELEMENT(compressedIndexPtr, binID, sigbytes);
        //VARLEN_TYPE_TO_MEM(binID, compressedIndexPtr, sigbytes);

        // Advance the index buffer pointer
        compressedIndexPtr += sigbytes;
    }

    FREE(binCurOffsets);
    return ALErrorNone;
}

