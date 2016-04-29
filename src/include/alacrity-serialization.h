#ifndef ALACRITY_SERIALIZATION_H_
#define ALACRITY_SERIALIZATION_H_

#include <stdint.h>
#include <memstream.h>
#include <alacrity-types.h>

#include <alacrity-serialization-legacy.h>
#include "alacrity-util.h"

ALError ALSerializePartitionData(const ALPartitionData *input, memstream_t *ms);
ALError ALDeserializePartitionData(ALPartitionData *partitionData, memstream_t *ms);

ALError ALSerializeMetadata(const ALMetadata *metadata, memstream_t *ms);
ALError ALDeserializeMetadata(ALMetadata *metadata, memstream_t *ms);

ALError ALSerializeData(const ALData *data, const ALMetadata *metadata, memstream_t *ms);
ALError ALDeserializeData(ALData *data, const ALMetadata *metadata, memstream_t *ms);

ALError ALSerializeIndex(const ALIndex *index, const ALMetadata *metadata, memstream_t *ms);
ALError ALDeserializeIndex(ALIndex *index, const ALMetadata *metadata, memstream_t *ms);

ALError ALSerializeBinLayout(const ALBinLayout *binLayout, uint8_t significantBytes, memstream_t *ms);

ALError ALDeserializeBinLayout(ALBinLayout *binLayout, uint8_t significantBytes, memstream_t *ms);

/* Get size functions */
uint64_t ALGetDataSize(const ALData *data, const ALMetadata *metadata);
uint64_t ALGetIndexSize(const ALIndex *index, const ALMetadata *metadata);
uint64_t ALGetBinLayoutSize(const ALBinLayout *binLayout, uint8_t significantBytes);
uint64_t ALGetPartitionDataSize(const ALPartitionData *partitionData);
uint64_t ALGetMetadataSize(const ALMetadata *metadata);
uint64_t ALGetIndexMetadataSize(const ALIndexMetadata *indexMeta, const ALMetadata *metadata);

/* Offset calculation */
// uint64_t ALGetDataBinOffset(const ALMetadata *metadata, bin_id_t bin);
// uint64_t ALGetIndexBinOffset(const ALMetadata *metadata, bin_id_t bin);

// In-place deserializeation
static inline ALError ALDeserializeBinLayoutInPlace (ALBinLayout *binLayout, uint8_t significantBytes, memstream_t *binLayoutBuffer);
static inline ALError ALDeserializeMetadataInPlace (ALMetadata *metadata, memstream_t *metadataBuffer);

// uint64_t ALGetDataBinSize(const ALMetadata *metadata, bin_id_t binID);
// 
// uint64_t ALGetIndexBinSize(const ALMetadata *metadata, bin_id_t binID);

static inline uint64_t ALGetDataBinOffset(const ALMetadata *metadata, bin_id_t bin) {
    const uint8_t insigbytes = alacrity_util_insigBytesCeil(metadata);
    return metadata->binLayout.binStartOffsets[bin] * insigbytes;
}

static inline uint64_t ALGetIndexBinOffset(const ALMetadata *metadata, bin_id_t bin) {
    const uint8_t sigbytes = alacrity_util_sigBytesCeil(metadata);
    switch (metadata->indexMeta.indexForm) {
    case ALInvertedIndex:
        return metadata->binLayout.binStartOffsets[bin] * sizeof(rid_t);
    case ALCompressedInvertedIndex:
        return metadata->indexMeta.u.ciim.indexBinStartOffsets[bin];
    case ALCompressedHybridInvertedIndex:
    case ALCompressedSkipInvertedIndex:
    case ALCompressedMixInvertedIndex:
    case ALCompressedExpansionII:
    	return metadata->indexMeta.u.ciim.indexBinStartOffsets[bin];

    case ALCompressionIndex:
        fprintf(stderr, "%s: Index has ALCompressionIndex form, but this form has no bins\n", __FUNCTION__);
        return 0;
    default:
        fprintf(stderr, "%s: Invalid indexing method %d\n", __FUNCTION__, metadata->indexMeta.indexForm);
        return 0;
    }
}

static inline uint64_t ALGetDataBinSize(const ALMetadata *metadata, bin_id_t binID)
{
    return ALGetDataBinOffset(metadata, binID + 1) - ALGetDataBinOffset(metadata, binID);
}

static inline uint64_t ALGetIndexBinSize(const ALMetadata *metadata, bin_id_t binID)
{
    return ALGetIndexBinOffset(metadata, binID + 1) - ALGetIndexBinOffset(metadata, binID);
}

#endif /*ALACRITY_SERIALIZATION_H_*/