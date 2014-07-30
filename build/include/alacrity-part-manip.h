#pragma once

#include <alacrity-types.h>

// ALACRITY encode/decode parameter structure
typedef struct {
    char          significantBits;
    char          elementSize;
    ALDatatype    datatype;
    ALIndexForm   indexForm;
} ALEncoderConfig;

// Initialize ALACRITY encode parameters
ALError ALEncoderConfigure(ALEncoderConfig  *config,
                           int              significantBits,
                           ALDatatype       datatype,
                           ALIndexForm      indexForm);

/*
 * Convert raw data to ALACRITY form
 * Assumptions:
 *   - calling application (or library) takes
 *     care of linearization, contiguity
 */
ALError ALEncode(const ALEncoderConfig     *config,
                 const void                *input,
                 uint64_t                inputCount,
                 ALPartitionData         *output);

/*
 * Convert ALACRITY form data back to raw data
 * Assumptions:
 *   - calling application (or library) takes
 *     care of (de)-linearization, contiguity
 */
ALError ALDecode(const ALPartitionData    *input,
                 void                    *output,
                uint64_t                *outputCount);

/*
 * Returns how many bytes decoding the given ALACRITY partition will produce
 */
uint64_t ALGetDecodeLength(const ALPartitionData *input);

/*
 * Convert index form between compressed and inverted
 * (or other forms if developed later)
 */
ALError ALConvertIndexForm(ALMetadata *metadata,
                           ALIndex *index,
                           ALIndexForm newForm);

ALError ALConvertPartialIndexForm(const ALMetadata *metadata,
                                  ALIndex *index,
                                  ALIndexForm newForm,
                                  bin_id_t lo_bin,
                                  bin_id_t hi_bin);

ALError ALTranslateRIDs(const ALMetadata *metadata,
                        ALIndex *index,
                        int32_t rid_offset);

/*
 * Merge two ALACRITY partitions into a new, larger one
 */
ALError ALMerge(const ALPartitionData *part1, const ALPartitionData *part2, ALPartitionData *partMerged);

ALError ALBuildBinLayout (const ALEncoderConfig *config, const void *input, uint64_t inputCount, ALBinLookupTable *binLookupTable, ALPartitionData *output);
ALError ALBuildInvertedIndexFromLayout (const ALEncoderConfig *config, const void *input, uint64_t inputCount, ALBinLookupTable *binLookupTable, ALPartitionData *output);


