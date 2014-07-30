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

static int cmp_metadata(const ALMetadata *m1, const ALMetadata *m2) {
    assert(m1->elementSize == m2->elementSize);
    assert(m1->significantBits == m2->significantBits);
    assert(m1->indexMeta.indexForm == m2->indexMeta.indexForm);
    assert(m1->partitionLength == m2->partitionLength);
    assert(m1->binLayout.numBins == m2->binLayout.numBins);
    assert(memcmp(m1->binLayout.binValues, m2->binLayout.binValues, m1->binLayout.numBins * sizeof (bin_offset_t)) == 0);
    assert(memcmp(m1->binLayout.binStartOffsets, m2->binLayout.binStartOffsets, (m1->binLayout.numBins + 1) * sizeof(bin_offset_t)) == 0);
    return 0;
}

static int cmp_parts(const ALPartitionData *p1, const ALPartitionData *p2) {
    assert(cmp_metadata(&p1->metadata, &p2->metadata) == 0);
    const uint64_t plen = p1->metadata.partitionLength;
    assert(memcmp(p1->data, p2->data, ALGetDataSize(&p1->data, &p1->metadata)) == 0);
    assert(memcmp(p1->index, p2->index, ALGetIndexSize(&p1->index, &p1->metadata)) == 0);
    return 0;
}

