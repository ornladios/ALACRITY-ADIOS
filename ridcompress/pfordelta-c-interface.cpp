/*
 * c-interface.c
 *
 *  Created on: Sep 12, 2012
 *      Author: David A. Boyuka II
 */

#include <iostream>
#include <stdint.h>
#include <cstring>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <assert.h>
#include "pfordelta-c-interface.h"
#include "patchedframeofreference.h"
using namespace std;
using namespace pfor;

#ifdef __cplusplus
extern "C" {
#endif


int adios_decode_deltas(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount) {

	uint32_t remaining_length = inputLength;
	uint32_t current_length = 0;
	uint32_t *current_data = output;
	*outputCount = 0;
	while (remaining_length > 0) {
		const char *current_buffer = (input + current_length);

		/*if (remaining_buffer < sizeof(uint32_t)) {
		 return 0;
		 }*/

		uint32_t data_size = *(const uint32_t *) current_buffer;

		current_buffer += sizeof(uint32_t);
		current_length += sizeof(uint32_t);
		remaining_length -= sizeof(uint32_t);

		uint32_t remaining_data = data_size;
		//cout << "reached here" << endl;
		while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			PatchedFrameOfReference::decode_new_to_deltas(current_buffer,
					remaining_length, current_data,
					PatchedFrameOfReference::kBatchSize, actual_data_size,
					actual_significant_data_size, actual_buffer_size);

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			*outputCount += actual_significant_data_size;
			current_data += PatchedFrameOfReference::kBatchSize;
			remaining_data -= PatchedFrameOfReference::kBatchSize;
		}
		//cout << "reached here too too" << endl;
		if (remaining_data > 0) {
			uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			PatchedFrameOfReference::decode_new_to_deltas(current_buffer,
					remaining_length, temp_data,
					PatchedFrameOfReference::kBatchSize, actual_data_size,
					actual_significant_data_size, actual_buffer_size);

			memcpy(current_data, temp_data,
					actual_significant_data_size * sizeof(uint32_t));

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			*outputCount += actual_significant_data_size;
			current_data += remaining_data;
			remaining_data -= remaining_data;
		}
	}

	return 1;
	  
}


/*
 * rle decoding to bitmap in another dimension space
 */
