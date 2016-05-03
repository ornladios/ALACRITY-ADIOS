#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Needed for memcpy

#include <alacrity-types.h>
#include <alacrity-serialization.h>
#include <alacrity-serialization-debug.h>
#include "include/alacrity-memstream.h"
#include "include/alacrity-util.h"

//
// Important notes:
// >> All ALSerialize* functions assume that all buffers in ALPartitionData are already allocated
//    (obviously, since it is being called to write data that already exists)
// >> All ALDeserialize* functions will malloc() buffers in ALPartitionData as needed, since it
//    assumed that the caller did not know how large they should be, and so has not already allocated them
//

// Custom memstream append/read functions for efficiency
MAKE_STATIC_APPEND_AND_READ_FUNCS(GlobalOffset, global_rid_t)
MAKE_STATIC_APPEND_AND_READ_FUNCS(PartitionLength, partition_length_t)
MAKE_STATIC_APPEND_AND_READ_FUNCS(IndexForm, ALIndexForm)
MAKE_STATIC_APPEND_AND_READ_FUNCS(BinID, bin_id_t)
MAKE_STATIC_APPEND_AND_READ_FUNCS(Datatype, ALDatatype)

// ************************************************
// (De)serialization data size calculations
// ************************************************

/*
 * INPUT: ALMetadata object
 * OUTPUT: size in bytes consumed by the metadata object in serialized format
 */
uint64_t ALGetMetadataSize(const ALMetadata *metadata) {
    return  sizeof(global_rid_t) +                                                  // Global RID offset
            sizeof(partition_length_t) +                                            // Partition length
            ALGetBinLayoutSize(&metadata->binLayout, metadata->significantBits) +   // Bin layout metadata
            ALGetIndexMetadataSize(&metadata->indexMeta, metadata) +                // Index metadata
            sizeof(char) +                                                          // Significant bytes
            sizeof(char) +                                                          // Element size
            sizeof(ALDatatype) +                                                    // Datatype
            sizeof(char);                                                           // Endianness
}

uint64_t ALGetIndexMetadataSize(const ALIndexMetadata *indexMeta, const ALMetadata *metadata) {
    uint64_t size = sizeof(indexMeta->indexForm);
    switch (indexMeta->indexForm) {
    case ALCompressionIndex:
    case ALInvertedIndex:
        break;
    case ALCompressedInvertedIndex:
        size += (metadata->binLayout.numBins + 1) * sizeof(indexMeta->u.ciim.indexBinStartOffsets[0]);
        break;
    case ALCompressedHybridInvertedIndex:
    case ALCompressedSkipInvertedIndex:
    case ALCompressedMixInvertedIndex:
    case ALCompressedExpansionII:
    	size += (metadata->binLayout.numBins + 1) * sizeof(indexMeta->u.ciim.indexBinStartOffsets[0]);
    	break;
    default:
        abort();
    }

    return size;
}

/*
 * INPUT: ALIndex object
 * OUTPUT: size in bytes consumed by the index object.
 */
uint64_t ALGetIndexSize(const ALIndex *index, const ALMetadata *metadata) {
    const uint8_t sigbytes = alacrity_util_sigBytesCeil(metadata);
    switch (metadata->indexMeta.indexForm) {
    case ALInvertedIndex:
        return metadata->partitionLength * sizeof(rid_t);
    case ALCompressionIndex:
        return metadata->partitionLength * sigbytes;
    case ALCompressedInvertedIndex:
        return metadata->indexMeta.u.ciim.indexBinStartOffsets[metadata->binLayout.numBins];
    case ALCompressedHybridInvertedIndex:
    case ALCompressedSkipInvertedIndex:
    case ALCompressedMixInvertedIndex:
    case ALCompressedExpansionII:
        return metadata->indexMeta.u.ciim.indexBinStartOffsets[metadata->binLayout.numBins];
    default:
        fprintf(stderr, "%s: Invalid indexing method %d\n", __FUNCTION__, metadata->indexMeta.indexForm);
        return 0;
    }
}

