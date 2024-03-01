#pragma once
/// 
/// @file
/// @brief Some commonly used structures and definitions, as well as some 
/// frequently used headers. Ensures various type definitions and
/// macros are well-defined across platforms.
/// 
/// This file defines macros for fixed-point number usage, error handling,
/// and argument access.
/// 
/// It also contains typedefs for short fixed-length types
/// such as u8, u16, etc. for platforms that do not define these by default.
/// (libnds provides these normally, but the PC build must re-define them.)
/// Additionally, idx and fixp types are defined for clearer intention for some
/// variables.
/// 
/// This file also contains wrapper functions for logging memory allocations,
/// via `malloc_log`, `calloc_log`, and `free_log`.

#include <stdint.h>
#include <stdio.h> // debugging
#include <stdlib.h>
#include <stdbool.h>

#define FIXPOINT 12
// Takes a fixed point number and gets the integer portion
#define FP_TO_INT(num) ((num) >> FIXPOINT)
// Takes an integer and converts it to fixed point
#define INT_TO_FP(num) ((num) << FIXPOINT)
// Creates a fixed point number from integer and fractional components

// for bad failures 
// and those that aren't properly handled yet...
#define ABORT(msg) { iprintf(msg"\n"); abort(); }

/// The following defines expect to be used within a function that has 
/// access to `struct ptc* p`, for error setting and other checks.

// For errors that can be handled by the interpreter
#define ERROR(code) { p->exec.error = code; return; }
// To read an argument on the stack
#define ARG(index) (stack_get(&p->stack, index))

#ifndef ARM9
// ignored for non-arm9 platforms
#define ITCM_CODE
#define DTCM_DATA
#define DTCM_BSS
#endif

#ifdef ARM9
#include <nds/ndstypes.h>
#include <nds.h>

#ifdef NDEBUG
#define iprintf if(0)iprintf
#endif
#endif

#ifndef ARM9
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
#ifdef NDEBUG
#define iprintf if(0)printf
#else
#define iprintf printf
#endif
#endif

// Type to handle instruction indexes
// Should correspond to realistic sizes for each platform
typedef unsigned int idx;
typedef signed int fixp;

#ifdef EZ80
#include <debug.h>
#undef iprintf
#define iprintf dbg_printf
#endif

#ifdef PC
#include "sbc_blockalloc.h"
#endif

static inline void* malloc_log(char* msg, size_t size){
#ifdef PC
	void *ptr = sbc_malloc(size);
#else
	void* ptr = malloc(size);
#endif
	iprintf("%s malloc'd %zu at %p\n", msg, size, ptr);
	return ptr;
}

static inline void* calloc_log(char* msg, size_t num, size_t size){
#ifdef PC
	void *ptr = sbc_calloc(num, size);
#else
	void* ptr = calloc(num, size);
#endif
	iprintf("%s calloc'd %zu(%zu*%zu) at %p\n", msg, num*size, num, size, ptr);
	return ptr;
}

static inline void free_log(char* msg, void* ptr){
	iprintf("%s free'd %p\n", msg, ptr);
#ifdef PC
	sbc_free(ptr);
#else
	free(ptr);
#endif
}


