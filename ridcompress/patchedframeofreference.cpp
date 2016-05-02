/*
 Patched Frame Of Reference algorithm
 Author : Saurabh V. Pendse
 */

#include "patchedframeofreference.h"
#include <iostream>
#include <cmath>
#include <iomanip>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#define release_assert assert

using namespace std;
using namespace pfor;

uint64_t PatchedFrameOfReference::EXP_SIZE[] = { 0 };
uint64_t PatchedFrameOfReference::B_TIMES[] = { 0 };
uint64_t PatchedFrameOfReference::H_BITS_EXP[] = { 0 };
bool PatchedFrameOfReference::TO_PRINT_SEQ = true;

uint64_t PRECALED2[64] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000,
		0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
		0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000, 0x20000000,
		0x40000000, 0x80000000, 0x100000000, 0x200000000, 0x400000000,
		0x800000000, 0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
		0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
		0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
		0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
		0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
		0x100000000000000, 0x200000000000000, 0x400000000000000,
		0x800000000000000, 0x1000000000000000, 0x2000000000000000,
		0x4000000000000000, 0x8000000000000000 };

static uint32_t single_set_bit(uint64_t value) {
	const uint64_t de_bruijn_multiplier = 0x022fdd63cc95386dull;
	static const uint32_t de_bruijn_table[64] = { 0, 1, 2, 53, 3, 7, 54, 27, 4,
			38, 41, 8, 34, 55, 48, 28, 62, 5, 39, 46, 44, 42, 22, 9, 24, 35,
			59, 56, 49, 18, 29, 11, 63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43,
			21, 23, 58, 17, 10, 51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15,
			30, 14, 13, 12, };

	return de_bruijn_table[(value * de_bruijn_multiplier) >> 58];
}

static uint32_t highest_set_bit(uint64_t value) {
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;

	value = value ^ (value >> 1);

	return single_set_bit(value);
}
bool ridConversion(uint64_t rid/*relative to local src selectoin*/, uint64_t *srcstart, uint64_t *srccount, uint64_t *deststart, uint64_t *destcount,
		int dim, uint64_t *relativeRid  );

uint64_t ridConversionWithoutChecking(uint64_t rid/*relative to local src selectoin*/,
		uint64_t *srcstart, uint64_t *srccount, uint64_t *deststart, uint64_t *destcount,
		int dim);


bool PatchedFrameOfReference::rle_decode_every_batch_to_selbox_withCheck(const void *buffer,
		uint32_t buffer_capacity, uint32_t &data_size,
		uint32_t &significant_data_size, uint32_t &buffer_size, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount, int dim, bmap_t **bmap) {
	data_size = 0;

	uint32_t effectDecodedSize = 0;

	//temporal data holder, mainly used for take the decompressed delta value.
	// This strategy is for memory efficiency purpose
	uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * kBatchSize);
	memset(data, 0, sizeof(uint32_t) * kBatchSize);

	uint32_t word; //tmp variable
	uint64_t rid; // NOTE: summary of all RIDs could exceed to uint32_t
	uint64_t newRid;

	if (buffer_capacity < kHeaderSize) {
		LOG_WARNING_RETURN_FAIL("invalid buffer size = %u \n", buffer_size)
		;
	}

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);

		rid = header.frame_of_reference_;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			if ( ridConversion(rid,  srcstart,srccount,deststart, destcount, dim, &newRid )  ){
				word = (uint32_t) (newRid >> 6);
				(*bmap)[word] |= PRECALED2[newRid & 0x3F];
				// increase decoded number
				effectDecodedSize ++;
			}
		}

	} else {

		if (header.fixed_length_ != 1) {
			fixed_length_decode((const char *) buffer + sizeof(uint64_t),
					PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
					header.fixed_length_, data, kBatchSize);

			patch_exceptions_new(header.exception_type_, data,
					header.first_exception_, header.significant_data_size_,
					(const char *) buffer + header.encoded_size_);

			rid = header.frame_of_reference_;
			for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				rid = rid + data[index];
				if ( ridConversion(rid,  srcstart,srccount,deststart, destcount, dim, &newRid )  ){
						word = (uint32_t) (newRid >> 6);
						(*bmap)[word] |= PRECALED2[newRid & 0x3F];
						// increase decoded number
						effectDecodedSize ++;
				}
			}
		} else { //decode for b =1 case

			decode_ones_zeros(buffer, &header, data); // data is recovered RIDs
			for(uint32_t i = 0; i < significant_data_size; i ++){
				if ( ridConversion(rid,  srcstart,srccount,deststart, destcount, dim, &newRid )  ){
					word = (uint32_t) (newRid >> 6);
					(*bmap)[word] |= PRECALED2[newRid & 0x3F];
					// increase decoded number
					effectDecodedSize ++;
				}
			}
		}

	}
	data_size = kBatchSize;
	significant_data_size =  effectDecodedSize;
	buffer_size = header.encoded_size_;
	free(data);
	return true;
}



bool PatchedFrameOfReference::rle_decode_every_batch_to_selbox_withoutCheck(const void *buffer,
		uint32_t buffer_capacity, uint32_t &data_size,
		uint32_t &significant_data_size, uint32_t &buffer_size, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount, int dim, bmap_t **bmap) {
	data_size = 0;

	//temporal data holder, mainly used for take the decompressed delta value.
	// This strategy is for memory efficiency purpose
	uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * kBatchSize);
	memset(data, 0, sizeof(uint32_t) * kBatchSize);

	uint32_t word; //tmp variable
	uint64_t rid; // NOTE: summary of all RIDs could exceed to uint32_t
	uint64_t newRid;

	if (buffer_capacity < kHeaderSize) {
		LOG_WARNING_RETURN_FAIL("invalid buffer size = %u \n", buffer_size)
		;
	}

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);

		rid = header.frame_of_reference_;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			newRid = ridConversionWithoutChecking(rid, srcstart,srccount,deststart, destcount, dim);
			word = (uint32_t) (newRid >> 6);
			(*bmap)[word] |= PRECALED2[newRid & 0x3F];
		}

	} else {

		if (header.fixed_length_ != 1) {
			fixed_length_decode((const char *) buffer + sizeof(uint64_t),
					PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
					header.fixed_length_, data, kBatchSize);

			patch_exceptions_new(header.exception_type_, data,
					header.first_exception_, header.significant_data_size_,
					(const char *) buffer + header.encoded_size_);

			rid = header.frame_of_reference_;
			for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				rid = rid + data[index];
				newRid = ridConversionWithoutChecking(rid, srcstart,srccount,deststart, destcount, dim);
				word = (uint32_t) (newRid >> 6);
				(*bmap)[word] |= PRECALED2[newRid & 0x3F];
			}
		} else { //decode for b =1 case

			decode_ones_zeros(buffer, &header, data); // data is recovered RIDs
			for(uint32_t i = 0; i < significant_data_size; i ++){
				newRid = ridConversionWithoutChecking(data[i], srcstart,srccount,deststart, destcount, dim);
				word = (uint32_t) (newRid >> 6);
				(*bmap)[word] |= PRECALED2[newRid & 0x3F];
			}
		}

	}
	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	free(data);
	return true;
}


bool PatchedFrameOfReference::rle_decode_every_batch_to_rids(const void *buffer,
		uint32_t buffer_capacity, uint32_t &data_size,
		uint32_t &significant_data_size, uint32_t &buffer_size, uint32_t *output /*it moves every batch*/) {
	data_size = 0;
	significant_data_size = 0;

	//temporal data holder, mainly used for take the decompressed delta value.
	// This strategy is for memory efficiency purpose
	uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * kBatchSize);
	memset(data, 0, sizeof(uint32_t) * kBatchSize);

	uint32_t word; //tmp variable
	uint64_t rid; // NOTE: summary of all RIDs could exceed to uint32_t

	if (buffer_capacity < kHeaderSize) {
		LOG_WARNING_RETURN_FAIL("invalid buffer size = %u \n", buffer_size)
		;
	}

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);

		rid = header.frame_of_reference_;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			output[index] = rid;
		}

	} else {

		if (header.fixed_length_ != 1) {
			fixed_length_decode((const char *) buffer + sizeof(uint64_t),
					PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
					header.fixed_length_, data, kBatchSize);

			patch_exceptions_new(header.exception_type_, data,
					header.first_exception_, header.significant_data_size_,
					(const char *) buffer + header.encoded_size_);

			rid = header.frame_of_reference_;
			for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				rid = rid + data[index];
				output[index] = rid;
			}
		} else { //decode for b =1 case

			decode_ones_zeros(buffer, &header, output);
		}

	}
	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	free(data);
	return true;
}


bool PatchedFrameOfReference::decode_ones_zeros(const void * buffer,
		Header * h, uint32_t *output) {
	uint32_t exception_count = h->first_exception_;// this filed is twisted
	uint32_t exception_value_size =
			get_exception_value_size(h->exception_type_);

	uint8_t * ones = (uint8_t *) buffer + sizeof(uint64_t) + (exception_count
			- 1) * exception_value_size;

	switch (h->exception_type_) {
	case EXCEPTION_UNSIGNED_CHAR:
		return decode_ones_zeros_typed<unsigned char> (ones, h,
				(unsigned char *) ((const char *) buffer + sizeof(uint64_t)),
				output);

	case EXCEPTION_SIGNED_CHAR:
		return decode_ones_zeros_typed<signed char> (ones, h,
				(signed char *) ((const char *) buffer + sizeof(uint64_t)),
				output);

	case EXCEPTION_UNSIGNED_SHORT:
		return decode_ones_zeros_typed<unsigned short> (ones, h,
				(unsigned short *) ((const char *) buffer + sizeof(uint64_t)),
				output);

	case EXCEPTION_SIGNED_SHORT:
		return decode_ones_zeros_typed<signed short> (ones, h,
				(signed short *) ((const char *) buffer + sizeof(uint64_t)),
				output);

	case EXCEPTION_UNSIGNED_INT:
		return decode_ones_zeros_typed<unsigned int> (ones, h,
				(unsigned int *) ((const char *) buffer + sizeof(uint64_t)),
				output);

	case EXCEPTION_SIGNED_INT:
		return decode_ones_zeros_typed<signed int> (ones, h,
				(signed int *) ((const char *) buffer + sizeof(uint64_t)),
				output);

	default:
		LOG_WARNING_RETURN_FAIL("unknown exception type: ", (int)h->exception_type_)
		;
	}

}


template<class ExceptionValueType>
bool PatchedFrameOfReference::decode_ones_zeros_typed(
		const uint8_t *ones, Header *h, const ExceptionValueType *exceptions, uint32_t *output) {
	uint32_t exception_count = h->first_exception_;// this filed is twisted
	uint32_t rid = h->frame_of_reference_; //the skip rid ( first zero)
	int index = 0, tmp = 0;;
	for (int i = 0; i < exception_count; i++) {
		// create RIDs for ones
		tmp = 0;
		while (tmp < ones[i]){
			rid = rid +1; // increase 1 every time
			output[index] = rid;
			tmp ++ ;
			index ++;
		}
		rid += exceptions[i]; // rest zeros
	}

}


int coordinateConversion(int * coordinates, const  int dim, const  uint64_t *srcstart, const  uint64_t *deststart, const  uint64_t *destend){

	// change to global coordinate
	for (int i = 0; i < dim; i++) {
		coordinates[i] += (srcstart[i] /*global coordinate*/ );
		if ( coordinates[i] < deststart[i] || coordinates[i] > destend[i]){
//			printf("new coordinate is not in the new selection box in dimension %d \n ", i+1);
			return (i+1) * -1;
		}
	}

	/*change coordinate to the destination box*/
	for (int i = 0; i < dim; i++) {
		coordinates[i] -=  deststart[i];
	}
	return 1;


}



