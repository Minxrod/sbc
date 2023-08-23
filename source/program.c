#include "program.h"

#include "system.h"
#include "header.h"

#include <stdlib.h>
#include <stdio.h>

/**
 * Initializes a block of program memory.
 * @param p Program struct
 * @param prg_size Size of program (in bytes/chars).
 * 
 * The allocated memory size will be 2*prg_size. For PTC, the typical maximum
 * prg_size would be 524228, and the allocated memory is 1048456 bytes, or
 * slightly less than 1MB.
 * 
 */
void init_mem_prg(struct program* p, int prg_size){
	p->data = calloc(2, prg_size);
}


/// Allocates memory equivalent to the program size!
void prg_load(struct program* p, const char* filename){
	FILE* f = fopen(filename, "r");
	if (!f){
		iprintf("File %s load failed!\n", filename);
		fclose(f);
		abort();
	}
	struct ptc_header h;
	size_t r;
	
	/// TODO:CODE I think this only works on little-endian devices...
	/// (reading into header memory directly)
	r = fread(&h, sizeof(char), PRG_HEADER_SIZE, f);
	if (r < PRG_HEADER_SIZE || ferror(f)){
		iprintf("Could not read header correctly!\n");
		fclose(f);
		abort();
	}
	// load success: read file to buffer
	
	p->size = h.prg_size;
	p->data = malloc(h.prg_size);
	r = fread(p->data, sizeof(char), h.prg_size, f);
	if (r < h.prg_size || ferror(f)){
		iprintf("Could not read file corrcetly!\n");
		fclose(f);
		abort();
	}
	// file read success: close file
	fclose(f);
}
