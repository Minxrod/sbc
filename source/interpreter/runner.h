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
	uint_fast8_t argcount; // Never possible to exceed ~50 in PTC2
	uint_fast16_t error; // I don't even think this will reach 100 right now
	struct program* code;
};

// By putting this in ITCM we go from ~420 to ~380 on a simple speed test
void run(struct program* code, struct ptc* p) ITCM_CODE;