uint32_t runlength_decode_rids_to_selbox(bool isPGContained /*1: PG space is fully contained in the selection box, 0: intersected*/
, const char *input, uint64_t inputLength, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount, int dim, void **bitmap) {
	bmap_t ** bmap = (bmap_t **) bitmap;
	uint32_t remaining_length = inputLength;
	uint32_t current_length = 0;

	uint32_t decodedDataElm = 0;

	if (isPGContained == true) {

		while (remaining_length > 0) {
			const char *current_buffer = (input + current_length);

			uint32_t data_size = *(const uint32_t *) current_buffer;

			current_buffer += sizeof(uint32_t);
			current_length += sizeof(uint32_t);
			remaining_length -= sizeof(uint32_t);

			uint32_t remaining_data = data_size;
			// When PG is within the selection box,
			// it does not filter out RID during decoding
			decodedDataElm += remaining_data;

			while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
				uint32_t actual_data_size;
				uint32_t actual_significant_data_size;
				uint32_t actual_buffer_size;

				if (!PatchedFrameOfReference::rle_decode_every_batch_to_selbox_withoutCheck(
						current_buffer, remaining_length, actual_data_size,
						actual_significant_data_size, actual_buffer_size,
						 srcstart, srccount, deststart,
						destcount, dim, bmap)) {
					return 0;
				}
				if (actual_data_size != PatchedFrameOfReference::kBatchSize
						|| actual_significant_data_size
								!= PatchedFrameOfReference::kBatchSize) {
					return 0;
				}

				current_buffer += actual_buffer_size;
				current_length += actual_buffer_size;
				remaining_length -= actual_buffer_size;

				remaining_data -= PatchedFrameOfReference::kBatchSize;
			}
			if (remaining_data > 0) {
				uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

				uint32_t actual_data_size;
				uint32_t actual_significant_data_size;
				uint32_t actual_buffer_size;

				if (!PatchedFrameOfReference::rle_decode_every_batch_to_selbox_withoutCheck(
						current_buffer, remaining_length, actual_data_size,
						actual_significant_data_size, actual_buffer_size,
						 srcstart, srccount, deststart,
						destcount, dim, bmap)) {
					return 0;
				}

				if (actual_data_size != PatchedFrameOfReference::kBatchSize
						|| actual_significant_data_size != remaining_data) {
				}

				current_buffer += actual_buffer_size;
				current_length += actual_buffer_size;
				remaining_length -= actual_buffer_size;
				remaining_data -= remaining_data;
			}
		}
		if (remaining_length != 0) {
			return 0;
		}
	} else {
		const char *current_buffer = (input + current_length);

		uint32_t data_size = *(const uint32_t *) current_buffer;

		current_buffer += sizeof(uint32_t);
		current_length += sizeof(uint32_t);
		remaining_length -= sizeof(uint32_t);

		uint32_t remaining_data = data_size;
		//cout << "reached here" << endl;
		uint32_t actual_significant_data_size;
		while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
			uint32_t actual_data_size;
			uint32_t actual_buffer_size;

			if (!PatchedFrameOfReference::rle_decode_every_batch_to_selbox_withCheck(
					current_buffer, remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size,  srcstart, srccount, deststart, destcount, dim, bmap)) {
				return 0;
			}

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			remaining_data -= PatchedFrameOfReference::kBatchSize;
			decodedDataElm += actual_significant_data_size;
		}
		if (remaining_data > 0) {
			uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

			uint32_t actual_data_size;
			uint32_t actual_buffer_size;

			if (!PatchedFrameOfReference::rle_decode_every_batch_to_selbox_withCheck(
					current_buffer, remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size, srcstart, srccount, deststart, destcount, dim, bmap)) {
				return 0;
			}

			decodedDataElm += actual_significant_data_size;
			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;
			remaining_data -= remaining_data;
		}
		if (remaining_length != 0) {
			return 0;
		}
	}

	return decodedDataElm;
}



//BitExp
//Decoding of BitExp is same as PForDelta decoding
int adios_expand_decode_rid(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount){
	return decode_rids(input,inputLength, output, outputCount); // decode
}

//BitRun
int adios_runlength_decode_rid(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount){

	    uint32_t remaining_length = inputLength;
		uint32_t current_length = 0;
		uint32_t *current_data = output;
		*outputCount = 0;
		while (remaining_length > 0) {
			const char *current_buffer = (input + current_length);
			uint32_t data_size = *(const uint32_t *) current_buffer;

			current_buffer += sizeof(uint32_t);
			current_length += sizeof(uint32_t);
			remaining_length -= sizeof(uint32_t);

			uint32_t remaining_data = data_size;
			while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
				uint32_t actual_data_size;
				uint32_t actual_significant_data_size;
				uint32_t actual_buffer_size;

				if (!PatchedFrameOfReference::rle_decode_every_batch_to_rids(
						current_buffer, remaining_length, actual_data_size,
						actual_significant_data_size, actual_buffer_size, current_data)) {
					return PRINT_ERROR_RETURN_FAIL("RLE decode failed \n ")
					;
				}
				if (actual_data_size != PatchedFrameOfReference::kBatchSize
						|| actual_significant_data_size
								!= PatchedFrameOfReference::kBatchSize) {
					return PRINT_ERROR_RETURN_FAIL("RLE decode size not match")
					;
				}

				current_buffer += actual_buffer_size;
				current_length += actual_buffer_size;
				remaining_length -= actual_buffer_size;

				*outputCount += actual_significant_data_size;
				current_data += PatchedFrameOfReference::kBatchSize;
				remaining_data -= PatchedFrameOfReference::kBatchSize;
			}
			if (remaining_data > 0) {
				uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };
				uint32_t actual_data_size;
				uint32_t actual_significant_data_size;
				uint32_t actual_buffer_size;

				if (!PatchedFrameOfReference::rle_decode_every_batch_to_rids(
						current_buffer, remaining_length, actual_data_size,
						actual_significant_data_size, actual_buffer_size, temp_data)) {
					return 0;
				}

				memcpy(current_data, temp_data,
						actual_significant_data_size * sizeof(uint32_t));

				current_buffer += actual_buffer_size;
				current_length += actual_buffer_size;
				remaining_length -= actual_buffer_size;

				*outputCount += actual_significant_data_size;
				current_data += remaining_data;
				remaining_data -= remaining_data;
			}
		}
		if (remaining_length != 0) {
			return 0;
		}
		return 1;
}

