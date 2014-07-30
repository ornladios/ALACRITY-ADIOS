#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <alacrity-types.h>
#include <alacrity-store.h>
#include <ALUtil.h>

typedef struct {
    // State required for reading/writing goes here.
    // NOTE: You might want two separate structures based on read/write mode.
    //       There is nothing requiring you to use the same thing for the state variable all the time.
} ALStoreTemplateState;

typedef struct {
    // State required for reading from a single partition
} ALPartitionStoreTemplateState;

// ALStore implementation functions
ALError ALStoreOpenTemplate(ALStore *store, const char *basename, const char *access_mode, _Bool legacyFormat);
ALError ALStoreLoadGlobalMetadataTemplate(ALStore *store);
ALError ALStoreReadPartitionTemplate(ALStore *store, ALPartitionData *part);
ALError ALStoreWritePartitionTemplate(ALStore *store, const ALPartitionData *part);
ALError ALStoreOpenPartitionTemplate(ALStore *store, ALPartitionStore *ps, _Bool open_excl);
ALError ALStoreCloseTemplate(ALStore *store);
// ALPartitionStore implementation functions
ALError ALPartitionStoreReadMetadataTemplate(ALPartitionStore *ps, ALMetadata *meta);
ALError ALPartitionStoreReadDataBinsTemplate(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALData *data);
ALError ALPartitionStoreReadIndexBinsTemplate(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALIndex *index);
ALError ALPartitionStoreCloseTemplate(ALPartitionStore *ps);

//////////////////////////////////////
// ALStore interface
//////////////////////////////////////

/*
 * Called by the user to open the given store. You may select any parameters
 * for this function that you wish, since the user will call it directly.
 * However, you must call the ALStoreOpen function from this function, so you
 * must collect the necessary information from the user to supply to it.
 */
ALError ALStoreOpenTemplate(ALStore *store, const char *basename, const char *access_mode, _Bool legacyFormat) {
    // Call the main ALStore open function to initialize the ALStore
    ALError err = ALStoreOpen(store, POSIX_STORE, access_mode);
    if (err != ALErrorNone)
        return err;

    // Implementation-specific initialization

    // Allocate memory for implementation-specific state
    ALStoreTemplateState *state = (ALStoreTemplateState*)malloc(sizeof(ALStoreTemplateState));
    store->impl_state = state;

    // Other initialization

    return ALErrorNone;
}

/*
 * Read the global metadata for the store into store->global_meta. On success,
 * it must also set store->global_meta_loaded to true.
 *
 * This will be called at most once. Since other functions may be called before
 * this (e.g., ReadPartition), you should check whether the global metadata is
 * loaded (with store->global_meta_loaded), and load it if need be (by calling
 * this function yourself, if you wish).
 *
 * Guaranteed to be called only in read mode.
 */
ALError ALStoreLoadGlobalMetadataTemplate(ALStore *store) {
    // Do some stuff to load the metadata

    store->global_meta_loaded = true;
    return ALErrorNone;
}

/*
 * Read the next partition from "store" into "part", unless no more partitions
 * remain, in which case an error should be returned.
 *
 * store->cur_partition SHOULD NOT be updated (this is done externally).
 *
 * Guaranteed to be called only in read mode.
 */
ALError ALStoreReadPartitionTemplate(ALStore *store, ALPartitionData *part) {
    // Load data into "part"
    return ALErrorNone;
}

/*
 * Append partition "part" into "store".
 *
 * store->global_meta SHOULD be updated. store->cur_partition SHOULD NOT be
 * updated (this is done externally).
 *
 * Guaranteed to be called only in write mode.
 */
ALError ALStoreWritePartitionTemplate(ALStore *store, const ALPartitionData *part) {
    // Write "part" into the store
    return ALErrorNone;
}

