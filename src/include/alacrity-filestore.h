#ifndef ALACRITY_FILESTORE_H_
#define ALACRITY_FILESTORE_H_

#include <stdint.h>
#include <alacrity-types.h>

// TODO: Use pointer indirection to make this struct opaque
// (i.e., typedef a pointer to this struct as ALFileStore, and dereference it in the main code)
typedef struct {
    FILE *metadatafp, *datafp, *indexfp;

    uint64_t num_partitions;
    uint64_t total_elements;
    uint64_t partition_size;

    uint64_t *meta_offsets;
    uint64_t *data_offsets;
    uint64_t *index_offsets;

    uint64_t cur_partition;
    uint64_t cur_meta_offset;
    uint64_t cur_data_offset;
    uint64_t cur_index_offset;

    _Bool legacyFormat;
} ALFileStore;

ALError ALFileStoreOpen(ALFileStore *fs, const char *basename, uint64_t num_partitions, ALIndexForm indexForm, _Bool legacyFormat);
ALError ALFileStoreAppend(ALFileStore *fs, const ALPartitionData *part);
ALError ALFileStoreClose(ALFileStore *fs);

#endif