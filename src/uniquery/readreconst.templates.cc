/*
 * reconst.templates.cc
 *
 *  Created on: Nov 14, 2012
 *      Author: David A. Boyuka II
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <alacrity.h>
#include <ALUtil.h>
}

template<class UT>
static uint64_t readAndReconstituteDataTemplate(ALPartitionStore *ps, const ALMetadata *meta,
                                                bin_id_t start_bin, bin_id_t end_bin,
                                                _Bool end_bins_only, ALData *data);


template<class UT>
static void reconstituteElements(const ALMetadata *meta, bin_id_t start_bin, bin_id_t end_bin,
		                         char *start_bin_input, char *start_bin_output);


extern "C" {
uint64_t readAndReconstituteData(ALPartitionStore *ps, const ALMetadata *meta,
                                 bin_id_t start_bin, bin_id_t end_bin,
                                 _Bool end_bins_only, ALData *data) {
    switch (meta->elementSize) {
    case sizeof(uint64_t):
        return readAndReconstituteDataTemplate<uint64_t>(ps, meta, start_bin, end_bin, end_bins_only, data);
    case sizeof(uint32_t):
        return readAndReconstituteDataTemplate<uint32_t>(ps, meta, start_bin, end_bin, end_bins_only, data);
    case sizeof(uint16_t):
        return readAndReconstituteDataTemplate<uint16_t>(ps, meta, start_bin, end_bin, end_bins_only, data);
    case sizeof(uint8_t):
        return readAndReconstituteDataTemplate<uint8_t>(ps, meta, start_bin, end_bin, end_bins_only, data);
    default:
        eprintf("Unsupported element size %d in %s\n", (int)meta->elementSize, __FUNCTION__);
        assert(false);
        return 0;
    }
}

void reconstituteData(const ALMetadata *meta, bin_id_t start_bin, bin_id_t end_bin,
		                         char *start_bin_input, char *start_bin_output) {

	 switch (meta->elementSize) {
	    case sizeof(uint64_t):
	        return reconstituteElements<uint64_t>( meta, start_bin, end_bin, start_bin_input, start_bin_output);
	    case sizeof(uint32_t):
	        return reconstituteElements<uint32_t>( meta, start_bin, end_bin, start_bin_input, start_bin_output);
	    case sizeof(uint16_t):
	        return reconstituteElements<uint16_t>( meta, start_bin, end_bin, start_bin_input, start_bin_output);
	    case sizeof(uint8_t):
	        return reconstituteElements<uint8_t>( meta, start_bin, end_bin, start_bin_input, start_bin_output);
	    default:
	        eprintf("Unsupported element size %d in %s\n", (int)meta->elementSize, __FUNCTION__);
	        assert(false);
	        return ;
	    }
}
}

// start_bin_{input,output} = pointer to the beginning of the data for the start_bin
// in the {input,output} buffer
template<class UT>
static void reconstituteElements(const ALMetadata *meta, bin_id_t start_bin, bin_id_t end_bin,
		                         char *start_bin_input, char *start_bin_output) {

	const ALBinLayout * const bl = &meta->binLayout;
	int insigbytes = insigBytesCeil(meta);
	int insigbits = (meta->elementSize << 3) - meta->significantBits;

	UT reconst_elem;
	UT high_mask;
	UT *next_output = (UT*)start_bin_output;
	char *next_input = (char*)start_bin_input;

	// First reconstitute the values
	// Iterate through the bins backwards
	for (bin_id_t bin = start_bin; bin < end_bin; bin++) {
		const bin_offset_t off_start = bl->binStartOffsets[bin];
		const bin_offset_t off_end = bl->binStartOffsets[bin + 1];

		high_mask = ((UT)bl->binValues[bin]) << insigbits;

		// Iterate through the elements backwards
		for (bin_offset_t off = off_start; off < off_end; off++) {
			GET_BUFFER_ELEMENT(reconst_elem, next_input, insigbytes);
			reconst_elem |= high_mask;
			*next_output++ = reconst_elem;
			next_input += insigbytes;
		}
	}
}

template<class UT>
static uint64_t readAndReconstituteDataTemplate(ALPartitionStore *ps, const ALMetadata *meta,
                                                bin_id_t start_bin, bin_id_t end_bin,
                                                _Bool end_bins_only, ALData *data) {
    const ALBinLayout * const bl = &meta->binLayout;
    const int insigbits = (meta->elementSize << 3) - meta->significantBits;
    const int insigbytes = insigBytesCeil(meta);

    const bin_offset_t elementCount = bl->binStartOffsets[end_bin] - bl->binStartOffsets[start_bin];
	const bin_offset_t start_bin_off = bl->binStartOffsets[start_bin];

    *data = (ALData)malloc(elementCount * meta->elementSize);

    // Read the low bytes aligned to the end of the data array, so we can
    // expand them backwards, allowing forward iteration rather than reverse
    ALData lowData = (char*)*data + (meta->elementSize - insigbytes) * elementCount;

    //dbprintf(">>> About to read data bins %hu to %hu (%hu bins, %lu elements)\n", start_bin, end_bin, (end_bin - start_bin), meta->binLayout.binStartOffsets[end_bin] - meta->binLayout.binStartOffsets[start_bin]);

    // If we are to read only the start and end bins, and there are more than 2
    // bins (meaning there is a gap in between them), perform separate reads
    // for each boundary bin. Otherwise, do a full bin read.
    if (end_bins_only && (end_bin - start_bin) > 2) {
    	bin_offset_t last_bin_elem_off = bl->binStartOffsets[end_bin - 1] - bl->binStartOffsets[start_bin];
    	uint64_t last_bin_byte_offset = last_bin_elem_off * insigbytes;

    	ALData first_bin_data = lowData;
    	ALData last_bin_data = lowData + last_bin_byte_offset;

    	UT *first_bin_output = ((UT*)*data);
    	UT *last_bin_output = ((UT*)*data) + last_bin_elem_off;
    	
    	// Read and reconst. the first bin
    	ALPartitionStoreReadDataBins(ps, meta, start_bin, start_bin + 1, &first_bin_data);
    	reconstituteElements<UT>(meta, start_bin, start_bin + 1, first_bin_data, (char*)first_bin_output);

    	// Read and reconst. the last bin
    	ALPartitionStoreReadDataBins(ps, meta, end_bin - 1, end_bin, &last_bin_data);
    	reconstituteElements<UT>(meta, end_bin - 1, end_bin, last_bin_data, (char*)last_bin_output);
    } else {
    	// Read and reconst. all bins
        ALPartitionStoreReadDataBins(ps, meta, start_bin, end_bin, &lowData);
    	reconstituteElements<UT>(meta, start_bin, end_bin, lowData, *data);
    }

    return elementCount;
}

