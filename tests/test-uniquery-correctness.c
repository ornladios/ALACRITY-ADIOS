#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>

#include <alacrity.h>
#include <uniquery.h>
#include "../src/include/alacrity-util.h"

const int N = (1<<20);


static void checkRegionResultDouble(ALUnivariateQuery *query, ALUnivariateQueryResult *result, double *dataset, uint64_t keep_mask, uint64_t replace_mask, _Bool exact_bounds) {
    // Check everything in result is in range and matches in dataset
    uint64_t i;

    printf("Checking with %s boundaries...\n", exact_bounds?"exact":"approximate");

    // Ensure every record in the results is within the query range
    for (i = 0; i < result->resultCount; i++) {
        rid_t rid = result->rids[i];

        double realVal = dataset[rid];

        // Compute the upper and lower bounds of the bin within which
        // this value appears, since an approximate region query includes,
        // in its entirety, any bin that intersects the query range at all
        uint64_t maskedRealValBits = ((uint64_t*)dataset)[rid];
        maskedRealValBits &= keep_mask;
        double maskedRealValL = *(double*)&maskedRealValBits;
        maskedRealValBits |= replace_mask;
        double maskedRealValH = *(double*)&maskedRealValBits;

        if (maskedRealValH < maskedRealValL) {
        	double t = maskedRealValH;
        	maskedRealValH = maskedRealValL;
        	maskedRealValL = t;
        }

        if (exact_bounds)
        	assert(realVal >= query->lb.asDouble && realVal < query->ub.asDouble);
        else
        	assert(maskedRealValH >= query->lb.asDouble && maskedRealValL < query->ub.asDouble);
    }

    // Ensure every value in the dataset that's within the query range is in the results
    uint64_t realResultCount = 0;
    for (i = 0; i < N; i++) {
        double realVal = dataset[i];
        uint64_t maskedRealValBits = ((uint64_t*)dataset)[i];
        maskedRealValBits &= keep_mask;
        double maskedRealValL = *(double*)&maskedRealValBits;
        maskedRealValBits |= replace_mask;
        double maskedRealValH = *(double*)&maskedRealValBits;

        if (maskedRealValH < maskedRealValL) {
        	double t = maskedRealValH;
        	maskedRealValH = maskedRealValL;
        	maskedRealValL = t;
        }

        if (exact_bounds)
            realResultCount += (realVal >= query->lb.asDouble && realVal < query->ub.asDouble);
        else
            realResultCount += (maskedRealValH >= query->lb.asDouble && maskedRealValL < query->ub.asDouble);
    }
    assert(realResultCount == result->resultCount);
}

static void checkResultDouble(ALUnivariateQuery *query, ALUnivariateQueryResult *result, double *dataset) {
    // Check everything in result is in range and matches in dataset
    uint64_t i;

    for (i = 0; i < result->resultCount; i++) {
        double val = result->data.asDouble[i];
        rid_t rid = result->rids[i];

        assert(val >= query->lb.asDouble);
        assert(val < query->ub.asDouble);
        assert(dataset[rid] == val);
    }

    // Check that all results were captured using the count
    uint64_t realResultCount = 0;
    for (i = 0; i < N; i++) {
        realResultCount += (dataset[i] >= query->lb.asDouble && dataset[i] < query->ub.asDouble);
    }

    assert(realResultCount == result->resultCount);
}

