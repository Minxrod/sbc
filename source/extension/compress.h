#pragma once

int sbc_cache_search(unsigned char value, const unsigned char* const cache, int cache_size);

void sbc_write_bits(unsigned char* output_data, int* output_index, int value, int bits);

void sbc_write_bit(unsigned char* output_data, int* output_index, int value);

int sbc_compress(const unsigned char* const source_data, unsigned char* output_data, int source_size, int cache_size, int unit_size);

/// Stores the result of a call to sbc_optimal_compress.
/// 
/// This contains all the information needed to create an SBC compressed file.
struct sbc_compression_result {
	/// Size of compressed result
	int size;
	/// Size of compression cache, in bits
	int bits;
	/// Pointer to output compressed data
	unsigned char* output;
};

// Allocates memory to char* output
struct sbc_compression_result sbc_optimal_compress(const unsigned char* const source_data, int source_size, int unit_size);

int sbc_read_bits(const unsigned char* const source_data, int* bit_index, int bits);

unsigned char* sbc_decompress(const unsigned char* const source_data, unsigned char* dest, int decompressed_size, int cache_bits);
