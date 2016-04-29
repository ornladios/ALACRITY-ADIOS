#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <alacrity-types.h>
#include <alacrity-filestore.h>
#include <alacrity-serialization.h>
#include <alacrity-serialization-legacy.h>
#include "include/alacrity-util.h"

// If defined, ALFileStore will print a warning to stderr if fewer partitions are appended to a file store than expected
// (this is not an error, it just wastes some disk space)
#define ALFILESTORE_CHECK_PARTITION_ALLOCATION_UNDERFLOW

static uint64_t ALFileStoreGetGlobalHeaderSize(ALFileStore *fs);
static int ALFileStorePreallocateGlobalHeader(ALFileStore *fs);
static int ALFileStoreWriteGlobalHeader(ALFileStore *fs);


ALError ALFileStoreOpen(ALFileStore *fs, const char *basename, uint64_t num_partitions, ALIndexForm indexForm, _Bool legacyFormat) {
    // Set up filename buffer. The prefix (basename) remains the same, we just strcpy the suffix partway into it.
    char buf[256];
    strcpy(buf, basename);
    char *bufappend = buf + strlen(basename);

    _Bool invertedIndex = (indexForm == ALInvertedIndex);

    // Open the requisite files
    strcpy(bufappend, "-metadata.dat");
    fs->metadatafp = fopen(buf, "w");
    strcpy(bufappend, "-compressed_data.dat");
    fs->datafp = fopen(buf, "w");
    strcpy(bufappend, invertedIndex ? "-query_index.dat" : "-index.dat");
    fs->indexfp = fopen(buf, "w");

    // Set the various parameter files of the struct, and malloc necessary lists
    fs->num_partitions = num_partitions;
    fs->total_elements = 0;
    fs->partition_size = 0;

    fs->cur_partition = 0;
    fs->meta_offsets = malloc(num_partitions * sizeof(uint64_t));
    fs->data_offsets = malloc(num_partitions * sizeof(uint64_t));
    fs->index_offsets = malloc(num_partitions * sizeof(uint64_t));

    fs->cur_meta_offset = 0;
    fs->cur_data_offset = 0;
    fs->cur_index_offset = 0;

    fs->legacyFormat = legacyFormat;

    // Finally, preallocate room for the global metadata in the metadata file (i.e., seek past its region)
    ALFileStorePreallocateGlobalHeader(fs);

    return ALErrorNone;
}

ALError ALFileStoreAppend(ALFileStore *fs, const ALPartitionData *part) {
    if (fs->cur_partition == fs->num_partitions) {
        eprintf("%s: Error: preallocated number of partitions (%llu) exceeded\n", __FUNCTION__, fs->num_partitions);
        // We have gone over our pre-allocated number of partitions
        return ALErrorSomething;
    }

    uint64_t cp = fs->cur_partition;
    fs->meta_offsets[cp] = fs->cur_meta_offset;
    fs->data_offsets[cp] = fs->cur_data_offset;
    fs->index_offsets[cp] = fs->cur_index_offset;

    if (fs->partition_size == 0) {
        fs->partition_size = part->metadata.partitionLength;
    } else if (fs->partition_size != part->metadata.partitionLength) {
        if (cp != fs->num_partitions - 1) {
            eprintf("%s: Error: just attempted to append a partition with length %lu, but previous partitions had length %lu, and this is not the final partition\n",
                    __FUNCTION__, part->metadata.partitionLength, fs->partition_size);
            return ALErrorSomething;
        }
    }

    // Memstream, to be reused several times
    memstream_t ms;

    // Write the metadata
    uint64_t metasize = fs->legacyFormat ? ALGetMetadataSizeLegacy(&part->metadata) : ALGetMetadataSize(&part->metadata);
    memstreamInit(&ms, malloc(metasize));
    if (fs->legacyFormat)
        ALSerializeMetadataLegacy(&part->metadata, &ms);
    else
        ALSerializeMetadata(&part->metadata, &ms);
    fwrite(ms.buf, metasize, 1, fs->metadatafp);
    memstreamDestroy(&ms, true);

    // Write the data
    uint64_t datasize = ALGetDataSize(&part->data, &part->metadata);
    memstreamInit(&ms, malloc(datasize));
    ALSerializeData(&part->data, &part->metadata, &ms);
    fwrite(ms.buf, datasize, 1, fs->datafp);
    memstreamDestroy(&ms, true);

    // Write the index
    uint64_t indexsize = ALGetIndexSize(&part->index, &part->metadata);
    memstreamInit(&ms, malloc(indexsize));
    ALSerializeIndex(&part->index, &part->metadata, &ms);
    fwrite(ms.buf, indexsize, 1, fs->indexfp);
    memstreamDestroy(&ms, true);

    fs->total_elements += part->metadata.partitionLength;
    fs->cur_meta_offset += metasize;
    fs->cur_data_offset += datasize;
    fs->cur_index_offset += indexsize;
    fs->cur_partition++;

    return ALErrorNone;
}

