/*
 Patched Frame Of Reference Header
 Author : Saurabh V. Pendse
 */
#ifndef PATCHEDFRAMEOFREFERENCE_H
#define PATCHEDFRAMEOFREFERENCE_H

#include <stdint.h>
#include <string>
#include <vector>
#include <stdio.h>
typedef uint64_t bmap_t;
#define BITSIZE (sizeof(bmap_t)*8)    // IN bits
#define BITNSLOTS(nb) ((nb + BITSIZE - 1) / BITSIZE)

#define NO_COMPRESS_THRESHOLD 5
#define LOG_ERROR_RETURN_FAIL(...) \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
    return false

#define LOG_WARNING_RETURN_FAIL(...) \
	fprintf(stderr, __VA_ARGS__); \
	fprintf(stderr, "\n"); \
    return false

#define LOG_WARNING(...) \

namespace pfor {
class PatchedFrameOfReference {
public:

	/************USED FOR INPSEC THE PFORDETLA */
	static uint64_t B_TIMES[33] ;
	static uint64_t EXP_SIZE[129] ;
	static bool TO_PRINT_SEQ;
	static uint64_t H_BITS_EXP[33]; // 33 is highest bits expressed ;  number of exception encoded with index
	/************END OF USED FOR INPSEC THE PFORDETLA */


	static  bool rle_decode_every_batch_to_selbox_withCheck(const void *buffer,
			uint32_t buffer_capacity, uint32_t &data_size,
			uint32_t &significant_data_size, uint32_t &buffer_size, uint64_t *srcstart //PG region dimension
			, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
			, uint64_t *destcount, int dim, bmap_t **bmap);

	static bool rle_decode_every_batch_to_selbox_withoutCheck(const void *buffer,
			uint32_t buffer_capacity, uint32_t &data_size,
			uint32_t &significant_data_size, uint32_t &buffer_size, uint64_t *srcstart //PG region dimension
			, uint64_t *srccount, uint64_t *deststart //region dimension of Selection box
			, uint64_t *destcount, int dim, bmap_t **bmap);


	static bool batch_decode_within_selbox_with_checking(const void *buffer,
				uint32_t buffer_capacity,
				uint32_t &data_size, uint32_t &significant_data_size,
				uint32_t &buffer_size
				, uint64_t *srcstart //PG region dimension
				, uint64_t *srccount
				, uint64_t *deststart //region dimension of Selection box
				, uint64_t *destcount
				, int dim
				,bmap_t **bitmap);


	static bool batch_decode_within_selbox_without_checking(const void *buffer,
				uint32_t buffer_capacity,
				uint32_t &data_size, uint32_t &significant_data_size,
				uint32_t &buffer_size
				, uint64_t *srcstart //PG region dimension
				, uint64_t *srccount
				, uint64_t *deststart //region dimension of Selection box
				, uint64_t *destcount
				, int dim
				,bmap_t **bitmap);



	static bool encode(const uint32_t *data, uint32_t data_size,
			std::string &buffer);

	static bool decode(const void *buffer, uint32_t buffer_size,
				std::vector<uint32_t> &data);


	static const uint32_t kBatchSize = 128;

	/*static const uint32_t kSufficientBufferCapacity = sizeof(uint64_t)
			+ kBatchSize * sizeof(uint32_t);*/
	/*************** BOUBLE the capacity size for hybrid encoding ******************/
	static const uint32_t kSufficientBufferCapacity = sizeof(uint64_t)
				+ kBatchSize * sizeof(uint32_t) *2 ;

	static bool encode(const uint32_t *data, uint32_t data_size,
			uint32_t significant_data_size, void *buffer,
			uint32_t buffer_capacity, uint32_t &buffer_size,
			bool verify = false);

	static bool decode(const void *buffer, uint32_t buffer_capacity,
			uint32_t *data, uint32_t data_capacity, uint32_t &data_size,
			uint32_t &significant_data_size, uint32_t &buffer_size);

	static bool decode_new(const void *buffer, uint32_t buffer_capacity,
			uint32_t *data, uint32_t data_capacity, uint32_t &data_size,
			uint32_t &significant_data_size, uint32_t &buffer_size);

	static bool decode_new_to_deltas(const void *buffer, uint32_t buffer_capacity,
			uint32_t *data, uint32_t data_capacity, uint32_t &data_size,
			uint32_t &significant_data_size, uint32_t &buffer_size);

