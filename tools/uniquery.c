/*
 * uniquery.c
 *
 *  Created on: Nov 14, 2012
 *      Author: David A. Boyuka II
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>

#include <alacrity.h>
#include <trycatch.h>
#include <timer.h>
#include "../src/include/alacrity-util.h"

struct {
	_Bool legacyFormat;
}static OPTIONS;

char *cmdstr;
void usage_and_exit() {
	fprintf(stderr, "Usage: uniquery [OPTIONS] FILEBASE QUERY_LB QUERY_UB [# OF RUNS]\n");
	exit(1);
}

void init_options() {
	OPTIONS.legacyFormat = false;
}

void parse_options(int *argc, char ***argv) {
	init_options();
	//*argc -= optind;
	//*argv += optind;
}

int doQuery(const char *filebase, double lb, double ub);

#define SHIFTN(n) { argc -= (n); argv += (n); }
#define SHIFT SHIFTN(1)

int main(int argc, char **argv) {
	cmdstr = argv[0];
	SHIFT

	parse_options(&argc, &argv);

	// Make sure there's at least one argument for the command, then capture
	// it and advance past it
	if (argc < 3){
		usage_and_exit();
		return 1;
	}

	const char *filebase = argv[0];
	double lb = atof(argv[1]);
	double ub = atof(argv[2]);

	/********  added by chris begins *********/
	int runs = 1;
	if (argc > 3 ){
		runs = atoi(argv[3]);
	}
	ALTimer_init();

	doQuery(filebase, lb, ub);

	/*int k = 0;
	for (; k < runs; k++) {
		ALTimer_start("uniquery");
		doQuery(filebase, lb, ub);
		ALTimer_stop("uniquery");
	}
	double t = ALTimer_get_total_interval("uniquery");
	printf("uniquery avg. time %f \n", t/runs);*/

	/******** added by chris ends *********/

	//ALTimer_print_timers_short();
	// At the compiler's complaint...
	return 0;
}

int doQuery(const char *filebase, double lb, double ub) {
	ALStore store;
	ALGlobalMetadata gmeta;
	ALQueryEngine qe;
	ALUnivariateQuery query;
	ALUnivariateQueryResult result;

ALTimer_start("totalqueryprocess");

	TRY(tc1)
	{
		ALError err = ALStoreOpenPOSIX(&store, filebase, "r",
				OPTIONS.legacyFormat);
		if (err != ALErrorNone) {
			THROW(tc1, 1,
					"Could not open ALACRITY store %s for reading", filebase);
		}
        if (ALStoreGetGlobalMetadata(&store, &gmeta) != ALErrorNone) {
            THROW(tc1, 2,
                  "Could not read global metadata from ALACRITY store %s", filebase);
        }
		ALQueryEngineInit(&qe, &store, true);
		ALQueryEngineStartUnivariateDoubleQuery(&qe, lb, ub, VALUE_RETRIEVAL_QUERY_TYPE, &query);
		//This modification is to measure the cost of data candidate check
		//ALQueryEngineStartUnivariateDoubleQuery(&qe, lb, ub, REGION_RETRIEVAL_CANDIDATE_CHECK_QUERY_TYPE, &query);
		uint64_t totalResultCount = 0;
		while (ALQueryNextResult(&query, &result)) {
		totalResultCount += result.resultCount;
        	if(result.data.asDouble != NULL	){
				uint64_t j = 0;
				double * d = (double *) result.data.asDouble;
				/*for(; j < result.resultCount; j ++){
					printf("rid : %d, val: %f \n", result.rids[j], d[j]);
				}*/
        	}
			ALQueryResultDestroy(&result);
		}

//        	printf("total results: [%llu] \n", totalResultCount);
ALTimer_stop("totalqueryprocess");
		printf("total_time: %9.3lf    metadata_read: %9.3lf     index_read: %9.3lf    lob_read: %9.3lf      candidate_check: %9.3lf\n ", ALTimer_get_total_interval("totalqueryprocess"),
				ALTimer_get_total_interval("metadata_read"), ALTimer_get_total_interval("index_read"), ALTimer_get_total_interval("lob_read"), ALTimer_get_total_interval("candidate_check"));
	}
	CATCH(tc1){
	IF_EL(2):
	IF_EL(1):
	eprintf(EMSG);
	eprintf("\n");
}ENDTRYCATCH
}

