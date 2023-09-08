#pragma once

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
// Validate number of arguments is expected
#define ARGCHECK(expected) { if (p->stack.stack_i != expected) { p->exec.error = ERR_WRONG_ARG_COUNT; return; }}

#ifdef ARM9
#include <nds/ndstypes.h>
#include <nds.h>

// Use this to build the "Main program" version of the software
// Instead of the test cases
#endif

#ifndef ARM9
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
#define iprintf printf
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
