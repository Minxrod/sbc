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
/// Note that some defines require a `struct ptc* p` to work as expected.
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
#include <assert.h>

/// Fractional bits in fixed point numbers
#define FIXPOINT 12
/// Takes a fixed point number and gets the integer portion
#define FP_TO_INT(num) ((num) >> FIXPOINT)
/// Takes an integer and converts it to fixed point
#define INT_TO_FP(num) ((fixp)(((uint32_t)(num)) << FIXPOINT))
/// Constant for one (20.12 fixed point)
#define FIXP_1 INT_TO_FP(1)

/// Kills the program with a message
/// For unrecoverable failures and those that aren't properly handled yet
#define ABORT(msg) { iprintf(msg"\n"); abort(); }

// The following defines expect to be used within a function that has
// access to `struct ptc* p`, for error setting and other checks.

/// For errors that can be handled by the interpreter
#define ERROR(code) do { p->exec.error = code; return; } while (0)
/// To read an argument by index on the stack
#define ARG(index) (stack_get(&p->stack, index))

#ifndef ARM9
/// Places the code in ITCM on NDS
/// @note Ignored for non-NDS platforms
#define ITCM_CODE
/// Places the code in DTCM on NDS
/// @note Ignored for non-NDS platforms
#define DTCM_DATA
/// Places the code in DTCM on NDS
/// @note Ignored for non-NDS platforms
#define DTCM_BSS
#endif

#ifdef ARM9
#include <nds/ndstypes.h>
#include <nds.h>
#endif

#ifndef ARM9
/// 8-bit unsigned int type
typedef uint8_t u8;
/// 16-bit unsigned int type
typedef uint16_t u16;
/// 32-bit unsigned int type
typedef uint32_t u32;
/// 8-bit signed int type
typedef int8_t s8;
/// 16-bit signed int type
typedef int16_t s16;
/// 32-bit signed int type
typedef int32_t s32;
// Macro to convert iprintf to printf, used on non-NDS platforms
#define iprintf printf
#endif

#ifdef NDEBUG
#ifdef iprintf
#undef iprintf
#endif
#define iprintf if(0)printf
#endif

/// Type to handle instruction indexes
typedef uint32_t idx;
/// Type to indicate a fixed-point number. This is expected to be 20.12
typedef int32_t fixp;

#ifdef EZ80
#include <debug.h>
#undef iprintf
#define iprintf dbg_printf
#endif

#ifdef PC
#include "sbc_blockalloc.h"
#endif

/// Logged memory management function
static inline void* malloc_log(char* msg, size_t size){
#ifdef PC
	void *ptr = sbc_malloc(size);
#else
	void* ptr = malloc(size);
#endif
	iprintf("%s malloc'd %zu at %p\n", msg, size, ptr);
	return ptr;
}

/// Logged memory management function
static inline void* calloc_log(char* msg, size_t num, size_t size){
#ifdef PC
	void *ptr = sbc_calloc(num, size);
#else
	void* ptr = calloc(num, size);
#endif
	iprintf("%s calloc'd %zu(%zu*%zu) at %p\n", msg, num*size, num, size, ptr);
	return ptr;
}

/// Logged memory management function
static inline void free_log(char* msg, void* ptr){
	iprintf("%s free'd %p\n", msg, ptr);
#ifdef PC
	sbc_free(ptr);
#else
	free(ptr);
#endif
}

#ifndef __GNUC__
// Note: creates various static copies - not a huge deal, but does waste some memory...
// loosely based on ideas from https://stackoverflow.com/questions/2100331/macro-definition-to-determine-big-endian-or-little-endian-machine
#ifndef LITTLE_ENDIAN
/// Placed in memory as 04 03 02 01 if LE
static const uint32_t endian = 0x01020304;

/// Macro that evaluates to true on platforms that store numbers as little-endian
/// Used to validate certain dubious "read file into memory" tricks as actually working
#define LITTLE_ENDIAN (\
	(((char*)&endian)[0] == 4) &&\
	(((char*)&endian)[1] == 3) &&\
	(((char*)&endian)[2] == 2) &&\
	(((char*)&endian)[3] == 1)\
)
#endif
#else // GCC specific extension endian check (but doesn't waste memory)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN 1
#else
#define LITTLE_ENDIAN 0
#endif // __BYTE_ORDER__
#endif // __GNUC__