int adios_expand_runlength_decode_rid(const char *input, uint64_t inputLength, uint32_t *output, uint32_t *outputCount){
	return adios_runlength_decode_rid(input,inputLength,output,outputCount);
}


uint32_t decode_rids_to_selbox(bool isPGContained /*1: PG space is fully contained in the selection box, 0: intersected*/
, const char *input, uint64_t inputLength, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount, int dim, void **bitmap) {
	bmap_t ** bmap = (bmap_t **) bitmap;
	uint32_t remaining_length = inputLength;
	uint32_t current_length = 0;

	uint32_t decodedDataElm = 0;

	if (isPGContained == true) {

		while (remaining_length > 0) {
			const char *current_buffer = (input + current_length);

			uint32_t data_size = *(const uint32_t *) current_buffer;

			current_buffer += sizeof(uint32_t);
			current_length += sizeof(uint32_t);
			remaining_length -= sizeof(uint32_t);

			uint32_t remaining_data = data_size;
			// When PG is within the selection box,
			// it does not filter out RID during decoding
			decodedDataElm += remaining_data;

			while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
				uint32_t actual_data_size;
				uint32_t actual_significant_data_size;
				uint32_t actual_buffer_size;

				if (!PatchedFrameOfReference::batch_decode_within_selbox_without_checking(
						current_buffer, remaining_length, actual_data_size,
						actual_significant_data_size, actual_buffer_size,
						 srcstart, srccount, deststart,
						destcount, dim, bmap)) {
					return 0;
				}
				if (actual_data_size != PatchedFrameOfReference::kBatchSize
						|| actual_significant_data_size
								!= PatchedFrameOfReference::kBatchSize) {
					return 0;
				}

				current_buffer += actual_buffer_size;
				current_length += actual_buffer_size;
				remaining_length -= actual_buffer_size;

				remaining_data -= PatchedFrameOfReference::kBatchSize;
			}
			if (remaining_data > 0) {
				uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

				uint32_t actual_data_size;
				uint32_t actual_significant_data_size;
				uint32_t actual_buffer_size;

				if (!PatchedFrameOfReference::batch_decode_within_selbox_without_checking(
						current_buffer, remaining_length, actual_data_size,
						actual_significant_data_size, actual_buffer_size,
						 srcstart, srccount, deststart,
						destcount, dim, bmap)) {
					return 0;
				}

				if (actual_data_size != PatchedFrameOfReference::kBatchSize
						|| actual_significant_data_size != remaining_data) {
				}

				current_buffer += actual_buffer_size;
				current_length += actual_buffer_size;
				remaining_length -= actual_buffer_size;
				remaining_data -= remaining_data;
			}
		}
		if (remaining_length != 0) {
			return 0;
		}
	} else {
		const char *current_buffer = (input + current_length);

		uint32_t data_size = *(const uint32_t *) current_buffer;

		current_buffer += sizeof(uint32_t);
		current_length += sizeof(uint32_t);
		remaining_length -= sizeof(uint32_t);

		uint32_t remaining_data = data_size;
		//cout << "reached here" << endl;
		uint32_t actual_significant_data_size;
		while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
			uint32_t actual_data_size;
			uint32_t actual_buffer_size;

			if (!PatchedFrameOfReference::batch_decode_within_selbox_with_checking(
					current_buffer, remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size,  srcstart, srccount, deststart, destcount, dim, bmap)) {
				return 0;
			}

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			remaining_data -= PatchedFrameOfReference::kBatchSize;
			decodedDataElm += actual_significant_data_size;
		}
		if (remaining_data > 0) {
			uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

			uint32_t actual_data_size;
			uint32_t actual_buffer_size;

			if (!PatchedFrameOfReference::batch_decode_within_selbox_with_checking(
					current_buffer, remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size, srcstart, srccount, deststart, destcount, dim, bmap)) {
				return 0;
			}

			decodedDataElm += actual_significant_data_size;
			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;
			remaining_data -= remaining_data;
		}
		if (remaining_length != 0) {
			return 0;
		}
	}

	return decodedDataElm;
}

