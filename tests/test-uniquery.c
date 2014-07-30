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
#include <uniquery.h>
#include <ALUtil.h>

const _Bool USE_LEGACY_FORMAT = false;

int main(int argc, char **argv) {
    if (argc < 5 || argc > 6) {
        fprintf(stderr, "Usage: %s <input filename> <output filename base> <low val> <high val> [<use CII? default false>]\n", argv[0]);
        return 1;
    }

    const char *infilename = argv[1];
    const char *outfilenamebase = argv[2];
    double lval = atof(argv[3]);
    double uval = atof(argv[4]);
    _Bool useCII = argc >= 6 ? atoi(argv[5]) > 0 : false;

    struct stat st;
    stat(infilename, &st);

    uint64_t numDoubles = st.st_size / sizeof(double);
    void *data = malloc(numDoubles * sizeof(double));

    FILE *infile = fopen(infilename, "r");
    if (infile == NULL) {
        fprintf(stderr, "Error opening input file %s\n", infilename);
        return 1;
    }

    int rcount = fread(data, sizeof(double), numDoubles, infile);
    if (rcount != numDoubles) {
        fprintf(stderr, "Expected %d doubles, read %d\n", numDoubles, rcount);
        return 1;
    }
    fclose(infile);

    ALEncoderConfig config;
    ALEncoderConfigure(&config, 16, DATATYPE_FLOAT64, ALInvertedIndex);

    ALPartitionData output;
    ALEncode(&config, data, numDoubles, &output);

    free(data); // Don't need raw data anymore

    printf("Encoded data in ALACRITY format with inverted index\n");

    if (useCII) {
        printf("Using compressed inverted index...\n");
        ALConvertIndexForm(&output.metadata, &output.index, ALCompressedInvertedIndex);
        printf("Compressed inverted index\n");
    }

    uint8_t insigbytes = insigBytesCeil(&output.metadata);

    ALStore store;
    ALStoreOpenPOSIX(&store, outfilenamebase, "w", USE_LEGACY_FORMAT);
    ALStoreWritePartition(&store, &output);
    ALStoreClose(&store);

    ALPartitionDataDestroy(&output); // We don't need the in-memory partition anymore

    printf("Stored the partition in ALACRITY file format\n");

    printf("Opening ALACRITY file for query...\n");
    ALStoreOpenPOSIX(&store, outfilenamebase, "r", USE_LEGACY_FORMAT);

    ALQueryEngine qe;
    ALQueryEngineInit(&qe, &store, true);

    printf("Performing query for values in range %lf to %lf...\n", lval, uval);

    ALUnivariateQuery uniquery;
    ALQueryEngineStartUnivariateDoubleQuery(&qe, lval, uval, VALUE_RETRIEVAL_QUERY_TYPE, &uniquery);

    ALUnivariateQueryResult result;
    while (ALQueryNextResult(&uniquery, &result)) {
        printf("Read %llu results\n", result.resultCount);
        ALQueryResultDestroy(&result);
    }

    ALStoreClose(&store);

    return 0;
}