ALError ALFileStoreClose(ALFileStore *fs) {
#ifdef ALFILESTORE_CHECK_PARTITION_ALLOCATION_UNDERFLOW
    if (fs->cur_partition != fs->num_partitions) {
        fprintf(stderr, "Warning: %s just closed a file with %llu partitions that was pre-allocated for %llu partitions (this is not fatal, it just wastes a little space in the file)\n",
                __FUNCTION__, fs->cur_partition, fs->num_partitions);
    }
#endif

    int err = ALFileStoreWriteGlobalHeader(fs);
    if (err != ALErrorNone)
        return err;

    fclose(fs->metadatafp);
    fclose(fs->datafp);
    fclose(fs->indexfp);

    free(fs->meta_offsets);
    free(fs->data_offsets);
    free(fs->index_offsets);

    return ALErrorNone;
}

// Utility functions

static uint64_t ALFileStoreGetGlobalHeaderSize(ALFileStore *fs) {
    return	sizeof(uint64_t) +	// total_elements
            sizeof(uint64_t) +	// num_partitions
            3 * fs->num_partitions * sizeof(uint64_t); // meta, data and index offsets
}

static int ALFileStorePreallocateGlobalHeader(ALFileStore *fs) {
    uint64_t size = ALFileStoreGetGlobalHeaderSize(fs);
    fseek(fs->metadatafp, size, SEEK_SET);
    return ALErrorNone;
}

static int ALFileStoreWriteGlobalHeader(ALFileStore *fs) {
    fseek(fs->metadatafp, 0, SEEK_SET); // Return to the beginning of the file

    memstream_t ms = memstreamInitReturn(malloc(ALFileStoreGetGlobalHeaderSize(fs)));

    memstreamAppendUint64(&ms, fs->total_elements);
    memstreamAppendUint64(&ms, fs->partition_size);
    memstreamAppendArray(&ms, fs->meta_offsets, sizeof(uint64_t), fs->num_partitions);
    memstreamAppendArray(&ms, fs->data_offsets, sizeof(uint64_t), fs->num_partitions);
    memstreamAppendArray(&ms, fs->index_offsets, sizeof(uint64_t), fs->num_partitions);

    fwrite(ms.buf, memstreamGetPosition(&ms), 1, fs->metadatafp);
    memstreamDestroy(&ms, true);

    return ALErrorNone;
}