/* Give a rid that is relative to a src region
 * return a rid that is relative to dest selection box
 * Assume all the start & count array has slowest dimension at first position
 *
 * NOTE: ***************** NOT USED ANY MORE ******************************
 */
uint64_t ridConversionWithoutChecking(uint64_t rid/*relative to local src selectoin*/,
		uint64_t *srcstart, uint64_t *srccount, uint64_t *deststart, uint64_t *destcount,
		int dim){

	uint64_t relativeRid = 0;
	int * coordinates = (int *) malloc(sizeof(int) * dim); // coordinate of current PG
	uint64_t * destend = (uint64_t*) malloc(sizeof(uint64_t)*dim); // coordinate of ending points on the destination box
	for(int i = 0; i < dim; i ++){
		destend[i] = deststart[i] + destcount[i] -1;
	}
	if (dim == 3) {
		coordinates[0] = rid / (srccount[1] * srccount[2]);
		coordinates[1] = (rid % (srccount[1] * srccount[2])) / srccount[2];
		coordinates[2] = (rid % (srccount[1] * srccount[2])) % srccount[2] ;
		relativeRid = coordinates[2] + coordinates[1] * destcount[2] + coordinates[0]* destcount[1] * destcount[2];
	}

	if (dim == 2){
		coordinates[0] = rid / (srccount[1]);
		coordinates[1] = rid % (srccount[1] );
		relativeRid = coordinates[1] + coordinates[0] * destcount[1] ;

	}

	free(destend);
	free(coordinates);
	return relativeRid;
}

/* Give a rid that is relative to a src region
 * return a rid that is relative to dest selection box
 * Assume all the start & count array has slowest dimension at first position
 */
bool ridConversion(uint64_t rid/*relative to local src selectoin*/, uint64_t *srcstart, uint64_t *srccount, uint64_t *deststart, uint64_t *destcount,
		int dim, uint64_t *relativeRid  ){

	*relativeRid = 0;
	int * coordinates = (int *) malloc(sizeof(int) * dim); // coordinate of current PG
	uint64_t * destend = (uint64_t*) malloc(sizeof(uint64_t)*dim); // coordinate of ending points on the destination box
	for(int i = 0; i < dim; i ++){
		destend[i] = deststart[i] + destcount[i] -1;
	}
		if (dim == 3) {
			coordinates[0] = rid / (srccount[1] * srccount[2]);
			coordinates[1] = (rid % (srccount[1] * srccount[2])) / srccount[2];
			coordinates[2] = (rid % (srccount[1] * srccount[2])) % srccount[2] ;

			if (coordinateConversion(coordinates, dim, srcstart, deststart, destend) < 0){
				free(coordinates);
				free(destend);
				return false;
			}

			*relativeRid = coordinates[2] + coordinates[1] * destcount[2] + coordinates[0]* destcount[1] * destcount[2];

		}

		if (dim == 2){

			coordinates[0] = rid / (srccount[1]);
			coordinates[1] = rid % (srccount[1] );

			if (coordinateConversion(coordinates, dim, srcstart, deststart, destend) < 0){
				free(coordinates);
				free(destend);
				return false;
			}

			*relativeRid = coordinates[1] + coordinates[0] * destcount[1] ;

		}

	free(destend);
	free(coordinates);
	return true;
}





bool PatchedFrameOfReference::batch_decode_within_selbox_without_checking(const void *buffer,
		uint32_t buffer_capacity, uint32_t &data_size,
		uint32_t &significant_data_size, uint32_t &buffer_size, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount, int dim, bmap_t **bmap) {

	data_size = 0;
	significant_data_size = 0;
	uint32_t word; //tmp variable
	uint64_t rid; // NOTE: summary of all RIDs could exceed to uint32_t
	uint64_t newRid;

	//temporal data holder, mainly used for take the decompressed delta value.
	// This strategy is for memory efficiency purpose
	uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * kBatchSize);
	memset(data, 0, sizeof(uint32_t) * kBatchSize);

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);

	} else {

		fixed_length_decode((const char *) buffer + sizeof(uint64_t),
				PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
				header.fixed_length_, data, kBatchSize);

		patch_exceptions_new(header.exception_type_, data,
				header.first_exception_, header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);
	}

	rid = header.frame_of_reference_;
    // for performance reason
	if ( dim == 1 ){
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				rid = rid + data[index];
				newRid = rid + srcstart[0] - deststart[0];
#ifdef RIDBUG
		    cout << rid << "->" << newRid  <<",";
//			printf("%"PRIu32"->%"PRIu32",", rid, newRid);
#endif
				word = (uint32_t) (newRid >> 6);
				(*bmap)[word] |= PRECALED2[newRid & 0x3F];
		}
	}else if (dim ==2 ){
		int coordinates[2];
		uint64_t destend[2];
		destend[0] = deststart[0] + destcount[0] -1;
		destend[1] = deststart[1] + destcount[1] -1;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			coordinates[0] = rid / (srccount[1]);
			coordinates[1] = rid % (srccount[1] );

			coordinates[0] += (srcstart[0] - deststart[0] /*global coordinate*/ );
			coordinates[1] += (srcstart[1] - deststart[1] /*global coordinate*/ );


			newRid = coordinates[1] + coordinates[0] * destcount[1] ;

#ifdef RIDBUG
		    cout << rid << "->" << newRid  <<",";
//			printf("%"PRIu32"->%"PRIu32",", rid, newRid);
#endif

			word = (uint32_t) (newRid >> 6);
			(*bmap)[word] |= PRECALED2[newRid & 0x3F];
		}
	}else if ( dim == 3 ){

		int coordinates[3];
		uint64_t destend[3];
		destend[0] = deststart[0] + destcount[0] -1;
		destend[1] = deststart[1] + destcount[1] -1;
		destend[2] = deststart[2] + destcount[2] -1;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				rid = rid + data[index];
				coordinates[0] = rid / (srccount[1] * srccount[2]);
				coordinates[1] = (rid % (srccount[1] * srccount[2])) / srccount[2];
				coordinates[2] = (rid % (srccount[1] * srccount[2])) % srccount[2] ;


				coordinates[0] += (srcstart[0] - deststart[0] /*global coordinate*/ );
				coordinates[1] += (srcstart[1] - deststart[1] /*global coordinate*/ );
				coordinates[2] += (srcstart[2] - deststart[2] /*global coordinate*/ );


				newRid = coordinates[2] + coordinates[1] * destcount[2] + coordinates[0]* destcount[1] * destcount[2];

#ifdef RIDBUG
			    cout << rid << "->" << newRid  <<"," ;
//			printf("%"PRIu32"->%"PRIu32",", rid, newRid);
#endif
				word = (uint32_t) (newRid >> 6);
				(*bmap)[word] |= PRECALED2[newRid & 0x3F];
		}
	}else if (dim >= 4){

		uint64_t * coordinates = (uint64_t *) malloc(sizeof(uint64_t) * dim);

		int i=0, j = 0, k;
		uint64_t tmpSize;
		uint64_t remain;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				rid = rid + data[index];

				// rid to coordinates in the src region
				j = 0;
				tmpSize = 1;
				remain = rid;
				while ( j < dim){ // j is the dimension to been set
					k = j + 1;
					tmpSize = 1;
					while ( k < dim){
						tmpSize *= srccount[k];
						k++;
					}
					coordinates[j] = remain / tmpSize ;
					remain = remain  %  tmpSize;
					j ++;
				}

				for(i = 0; i < dim ; i ++){
					coordinates[i] += (srcstart[i] - deststart[i]);
				}

				//coordinates to rid in the dest region
				tmpSize = 1;
				newRid = 0;
				for (i = 0; i < dim; i ++){
					tmpSize = coordinates[i];
					for ( j = i + 1; j < dim ; j ++ ){
						tmpSize = tmpSize * destcount[j];
					}
					newRid = newRid + tmpSize;
				}

#ifdef RIDBUG
			    cout << rid << "->" << newRid <<",";
//			printf("%"PRIu32"->%"PRIu32",", rid, newRid);
#endif

				word = (uint32_t) (newRid >> 6);
				(*bmap)[word] |= PRECALED2[newRid & 0x3F];
		}
		free(coordinates);


	}

	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	free(data);
	return true;

}

/*
 * "significant_data_size" is used to keep the actual decoded data
 */
