#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <alacrity.h>
#include <ALUtil.h>

// TODO: relativize RIDs to some base RID per partition

static void makeDummyPartition(    ALPartitionData *part, partition_length_t len, int sigbytes, int elemsize, ALIndexForm indexForm,
                                bin_id_t numBins, high_order_bytes_t *binValues, bin_offset_t *binLens, low_order_bytes_t low_value, rid_t baseRID) {
    ALMetadata *meta = &part->metadata;

    meta->partitionLength = len;
    meta->elementSize = elemsize;
    meta->significantBits = sigbytes;
    meta->indexMeta.indexForm = indexForm;
    const int insigbytes = elemsize - sigbytes;

    meta->binLayout.numBins = numBins;
    meta->binLayout.binValues = malloc(numBins * sizeof(bin_offset_t));
    meta->binLayout.binStartOffsets = malloc(numBins * sizeof(bin_offset_t)); 

    part->data = malloc(len * insigbytes);
    part->index = (indexForm == ALInvertedIndex ? malloc(len * sizeof(rid_t)) : malloc(len * sigbytes));

    bin_offset_t curOffset = 0;
    for (bin_id_t binID = 0; binID < numBins; binID++) {
        const bin_offset_t len = binLens[binID];
        meta->binLayout.binValues [binID] = binValues[binID];

        for (int i = 0; i < len; i++) {
            SET_ITH_BUFFER_ELEMENT(part->data, curOffset + i, low_value, insigbytes);
            rid_t rid = baseRID + curOffset + i;
            if (indexForm == ALInvertedIndex) {
                SET_ITH_BUFFER_ELEMENT(part->index, curOffset + i, rid, sizeof(rid_t));
            } else {
                ; // TODO: do this
            }
        }

        curOffset += len;
        meta->binLayout.binStartOffsets[binID] = curOffset;
    }
}

void printPartition(ALPartitionData *part) {
    const ALMetadata *meta = &part->metadata;
    const ALBinLayout *bl = &meta->binLayout;

    printf("%lu elements with sig/insig bytes %d/%d, index form %s\n", meta->partitionLength, meta->significantBits, meta->elementSize - meta->significantBits, meta->indexMeta.indexForm == ALInvertedIndex ? "INVERTED" : "COMPRESSION");
    printf("Num bins: %lu\n", bl->numBins);

    high_order_bytes_t binval;
    bin_offset_t binlen, lastoff = 0;
    low_order_bytes_t dataval;
    rid_t indexval;
    for (bin_id_t i = 0; i < bl->numBins; i++) {
        binval = bl->binValues [i];
        binlen = bl->binStartOffsets[i] - lastoff;
        printf("Bin %lu, value %lu, size %lu\n", i, binval, binlen);

        for (int j = 0; j < binlen; j++) {
            GET_ITH_BUFFER_ELEMENT(dataval, part->data, lastoff + j, meta->elementSize - meta->significantBits);
            GET_ITH_BUFFER_ELEMENT(indexval, part->index, lastoff + j, sizeof(rid_t));

            printf("  (%lu, %llu)\n", indexval, dataval);
        }

        lastoff += binlen;
    }
}

int main(int argc, char **argv) {
    ALPartitionData p1, p2;

    high_order_bytes_t binVals1[] = {1,2,4,5};
    high_order_bytes_t binVals2[] = {2,3,4,5};
    high_order_bytes_t binLens1[] = {3,4,2,1};
    high_order_bytes_t binLens2[] = {1,1,2,6};
    makeDummyPartition(&p1, 10, 2, 8, ALInvertedIndex, 4, binVals1, binLens1, 12345, 0);
    makeDummyPartition(&p2, 10, 2, 8, ALInvertedIndex, 4, binVals2, binLens2, 54321, 10);

    printPartition(&p1);
    printf("----\n");
    printPartition(&p2);
    printf("----\n");

    ALPartitionData pm;
    ALMerge(&p1, &p2, &pm);

    printPartition(&pm);
}

