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
#include <ALUtil.h>

#include <sys/time.h>
double dclock1(void) {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (double) tv.tv_sec + (double) tv.tv_usec * 1e-6;
}

const _Bool USE_LEGACY_FORMAT = false;

int main(int argc, char **argv) {
	 if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <input path & base name>  <low val> <high val> [<use CII? default false>]\n", argv[0]);
        return 1;
    }
    int i = 1;
    const char *infilename = argv[i++];
//    const char *outfilenamebase = argv[2];
    double lval = atof(argv[i++]);
    double uval = atof(argv[i++]);
    _Bool useCII = argc >= 5 ? atoi(argv[i++]) > 0 : false;

    ALStore store;
    ALStoreOpenPOSIX(&store,infilename , "r", USE_LEGACY_FORMAT);

    ALGlobalMetadata gmeta;

    ALStoreGetGlobalMetadata(&store, &gmeta);

//    printf("number of partition %llu \n", gmeta.num_partitions);

    ALQueryEngine qe;
    ALQueryEngineInit(&qe, &store, true);

    printf("Performing query for values in range %lf to %lf...\n", lval, uval);

    ALUnivariateQuery uniquery;
    ALUnivariateQueryResult result;
    double s_time , e_time;
    ALQueryEngineStartUnivariateDoubleQuery(&qe, lval, uval, VALUE_RETRIEVAL_QUERY_TYPE, &uniquery);
    s_time = dclock1();
    while (ALQueryNextResult(&uniquery, &result)) {
//        printf("Read %llu results\n", result.resultCount);
        ALQueryResultDestroy(&result);
    }
	e_time = dclock1();
    ALStoreClose(&store);
	printf("Uniquery time performance %f \n ", e_time - s_time);
    return 0;
}