bool PatchedFrameOfReference::batch_decode_within_selbox_with_checking(const void *buffer,
		uint32_t buffer_capacity, uint32_t &data_size,
		uint32_t &significant_data_size, uint32_t &buffer_size, uint64_t *srcstart //PG region dimension
		, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
		, uint64_t *destcount, int dim, bmap_t **bmap) {

	data_size = 0;
	significant_data_size = 0;
	uint32_t word; //tmp variable
	uint64_t rid; // NOTE: summary of all RIDs could exceed to uint32_t
	uint64_t newRid;

	//temporal data holder, mainly used for take the decompressed delta value.
	// This strategy is for memory efficiency purpose
	uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * kBatchSize);
	memset(data, 0, sizeof(uint32_t) * kBatchSize);

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);
	} else {

		fixed_length_decode((const char *) buffer + sizeof(uint64_t),
				PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
				header.fixed_length_, data, kBatchSize);

		patch_exceptions_new(header.exception_type_, data,
				header.first_exception_, header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);
	}
	rid = header.frame_of_reference_;
	if ( dim == 1 ){
		uint64_t coordinates, destend;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];

			destend = deststart[0] + destcount[0] -1 ;
			coordinates =  rid + srcstart[0]; /*global coordinate*/
			if (coordinates < deststart[0] || coordinates > destend ){
				continue ;
			}
			newRid = coordinates - deststart[0];
			word = (uint32_t) (newRid >> 6);
			(*bmap)[word] |= PRECALED2[newRid & 0x3F];
			// increase decoded number
			significant_data_size ++;
		}

	}else if (dim == 2) {

		uint64_t coordinates[2]= {0}, destend[2]={0};
		destend[0] = deststart[0] + destcount[0] -1 ;
		destend[1] = deststart[1] + destcount[1] -1 ;

		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];

			coordinates[0] = rid / (srccount[1]);
			coordinates[1] = rid % (srccount[1] );

			coordinates[0] += (srcstart[0] /*global coordinate*/ );
			if ( coordinates[0] < deststart[0] || coordinates[0] > destend[0])
				continue;

			coordinates[1] += (srcstart[1] /*global coordinate*/ );
			if ( coordinates[1] < deststart[1] || coordinates[1] > destend[1])
				continue;

			/*change coordinate to the destination box*/
			coordinates[0] -=  deststart[0];
			coordinates[1] -=  deststart[1];
			newRid= coordinates[1] + coordinates[0] * destcount[1] ;
			word = (uint32_t) (newRid >> 6);
				(*bmap)[word] |= PRECALED2[newRid & 0x3F];
				// increase decoded number
				significant_data_size ++;
		}

	}else if (dim == 3) {

		uint64_t coordinates[3]= {0}, destend[3]={0};
		destend[0] = deststart[0] + destcount[0] -1 ;
		destend[1] = deststart[1] + destcount[1] -1 ;
		destend[2] = deststart[2] + destcount[2] -1 ;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];

			coordinates[0] = rid / (srccount[1] * srccount[2]) ;
			coordinates[1] = (rid % (srccount[1] * srccount[2])) / srccount[2];
			coordinates[2] = (rid % (srccount[1] * srccount[2])) % srccount[2] ;

			coordinates[0] += (srcstart[0] /*global coordinate*/ );
			if ( coordinates[0] < deststart[0] || coordinates[0] > destend[0])
				return false;

			coordinates[1] += (srcstart[1] /*global coordinate*/ );
			if ( coordinates[1] < deststart[1] || coordinates[1] > destend[1])
				return false;

			coordinates[2] += (srcstart[2] /*global coordinate*/ );
			if ( coordinates[2] < deststart[2] || coordinates[2] > destend[2])
				return false;

			/*change coordinate to the destination box*/
			coordinates[0] -=  deststart[0];
			coordinates[1] -=  deststart[1];
			coordinates[2] -=  deststart[2];
			newRid = coordinates[2] + coordinates[1] * destcount[2] + coordinates[0]* destcount[1] * destcount[2];
			word = (uint32_t) (newRid >> 6);
				(*bmap)[word] |= PRECALED2[newRid & 0x3F];
				// increase decoded number
				significant_data_size ++;
		}

	}else if (dim >= 4){
		int i = 0;
		uint64_t * coordinates = (uint64_t *) malloc(sizeof(uint64_t) * dim);
		uint64_t * destend = (uint64_t *) malloc(sizeof(uint64_t) * dim);
		for(i=0; i < dim ; i ++){
			destend[i] = deststart[i] + destcount[i] -1;
		}

		//calculate coordinates
		int j = 0, k;
		uint64_t tmpSize = 1;

		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];

			j = 0;
			tmpSize = 1;
			uint64_t remain = rid;
			while ( j < dim){ // j is the dimension to been set
				k = j + 1;
				tmpSize = 1;
				while ( k < dim){
					tmpSize *= srccount[k];
					k++;
				}
				coordinates[j] = remain / tmpSize ;
				remain = remain  %  tmpSize;
				j ++;
			}

			bool outBoundary = false ;
			for (i = 0; i < dim; i++) {
				coordinates[i] += (srcstart[i] /*global coordinate*/ );
				if ( coordinates[i] < deststart[i] || coordinates[i] > destend[i]){
					outBoundary = true;
					continue;
				}
			}
			if (outBoundary)  continue ;

			/*change coordinate to the destination box*/
			for (i = 0; i < dim; i++) {
				coordinates[i] -=  deststart[i];
			}

			newRid = 0;
			for (i = 0; i < dim; i ++){
				tmpSize = coordinates[i];
				for ( j = i + 1; j < dim ; j ++ ){
					tmpSize = tmpSize * destcount[j];
				}
				newRid = newRid + tmpSize;
			}

			word = (uint32_t) (newRid >> 6);
			(*bmap)[word] |= PRECALED2[newRid & 0x3F];
			// increase decoded number
			significant_data_size ++;
		}
		free(coordinates);
		free(destend);
	}



	data_size = kBatchSize;
	buffer_size = header.encoded_size_;
	free(data);
	return true;

}

/*
 * Expansion + runlength is combination of PFD, in which the
 * b=1 case, it uses Run length encoding, whereas, the b ~= 1 case, it uses expansion method
 */
bool PatchedFrameOfReference::expand_runlength_encode(const uint32_t *data,
		uint32_t data_size, uint32_t significant_data_size, void *buffer,
		uint32_t buffer_capacity, uint32_t &buffer_size, bool verify) {

	uint32_t fixed_length;
	uint32_t frame_of_reference;
	ExceptionType exception_type;

	uint32_t deltas[PatchedFrameOfReference::kBatchSize] = { 0 };
	deltas[0] = 0;
	for (uint32_t index = 1; index < significant_data_size; ++index) {
		deltas[index] = data[index] - data[index - 1];
	}
	for (uint32_t index = significant_data_size; index < kBatchSize; ++index) {
		deltas[index] = 0;
	}

	frame_of_reference = 0;
	if (data_size != kBatchSize) {
		LOG_WARNING_RETURN_FAIL("data size not ", kBatchSize, ": ", data_size)
		;
	}

	if (significant_data_size > data_size) {
		LOG_WARNING_RETURN_FAIL("invalid significant data size: ", significant_data_size)
		;
	}

	uint32_t length_counts[32];

	for (uint32_t length = 0; length < 32; ++length) {
		length_counts[length] = 0;
	}

	uint32_t mask = 0;

	for (uint32_t idx = 0; idx < kBatchSize; ++idx) {
		++length_counts[highest_set_bit(deltas[idx] | 1)];
		mask |= deltas[idx];
	}

	uint32_t mask_bits = highest_set_bit(mask | 1);

	uint32_t exception_value_size;

	if (mask_bits < 8) {
		exception_type = EXCEPTION_UNSIGNED_CHAR;
		exception_value_size = 1;
	} else if (mask_bits < 16) {
		exception_type = EXCEPTION_UNSIGNED_SHORT;
		exception_value_size = 2;
	} else {
		exception_type = EXCEPTION_UNSIGNED_INT;
		exception_value_size = 4;
	}

	uint32_t exception_counts[32];

	exception_counts[31] = 0;

	for (uint32_t length = 31; length > 0; --length) {
		exception_counts[length - 1] = length_counts[length]
				+ exception_counts[length];
	}

	uint32_t best_length;
	uint32_t best_buffer_size;
	uint32_t temp_best_buffer_size;
	uint32_t temp_best_length;

	if (significant_data_size < kSufficientBufferCapacity) {
		best_length = 0;
		best_buffer_size = sizeof(uint64_t) + (significant_data_size
				* exception_value_size + sizeof(uint64_t) - 1)
				/ sizeof(uint64_t) * sizeof(uint64_t);
	} else {
		best_length = 32;
		best_buffer_size = kSufficientBufferCapacity;
	}
	for (uint32_t length = 0; length < 32; ++length) {
		uint32_t buffer_size = required_buffer_size(length + 1,
				exception_counts[length], exception_value_size);
		if (buffer_size < best_buffer_size) {
			best_buffer_size = buffer_size;
			best_length = length + 1;
		}
	}

	fixed_length = best_length;

	frame_of_reference = data[0];

	if (fixed_length == 1) { // particular encoding for b = 1
		if (!encode_on_b_equal_one(deltas, data_size, significant_data_size,
				fixed_length, frame_of_reference, exception_type, buffer,
				buffer_capacity, buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to encode with optimal parameters: ", fixed_length, " ", frame_of_reference)
			;
		}

	} else {

		/*
		 * ORIGNAL EPFD , remove all exception value or not
		 * expand the `bit width`, threshold is 1.6
		 */
		float threshold = 1.6;
		for (uint32_t length = (fixed_length + 1); length < 32; ++length) {
			if (exception_counts[length] == 0) {
				uint32_t expand_buf_size = required_buffer_size(length + 1,
						exception_counts[length], exception_value_size);
				if (expand_buf_size < best_buffer_size * threshold) {
					fixed_length = length + 1;
					break;
				}

			}
		}

		if (!encode_param_new(deltas, data_size, significant_data_size,
				fixed_length, frame_of_reference, exception_type, buffer,
				buffer_capacity, buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to encode with optimal parameters: ", fixed_length, " ", frame_of_reference)
			;
		}
	}

	return true;

}


bool PatchedFrameOfReference::optimal_param(const uint32_t *data,
		uint32_t data_size, uint32_t significant_data_size,
		uint32_t &fixed_length, uint32_t &frame_of_reference,
		ExceptionType &exception_type) {

	frame_of_reference = 0;
	if (data_size != kBatchSize) {
		LOG_WARNING_RETURN_FAIL("data size not ", kBatchSize, ": ", data_size)
		;
	}

	if (significant_data_size > data_size) {
		LOG_WARNING_RETURN_FAIL("invalid significant data size: ", significant_data_size)
		;
	}

	uint32_t length_counts[32];

	for (uint32_t length = 0; length < 32; ++length) {
		length_counts[length] = 0;
	}

	uint32_t mask = 0;

	for (uint32_t idx = 0; idx < kBatchSize; ++idx) {
		++length_counts[highest_set_bit(data[idx] | 1)];
		mask |= data[idx];
	}

	uint32_t mask_bits = highest_set_bit(mask | 1);

	uint32_t exception_value_size;

	if (mask_bits < 8) {
		exception_type = EXCEPTION_UNSIGNED_CHAR;
		exception_value_size = 1;
	} else if (mask_bits < 16) {
		exception_type = EXCEPTION_UNSIGNED_SHORT;
		exception_value_size = 2;
	} else {
		exception_type = EXCEPTION_UNSIGNED_INT;
		exception_value_size = 4;
	}

	uint32_t exception_counts[32];

	exception_counts[31] = 0;

	for (uint32_t length = 31; length > 0; --length) {
		exception_counts[length - 1] = length_counts[length]
				+ exception_counts[length];
	}

	uint32_t best_length;
	uint32_t best_buffer_size;
	uint32_t temp_best_buffer_size;
	uint32_t temp_best_length;

	if (significant_data_size < kSufficientBufferCapacity) {
		best_length = 0;
		best_buffer_size = sizeof(uint64_t) + (significant_data_size
				* exception_value_size + sizeof(uint64_t) - 1)
				/ sizeof(uint64_t) * sizeof(uint64_t);
	} else {
		best_length = 32;
		best_buffer_size = kSufficientBufferCapacity;
	}
	for (uint32_t length = 0; length < 32; ++length) {
		uint32_t buffer_size = required_buffer_size(length + 1,
				exception_counts[length], exception_value_size);
		if (buffer_size < best_buffer_size) {
			best_buffer_size = buffer_size;
			best_length = length + 1;
		}
	}

	fixed_length = best_length;

	return true;
}

/*
 * copy from optimal_param function
 * it determine b and meanwhile, if satisfying condition, it expands b to b+n
 */
bool PatchedFrameOfReference::determine_b_to_expand(const uint32_t *data,
		uint32_t data_size, uint32_t significant_data_size,
		uint32_t &fixed_length, uint32_t &frame_of_reference,
		ExceptionType &exception_type) {

	frame_of_reference = 0;
	if (data_size != kBatchSize) {
		LOG_WARNING_RETURN_FAIL("data size not ", kBatchSize, ": ", data_size)
		;
	}

	if (significant_data_size > data_size) {
		LOG_WARNING_RETURN_FAIL("invalid significant data size: ", significant_data_size)
		;
	}

	uint32_t length_counts[32];

	for (uint32_t length = 0; length < 32; ++length) {
		length_counts[length] = 0;
	}

	uint32_t mask = 0;

	for (uint32_t idx = 0; idx < kBatchSize; ++idx) {
		++length_counts[highest_set_bit(data[idx] | 1)];
		mask |= data[idx];
	}

	uint32_t mask_bits = highest_set_bit(mask | 1);

	uint32_t exception_value_size;

	if (mask_bits < 8) {
		exception_type = EXCEPTION_UNSIGNED_CHAR;
		exception_value_size = 1;
	} else if (mask_bits < 16) {
		exception_type = EXCEPTION_UNSIGNED_SHORT;
		exception_value_size = 2;
	} else {
		exception_type = EXCEPTION_UNSIGNED_INT;
		exception_value_size = 4;
	}

	uint32_t exception_counts[32];

	exception_counts[31] = 0;

	for (uint32_t length = 31; length > 0; --length) {
		exception_counts[length - 1] = length_counts[length]
				+ exception_counts[length];
	}

	uint32_t best_length;
	uint32_t best_buffer_size;
	uint32_t temp_best_buffer_size;
	uint32_t temp_best_length;

	if (significant_data_size < kSufficientBufferCapacity) {
		best_length = 0;
		best_buffer_size = sizeof(uint64_t) + (significant_data_size
				* exception_value_size + sizeof(uint64_t) - 1)
				/ sizeof(uint64_t) * sizeof(uint64_t);
	} else {
		best_length = 32;
		best_buffer_size = kSufficientBufferCapacity;
	}

	for (uint32_t length = 0; length < 32; ++length) {
		uint32_t buffer_size = required_buffer_size(length + 1,
				exception_counts[length], exception_value_size);
		if (buffer_size < best_buffer_size) {
			best_buffer_size = buffer_size;
			best_length = length + 1;
		}

	}

	fixed_length = best_length;

	/*
	 * ORIGNAL EPFD , remove all exception value or not
	 */

	for (uint32_t length = (fixed_length + 1); length < 32; ++length) {
		if (exception_counts[length] == 0) {
			uint32_t expand_buf_size = required_buffer_size(length + 1,
					exception_counts[length], exception_value_size);
			if (expand_buf_size < best_buffer_size * 1.6) {
				fixed_length = length + 1;
				return true;
			}

		}
	}


	return true;
}

bool PatchedFrameOfReference::expand_decode_every_batch(const void *buffer,
		uint32_t buffer_capacity, uint32_t &data_size,
		uint32_t &significant_data_size, uint32_t &buffer_size, bmap_t **bmap) {

	data_size = 0;
	significant_data_size = 0;
	uint32_t word; //tmp variable
	uint64_t rid; // NOTE: summary of all RIDs could exceed to uint32_t

	//temporal data holder, mainly used for take the decompressed delta value.
	// This strategy is for memory efficiency purpose
	uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * kBatchSize);
	//	memset(data, 0, sizeof(uint32_t)*kBatchSize);


	Header header;
	header.read(buffer);


	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);


		rid = header.frame_of_reference_;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			word = (uint32_t) (rid >> 6);
			(*bmap)[word] |= PRECALED2[rid & 0x3F];
		}

	} else {
		fixed_length_decode((const char *) buffer + sizeof(uint64_t),
				PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
				header.fixed_length_, data, kBatchSize);
		patch_exceptions_new(header.exception_type_, data,
				header.first_exception_, header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);


		rid = header.frame_of_reference_;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			word = (uint32_t) (rid >> 6);
			(*bmap)[word] |= PRECALED2[rid & 0x3F];
		}

	}
	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	free(data);
	return true;
}

