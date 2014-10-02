#ifndef ALACRITY_TYPES_H_
#define ALACRITY_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// NOTE: throughout the comments, "", "elementSize", "partitionLength" and "indexForm" are mentioned. These always refer to
// the value from the ALMetadata struct.

//////
// GENERAL PURPOSE TYPES AND ENUMS
//////

typedef enum {
    ALErrorNone, //A OK
    ALErrorSomething, // Some error or something!
    ALErrorEndOfStore, // No more partitions available to read from an ALStore
} ALError;

//////
// ALACRITY DATA STRUCTURE TYPES AND ENUMS
//////

//////
// Simple datatype typedefs
//////

typedef uint32_t partition_length_t;        // Type to hold the number of elements in a partition
typedef partition_length_t bin_offset_t;    // Type to hold element offset within the partition

typedef uint64_t low_order_bytes_t;            // Type to hold the low-order bytes for a datum (sized for worst-case)
typedef uint32_t high_order_bytes_t;        // Type to hold the high-order bytes for a datum (sized for worst-case)
typedef int32_t signed_high_order_bytes_t;    // Type to hold the high-order bytes for a datum (sized for worst-case), interpreted as signed (useful for comparisons)

typedef uint64_t global_rid_t;    // Type to hold the global RID offset of a partition

//////
// Derived simple datatype typedefs
//////

typedef partition_length_t rid_t;        // Type to hold a row ID
typedef high_order_bytes_t bin_id_t;    // Type to hold a bin ID (same as that needed to hold a bin header value) (sized for the worst-case; in the bin value table and/or compressed index, these are packed "" bytes per bin ID)

//////
// Enums
//////

// Index form for an ALACRITY partition
typedef enum {
    ALCompressionIndex,        // Compressed index (rowID->binID mapping)
    ALInvertedIndex,        // Inverted index (binID->rowID mapping)
    ALCompressedInvertedIndex,    // Inverted index compressed by some means (different from ALCompressionIndex)
    ALCompressedHybridInvertedIndex, // inverted index compressed by p4d + RLE
    ALCompressedSkipInvertedIndex, //  skipping inverted index compressed by p4d
    ALCompressedMixInvertedIndex,   // change to EPFD + RPFD
    ALCompressedExpansionII      // expand `b` to `b + n`

} ALIndexForm;

typedef enum {
    ENDIAN_LITTLE,
    ENDIAN_BIG,
    ENDIAN_UNDEFINED
} ALEndianness;

typedef enum {
	MIN_DATATYPE_ENUM = -1,
	DATATYPE_UNDEFINED,
	DATATYPE_FLOAT64,
	DATATYPE_FLOAT32,
	MAX_DATATYPE_ENUM,
} ALDatatype;

//////
// Partition data struct definitions
//////

// Partition bin layout information
typedef struct {
    bin_id_t numBins;               // Total number of bins
    bin_offset_t *binValues;            // Bin header values, packed "" bytes at a time, total size is (numBins * sizeof (bin_id_t))
    bin_offset_t *binStartOffsets;  // Element offsets to the start of the bins, exclusive (i.e. binStartOffsets[i] points to the first element of bin i.
                                    // There are numBins + 1 binStartOffsets where the last value points to the end of all the offsets
} ALBinLayout;

// Index metadata structures
typedef struct {} ALCompressionIndexMetadata; // No additional metadata
typedef struct {} ALInvertedIndexMetadata;    // No additional metadata
typedef struct {
    uint64_t *indexBinStartOffsets;
} ALCompressedInvertedIndexMetadata;

typedef struct {
    ALIndexForm indexForm;                     // The form of the index in this partition
    union {                                    // Only one of the contained metadata structs is used at once; namely, the one matching indexForm
        ALCompressionIndexMetadata            cim;
        ALInvertedIndexMetadata               iim;
        ALCompressedInvertedIndexMetadata    ciim;
    } u;
} ALIndexMetadata;

// Metadata for an ALACRITY partition
typedef struct {
    global_rid_t          globalOffset;    // RID offset of this partition relative to the global RID space
    partition_length_t    partitionLength;  // Number of elements in the partition
    ALBinLayout           binLayout;        // The bin layout for this partition

    char                  significantBits;  // The number of bits considered "high-order"
                                            // (used to always be 2, now it's variable, and in bits as well)
    char                  elementSize;      // The total number of bytes in each datum/element (used to always be 8, now it's variable)
    ALDatatype            datatype;         // The datatype of the elements stored in this partition (primarily used during query)
    char                  endianness;		// The endianness of this partition (TODO: implement, as this is not currently used)

    ALIndexMetadata       indexMeta;        // All metadata specifically associated with the index component of the partition
} ALMetadata;

// Data and index buffer
typedef char *ALData;        // Buffer of low-order bytes from the original data. Each element is "elementSize - " bytes, total size (partitionLength*elementSize)
typedef char *ALIndex;        // Buffer of index elements. If indexForm == ALCompressedIndex, then each element is a bin ID, and thus is "" bytes long. Otherwise, if indexForm == ALInvertedIndex, each element is of type "rid_t".
typedef bin_id_t *ALBinLookupTable; // Buffer of bin ids that holds the count of each bin value seen.

// The main ALACRITY partition data struct
typedef struct {
    ALMetadata metadata;
    ALData data;
    ALIndex index;
    _Bool ownsBuffers;    // If true, this partition owns the various buffers it contains, which should be deallocated when the partition is destroyed
                        // Otherwise, this partition's various buffers are owned somewhere else, and should not be deallocated when the partition is destroyed
} ALPartitionData;

#ifdef __cplusplus
} // extern "C"
#endif

#endif