#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Needed for memcpy

#include <ALUtil.h>
#include <memstream.h>
#include <alacrity-types.h>
#include <alacrity-serialization.h>
#include <alacrity-serialization-legacy.h>
#include <alacrity-serialization-debug.h>


// Custom memstream append/read functions for efficiency
MAKE_STATIC_APPEND_AND_READ_FUNCS(PartitionLength, partition_length_t)
MAKE_STATIC_APPEND_AND_READ_FUNCS(IndexForm, ALIndexForm)
MAKE_STATIC_APPEND_AND_READ_FUNCS(BinID, bin_id_t)

// ************************************************
// (De)serialization data size calculations
// ************************************************

/*
 * INPUT: ALMetadata object
 * OUTPUT: size in bytes consumed by the metadata object in legacy serialized format
 */
uint64_t ALGetMetadataSizeLegacy(const ALMetadata *metadata) {
    return	sizeof(uint64_t) +				// Partition length (in elements)
            sizeof(uint16_t) +				// Number of bins
            metadata->binLayout.numBins * (	// For each bin...
                sizeof(uint16_t) +				// Bin value
                sizeof(uint64_t) +				// Bin offset (in bytes)
                sizeof(uint64_t) +				// Bin offset (in elements
                sizeof(char)					// Is bin compressed?
            );
}



// ************************************************
// (De)serialization functions
// ************************************************

// ************************************************
// (De)serialization functions
// ************************************************

/*
 * Serializes the metadata in the legacy ALACRITY format
 * INPUT: 	ALMetadata *metadata
 * OUTPUT: 	void *output, uint64_t *output_size
 * RETURN: 	ALError
 *
 * NOTE:    We can't handle elementSize != 8 or significantBytes != 2 because the legacy file format is rigid and can't handle it.
 *          For instance, we would have to write a variable-length integer for numBins before we even stored how long it is (significantBytes)
 */
ALError ALSerializeMetadataLegacy(const ALMetadata *metadata, memstream_t *ms) {
    if (metadata->elementSize != 8 || metadata->significantBits != 16) {
        if (metadata->elementSize != 8)
            fprintf(stderr, "Error: %s cannot serialize partition because elementSize is %d, must be 8\n", __FUNCTION__, (int)metadata->elementSize);
        else
            fprintf(stderr, "Error: %s cannot serialize partition because significantBits is %d, must be 16\n", __FUNCTION__, (int)metadata->significantBits);
        return ALErrorSomething;
    }

    // Serialize partition length and bin count
    memstreamAppendUint64(ms, (uint64_t)metadata->partitionLength);
    memstreamAppendUint16(ms, metadata->binLayout.numBins);

    // Serialize bin values
    memstreamAppendArray(ms, metadata->binLayout.binValues, sizeof(unsigned short int), metadata->binLayout.numBins);

    //////
    // START serialization of bin offsets (this is a bit tricky)
    //////
    // Calculate legacy-translated bin byte offsets
    // (scale to byte, instead of element, offsets)
    uint64_t *bin_offsets = malloc(metadata->binLayout.numBins * sizeof(uint64_t));
    for (bin_id_t i = 0; i < metadata->binLayout.numBins; i++)
        bin_offsets[i] = metadata->binLayout.binStartOffsets[i] * 6; // Offset is scaled by the low-byte element size (6)

    // Serialize the translated bin byte offsets
    memstreamAppendArray(ms, bin_offsets, sizeof(uint64_t), metadata->binLayout.numBins);

    // Translate back to bin element offsets
    for (bin_id_t i = 0; i < metadata->binLayout.numBins; i++)
        bin_offsets[i] /= 6; // Undo the element scaling to get element offsets again

    // Serialize the translated bin element offsets
    memstreamAppendArray(ms, bin_offsets, sizeof(uint64_t), metadata->binLayout.numBins);

    // Free the temp array
    free(bin_offsets);
    //////
    // END serialization of bin offsets (phew)
    //////

    // Serialize bin compression flags (not used by us, always false)
    memstreamFill(ms, 0, metadata->binLayout.numBins * sizeof(char));

    return ALErrorNone;
}

ALError ALDeserializeMetadataLegacy(ALMetadata *metadata, memstream_t *ms) {
    // Fixed in the legacy format
    metadata->elementSize = 8;
    metadata->significantBits = 16;
    metadata->endianness = ENDIAN_LITTLE;
    metadata->datatype = DATATYPE_FLOAT64;
    metadata->indexMeta.indexForm = ALInvertedIndex; // Assume this for now

    // Deserialize partition length and bin count
    metadata->partitionLength = memstreamReadUint64(ms);
    metadata->binLayout.numBins = memstreamReadUint16(ms);

    metadata->binLayout.binValues = malloc(metadata->binLayout.numBins * (metadata->significantBits >> 3));
    metadata->binLayout.binStartOffsets = malloc((metadata->binLayout.numBins + 1) * sizeof(bin_offset_t));

    // Serialize bin values
    memstreamReadArray(ms, metadata->binLayout.binValues, sizeof(unsigned short int), metadata->binLayout.numBins);

    //////
    // START deserialization of bin offsets (this is a bit tricky)
    //////
    // Ignore bin byte offsets, since we don't care
    memstreamSkip(ms, sizeof(uint64_t) * metadata->binLayout.numBins);

    // Deserialize the bin element offsets (start offsets)
    uint64_t *bin_offsets = malloc(metadata->binLayout.numBins * sizeof(uint64_t));
    memstreamReadArray(ms, bin_offsets, sizeof(uint64_t), metadata->binLayout.numBins);

    // Now left-shift all offsets to make them end offsets instead
    for (bin_id_t i = 0; i < metadata->binLayout.numBins; i++)
        metadata->binLayout.binStartOffsets[i] = bin_offsets[i];
    metadata->binLayout.binStartOffsets[metadata->binLayout.numBins] = metadata->partitionLength;

    free(bin_offsets);

    //////
    // END deserialization of bin offsets
    //////

    // Skip the bin compression flags, since we don't care
    memstreamSkip(ms, metadata->binLayout.numBins * sizeof(char));

    return ALErrorNone;
}