	static bool decode_set_bmap(const void *buffer, uint32_t buffer_size,
			 bmap_t ** bmap);

	static bool decode_every_batch(const void *buffer,
			uint32_t buffer_capacity,
			uint32_t &data_size, uint32_t &significant_data_size,
			uint32_t &buffer_size, bmap_t **bitmap);


#define PFOR_FIXED_LENGTH_BUFFER_SIZE(FIXED_LENGTH) \
            ((FIXED_LENGTH) * pfor::PatchedFrameOfReference::kBatchSize / 8)

	// TODO: remove minimal restriction on fixed_length by introducing auxiliary exception nodes.
	static const uint32_t kMinFixedLength = 7;

	static bool fixed_length_encode(const uint32_t *data, uint32_t data_size,
			uint32_t fixed_length, void *buffer, uint32_t buffer_size);

	static bool fixed_length_decode(const void *buffer, uint32_t buffer_size,
			uint32_t fixed_length, uint32_t *data, uint32_t data_size);

	static uint32_t get_sufficient_buffer_capacity(uint32_t bin_length);

	/****************hybrid encode & decode ****************/
	static bool hybrid_encode(const uint32_t *data, uint32_t data_size,
				uint32_t significant_data_size, void *buffer,
				uint32_t buffer_capacity, uint32_t &buffer_size,
				bool verify = false);

	static bool rle_decode_every_batch(const void *buffer,
				uint32_t buffer_capacity,
				uint32_t &data_size, uint32_t &significant_data_size,
				uint32_t &buffer_size, bmap_t **bmap);


	static bool rle_decode_every_batch_to_rids(const void *buffer,
					uint32_t buffer_capacity,
					uint32_t &data_size, uint32_t &significant_data_size,
					uint32_t &buffer_size, uint32_t *output);


	/*******************END ******************/

	/*************** expansion + runlength encode & decode ******************/

	static bool expand_runlength_encode(const uint32_t *data, uint32_t data_size,
				uint32_t significant_data_size, void *buffer,
				uint32_t buffer_capacity, uint32_t &buffer_size,
				bool verify = false);

	/*************** END ******************/


	/*************** expansion encode & decode ******************/
	static bool expand_encode(const uint32_t *data, uint32_t data_size,
			uint32_t significant_data_size, void *buffer,
			uint32_t buffer_capacity, uint32_t &buffer_size,
			bool verify = false);
	static bool expand_decode_every_batch(const void *buffer,
				uint32_t buffer_capacity,
				uint32_t &data_size, uint32_t &significant_data_size,
				uint32_t &buffer_size, bmap_t **bmap);

	static bool fixed_decode_no_exp(const void *buffer, uint32_t buffer_size,
				uint32_t fixed_length, uint32_t fof, bmap_t **bmap);

	/**************** END *********************/


	enum ExceptionType {
		EXCEPTION_UNSIGNED_CHAR = 0,
		EXCEPTION_SIGNED_CHAR,
		EXCEPTION_UNSIGNED_SHORT,
		EXCEPTION_SIGNED_SHORT,
		EXCEPTION_UNSIGNED_INT,
		EXCEPTION_SIGNED_INT
	};

    /**************** end *********************************************/


	/********************expansion encode & decode *****************/

	static bool determine_b_to_expand(const uint32_t *data, uint32_t data_size,
				uint32_t significant_data_size, uint32_t &fixed_length,
				uint32_t &frame_of_reference, ExceptionType &exception_type);

	/**************** END *********************/




	static uint32_t get_exception_value_size(ExceptionType exception_type);

	static uint32_t required_buffer_size(uint32_t fixed_length,
			uint32_t exception_count, uint32_t exception_value_size);

	static bool optimal_param(const uint32_t *data, uint32_t data_size,
			uint32_t significant_data_size, uint32_t &fixed_length,
			uint32_t &frame_of_reference, ExceptionType &exception_type);

	static bool encode_param(const uint32_t *data, uint32_t data_size,
			uint32_t significant_data_size, uint32_t fixed_length,
			uint32_t frame_of_reference, ExceptionType exception_type,
			void *buffer, uint32_t buffer_capacity, uint32_t &buffer_size,
			bool verify = true);

