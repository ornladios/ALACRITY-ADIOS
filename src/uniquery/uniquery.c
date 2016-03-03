/*
 * uniquery.c
 *
 *  Created on: Sep 14, 2012
 *      Author: David A. Boyuka II
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <alacrity.h>
#include <uniquery.h>
#include <ALUtil.h>

// Internal headers under src/
#include <uniquery/helpers.h>


void ensureMetadataReady(ALQueryEngine *qe);

void ALQueryEngineInit(ALQueryEngine *qe, ALStore *store, _Bool preload_metadata) {
    qe->store = store;

    qe->gmeta = NULL;
    qe->metadatas = NULL;

    if (preload_metadata) {
        ensureMetadataReady(qe);
    }
}

void ALQueryEngineDestroy(ALQueryEngine *qe) {
	qe->store = NULL;
	if (qe->gmeta) {
		if (qe->metadatas) {
			uint64_t i;
			for (i = 0; i < qe->gmeta->num_partitions; i++)
				FREE(qe->metadatas[i]);

			FREE(qe->metadatas);
		}
		FREE(qe->gmeta);
	}
}

 void ensureMetadataReady(ALQueryEngine *qe) {
    if (!qe->gmeta) {
        qe->gmeta = malloc(sizeof(ALGlobalMetadata));
        ALStoreGetGlobalMetadata(qe->store, qe->gmeta);
    }

    if (!qe->metadatas)
        qe->metadatas = (ALMetadata**)calloc(qe->gmeta->num_partitions, sizeof(ALMetadata*));
}

void ALQueryEngineStartUnivariateDoubleQuery(ALQueryEngine *qe, double lval, double uval, ALQueryType queryType, ALUnivariateQuery *uniquery) {
    uniquery->qe = qe;
    uniquery->lb.asDouble = lval;
    uniquery->ub.asDouble = uval;
    uniquery->queryType = queryType;
}

void ALQueryEngineStartUnivariateFloatQuery(ALQueryEngine *qe, float lval, float uval, ALQueryType queryType, ALUnivariateQuery *uniquery) {
    uniquery->qe = qe;
    uniquery->lb.asFloat = lval;
    uniquery->ub.asFloat = uval;
    uniquery->queryType = queryType;
}

void ALQueryDestroy(ALUnivariateQuery *uniquery) {
	uniquery->qe = NULL;
	uniquery->lb.asUint64 = 0;
	uniquery->ub.asUint64 = 0;
}

 const ALMetadata * getPartitionMetadata(ALQueryEngine *qe, ALPartitionStore *ps) {
    const uint64_t partnum = ps->partition_num; // TODO: Accessor function

    ensureMetadataReady(qe);

    // Read the metadata if necessary
    if (!qe->metadatas[partnum]) {
        qe->metadatas[partnum] = malloc(sizeof(ALMetadata));
        ALPartitionStoreReadMetadata(ps, qe->metadatas[partnum]);
    }

    return qe->metadatas[partnum];
}

static void clearQueryResult(ALUnivariateQueryResult *result) {
    result->data.asChar = NULL;
    result->data_backing = NULL;
    result->rids = NULL;
    result->rids_backing = NULL;
    result->resultCount = 0;
}

static void populateQueryResult(ALUnivariateQueryResult *result, ALData data, ALIndex index, uint64_t resultCount) {
	result->data_backing = data;
	result->data.asChar = (char*)data;
	result->rids = (rid_t*)index;
	result->rids_backing = index;
	result->resultCount = resultCount;
}

 void readIndex(ALPartitionStore *ps, const ALMetadata *meta, bin_id_t start_bin, bin_id_t end_bin, ALIndex *index) {
    const ALBinLayout * const bl = &meta->binLayout;

    dbprintf(">>> About to read index bins %hu to %hu (%hu bins, %lu elements)\n", start_bin, end_bin, (end_bin - start_bin), meta->binLayout.binStartOffsets[end_bin] - meta->binLayout.binStartOffsets[start_bin]);
    *index = NULL;
    ALPartitionStoreReadIndexBins(ps, meta, start_bin, end_bin, index);

    if (meta->indexMeta.indexForm == ALCompressedInvertedIndex)
    	ALConvertPartialIndexForm(meta, index, ALInvertedIndex, start_bin, end_bin);
}

_Bool ALQueryNextResult(ALUnivariateQuery *uniquery, ALUnivariateQueryResult *result) {
    ALQueryEngine *qe = uniquery->qe;
    if (ALStoreEOF(qe->store))
        return false;

    ensureMetadataReady(qe);

    ALPartitionStore ps;
    ALStoreOpenPartition(qe->store, &ps, true);

    const ALMetadata * const meta = getPartitionMetadata(qe, &ps);
    const ALBinLayout * const bl = &meta->binLayout;
    bin_id_t start_bin, end_bin;
    uint64_t resultCount;

    // First, find which bins are touched by the query (all elements in the query range will
    // fall into these bins, however not all elements in these bins fall into the query range).
    _Bool are_bins_touched = findBinRange1C(meta, uniquery, &start_bin, &end_bin);

    // If the query range overlaps no bins, skip this partition
    // Else, proceed with reading and processing the results
    if (!are_bins_touched) {
        clearQueryResult(result);
    } else {
    	ALData data;
    	ALIndex index;

    	resultCount = bl->binStartOffsets[end_bin] - bl->binStartOffsets[start_bin];

    	readIndex(&ps, meta, start_bin, end_bin, &index);

    	//printf("touched bin range [%d, %d] \n ", start_bin, end_bin);

    	timer_start("datacandidate");

    	if (uniquery->queryType == REGION_RETRIEVAL_INDEX_ONLY_QUERY_TYPE) {
    		// No data, no candidate checks, just return the results as-is
    		data = NULL;
        	populateQueryResult(result, data, index, resultCount);
    	} else {
    		// Some or all data will be read, with candidate checks either way

    		// Read what data is required
    		if (uniquery->queryType == REGION_RETRIEVAL_CANDIDATE_CHECK_QUERY_TYPE)
            	readAndReconstituteData(&ps, meta, start_bin, end_bin, true, &data); // read end bins only
    		else if (uniquery->queryType == VALUE_RETRIEVAL_QUERY_TYPE)
            	readAndReconstituteData(&ps, meta, start_bin, end_bin, false, &data); // read all data
    		else {
    			eprintf("Invalid query type %d in fucntion %s\n", uniquery->queryType, __FUNCTION__);
    			assert(false); // Bad query type
    		}

    		// Package results
        	populateQueryResult(result, data, index, resultCount);

        	// Candidate checks: trim values in the boundary bins that do not lie in the query range
        	uint64_t trimmed = trimQueryResults(result, meta, uniquery, start_bin, end_bin);

        	// If it's a region retrieval query, we can free the data buffer
        	if (uniquery->queryType == REGION_RETRIEVAL_CANDIDATE_CHECK_QUERY_TYPE) {
        		result->data.asChar = NULL;
        		FREE(result->data_backing);
        	}
    	}

    	timer_stop("datacandidate");
    	//printf("candidate check %f \n", timer_get_total_interval("datacandidate"));
    }

    ALPartitionStoreClose(&ps);

    return true;
}

void ALQueryResultDestroy(ALUnivariateQueryResult *result) {
    result->data.asChar = 0;
    result->rids = 0;
    result->resultCount = 0;

    FREE(result->data_backing);
    FREE(result->rids_backing);
}
