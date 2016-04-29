#ifndef ALACRITY_UTIL_H_
#define ALACRITY_UTIL_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <alacrity-types.h>

uint8_t alacrity_util_sigBytesCeil(const ALMetadata *meta);
uint8_t alacrity_util_insigBytesCeil(const ALMetadata *meta);
char *alacrity_util_to_binary (void *value, uint8_t sz, uint8_t nbits);

// Checks the endianness of the system by making a 2-byte type, and then
// checking the value of the first (lower-address) byte
static inline ALEndianness alacrity_util_detectEndianness() {
	uint16_t val = 0x1122;
	uint8_t firstbyte = *(uint8_t*)&val;
	return firstbyte == 0x22 ? ENDIAN_LITTLE : ENDIAN_BIG;
}

// Returns the size of member field F of type T. F may be a "path"; that is
// a field of any depth of substructs. Examples:
//   msizeof(ALMetadata, elementSize)
//   msizeof(ALMetadata, indexMeta.indexForm)
#define msizeof(T,F) sizeof(((T*)0)->F)

// Macros to convert integers from 1's complement to 2's complement
// The first two macros assume prior knowledge that val is positive/negative,
// whereas the third performs a test to determine the sign
// NOTE: val must be a signed type!
#define CONV_POS_1C_TO_2C(val, signmask) (val)
#define CONV_NEG_1C_TO_2C(val, signmask) (-((val) ^ (signmask)))
#define CONV_1C_TO_2C(val, signmask) ((val) & (signmask) ? CONV_NEG_1C_TO_2C(val, signmask) : CONV_POS_1C_TO_2C(val, signmask))

#define SPLIT_DATUM_HELPER(datumptr, elem_ctype, sigbytes, highvar, lowvar) {   \
    const elem_ctype _tmp = *(elem_ctype *)(datumptr);                          \
    const char _sigbits = (sigbytes) << 3;                                      \
    const char _insigbits = (sizeof(elem_ctype)<<3) - _sigbits;                 \
    highvar = _tmp >> _insigbits;                                               \
    lowvar = _tmp & ((1ULL << _insigbits) - 1);                                 \
}

#define SPLIT_DATUM(datumptr, elem_size, sigbytes, highvar, lowvar)         \
    switch (elem_size) {                                                    \
        case sizeof(uint16_t):                                              \
        SPLIT_DATUM_HELPER(datumptr, uint16_t, sigbytes, highvar, lowvar)   \
        break;                                                              \
        case sizeof(uint32_t):                                              \
        SPLIT_DATUM_HELPER(datumptr, uint32_t, sigbytes, highvar, lowvar)   \
        break;                                                              \
        case sizeof(uint64_t):                                              \
        SPLIT_DATUM_HELPER(datumptr, uint64_t, sigbytes, highvar, lowvar)   \
        break;                                                              \
    }

#define SPLIT_DATUM_BIT_HELPER(datumptr, elem_ctype, sigbits, highvar, lowvar)  {   \
    const elem_ctype _tmp = *(elem_ctype *)(datumptr);                          \
    const char _sigbits = (sigbits);                                            \
    const char _insigbits = (sizeof(elem_ctype)<<3) - _sigbits;                 \
    highvar = _tmp >> _insigbits;                                               \
    lowvar = _tmp & (~0ULL >> _sigbits);                                        \
}

#define SPLIT_DATUM_BITS(datumptr, elem_size, sigbits, highvar, lowvar)         \
    switch (elem_size) {                                                    \
        case sizeof(uint16_t):                                              \
        SPLIT_DATUM_BIT_HELPER(datumptr, uint16_t, sigbits, highvar, lowvar)    \
        break;                                                              \
        case sizeof(uint32_t):                                              \
        SPLIT_DATUM_BIT_HELPER(datumptr, uint32_t, sigbits, highvar, lowvar)    \
        break;                                                              \
        case sizeof(uint64_t):                                              \
        SPLIT_DATUM_BIT_HELPER(datumptr, uint64_t, sigbits, highvar, lowvar)    \
        break;                                                              \
    }

#define REJOIN_DATUM_BITS_HELPER(datumptr, elem_ctype, sigbits, highvar, lowvar) {   \
    elem_ctype *_tmp = (elem_ctype *)(datumptr);                                     \
    const char _insigbits = (sizeof(elem_ctype)<<3) - (sigbits);                     \
    *_tmp = ((uint64_t)(highvar)) << _insigbits;                                     \
    *_tmp |= ((uint64_t)(lowvar)) & (~0ULL >> (sigbits));                            \
}

#define REJOIN_DATUM_BITS(datumptr, elem_size, sigbits, highvar, lowvar)        \
    switch (elem_size) {                                                        \
        case sizeof(uint16_t):                                                  \
        REJOIN_DATUM_BITS_HELPER(datumptr, uint16_t, sigbits, highvar, lowvar)  \
        break;                                                                  \
        case sizeof(uint32_t):                                                  \
        REJOIN_DATUM_BITS_HELPER(datumptr, uint32_t, sigbits, highvar, lowvar)  \
        break;                                                                  \
        case sizeof(uint64_t):                                                  \
        REJOIN_DATUM_BITS_HELPER(datumptr, uint64_t, sigbits, highvar, lowvar)  \
        break;                                                                  \
    }

#define REJOIN_DATUM(datumptr, elem_size, sigbytes, highvar, lowvar)        \
        REJOIN_DATUM_BITS(datumptr, elem_size, (sigbytes << 3), highvar, lowvar)

#define VARLEN_TYPE_TO_MEM(invar, outptr, size)     \
    memcpy(outptr, &invar, size)

