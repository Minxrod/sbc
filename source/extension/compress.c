#include "compress.h"

#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

char* bin[] = {
	"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
	"1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"
};

int sbc_cache_search(unsigned char value, const unsigned char* const cache, int cache_size){
	for (int i = 0; i < cache_size; ++i){
		if (value == cache[i]){
			return i;
		}
	}
	return -1;
}

void sbc_write_bits(unsigned char* output_data, int* output_index, int value, int bits){
	value <<= 8 - (*output_index % 8) + (8 - bits);
	int value_high = value >> 8;
	int value_low = value & 0xff;
	output_data[*output_index / 8 + 0] |= value_high;
	output_data[*output_index / 8 + 1] |= value_low;
	*output_index += bits;
}

void sbc_write_bit(unsigned char* output_data, int* output_index, int value){
	sbc_write_bits(output_data, output_index, value, 1);
}

// cache_size = number of bits for cache indexes
// unit_size = size of units being compressed
int sbc_compress(const unsigned char* const source_data, unsigned char* output_data, int source_size, int cache_size, int unit_size){
	if (cache_size == -1){
		// Don't compress
		memcpy(output_data, source_data, source_size);
		return source_size;
	}
	memset(output_data, 0, source_size);
	
	unsigned char cache[128] = {0};
	const int cache_maximum = 1 << cache_size;
	for (int i = 0; i < cache_maximum; ++i){
		cache[i] = i;
	}
	
	int cache_next_slot = 0;
	int source_index = 0;
	int output_index = 0;
	while (source_index < source_size && (output_index / 8 + (output_index % 8 != 0)) < source_size){
		int unit = source_data[source_index];
		++source_index;
		int cache_index = sbc_cache_search(unit, cache, cache_maximum);
		if (cache_index > -1){
			// Found a previous value
			sbc_write_bit(output_data, &output_index, 0);
			sbc_write_bits(output_data, &output_index, cache_index, cache_size);
		} else {
			// Did not find previous value
			sbc_write_bit(output_data, &output_index, 1);
			sbc_write_bits(output_data, &output_index, unit, unit_size);
			// Add cache entires in a loop
			cache[cache_next_slot] = unit;
			cache_next_slot = (cache_next_slot + 1) % cache_maximum;
		}
	}
	if (output_index / 8 + (output_index % 8 != 0) >= source_size){
		// Did not compress better than source
		return -1;
	}
	// return size of output data
	return output_index / 8 + (output_index % 8 != 0);
}

// Allocates memory to char* output
struct sbc_compression_result sbc_optimal_compress(const unsigned char* const source_data, int source_size, int unit_size){
	struct sbc_compression_result result;
	result.output = calloc_log("sbc_optimal_compress", 1, source_size); // only allow results smaller than size
	result.size = source_size;
	result.bits = -1;
	for (int bits = 0; bits < 8; ++bits){
		int bits_size = sbc_compress(source_data, result.output, source_size, bits, unit_size);
		if (bits_size != -1 && bits_size < result.size){
			result.size = bits_size;
			result.bits = bits;
		}
		printf("cache bits: %d size: %d\n", bits, bits_size);
	}
	// Ensure the allocated storage of optimal_compress is actualy optimal
	sbc_compress(source_data, result.output, source_size, result.bits, unit_size);
	return result;
}

int sbc_read_bits(const unsigned char* const source_data, int* bit_index, int bits){
	assert(bits <= 8);
	int byte_index = *bit_index / 8;
	int bit = *bit_index % 8;
	*bit_index += bits;
	if (bit + bits <= 8){
		int num = source_data[byte_index];
		num >>= 8 - bit - bits;
		return num & ((1 << bits) - 1);
	} else {
		int num = source_data[byte_index] << 8;
		num += source_data[byte_index+1];
		num >>= 16 - bit - bits;
		return num & ((1 << bits) - 1);
	}
	//1yyyy
	//  1yyyy
	//     1yyy y
	// 1xxxxxxx x
	//  1xxxxxx xx
	//      1xx xxxxxx
}

// TODO:IMPL:MED decompress into pre-alloc'd memory version
unsigned char* sbc_decompress(const unsigned char* const source_data, int decompressed_size, int cache_bits){
	// Cache info
	unsigned char cache[128] = {0};
	const int cache_maximum = 1 << cache_bits;
	for (int i = 0; i < cache_maximum; ++i){
		cache[i] = i;
	}
	
	int cache_next_slot = 0;
	
	unsigned char* output = malloc_log("sbc_decompress", decompressed_size); // region will be entirely filled
	
	int source_bit = 0;
	for (int i = 0; i < decompressed_size; ++i){
		if (sbc_read_bits(source_data, &source_bit, 1)){
			// literal
			output[i] = sbc_read_bits(source_data, &source_bit, 8);
//			printf("%s%s \n", bin[((unsigned char)output[i]) >> 4], bin[output[i] & 0xf]);
			cache[cache_next_slot] = output[i];
			cache_next_slot = (cache_next_slot + 1) % cache_maximum;
		} else {
			// cache
			output[i] = cache[sbc_read_bits(source_data, &source_bit, cache_bits)];
		}
//		printf("%d,",output[i]);
	}
	printf("\n");
	
	return output;
}

/*int main(int argc, char** argv){
	if (argc < 2){
//		printf("usage: compress string\n");
		return 1;
	}
//	char* data = argv[1];
//	int source_size = strlen(data);
	FILE* source_file = fopen(argv[1], "rb");
	
	fseek(source_file, 0L, SEEK_END);
	int source_size = ftell(source_file);
	unsigned char* data = calloc_log("compress test main", source_size, 1);
	fseek(source_file, 0L, SEEK_SET);
	fread(data, 1, source_size, source_file);
	fclose(source_file);
	printf("%s\n", data);
	
	// Compression
	struct compression_result result = optimal_compress(data, source_size, 8);
	
	// Display result
	for (int i = 0; i < result.size; ++i){
		printf("%s%s", bin[((unsigned char)result.output[i]) >> 4], bin[result.output[i] & 0xf]);
	}
	printf("\ncompressed/original: %d/%d bytes cache: %d bits\n", result.size, source_size, result.bits);
	
	char* original = decompress(result.output, source_size, result.bits);
	
//	printf("\noriginal: %s\n", original);
	// Cleanup
	free_log("compress main", result.output);
	free_log("compress main", original);
}*/