static void checkResultFloat(ALUnivariateQuery *query, ALUnivariateQueryResult *result, float *dataset) {
    // Check everything in result is in range and matches in dataset
    uint64_t i;

    for (i = 0; i < result->resultCount; i++) {
        float val = result->data.asFloat[i];
        rid_t rid = result->rids[i];

        assert(val >= query->lb.asFloat);
        assert(val < query->ub.asFloat);
        assert(dataset[rid] == val);
    }

    // Check that all results were captured using the count
    uint64_t realResultCount = 0;
    for (i = 0; i < N; i++) {
        realResultCount += (dataset[i] >= query->lb.asFloat && dataset[i] < query->ub.asFloat);
    }

    assert(realResultCount == result->resultCount);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 2) {
        fprintf(stderr, "Usage: %s <tmp dir>\n", argv[0]);
        return 1;
    }

    char dataset1Basename[BUFSIZ];
    char dataset2Basename[BUFSIZ];

    const char *tmpdir = argv[1];
    strcpy(dataset1Basename, tmpdir);
    strcpy(dataset2Basename, tmpdir);
    strcat(dataset1Basename, "/double");
    strcat(dataset2Basename, "/float");

    double *dataset1 = (double*)malloc(N * sizeof(double));
    float *dataset2 = (float*)malloc(N * sizeof(float));
    for (int i = 0; i < N; i++) {
        dataset1[i] = 2 * (double)i / N - 1;
        dataset2[i] = 2 * (float)i / N - 1;
    }

    ALEncoderConfig config;
    ALPartitionData output;
    ALStore store;

    ALEncoderConfigure(&config, 16, DATATYPE_FLOAT64, ALInvertedIndex);
    ALEncode(&config, dataset1, N, &output);
    ALStoreOpenPOSIX(&store, dataset1Basename, "w", false);
    ALStoreWritePartition(&store, &output);
    ALStoreClose(&store);
    ALPartitionDataDestroy(&output);
    printf("Stored the double-prec partition in ALACRITY file format\n");

    ALEncoderConfigure(&config, 16, DATATYPE_FLOAT32, ALInvertedIndex);
    ALEncode(&config, dataset2, N, &output);
    ALStoreOpenPOSIX(&store, dataset2Basename, "w", false);
    ALStoreWritePartition(&store, &output);
    ALStoreClose(&store);
    ALPartitionDataDestroy(&output);
    printf("Stored the single-prec partition in ALACRITY file format\n");

    ALQueryEngine qe;
    ALUnivariateQuery query;
    ALUnivariateQueryResult result;

    printf("Querying -0.5 to 0.5 for double-prec dataset\n");
    ALStoreOpenPOSIX(&store, dataset1Basename, "r", false);
    ALQueryEngineInit(&qe, &store, true);
    ALQueryEngineStartUnivariateDoubleQuery(&qe, -0.5, 0.5, VALUE_RETRIEVAL_QUERY_TYPE, &query);
    ALQueryNextResult(&query, &result);
    checkResultDouble(&query, &result, dataset1);
    ALQueryResultDestroy(&result);
    ALQueryDestroy(&query);
    ALQueryEngineDestroy(&qe);
    ALStoreClose(&store);

    printf("Querying -0.5 to 0.5 for single-prec dataset\n");
    ALStoreOpenPOSIX(&store, dataset2Basename, "r", false);
    ALQueryEngineInit(&qe, &store, true);
    ALQueryEngineStartUnivariateFloatQuery(&qe, -0.5f, 0.5f, VALUE_RETRIEVAL_QUERY_TYPE, &query);
    ALQueryNextResult(&query, &result);
    checkResultFloat(&query, &result, dataset2);
    ALQueryResultDestroy(&result);
    ALQueryDestroy(&query);
    ALQueryEngineDestroy(&qe);
    ALStoreClose(&store);

    printf("Querying -0.4 to 0.4 for double-prec dataset\n");
    ALStoreOpenPOSIX(&store, dataset1Basename, "r", false);
    ALQueryEngineInit(&qe, &store, true);
    ALQueryEngineStartUnivariateDoubleQuery(&qe, -0.4, 0.4, VALUE_RETRIEVAL_QUERY_TYPE, &query);
    ALQueryNextResult(&query, &result);
    checkResultDouble(&query, &result, dataset1);
    ALQueryResultDestroy(&result);
    ALQueryDestroy(&query);
    ALQueryEngineDestroy(&qe);
    ALStoreClose(&store);

    printf("Querying -0.4 to 0.4 for double-prec dataset, region-retrieval, candidate checks\n");
    ALStoreOpenPOSIX(&store, dataset1Basename, "r", false);
    ALQueryEngineInit(&qe, &store, true);
    ALQueryEngineStartUnivariateDoubleQuery(&qe, -0.4, 0.4, REGION_RETRIEVAL_CANDIDATE_CHECK_QUERY_TYPE, &query);
    ALQueryNextResult(&query, &result);
    checkRegionResultDouble(&query, &result, dataset1,
    		                0xFFFF000000000000,
    		                0x0000FFFFFFFFFFFF,
    		                true);
    ALQueryResultDestroy(&result);
    ALQueryDestroy(&query);
    ALQueryEngineDestroy(&qe);
    ALStoreClose(&store);

    printf("Querying -0.4 to 0.4 for double-prec dataset, region-retrieval, no candidate checks\n");
    ALStoreOpenPOSIX(&store, dataset1Basename, "r", false);
    ALQueryEngineInit(&qe, &store, true);
    ALQueryEngineStartUnivariateDoubleQuery(&qe, -0.4, 0.4, REGION_RETRIEVAL_INDEX_ONLY_QUERY_TYPE, &query);
    ALQueryNextResult(&query, &result);
    checkRegionResultDouble(&query, &result, dataset1,
    		                0xFFFF000000000000,
    		                0x0000FFFFFFFFFFFF,
    		                false);
    ALQueryResultDestroy(&result);
    ALQueryDestroy(&query);
    ALQueryEngineDestroy(&qe);
    ALStoreClose(&store);

    free(dataset1);
    free(dataset2);

    return 0;
}