uint64_t ALGetDataSize(const ALData *data, const ALMetadata *metadata) {
    const uint8_t insigbytes = ((metadata->elementSize << 3) - metadata->significantBits + 0x07) >> 3;
    return metadata->partitionLength * insigbytes;
}

/*
 * INPUT: ALBinLayout object
 * OUTPUT: size in bytes consumed by the binLayout object.
 */
uint64_t ALGetBinLayoutSize(const ALBinLayout *binLayout, uint8_t significantBits) {
    const uint8_t sigbytes = (significantBits + 0x07) >> 3;
    return    sizeof(binLayout->numBins) +                      // Number of bins
            (binLayout->numBins) * sizeof(bin_offset_t) +                   // Bin Values
            (binLayout->numBins + 1) * sizeof(bin_offset_t);    // The Bin Offsets
}

/*
 * INPUT: ALPartitionData object
 * OUTPUT: size in bytes consumed by the serialized ALPartitionData object.
 */
uint64_t ALGetPartitionDataSize(const ALPartitionData *partitionData) {
    return    ALGetMetadataSize(&partitionData->metadata) +                        // Metadata
            ALGetIndexSize(&partitionData->index, &partitionData->metadata) +    // Index
            ALGetDataSize(&partitionData->data, &partitionData->metadata);        // Data
}

// NOTE: This code was unused, but could be useful later. It must be updated to use the new
//       compressed inverted index deserialization
/*
static inline ALError ALDeserializeBinLayoutInPlace (ALBinLayout *binLayout, uint8_t significantBytes, memstream_t *binLayoutBuffer)
{
    memstreamAppendBinID (binLayoutBuffer, binLayout->numBins);
    binLayout->binValues = (void *) binLayoutBuffer->buf;
    binLayout->binStartOffsets = ((bin_id_t *) binLayoutBuffer->buf + binLayout->numBins * significantBytes);

    return ALErrorNone;

}

static inline ALError ALDeserializeMetadataInPlace (ALMetadata *metadata, memstream_t *metadataBuffer)
{
    memstreamAppendPartitionLength(metadataBuffer, metadata->partitionLength);
    memstreamAppendIndexForm(metadataBuffer, metadata->indexMeta.indexForm);
    memstreamAppendChar(metadataBuffer, metadata->significantBits);
    memstreamAppendChar(metadataBuffer, metadata->elementSize);

    ALDeserializeBinLayoutInPlace (&(metadata->binLayout), bytesNeeded (metadata->significantBits), metadataBuffer);

    return ALErrorNone;
}
*/

/*
 * INPUT: Pointer to the start of the ALBinLayout buffer
 * OUTPUT: numBins
 */
#define ALGetBinLayoutNumBins(buf, numBins) { \
    numBins = * (uint32_t *) buf; \
}

/*
 * INPUT: Pointer to the START of the ALBinLayout buffer
 * OUTPUT: Pointer to the start of the binValues
 */
#define ALGetBinLayoutBinValuesPtr(buf, binValues) { \
    binValues = buf + sizeof (uint32_t); \
}
/*
 * INPUT: Pointer to the START of the ALBinLayout buffer
 * OUTPUT: Pointer to the start of the binOffsets
 */
#define ALGetBinLayoutBinEndOffsetsPtr(buf, binEndOffsets) { \
    uint32_t numBins = 0; \
    ALGetBinLayoutNumBins (buf, numBins); \
    binEndOffsets = buf + sizeof (numBins) + numBins * sizeof (bin_offset_t); \
}

// ************************************************
// (De)serialization functions
// ************************************************

/*
 * INPUT:      ALBinLayout *binLayout
 * OUTPUT:     memstream_t *ms (bin layout is serialized into the memstream)
 * RETURN:     ALError
 */
ALError ALSerializeBinLayout(const ALBinLayout *binLayout, uint8_t significantBits, memstream_t *ms) {
    const uint8_t sigbytes = (significantBits + 0x07) >> 3;

    memstreamAppendBinID(ms, binLayout->numBins);
    memstreamAppendArray(ms, binLayout->binValues, sizeof (bin_offset_t), binLayout->numBins);
    memstreamAppendArray(ms, binLayout->binStartOffsets, sizeof(bin_offset_t), (binLayout->numBins + 1));

    return ALErrorNone;
}


