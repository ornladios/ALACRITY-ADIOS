#ifndef ALACRITY_SERIALIZATION_DEBUG_H_
#define ALACRITY_SERIALIZATION_DEBUG_H_

#include "ALUtil.h"

#define ALPrintData(data, n) \
{ \
    for (int i = 0; i < 10; i ++) { \
        printf ("%d ", ((int8_t *) data)[i]); \
    } \
    \
    for (int i = 0; i < 10; i ++) { \
        printf ("%d ", ((int8_t *) data)[n - 1 - i]); \
    } \
    printf ("\n"); \
}

#define ALPrintBinLayout(binLayout) \
{ \
    uint32_t binCount = 0; \
    uint32_t i = 0; \
	uint32_t binValue = 0; \
    printf ("[%s] numBins = %u\n", __FUNCTION__, binLayout.numBins); \
    for (i = 0; i < binLayout.numBins; i ++) { \
		binValue = binLayout.binValues [i]; \
		GET_BIN_COUNT(binLayout, i, binCount); \
        printf ("[%s] binValue %u has count %u elements\n", __FUNCTION__, binValue, binCount); \
    } \
}


#define ALPrintMetaData(metadata) \
{ \
    ALPrintBinLayout(metadata.binLayout) \
    printf ("[%s] partitionLength = %u\n", __FUNCTION__, metadata.partitionLength); \
    printf ("[%s] elementSize = %d\n", __FUNCTION__, metadata.elementSize); \
    printf ("[%s] significantBytes = %d\n", __FUNCTION__, metadata.significantBits); \
    /*printf ("[%s] indexForm = %d\n", __FUNCTION__, metadata.indexForm);*/ \
}

#endif /*ALACRITY_SERIALIZATION_DEBUG_H_*/