#ifndef ALACRITY_STORE_H_
#define ALACRITY_STORE_H_

#include <stdint.h>
#include <stdbool.h>
#include <alacrity-types.h>

////////////
// ENUMS
////////////

// Defines how the ALStore is opened (for read or write).
typedef enum {
    ACCESS_MODE_READ,
    ACCESS_MODE_WRITE,
} ALStoreAccessMode;

// A list of the types of ALStores that exist.
typedef enum {
    POSIX_STORE,
    ADIOS_BP_STORE,
} ALStoreBackendType;

////////////
// STRUCTS
////////////

/*
 * Generic global metadata for an ALACRITY store. Individual store
 * implementations may require additional, internal metadata.
 * (file offsets, etc.)
 */
typedef struct {
    uint64_t num_partitions;
    uint64_t total_elements;
    uint64_t partition_size;
} ALGlobalMetadata;

/*
 * Main struct for an ALACRITY store. Must be opened with an open function
 * specific to the store implementation. From there on, it may be operated on,
 * and closed by, generic ALStore functions.
 */
typedef struct {
    ALStoreAccessMode access_mode;

    ALStoreBackendType backend_type;
    void *impl_state;

    _Bool global_meta_loaded;
    ALGlobalMetadata global_meta;

    uint64_t cur_partition;

    _Bool has_part_store_excl_open;
} ALStore;

/*
 * Encapsulates a particular ALACRITY partition within an ALStore, and
 * enables fine-grained access, for example, reading only metadata,
 * or reading only certain data/index bins.
 */
typedef struct {
    _Bool is_open;
    _Bool is_open_excl;
    ALStore *source;
    uint64_t partition_num;

    ALStoreBackendType backend_type;
    void *impl_state;
} ALPartitionStore;

// DO NOT CALL THIS AS A USER; INTERNAL USE ONLY
ALError ALStoreOpen(ALStore *store, ALStoreBackendType type, const char *access_mode); // TODO: Internalize part of this header

// General (polymorphic) ALStore functions (must have implementation-specific version for each ALStore backend type)
ALError ALStoreGetGlobalMetadata(ALStore *store, ALGlobalMetadata *gmeta);
ALError ALStoreWritePartition(ALStore *fs, const ALPartitionData *part);
_Bool ALStoreEOF(ALStore *fs);
ALError ALStoreReadPartition(ALStore *fs, ALPartitionData *part);
ALError ALStoreOpenPartition(ALStore *fs, ALPartitionStore *ps, _Bool open_excl);
ALError ALStoreClose(ALStore *store);

// Implementation-specific open functions
ALError ALStoreOpenPOSIX(ALStore *store, const char *basename, const char *access_mode, _Bool legacy_format);

// ALPartitionStore functoins

// DO NOT CALL THIS AS A USER; INTERNAL USE ONLY
ALError ALPartitionStoreOpen(ALPartitionStore *pstore, ALStore *store, _Bool open_excl); // TODO: Internalize part of this header

ALError ALPartitionStoreReadMetadata(ALPartitionStore *ps, ALMetadata *meta);
ALError ALPartitionStoreReadDataBins(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALData *data);
ALError ALPartitionStoreReadIndexBins(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALIndex *index);
ALError ALPartitionStoreClose(ALPartitionStore *pstore);

#endif