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
#include <ALUtil.h>

const _Bool USE_LEGACY_FORMAT = false;

static void cmp_metadata(ALMetadata *m1, ALMetadata *m2) {
    assert(m1->elementSize == m2->elementSize);
    assert(m1->significantBits == m2->significantBits);
    assert(m1->indexMeta.indexForm == m2->indexMeta.indexForm);
    assert(m1->partitionLength == m2->partitionLength);
    assert(m1->binLayout.numBins == m2->binLayout.numBins);
    assert(memcmp(m1->binLayout.binValues, m2->binLayout.binValues, m1->binLayout.numBins * sizeof(bin_offset_t)) == 0);
    assert(memcmp(m1->binLayout.binStartOffsets, m2->binLayout.binStartOffsets, (m1->binLayout.numBins+1) * sizeof(bin_offset_t)) == 0);
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <input filename> <output filename base> <invert index?> [compare to filename base]\n", argv[0]);
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
    ALEncoderConfigure(&config, 16, DATATYPE_FLOAT64, invertIndex ? ALInvertedIndex : ALCompressionIndex);

    ALPartitionData output;
    ALEncode(&config, data, numDoubles, &output);
    free(data); // Don't need raw data anymore

    printf("Encoded data in ALACRITY format\n");
    ALConvertIndexForm(&output.metadata, &output.index, ALCompressedInvertedIndex);

    printf("Compressed inverted index\n");

    uint8_t insigbytes = insigBytesCeil(&output.metadata);

    ALStore store;
    ALStoreOpenPOSIX(&store, outfilenamebase, "w", USE_LEGACY_FORMAT);
    ALStoreWritePartition(&store, &output);
    ALStoreClose(&store);

    printf("Stored the partition in ALACRITY file format\n");

    if (argc == 5) {
        const char *compfilenamebase = argv[4];
        char mymetafilename[256];
        char compmetafilename[256];
        char compindexfilename[256];
        char compdatafilename[256];
        strcpy(mymetafilename, outfilenamebase);
        strcat(mymetafilename, "-metadata.dat");
        strcpy(compmetafilename, compfilenamebase);
        strcat(compmetafilename, "-metadata.dat");
        strcpy(compindexfilename, compfilenamebase);
        strcat(compindexfilename, invertIndex ? "-query_index.dat" : "-index.dat");
        strcpy(compdatafilename, compfilenamebase);
        strcat(compdatafilename, "-compressed_data.dat");
        printf("Reading comparison files %s, %s and %s\n", compmetafilename, compdatafilename, compindexfilename);

        stat(compdatafilename, &st);
        uint64_t numDoubles = st.st_size / (insigbytes);

        stat(mymetafilename, &st);
        uint64_t mymetasize = st.st_size;

        stat(compmetafilename, &st);
        uint64_t metasize = st.st_size;

        uint64_t datasize = numDoubles * insigbytes;
        uint64_t indexsize = ALGetIndexSize(&output.index, &output.metadata);
        void *mymeta = malloc(mymetasize);
        void *compmeta = malloc(metasize);
        void *compdata = malloc(datasize);
        void *compindex = malloc(indexsize);
        assert(mymeta != NULL);
        assert(compmeta != NULL);
        assert(compdata != NULL);
        assert(compindex != NULL);

        FILE *mmf = fopen(mymetafilename, "r");
        FILE *cmf = fopen(compmetafilename, "r");
        FILE *cdf = fopen(compdatafilename, "r");
        FILE *cif = fopen(compindexfilename, "r");
        assert(mmf);
        assert(cmf);
        assert(cdf);
        assert(cif);

        int rm = fread(mymeta, 1, mymetasize, mmf);
        int r0 = fread(compmeta, 1, metasize, cmf);
        int r1 = fread(compdata, 1, datasize, cdf);
        int r2 = fread(compindex, 1, indexsize, cif);
        fclose(mmf);
        fclose(cmf);
        fclose(cdf);
        fclose(cif);

        assert(rm == mymetasize);
        assert(r0 == metasize);
        assert(r1 == datasize);
        assert(r2 == indexsize);

        printf("Testing against comparison metadata file %s...\n", compmetafilename);
        assert(mymetasize == metasize);
        assert(memcmp(mymeta, compmeta, metasize) == 0);
        printf("Success!\n");
        printf("Testing against comparison data file %s...\n", compdatafilename);
        assert(memcmp(output.data, compdata, datasize) == 0);
        printf("Success!\n");
        printf("Testing against comparison index file %s...\n", compindexfilename);
        assert(memcmp(output.index, compindex, indexsize) == 0);
        printf("Success!\n");

        free(compmeta);
        free(compdata);
        free(compindex);
        free(mymeta);
    }

    printf("Loading ALACRITY data back through POSIX ALStore interface to compare...\n");
    ALPartitionData comppart;
    ALStoreOpenPOSIX(&store, outfilenamebase, "r", USE_LEGACY_FORMAT);
    ALStoreReadPartition(&store, &comppart);
    ALStoreClose(&store);

    cmp_metadata(&output.metadata, &comppart.metadata);
    assert(memcmp(output.data, comppart.data, ALGetDataSize(&output.data, &output.metadata)) == 0);
    assert(memcmp(output.index, comppart.index, ALGetIndexSize(&output.index, &output.metadata)) == 0);
    printf("Success!\n");
    ALPartitionDataDestroy(&comppart);
    ALPartitionDataDestroy(&output);
    return 0;
}
