#pragma once

#include "common.h"

// Program source loaded from ex. PTC file?
struct program {
	// Size of program (characters/bytes)
	idx size;
	// Program data
	char* data;
};

void init_mem_prg(struct program* p, int prg_size);

void prg_load(struct program* p, const char* filename);
