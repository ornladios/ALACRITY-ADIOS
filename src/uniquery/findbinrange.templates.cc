
/*
 * binrange_template.cc
 *
 *  Created on: Nov 12, 2012
 *      Author: David A. Boyuka II
 */

extern "C" {
#include <stdint.h>
#include <alacrity.h>
#include "../include/alacrity-util.h"
#include <uniquery/helpers.h>
}

// Find the bin range that intersects with the given query interval, assuming elements are in
// one's complement (i.e., floating point numbers).
// Template parameters should have UT = unsigned integer of size elementSize, and ST = signed integer of size elementSize
namespace {
template<class UT, class ST>
bool findBinRange1CTemplate(const ALMetadata *meta, UT query_lb, UT query_ub, bin_id_t *start_bin, bin_id_t *end_bin);
}

extern "C" {
    #include <stdbool.h>
    _Bool findBinRange1C(const ALMetadata *meta, ALUnivariateQuery *query,
                         bin_id_t *start_bin, bin_id_t *end_bin) {
        switch (meta->elementSize) {
        case sizeof(uint64_t):
            return findBinRange1CTemplate<uint64_t, int64_t>(meta, query->lb.asUint64, query->ub.asUint64, start_bin, end_bin);
        case sizeof(uint32_t):
            return findBinRange1CTemplate<uint32_t, int32_t>(meta, query->lb.asUint32, query->ub.asUint32, start_bin, end_bin);
        case sizeof(uint16_t):
            return findBinRange1CTemplate<uint16_t, int16_t>(meta, query->lb.asUint16, query->ub.asUint16, start_bin, end_bin);
        case sizeof(uint8_t):
            return findBinRange1CTemplate<uint8_t, int8_t>(meta, query->lb.asUint8, query->ub.asUint8, start_bin, end_bin);
        default:
            eprintf("Unsupported element size %d in %s\n", (int)meta->elementSize, __FUNCTION__);
            return false;
        }
    }
}