bool PatchedFrameOfReference::rle_decode_every_batch(const void *buffer,
		uint32_t buffer_capacity, uint32_t &data_size,
		uint32_t &significant_data_size, uint32_t &buffer_size, bmap_t **bmap) {
	data_size = 0;
	significant_data_size = 0;

	//temporal data holder, mainly used for take the decompressed delta value.
	// This strategy is for memory efficiency purpose
	uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * kBatchSize);
	memset(data, 0, sizeof(uint32_t) * kBatchSize);

	uint32_t word; //tmp variable
	uint64_t rid; // NOTE: summary of all RIDs could exceed to uint32_t

	if (buffer_capacity < kHeaderSize) {
		LOG_WARNING_RETURN_FAIL("invalid buffer size = %u \n", buffer_size)
		;
	}

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);


		rid = header.frame_of_reference_;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			word = (uint32_t) (rid >> 6);
			(*bmap)[word] |= PRECALED2[rid & 0x3F];
		}

	} else {

		if (header.fixed_length_ != 1) {
			fixed_length_decode((const char *) buffer + sizeof(uint64_t),
					PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
					header.fixed_length_, data, kBatchSize);


			patch_exceptions_new(header.exception_type_, data,
					header.first_exception_, header.significant_data_size_,
					(const char *) buffer + header.encoded_size_);


			rid = header.frame_of_reference_;
			for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				rid = rid + data[index];
				word = (uint32_t) (rid >> 6);
				(*bmap)[word] |= PRECALED2[rid & 0x3F];
			}
		} else { //decode for b =1 case

			expand_exceptions_binary(buffer, &header, bmap);
		}

	}
	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	free(data);
	return true;
}

template<class ExceptionValueType>
bool PatchedFrameOfReference::expand_exceptions_binary_typed(
		const uint8_t *ones, Header *h, const ExceptionValueType *exceptions,
		bmap_t **bmap) {
	uint32_t exception_count = h->first_exception_;// this filed is twisted
	// this is important, as the value of first exception could be 0
	// and the consecutive 1s includes 1 bit from exception value,
	// in addition, bit position start from 0, for exception value (say, 10),
	// we skip 9 bits and set 10th bit as 1 which is included into consecutive 1s list
	uint32_t first_exception = h->frame_of_reference_;

	bmap_t * tmp_bitmap = *bmap;
	uint32_t bitptr = first_exception; // skip over # of bit
	uint32_t slot, remainder, bitspan, pass_bits;

	uint64_t ones_mask = (uint64_t) (~0); // must have casting, otherwise, will only generate 32 1s
	//	printf(" decode : exp_num [%u ], ones_zeros: ", exception_count );
	for (int i = 0; i < exception_count; i++) {
		slot = (uint32_t) (bitptr >> 6);
		remainder = bitptr & 0x3F;
		pass_bits = remainder; // remainder = rid_val % 64, it actually corresponds (remainder) bits,
		bitspan = pass_bits + ones[i]; // number of bits ( sum of # of remainder bits and # of 1s bit
		bitptr += ones[i];
		//			printf ("%u ,", ones[i]);

		//PAY ATTENTION, MOST machines now operate on little endian, except blue gene
		// Little endian store the bits in a reverse order of the human reading order
		if (bitspan <= 64) { // one slot
			/*************LITTLE ENDIAN : Significant bit goes to high address (right side)*******/
			/*                     0000| 1111111   |xxxxxxxxxx           */
			/* left (64-remainder-ones | ones      | remainder     |     */
			/********************/
			/**** remainder_mask = |0000...000|11111111..1111  */
			/******************               |remainder |         ******/
			uint64_t remainder_mask = ones_mask << pass_bits;
			/**** left_mask =    00000   |1111...11111|   */
			/*                            64-remainder-ones*/
			uint64_t left_mask = ones_mask >> (64 - pass_bits - ones[i]);
			tmp_bitmap[slot] |= (remainder_mask & left_mask);
		} else if (bitspan <= 128) { // two slots
			// ** little
			tmp_bitmap[slot] |= (ones_mask << pass_bits); /**first slot, from remainder to 64, all 1s**/
			tmp_bitmap[slot + 1] |= (ones_mask >> (128 - bitspan));
		} else { // three slots
			tmp_bitmap[slot] |= (ones_mask << pass_bits); // same as 2nd case
			tmp_bitmap[slot + 1] |= ones_mask; // all ones
			tmp_bitmap[slot + 2] |= (ones_mask >> (192 - bitspan));
		}
		if (i < exception_count - 1) { // first exception had been handled outside of for loop
			// the exceptions list only has exception_count -1 number of exceptions
			bitptr += exceptions[i];
		}
	}

}

bool PatchedFrameOfReference::expand_exceptions_binary(const void * buffer,
		Header * h, bmap_t **bitmap) {
	uint32_t exception_count = h->first_exception_;// this filed is twisted
	uint32_t exception_value_size =
			get_exception_value_size(h->exception_type_);

	uint8_t * ones = (uint8_t *) buffer + sizeof(uint64_t) + (exception_count
			- 1) * exception_value_size;

	switch (h->exception_type_) {
	case EXCEPTION_UNSIGNED_CHAR:
		return expand_exceptions_binary_typed<unsigned char> (ones, h,
				(unsigned char *) ((const char *) buffer + sizeof(uint64_t)),
				bitmap);

	case EXCEPTION_SIGNED_CHAR:
		return expand_exceptions_binary_typed<signed char> (ones, h,
				(signed char *) ((const char *) buffer + sizeof(uint64_t)),
				bitmap);

	case EXCEPTION_UNSIGNED_SHORT:
		return expand_exceptions_binary_typed<unsigned short> (ones, h,
				(unsigned short *) ((const char *) buffer + sizeof(uint64_t)),
				bitmap);

	case EXCEPTION_SIGNED_SHORT:
		return expand_exceptions_binary_typed<signed short> (ones, h,
				(signed short *) ((const char *) buffer + sizeof(uint64_t)),
				bitmap);

	case EXCEPTION_UNSIGNED_INT:
		return expand_exceptions_binary_typed<unsigned int> (ones, h,
				(unsigned int *) ((const char *) buffer + sizeof(uint64_t)),
				bitmap);

	case EXCEPTION_SIGNED_INT:
		return expand_exceptions_binary_typed<signed int> (ones, h,
				(signed int *) ((const char *) buffer + sizeof(uint64_t)),
				bitmap);

	default:
		LOG_WARNING_RETURN_FAIL("unknown exception type: ", (int)h->exception_type_)
		;
	}

}

/*
 * hybrid_encode is same as `encode` function, exception it calls
 * encode_on_b_equal_one function instead of encode_param_new
 */