/*
 * expansion + runlength encode
 */

int exapnd_runlength_encode_rids(const uint32_t *input, uint32_t inputCount,
		char *output, uint64_t *outputLength) {
	*((uint32_t *) output) = inputCount;
	*outputLength = sizeof(uint32_t);
	output += sizeof(uint32_t);

	const uint32_t *current_data = input;
	uint32_t remaining_data = inputCount;

	char *current_buffer = output;
	char temp_buffer[PatchedFrameOfReference::kSufficientBufferCapacity];
	uint32_t temp_buffer_size;

	while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
		if (!PatchedFrameOfReference::expand_runlength_encode(current_data,
				PatchedFrameOfReference::kBatchSize,
				PatchedFrameOfReference::kBatchSize, temp_buffer,
				sizeof(temp_buffer), temp_buffer_size)) {
			return 0;
		}
		memcpy(current_buffer, temp_buffer, temp_buffer_size);
		*outputLength += temp_buffer_size;

		current_buffer += temp_buffer_size;
		current_data += PatchedFrameOfReference::kBatchSize;
		remaining_data -= PatchedFrameOfReference::kBatchSize;

	}

	if (remaining_data > 0) {
		uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };
		memcpy(temp_data, current_data, remaining_data * sizeof(uint32_t));

		if (!PatchedFrameOfReference::expand_runlength_encode(temp_data,
				PatchedFrameOfReference::kBatchSize, remaining_data,
				temp_buffer, sizeof(temp_buffer), temp_buffer_size)) {
			return 0;
		}
		memcpy(current_buffer, temp_buffer, temp_buffer_size);
		*outputLength += temp_buffer_size;

		current_buffer += temp_buffer_size;
		current_data += remaining_data;
		remaining_data -= remaining_data;
	}
	return 1;
}

/*
 *
 * 	THE decoding of expansion + runlength is same as the decoding of runlength
 */
int expand_runlength_decode_rids(const char *input, uint64_t inputLength,
		void **bitmap) {
	rle_decode_rids(input, inputLength, bitmap);
}


/*
 * expansion encode
 */

int expand_encode_rids(const uint32_t *input, uint32_t inputCount,
		char *output, uint64_t *outputLength) {
	*((uint32_t *) output) = inputCount;
	*outputLength = sizeof(uint32_t);
	output += sizeof(uint32_t);

	const uint32_t *current_data = input;
	uint32_t remaining_data = inputCount;

	char *current_buffer = output;
	char temp_buffer[PatchedFrameOfReference::kSufficientBufferCapacity];
	uint32_t temp_buffer_size;

	while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
		if (!PatchedFrameOfReference::expand_encode(current_data,
				PatchedFrameOfReference::kBatchSize,
				PatchedFrameOfReference::kBatchSize, temp_buffer,
				sizeof(temp_buffer), temp_buffer_size)) {
			return 0;
		}
		memcpy(current_buffer, temp_buffer, temp_buffer_size);
		*outputLength += temp_buffer_size;

		current_buffer += temp_buffer_size;
		current_data += PatchedFrameOfReference::kBatchSize;
		remaining_data -= PatchedFrameOfReference::kBatchSize;

	}

	if (remaining_data > 0) {
		uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };
		memcpy(temp_data, current_data, remaining_data * sizeof(uint32_t));

		if (!PatchedFrameOfReference::expand_encode(temp_data,
				PatchedFrameOfReference::kBatchSize, remaining_data,
				temp_buffer, sizeof(temp_buffer), temp_buffer_size)) {
			return 0;
		}
		memcpy(current_buffer, temp_buffer, temp_buffer_size);
		*outputLength += temp_buffer_size;

		current_buffer += temp_buffer_size;
		current_data += remaining_data;
		remaining_data -= remaining_data;
	}
	return 1;
}

