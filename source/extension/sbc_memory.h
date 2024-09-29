#pragma once

#include "common.h"

struct sbc;

/// Struct to contain memory-related values useful to the SBC memory functions.
/// 
/// This only contains the memsafe sysvar, currently.
struct memory_api {
	/// System variable that enables pointer validation checks when set.
	/// Depending on the function, this can enable some weak bounds checks,
	/// checks for overlapping ranges, and misaligned pointers.
	/// 
	/// This also functionally limits the range of memory available to that
	/// which is part of internal interpreter state, and can be made to work
	/// cross-platform with some creative pointer arithmetic.
	fixp sys_memsafe;
};

void cmd_poke(struct sbc* p);
void cmd_pokeh(struct sbc* p);
void cmd_pokeb(struct sbc* p);

void cmd_memcopy(struct sbc* p);
void cmd_memfill(struct sbc* p);

void func_peek(struct sbc* p);
void func_peekh(struct sbc* p);
void func_peekb(struct sbc* p);

void func_addr(struct sbc* p);
void func_ptr(struct sbc* p);

void sys_memsafe(struct sbc* p);
void syschk_memsafe(struct sbc* p);