bool PatchedFrameOfReference::hybrid_encode(const uint32_t *data,
		uint32_t data_size, uint32_t significant_data_size, void *buffer,
		uint32_t buffer_capacity, uint32_t &buffer_size, bool verify) {
	uint32_t fixed_length;
	uint32_t frame_of_reference;
	ExceptionType exception_type;

	uint32_t deltas[PatchedFrameOfReference::kBatchSize] = { 0 };
	deltas[0] = 0;
	for (uint32_t index = 1; index < significant_data_size; ++index) {
		deltas[index] = data[index] - data[index - 1];
	}
	for (uint32_t index = significant_data_size; index < kBatchSize; ++index) {
		deltas[index] = 0;
	}

	if (!optimal_param(deltas, data_size, significant_data_size, fixed_length,
			frame_of_reference, exception_type)) {
		LOG_WARNING_RETURN_FAIL("failed to find optimal parameters")
		;
	}
	frame_of_reference = data[0];

	if (fixed_length == 1) { // particular encoding for b = 1
		if (!encode_on_b_equal_one(deltas, data_size, significant_data_size,
				fixed_length, frame_of_reference, exception_type, buffer,
				buffer_capacity, buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to encode with optimal parameters: ", fixed_length, " ", frame_of_reference)
			;
		}

	} else {
		if (!encode_param_new(deltas, data_size, significant_data_size,
				fixed_length, frame_of_reference, exception_type, buffer,
				buffer_capacity, buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to encode with optimal parameters: ", fixed_length, " ", frame_of_reference)
			;
		}
	}

	return true;
}

/*
 * b =1 , encode the binary string as the (# of 0s, # of 1s)
 */
bool PatchedFrameOfReference::encode_on_b_equal_one(const uint32_t *deltas,
		uint32_t data_size, uint32_t significant_data_size,
		uint32_t fixed_length, uint32_t frame_of_reference,
		ExceptionType exception_type, void *buffer, uint32_t buffer_capacity,
		uint32_t &buffer_size) {

	//	cout << "enter b equal to one" << endl;
	uint32_t exception_value_size = get_exception_value_size(exception_type);

	Header header;
	header.frame_of_reference_ = frame_of_reference;
	header.significant_data_size_ = significant_data_size;
	header.fixed_length_ = fixed_length;
	header.exception_type_ = exception_type;

	uint32_t exception_indexes[kBatchSize + 1];
	uint32_t adjusted_data[kBatchSize];

	// First delta is always 0, use it as the exception flag
	uint32_t exception_count = 1; // frame_of_reference is always a exception
	exception_indexes[0] = 0;
	uint32_t max_value = 1; // equivalent to (1 << fixed_length ) - 1

	for (uint32_t idx = 1; idx < kBatchSize; ++idx) { // skip first delta
		//uint32_t adjusted_value = data[idx] - frame_of_reference;
		//cout << data[idx] << " ";
		uint32_t adjusted_value = deltas[idx];
		adjusted_data[idx] = adjusted_value;
		exception_indexes[exception_count] = idx;
		exception_count += (uint32_t) (adjusted_value > max_value);
	}

	uint32_t req_buffer_size = sizeof(uint64_t) /*header size */
	+ sizeof(char) /*pair with f_o_r*/
	+ (exception_count - 1) /*not count f_o_r*/
	* (sizeof(exception_value_size) + sizeof(char));

	//  header encoded_size require division of 8 bytes
	req_buffer_size = (req_buffer_size + sizeof(uint64_t) - 1)
			/ sizeof(uint64_t) * sizeof(uint64_t);

	// encode # of 1s
	uint8_t * ones = (uint8_t*) buffer + sizeof(uint64_t) + (exception_count
			- 1) * exception_value_size; // jump over all exception value
	int ones_idx = 0;
	//	cout << "encode [ ones: " ;
	for (uint32_t idx = 1; idx < exception_count; ++idx) {
		ones[ones_idx++] = exception_indexes[idx] - exception_indexes[idx - 1];
		//		 printf("%u,",  ones[ones_idx]);
		/*
		 * exp:     1238         89
		 * deltas:  0   1 , 1 , 0 ,
		 * del_idx: 0   1 , 2 , 3 ,
		 * ones         3 - 0    = 3 includes 1 extra bit for exception value
		 */
	}

	exception_as_number_of_zeros(header.exception_type_,
			(void *) ((const char *) buffer + sizeof(uint64_t)), adjusted_data,
			exception_indexes, exception_count);

	ones[ones_idx] = (significant_data_size - 1) /*end of block*/
	- exception_indexes[exception_count - 1] /*index of last exception*/
	+ 1 /*plus extra 1 bit for exception value*/;

	header.first_exception_ = exception_count; // this field is changed to represent number of exception value ( # of 0s )

	header.encoded_size_ = req_buffer_size;

	//	printf("first exception = %lu | # of 1s = %u\n",header.frame_of_reference_, ones[0]);
	//header.print();
	if (!header.write(buffer)) {
		cout << "here write" << endl;
		LOG_WARNING_RETURN_FAIL("failed to write header")
		;
	}

	buffer_size = header.encoded_size_;

	return true;

}

template<class ExceptionValueType>
bool PatchedFrameOfReference::exception_as_number_of_zeros_typed(
		uint32_t *adjusted_data, const uint32_t *exception_indexes,
		uint32_t exception_count, ExceptionValueType *zeros) {
	int exp_offset = 0;
	//	cout << "zeros :" ;
	for (uint32_t idx = 1; idx < exception_count; ++idx) {
		// exception is greater than 1
		// exception could not be 0 because there is no same value in RID list, no zero for delta value
		// exception could not be 1 because  one bit can encode 1
		zeros[exp_offset++] = adjusted_data[exception_indexes[idx]] - 1; // exception value -1 means the # of 0s
		//		printf("%u,", zeros[exp_offset]);
	}

}

bool PatchedFrameOfReference::exception_as_number_of_zeros(
		ExceptionType exception_type, void *exceptions,
		uint32_t *adjusted_data, const uint32_t *exception_indexes,
		uint32_t exception_count) {

	switch (exception_type) {
	case EXCEPTION_UNSIGNED_CHAR:
		return exception_as_number_of_zeros_typed<unsigned char> (
				adjusted_data, exception_indexes, exception_count,
				(unsigned char *) exceptions);

	case EXCEPTION_SIGNED_CHAR:
		return exception_as_number_of_zeros_typed<signed char> (adjusted_data,
				exception_indexes, exception_count, (signed char *) exceptions);

	case EXCEPTION_UNSIGNED_SHORT:
		return exception_as_number_of_zeros_typed<unsigned short> (
				adjusted_data, exception_indexes, exception_count,
				(unsigned short *) exceptions);

	case EXCEPTION_SIGNED_SHORT:
		return exception_as_number_of_zeros_typed<signed short> (adjusted_data,
				exception_indexes, exception_count, (signed short *) exceptions);

	case EXCEPTION_UNSIGNED_INT:
		return exception_as_number_of_zeros_typed<unsigned int> (adjusted_data,
				exception_indexes, exception_count, (unsigned int *) exceptions);

	case EXCEPTION_SIGNED_INT:
		return exception_as_number_of_zeros_typed<signed int> (adjusted_data,
				exception_indexes, exception_count, (signed int *) exceptions);

	default:
		LOG_WARNING_RETURN_FAIL("unknown exception type: ", (int)exception_type)
		;
	}

}


bool PatchedFrameOfReference::decode_set_bmap(const void *buffer,
		uint32_t buffer_size, bmap_t ** bmap) {

	const char *current_buffer = (const char *) buffer;
	uint32_t remaining_buffer = buffer_size;


	uint32_t data_size = *(const uint32_t *) current_buffer;
	uint32_t remaining_data = data_size;
	current_buffer += sizeof(uint32_t);
	remaining_buffer -= sizeof(uint32_t);


	while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
		uint32_t actual_data_size;
		uint32_t actual_significant_data_size;
		uint32_t actual_buffer_size;

		decode_every_batch(current_buffer, remaining_buffer, actual_data_size,
				actual_significant_data_size, actual_buffer_size, bmap);

		current_buffer += actual_buffer_size;
		remaining_buffer -= actual_buffer_size;

		remaining_data -= PatchedFrameOfReference::kBatchSize;
	}

	if (remaining_data > 0) {
		uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

		uint32_t actual_data_size;
		uint32_t actual_significant_data_size;
		uint32_t actual_buffer_size;

		decode_every_batch(current_buffer, remaining_buffer, actual_data_size,
				actual_significant_data_size, actual_buffer_size, bmap);

		current_buffer += actual_buffer_size;
		remaining_buffer -= actual_buffer_size;

		remaining_data -= remaining_data;
	}
	/*if (remaining_buffer != 0) {
	 LOG_ERROR_RETURN_FAIL("unexpected content");
	 }*/
	return true;
}


bool PatchedFrameOfReference::decode_every_batch(const void *buffer,
		uint32_t buffer_capacity, uint32_t &data_size,
		uint32_t &significant_data_size, uint32_t &buffer_size, bmap_t **bmap) {
	data_size = 0;
	significant_data_size = 0;
	uint32_t word; //tmp variable
	uint64_t rid; // NOTE: summary of all RIDs could exceed to uint32_t

	//temporal data holder, mainly used for take the decompressed delta value.
	// This strategy is for memory efficiency purpose
	uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * kBatchSize);
	memset(data, 0, sizeof(uint32_t) * kBatchSize);

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);
		rid = header.frame_of_reference_;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			word = (uint32_t) (rid >> 6);
			(*bmap)[word] |= PRECALED2[rid & 0x3F];
		}

	} else {

		fixed_length_decode((const char *) buffer + sizeof(uint64_t),
				PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
				header.fixed_length_, data, kBatchSize);

		patch_exceptions_new(header.exception_type_, data,
				header.first_exception_, header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);

		rid = header.frame_of_reference_;
		for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
			rid = rid + data[index];
			word = (uint32_t) (rid >> 6);
			(*bmap)[word] |= PRECALED2[rid & 0x3F];
		}

	}
	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	free(data);
	return true;
}

bool PatchedFrameOfReference::decode(const void *buffer, uint32_t buffer_size,
		vector<uint32_t> &data) {
	data.resize(0);

	const char *current_buffer = (const char *) buffer;
	uint32_t remaining_buffer = buffer_size;

	if (remaining_buffer < sizeof(uint32_t)) {
		LOG_ERROR_RETURN_FAIL("no data size")
		;
	}

	uint32_t data_size = *(const uint32_t *) current_buffer;
	uint32_t remaining_data = data_size;
	current_buffer += sizeof(uint32_t);
	remaining_buffer -= sizeof(uint32_t);

	if (data_size <= NO_COMPRESS_THRESHOLD) {
		while (remaining_data > 0) {
			data.push_back(*((uint32_t *) current_buffer));
			current_buffer += sizeof(uint32_t);
			remaining_buffer -= sizeof(uint32_t);
			remaining_data--;
		}
		if (remaining_buffer != 0) {
			cout << "remaining buffer not zero " << endl;
			LOG_ERROR_RETURN_FAIL("unexpected content")
			;
		}
		for (uint32_t index = 0; index < data.size(); ++index) {
			cout << data[index] << " ";
		}
		cout << endl;
		return true;
	}

	data.resize(data_size);
	uint32_t *current_data = &data[0];

	while (remaining_data >= PatchedFrameOfReference::kBatchSize) {
		uint32_t actual_data_size;
		uint32_t actual_significant_data_size;
		uint32_t actual_buffer_size;
		/*if (! decode(current_buffer, 
		 remaining_buffer,
		 current_data,
		 PatchedFrameOfReference::kBatchSize,
		 actual_data_size,
		 actual_significant_data_size,
		 actual_buffer_size)) {
		 LOG_ERROR_RETURN_FAIL("failed to decode a batch");
		 }*/

		if (!decode_new(current_buffer, remaining_buffer, current_data,
				PatchedFrameOfReference::kBatchSize, actual_data_size,
				actual_significant_data_size, actual_buffer_size)) {
			LOG_ERROR_RETURN_FAIL("failed to decode a batch")
			;
		}

		release_assert(actual_buffer_size <= remaining_buffer);

		if (actual_data_size != PatchedFrameOfReference::kBatchSize
				|| actual_significant_data_size
						!= PatchedFrameOfReference::kBatchSize) {
			LOG_ERROR_RETURN_FAIL("invalid partial batch")
			;
		}

		current_buffer += actual_buffer_size;
		remaining_buffer -= actual_buffer_size;

		current_data += PatchedFrameOfReference::kBatchSize;
		remaining_data -= PatchedFrameOfReference::kBatchSize;
	}

	if (remaining_data > 0) {
		uint32_t temp_data[PatchedFrameOfReference::kBatchSize] = { 0 };

		uint32_t actual_data_size;
		uint32_t actual_significant_data_size;
		uint32_t actual_buffer_size;

		if (!decode_new(current_buffer, remaining_buffer, temp_data,
				PatchedFrameOfReference::kBatchSize, actual_data_size,
				actual_significant_data_size, actual_buffer_size)) {
			LOG_ERROR_RETURN_FAIL("failed to decode a batch")
			;
		}

		release_assert(actual_buffer_size <= remaining_buffer);

		if (actual_data_size != PatchedFrameOfReference::kBatchSize
				|| actual_significant_data_size != remaining_data) {
			LOG_ERROR_RETURN_FAIL("invalid partial batch")
			;
		}

		memcpy(current_data, temp_data,
				actual_significant_data_size * sizeof(uint32_t));

		current_buffer += actual_buffer_size;
		remaining_buffer -= actual_buffer_size;

		current_data += remaining_data;
		remaining_data -= remaining_data;
	}
	if (remaining_buffer != 0) {
		LOG_ERROR_RETURN_FAIL("unexpected content")
		;
	}
	return true;
}

