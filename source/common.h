#pragma once

#include <stdint.h>
#include <stdio.h> // debugging
#include <stdlib.h>

#define FIXPOINT 12
// Takes a fixed point number and gets the integer portion
#define FP_TO_INT(num) (num >> FIXPOINT)
// Takes an integer and converts it to fixed point
#define INT_TO_FP(num) (num << FIXPOINT)
// Creates a fixed point number from integer and fractional components

// TODO:CODE Create floating point type alias to allow easier change later

// for bad failures 
// and those that aren't properly handled yet...
#define ABORT(msg) { iprintf(msg"\n"); abort(); }

/// The following defines expect to be used within a function that has 
/// access to `struct ptc* p`, for error setting and other checks.

// For errors that can be handled by the interpreter
#define ERROR(code) { p->exec.error = code; return; }
// To read an argument on the stack
#define ARG(index) (stack_get(&p->stack, index))


#ifdef ARM9
#include <nds/ndstypes.h>
#include <nds.h>

// Use this to build the "Main program" version of the software
// Instead of the test cases
// TODO:CODE Fix the libnds Makefile to render this unnecessary...
// (want separate tests / main builds)
#endif

#ifndef ARM9
#include <stdbool.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
#define iprintf printf
#endif

#ifdef EZ80
#include <debug.h>
#undef iprintf
#define iprintf dbg_printf
#endif
