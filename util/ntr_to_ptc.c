#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define SUCCESS 0
#define FAILURE 1

// Sizes listed do not include type string
/// Size in bytes of one PTC CHR resource, in bytes
#define PTC_CHR_BANK_SIZE (256*32)
/// Type string for PTC CHR file
#define PTC_CHR_TYPE "PETC0100RCHR"
/// Size in bytes of one PTC COL resource, in bytes
#define PTC_COL_BANK_SIZE (256*2)
/// Type string for PTC COL file
#define PTC_COL_TYPE "PETC0100RCOL"

// NOTE: CHAR != CHR
// CHAR is the magic ID within NCGR files.
// CHR is a filetype in PTC.
/// NCGR magic identifier
#define NCGR_MAGIC "RGCN"
/// NCGR header size, in bytes
#define NCGR_HEADER_SIZE 0x10
/// Size of CHAR header contained within NCGR file, in bytes
#define CHAR_HEADER_SIZE 0x20
/// Offset within CHAR header to size of file, in bytes
#define CHAR_SIZE_OFS 0x4
/// Length of size within CHAR header, in bytes
#define CHAR_SIZE_LEN 0x4

/// NCLR magic identifer
#define NCLR_MAGIC "RLCN"
/// NCLR header size, in bytes
#define NCLR_HEADER_SIZE 0x10
/// PLTT header size, in bytes
#define PLTT_HEADER_SIZE 0x18
/// Offset within PLTT header to size of file, in bytes
#define PLTT_SIZE_OFS 0x4
/// Length of size within PLTT header, in bytes
#define PLTT_SIZE_LEN 0x4

/// returns 0 on success, 1 on failure
int _safe_fseek(FILE* f, long ofs, int dir){
	int result = fseek(f, ofs, dir);
	if (result || ferror(f)){
		printf("Failed to seek to %ld\n", ofs);
		perror("File error");
		fclose(f);
		return FAILURE;
	}
	return SUCCESS;
}
/// Macro to return on fseek error
#define SAFE_FSEEK(f,ofs,dir) do { if (FAILURE == _safe_fseek(f, ofs, dir)) { return FAILURE; } } while(0)

/// read from file with error check
int _safe_fread(char* dest, size_t size, size_t n, FILE* f){
	unsigned int read = fread(dest, size, n, f);
	if (read != n || ferror(f)){
		printf("Failed to read %zd bytes from file (read %u)\n", size * n, read);
		perror("File error");
		fclose(f);
		return FAILURE;
	}
	return SUCCESS;
}

/// Macro to return on fread error
#define SAFE_FREAD(dest,size,n,f) do { if (FAILURE == _safe_fread(dest,size,n,f)){ return FAILURE; } } while(0)

/// write to file with error check
int _safe_fwrite(char* src, size_t size, size_t n, FILE* f){
	unsigned int write = fwrite(src, size, n, f);
	if (write != n || ferror(f)){
		printf("Failed to write %zd bytes to file (wrote %u)\n", size * n, write);
		perror("File error");
		fclose(f);
		return FAILURE;
	}
	return SUCCESS;
}

/// Macro to return on fwrite error
#define SAFE_FWRITE(src,size,n,f) do { if (FAILURE == _safe_fwrite(src,size,n,f)){ return FAILURE; } } while(0)

/// Reads 4 bytes of little-endian data as an unsigned 32-bit int
uint32_t read_u32(const char* data){
	return (uint8_t)data[0] | ((uint8_t)data[1] << 8) | ((uint8_t)data[2] << 16) | ((uint8_t)data[3] << 24);
}

