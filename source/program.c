#include "program.h"

#include "system.h"
#include "header.h"
#include "ptc.h"
#include "interpreter/label.h"

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
	iprintf("init_mem_prg calloc: %d\n", prg_size*2);
	p->size = 0;
	p->data = calloc(2, prg_size);
}

struct bytecode init_bytecode(int bc_max_size){
	iprintf("init_bytecode calloc: %d\n", bc_max_size*2);
	// labels struct is expected to be zero-initialized. Others, it may not be necessary.
	struct bytecode bc = {
		0, calloc(2, bc_max_size), calloc(1, MAX_LINES), calloc(1, sizeof(struct labels))
	};
	assert(bc.data);
	assert(bc.labels);
	assert(bc.line_length);
	return bc;
}

void free_bytecode(struct bytecode bc){
	free(bc.data);
	free(bc.line_length);
	free(bc.labels);
}

// Scans for a specific instruction. Special purpose function to handle the
// various instruction lengths.
idx bc_scan(struct bytecode code, idx index, u8 find){
	// search for find in code.data
	while (index < code.size){ 
		u8 cur = code.data[index];
//		iprintf("%d: %c,%d", (int)index, cur >= 32 ? cur : '?', code->data[index+1]);
		if (cur == find){
			return index;
		}
		//debug! 
		u8 len = code.data[++index];
//		iprintf("/%.*s\n", len, &code->data[index+1]);
		if (cur == BC_STRING || cur == BC_LABEL || cur == BC_LABEL_STRING || cur == BC_DATA){
			index++;
			index += len + (len & 1);
		} else if (cur == BC_WIDE_STRING){
			index++;
			index += sizeof(u16) * len;
		} else if (cur == BC_DIM || cur == BC_VARIABLE_NAME){
			cur = len;
			++index;
			if (cur >= 'A'){
			} else {
				index += len + (len & 1);
			}
		} else if (cur == BC_NUMBER){
			index += 6-1; // change if number syntax gets modified?
		} else {
			index += 2-1;
		}
	}
	return BC_SCAN_NOT_FOUND;
}

/// @warning Only capable of finding instructions that have size of two bytes.
/// Such as BC_COMMAND, BC_FUNCTION, etc.
/*idx bc_scan_2(struct bytecode code, idx index, u8 instr, u8 data){
	while (index < code->size){
		index = bc_scan(code, index, instr);
		if (index == BC_SCAN_NOT_FOUND) return index; // not found
		else if (code->data[index+1] == data) return index; // found exact match
		else index += 2; // found instruction but not data; continue search
	}
	return BC_SCAN_NOT_FOUND;
}*/


/// Allocates memory equivalent to the program size!
void prg_load(struct program* p, const char* filename){
	FILE* f = fopen(filename, "r");
	if (!f){
		iprintf("File %s load failed!\n", filename);
		abort();
	}
	struct ptc_header h;
	size_t r;
	
	/// TODO:CODE:MED I think this only works on little-endian devices...
	/// (reading into header memory directly)
	r = fread(&h, sizeof(char), PRG_HEADER_SIZE, f);
	if (r < PRG_HEADER_SIZE || ferror(f)){
		iprintf("Could not read header correctly!\n");
		fclose(f);
		abort();
	}
	// load success: read file to buffer
	
	p->size = h.prg_size;
	iprintf("prg_load malloc: %d\n", (int)h.prg_size);
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
