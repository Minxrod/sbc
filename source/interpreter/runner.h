#pragma once
/// 
/// @file
/// @brief Defines the bytecode interpreter function, as well as a struct
/// containing some basic execution information
/// 

#include "common.h"

#include "program.h"

struct ptc;

/**
 * Execution info: current instruction, error status, etc.
 */
struct runner {
	idx index;
	idx data_index;
	idx data_offset; // Should never exceed ~95 anyways
	int argcount; // Never possible to exceed ~50 in PTC2
	u32 error; // I don't even think this will reach 100 right now
	char error_info[32];
	struct bytecode code; // Stored in entirety (this only contains pointers)
	struct program prg; // points into same array as bytecode
};

// Gains about 10% speed by being placed in ITCM_CODE
void run(struct bytecode code, struct ptc* p) ITCM_CODE;

void cmd_exec(struct ptc* p);
void cmd_run(struct ptc* p);

void sys_err(struct ptc* p);
void sys_erl(struct ptc* p);