static int cmp_files(const char *f1, const char *f2) {
    printf("Comparing files %s and %s for content...\n", f1, f2);

    struct stat st;

    stat(f1, &st);
    uint64_t f1size = st.st_size;
    stat(f2, &st);
    uint64_t f2size = st.st_size;

    assert(f1size == f2size);

    void *f1buf = malloc(f1size);
    void *f2buf = malloc(f2size);
    assert(f1buf != NULL && f2buf != NULL);

    FILE *f1fp = fopen(f1, "r");
    FILE *f2fp = fopen(f2, "r");
    assert(f1fp);
    assert(f2fp);

    size_t r1 = fread(f1buf, 1, f1size, f1fp);
    size_t r2 = fread(f2buf, 1, f2size, f2fp);
    fclose(f1fp);
    fclose(f2fp);

    assert(r1 == f1size);
    assert(r2 == f2size);

    assert(memcmp(f1buf, f2buf, f1size) == 0);
    printf("Success!\n");

    free(f1buf);
    free(f2buf);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <input filename> <output filename base> <part size (in doubles)> [compare to filename base]\n", argv[0]);
        return 1;
    }

    const char *infilename = argv[1];
    const char *outfilenamebase = argv[2];
    uint64_t partsize = atoll(argv[3]);

    struct stat st;
    stat(infilename, &st);

    uint64_t numDoubles = st.st_size / sizeof(double);
    void *data = malloc(numDoubles * sizeof(double));

    uint64_t num_parts = (numDoubles - 1) / partsize + 1;

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

    // Peanut buffer encoding time!
    ALEncoderConfig config;
    ALEncoderConfigure(&config, 16, DATATYPE_FLOAT64, ALInvertedIndex);

    ALStore store;
    printf("Opening ALStore for storage of ALACRITY data...\n");
    assert(ALStoreOpenPOSIX(&store, outfilenamebase, "w", USE_LEGACY_FORMAT) == ALErrorNone);
    printf("ALStore open, about to encode and write data...\n");

    ALPartitionData output[num_parts];
    for (uint64_t pnum = 0; pnum < num_parts; pnum++) {
        uint64_t elem_offset = pnum * partsize;
        uint64_t num_elem = (pnum != num_parts - 1) ? partsize : numDoubles - elem_offset;

        assert(ALEncode(&config, (char*)data + elem_offset * sizeof(double), num_elem, &output[pnum]) == ALErrorNone);
        printf("Encoded partition %llu in ALACRITY format\n", pnum);

        assert(ALStoreWritePartition(&store, &output[pnum]) == ALErrorNone);
        printf("Stored partition %llu in ALStore\n", pnum);
    }

    printf("Closing ALStore...\n");
    assert(ALStoreClose(&store) == ALErrorNone);
    printf("Storage of ALACRITY data completed\n");

    free(data); // Don't need raw data anymore

    if (argc == 5) {
        const char *compfilenamebase = argv[4];
        char mymetafilename[256];
        char myindexfilename[256];
        char mydatafilename[256];
        char compmetafilename[256];
        char compindexfilename[256];
        char compdatafilename[256];
        strcpy(mymetafilename, outfilenamebase);
        strcat(mymetafilename, "-metadata.dat");
        strcpy(myindexfilename, outfilenamebase);
        strcat(myindexfilename, "-query_index.dat");
        strcpy(mydatafilename, outfilenamebase);
        strcat(mydatafilename, "-compressed_data.dat");

        strcpy(compmetafilename, compfilenamebase);
        strcat(compmetafilename, "-metadata.dat");
        strcpy(compindexfilename, compfilenamebase);
        strcat(compindexfilename, "-query_index.dat");
        strcpy(compdatafilename, compfilenamebase);
        strcat(compdatafilename, "-compressed_data.dat");
        printf("Reading comparison files %s, %s and %s\n", compmetafilename, compdatafilename, compindexfilename);

        cmp_files(mymetafilename, compmetafilename);
        cmp_files(myindexfilename, myindexfilename);
        cmp_files(mydatafilename, mydatafilename);
    }

    printf("Loading ALACRITY data back through POSIX ALStore interface to compare...\n");
    ALPartitionData comppart;
    ALGlobalMetadata gmeta;

    assert(ALStoreOpenPOSIX(&store, outfilenamebase, "r", USE_LEGACY_FORMAT) == ALErrorNone);
    assert(ALStoreGetGlobalMetadata(&store, &gmeta) == ALErrorNone);
    assert(gmeta.num_partitions == num_parts);

    for (uint64_t pnum = 0; pnum < num_parts; pnum++) {
        assert(ALStoreReadPartition(&store, &comppart) == ALErrorNone);
        printf("Comparing partition number %llu/%llu\n", pnum, num_parts);

        assert(cmp_parts(&output[pnum], &comppart) == 0);
        ALPartitionDataDestroy(&comppart);
    }
    assert(ALStoreClose(&store) == ALErrorNone);
    printf("Successfully recovered data through ALStore read interface\n");

    printf("Using fine-grained ALPartitionStore API to access data...\n");
    ALPartitionStore pstore;
    ALMetadata compmeta;
    ALData compdatabuf = NULL;        // This should cause the Read*Bins functions to allocate a new buffer (it's more convenient this way)
    ALIndex compindexbuf = NULL;    // This should cause the Read*Bins functions to allocate a new buffer (it's more convenient this way)
    int lobin, hibin;
    uint64_t lobin_elemoff, hibin_elemoff;

    assert(ALStoreOpenPOSIX(&store, outfilenamebase, "r", USE_LEGACY_FORMAT) == ALErrorNone);
    assert(ALStoreOpenPartition(&store, &pstore, true) == ALErrorNone);

    printf("Reading metadata...\n");
    assert(ALPartitionStoreReadMetadata(&pstore, &compmeta) == ALErrorNone);
    lobin = compmeta.binLayout.numBins/3; hibin = compmeta.binLayout.numBins*2/3;
    if (lobin == hibin) {
        lobin = 0;
        hibin = compmeta.binLayout.numBins;
    }
    lobin_elemoff = compmeta.binLayout.binStartOffsets[lobin];
    hibin_elemoff = compmeta.binLayout.binStartOffsets[hibin];

    printf("Reading some data bins...\n");
    assert(ALPartitionStoreReadDataBins(&pstore, &compmeta, lobin, hibin, &compdatabuf) == ALErrorNone);
    printf("Reading some index bins...\n");
    assert(ALPartitionStoreReadIndexBins(&pstore, &compmeta, lobin, hibin, &compindexbuf) == ALErrorNone);

    printf("Comparing metadata...\n");
    assert(cmp_metadata(&compmeta, &output[0].metadata) == 0);
    printf("Success!\n");
    printf("Comparing data...\n");
    assert(memcmp(compdatabuf, (char*)output[0].data + lobin_elemoff * (8 - 2), (hibin_elemoff - lobin_elemoff) * (8 - 2)) == 0);
    printf("Success!\n");
    printf("Comparing index...\n");
    uint64_t lobin_indexoff = ALGetIndexBinOffset(&compmeta, lobin);
    uint64_t hibin_indexoff = ALGetIndexBinOffset(&compmeta, hibin);
    if (output[0].metadata.indexMeta.indexForm == ALInvertedIndex)
        assert(memcmp(compindexbuf, (char*)output[0].index + lobin_indexoff, (hibin_indexoff - lobin_indexoff)) == 0);
    else
        printf("Skipping check because it's not an inverted index (TODO: fix this)\n");
    printf("Success!\n");

    assert(ALPartitionStoreClose(&pstore) == ALErrorNone);
    assert(ALStoreClose(&store) == ALErrorNone);

    free(compdatabuf);
    free(compindexbuf);
    for (uint64_t pnum = 0; pnum < num_parts; pnum++)
        ALPartitionDataDestroy(&output[pnum]);

    return 0;
}
