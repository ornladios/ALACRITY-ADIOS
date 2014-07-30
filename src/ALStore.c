#include <alacrity-types.h>
#include <alacrity-serialization.h>
#include <alacrity-serialization-legacy.h>
#include <alacrity-store.h>
#include <ALUtil.h>

typedef ALError (*LoadGlobalMetadataFunc)(ALStore *store);
typedef ALError (*WritePartitionFunc)(ALStore *store, const ALPartitionData *part);
typedef ALError (*ReadPartitionFunc)(ALStore *store, ALPartitionData *part);
typedef ALError (*OpenPartitionFunc)(ALStore *store, ALPartitionStore *ps, _Bool open_excl);
typedef ALError (*CloseFunc)(ALStore *store);

typedef ALError (*PStoreReadMetaFunc)(ALPartitionStore *pstore, ALMetadata *meta);
typedef ALError (*PStoreReadDataBinsFunc)(ALPartitionStore *pstore, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALData *data);
typedef ALError (*PStoreReadIndexBinsFunc)(ALPartitionStore *pstore, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALIndex *index);
typedef ALError (*PStoreCloseFunc)(ALPartitionStore *pstore);

#define DECLARE_IO_METHOD_FUNCTIONS(suffix) \
    /* ALStore functions*/ \
    extern ALError ALStoreLoadGlobalMetadata##suffix(ALStore *store);                            \
    extern ALError ALStoreWritePartition##suffix(ALStore *store, const ALPartitionData *part);    \
    extern ALError ALStoreReadPartition##suffix(ALStore *store, ALPartitionData *part);            \
    extern ALError ALStoreOpenPartition##suffix(ALStore *store, ALPartitionStore *ps, _Bool open_excl);    \
    extern ALError ALStoreClose##suffix(ALStore *store); \
    /* ALPartitionStore functions*/ \
    extern ALError ALPartitionStoreReadMetadata##suffix(ALPartitionStore *pstore, ALMetadata *meta);    \
    extern ALError ALPartitionStoreReadDataBins##suffix(ALPartitionStore *pstore, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALData *data);    \
    extern ALError ALPartitionStoreReadIndexBins##suffix(ALPartitionStore *pstore, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALIndex *index);    \
    extern ALError ALPartitionStoreClose##suffix(ALPartitionStore *pstore);

#define IO_METHOD_FUNCTION_LIST(suffix) \
        ALStoreLoadGlobalMetadata##suffix, ALStoreWritePartition##suffix, ALStoreReadPartition##suffix, \
        ALStoreOpenPartition##suffix, ALStoreClose##suffix,    \
        ALPartitionStoreReadMetadata##suffix, ALPartitionStoreReadDataBins##suffix, \
        ALPartitionStoreReadIndexBins##suffix, ALPartitionStoreClose##suffix

// Global backend function table
DECLARE_IO_METHOD_FUNCTIONS(POSIX)

static struct {
    LoadGlobalMetadataFunc lgmf;
    WritePartitionFunc wpf;
    ReadPartitionFunc rpf;
    OpenPartitionFunc opf;
    CloseFunc cf;

    PStoreReadMetaFunc psrmf;
    PStoreReadDataBinsFunc psrdbf;
    PStoreReadIndexBinsFunc psribf;
    PStoreCloseFunc pscf;
} BACKEND_FUNC_TABLE[] = {
        { IO_METHOD_FUNCTION_LIST(POSIX) },
};

/////////////
// ALStore function implementations
/////////////

/*
 * Not called by the user! This is a utility function for initializing the
 * state of the main ALStore struct, and should be called by
 * implementation-specific "open" functions
 */
ALError ALStoreOpen(ALStore *store, ALStoreBackendType type, const char *access_mode) {
    if (strcmp(access_mode, "r") == 0) {
        store->access_mode = ACCESS_MODE_READ;
    } else if (strcmp(access_mode, "w") == 0) {
        store->access_mode = ACCESS_MODE_WRITE;
    } else {
        fprintf(stderr, "%s: Unknown access mode %s\n", __FUNCTION__, access_mode);
        return ALErrorSomething; // TODO: Error code
    }

    store->backend_type = type;
    store->impl_state = NULL;

    store->global_meta_loaded = false;
    store->global_meta.num_partitions = 0;
    store->global_meta.total_elements = 0;
    store->global_meta.partition_size = 0;
    store->cur_partition = 0;
    store->has_part_store_excl_open = false;

    return ALErrorNone;
}

