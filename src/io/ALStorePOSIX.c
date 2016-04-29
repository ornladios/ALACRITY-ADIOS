/*
 * LIMITATIONS:
 * > Currently only supports legacy format
 * > Currently only supports inverted index
 * > Currently only supports opening partition stores in exclusive mode
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <alacrity-types.h>
#include <alacrity-store.h>
#include <alacrity-serialization.h>
#include <alacrity-serialization-legacy.h>
#include "../include/alacrity-util.h"

typedef struct {
    FILE *metadatafp, *tmpmetadatafp, *datafp, *iindexfp, *cindexfp;
    _Bool legacyFormat;

    // Shared state
    uint64_t partition_capacity;
    uint64_t *meta_offsets;
    uint64_t *data_offsets;
    uint64_t *index_offsets;

    // Read state

    // Write state
    uint64_t cur_meta_offset;
    uint64_t cur_data_offset;
    uint64_t cur_index_offset;
    _Bool has_last_partition_been_written;
} ALStorePOSIXState;

typedef struct {
    // true if the metadata file pointer from the parent ALStore still points the beginning of the
    // partition's metadata (i.e., ALPartitionStoreReadMetadata() hasn't been called yet to move it)
    _Bool metafp_unchanged;
    uint64_t meta_offset;
    uint64_t data_offset;
    uint64_t index_offset;

    _Bool iindexfp_changed;
    _Bool cindexfp_changed;
} ALPartitionStorePOSIXState;

static ALError ALStorePOSIXOpenFiles(ALStore *store, const char *basename, _Bool is_reading) {
    // Set up filename buffer. The prefix (basename) remains the same, we just strcpy the suffix partway into it.
    char buf[256];
    strcpy(buf, basename);
    char *bufappend = buf + strlen(basename);

    // Get the state struct
    ALStorePOSIXState *params = (ALStorePOSIXState*)store->impl_state;

    const char *access_mode = is_reading ? "r" : "w";

    // Open the requisite files
    strcpy(bufappend, "-metadata.dat");
    params->metadatafp         = fopen(buf, access_mode);
    strcpy(bufappend, "-compressed_data.dat");
    params->datafp             = fopen(buf, access_mode);
    strcpy(bufappend, "-query_index.dat");
    params->iindexfp         = fopen(buf, access_mode);
    strcpy(bufappend, "-index.dat");
    params->cindexfp         = fopen(buf, access_mode);
    if (!is_reading) {
        strcpy(bufappend, "-metadata.dat.tmp");
        params->tmpmetadatafp = fopen(buf, "w+");
    } else {
        params->tmpmetadatafp = NULL;
    }

    return ALErrorNone;
}

#define ALSTORE_POSIX_INITIAL_PARTITION_CAPACITY 16
ALError ALStoreOpenPOSIX(ALStore *store, const char *basename, const char *access_mode, _Bool legacyFormat) {
    // Call the generic open code
    ALError err = ALStoreOpen(store, POSIX_STORE, access_mode);
    if (err != ALErrorNone) return err;

    // Set up our implementation-specific parameters
    ALStorePOSIXState *params = (ALStorePOSIXState*)malloc(sizeof(ALStorePOSIXState));
    store->impl_state = params;

    // Get some local variables for simplicity
    ALGlobalMetadata *gmeta = &store->global_meta;
    _Bool is_reading = (store->access_mode == ACCESS_MODE_READ);

    // Open the files needed
    err = ALStorePOSIXOpenFiles(store, basename, is_reading);
    if (err != ALErrorNone) return err;

    // Set the various parameter files of the struct, and malloc necessary lists
    params->partition_capacity = ALSTORE_POSIX_INITIAL_PARTITION_CAPACITY;
    params->meta_offsets = malloc(params->partition_capacity * sizeof(uint64_t));
    params->data_offsets = malloc(params->partition_capacity * sizeof(uint64_t));
    params->index_offsets = malloc(params->partition_capacity * sizeof(uint64_t));

    params->cur_meta_offset = 0;
    params->cur_data_offset = 0;
    params->cur_index_offset = 0;
    params->has_last_partition_been_written = false;

    params->legacyFormat = legacyFormat;

    return ALErrorNone;
}

static void extendOffsetBuffers(ALStorePOSIXState *params, uint64_t new_capacity, _Bool copy_contents) {
    if (new_capacity == 0)
        new_capacity = params->partition_capacity * 2;

    if (copy_contents) {
        params->meta_offsets = realloc(params->meta_offsets, new_capacity * sizeof(uint64_t));
        params->index_offsets = realloc(params->index_offsets, new_capacity * sizeof(uint64_t));
        params->data_offsets = realloc(params->data_offsets, new_capacity * sizeof(uint64_t));
    } else {
        free(params->meta_offsets);
        free(params->index_offsets);
        free(params->data_offsets);
        params->meta_offsets = malloc(new_capacity * sizeof(uint64_t));
        params->index_offsets = malloc(new_capacity * sizeof(uint64_t));
        params->data_offsets = malloc(new_capacity * sizeof(uint64_t));
    }
    params->partition_capacity = new_capacity;
}

static uint64_t getGlobalHeaderSize(const ALStore *store) {
    const ALStorePOSIXState *params = (ALStorePOSIXState*)store->impl_state;
    const ALGlobalMetadata *gmeta = &store->global_meta;

    if (params->legacyFormat) {
        return  sizeof(uint64_t) +    // total_elements
                sizeof(uint64_t) +    // part_size
                3 * gmeta->num_partitions * sizeof(uint64_t); // meta, data and index offsets
    } else {
        return  sizeof(uint64_t) +    // total_elements
                sizeof(uint64_t) +    // part_size
                sizeof(uint64_t) +    // num_partitions
                3 * (gmeta->num_partitions + 1) * sizeof(uint64_t); // meta, data and index offsets
    }
}

ALError ALStoreLoadGlobalMetadataPOSIX(ALStore *store) {
    if (store->access_mode != ACCESS_MODE_READ) {
        eprintf("[%s] Error: attempt to load global metadata from file without being in read access mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    ALStorePOSIXState *params = (ALStorePOSIXState*)store->impl_state;
    ALGlobalMetadata *gmeta = &store->global_meta;

    if (params->legacyFormat) {
        fseek(params->metadatafp, 0, SEEK_SET);
        fread(&gmeta->total_elements, sizeof(uint64_t), 1, params->metadatafp);
        fread(&gmeta->partition_size, sizeof(uint64_t), 1, params->metadatafp);
        gmeta->num_partitions = (gmeta->total_elements - 1) / gmeta->partition_size + 1; // Rounded up

        if (gmeta->num_partitions > params->partition_capacity)
            extendOffsetBuffers(params, gmeta->num_partitions, false);

        fread(params->meta_offsets, sizeof(uint64_t), gmeta->num_partitions, params->metadatafp);
        fread(params->data_offsets, sizeof(uint64_t), gmeta->num_partitions, params->metadatafp);
        fread(params->index_offsets, sizeof(uint64_t), gmeta->num_partitions, params->metadatafp);

        if (!ferror(params->metadatafp)) {
            store->global_meta_loaded = true;
            return ALErrorNone;
        } else {
            return ALErrorSomething; // TODO: Error code
        }
    } else {
        fseek(params->metadatafp, 0, SEEK_SET);

        fread(&gmeta->total_elements, sizeof(uint64_t), 1, params->metadatafp);
        fread(&gmeta->partition_size, sizeof(uint64_t), 1, params->metadatafp);
        fread(&gmeta->num_partitions, sizeof(uint64_t), 1, params->metadatafp);

        if (gmeta->num_partitions + 1 > params->partition_capacity)
            extendOffsetBuffers(params, gmeta->num_partitions + 1, false);

        fread(params->meta_offsets, sizeof(uint64_t), gmeta->num_partitions + 1, params->metadatafp);
        fread(params->data_offsets, sizeof(uint64_t), gmeta->num_partitions + 1, params->metadatafp);
        fread(params->index_offsets, sizeof(uint64_t), gmeta->num_partitions + 1, params->metadatafp);

        if (!ferror(params->metadatafp)) {
            store->global_meta_loaded = true;
            return ALErrorNone;
        } else {
            return ALErrorSomething; // TODO: Error code
        }

        fprintf(stderr, "[%s] Error: new POSIX store format unsupported at this time.\n", __FUNCTION__);
        abort();
        return ALErrorSomething;
    }
}

#define METADATA_COPY_BUFSIZE 65536
static ALError finalizeMetadata(ALStore *store) {
    const ALGlobalMetadata *gmeta = &store->global_meta;
    ALStorePOSIXState *params = (ALStorePOSIXState*)store->impl_state;

    // Shift the metadata offsets since we are prepending the global header
    const uint64_t global_header_size = getGlobalHeaderSize(store);
    uint64_t i = 0;
    for (; i < gmeta->num_partitions; i ++){
        params->meta_offsets[i] += global_header_size;
    }

    // First write the global header
    memstream_t ms = memstreamInitReturn(malloc(getGlobalHeaderSize(store)));

    if (params->legacyFormat) {
        memstreamAppendUint64(&ms, gmeta->total_elements);
        memstreamAppendUint64(&ms, gmeta->partition_size);
        memstreamAppendArray(&ms, params->meta_offsets, sizeof(uint64_t), gmeta->num_partitions);
        memstreamAppendArray(&ms, params->data_offsets, sizeof(uint64_t), gmeta->num_partitions);
        memstreamAppendArray(&ms, params->index_offsets, sizeof(uint64_t), gmeta->num_partitions);
    } else {
        if (gmeta->num_partitions + 1 > params->partition_capacity)
            extendOffsetBuffers(params, gmeta->num_partitions + 1, true);

        // Add the final offsets, pointing to the ends of the files
        params->meta_offsets[gmeta->num_partitions] = params->cur_meta_offset + global_header_size;
        params->data_offsets[gmeta->num_partitions] = params->cur_data_offset;
        params->index_offsets[gmeta->num_partitions] = params->cur_index_offset;

        memstreamAppendUint64(&ms, gmeta->total_elements);
        memstreamAppendUint64(&ms, gmeta->partition_size);
        memstreamAppendUint64(&ms, gmeta->num_partitions);
        memstreamAppendArray(&ms, params->meta_offsets, sizeof(uint64_t), gmeta->num_partitions + 1);
        memstreamAppendArray(&ms, params->data_offsets, sizeof(uint64_t), gmeta->num_partitions + 1);
        memstreamAppendArray(&ms, params->index_offsets, sizeof(uint64_t), gmeta->num_partitions + 1);
    }

    //fseek(params->metadatafp, 0, SEEK_SET); // Not needed, since we don't seek this in write mode
    fwrite(ms.buf, memstreamGetPosition(&ms), 1, params->metadatafp);
    dbprintf("[%s] Wrote %llu of global header data\n", __FUNCTION__, memstreamGetPosition(&ms));
    memstreamDestroy(&ms, true);

    // Then copy the temp file over
    fseek(params->tmpmetadatafp, 0, SEEK_SET); // Go back to the beginning of the per-partition metadata

    int len;
    void *buf = malloc(METADATA_COPY_BUFSIZE);
    while (!feof(params->tmpmetadatafp) && !ferror(params->tmpmetadatafp)) {
        len = fread(buf, 1, METADATA_COPY_BUFSIZE, params->tmpmetadatafp);
        fwrite(buf, 1, len, params->metadatafp);
        dbprintf("[%s] Copied %d of partition metadata from the temporary metadata file to the real one...\n", __FUNCTION__, (int)len);
    }
    free(buf);

    dbprintf("[%s] Done copying metadata. Error in read/writing? %c\n", __FUNCTION__, !ferror(params->tmpmetadatafp) && !ferror(params->metadatafp) ? 'N' : 'Y');
    return !ferror(params->tmpmetadatafp) && !ferror(params->metadatafp) ? ALErrorNone : ALErrorSomething; // TODO: Error code
}

ALError ALStoreWritePartitionPOSIX(ALStore *store, const ALPartitionData *part) {
    ALStorePOSIXState *params = (ALStorePOSIXState*)store->impl_state;
    ALGlobalMetadata *gmeta = &store->global_meta;

    if (params->legacyFormat && part->metadata.indexMeta.indexForm != ALInvertedIndex) {
        eprintf("[%s] Error: non-inverted index writing is not currently supported in the legacy format.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    if (params->partition_capacity == gmeta->num_partitions) {
        dbprintf("[%s] Current metadata buffer capacity for partitions met, expanding capacity...\n", __FUNCTION__);
        extendOffsetBuffers(params, 0, true); // 0 -> double capacity, true -> maintain contents
    }

    uint64_t cp = store->cur_partition;
    params->meta_offsets[cp] = params->cur_meta_offset;
    params->data_offsets[cp] = params->cur_data_offset;
    params->index_offsets[cp] = params->cur_index_offset;

    if (gmeta->partition_size == 0) {
        gmeta->partition_size = part->metadata.partitionLength;
    } else if (gmeta->partition_size != part->metadata.partitionLength) {
        if (params->has_last_partition_been_written) {
            eprintf("%s: Error: just attempted to append a partition with length %lu, but previous partitions had length %lu, and this is not the final partition\n",
                    __FUNCTION__, part->metadata.partitionLength, gmeta->partition_size);
            return ALErrorSomething;
        }
        params->has_last_partition_been_written = true;
    }

    // Memstream, to be reused several times
    memstream_t ms;

    // Write the metadata
    uint64_t metasize = params->legacyFormat ? ALGetMetadataSizeLegacy(&part->metadata) : ALGetMetadataSize(&part->metadata);
    memstreamInit(&ms, malloc(metasize));
    if (params->legacyFormat)
        ALSerializeMetadataLegacy(&part->metadata, &ms);
    else
        ALSerializeMetadata(&part->metadata, &ms);
    fwrite(ms.buf, metasize, 1, params->tmpmetadatafp); // Write to the temp metadata file
    memstreamDestroy(&ms, true);

    // Write the data
    uint64_t datasize = ALGetDataSize(&part->data, &part->metadata);
    //memstreamInit(&ms, malloc(datasize));
    //ALSerializeData(&part->data, &part->metadata, &ms);
    //fwrite(ms.buf, datasize, 1, params->datafp);
    //memstreamDestroy(&ms, true);
    fwrite(part->data, datasize, 1, params->datafp); // Optimization, fix serialization later with file_stream_t

    // Write the index
    uint64_t indexsize = ALGetIndexSize(&part->index, &part->metadata);
    //memstreamInit(&ms, malloc(indexsize));
    //ALSerializeIndex(&part->index, &part->metadata, &ms);
    //fwrite(ms.buf, indexsize, 1, params->iindexfp);
    //memstreamDestroy(&ms, true);
    FILE *indexfp = params->legacyFormat &&
                    part->metadata.indexMeta.indexForm == ALCompressionIndex ?
                        params->cindexfp : params->iindexfp;
    fwrite(part->index, indexsize, 1, indexfp); // Optimization, fix serialization later with file_stream_t

    gmeta->total_elements += part->metadata.partitionLength;
    gmeta->num_partitions++;
    params->cur_meta_offset += metasize;
    params->cur_data_offset += datasize;
    params->cur_index_offset += indexsize;

    return ALErrorNone;
}

ALError ALStoreClosePOSIX(ALStore *store) {
    if (store->access_mode == ACCESS_MODE_WRITE) {
        int err = finalizeMetadata(store);
        if (err != ALErrorNone) return err;
    }

    ALStorePOSIXState *params = (ALStorePOSIXState*)store->impl_state;

    fclose(params->metadatafp);
    fclose(params->datafp);
    fclose(params->iindexfp);
    fclose(params->cindexfp);

    if (params->tmpmetadatafp) {
        // Truncate and close
        freopen(NULL, "w", params->tmpmetadatafp);
        fclose(params->tmpmetadatafp);
    }

    FREE(params->meta_offsets);
    FREE(params->data_offsets);
    FREE(params->index_offsets);
    FREE(store->impl_state);

    return ALErrorNone;
}

#define MAX_LEGACY_BINS (1<<16)
#define MAX_LEGACY_PARTITION_METADATA_SIZE \
    (sizeof(uint64_t) + sizeof(unsigned short int) + MAX_LEGACY_BINS * ( sizeof(unsigned short int) +  2 * sizeof(uint64_t) + sizeof(unsigned char) ))

ALError ALStoreReadPartitionPOSIX(ALStore *store, ALPartitionData *part) {
    if (store->access_mode != ACCESS_MODE_READ) {
        eprintf("[%s] Error: attempt to read partition data from file without being in read access mode.\n", __FUNCTION__);
        return ALErrorSomething; // TODO: Error code
    }

    // Ensure global metadata is loaded
    if (!store->global_meta_loaded)
        ALStoreLoadGlobalMetadataPOSIX(store);

    const ALGlobalMetadata *gmeta = &store->global_meta;
    ALStorePOSIXState *params = (ALStorePOSIXState*)store->impl_state;

    // At this point, all of the file pointers point to the next partition's data
    // > If this is the first partition, data/index fps point to the beginning,
    //   and the metadata fp has just passed the global metadata
    // > If this is a later partition, we're at the right place as a postcondition
    //   of a previous ALStore{Read,Open}PartitionPOSIX call.

    const uint64_t cp = store->cur_partition;

    if (params->legacyFormat) {
        uint64_t metalen = cp != gmeta->num_partitions - 1 ?
                             params->meta_offsets[cp + 1] - params->meta_offsets[cp] :
                             MAX_LEGACY_PARTITION_METADATA_SIZE;

        void *buf = malloc(metalen);
        metalen = fread(buf, 1, metalen, params->metadatafp);

        memstream_t ms = memstreamInitReturn(buf);
        ALDeserializeMetadataLegacy(&part->metadata, &ms);
        memstreamDestroy(&ms, true);

        // TODO: Use ALDeserializeIndex/Data, but use a filestream_t when implemented
        uint64_t dataSize = ALGetDataSize(NULL, &part->metadata);
        part->data = (ALData)malloc(dataSize);
        fread(part->data, 1, dataSize, params->datafp);

        uint64_t indexSize = ALGetIndexSize(NULL, &part->metadata);
        part->index = (ALIndex)malloc(indexSize);
        fread(part->index, 1, indexSize, part->metadata.indexMeta.indexForm == ALInvertedIndex ? params->iindexfp : params->cindexfp);

        return ALErrorNone;
    } else {
        uint64_t metalen = params->meta_offsets[cp + 1] - params->meta_offsets[cp]; // We can do this since we have np+1 offsets

        memstream_t ms = memstreamInitReturn(malloc(metalen));
        metalen = fread(ms.buf, 1, metalen, params->metadatafp);
        ALDeserializeMetadata(&part->metadata, &ms);
        memstreamDestroy(&ms, true);

        uint64_t dataSize = ALGetDataSize(NULL, &part->metadata); // TODO: remove data arg
        part->data = (ALData)malloc(dataSize);
        fread(part->data, 1, dataSize, params->datafp);

        uint64_t indexSize = ALGetIndexSize(NULL, &part->metadata); // TODO: remove index arg
        part->index = (ALIndex)malloc(indexSize);
        fread(part->index, 1, indexSize, params->iindexfp);

        return ALErrorNone;
    }
}

/////////////////
// Partition opening code
/////////////////

ALError ALStoreOpenPartitionPOSIX(ALStore *store, ALPartitionStore *ps, _Bool open_excl) {
    // Disallow non-exclusive opens for now
    if (!open_excl) {
        eprintf("[%s] Non-exclusive partition open mode not currently supported.\n", __FUNCTION__);
        return ALErrorSomething;
    }

    // Ensure global metadata is loaded (we will need this)
    if (!store->global_meta_loaded)
        ALStoreLoadGlobalMetadataPOSIX(store);

    const ALGlobalMetadata *gmeta = &store->global_meta;
    const ALStorePOSIXState *params = (ALStorePOSIXState*)store->impl_state;

    // Now that we've check preconditions, do the initialization
    ALPartitionStoreOpen(ps, store, open_excl);
    ALPartitionStorePOSIXState* pstore_state = (ALPartitionStorePOSIXState*)malloc(sizeof(ALPartitionStorePOSIXState));

    pstore_state->metafp_unchanged = true;
    pstore_state->iindexfp_changed = pstore_state->cindexfp_changed = false;
    pstore_state->meta_offset = params->meta_offsets[store->cur_partition];
    pstore_state->data_offset = params->data_offsets[store->cur_partition];
    pstore_state->index_offset = params->index_offsets[store->cur_partition];

    ps->impl_state = pstore_state;

    return ALErrorNone;
}

ALError ALPartitionStoreReadMetadataPOSIX(ALPartitionStore *ps, ALMetadata *meta) {
    const ALGlobalMetadata *gmeta = &ps->source->global_meta;
    ALStorePOSIXState *params = (ALStorePOSIXState*)ps->source->impl_state;
    ALPartitionStorePOSIXState *pstore_state = (ALPartitionStorePOSIXState*)ps->impl_state;

    // If we aren't already at the beginning of the metadata, go there now
    if (!pstore_state->metafp_unchanged)
        fseek(params->metadatafp, pstore_state->meta_offset, SEEK_SET);

    const uint64_t cp = ps->partition_num;
    if (params->legacyFormat) {
        uint64_t metalen = cp != gmeta->num_partitions - 1 ?
                           params->meta_offsets[cp + 1] - params->meta_offsets[cp] :
                           MAX_LEGACY_PARTITION_METADATA_SIZE;

        void *buf = malloc(metalen); // TODO: Check for out-of-memory
        metalen = fread(buf, 1, metalen, params->metadatafp);

        if (ferror(params->metadatafp)) {
            free(buf);
            return ALErrorSomething; // TODO: Error code
        }

        memstream_t ms = memstreamInitReturn(buf);
        ALDeserializeMetadataLegacy(meta, &ms);
        memstreamDestroy(&ms, true);

        pstore_state->metafp_unchanged = false; // We've moved the file pointer, so remember that

        return ALErrorNone;
    } else {
        uint64_t metalen = params->meta_offsets[cp + 1] - params->meta_offsets[cp];

        memstream_t ms = memstreamInitReturn(malloc(metalen));
        metalen = fread(ms.buf, 1, metalen, params->metadatafp);

        if (ferror(params->metadatafp)) {
            memstreamDestroy(&ms, true);
            return ALErrorSomething; // TODO: Error code
        }

        ALDeserializeMetadata(meta, &ms);
        memstreamDestroy(&ms, true);

        pstore_state->metafp_unchanged = false; // We've moved the file pointer, so remember that

        return ALErrorNone;
    }
}

ALError ALPartitionStoreReadDataBinsPOSIX(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALData *data) {
    ALStorePOSIXState *params = (ALStorePOSIXState*)ps->source->impl_state;
    ALPartitionStorePOSIXState *pstore_state = (ALPartitionStorePOSIXState*)ps->impl_state;

    const char insigbytes = alacrity_util_insigBytesCeil(meta);
    const uint64_t first_bin_off = pstore_state->data_offset + meta->binLayout.binStartOffsets[low_bin] * insigbytes;
    const uint64_t last_bin_off = pstore_state->data_offset + meta->binLayout.binStartOffsets[hi_bin] * insigbytes;
    const uint64_t bin_read_len = last_bin_off - first_bin_off;

    if (*data == NULL)
        *data = malloc(bin_read_len);

    fseek(params->datafp, first_bin_off, SEEK_SET);
    fread(*data, 1, bin_read_len, params->datafp);

    return ferror(params->datafp) ? ALErrorSomething : ALErrorNone; // TODO: Error code
}

ALError ALPartitionStoreReadIndexBinsPOSIX(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t low_bin, bin_id_t hi_bin, ALIndex *index) {
    ALStorePOSIXState *params = (ALStorePOSIXState*)ps->source->impl_state;
    ALPartitionStorePOSIXState *pstore_state = (ALPartitionStorePOSIXState*)ps->impl_state;

    const char insigbytes = alacrity_util_insigBytesCeil(meta);
    const uint64_t first_bin_off = pstore_state->index_offset + ALGetIndexBinOffset(meta, low_bin);
    const uint64_t last_bin_off = pstore_state->index_offset + ALGetIndexBinOffset(meta, hi_bin);
    const uint64_t bin_read_len = last_bin_off - first_bin_off;
    _Bool use_cindex_fp = params->legacyFormat && meta->indexMeta.indexForm == ALCompressionIndex;
    FILE *fp = use_cindex_fp ? params->cindexfp : params->iindexfp;

    if (*index == NULL)
        *index = malloc(bin_read_len);

    fseek(fp, first_bin_off, SEEK_SET);
    fread(*index, 1, bin_read_len, fp);

    if (use_cindex_fp)
        pstore_state->cindexfp_changed = true;
    else
        pstore_state->iindexfp_changed = true;

    return ferror(fp) ? ALErrorSomething : ALErrorNone; // TODO: Error code
}

ALError ALPartitionStoreClosePOSIX(ALPartitionStore *ps) {
    // All the basic stuff is updated (unlocking the parent ALStore, setting
    // the is_open flag to false). What we must do is reset the parent
    // ALStore's file pointers to the cur_partition.

    ALStorePOSIXState *params = (ALStorePOSIXState*)ps->source->impl_state;
    const ALPartitionStorePOSIXState *pstore_state = (ALPartitionStorePOSIXState*)ps->impl_state;

    // Move the file pointers
    fseek(params->metadatafp, params->meta_offsets[ps->source->cur_partition], SEEK_SET);
    fseek(params->datafp, params->data_offsets[ps->source->cur_partition], SEEK_SET);
    if (pstore_state->cindexfp_changed)
        fseek(params->iindexfp, params->index_offsets[ps->source->cur_partition], SEEK_SET);
    if (pstore_state->iindexfp_changed)
        fseek(params->cindexfp, params->index_offsets[ps->source->cur_partition], SEEK_SET);

    // Free the implementation-dependent state data
    FREE(ps->impl_state);
    return ALErrorNone;
}