/*
static int ALFileStoreReadGlobalHeader(ALFileStore *fs) {
    fread(&fs->total_elements, sizeof(uint64_t), 1, fs->metadatafp);
    fread(&fs->partition_size, sizeof(uint64_t), 1, fs->metadatafp);

    fs->meta_offsets = malloc(fs->num_partitions * sizeof(uint64_t));
    fs->data_offsets = malloc(fs->num_partitions * sizeof(uint64_t));
    fs->index_offsets = malloc(fs->num_partitions * sizeof(uint64_t));

    fread(&fs->meta_offsets, sizeof(uint64_t), fs->num_partitions, fs->metadatafp);
    fread(&fs->data_offsets, sizeof(uint64_t), fs->num_partitions, fs->metadatafp);
    fread(&fs->index_offsets, sizeof(uint64_t), fs->num_partitions, fs->metadatafp);

    return ALErrorNone;
}

ALError ALFileStoreOpenForRead(ALFileStore *fs, const char *basename, _Bool invertedIndex, _Bool legacyFormat) {
    fs->legacyFormat = legacyFormat;

    // Set up filename buffer. The prefix (basename) remains the same, we just strcpy the suffix partway into it.
    char buf[256];
    strcpy(buf, basename);
    char *bufappend = buf + strlen(basename);

    // Open the requisite files
    strcpy(bufappend, "-metadata.dat");
    fs->metadatafp = fopen(buf, "r");

    ALFileStoreReadGlobalHeader(fs, &invertedIndex);

    strcpy(bufappend, "-compressed_data.dat");
    fs->datafp = fopen(buf, "r");
    strcpy(bufappend, invertedIndex ? "-query_index.dat" : "-index.dat");
    fs->indexfp = fopen(buf, "r");

    fs->cur_partition = 0;
    fs->meta_offsets = malloc(fs->num_partitions * sizeof(uint64_t));
    fs->data_offsets = malloc(fs->num_partitions * sizeof(uint64_t));
    fs->index_offsets = malloc(fs->num_partitions * sizeof(uint64_t));

    fs->cur_meta_offset = 0;
    fs->cur_data_offset = 0;
    fs->cur_index_offset = 0;

    return ALErrorNone;
}

ALError ALFileStoreRead(ALFileStore *fs, ALPartitionData *part) {
    if (fs->cur_partition == fs->num_partitions) {
        // We have run out of partitions
        return ALErrorSomething;
    }

    // TODO: Finish this function
    return ALErrorSomething;

    uint64_t cp = fs->cur_partition;
    fs->meta_offsets[cp] = fs->cur_meta_offset;
    fs->data_offsets[cp] = fs->cur_data_offset;
    fs->index_offsets[cp] = fs->cur_index_offset;

    if (fs->partition_size == 0) {
        fs->partition_size = part->metadata.partitionLength;
    } else if (fs->partition_size != part->metadata.partitionLength) {
        if (cp != fs->num_partitions - 1) {
            fprintf(stderr, "Error: %s just attempted to append a partition with length %lu, but previous partitions had length %lu, and this is not the final partition\n",
                    __FUNCTION__, part->metadata.partitionLength, fs->partition_size);
            return ALErrorSomething;
        }
    }

    // Write the metadata
    uint64_t metasize = fs->legacyFormat ? ALGetMetadataSizeLegacy(&part->metadata) : ALGetMetadataSize(&part->metadata);
    void *metabuf = malloc(metasize);
    if (fs->legacyFormat)
        ALSerializeMetadataLegacy(&part->metadata, metabuf, &metasize);
    else
        ALSerializeMetadata(&part->metadata, metabuf, &metasize);
    fwrite(metabuf, metasize, 1, fs->metadatafp);
    free(metabuf);

    // Write the data
    uint64_t datasize = ALGetDataSize(&part->data, &part->metadata);
    void *databuf = malloc(datasize);
    ALSerializeData(&part->data, &part->metadata, databuf, &datasize);
    fwrite(databuf, datasize, 1, fs->datafp);
    free(databuf);

    // Write the index
    uint64_t indexsize = ALGetIndexSize(&part->index, &part->metadata);
    void *indexbuf = malloc(indexsize);
    ALSerializeIndex(&part->index, &part->metadata, indexbuf, &indexsize);
    fwrite(indexbuf, indexsize, 1, fs->indexfp);
    free(indexbuf);

    fs->total_elements += part->metadata.partitionLength;
    fs->cur_meta_offset += metasize;
    fs->cur_data_offset += datasize;
    fs->cur_index_offset += indexsize;
    fs->cur_partition++;

    return ALErrorNone;
}
*/