int expand_decode_rids(const char *input, uint64_t inputLength, void **bitmap) {

	bmap_t ** bmap = (bmap_t **) bitmap;
	uint32_t remaining_length = inputLength;
	uint32_t current_length = 0;
	//	uint32_t *current_data = output;
	//	*outputCount = 0;
	while (remaining_length > 0) {
		const char *current_buffer = (input + current_length);

		uint32_t data_size = *(const uint32_t *) current_buffer;

		current_buffer += sizeof(uint32_t);
		current_length += sizeof(uint32_t);
		remaining_length -= sizeof(uint32_t);

		uint32_t remaining_data = data_size;
		while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			PatchedFrameOfReference::expand_decode_every_batch(current_buffer,
					remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size, bmap);

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			/**outputCount += actual_significant_data_size;
			 current_data += PatchedFrameOfReference::kBatchSize;*/
			remaining_data -= PatchedFrameOfReference::kBatchSize;
		}
		if (remaining_data > 0) {
			uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			PatchedFrameOfReference::expand_decode_every_batch(current_buffer,
					remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size, bmap);


			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			remaining_data -= remaining_data;
		}
	}
	if (remaining_length != 0) {
		return 0;
	}
	return 1;
}


uint32_t pfordelta_block_size() {
	return PatchedFrameOfReference::kBatchSize;
}


int rle_decode_rids(const char *input, uint64_t inputLength, void **bitmap) {

	bmap_t ** bmap = (bmap_t **) bitmap;
	uint32_t remaining_length = inputLength;
	uint32_t current_length = 0;
	//	uint32_t *current_data = output;
	//	*outputCount = 0;
	while (remaining_length > 0) {
		const char *current_buffer = (input + current_length);
		uint32_t data_size = *(const uint32_t *) current_buffer;

		current_buffer += sizeof(uint32_t);
		current_length += sizeof(uint32_t);
		remaining_length -= sizeof(uint32_t);

		uint32_t remaining_data = data_size;
		while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			if (!PatchedFrameOfReference::rle_decode_every_batch(
					current_buffer, remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size, bmap)) {
				return PRINT_ERROR_RETURN_FAIL("RLE decode failed \n ")
				;
			}
			if (actual_data_size != PatchedFrameOfReference::kBatchSize
					|| actual_significant_data_size
							!= PatchedFrameOfReference::kBatchSize) {
				return PRINT_ERROR_RETURN_FAIL("RLE decode size not match")
				;
			}

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			//			*outputCount += actual_significant_data_size;
			//			current_data += PatchedFrameOfReference::kBatchSize;
			remaining_data -= PatchedFrameOfReference::kBatchSize;
		}
		if (remaining_data > 0) {

			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			if (!PatchedFrameOfReference::rle_decode_every_batch(
					current_buffer, remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size, bmap)) {
				return 0;
			}

			if (actual_data_size != PatchedFrameOfReference::kBatchSize
					|| actual_significant_data_size != remaining_data) {
			}

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			//			*outputCount += actual_significant_data_size;
			//			current_data += remaining_data;
			remaining_data -= remaining_data;
		}
	}
	if (remaining_length != 0) {
		return 0;
	}
	return 1;
}

int hybrid_encode_rids(const uint32_t *input, uint32_t inputCount,
		char *output, uint64_t *outputLength) {
	*((uint32_t *) output) = inputCount;
	*outputLength = sizeof(uint32_t);
	output += sizeof(uint32_t);

	const uint32_t *current_data = input;
	uint32_t remaining_data = inputCount;

	char *current_buffer = output;
	char temp_buffer[PatchedFrameOfReference::kSufficientBufferCapacity];
	uint32_t temp_buffer_size;

	while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
		if (!PatchedFrameOfReference::hybrid_encode(current_data,
				PatchedFrameOfReference::kBatchSize,
				PatchedFrameOfReference::kBatchSize, temp_buffer,
				sizeof(temp_buffer), temp_buffer_size)) {
			return 0;
		}
		memcpy(current_buffer, temp_buffer, temp_buffer_size);
		*outputLength += temp_buffer_size;

		current_buffer += temp_buffer_size;
		current_data += PatchedFrameOfReference::kBatchSize;
		remaining_data -= PatchedFrameOfReference::kBatchSize;

	}

	if (remaining_data > 0) {
		uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };
		memcpy(temp_data, current_data, remaining_data * sizeof(uint32_t));

		if (!PatchedFrameOfReference::hybrid_encode(temp_data,
				PatchedFrameOfReference::kBatchSize, remaining_data,
				temp_buffer, sizeof(temp_buffer), temp_buffer_size)) {
			return 0;
		}
		memcpy(current_buffer, temp_buffer, temp_buffer_size);
		*outputLength += temp_buffer_size;

		current_buffer += temp_buffer_size;
		current_data += remaining_data;
		remaining_data -= remaining_data;
	}
	return 1;
}