// Variable naming conventions:
// "lb" or "ub" appearing in a variable name means it's related to a lower or upper bound
// "s_" prefix means signed (no such prefix means unsigned)
// "_hi" suffix means the variable contains a value to be interpreted as "sigbits" long
//   (though 2's complement may fill the higher bits with 1's for negative numbers)
namespace {
template<class UT, class ST>
bool findBinRange1CTemplate(const ALMetadata *meta, UT query_lb, UT query_ub, bin_id_t *start_bin, bin_id_t *end_bin) {
    const ALBinLayout * const bl = &meta->binLayout;
    const int sigbits = meta->significantBits;
    const int insigbits = (meta->elementSize << 3) - sigbits;
    const int sigbytes = alacrity_util_sigBytesCeil(meta);
    const int insigbytes = alacrity_util_insigBytesCeil(meta);
    const UT sign_mask_hi = ((UT)1) << (sigbits - 1);

    // Note, by converting the query bounds to bin values, we can treat them as
    // inclusive whenever they intersect a bin value.
    // The reason for this is not obvious, since it rounds the values toward 0,
    // but it ends up working (see the comment at the top of this file for more info,
    // if I've bothered to write it yet).
    const UT query_lb_hi = query_lb >> insigbits;
    const UT query_ub_hi = query_ub >> insigbits;
    const ST s_query_lb_hi = CONV_1C_TO_2C(query_lb_hi, sign_mask_hi);
    const ST s_query_ub_hi = CONV_1C_TO_2C(query_ub_hi, sign_mask_hi);

    dbprintf("Query bound unsigned high parts: %ld / %lu\n", query_lb_hi, query_ub_hi);
    dbprintf("Query bound signed high parts: %ld / %lu\n", s_query_lb_hi, s_query_ub_hi);

    const high_order_bytes_t * const startBinValPtr = bl->binValues;
    const high_order_bytes_t * const endBinValPtr = bl->binValues + bl->numBins;
    const high_order_bytes_t *curBinValPtr = startBinValPtr;

    bool found_start = false, found_end = false;

    // We break the bin loop into 4 for loops for simplicity and efficiency. They all share the
    // same iterator pointer, and so are always moving that pointer forward (toward bins with
    // more positive value ranges).
    // * The first pair of loops find the low bin, and the second pair find the high bin
    // * The first loop of each pair iterates over the negative bins, while the second completes the positive ones
    //
    // * In summary, each loop has a set of terminal conditions, and is idempotent if any are met upon entering:
    //     Loop 1: out of bins OR current bin is positive OR start bin is found
    //     Loop 2: out of bins OR start bin is found
    //     Loop 3: out of bins OR current bin is positive OR end bin is found
    //     Loop 4: out of bins OR end bin is found
    //   Note that being in Loop 2 or 4 implies that the current bin is positive, so this isn't explicitly checked.
    //
    // Let the value for bin B be V[B].
    // * If V[B] is negative, bin bounds are ( V[B] + 1, V[B] ]   (larger unsigned values are smaller signed values)
    // * If V[B] is positive, bin bounds are [ V[B], V[B] + 1 )   (larger unsigned values are larger signed values)
    // However, we don't need to worry about this; we can treat a bin as a single point at its bin value.
    // The query interval intersects a bin if this single point is in the (inclusive) query interval

    // Low bin: the bin closest to -inf that is at or above the low bound of the query inverval
    // High bin: the bin closest to +inf that is at or below the high bound of the query inverval
    //   Alternatively: the bin before the bin closest to -inf that is above the high bound of the query interval
    // Note: if low bin > query ub || high bin < query lb, fail

    // BEGIN FIND START BIN

    // Neg. bins, find start bin.
    while (curBinValPtr != endBinValPtr) {
        const UT binval_hi = *curBinValPtr;
        if ((binval_hi & sign_mask_hi) == 0) break; // Break if bin value is positive

        const ST s_binval_hi = CONV_NEG_1C_TO_2C((ST)binval_hi, sign_mask_hi);

        // If the bin is at or above the lower bound of the query range, this is the edge bin
        if (s_binval_hi >= s_query_lb_hi) {
            if (s_binval_hi > s_query_ub_hi)
                break;

            found_start = true;
            *start_bin = curBinValPtr - startBinValPtr;
            break;
        }
        curBinValPtr++;
    }
    // Pos. bins, find start bin.
    if (!found_start) {
        while (curBinValPtr != endBinValPtr) {
            const UT binval_hi = *curBinValPtr;
            const ST s_binval_hi = CONV_POS_1C_TO_2C((ST)binval_hi, sign_mask_hi);

            // If the bin is at or above the lower bound of the query range, this is the edge bin
            if (s_binval_hi >= s_query_lb_hi) {
                if (s_binval_hi > s_query_ub_hi)
                    break;

                found_start = true;
                *start_bin = curBinValPtr - startBinValPtr;
                break;
            }
            curBinValPtr++;
        }
    }

    // If we didn't find the start bin, quit now
    if (!found_start)
        return false;

    // END FIND START BIN

    // At this point, the start bin has been found, but it may be entirely
    // above the query interval

    // Neg. bins, find end bin.
    while (curBinValPtr != endBinValPtr) {
        const UT binval_hi = *curBinValPtr;
        if ((binval_hi & sign_mask_hi) == 0) break; // Break if bin value is positive

        const ST s_binval_hi = CONV_NEG_1C_TO_2C((ST)binval_hi, sign_mask_hi);

        // If the bin is strictly above the upper bound of the query range, the previous bin is the last bin
        // to query, making this bin the end bin (end_bin is exclusive, remember)
        if (s_binval_hi > s_query_ub_hi) {
            // If we are at the first bin, then all bins are totally above the query interval.
            // Just pick the first bin as end bin; it will be deemed invalid in the check
            // after the loops.
            if (curBinValPtr == startBinValPtr)
                *end_bin = 0;
            else
                *end_bin = curBinValPtr - startBinValPtr;

            found_end = true;
            break;
        }
        curBinValPtr++;
    }
    if (!found_end) {
        // Pos. bins, find end bin. Note: bin range = [ VVVV0000, VVVV0000 + 1 ) (V = high bits from bin value)
        while (curBinValPtr != endBinValPtr) {
            const UT binval_hi = *curBinValPtr;

            const ST s_binval_hi = CONV_POS_1C_TO_2C((ST)binval_hi, sign_mask_hi);

            // If the bin is strictly above the upper bound of the query range, the previous bin is the last bin
            // to query, making this bin the end bin (end_bin is exclusive, remember)
            if (s_binval_hi > s_query_ub_hi) {
                // If we are at the first bin, then all bins are totally above the query interval.
                // Just pick the first bin as end bin; it will be deemed invalid in the check
                // after the loops.
                if (curBinValPtr == startBinValPtr)
                    *end_bin = 0;
                else
                    *end_bin = curBinValPtr - startBinValPtr;

                found_end = true;
                break;
            }
            curBinValPtr++;
        }
    }

    // If we didn't find an end bin, either the last bin is within
    // the query range (so there was no next bin to ``back up'' from),
    // or the query range is disjoint from the bin set. Set the end
    // bin to the last bin; it will solve the first problem, and
    // the second problem will still be detected momentarily.
    if (!found_end)
        *end_bin = bl->numBins;

    // This shouldn't happen, but if the bin range is empty, return immediately
    if (*start_bin == *end_bin)
    	return false;

    // If the start bin is above the query end, or the end bin
    // is below the query start, fail, because the query range
    // is disjoint from the bin set
    ST s_first_bin_val = CONV_1C_TO_2C((ST)bl->binValues[*start_bin], sign_mask_hi);
    ST s_last_bin_val = CONV_1C_TO_2C((ST)bl->binValues[*end_bin - 1], sign_mask_hi);
    if (s_first_bin_val > s_query_ub_hi ||
    	s_last_bin_val < s_query_lb_hi)
        return false;

    // All tests have now passed:
    // * A start and end bin have been found
    // * start_bin_val >= query_lb and end_bin_val <= query_ub by construction
    // * start_bin_val <= query_ub and end_bin_val >= query_lb, so the query
    //   range and bin range intersect
    return true;
}
}