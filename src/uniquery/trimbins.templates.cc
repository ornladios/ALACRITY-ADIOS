/*
 * trimbins.templates.cc
 *
 *  Created on: Nov 14, 2012
 *      Author: Drew
 */

extern "C" {
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <alacrity.h>
#include <ALUtil.h>
#include <uniquery/helpers.h>
}

namespace {
template<class ET>
uint64_t trimQueryResultsTemplate(ALUnivariateQueryResult *result, const ALMetadata *meta,
                                  ET lb, ET ub, bin_id_t start_bin, bin_id_t end_bin);
}

extern "C" {
uint64_t trimQueryResults(ALUnivariateQueryResult *result, const ALMetadata *meta,
                          ALUnivariateQuery *query, bin_id_t start_bin, bin_id_t end_bin) {
    switch (meta->datatype) {
    case DATATYPE_FLOAT64:
		return trimQueryResultsTemplate<double>(result, meta, query->lb.asDouble, query->ub.asDouble, start_bin, end_bin);
    case DATATYPE_FLOAT32:
		return trimQueryResultsTemplate<float>(result, meta, query->lb.asFloat, query->ub.asFloat, start_bin, end_bin);
    default:
        eprintf("Unsupported data type %d in %s\n", (int)meta->datatype, __FUNCTION__);
        assert(false);
    	return 0;
    }
}
}

namespace {
template<class ET>
uint64_t trimQueryResultsTemplate(ALUnivariateQueryResult *result, const ALMetadata *meta,
                                  ET lb, ET ub, bin_id_t start_bin, bin_id_t end_bin) {
	const ALBinLayout * const bl = &meta->binLayout;
	const bin_offset_t start_bin_off = bl->binStartOffsets[start_bin];

	// Relative to the first bin's start offset
	const bin_offset_t firstBinEndOff = bl->binStartOffsets[start_bin + 1] - start_bin_off;
	const bin_offset_t lastBinStartOff = bl->binStartOffsets[end_bin - 1] - start_bin_off;
	const bin_offset_t lastBinEndOff = bl->binStartOffsets[end_bin] - start_bin_off;

	ET *dataPtr = (ET*)result->data.asChar;

	bin_offset_t inputOff;
	bin_offset_t firstBinTrimmedStartOff;
	bin_offset_t lastBinTrimmedEndOff;

	// Trim first bin
	// Iterate from firstBinEndOff - 1 to 0, inclusive
	firstBinTrimmedStartOff = firstBinEndOff;
	for (inputOff = firstBinEndOff - 1; inputOff + 1 > 0; inputOff--) {
		ET val = dataPtr[inputOff];
		if (val >= lb && val < ub) {
			firstBinTrimmedStartOff--;
			dataPtr[firstBinTrimmedStartOff] = val;
			result->rids[firstBinTrimmedStartOff] = result->rids[inputOff];
		}
	}

	// If the end bin is distinct (i.e., >= 2 bins exist), filter that too
	if (end_bin - start_bin > 1) {
		lastBinTrimmedEndOff = lastBinStartOff;
		for (inputOff = lastBinStartOff; inputOff < lastBinEndOff; inputOff++) {
			ET val = dataPtr[inputOff];
			if (val >= lb && val < ub) {
				dataPtr[lastBinTrimmedEndOff] = val;
				result->rids[lastBinTrimmedEndOff] = result->rids[inputOff];
				lastBinTrimmedEndOff++;
			}
		}
	} else {
		lastBinTrimmedEndOff = lastBinEndOff;
	}

	//            printf("End cuts: %llu/%llu\n", results_begin_off, (last_bin_end_off - results_end_off));

	result->data.asChar += firstBinTrimmedStartOff * meta->elementSize;
	result->rids += firstBinTrimmedStartOff;

	const uint64_t oldResultCount = result->resultCount;
	result->resultCount = lastBinTrimmedEndOff - firstBinTrimmedStartOff;

	return oldResultCount - result->resultCount;
}
}