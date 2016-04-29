#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>

#include <alacrity.h>
#include "../src/include/alacrity-util.h"

int ALBinLayout_is_equal (ALBinLayout leftBinLayout, ALBinLayout rightBinLayout, uint8_t significantBits)
{
    const char sigbytes = (significantBits + 0x07) >> 3;
    assert (leftBinLayout.numBins == rightBinLayout.numBins);
    assert (memcmp (leftBinLayout.binValues, rightBinLayout.binValues, leftBinLayout.numBins * sizeof (bin_offset_t)) == 0);
    assert (memcmp (leftBinLayout.binStartOffsets, rightBinLayout.binStartOffsets, (leftBinLayout.numBins + 1) * sizeof (bin_id_t)) == 0);

    return 0;
}

int ALMetadata_is_equal (ALMetadata leftMetadata, ALMetadata rightMetadata)
{
    assert (leftMetadata.partitionLength == rightMetadata.partitionLength);
    assert (leftMetadata.indexMeta.indexForm == rightMetadata.indexMeta.indexForm);
    assert (leftMetadata.significantBits == rightMetadata.significantBits);
    assert (leftMetadata.elementSize == rightMetadata.elementSize);

    assert (ALBinLayout_is_equal (leftMetadata.binLayout, rightMetadata.binLayout, leftMetadata.significantBits) == 0);

    return 0;
}

int ALPartitionData_is_equal (ALPartitionData leftPartitionData, ALPartitionData rightPartitionData)
{
    const char sigbits = leftPartitionData.metadata.significantBits;
    const char insigbits = (leftPartitionData.metadata.elementSize << 3) - leftPartitionData.metadata.significantBits;
    const char sigbytes = (sigbits + 0x07) >> 3;
    const char insigbytes = (insigbits + 0x07) >> 3;
    ALMetadata leftMetadata = leftPartitionData.metadata;
    ALMetadata rightMetadata = rightPartitionData.metadata;

    assert (ALMetadata_is_equal (leftMetadata, rightMetadata) == 0);
    assert (memcmp (leftPartitionData.index, rightPartitionData.index, ALGetIndexSize(&leftPartitionData.index, &leftMetadata)) == 0);
    assert (memcmp (leftPartitionData.data, rightPartitionData.data, ALGetDataSize(&leftPartitionData.data, &leftMetadata)) == 0);

    return 1;
}

void dup_part_data(ALPartitionData *in, ALPartitionData *out) {
    // Start a new memstream of the appropriate size
    uint64_t outputSize = ALGetPartitionDataSize (in);
    memstream_t ms;
    memstreamInit(&ms, malloc(outputSize));

    // Serialize into the memstream
    ALSerializePartitionData (in, &ms);

    // Reset the memstream and deserialize from it
    memstreamReset(&ms);
    ALDeserializePartitionData(out, &ms);
    memstreamDestroy(&ms, true);
}

int main (int argc, char *argv [])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input filename> <output filename base>\n", argv[0]);
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
    char outindexfilename[256];
    char outdatafilename[256];
    strcpy(outindexfilename, outfilenamebase);
    strcat(outindexfilename, "-index.dat");
    strcpy(outdatafilename, outfilenamebase);
    strcat(outdatafilename, "-data.dat");

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
    ALEncoderConfigure(&config, 16, DATATYPE_FLOAT64, ALCompressedInvertedIndex);

    ALPartitionData output;
    ALEncode(&config, data, numDoubles, &output);

    printf("Encoded the data with an inverted index\n");

    ALPartitionData output_ii;
    dup_part_data(&output, &output_ii);

    ALConvertIndexForm(&output.metadata, &output.index, ALCompressedInvertedIndex);
    printf("Compressed inverted index\n");

    const uint8_t sigbtyes = alacrity_util_sigBytesCeil(&output.metadata);
    const uint8_t insigbytes = alacrity_util_insigBytesCeil(&output.metadata);
    int wcount = fwrite(output.index, 1, ALGetIndexSize(&output.index, &output.metadata), outindexfile);
    fprintf(stderr, "Wrote %d index bytes\n", wcount);
    if (wcount != ALGetIndexSize(&output.index, &output.metadata)) {
        fprintf(stderr, "Expected write %d index bytes, wrote %d\n", ALGetIndexSize(&output.index, &output.metadata), wcount);
        return 1;
    }

    wcount = fwrite(output.data, insigbytes, numDoubles, outdatafile);
    if (wcount != numDoubles) {
        fprintf(stderr, "Expected write %d data elements, wrote %d\n", numDoubles, wcount);
        return 1;
    }

    // Start a new memstream of the appropriate size
    uint64_t outputSize = ALGetPartitionDataSize (&output);
    memstream_t ms;
    memstreamInit(&ms, malloc(outputSize));

    // Serialize into the memstream
    ALSerializePartitionData (&output, &ms);
    printf("\n");

    // Reset the memstream and deserialize from it
    memstreamReset(&ms);
    ALPartitionData output_new;
    ALDeserializePartitionData(&output_new, &ms);
    printf("\n");

    // Finally, destroy the memstream, deallocating the buffer
    memstreamDestroy(&ms, true);

    printf ("----------------------------------------------------------------\n");
    printf ("[%s] outputSize = %llu\n", __FUNCTION__, outputSize);
    printf ("[%s] ALGetPartitionDataSize () = %llu\n", __FUNCTION__, ALGetPartitionDataSize (&output));
    printf ("[%s] ALGetPartitionDataSize () = %llu\n", __FUNCTION__, ALGetPartitionDataSize (&output_new));
    assert (outputSize == ALGetPartitionDataSize (&output_new));
    printf ("----------------------------------------------------------------\n");

    printf ("\n");
    ALPartitionData_is_equal (output, output_new);

    printf ("\n");
    printf ("Serialized+deserialized ALPartitionData MATCHES the original\n");

    printf ("Testing decoding of compressed inverted index...\n");
    ALConvertIndexForm(&output.metadata, &output.index, ALInvertedIndex);
    ALPartitionData_is_equal (output, output_ii);

    printf ("Decompressed inverted index MATCHES the original\n");
    printf ("SUCCESS\n");

    free (output_new.metadata.binLayout.binValues);
    free (output_new.metadata.binLayout.binStartOffsets);
    free (output_new.data);
    free (output_new.index);

    free (output.metadata.binLayout.binValues);
    free (output.metadata.binLayout.binStartOffsets);
    free (output.data);
    free (output.index);

    fclose(infile);
    fclose(outindexfile);
    fclose(outdatafile);

    return 0;
}
