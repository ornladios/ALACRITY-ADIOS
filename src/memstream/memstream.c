#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../include/alacrity-memstream.h"

// TODO: Add conditionally-compiled bounds checking

memstream_t memstreamInitReturn(void *buf) {
    return (memstream_t){ buf, buf };
}

void memstreamInit(memstream_t *ms, void *buf) {
    ms->buf = ms->ptr = buf;
}

void memstreamReset(memstream_t *ms) {
    ms->ptr = ms->buf;
}



void memstreamAppend(memstream_t *ms, const void *data, uint64_t len) {
    memcpy(ms->ptr, data, len);
    ms->ptr = (char*)ms->ptr + len;
}

void memstreamAppendArray(memstream_t *ms, const void *data, uint64_t size, uint64_t count) {
    memstreamAppend(ms, data, size * count);
}

void memstreamRead(memstream_t *ms, void *data, uint64_t len) {
    memcpy(data, ms->ptr, len);
    ms->ptr = (char*)ms->ptr + len;
}

void memstreamReadArray(memstream_t *ms, void *data, uint64_t size, uint64_t count) {
    memstreamRead(ms, data, size * count);
}

#define MAKE_APPEND_AND_READ_FUNCS(SUFFIX, TYPE) 																					\
    void memstreamAppend##SUFFIX(memstream_t *ms, TYPE datum) { *(TYPE*)ms->ptr = datum; ms->ptr = (TYPE*)ms->ptr + 1; }			\
    TYPE memstreamRead##SUFFIX(memstream_t *ms) { const TYPE ret = *(TYPE*)ms->ptr; ms->ptr = (TYPE*)ms->ptr + 1; return ret; }

MAKE_APPEND_AND_READ_FUNCS(Char, char)
MAKE_APPEND_AND_READ_FUNCS(Int, int)
MAKE_APPEND_AND_READ_FUNCS(Uint16, uint16_t)
MAKE_APPEND_AND_READ_FUNCS(Int64, int64_t)
MAKE_APPEND_AND_READ_FUNCS(Uint64, uint64_t)

void memstreamSkip(memstream_t *ms, uint64_t len) {
    ms->ptr = (char*)ms->ptr + len;
}

void memstreamFill(memstream_t *ms, char val, uint64_t num) {
    memset(ms->ptr, val, num);
    ms->ptr = (char*)ms->ptr + num;
}

uint64_t memstreamGetPosition(const memstream_t *ms) {
    return (char*)ms->ptr - (char*)ms->buf;
}

void memstreamDestroy(memstream_t *ms, _Bool freeBuf) {
    if (freeBuf) free(ms->buf);
    ms->buf = ms->ptr = NULL;
}
