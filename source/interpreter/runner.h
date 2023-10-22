#pragma once

#include "common.h"

struct program;
struct ptc;

/**
 * Execution info: current instruction, stack, etc.
 */
struct runner {
	idx index;
	idx data_index;
	idx data_offset; // Should never exceed ~95 anyways
	u8 argcount; // Never possible to exceed ~50 in PTC2
	u16 error; // I don't even think this will reach 100 right now
	struct program* code;
};

void run(struct program* code, struct ptc* p);


