/*
 * uniquery.h
 *
 *  Created on: Sep 14, 2012
 *      Author: David A. Boyuka II
 */

#ifndef ALACRITY_UNIQUERY_H_
#define ALACRITY_UNIQUERY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <alacrity-store.h>

typedef enum {
	VALUE_RETRIEVAL_QUERY_TYPE,
	REGION_RETRIEVAL_INDEX_ONLY_QUERY_TYPE,
	REGION_RETRIEVAL_CANDIDATE_CHECK_QUERY_TYPE,
} ALQueryType;

typedef struct {
    ALStore *store;

    ALGlobalMetadata *gmeta;
    ALMetadata **metadatas;
} ALQueryEngine;

typedef union {
    uint8_t asUint8;
    uint16_t asUint16;
    float asFloat;
    uint32_t asUint32;
    double asDouble;
    uint64_t asUint64;
} value_types_t;

typedef struct {
    ALQueryEngine *qe;

    value_types_t lb;
    value_types_t ub;

    ALQueryType queryType;
} ALUnivariateQuery;

typedef struct {
    uint64_t resultCount;
    rid_t *rids;
    union {
        char *asChar;
        float *asFloat;
        double *asDouble;
    } data;

    void *rids_backing;
    void *data_backing;
} ALUnivariateQueryResult;

void ALQueryEngineInit(ALQueryEngine *qe, ALStore *store, _Bool preload_metadata);
void ALQueryEngineStartUnivariateDoubleQuery(ALQueryEngine *qe, double lval, double uval, ALQueryType queryType, ALUnivariateQuery *uniquery);
void ALQueryEngineStartUnivariateFloatQuery(ALQueryEngine *qe, float lval, float uval, ALQueryType queryType, ALUnivariateQuery *uniquery);
void ALQueryEngineDestroy(ALQueryEngine *qe);

void ALQueryDestroy(ALUnivariateQuery *uniquery);

_Bool ALQueryNextResult(ALUnivariateQuery *uniquery, ALUnivariateQueryResult *result);
void ALQueryResultDestroy(ALUnivariateQueryResult *result);

/*
 * required by multi-variate query
 */
 void ALQueryEnsureMetadataReady(ALQueryEngine *qe);

 const ALMetadata * ALQueryGetPartitionMetadata(ALQueryEngine *qe, ALPartitionStore *ps);

 void ALQueryReadIndex(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t start_bin, bin_id_t end_bin, ALIndex *index);

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* ALACRITY_UNIQUERY_H_ */
