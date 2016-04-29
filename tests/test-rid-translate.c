#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>

#include <alacrity.h>
#include "../src/include/alacrity-util.h"

#define N 1024
#define DELTA1 ((int32_t)50)
#define DELTA2 ((int32_t)-25)

int main (int argc, char *argv [])
{
    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return 1;
    }

    double *data = malloc(N * sizeof(double));

    ALEncoderConfig config;
    ALEncoderConfigure(&config, 16, DATATYPE_FLOAT64, ALInvertedIndex);

    ALPartitionData output;
    ALEncode(&config, data, N, &output);

    printf("Encoded the data with an inverted index\n");
    printf("Num bins: %d\n", (int)output.metadata.binLayout.numBins);

    rid_t *old_rids = malloc(N * sizeof(rid_t));
    memcpy(old_rids, output.index, N * sizeof(rid_t));

    ALTranslateRIDs(&output.metadata, &output.index, DELTA1);
    printf("Attempted to translate inverted index by %d...\n", DELTA1);

    rid_t *ip = (rid_t*)output.index;
    for (bin_offset_t i = 0; i < N; i++) {
        if ((old_rids[i] + DELTA1) != ip[i]) {
            fprintf(stderr, "Error! RID is not properly offset at position %u (original %lu, new %lu, should be %lu) DELTA %d",
                    i, old_rids[i], ip[i], (old_rids[i] + DELTA1), DELTA1);
            abort();
        }
    }
    printf("Inverted index properly translated by %d\n", DELTA1);

    ALConvertIndexForm(&output.metadata, &output.index, ALCompressedInvertedIndex);
    printf("Compressed inverted index\n");

    ALTranslateRIDs(&output.metadata, &output.index, DELTA2);
    printf("Attempted to translate compressed inverted index by %d...\n", DELTA2);

    ALConvertIndexForm(&output.metadata, &output.index, ALInvertedIndex);
    printf("Decompressed inverted index\n");

    for (bin_offset_t i = 0; i < N; i++) {
        if (old_rids[i] + DELTA1 + DELTA2 != ((rid_t*)output.index)[i]) {
            fprintf(stderr, "Error! RID is not properly offset at position %u (original %u, new %u, should be %u)",
                    old_rids[i], ((rid_t*)output.index)[i], old_rids[i] + DELTA1 + DELTA2 );
            abort();
        }
    }
    printf("Compressed inverted index properly translated\n");

    ALPartitionDataDestroy(&output);

    return 0;
}
