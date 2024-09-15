#include "program.h"

#include "system.h"
#include "header.h"
#include "ptc.h"
#include "interpreter/label.h"
#include "common.h"

#include <stdlib.h>
#include <stdio.h>

/**
 * Initializes a block of program memory.
 * @param p Program struct
 * @param prg_size Size of program (in bytes/chars).
 */
void init_mem_prg(struct program* p, int prg_size){
	p->size = 0;
	p->data = calloc_log("init_mem_prg", 1, prg_size);
}

struct bytecode init_bytecode(void){
	// labels struct is expected to be zero-initialized. Others, it may not be necessary.
	struct bytecode bc = {
		0,
		calloc_log("init_bytecode", 1, MAX_SOURCE_SIZE),
		calloc_log("init_bytecode", 1, MAX_LINES),
		init_labels(MAX_LABELS)
	};
	assert(bc.data);
	assert(bc.line_length);
	assert(bc.labels.entry);
	return bc;
}

/// @param size Size of bytecode buffer
struct bytecode init_bytecode_size(int size, int lines, int labels){
	// labels struct is expected to be zero-initialized. Others, it may not be necessary.
	struct bytecode bc = {
		0,
		calloc_log("init_bytecode_size", 1, size),
		calloc_log("init_bytecode", 1, lines),
		init_labels(labels)
	};
	assert(bc.data);
	assert(bc.line_length);
	assert(bc.labels.entry || !labels);
	return bc;
}

void free_bytecode(struct bytecode bc){
	free_log("free_bytecode", bc.data);
	free_log("free_bytecode", bc.line_length);
	free_labels(bc.labels);
}

struct bytecode_params calc_min_bytecode(struct program* p){
	struct bytecode_params params = {0};
	bool first_of_line = true;
	for (uint32_t i = 0; i < p->size; ++i){
		// Count line endings == number of lines
		if (p->data[i] == '\r'){
			++params.lines;
			first_of_line = true;
		} else if (first_of_line && p->data[i] == '@'){
			// Count labels (only definable at beginning of line)
			++params.labels;
		} else if (p->data[i] != ' '){
			first_of_line = false;
		}
	}
	// determine necessary labels size
	params.labels--;
	params.labels |= params.labels >> 16;
	params.labels |= params.labels >> 8;
	params.labels |= params.labels >> 4;
	params.labels |= params.labels >> 2;
	params.labels |= params.labels >> 1;
	params.labels++;

	return params;
}

// Scans for two instructions, searching for the second and storing the prior one as well
struct idx_pair bc_scan_pair(struct bytecode code, idx index, u8 find){
	// search for find in code.data
	struct idx_pair result = { BC_SCAN_NOT_FOUND, index };
	while (index < code.size){ 
		u8 cur = code.data[index];
//		iprintf("%d: %c,%d", (int)index, cur >= 32 ? cur : '?', code->data[index+1]);
		if (cur == find/* || (find == BC_COMMAND && cur == BC_COMMAND_FIRST)*/){
			result.index = index;
			return result;
		}
		result.prev = index;
		//debug!
		u8 len = code.data[++index];
//		iprintf("/%.*s\n", len, &code->data[index+1]);
		if (cur == BC_STRING || cur == BC_LABEL || cur == BC_LABEL_STRING || cur == BC_DATA){
			index++;
			index += len + (len & 1);
		} else if (cur == BC_WIDE_STRING){
			index++;
			index += sizeof(u16) * len;
		} else if (cur == BC_DIM || cur == BC_VARIABLE_NAME || cur == BC_ARRAY_NAME){
			cur = len;
			++index;
			if (cur < 'A'){
				index += len + (len & 1);
			}
		} else if (cur == BC_NUMBER){
			index += BC_NUMBER_SIZE-1; // change if number syntax gets modified?
		} else if (cur == BC_VARIABLE_ID){
			index += BC_INSTR_SIZE-1+2;
		} else {
			index += BC_INSTR_SIZE-1;
		}
	}
	result.index = BC_SCAN_NOT_FOUND;
	return result;
}

// Scans for a specific instruction. Special purpose function to handle the
// various instruction lengths.
idx bc_scan(struct bytecode code, idx index, u8 find){
	// search for find in code.data
	return bc_scan_pair(code, index, find).index;
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

bool load_prg_internal(struct program* p, const char* filename, bool alloc){
	p->size = 0;
	FILE* f = fopen(filename, "r");
	if (!f){
		iprintf("File %s load failed!\n", filename);
		return false;
	}
	struct ptc_header h;
	size_t r;
	
	assert(LITTLE_ENDIAN);
	// only works on little-endian devices...
	// (reading into header memory directly)
	r = fread(&h, sizeof(char), PRG_HEADER_SIZE, f);
	if (r < PRG_HEADER_SIZE || ferror(f)){
		iprintf("Could not read header correctly!\n");
		fclose(f);
		return false;
	}
	if (h.type_str[9] == 'P' && h.type_str[10] == 'R' && h.type_str[11] == 'G'){
		// load success: read file to buffer
	} else {
		fclose(f);
		iprintf("Not a program file!\n");
		return false;
	}
	
	p->size = h.prg_size;
	if (alloc){
		p->data = malloc_log("prg_load", h.prg_size);
	}
	r = fread(p->data, sizeof(char), h.prg_size, f);
	if (r < h.prg_size || ferror(f)){
		iprintf("Could not read file corrcetly!\n");
		fclose(f);
		if (alloc){
			free_log("prg_load", p->data);
		}
		return false;
	}
	// file read success: close file
	fclose(f);
	return true;

}

/// Allocates memory equivalent to the program size on success
/// Returns true on success, and false on failure
bool load_prg_alloc(struct program* p, const char* filename){
	p->data = NULL;
	return load_prg_internal(p, filename, true);
}

bool load_prg(struct program* p, const char* filename){
	return load_prg_internal(p, filename, false);
}