ALError ALStoreGetGlobalMetadata(ALStore *store, ALGlobalMetadata *gmeta) {
    if (store->access_mode != ACCESS_MODE_READ) {
        wprintf("[%s] Warning: attempt to read global metadata without being in read access mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    if (!store->global_meta_loaded) {
        if (store->has_part_store_excl_open) {
            eprintf("[%s] Error: attempt to get global metadata, which has not been previously loaded and "
                    "cached, when an outstanding ALPartitionStore is open in exclusive mode.\n", __FUNCTION__);
            return ALErrorSomething; // TODO: Error code
        }

        ALError err = BACKEND_FUNC_TABLE[store->backend_type].lgmf(store);
        if (err != ALErrorNone) return err;
        if (!store->global_meta_loaded) return ALErrorSomething; // TODO: Error code
    }

    // Copy it, it isn't that big. Possible optimization: take a double pointer
    // as argument, or keep a pointer in the struct and dynamically allocated
    // the global meta struct
    if (gmeta)
        *gmeta = store->global_meta;

    return ALErrorNone;
}

ALError ALStoreWritePartition(ALStore *store, const ALPartitionData *part) {
    if (store->access_mode != ACCESS_MODE_WRITE) {
        wprintf("[%s] Warning: attempt to write a partition without being in write access mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    ALError err = BACKEND_FUNC_TABLE[store->backend_type].wpf(store, part);
    if (err != ALErrorNone) return err;

    store->cur_partition++;
    return ALErrorNone;
}

_Bool ALStoreEOF(ALStore *store) {
    // If we are in write mode, there is no EOF
    if (store->access_mode != ACCESS_MODE_READ) {
        return false;
    }

    if (!store->global_meta_loaded) {
        ALError err = ALStoreGetGlobalMetadata(store, NULL);
        if (err != ALErrorNone)
            return err;
    }

    return store->cur_partition >= store->global_meta.num_partitions;
}

ALError ALStoreReadPartition(ALStore *store, ALPartitionData *part) {
    if (store->access_mode != ACCESS_MODE_READ) {
        wprintf("[%s] Warning: attempt to read a partition without being in read access mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    if (store->has_part_store_excl_open) {
        eprintf("[%s] Error: attempt to perform I/O on an ALStore when an outstanding ALPartitionStore is open in exclusive mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    ALError err = BACKEND_FUNC_TABLE[store->backend_type].rpf(store, part);
    if (err != ALErrorNone) return err;

    store->cur_partition++;
    return ALErrorNone;
}

ALError ALStoreOpenPartition(ALStore *store, ALPartitionStore *ps, _Bool open_excl) {
    if (store->access_mode != ACCESS_MODE_READ) {
        wprintf("[%s] Warning: attempt to open a partition without being in read access mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    if (store->has_part_store_excl_open) {
        eprintf("[%s] Error: attempt to perform I/O on an ALStore when an outstanding ALPartitionStore is open in exclusive mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    ALError err = BACKEND_FUNC_TABLE[store->backend_type].opf(store, ps, open_excl);
    if (err != ALErrorNone) return err;

    store->cur_partition++;
    store->has_part_store_excl_open = open_excl;
    return err;
}

ALError ALStoreClose(ALStore *store) {
    if (store->has_part_store_excl_open) {
        eprintf("[%s] Error: attempt to close an ALStore when an outstanding ALPartitionStore is open in exclusive mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    return BACKEND_FUNC_TABLE[store->backend_type].cf(store);
}

/////////////
// ALStore function implementations
/////////////

ALError ALPartitionStoreOpen(ALPartitionStore *pstore, ALStore *store, _Bool open_excl) {
    pstore->is_open_excl = open_excl;

    pstore->source = store;
    pstore->backend_type = store->backend_type;
    pstore->partition_num = store->cur_partition;

    pstore->is_open = true;
    return ALErrorNone;
}

ALError ALPartitionStoreReadMetadata(ALPartitionStore *ps, ALMetadata *meta) {
    return BACKEND_FUNC_TABLE[ps->backend_type].psrmf(ps, meta);
}

// TODO: Move buffer allocation code here; it only depends on information from ALMetadata
/*
 * If data == NULL, then a new buffer is allocated. Otherwise, it is assumed to
 * be already allocated, and is filled.
 */
ALError ALPartitionStoreReadDataBins(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALData *data) {
    return BACKEND_FUNC_TABLE[ps->backend_type].psrdbf(ps, meta, low_bin, hi_bin, data);
}

// TODO: Move buffer allocation code here; it only depends on information from ALMetadata
/*
 * If index == NULL, then a new buffer is allocated. Otherwise, it is assumed to
 * be already allocated, and is filled.
 */
ALError ALPartitionStoreReadIndexBins(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALIndex *index) {
    return BACKEND_FUNC_TABLE[ps->backend_type].psribf(ps, meta, low_bin, hi_bin, index);
}

ALError ALPartitionStoreClose(ALPartitionStore *ps) {
    ALError err = BACKEND_FUNC_TABLE[ps->backend_type].pscf(ps);

    ps->source->has_part_store_excl_open = false;
    ps->is_open = false;
    return err;
}