int encode_rids(const uint32_t *input, uint32_t inputCount, char *output,
		uint64_t *outputLength) {
	*((uint32_t *) output) = inputCount;
	*outputLength = sizeof(uint32_t);
	output += sizeof(uint32_t);

	const uint32_t *current_data = input;
	uint32_t remaining_data = inputCount;

	char *current_buffer = output;
	char temp_buffer[PatchedFrameOfReference::kSufficientBufferCapacity];
	uint32_t temp_buffer_size;

	while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
		if (!PatchedFrameOfReference::encode(current_data,
				PatchedFrameOfReference::kBatchSize,
				PatchedFrameOfReference::kBatchSize, temp_buffer,
				sizeof(temp_buffer), temp_buffer_size)) {
			return 0;
		}
		memcpy(current_buffer, temp_buffer, temp_buffer_size);
		*outputLength += temp_buffer_size;

		current_buffer += temp_buffer_size;
		current_data += PatchedFrameOfReference::kBatchSize;
		remaining_data -= PatchedFrameOfReference::kBatchSize;

	}

	if (remaining_data > 0) {
		uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };
		memcpy(temp_data, current_data, remaining_data * sizeof(uint32_t));

		if (!PatchedFrameOfReference::encode(temp_data,
				PatchedFrameOfReference::kBatchSize, remaining_data,
				temp_buffer, sizeof(temp_buffer), temp_buffer_size)) {
			return 0;
		}
		memcpy(current_buffer, temp_buffer, temp_buffer_size);
		*outputLength += temp_buffer_size;

		current_buffer += temp_buffer_size;
		current_data += remaining_data;
		remaining_data -= remaining_data;
	}
	return 1;
}
int update_rids(char *input, uint64_t inputLength, int32_t rid_offset) {
	char *current_buffer = input;
	uint32_t remaining_buffer = inputLength;

	if (remaining_buffer < sizeof(uint32_t)) {
		return 0;
	}
	uint32_t data_size = *(const uint32_t *) current_buffer;

	current_buffer += sizeof(uint32_t);
	remaining_buffer -= sizeof(uint32_t);

	uint32_t remaining_data = data_size;
	while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
		PatchedFrameOfReference::Header header;
		if (!header.read(current_buffer)) {
			//cout << "here 3" << endl;
			LOG_WARNING_RETURN_FAIL("invalid buffer header")
			;
		}
		header.frame_of_reference_ += rid_offset;

		if (!header.write(current_buffer)) {
			LOG_WARNING_RETURN_FAIL("failed to write header")
			;
		}

		current_buffer += header.encoded_size_;
		remaining_buffer -= header.encoded_size_;
		remaining_data -= PatchedFrameOfReference::kBatchSize;
	}
	if (remaining_data > 0) {
		PatchedFrameOfReference::Header header;
		if (!header.read(current_buffer)) {
			LOG_WARNING_RETURN_FAIL("invalid buffer header")
			;
		}
		header.frame_of_reference_ += rid_offset;

		if (!header.write(current_buffer)) {
			LOG_WARNING_RETURN_FAIL("failed to write header")
			;
		}

		current_buffer += header.encoded_size_;
		remaining_buffer -= header.encoded_size_;
		remaining_data -= remaining_data;
	}
	if (remaining_buffer != 0) {
		//cout << "here4 : remaining_buffer = " << remaining_buffer << endl;
		return 0;
	}
	return 1;
}

