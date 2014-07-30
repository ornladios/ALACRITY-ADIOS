/*
 * binrange.h
 *
 *  Created on: Nov 14, 2012
 *      Author: David A. Boyuka II
 */

#ifndef BINRANGE_H_
#define BINRANGE_H_

#include <stdint.h>
#include <stdbool.h>
#include <alacrity.h>

// All bin ranges are [start,end)

/*
 * Finds all bins that intersect the given query range, assuming the elements
 * are one's-complement-based (float, double). Both query bounds are void*
 * pointers because their type size differs depending on the element size
 * (they must both be of size exactly meta->elementSize).
 * @param meta the metadata for the partition to query
 * @param query the query containing the query range
 * @param start_bin an output pointer where the start bin ID will be placed
 * @param end_bin an output pointer where the end bin ID will be placed
 * @return true if an intersecting bin range is found, false if the query interval
 *         intersects no bins in this partition.
 */
_Bool findBinRange1C(const ALMetadata *meta,
                     ALUnivariateQuery *query,
                     bin_id_t *start_bin, bin_id_t *end_bin);

/*
 * Read the given bins data from the given partition store.
 * The data is reconstituted into full meta->elementSize byte elements.
 * Note: the bin range is inclusive/exclusive (i.e., [start, end) )
 * @param ps the partition store from which to read the index/data bins
 * @param meta the metadata for this partition
 * @param start_bin the start of the bin range to read (inclusive)
 * @param end_bin the end of the bin range to read (exclusive)
 * @param end_bins_only if true, only the first and last bins are read;
 *        otherwise, all bins in the range are read
 * @param data a pointer to the ALData that will be filled
 * @return the number of elements read and reconstituted
 */
uint64_t readAndReconstituteData(ALPartitionStore *ps, const ALMetadata *meta,
                                         bin_id_t start_bin, bin_id_t end_bin,
                                         _Bool end_bins_only, ALData *data);

/*
 * Scans the start and end bin of query results and performs fine-grained trimming
 * of values that are not in the query range (i.e., they fall in a bin that
 * contains only *some* values in the range). Updates results->data and results->rids
 * as output, and returns the number of values trimmed.
 * @param result the query result to trim
 * @param meta the metadata for the partition from which these results come
 * @param query the query containing the query range with which to trim
 * @param start_bin the ID of the first bin contained in the results (i.e. first touched bin)
 * @param end_bin the ID of the last bin contained in the results (i.e. last touched bin)
 */
uint64_t trimQueryResults(ALUnivariateQueryResult *result, const ALMetadata *meta,
                          ALUnivariateQuery *query, bin_id_t start_bin, bin_id_t end_bin);

/*
 * reassemble the low-order byte & bin header value to the original value
 * for lower-order bins ( [start_bin , end_bin) )
 */
void reconstituteData(const ALMetadata *meta, bin_id_t start_bin, bin_id_t end_bin,
		                         char *start_bin_input, char *start_bin_output);
#endif /* BINRANGE_H_ */
