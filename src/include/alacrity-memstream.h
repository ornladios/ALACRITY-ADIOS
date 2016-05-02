#ifndef ALACRITY_MEMSTREAM_H_
#define ALACRITY_MEMSTREAM_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    void *buf;
    void *ptr;
} memstream_t;

memstream_t memstreamInitReturn(void *buf);
void memstreamInit(memstream_t *ms, void *buf);
void memstreamReset(memstream_t *ms);

void memstreamAppend(memstream_t *ms, const void *data, uint64_t len);
void memstreamAppendArray(memstream_t *ms, const void *data, uint64_t size, uint64_t count);
void memstreamAppendChar(memstream_t *ms, char datum);
void memstreamAppendInt(memstream_t *ms, int datum);
void memstreamAppendUint16(memstream_t *ms, uint16_t datum);
void memstreamAppendInt64(memstream_t *ms, int64_t datum);
void memstreamAppendUint64(memstream_t *ms, uint64_t datum);

void memstreamRead(memstream_t *ms, void *data, uint64_t len);
void memstreamReadArray(memstream_t *ms, void *data, uint64_t size, uint64_t count);
char memstreamReadChar(memstream_t *ms);
int memstreamReadInt(memstream_t *ms);
uint16_t memstreamReadUint16(memstream_t *ms);
int64_t memstreamReadInt64(memstream_t *ms);
uint64_t memstreamReadUint64(memstream_t *ms);

// Note to users: you may use this function to generate custom, efficient
// append/read functions for various typedefs
// Example:
//   MAKE_APPEND_AND_READ_FUNCS(MyType, my_type_t)
//   ...
//   my_type_t val = ...;
//   memstreamAppendMyType(msPtr, val);
//   val =memstreamReadMyType(msPtr);
//
#define MAKE_STATIC_APPEND_AND_READ_FUNCS(SUFFIX, TYPE) 																					\
    static void memstreamAppend##SUFFIX(memstream_t *ms, TYPE datum) { *(TYPE*)ms->ptr = datum; ms->ptr = (TYPE*)ms->ptr + 1; }			\
    static TYPE memstreamRead##SUFFIX(memstream_t *ms) { const TYPE ret = *(TYPE*)ms->ptr; ms->ptr = (TYPE*)ms->ptr + 1; return ret; }

void memstreamSkip(memstream_t *ms, uint64_t len);
void memstreamFill(memstream_t *ms, char val, uint64_t num);


uint64_t memstreamGetPosition(const memstream_t *ms);

void memstreamDestroy(memstream_t *ms, _Bool freeBuf);

#endif