int decode_rids_set_bmap(const char *input, uint64_t inputLength, void **bitmap) {
	bmap_t ** bmap = (bmap_t **) bitmap;
	uint32_t remaining_length = inputLength;
	uint32_t current_length = 0;
	/*uint32_t *current_data = output;
	 *outputCount = 0;*/
	while (remaining_length > 0) {
		const char *current_buffer = (input + current_length);

		uint32_t data_size = *(const uint32_t *) current_buffer;

		current_buffer += sizeof(uint32_t);
		current_length += sizeof(uint32_t);
		remaining_length -= sizeof(uint32_t);

		uint32_t remaining_data = data_size;
		//cout << "reached here" << endl;
		while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			if (!PatchedFrameOfReference::decode_every_batch(current_buffer,
					remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size, bmap)) {
				return 0;
			}
			if (actual_data_size != PatchedFrameOfReference::kBatchSize
					|| actual_significant_data_size
							!= PatchedFrameOfReference::kBatchSize) {
				return 0;
			}

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			/*	*outputCount += actual_significant_data_size;
			 current_data += PatchedFrameOfReference::kBatchSize;*/
			remaining_data -= PatchedFrameOfReference::kBatchSize;
		}
		if (remaining_data > 0) {
			uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			if (!PatchedFrameOfReference::decode_every_batch(current_buffer,
					remaining_length, actual_data_size,
					actual_significant_data_size, actual_buffer_size, bmap)) {
				return 0;
			}

			if (actual_data_size != PatchedFrameOfReference::kBatchSize
					|| actual_significant_data_size != remaining_data) {
			}
			//cout << "before memcpy" << endl;
			/*	memcpy(current_data, temp_data,
			 actual_significant_data_size * sizeof(uint32_t));*/

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			/*	*outputCount += actual_significant_data_size;
			 current_data += remaining_data;*/
			remaining_data -= remaining_data;
		}
	}
	if (remaining_length != 0) {
		return 0;
	}
	return 1;
}
int decode_rids(const char *input, uint64_t inputLength, uint32_t *output,
		uint32_t *outputCount) {

	uint32_t remaining_length = inputLength;
	uint32_t current_length = 0;
	uint32_t *current_data = output;
	*outputCount = 0;
	while (remaining_length > 0) {
		const char *current_buffer = (input + current_length);

		/*if (remaining_buffer < sizeof(uint32_t)) {
		 return 0;
		 }*/

		uint32_t data_size = *(const uint32_t *) current_buffer;

		current_buffer += sizeof(uint32_t);
		current_length += sizeof(uint32_t);
		remaining_length -= sizeof(uint32_t);

		uint32_t remaining_data = data_size;
		//cout << "reached here" << endl;
		while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			PatchedFrameOfReference::decode_new(current_buffer,
					remaining_length, current_data,
					PatchedFrameOfReference::kBatchSize, actual_data_size,
					actual_significant_data_size, actual_buffer_size);

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			*outputCount += actual_significant_data_size;
			current_data += PatchedFrameOfReference::kBatchSize;
			remaining_data -= PatchedFrameOfReference::kBatchSize;
		}
		//cout << "reached here too too" << endl;
		if (remaining_data > 0) {
			uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

			uint32_t actual_data_size;
			uint32_t actual_significant_data_size;
			uint32_t actual_buffer_size;

			PatchedFrameOfReference::decode_new(current_buffer,
					remaining_length, temp_data,
					PatchedFrameOfReference::kBatchSize, actual_data_size,
					actual_significant_data_size, actual_buffer_size);

			memcpy(current_data, temp_data,
					actual_significant_data_size * sizeof(uint32_t));

			current_buffer += actual_buffer_size;
			current_length += actual_buffer_size;
			remaining_length -= actual_buffer_size;

			*outputCount += actual_significant_data_size;
			current_data += remaining_data;
			remaining_data -= remaining_data;
		}
	}

	return 1;
}

uint32_t get_sufficient_buffer_capacity(uint32_t bin_length) {
	return PatchedFrameOfReference::get_sufficient_buffer_capacity(bin_length);
}

#ifdef __cplusplus
}
#endif
