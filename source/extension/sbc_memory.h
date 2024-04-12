#pragma once

#include "common.h"

struct ptc;

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

void cmd_poke(struct ptc* p);
void cmd_pokeh(struct ptc* p);
void cmd_pokeb(struct ptc* p);

void cmd_memcopy(struct ptc* p);
void cmd_memfill(struct ptc* p);

void func_peek(struct ptc* p);
void func_peekh(struct ptc* p);
void func_peekb(struct ptc* p);

void func_addr(struct ptc* p);
void func_ptr(struct ptc* p);

void sys_memsafe(struct ptc* p);
void syschk_memsafe(struct ptc* p);
