#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>

#include <alacrity.h>
#include <timer.h>
#include "../src/include/alacrity-util.h"

int main (int argc, char *argv [])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input filename> <elemsize> <sigbits>\n", argv[0]);
        return 1;
    }

    ALTimer_init();

    const char *infilename = argv[1];
    int elemsize = atoi(argv[2]);

    struct stat st;
    stat(infilename, &st);

    uint64_t numDoubles = st.st_size / elemsize;
    void *data = malloc(numDoubles * elemsize);

    FILE *infile = fopen(infilename, "r");
    if (infile == NULL) {
        fprintf(stderr, "Error opening input file %s\n", infilename);
        return 1;
    }

    int rcount = fread(data, elemsize, numDoubles, infile);
    if (rcount != numDoubles) {
        fprintf(stderr, "Expected %d doubles, read %d\n", numDoubles, rcount);
        return 1;
    }

    ALEncoderConfig config;
    ALEncoderConfigure(&config, atoi(argv[3]), elemsize == 4 ? DATATYPE_FLOAT32 : DATATYPE_FLOAT64, ALInvertedIndex);

    ALPartitionData output;
    ALBinLookupTable lookupTable;

    ALTimer_start("buildBinLayout");
    ALBuildBinLayout(&config, data, numDoubles, &lookupTable, &output);
    ALTimer_stop("buildBinLayout");

    ALTimer_start("buildIndex");
    ALBuildInvertedIndexFromLayout(&config, data, numDoubles, &lookupTable, &output);
    ALTimer_stop("buildIndex");

    dbprintf("Encoded the data with an inverted index\n");

    uint64_t data_size = ALGetDataSize(NULL, &output.metadata);
    uint64_t metadata_size = ALGetMetadataSize(&output.metadata);

    uint64_t old_size = ALGetIndexSize(&output.index, &output.metadata);

    ALTimer_start("compressIndex");
    ALConvertIndexForm(&output.metadata, &output.index, ALCompressedInvertedIndex);
    ALTimer_stop("compressIndex");
    dbprintf("Compressed inverted index\n");

    uint64_t new_size = ALGetIndexSize(&output.index, &output.metadata);

    printf("Input file = %s\n", argv[1]);
    printf("Params: %d elemsize, %d sigbits\n", (int)output.metadata.elementSize, (int)output.metadata.significantBits);
    printf("Input file size = %llu\n", sizeof(double) * numDoubles); 
    printf("Number of bins = %llu\n", (uint64_t)output.metadata.binLayout.numBins);
    printf("Old index size = %llu\n", old_size);
    printf("New index size = %llu\n", new_size);
    printf("Index Compression ratio = %f\n", 1.0 * old_size / new_size);
    printf("Old index, data, metadata = %llu\n", data_size + metadata_size + old_size);
    printf("New index, data, metadata = %llu\n", data_size + metadata_size + new_size);
    printf("Overall Compression ratio = %f\n", 1.0 * (data_size + metadata_size + old_size) / (data_size + metadata_size + new_size));
    dbprintf("Done\n");

    ALTimer_print_timers_short();

    ALStore store;
    ALStoreOpenPOSIX(&store, "/intrepid-fs1/users/daboyuka/scratch/q", "w", false);
    ALStoreWritePartition(&store, &output);
    ALStoreClose(&store);

    ALPartitionDataDestroy(&output);
    fclose(infile);

    return 0;
}