/*
 * INPUT:     memstream_t *ms
 * OUTPUT:     ALBinLayout *binLayout
 * RETURN:     ALError
 */
ALError ALDeserializeBinLayout(ALBinLayout *binLayout, uint8_t significantBits, memstream_t *ms) {
    const char sigbytes = (significantBits + 0x07) >> 3;

    // Read number of bins
    binLayout->numBins = memstreamReadBinID(ms);

    // Allocate and read bin value list
    binLayout->binValues = (bin_offset_t *) malloc(binLayout->numBins * sizeof (bin_offset_t));
    memstreamReadArray(ms, binLayout->binValues, sizeof (bin_offset_t), binLayout->numBins);

    binLayout->binStartOffsets = (bin_offset_t*)malloc((binLayout->numBins + 1) * sizeof(bin_offset_t));
    memstreamReadArray(ms, binLayout->binStartOffsets, sizeof(bin_offset_t), (binLayout->numBins + 1));

    return ALErrorNone;
}

ALError ALSerializeIndexMetadata(const ALIndexMetadata *indexMeta, const ALBinLayout *bl, memstream_t *ms) {
    memstreamAppendIndexForm(ms, indexMeta->indexForm);

    switch (indexMeta->indexForm) {
    case ALCompressedInvertedIndex:
    case ALCompressedHybridInvertedIndex: // inverted index compressed by p4d + RLE
    case ALCompressedSkipInvertedIndex: //  skipping inverted index compressed by p4d
    case ALCompressedMixInvertedIndex :  // skipping inverted index compressed by p4d + RLE
    case ALCompressedExpansionII:
        memstreamAppendArray(ms, indexMeta->u.ciim.indexBinStartOffsets, sizeof(uint64_t), bl->numBins + 1);
        break;
    default:
        break;
    }

    return ALErrorNone;
}

ALError ALDeserializeIndexMetadata(ALIndexMetadata *indexMeta, const ALBinLayout *bl, memstream_t *ms) {
    indexMeta->indexForm = memstreamReadIndexForm(ms);

    switch (indexMeta->indexForm) {

    case ALCompressedInvertedIndex:
    case ALCompressedHybridInvertedIndex: // inverted index compressed by p4d + RLE
    case ALCompressedSkipInvertedIndex: //  skipping inverted index compressed by p4d
    case ALCompressedMixInvertedIndex :  // skipping inverted index compressed by p4d + RLE
    case ALCompressedExpansionII:
        indexMeta->u.ciim.indexBinStartOffsets = (uint64_t *) malloc((bl->numBins + 1) * sizeof(uint64_t));
        memstreamReadArray(ms, indexMeta->u.ciim.indexBinStartOffsets, sizeof(uint64_t), bl->numBins + 1);
        break;
    default:
        break;
    }

    return ALErrorNone;
}

/*
 * INPUT:     ALMetadata *metadata
 * OUTPUT:     memstream_t *ms (metadata is serialized into the memstream)
 * RETURN:     ALError
 */
ALError ALSerializeMetadata(const ALMetadata *metadata, memstream_t *ms) {
    const char sigbytes = (metadata->significantBits + 0x07) >> 3;

    memstreamAppendGlobalOffset(ms, metadata->globalOffset);
    memstreamAppendPartitionLength(ms, metadata->partitionLength);
    memstreamAppendChar(ms, metadata->significantBits);
    memstreamAppendChar(ms, metadata->elementSize);
    memstreamAppendChar(ms, metadata->endianness);
    memstreamAppendDatatype(ms, metadata->datatype);

    // Serialize binLayout
    ALSerializeBinLayout(&metadata->binLayout, metadata->significantBits, ms);

    // Serialize indexMeta
    ALSerializeIndexMetadata(&metadata->indexMeta, &metadata->binLayout, ms);

    return ALErrorNone;
}

/*
 * INPUT:   memstream_t *ms
 * OUTPUT:  ALMetadata *metadata
 * RETURN:  ALError
 */