int extract_nclr(FILE* f, const char* name){
	if (strlen(name) != 4){
		printf("Filename is invalid\n");
		return FAILURE;
	}
	// Skip header
	SAFE_FSEEK(f, NCLR_HEADER_SIZE, SEEK_SET);
	// Get size of PLTT from header
	char pltt_header[PLTT_HEADER_SIZE];
	SAFE_FREAD(pltt_header, sizeof(char), PLTT_HEADER_SIZE, f);

	uint32_t pltt_size = read_u32(pltt_header + CHAR_SIZE_OFS) - CHAR_HEADER_SIZE;
	char* pltt_data = malloc(pltt_size);
	SAFE_FREAD(pltt_data, sizeof(char), pltt_size, f);
	fclose(f); // done reading; data is stored in memory

	/// Construct output name
	char output_name[9] = "TTTN.PTC";
	strncpy(output_name, name, 4); // write prefix
	// Write to output file
	FILE* ptc_file = fopen(output_name, "wb");
	if (!ptc_file){
		printf("Error opening file %s for writing\n", output_name);
		return FAILURE;
	}
	// Write internal type header
	SAFE_FWRITE(PTC_COL_TYPE, sizeof(char), strlen(PTC_COL_TYPE), ptc_file);
	// Write data to file
	SAFE_FWRITE(pltt_data, sizeof(char), PTC_COL_BANK_SIZE, ptc_file);
	fclose(ptc_file); // Done writing this file
	free(pltt_data); // done with PLTT data
	return SUCCESS;
}

/// Converts NCGR file to a set of PTC files
int extract_ncgr(FILE* f, const char* prefix){
	if (strlen(prefix) != 3){
		printf("Filename prefix is invalid\n");
		return FAILURE;
	}
	// Skip header
	SAFE_FSEEK(f, NCGR_HEADER_SIZE, SEEK_SET);
	// Get size of CHAR from header
	char char_header[CHAR_HEADER_SIZE];
	SAFE_FREAD(char_header, sizeof(char), CHAR_HEADER_SIZE, f);

	uint32_t char_size = read_u32(char_header + CHAR_SIZE_OFS) - CHAR_HEADER_SIZE;
	char* char_data = malloc(char_size);
	SAFE_FREAD(char_data, sizeof(char), char_size, f);
	fclose(f); // done reading; data is stored in memory

	int ptc_chunks = char_size / PTC_CHR_BANK_SIZE;
	printf("Splitting into %d chunks\n", ptc_chunks);

	// Create output filename format
	char output_name[9] = "TTTN.PTC";
	strncpy(output_name, prefix, 3); // write prefix
	// Split NCGR into multiple PTC files
	for (int i = 0; i < ptc_chunks; ++i){
		// Determine output file
		output_name[3] = '0' + i;
		FILE* ptc_file = fopen(output_name, "wb");
		if (!ptc_file){
			printf("Error opening file %s for writing\n", output_name);
			return FAILURE;
		}
		// Write internal type header
		SAFE_FWRITE(PTC_CHR_TYPE, sizeof(char), strlen(PTC_CHR_TYPE), ptc_file);
		// Write data to file
		SAFE_FWRITE(char_data + i * PTC_CHR_BANK_SIZE, sizeof(char), PTC_CHR_BANK_SIZE, ptc_file);
		fclose(ptc_file); // Done writing this file
	}
	free(char_data); // done with CHAR data
	return SUCCESS;
}

int main(int argc, char** argv){
#ifdef DEBUG
	for (int i = 0; i < argc; ++i){
		printf("argv[%d]=%s\n", i, argv[i]);
	}
#endif

	if (argc != 3){
		printf("Usage:\n./ntr_to_ptc ncgr_file output_chr\n./ntr_to_ptc nclr_file output_col\n");
		printf("\nExamples:\n./ntr_to_ptc BGU.NCGR BGU\nThis creates BGU0.PTC through BGU3.PTC.\n./ntr_to_ptc defBGSP.NCLR COL0\nThis creates COL0.PTC.\n");
		return FAILURE;
	}

	FILE* f = fopen(argv[1], "rb");
	if (!f){
		printf("Failed to open file %s\n", argv[1]);
		return FAILURE;
	}

	char magic[4];
	SAFE_FREAD(magic, sizeof(char), 4, f);

	if (!strncmp(NCGR_MAGIC, magic, 4)){
		printf("Extracting PTCs from NCGR\n");
		int res = extract_ncgr(f, argv[2]);
		printf(res == SUCCESS ? "Success!\n" : "Error!\n");
		return res;
	}
	if (!strncmp(NCLR_MAGIC, magic, 4)){
		printf("Extracting PTC from NCLR\n");
		int res = extract_nclr(f, argv[2]);
		printf(res == SUCCESS ? "Success!\n" : "Error!\n");
		return res;
	}
	printf("Unrecognized NTR file type (expects NCGR or NCLR).\n");
	fclose(f);
	return FAILURE;
}
