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


int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input filename> <output filename base> <invert index?>\n", argv[0]);
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
    const char *outfilenamebase = argv[2];
    _Bool invertIndex = atoi(argv[3]) > 0;
    char outindexfilename[256];
    char outdatafilename[256];
    strcpy(outindexfilename, outfilenamebase);
    strcat(outindexfilename, invertIndex ? "-query_index.dat" : "-index.dat");
    strcpy(outdatafilename, outfilenamebase);
    strcat(outdatafilename, "-compressed_data.dat");

    struct stat st;
    stat(infilename, &st);

    uint64_t numDoubles = st.st_size / sizeof(double);
    void *data = malloc(numDoubles * sizeof(double));

    FILE *infile = fopen(infilename, "r");
    FILE *outindexfile = fopen(outindexfilename, "w");
    FILE *outdatafile = fopen(outdatafilename, "w");
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

    int rcount = fread(data, sizeof(double), numDoubles, infile);
    if (rcount != numDoubles) {
        fprintf(stderr, "Expected %d doubles, read %d\n", numDoubles, rcount);
        return 1;
    }

    ALEncoderConfig config;
    ALEncoderConfigure(&config, 16, DATATYPE_FLOAT64, invertIndex ? ALInvertedIndex : ALCompressionIndex);

    ALPartitionData output;
    ALEncode(&config, data, numDoubles, &output);

    int insigbytes = output.metadata.elementSize - output.metadata.significantBits;

    int wcount = fwrite(output.index, invertIndex ? sizeof(rid_t) : output.metadata.significantBits, numDoubles, outindexfile);
    if (wcount != numDoubles) {
        fprintf(stderr, "Expected write %d index elements, wrote %d\n", numDoubles, wcount);
        return 1;
    }

    wcount = fwrite(output.data, insigbytes, numDoubles, outdatafile);
    if (wcount != numDoubles) {
        fprintf(stderr, "Expected write %d data elements, wrote %d\n", numDoubles, wcount);
        return 1;
    }

    if (argc == 5) {
        const char *compfilenamebase = argv[4];
        char compindexfilename[256];
        char compdatafilename[256];
        strcpy(compindexfilename, compfilenamebase);
        strcat(compindexfilename, invertIndex ? "-query_index.dat" : "-index.dat");
        strcpy(compdatafilename, compfilenamebase);
        strcat(compdatafilename, "-compressed_data.dat");
        printf("Reading comparison files %s and %s\n", compdatafilename, compindexfilename);

        stat(compdatafilename, &st);

        uint64_t numDoubles = st.st_size / (insigbytes);

        uint64_t datasize = numDoubles * insigbytes;
        uint64_t indexsize = numDoubles * (invertIndex ? sizeof(rid_t) : output.metadata.significantBits);
        void *compdata = malloc(datasize);
        void *compindex = malloc(indexsize);
        assert(compdata != NULL);
        assert(compindex != NULL);

        FILE *cdf = fopen(compdatafilename, "r");
        FILE *cif = fopen(compindexfilename, "r");
        assert(cdf);
        assert(cif);

        int r1 = fread(compdata, 1, datasize, cdf);
        int r2 = fread(compindex, 1, indexsize, cif);
        fclose(cdf);
        fclose(cif);

        assert(r1 == datasize);
        assert(r2 == indexsize);

        printf("Testing against comparison data file %s...\n", compdatafilename);
        assert(memcmp(output.data, compdata, datasize) == 0);
        printf("Success!\n");
        printf("Testing against comparison index file %s...\n", compindexfilename);
        assert(memcmp(output.index, compindex, indexsize) == 0);
        printf("Success!\n");
    }

    fclose(infile);
    fclose(outindexfile);
    fclose(outdatafile);

    return 0;
}
