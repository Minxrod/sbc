#pragma once

#include "common.h"

struct labels;

#define MAX_LINES 10000

// Program source loaded from ex. PTC file?
struct program {
	// Size of program (characters/bytes)
	idx size;
	// Program data
	char* data;
};

// Compiled program code
struct bytecode {
	// Size of bytecode
	idx size;
	// Program data (pairs of instructions and data)
	u8* data;
//	// Can be used to traverse line-by-line
	u8* line_length;
	// 
	struct labels* labels;
};

void init_mem_prg(struct program* p, int prg_size);
struct bytecode init_bytecode(void);
void free_bytecode(struct bytecode);

struct program init_ptr_prg(char* c);

bool prg_load(struct program* p, const char* filename);
bool load_prg(struct program* p, const char* filename);

// Scans for location of some instruction starting from index
// Returns the index of found string
idx bc_scan(struct bytecode code, idx index, u8 find) ITCM_CODE;
//idx bc_scan_2(struct bytecode code, idx index, u8 instr, u8 data);

#include <limits.h>
#define BC_SCAN_NOT_FOUND UINT_MAX
