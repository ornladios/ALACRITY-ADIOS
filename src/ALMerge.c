#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <alacrity.h>
#include "include/alacrity-util.h"

// TODO: relativize RIDs to some base RID per partition

const int PART1 = 0;
const int PART2 = 1;
const int MERGEDPART = 2;

ALError ALMerge(const ALPartitionData *part1, const ALPartitionData *part2, ALPartitionData *partMerged) {
    const ALMetadata *meta1 = &part1->metadata;
    const ALMetadata *meta2 = &part2->metadata;
    ALMetadata *metaMerged = &partMerged->metadata;

    const ALBinLayout *binlayout1 = &meta1->binLayout;
    const ALBinLayout *binlayout2 = &meta2->binLayout;
    ALBinLayout *binlayoutMerged = &metaMerged->binLayout;

    const int numBins1 = binlayout1->numBins;
    const int numBins2 = binlayout2->numBins;

    const int elemsize = meta1->elementSize;
    const int sigbits = meta1->significantBits;
    const int insigbits = elemsize - sigbits;
    const int sigbytes = (sigbits + 0x07) >> 3;
    const int insigbytes = (insigbits + 0x07) >> 3;
    const ALIndexForm indexForm = meta1->indexMeta.indexForm;
    const partition_length_t partlen1 = meta1->partitionLength;
    const partition_length_t partlen2 = meta2->partitionLength;
    const partition_length_t partlenMerged = partlen1 + partlen2;

    // First set the easy fields
    metaMerged->elementSize = elemsize;
    metaMerged->significantBits = sigbits;
    metaMerged->indexMeta.indexForm = indexForm;
    metaMerged->partitionLength = partlenMerged;

    // Next allocate the necessary buffers
    binlayoutMerged->binValues = malloc((numBins1 + numBins2) * sizeof (bin_offset_t)); // Worst-case size
    binlayoutMerged->binStartOffsets = malloc((numBins1 + numBins2) * sizeof(bin_offset_t)); // Worst-case size
    partMerged->data = malloc(partlenMerged * insigbytes);
    partMerged->index = malloc(partlenMerged * (indexForm == ALInvertedIndex ? sizeof(rid_t) : sigbytes));

    // Now iterate over the bin layouts and merge them, along with the data and index

    // Each of these arrays is indexed using constants PART1, PART2, MERGEDPART
    // One-per-bin variables
    int curBin[3] = { 0, 0, 0 };
    char *binValuePtr[3] = { binlayout1->binValues, binlayout2->binValues, binlayoutMerged->binValues };
    bin_offset_t *binEndOffPtr[3] = { binlayout1->binStartOffsets, binlayout2->binStartOffsets, binlayoutMerged->binStartOffsets };
    // Location variables
    bin_offset_t curBinOffset[3] = { 0, 0, 0 };
    char *dataPtr[3] = { part1->data, part2->data, partMerged->data };
    char *indexPtr[3] = { part1->index, part2->index, partMerged->index };    // Only used for inverted index...

    high_order_bytes_t nextBinVals[2]; // To hold the next bin value for each of the two bin layouts, for list-merge comparison
    int srcPart;    // Which partition's bin should we append next? Possible values are PART1, PART2
    _Bool doBothParts;

    while (curBin[PART1] < numBins1 || curBin[PART2] < numBins2) {
        // Get the value of the next bin for each
        if (curBin[PART1] < numBins1) nextBinVals[PART1] = binValuePtr[PART1][curBin[PART1]];
        if (curBin[PART2] < numBins2) nextBinVals[PART2] = binValuePtr[PART2][curBin[PART2]];

        // Choose which partition has the bin to append next

        // If either PART2's bins are gone, or PART1 has at least one bin, and it's less than PART2's next bin
        if (curBin[PART2] == numBins2 ||
            (curBin[PART1] < numBins1 && nextBinVals[PART1] < nextBinVals[PART2])) {
            srcPart = PART1;
            doBothParts = false;

        // Else, if either PART1's bins are gone, or PART2's next bin is less than PART1's next bin
        // (we know PART2 has at least one bin from the previous conditional)
        } else if (curBin[PART1] == numBins1 || nextBinVals[PART1] > nextBinVals[PART2]) {

            srcPart = PART2;
            doBothParts = false;

        // Else, there must be at least one bin left in each partition, and both must be equal
        } else {
            srcPart = PART1;
            doBothParts = true;
        }

        // Now srcPart is the ID of the partition to append next, and if doBothParts
        // is true, the other one should be appended after it to the same bin in the merged partition

        // Now append the given bin to the new partition

        // Copy info from source partition (srcPart), and then from the
        // other, too, (if doBothParts is true)
        while (true) {
            // Get some info from the source partition's bin
            const int srcPartCurBin = curBin[srcPart];
            const int srcPartCurOffset = curBinOffset[srcPart];
            const int srcPartEndOffset = *binEndOffPtr[srcPart];

            // Calculate the size of the bin to append
            const bin_offset_t srcPartBinSize = srcPartEndOffset - srcPartCurOffset;
            const uint64_t srcPartBinDataSize = srcPartBinSize * insigbytes;
            const uint64_t srcPartBinIndexSize = srcPartBinSize * sizeof(rid_t);

            printf("Appending part %d/bin %lu, value %lu, size %lu as bin %lu at offset %lu\n", srcPart, srcPartCurBin, nextBinVals[srcPart], srcPartBinSize, curBin[MERGEDPART], curBinOffset[MERGEDPART]);

            // Append the bin (offset, data, index)
            memcpy(dataPtr[MERGEDPART], dataPtr[srcPart], srcPartBinDataSize);
            if (indexForm == ALInvertedIndex)
                memcpy(indexPtr[MERGEDPART], indexPtr[srcPart], srcPartBinIndexSize);

            // Increment source/dest location pointers/counters
            curBinOffset[srcPart] += srcPartBinSize;
            curBinOffset[MERGEDPART] += srcPartBinSize;
            dataPtr[srcPart] += srcPartBinDataSize;
            dataPtr[MERGEDPART] += srcPartBinDataSize;
            if (indexForm == ALInvertedIndex) {
                indexPtr[srcPart] += srcPartBinIndexSize;
                indexPtr[MERGEDPART] += srcPartBinIndexSize;
            }

            // Increment source bin counters
            curBin[srcPart]++;
            binValuePtr[srcPart] += sigbytes;
            binEndOffPtr[srcPart]++;

            // If that was the only partition bin to append, stop
            // Else, do the other partition bin next
            if (!doBothParts) break;
            else {
                doBothParts = false;
                srcPart = 1 - srcPart;
            }
        }

        // Copy the one-per-bin items (bin value, end offset)
        binValuePtr[MERGEDPART][curBin[MERGEDPART]] = nextBinVals[srcPart]; // Just copy the bin value
        *binEndOffPtr[MERGEDPART] = curBinOffset[MERGEDPART]; // Copy in the current offset (this works whether or not two bins were just merged

        // Increment all one-per-bin pointers/counters
        binValuePtr[MERGEDPART] ++;
        binEndOffPtr[MERGEDPART]++;
        curBin[MERGEDPART]++;
    }

    binlayoutMerged->numBins = curBin[MERGEDPART];    // Finally, copy out this value into the actual partition
                                                    // (the rest of the stuff was set directly by the above loop)

    // If the index is instead in compressed form, we simply concatenate them
    if (indexForm == ALCompressionIndex) {
        memcpy((char*)partMerged->index, part1->index, partlen1 * sigbytes);
        memcpy((char*)partMerged->index + partlen1 * sigbytes, part2->index, partlen2 * sigbytes);
    }

    return ALErrorNone;
}