//uint64_t * PatchedFrameOfReference::B_TIMES;

//uint64_t * PatchedFrameOfReference::EXP_SIZE;

const uint32_t PatchedFrameOfReference::kBatchSize;

const uint32_t PatchedFrameOfReference::kSufficientBufferCapacity;

const uint32_t PatchedFrameOfReference::kMinFixedLength;

#define INCREASE_DATA_16(OFFSET, INCREMENT) \
	do { \
		data[OFFSET +  0] += INCREMENT; data[OFFSET +  1] += INCREMENT; \
		data[OFFSET +  2] += INCREMENT; data[OFFSET +  3] += INCREMENT; \
		data[OFFSET +  4] += INCREMENT; data[OFFSET +  5] += INCREMENT; \
		data[OFFSET +  6] += INCREMENT; data[OFFSET +  7] += INCREMENT; \
		data[OFFSET +  8] += INCREMENT; data[OFFSET +  9] += INCREMENT; \
		data[OFFSET + 10] += INCREMENT; data[OFFSET + 11] += INCREMENT; \
		data[OFFSET + 12] += INCREMENT; data[OFFSET + 13] += INCREMENT; \
		data[OFFSET + 14] += INCREMENT; data[OFFSET + 15] += INCREMENT; \
	} while (false)

#define INCREASE_DATA(INCREMENT) \
	do { \
		INCREASE_DATA_16(  0, INCREMENT); INCREASE_DATA_16( 16, INCREMENT); \
		INCREASE_DATA_16( 32, INCREMENT); INCREASE_DATA_16( 48, INCREMENT); \
		INCREASE_DATA_16( 64, INCREMENT); INCREASE_DATA_16( 80, INCREMENT); \
		INCREASE_DATA_16( 96, INCREMENT); INCREASE_DATA_16(112, INCREMENT); \
	} while (false)

bool PatchedFrameOfReference::expand_encode(const uint32_t *data,
		uint32_t data_size, uint32_t significant_data_size, void *buffer,
		uint32_t buffer_capacity, uint32_t &buffer_size, bool verify) {
	uint32_t fixed_length;
	uint32_t frame_of_reference;
	ExceptionType exception_type;

	uint32_t deltas[PatchedFrameOfReference::kBatchSize] = { 0 };
	deltas[0] = 0;
	for (uint32_t index = 1; index < significant_data_size; ++index) {
		deltas[index] = data[index] - data[index - 1];
	}
	for (uint32_t index = significant_data_size; index < kBatchSize; ++index) {
		deltas[index] = 0;
	}

	if (!determine_b_to_expand(deltas, data_size, significant_data_size,
			fixed_length, frame_of_reference, exception_type)) {
		LOG_WARNING_RETURN_FAIL("failed to find optimal parameters")
		;
	}
	frame_of_reference = data[0];

	if (!encode_param_new(deltas, data_size, significant_data_size,
			fixed_length, frame_of_reference, exception_type, buffer,
			buffer_capacity, buffer_size)) {
		LOG_WARNING_RETURN_FAIL("failed to encode with optimal parameters: ", fixed_length, " ", frame_of_reference)
		;
	}

	return true;
}

bool PatchedFrameOfReference::encode(const uint32_t *data, uint32_t data_size,
		uint32_t significant_data_size, void *buffer, uint32_t buffer_capacity,
		uint32_t &buffer_size, bool verify) {
	uint32_t fixed_length;
	uint32_t frame_of_reference;
	ExceptionType exception_type;

	uint32_t deltas[PatchedFrameOfReference::kBatchSize] = { 0 };
	deltas[0] = 0;
	for (uint32_t index = 1; index < significant_data_size; ++index) {
		deltas[index] = data[index] - data[index - 1];
	}
	for (uint32_t index = significant_data_size; index < kBatchSize; ++index) {
		deltas[index] = 0;
	}

	if (!optimal_param(deltas, data_size, significant_data_size, fixed_length,
			frame_of_reference, exception_type)) {
		LOG_WARNING_RETURN_FAIL("failed to find optimal parameters")
		;
	}
	frame_of_reference = data[0];

	if (!encode_param_new(deltas, data_size, significant_data_size,
			fixed_length, frame_of_reference, exception_type, buffer,
			buffer_capacity, buffer_size)) {
		LOG_WARNING_RETURN_FAIL("failed to encode with optimal parameters: ", fixed_length, " ", frame_of_reference)
		;
	}

	return true;
}


bool PatchedFrameOfReference::decode_new_to_deltas(const void *buffer,
		uint32_t buffer_capacity, uint32_t *data, uint32_t data_capacity,
		uint32_t &data_size, uint32_t &significant_data_size,
		uint32_t &buffer_size) {
	data_size = 0;
	significant_data_size = 0;

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);
		data[0] += header.frame_of_reference_; // rest are deltas

	} else {
		fixed_length_decode((const char *) buffer + sizeof(uint64_t),
				PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
				header.fixed_length_, data, kBatchSize);
		patch_exceptions_new(header.exception_type_, data,
				header.first_exception_, header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);

		data[0] += header.frame_of_reference_; // rest are deltas
	}
	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	return true;
}


bool PatchedFrameOfReference::decode_new(const void *buffer,
		uint32_t buffer_capacity, uint32_t *data, uint32_t data_capacity,
		uint32_t &data_size, uint32_t &significant_data_size,
		uint32_t &buffer_size) {
	data_size = 0;
	significant_data_size = 0;

	Header header;
	header.read(buffer);

	if (header.fixed_length_ == 32) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);

		for (uint32_t index = 1; index < header.significant_data_size_; ++index) {
			data[index] += data[index - 1];
		}

		if (header.frame_of_reference_ != 0) {
			for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				data[index] += header.frame_of_reference_;
			}


		}
	} else {
		fixed_length_decode((const char *) buffer + sizeof(uint64_t),
				PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
				header.fixed_length_, data, kBatchSize);
		patch_exceptions_new(header.exception_type_, data,
				header.first_exception_, header.significant_data_size_,
				(const char *) buffer + header.encoded_size_);

		data[0] += header.frame_of_reference_;
		for (uint32_t index = 1; index < header.significant_data_size_; ++index) {
			data[index] += data[index - 1];
		}

	}
	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	return true;
}

bool PatchedFrameOfReference::decode(const void *buffer,
		uint32_t buffer_capacity, uint32_t *data, uint32_t data_capacity,
		uint32_t &data_size, uint32_t &significant_data_size,
		uint32_t &buffer_size) {
	data_size = 0;
	significant_data_size = 0;

	if (data_capacity < kBatchSize) {
		LOG_WARNING_RETURN_FAIL("data capacity less than ", kBatchSize, ": ", data_capacity)
		;
	}

	if (buffer_capacity < kHeaderSize) {
		LOG_WARNING_RETURN_FAIL("invalid buffer size ", buffer_size)
		;
	}

	Header header;
	if (!header.read(buffer)) {
		LOG_WARNING_RETURN_FAIL("invalid buffer header")
		;
	}

	if (buffer_capacity < header.encoded_size_) {
		LOG_WARNING_RETURN_FAIL("no capacity for encoded size: ", header.encoded_size_)
		;
	}
	if (header.fixed_length_ == 1) {
		uint32_t req_buffer_size = sizeof(uint64_t)
				+ (header.significant_data_size_ * get_exception_value_size(
						header.exception_type_) + sizeof(uint64_t) - 1)
						/ sizeof(uint64_t) * sizeof(uint64_t);

		if (header.encoded_size_ < req_buffer_size) {
			LOG_WARNING_RETURN_FAIL("buffer size ", header.encoded_size_, " too small for encoded exceptions ",
					req_buffer_size)
			;
		}

		if (!decode_as_exceptions(header.exception_type_, data,
				header.significant_data_size_,
				(const char *) buffer + header.encoded_size_)) {
			LOG_WARNING_RETURN_FAIL("failed to decode as exceptions")
			;
		}
		for (uint32_t index = 1; index < header.significant_data_size_; ++index) {
			data[index] += data[index - 1];
		}

		if (header.frame_of_reference_ != 0) {
			for (uint32_t index = 0; index < header.significant_data_size_; ++index) {
				data[index] += header.frame_of_reference_;
			}
			/*cout << "\n\nDecoded block, fixed length = 1\n\n";
			 for (uint32_t index = 1; index < header.significant_data_size_; ++index) {
			 cout << data[index] << " ";
			 }
			 cout << endl;*/

		}
	} else {
		if (header.encoded_size_ - sizeof(uint64_t)
				< PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_)) {
			LOG_WARNING_RETURN_FAIL("buffer size ", header.encoded_size_,
					" too small for fixed length ", header.fixed_length_)
			;
		}

		if (!fixed_length_decode((const char *) buffer + sizeof(uint64_t),
				PFOR_FIXED_LENGTH_BUFFER_SIZE(header.fixed_length_),
				header.fixed_length_, data, kBatchSize)) {
			LOG_WARNING_RETURN_FAIL("failed to decode")
			;
		}

		if (!patch_exceptions(header.exception_type_, data,
				header.first_exception_,
				(const char *) buffer + header.encoded_size_)) {
			LOG_WARNING_RETURN_FAIL("failed to patch exceptions")
			;
		}
		for (uint32_t index = 1; index < header.significant_data_size_; ++index) {
			data[index] += data[index - 1];
		}
		if (header.frame_of_reference_ != 0) {
			INCREASE_DATA(header.frame_of_reference_);
		}
	}

	data_size = kBatchSize;
	significant_data_size = header.significant_data_size_;
	buffer_size = header.encoded_size_;
	return true;
}

static uint32_t exception_value_sizes[] = { 1, 1, 2, 2, 4, 4 };

uint32_t PatchedFrameOfReference::get_sufficient_buffer_capacity(
		uint32_t bin_length) {
	return kSufficientBufferCapacity * (uint32_t) ceil(
			1.0 * bin_length / kBatchSize);
}

uint32_t PatchedFrameOfReference::get_exception_value_size(
		ExceptionType exception_type) {
	if ((uint32_t) exception_type > (sizeof(exception_value_sizes)
			/ sizeof(exception_value_sizes[0]))) {
		return 4;
	} else {
		return exception_value_sizes[(uint32_t) exception_type];
	}
}

uint32_t PatchedFrameOfReference::required_buffer_size(uint32_t fixed_length,
		uint32_t exception_count, uint32_t exception_value_size) {
	uint32_t buffer_size = sizeof(uint64_t)
			+ PFOR_FIXED_LENGTH_BUFFER_SIZE(fixed_length) + (exception_count
			* exception_value_size + sizeof(uint64_t) - 1) / sizeof(uint64_t)
			* sizeof(uint64_t);

	return buffer_size;
}

