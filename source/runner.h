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
};

void run(struct program* code, struct ptc* runner);
