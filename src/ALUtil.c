#include "ALUtil.h"
#include <string.h>
#include <stdlib.h>

uint8_t sigBytesCeil(const ALMetadata *meta) {
    return (meta->significantBits + 0x07) >> 3;
}

uint8_t insigBytesCeil(const ALMetadata *meta) {
    return ((meta->elementSize << 3) - meta->significantBits + 0x07) >> 3;
}

char *to_binary (void *value, uint8_t sz, uint8_t nbits)
{
    int32_t i = 0;
    char *binary_value = (char *) malloc (128);
    uint8_t sbits = sz << 3;

    memset (binary_value, 0, 128);

    if (sz == sizeof (uint64_t)) {
        uint64_t *ptr = (uint64_t *) value;

        for (i = nbits - 1; i >= 0; i --) {
            binary_value [sbits - 1 - i] = ((*ptr >> i) & 1) + '0';
        }

    } else if (sz == sizeof (uint32_t)) {
        uint32_t *ptr = (uint32_t *) value;

        for (i = nbits - 1; i >= 0; i --) {
            binary_value [sbits - 1 - i] = ((*ptr >> i) & 1) + '0';
        }
    }

    return binary_value;
}