ALError ALDeserializeMetadata(ALMetadata *metadata, memstream_t *ms) {
    metadata->globalOffset    = memstreamReadGlobalOffset(ms);
    metadata->partitionLength = memstreamReadPartitionLength(ms);
    metadata->significantBits = memstreamReadChar(ms);
    metadata->elementSize     = memstreamReadChar(ms);
    metadata->endianness      = memstreamReadChar(ms);
    metadata->datatype        = memstreamReadDatatype(ms);

    const char sigbytes = (metadata->significantBits + 0x07) >> 3;

    ALDeserializeBinLayout(&metadata->binLayout, metadata->significantBits, ms);

    ALDeserializeIndexMetadata(&metadata->indexMeta, &metadata->binLayout, ms);

    return ALErrorNone;
}

// TODO: Make an ALDeserializeData function
/*
 * INPUT:   ALData *data, ALMetadata *metadata
 * OUTPUT:  memstream_t *ms (data buffer is serialized into the memstream)
 * RETURN:  ALError
 */
ALError ALSerializeData(const ALData *data, const ALMetadata *metadata, memstream_t *ms) {
    const uint8_t insigbytes = ((metadata->elementSize << 3) - metadata->significantBits + 0x07) >> 3;
    memstreamAppendArray(ms, *data, insigbytes, metadata->partitionLength);

    return ALErrorNone;
}

/*
 * INPUT:   ALMetadata *metadata, memstream_t *ms
 * OUTPUT:  ALData *data
 * RETURN:  ALError
 */
ALError ALDeserializeData(ALData *data, const ALMetadata *metadata, memstream_t *ms) {
    uint64_t dataSize = ALGetDataSize(data, metadata);
    *data = (ALData)malloc(dataSize);

    memstreamRead(ms, *data, dataSize);

    return ALErrorNone;
}



/*
 * INPUT:   ALIndex *index, ALMetadata *metadata
 * OUTPUT:  memstream_t *ms (index is serialized into the memstream)
 * RETURN:  ALError
 */
ALError ALSerializeIndex(const ALIndex *index, const ALMetadata *metadata, memstream_t *ms) {
    uint64_t indexSize = ALGetIndexSize(index, metadata);
    memstreamAppendArray(ms, *index, 1, indexSize);

    return ALErrorNone;
}

/*
 * INPUT:   ALMetadata *metadata, memstream_t *ms
 * OUTPUT:  ALIndex *index
 * RETURN:  ALError
 * NOTE:    The size of the output depends on the indexForm. Additional
 *          information would likely be required to be kept with the index
 *          once we make it more robust with inverted index compression etc.
 */
ALError ALDeserializeIndex(ALIndex *index, const ALMetadata *metadata, memstream_t *ms) {
    uint64_t indexSize = ALGetIndexSize(index, metadata);
    *index = (ALIndex)malloc(indexSize);

    memstreamRead(ms, *index, indexSize);

    return ALErrorNone;
}

/*
 * INPUT:     ALPartitionData *partitionData
 * OUTPUT:     memstream_t *ms (partition data is serialized into the memstream)
 * RETURN:     ALError
 */
ALError ALSerializePartitionData(const ALPartitionData *partitionData, memstream_t *ms) {
    ALSerializeMetadata(&partitionData->metadata, ms);
    ALSerializeData(&partitionData->data, &partitionData->metadata, ms);
    ALSerializeIndex(&partitionData->index, &partitionData->metadata, ms);

    return ALErrorNone;
}

/*
 * INPUT:   memstream_t *ms
 * OUTPUT:  ALPartitionData *partitionData
 * RETURN:  ALError
 */
ALError ALDeserializePartitionData(ALPartitionData *partitionData, memstream_t *ms) {
    ALDeserializeMetadata(&partitionData->metadata, ms);
    ALDeserializeData(&partitionData->data, &partitionData->metadata, ms);
    ALDeserializeIndex(&partitionData->index, &partitionData->metadata, ms);
	partitionData->ownsBuffers = true;

    return ALErrorNone;
}