// Example usage:
//   uint64_t cur_value;
//   void *input_buf = ...;
//   char elemsize = 3; // 3 bytes per element
//   void *end_ptr = input_buf + numelements*elemsize;
// ITERATE_OVER_BUFFER(cur_value, input_buf, end_ptr, elemsize,
//   // ... do something with cur_value, it is your iterator variable ...
//   // Don't forget backslashes at each line end (line continuations)
// )
#define ITERATE_OVER_BUFFER(val, buf, endptr, elemsize, CODE) \
        { \
        void *_ptr;                 \
        uint64_t _mask = ((1ULL << ((elemsize)<<3)) - 1); \
        for (_ptr = (buf); _ptr <= (endptr)-sizeof(uint64_t); _ptr += (elemsize)) { \
            val = *(uint64_t*)_ptr; \
            val &= _mask;           \
            CODE                    \
        }                           \
        uint64_t _lastbytes = *(uint64_t*)((endptr) - sizeof(uint64_t));    \
        _lastbytes >>= ((sizeof(uint64_t) - ((endptr) - _ptr))<<3);         /* Shift down to get rid of bytes we've already read */ \
        do {                                \
            val = _lastbytes & _mask;       \
            CODE                            \
            _lastbytes >>= ((elemsize)<<3); \
            _ptr += (elemsize);             \
        } while (_ptr < (endptr));          \
        }

// Usage: iterates over all possible bin values 0...(1<<bits)-1, but does so in increasing value
//        order according to one's complement (as is used by floats and dobules).
//        "var" will be the iterator variable in the body of "code", which will be executed once per
//        value.
#define FOR_BIN_VALUE_IN_1C_ORDER(var, bits, code) {          \
    const high_order_bytes_t _signbit = 1ULL << ((bits) - 1); \
    const high_order_bytes_t _maxval = _signbit - 1;          \
    const high_order_bytes_t _minval = _maxval + _signbit;    \
    high_order_bytes_t var;                                   \
    for (var = _minval; var >= _signbit; var--) { code };     \
    for (var = 0; var <= _maxval; var++) { code };            \
}

#ifdef BGP
    #define GET_BUFFER_ELEMENT(val, ptr, elemsize) { \
        val = 0;                                     \
        memcpy(((uint8_t*)&(val)) + (sizeof(val) - (elemsize)), (ptr), (elemsize)); \
    }
#else
    #define GET_BUFFER_ELEMENT(val, ptr, elemsize) { \
        val = 0;                                     \
        memcpy ((uint8_t *) &(val), (uint8_t *)(ptr), (elemsize)); \
    }
#endif

#define GET_BUFFER_ARRAY(src_ptr, dest_ptr, num, elemsize) \
    memcpy ((src_ptr), (dest_ptr), (num) * (elemsize));

#define GET_ITH_BUFFER_ELEMENT(val, baseptr, i, elemsize) \
        GET_BUFFER_ELEMENT(val, (uint8_t *)(baseptr) + (i) * (elemsize), (elemsize))

// Sriram is replacing the getbuffer with malloc
#ifdef BGP
    #define SET_BUFFER_ELEMENT(ptr, val, elemsize) \
        memcpy((ptr), ((uint8_t*)&(val)) + (sizeof(val) - (elemsize)), (elemsize));
#else
    #define SET_BUFFER_ELEMENT(ptr, val, elemsize) \
        memcpy ((uint8_t *) (ptr), (uint8_t *) &(val), (elemsize));
#endif

#define SET_ITH_BUFFER_ELEMENT(baseptr, i, val, elemsize) \
        SET_BUFFER_ELEMENT((uint8_t *)(baseptr) + (uint64_t)(i) * (elemsize), val, (elemsize))

#define SET_BUFFER_ARRAY(src_ptr, dest_ptr, num, elemsize) \
    memcpy ((src_ptr), (dest_ptr), (uint64_t)(num) * (elemsize));

#define GET_BIN_COUNT(binLayout, binID, binCount) \
    (binCount) = ((binLayout).binStartOffsets [binID + 1] - (binLayout).binStartOffsets [binID]);

#define SWAP_2(x) ( (((x) & 0xFF) << 8) | \
                    ((x) >> 8) )

#define SWAP_4(x) ( ((x) << 24) | \
                    (((x) << 8) & 0x00FF0000) | \
                    (((x) >> 8) & 0x0000FF00) | \
                    ((x) >> 24) )

#define SWAP_8(x) ( (((x) & 0xFF00000000000000ull) >> 56) | \
                    (((x) & 0x00FF000000000000ull) >> 40) | \
                    (((x) & 0x0000FF0000000000ull) >> 24) | \
                    (((x) & 0x000000FF00000000ull) >> 8 ) | \
                    (((x) & 0x00000000FF000000ull) << 8 ) | \
                    (((x) & 0x0000000000FF0000ull) << 24) | \
                    (((x) & 0x000000000000FF00ull) << 40) | \
                    (((x) & 0x00000000000000FFull) << 56))


// Safe free
#define FREE(x) if (x) {free((void*)(x)); (x) = NULL;}

// Debugging commands

#ifdef DEBUG
#define dbprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define dbprintf(...) (void)0
#endif

#ifdef DEBUG
#define PRINTWARNING
#endif

#ifdef PRINTWARNING
#define wprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define wprintf(...) (void)0
#endif

#ifdef PRINTWARNING
#define PRINTERROR
#endif

#ifdef PRINTERROR
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define eprintf(...) (void)0
#endif

#endif