	static bool encode_param_new(const uint32_t *data, uint32_t data_size,
			uint32_t significant_data_size, uint32_t fixed_length,
			uint32_t frame_of_reference, ExceptionType exception_type,
			void *buffer, uint32_t buffer_capacity, uint32_t &buffer_size);


	static const uint32_t kHeaderSize = sizeof(uint64_t);


	/***********Hybrid encoding embraces pfordelta (b!=1 cases) and  counting (b=1 case)****/
	static bool encode_on_b_equal_one(const uint32_t *data, uint32_t data_size,
				uint32_t significant_data_size, uint32_t fixed_length,
				uint32_t frame_of_reference, ExceptionType exception_type,
				void *buffer, uint32_t buffer_capacity, uint32_t &buffer_size);

	/**********END***********************/




public:
	class Header {
	public:
		Header();

		bool read(const void *buffer);

		bool write(void *buffer) const;

		void print();

		uint32_t frame_of_reference_;

		uint32_t first_exception_;

		uint32_t significant_data_size_;

		uint32_t fixed_length_;

		ExceptionType exception_type_;

		uint32_t encoded_size_;
	};

	// total header size is sizeof(uint64_t)
	struct CompactHeader {
		uint32_t frame_of_reference_;

		uint32_t first_exception_ :8; // using 8 bits instead of 32bits same thing as follows

		uint32_t significant_data_size_ :8;

		uint32_t fixed_length_ :5;

		uint32_t exception_type_ :3;

		uint32_t encoded_size_ :8;
	};

private:


	template<class ExceptionValueType>
	static bool patch_typed_exceptions(uint32_t *data,
			uint32_t exception_offset, const ExceptionValueType *exceptions);

	static bool patch_exceptions(ExceptionType exception_type, uint32_t *data,
			uint32_t exception_offset, const void *exceptions);

	template<class ExceptionValueType>
	static bool patch_typed_exceptions_new(uint32_t *data,
			uint32_t exception_offset, uint32_t significant_data_size,
			const ExceptionValueType *exceptions);

	static bool patch_exceptions_new(ExceptionType exception_type,
			uint32_t *data, uint32_t exception_offset,
			uint32_t significant_data_size, const void *exceptions);

	template<class ExceptionValueType>
	static bool compute_typed_exceptions(uint32_t *adjusted_data,
			const uint32_t *exception_indexes, uint32_t exception_count,
			ExceptionValueType *exceptions);

	static bool compute_exceptions(ExceptionType exception_type,
			uint32_t *adjusted_data, const uint32_t *exception_indexes,
			uint32_t exception_count, void *exceptions);

	template<class ExceptionValueType>
	static bool decode_as_typed_exceptions(uint32_t *data,
			uint32_t significant_data_size,
			const ExceptionValueType *exceptions);

	static bool decode_as_exceptions(ExceptionType exception_type,
			uint32_t *data, uint32_t significant_data_size,
			const void *exceptions);

	template<class ExceptionValueType>
	static bool encode_as_typed_exceptions(const uint32_t *adjusted_data,
			uint32_t significant_data_size, ExceptionValueType *exceptions);

	static bool encode_as_exceptions(ExceptionType exception_type,
			const uint32_t *adjusted_data, uint32_t significant_data_size,
			void *exceptions);

	/***** b = 1 case ***/
	template<class ExceptionValueType>
	static bool exception_as_number_of_zeros_typed(
				uint32_t *adjusted_data, const uint32_t *exception_indexes,
				uint32_t exception_count, ExceptionValueType *exceptions);

	static bool exception_as_number_of_zeros(ExceptionType exception_type,
			void *exception_buffer, uint32_t *adjusted_data, const uint32_t *exception_indexes,
			uint32_t exception_count
			);

	template<class ExceptionValueType>
	static bool expand_exceptions_binary_typed(
			const uint8_t *ones, Header *h ,
			const ExceptionValueType *exceptions, bmap_t **bitmap);

	static bool expand_exceptions_binary(
			const void * buffer, Header * h , bmap_t **bitmap);


	static bool decode_ones_zeros(const void * buffer,
				Header * h, uint32_t *output);

	template<class ExceptionValueType>
	static bool decode_ones_zeros_typed(
			const uint8_t *ones, Header *h ,
			const ExceptionValueType *exceptions, uint32_t *output);


};
}

#endif