bool PatchedFrameOfReference::encode_param_new(const uint32_t *data,
		uint32_t data_size, uint32_t significant_data_size,
		uint32_t fixed_length, uint32_t frame_of_reference,
		ExceptionType exception_type, void *buffer, uint32_t buffer_capacity,
		uint32_t &buffer_size) {

	uint32_t exception_value_size = get_exception_value_size(exception_type);

	Header header;
	header.frame_of_reference_ = frame_of_reference;
	header.significant_data_size_ = significant_data_size;
	header.fixed_length_ = fixed_length;
	header.exception_type_ = exception_type;

	if (fixed_length == 0) {
		header.first_exception_ = kBatchSize;

		uint32_t adjusted_data[kBatchSize];

		for (uint32_t idx = 0; idx < kBatchSize; ++idx) {
			//uint32_t adjusted_value = data[idx] - frame_of_reference;
			//adjusted_data[idx] = adjusted_value;
			uint32_t adjusted_value = data[idx];
			adjusted_data[idx] = adjusted_value;
		}
		uint32_t req_buffer_size = sizeof(uint64_t) + (significant_data_size
				* exception_value_size + sizeof(uint64_t) - 1)
				/ sizeof(uint64_t) * sizeof(uint64_t);

		//cout << "required buffer size = " << req_buffer_size << endl; 
		if (buffer_capacity < req_buffer_size) {
			LOG_WARNING_RETURN_FAIL("insufficient buffer capacity: ", buffer_capacity, " required: ", req_buffer_size)
			;
		}

		if (significant_data_size != 0) {
			*(uint64_t *) ((char *) buffer + sizeof(uint64_t)) = 0;
		}

		if (!encode_as_exceptions(header.exception_type_, adjusted_data,
				significant_data_size, (char *) buffer + req_buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to encode as exceptions")
			;
		}

		header.encoded_size_ = req_buffer_size;

	} else {
		/////////////////////////////////////////////////////////////

		uint32_t exception_indexes[kBatchSize + 1];
		uint32_t adjusted_data[kBatchSize];

		uint32_t exception_count = 0;
		uint32_t max_value;

		if (fixed_length == 32) {
			max_value = (uint32_t) -1;
		} else {
			max_value = (1 << fixed_length) - 1;
		}

		for (uint32_t idx = 0; idx < kBatchSize; ++idx) {
			uint32_t adjusted_value = data[idx];
			adjusted_data[idx] = adjusted_value;
			exception_indexes[exception_count] = idx;
			exception_count += (uint32_t) (adjusted_value > max_value);
		}

		uint32_t req_buffer_size = required_buffer_size(fixed_length,
				exception_count, exception_value_size);

		if (buffer_capacity < req_buffer_size) {
			LOG_WARNING_RETURN_FAIL("insufficient buffer capacity: ", buffer_capacity, " required: ", req_buffer_size)
			;
		}

		exception_indexes[exception_count] = kBatchSize;

		header.first_exception_ = exception_indexes[0];

		if (exception_count != 0) {
			*(uint64_t *) ((char *) buffer + sizeof(uint64_t)
					+ PFOR_FIXED_LENGTH_BUFFER_SIZE(fixed_length)) = 0;
		}

		//cout << "Exception count = " << exception_count << endl;

		if (!compute_exceptions(header.exception_type_, adjusted_data,
				exception_indexes, exception_count,
				(char *) buffer + req_buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to compute exceptions")
			;
		}
		// compute_exceptions has the exception offset chain,
		// here, it overrides the exception position with value zero
		for (uint32_t idx = 0; idx < exception_count; ++idx) {
			adjusted_data[exception_indexes[idx]] = 0;
		}

		if (!fixed_length_encode(adjusted_data, kBatchSize,
				header.fixed_length_, (char *) buffer + sizeof(uint64_t),
				PFOR_FIXED_LENGTH_BUFFER_SIZE(fixed_length))) {
			LOG_WARNING_RETURN_FAIL("failed to encode with fixed length")
			;
		}

		header.encoded_size_ = req_buffer_size;

	}
	//header.print();
	if (!header.write(buffer)) {
		cout << "here write" << endl;
		LOG_WARNING_RETURN_FAIL("failed to write header")
		;
	}

	buffer_size = header.encoded_size_;

	return true;
}

bool PatchedFrameOfReference::encode_param(const uint32_t *data,
		uint32_t data_size, uint32_t significant_data_size,
		uint32_t fixed_length, uint32_t frame_of_reference,
		ExceptionType exception_type, void *buffer, uint32_t buffer_capacity,
		uint32_t &buffer_size, bool verify) {
	if (fixed_length == 1) {
	} else if (fixed_length == 0 || fixed_length < kMinFixedLength
			|| fixed_length > 32) {
		LOG_WARNING_RETURN_FAIL("invalid fixed length: ", fixed_length)
		;
	}

	if (data_size != kBatchSize) {
		LOG_WARNING_RETURN_FAIL("data size not ", kBatchSize, ": ", data_size)
		;
	}

	uint32_t exception_value_size = get_exception_value_size(exception_type);

	Header header;
	header.frame_of_reference_ = frame_of_reference;
	header.significant_data_size_ = significant_data_size;
	header.fixed_length_ = fixed_length;
	header.exception_type_ = exception_type;

	if (fixed_length == 1) {
		header.first_exception_ = kBatchSize;

		uint32_t adjusted_data[kBatchSize];

		for (uint32_t idx = 0; idx < kBatchSize; ++idx) {
			//uint32_t adjusted_value = data[idx] - frame_of_reference;
			//adjusted_data[idx] = adjusted_value;
			uint32_t adjusted_value = data[idx];
			adjusted_data[idx] = adjusted_value;
		}
		uint32_t req_buffer_size = sizeof(uint64_t) + (significant_data_size
				* exception_value_size + sizeof(uint64_t) - 1)
				/ sizeof(uint64_t) * sizeof(uint64_t);

		if (buffer_capacity < req_buffer_size) {
			LOG_WARNING_RETURN_FAIL("insufficient buffer capacity: ", buffer_capacity, " required: ", req_buffer_size)
			;
		}

		if (significant_data_size != 0) {
			*(uint64_t *) ((char *) buffer + sizeof(uint64_t)) = 0;
		}

		if (!encode_as_exceptions(header.exception_type_, adjusted_data,
				significant_data_size, (char *) buffer + req_buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to encode as exceptions")
			;
		}

		header.encoded_size_ = req_buffer_size;

	} else {
		uint32_t exception_indexes[kBatchSize + 1];
		uint32_t adjusted_data[kBatchSize];

		uint32_t exception_count = 0;
		uint32_t max_value;

		if (fixed_length == 32) {
			max_value = (uint32_t) -1;
		} else {
			max_value = (1 << fixed_length) - 1;
		}

		for (uint32_t idx = 0; idx < kBatchSize; ++idx) {
			//uint32_t adjusted_value = data[idx] - frame_of_reference;
			uint32_t adjusted_value = data[idx];
			adjusted_data[idx] = adjusted_value;
			exception_indexes[exception_count] = idx;
			exception_count += (uint32_t) (adjusted_value > max_value);
		}

		//cout << "\nException Indexes\n";
		//for (uint32_t index = 0; index < exception_count; ++index) {
		//	cout << exception_indexes[index] << " ";
		//}
		//cout << endl;
		uint32_t req_buffer_size = required_buffer_size(fixed_length,
				exception_count, exception_value_size);

		if (buffer_capacity < req_buffer_size) {
			LOG_WARNING_RETURN_FAIL("insufficient buffer capacity: ", buffer_capacity, " required: ", req_buffer_size)
			;
		}

		exception_indexes[exception_count] = kBatchSize;

		header.first_exception_ = exception_indexes[0];

		if (exception_count != 0) {
			*(uint64_t *) ((char *) buffer + sizeof(uint64_t)
					+ PFOR_FIXED_LENGTH_BUFFER_SIZE(fixed_length)) = 0;
		}

		if (!compute_exceptions(header.exception_type_, adjusted_data,
				exception_indexes, exception_count,
				(char *) buffer + req_buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to compute exceptions")
			;
		}

		if (!fixed_length_encode(adjusted_data, kBatchSize,
				header.fixed_length_, (char *) buffer + sizeof(uint64_t), /*jump over header*/
				PFOR_FIXED_LENGTH_BUFFER_SIZE(fixed_length))) {
			LOG_WARNING_RETURN_FAIL("failed to encode with fixed length")
			;
		}

		header.encoded_size_ = req_buffer_size;
	}
	if (!header.write(buffer)) {
		LOG_WARNING_RETURN_FAIL("failed to write header")
		;
	}

	buffer_size = header.encoded_size_;

	if (verify) {
		uint32_t decoded[kBatchSize];
		uint32_t decoded_data_size;
		uint32_t decoded_significant_data_size;
		uint32_t decoded_buffer_size;

		if (!decode(buffer, buffer_size, decoded, kBatchSize,
				decoded_data_size, decoded_significant_data_size,
				decoded_buffer_size)) {
			LOG_WARNING_RETURN_FAIL("failed to verify decoding")
			;
		}

		if (decoded_data_size != data_size) {
			LOG_WARNING_RETURN_FAIL("failed verification of decoded data size")
			;
		}

		if (decoded_significant_data_size != significant_data_size) {
			LOG_WARNING_RETURN_FAIL("failed verification of decoded significant data size")
			;
		}

		if (decoded_buffer_size != buffer_size) {
			LOG_WARNING_RETURN_FAIL("failed verification of buffer size")
			;
		}
		uint32_t *verify_data = (uint32_t *) malloc(
				sizeof(uint32_t) * data_size);
		memcpy(verify_data, data, data_size);
		for (uint32_t index = 1; index < significant_data_size; ++index) {
			verify_data[index] += verify_data[index - 1];
		}
		for (uint32_t index = 0; index < significant_data_size; ++index) {
			verify_data[index] += frame_of_reference;
		}
		if (memcmp(decoded, verify_data,
				decoded_significant_data_size * sizeof(uint32_t)) != 0) {
			LOG_WARNING("decoded_significant_data_size: ", decoded_significant_data_size);
			for (uint32_t pos = 0; pos < decoded_significant_data_size; ++pos) {
				cout << setw(4) << pos << ": " << setw(8) << verify_data[pos]
						<< " " << setw(8) << decoded[pos] << endl;
			}
			LOG_WARNING_RETURN_FAIL("failed verification of decoded data")
			;
		}
	}

	return true;
}

PatchedFrameOfReference::Header::Header() :
	frame_of_reference_(0), first_exception_(0), significant_data_size_(0),
			fixed_length_(0), exception_type_(EXCEPTION_UNSIGNED_CHAR) {
}

bool PatchedFrameOfReference::Header::read(const void *buffer) {
	const CompactHeader &compact_header = *(const CompactHeader *) buffer;

	if (compact_header.first_exception_ > kBatchSize) {
		cout << "here 11" << endl;
		LOG_WARNING_RETURN_FAIL("invalid first exception offset: ", compact_header.first_exception_)
		;
	}

	if (compact_header.significant_data_size_ > kBatchSize) {
		LOG_WARNING_RETURN_FAIL("invalid significant data size : %u ", compact_header.significant_data_size_)
		;
	}

	if ((uint32_t) compact_header.fixed_length_ + 1 == 1) {

	} else if ((uint32_t) compact_header.fixed_length_ + 1 < 1
			|| (uint32_t) compact_header.fixed_length_ + 1 > 32) {
		LOG_WARNING_RETURN_FAIL("invalid fixed length: ", compact_header.fixed_length_)
		;
	}

	if (compact_header.exception_type_ > (uint32_t) EXCEPTION_SIGNED_INT) {
		LOG_WARNING_RETURN_FAIL("invalid exception type: ", compact_header.exception_type_)
		;
	}

	if (compact_header.encoded_size_ * sizeof(uint64_t) == 0
			|| compact_header.encoded_size_ * sizeof(uint64_t)
					> kSufficientBufferCapacity) {
		LOG_WARNING_RETURN_FAIL("invalid size in header %lu \n ", compact_header.encoded_size_ * sizeof(uint64_t))
		;
	}

	frame_of_reference_ = compact_header.frame_of_reference_;
	first_exception_ = compact_header.first_exception_;
	significant_data_size_ = compact_header.significant_data_size_;
	fixed_length_ = (compact_header.fixed_length_ + 1);
	exception_type_ = (ExceptionType) compact_header.exception_type_;
	encoded_size_ = compact_header.encoded_size_ * sizeof(uint64_t);

	return true;
}

bool PatchedFrameOfReference::Header::write(void *buffer) const {
	char *char_buffer = (char *) buffer;
	CompactHeader compact_header;
	compact_header.frame_of_reference_ = frame_of_reference_;
	compact_header.first_exception_ = first_exception_;
	compact_header.significant_data_size_ = significant_data_size_;
	compact_header.fixed_length_ = (fixed_length_ - 1);
	compact_header.exception_type_ = (uint32_t) exception_type_;
	release_assert(encoded_size_ % sizeof(uint64_t) == 0);
	compact_header.encoded_size_ = encoded_size_ / sizeof(uint64_t);

	*(CompactHeader *) &char_buffer[0] = compact_header;

	return true;
}


void PatchedFrameOfReference::Header::print() {
	cout << "Frame of reference =" << frame_of_reference_ << " ,";
	cout << "First exception =" << first_exception_ << " ,";
	cout << "Sign. data size =" << significant_data_size_ << " ,";
	cout << "Fixed length = " << fixed_length_ << " ,";
	cout << "Exception type = " << exception_type_ << " ,";
	cout << "Encoded size = " << encoded_size_ << " ,";
}

template<class ExceptionValueType>
bool PatchedFrameOfReference::patch_typed_exceptions(uint32_t *data,
		uint32_t exception_offset, const ExceptionValueType *exceptions) {
	while (exception_offset < kBatchSize) {
		uint32_t next_exception_offset = exception_offset
				+ data[exception_offset] + 1;
		data[exception_offset] = (uint32_t) *--exceptions;
		exception_offset = next_exception_offset;
	}
	return true;
}

template<class ExceptionValueType>
bool PatchedFrameOfReference::patch_typed_exceptions_new(uint32_t *data,
		uint32_t exception_offset, uint32_t significant_data_size,
		const ExceptionValueType *exceptions) {

	int count_exp = 0;
	while (exception_offset < significant_data_size) {
		if (data[exception_offset] == 0) {
			data[exception_offset] = (uint32_t) *--exceptions;
			count_exp++;
		}
		++exception_offset;
	}
	//	cout << "exception number " << count_exp << endl;
	return true;
}

bool PatchedFrameOfReference::patch_exceptions(ExceptionType exception_type,
		uint32_t *data, uint32_t exception_offset, const void *exceptions) {
	switch (exception_type) {
	case EXCEPTION_UNSIGNED_CHAR:
		return patch_typed_exceptions<unsigned char> (data, exception_offset,
				(const unsigned char *) exceptions);

	case EXCEPTION_SIGNED_CHAR:
		return patch_typed_exceptions<signed char> (data, exception_offset,
				(const signed char *) exceptions);

	case EXCEPTION_UNSIGNED_SHORT:
		return patch_typed_exceptions<unsigned short> (data, exception_offset,
				(const unsigned short *) exceptions);

	case EXCEPTION_SIGNED_SHORT:
		return patch_typed_exceptions<signed short> (data, exception_offset,
				(const signed short *) exceptions);

	case EXCEPTION_UNSIGNED_INT:
		return patch_typed_exceptions<unsigned int> (data, exception_offset,
				(const unsigned int *) exceptions);

	case EXCEPTION_SIGNED_INT:
		return patch_typed_exceptions<signed int> (data, exception_offset,
				(const signed int *) exceptions);

	default:
		LOG_WARNING_RETURN_FAIL("unknown exception type: %d\n ", (int)exception_type)
		;
	}
}

bool PatchedFrameOfReference::patch_exceptions_new(
		ExceptionType exception_type, uint32_t *data,
		uint32_t exception_offset, uint32_t significant_data_size,
		const void *exceptions) {
	switch (exception_type) {
	case EXCEPTION_UNSIGNED_CHAR:
		return patch_typed_exceptions_new<unsigned char> (data,
				exception_offset, significant_data_size,
				(const unsigned char *) exceptions);

	case EXCEPTION_SIGNED_CHAR:
		return patch_typed_exceptions_new<signed char> (data, exception_offset,
				significant_data_size, (const signed char *) exceptions);

	case EXCEPTION_UNSIGNED_SHORT:
		return patch_typed_exceptions_new<unsigned short> (data,
				exception_offset, significant_data_size,
				(const unsigned short *) exceptions);

	case EXCEPTION_SIGNED_SHORT:
		return patch_typed_exceptions_new<signed short> (data,
				exception_offset, significant_data_size,
				(const signed short *) exceptions);

	case EXCEPTION_UNSIGNED_INT:
		return patch_typed_exceptions_new<unsigned int> (data,
				exception_offset, significant_data_size,
				(const unsigned int *) exceptions);

	case EXCEPTION_SIGNED_INT:
		return patch_typed_exceptions_new<signed int> (data, exception_offset,
				significant_data_size, (const signed int *) exceptions);

	default:
		LOG_WARNING_RETURN_FAIL("unknown exception type: %d \n", (int)exception_type)
		;
	}
}

template<class ExceptionValueType>
bool PatchedFrameOfReference::compute_typed_exceptions(uint32_t *adjusted_data,
		const uint32_t *exception_indexes, uint32_t exception_count,
		ExceptionValueType *exceptions) {
	uint32_t current_idx = exception_indexes[0];

	for (uint32_t exception_offset = 0; exception_offset < exception_count; ++exception_offset) {
		*--exceptions = (ExceptionValueType) adjusted_data[current_idx];
		if ((uint32_t) *exceptions != adjusted_data[current_idx]) {
			LOG_WARNING_RETURN_FAIL("invalid data for exception")
			;
		}
		uint32_t next_idx = exception_indexes[exception_offset + 1];
		adjusted_data[current_idx] = next_idx - current_idx - 1; // exception value position has the offset for next exception value
		current_idx = next_idx;
	}

	return true;
}
;

bool PatchedFrameOfReference::compute_exceptions(ExceptionType exception_type,
		uint32_t *adjusted_data, const uint32_t *exception_indexes,
		uint32_t exception_count, void *exceptions) {
	switch (exception_type) {
	case EXCEPTION_UNSIGNED_CHAR:
		return compute_typed_exceptions<unsigned char> (adjusted_data,
				exception_indexes, exception_count,
				(unsigned char *) exceptions);

	case EXCEPTION_SIGNED_CHAR:
		return compute_typed_exceptions<signed char> (adjusted_data,
				exception_indexes, exception_count, (signed char *) exceptions);

	case EXCEPTION_UNSIGNED_SHORT:
		return compute_typed_exceptions<unsigned short> (adjusted_data,
				exception_indexes, exception_count,
				(unsigned short *) exceptions);

	case EXCEPTION_SIGNED_SHORT:
		return compute_typed_exceptions<signed short> (adjusted_data,
				exception_indexes, exception_count, (signed short *) exceptions);

	case EXCEPTION_UNSIGNED_INT:
		return compute_typed_exceptions<unsigned int> (adjusted_data,
				exception_indexes, exception_count, (unsigned int *) exceptions);

	case EXCEPTION_SIGNED_INT:
		return compute_typed_exceptions<signed int> (adjusted_data,
				exception_indexes, exception_count, (signed int *) exceptions);

	default:
		LOG_WARNING_RETURN_FAIL("unknown exception type: %d \n ", (int)exception_type)
		;
	}
}

template<class ExceptionValueType>
bool PatchedFrameOfReference::decode_as_typed_exceptions(uint32_t *data,
		uint32_t significant_data_size, const ExceptionValueType *exceptions) {
	//	uint32_t exp_num = 0;
	for (uint32_t index = 0; index < significant_data_size; ++index) {
		//		cout << *exceptions << ",";
		//		exp_num++;
		data[index] = (uint32_t) *--exceptions;
	}
	//	PatchedFrameOfReference::EXP_SIZE[exp_num]++;
	//	cout << "Exception # =" << exp_num << endl;
	return true;
}

bool PatchedFrameOfReference::decode_as_exceptions(
		ExceptionType exception_type, uint32_t *data,
		uint32_t significant_data_size, const void *exceptions) {
	switch (exception_type) {
	case EXCEPTION_UNSIGNED_CHAR:
		return decode_as_typed_exceptions<unsigned char> (data,
				significant_data_size, (const unsigned char *) exceptions);

	case EXCEPTION_SIGNED_CHAR:
		return decode_as_typed_exceptions<signed char> (data,
				significant_data_size, (const signed char *) exceptions);

	case EXCEPTION_UNSIGNED_SHORT:
		return decode_as_typed_exceptions<unsigned short> (data,
				significant_data_size, (const unsigned short *) exceptions);

	case EXCEPTION_SIGNED_SHORT:
		return decode_as_typed_exceptions<signed short> (data,
				significant_data_size, (const signed short *) exceptions);

	case EXCEPTION_UNSIGNED_INT:
		return decode_as_typed_exceptions<unsigned int> (data,
				significant_data_size, (const unsigned int *) exceptions);

	case EXCEPTION_SIGNED_INT:
		return decode_as_typed_exceptions<signed int> (data,
				significant_data_size, (const signed int *) exceptions);

	default:
		LOG_WARNING_RETURN_FAIL("unknown exception type: %d \n", (int)exception_type)
		;
	}

	return true;
}

template<class ExceptionValueType>
bool PatchedFrameOfReference::encode_as_typed_exceptions(
		const uint32_t *adjusted_data, uint32_t significant_data_size,
		ExceptionValueType *exceptions) {
	for (uint32_t index = 0; index < significant_data_size; ++index) {
		*--exceptions = (ExceptionValueType) adjusted_data[index];
	}
	return true;
}

bool PatchedFrameOfReference::encode_as_exceptions(
		ExceptionType exception_type, const uint32_t *adjusted_data,
		uint32_t significant_data_size, void *exceptions) {
	switch (exception_type) {
	case EXCEPTION_UNSIGNED_CHAR:
		return encode_as_typed_exceptions<unsigned char> (adjusted_data,
				significant_data_size, (unsigned char *) exceptions);

	case EXCEPTION_SIGNED_CHAR:
		return encode_as_typed_exceptions<signed char> (adjusted_data,
				significant_data_size, (signed char *) exceptions);

	case EXCEPTION_UNSIGNED_SHORT:
		return encode_as_typed_exceptions<unsigned short> (adjusted_data,
				significant_data_size, (unsigned short *) exceptions);

	case EXCEPTION_SIGNED_SHORT:
		return encode_as_typed_exceptions<signed short> (adjusted_data,
				significant_data_size, (signed short *) exceptions);

	case EXCEPTION_UNSIGNED_INT:
		return encode_as_typed_exceptions<unsigned int> (adjusted_data,
				significant_data_size, (unsigned int *) exceptions);

	case EXCEPTION_SIGNED_INT:
		return encode_as_typed_exceptions<signed int> (adjusted_data,
				significant_data_size, (signed int *) exceptions);

	default:
		LOG_WARNING_RETURN_FAIL("unknown exception type: %d \n ", (int)exception_type)
		;
	}

	return true;
}

