#pragma once

int sbc_cache_search(unsigned char value, unsigned char* cache, int cache_size);

// TODO rename
void sbc_write_bits(unsigned char* output_data, int* output_index, int value, int bits);

void sbc_write_bit(unsigned char* output_data, int* output_index, int value);

int sbc_compress(unsigned char* source_data, unsigned char* output_data, int source_size, int cache_size, int unit_size);

struct sbc_compression_result {
	int size;
	int bits;
	unsigned char* output;
};

// Allocates memory to char* output
struct sbc_compression_result sbc_optimal_compress(unsigned char* source_data, int source_size, int unit_size);

int sbc_read_bits(unsigned char* source_data, int* bit_index, int bits);

// TODO unit size?
// TODO Read version with header
unsigned char* sbc_decompress(unsigned char* source_data, int decompressed_size, int cache_bits);