/*
 * Open the next partition for fine-grained access through an ALPartitionStore.
 * This function should initialize the ALPartitionStore in a custom way,
 * inserting custom state information as required. However, it is strongly
 * recommended that the generic ALPartitionStoreOpen() be used to initialize
 * the generic state of the ALPartitionStore before custom initialization is
 * performed.
 *
 * If open_excl is true, the user guarantees that the newly-returned
 * ALPartitionStore will be close before any further functions are called on
 * the ALStore. This is checked and enforced by the generic interface, so no
 * additional work is needed in the implementation to support this. However,
 * the implementation may take advantage of exclusive mode when it is active.
 *
 * This function will advance to the next partition. store->cur_partition
 * SHOULD NOT be updated (this is done externally).
 *
 * Guaranteed to be called only in read mode.
 */
ALError ALStoreOpenPartitionTemplate(ALStore *store, ALPartitionStore *ps, _Bool open_excl) {
    // Call the generic open function to do generic initialization
    ALPartitionStoreOpen(ps, store, open_excl);

    // Allocate memory for implementation-specific state
    ps->impl_state = (ALPartitionStoreTemplateState*)malloc(sizeof(ALPartitionStoreTemplateState));

    // Other initialization here

    return ALErrorNone;
}

/*
 * Close the partition store.
 */
ALError ALStoreCloseTemplate(ALStore *store) {
    // Do cleanup here

    free(store->impl_state);

    return ALErrorNone;
}

//////////////////////////////////////
// ALPartitionStore interface
//////////////////////////////////////

/*
 * Read the partition metadata from the ALPartitionStore.
 */
ALError ALPartitionStoreReadMetadataTemplate(ALPartitionStore *ps, ALMetadata *meta) {
    // Fill "meta" here
    return ALErrorNone;
}

/*
 * Read the low-order bytes for the given bin IDs from the ALPartitionStore
 * into "data".
 *
 * If "data" is NULL, allocate a buffer of sufficient size for the data to
 * return, assign it to "data", and fill it as usual.
 */
ALError ALPartitionStoreReadDataBinsTemplate(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALData *data) {
    const char insigbytes = meta->elementSize - meta->significantBytes;
    const uint64_t first_bin_elem_off = meta->binLayout.binEndOffsets[low_bin];
    const uint64_t last_bin_elem_off = meta->binLayout.binEndOffsets[hi_bin];
    const uint64_t num_elems_to_read = last_bin_elem_off - first_bin_elem_off;
    const uint64_t num_bytes_to_read = num_elems_to_read * insigbytes;

    if (*data == NULL)
        *data = malloc(num_bytes_to_read);

    // Fill "data" here
    return ALErrorNone;
}

/*
 * Read the index for the given bin IDs from the ALPartitionStore into "index".
 *
 * If "index" is NULL, allocate a buffer of sufficient size for the index to
 * return, assign it to "index", and fill it as usual.
 *
 */
ALError ALPartitionStoreReadIndexBinsTemplate(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALIndex *index) {
    const char index_row_size = (meta->indexForm == ALInvertedIndex) ? sizeof(rid_t) : meta->significantBytes;
    const uint64_t first_bin_elem_off = meta->binLayout.binEndOffsets[low_bin];
    const uint64_t last_bin_elem_off = meta->binLayout.binEndOffsets[hi_bin];
    const uint64_t num_elems_to_read = last_bin_elem_off - first_bin_elem_off;
    const uint64_t num_bytes_to_read = num_elems_to_read * index_row_size;

    if (*index == NULL)
        *index = malloc(num_bytes_to_read);

    // Fill "index" here
    return ALErrorNone;
}

/*
 * Close the ALPartitionStore.
 *
 * The generic interface will automatically release exclusive mode if it was
 * enabled. If the implementation took advantage of exclusive mode and
 * perturbed the parent ALStore's state information, it should revert it as
 * necessary here.
 */
ALError ALPartitionStoreCloseTemplate(ALPartitionStore *ps) {
    // Do cleanup here

    // Free the implementation-specific state
    free(ps->impl_state);

    return ALErrorNone;
}
