#pragma once

#include "common.h"

struct program;
struct ptc;

/**
 * Execution info: current instruction, stack, etc.
 */
struct runner {
	u32 index;
	u32 argcount;
	u32 error;
	struct program* code;
};

void run(struct program* code, struct ptc* runner);

// Scans for location of some instruction starting from index
// Returns the index of found string
u32 bc_scan(struct program* code, u32 index, u8 find);
