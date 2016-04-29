#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>

#include <alacrity.h>
#include "../src/include/alacrity-util.h"

// #include "timer.h"

int main(int argc, char **argv) {
    if (argc < 5 || argc > 6) {
        fprintf(stderr, "Usage: %s <input filename> <element size in bytes> <output filename base> <invert index?> [compare to filename base]\n", argv[0]);
        return 1;
    }

    /*
    uint64_t test = 0x1234567887654321;
    high_order_bytes_t hi;
    low_order_bytes_t lo;
    SPLIT_DATUM(&test, 8, 2, hi, lo);
    printf("%016llx -> %08lx:%016llx", test, hi, lo);
    return 0;
    */

    const char *infilename = argv[1];
    const char *outfilenamebase = argv[3];
    _Bool invertIndex = atoi(argv[4]) > 0;
    uint8_t elementSize = atoi (argv [2]);

    char outindexfilename[256];
    char outdatafilename[256];
    char outmetadatafilename[256];
    strcpy(outindexfilename, outfilenamebase);
    strcat(outindexfilename, invertIndex ? "-query_index.dat" : "-index.dat");
    strcpy(outdatafilename, outfilenamebase);
    strcat(outdatafilename, "-compressed_data.dat");
    strcpy(outmetadatafilename, outfilenamebase);
    strcat(outmetadatafilename, "-metadata.dat");

    struct stat st;
    stat(infilename, &st);

    uint64_t numElements = st.st_size / elementSize;
    void *data = malloc(numElements * elementSize);

    FILE *infile = fopen(infilename, "r");
    FILE *outindexfile = fopen(outindexfilename, "w");
    FILE *outdatafile = fopen(outdatafilename, "w");
    FILE *outmetadatafile = fopen(outmetadatafilename, "w");

    if (infile == NULL) {
        fprintf(stderr, "Error opening input file %s\n", infilename);
        return 1;
    }
    if (outindexfile == NULL) {
        fprintf(stderr, "Error opening output file %s\n", outindexfilename);
        return 1;
    }
    if (outdatafile == NULL) {
        fprintf(stderr, "Error opening output file %s\n", outdatafilename);
        return 1;
    }
    if (outmetadatafile == NULL) {
        fprintf(stderr, "Error opening output file %s\n", outmetadatafilename);
        return 1;
    }

    int rcount = fread(data, elementSize, numElements, infile);
    if (rcount != numElements) {
        fprintf(stderr, "Expected %d doubles, read %d\n", numElements, rcount);
        return 1;
    }

    // start_clock ();
    ALEncoderConfig config;
    ALEncoderConfigure(&config, 16, elementSize == 4 ? DATATYPE_FLOAT32 : DATATYPE_FLOAT64, invertIndex ? ALInvertedIndex : ALCompressionIndex);

    ALPartitionData output;
    ALBinLookupTable binLookupTable;

    ALBuildBinLayout (&config, data, numElements, &binLookupTable, &output);

    ALBuildInvertedIndexFromLayout (&config, data, numElements, &binLookupTable, &output);
	ALConvertIndexForm (&output.metadata, &output.index, ALCompressedInvertedIndex);
    ALPrintBinLayout ((output.metadata.binLayout));
    int insigbits = output.metadata.elementSize - output.metadata.significantBits;

    // stop_clock ();
    // double index_time = get_current_time ();

    // printf ("Indexing throughput: %lf MB /s\n", st.st_size / index_time / 1024 / 1024);
    int wcount = fwrite(output.index, 1, ALGetIndexSize(NULL, &(output.metadata)), outindexfile);
    if (wcount != ALGetIndexSize(NULL, &(output.metadata))) {
        fprintf(stderr, "Expected write %d index elements, wrote %d\n", numElements, wcount);
        return 1;
    }

    wcount = fwrite(output.data, alacrity_util_insigBytesCeil(&output.metadata), numElements, outdatafile);
    if (wcount != numElements) {
        fprintf(stderr, "Expected write %d data elements, wrote %d\n", numElements, wcount);
        return 1;
    }

    memstream_t ms;
    memstreamInit(&ms, malloc(ALGetMetadataSize (&output.metadata)));
	ALSerializeMetadata (&(output.metadata), &ms);

    uint64_t num_partitions = 1;
    uint64_t total_elements = output.metadata.partitionLength;
    uint64_t partition_size = output.metadata.partitionLength;

    uint64_t meta_offsets [] = {0LL, ALGetMetadataSize(&(output.metadata))};
    uint64_t data_offsets [] = {0LL, ALGetDataSize(NULL, &(output.metadata))};
    uint64_t index_offsets [] = {0LL, ALGetIndexSize(NULL, &(output.metadata))};

    fwrite (&total_elements, sizeof (uint64_t), 1, outmetadatafile);
    fwrite (&partition_size, sizeof (uint64_t), 1, outmetadatafile);
    fwrite (&num_partitions, sizeof (uint64_t), 1, outmetadatafile);
    fwrite (meta_offsets, sizeof (uint64_t), num_partitions + 1, outmetadatafile);
    fwrite (data_offsets, sizeof (uint64_t), num_partitions + 1, outmetadatafile);
    fwrite (index_offsets, sizeof (uint64_t), num_partitions + 1, outmetadatafile);

    wcount = fwrite(ms.buf, sizeof (char), ALGetMetadataSize (&output.metadata), outmetadatafile);
    if (wcount != ALGetMetadataSize (&output.metadata)) {
        fprintf(stderr, "Expected write %d data elements, wrote %d\n", ALGetMetadataSize (&output.metadata), wcount);
        return 1;
    }

	memstreamDestroy (&ms, true);
	
    // if (argc == 5) {
    //     const char *compfilenamebase = argv[4];
    //     char compindexfilename[256];
    //     char compdatafilename[256];
    //     strcpy(compindexfilename, compfilenamebase);
    //     strcat(compindexfilename, invertIndex ? "-query_index.dat" : "-index.dat");
    //     strcpy(compdatafilename, compfilenamebase);
    //     strcat(compdatafilename, "-compressed_data.dat");
    //     printf("Reading comparison files %s and %s\n", compdatafilename, compindexfilename);

    //     stat(compdatafilename, &st);

    //     uint64_t numElements = st.st_size / (insigbytes);

    //     uint64_t datasize = numElements * insigbytes;
    //     uint64_t indexsize = numElements * (invertIndex ? sizeof(rid_t) : output.metadata.significantBits);
    //     void *compdata = malloc(datasize);
    //     void *compindex = malloc(indexsize);
    //     assert(compdata != NULL);
    //     assert(compindex != NULL);

    //     FILE *cdf = fopen(compdatafilename, "r");
    //     FILE *cif = fopen(compindexfilename, "r");
    //     assert(cdf);
    //     assert(cif);

    //     int r1 = fread(compdata, 1, datasize, cdf);
    //     int r2 = fread(compindex, 1, indexsize, cif);
    //     fclose(cdf);
    //     fclose(cif);

    //     assert(r1 == datasize);
    //     assert(r2 == indexsize);

    //     printf("Testing against comparison data file %s...\n", compdatafilename);
    //     assert(memcmp(output.data, compdata, datasize) == 0);
    //     printf("Success!\n");
    //     printf("Testing against comparison index file %s...\n", compindexfilename);
    //     assert(memcmp(output.index, compindex, indexsize) == 0);
    //     printf("Success!\n");
    // }

    fclose(infile);
    fclose(outindexfile);
    fclose(outdatafile);
    fclose(outmetadatafile);

    return 0;
}
