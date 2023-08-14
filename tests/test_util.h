#pragma once

#include "common.h"

// behold, "tests"
// Expect true
#define ASSERT(a, msg) if ((check_fail = !(a))) { iprintf("\033[31mFailed: "msg"\033[0m\n"); return 1; }
// Expect false
#define DENY(a, msg) if ((check_fail = a)) { iprintf("\033[31mFailed: "msg"\033[0m\n"); return 1; }
// Expect true; failure is permitted to continue (no return)
#define CHECK(a, msg)  if ((check_fail = !(a))) { iprintf("\033[31mFailed (Continued): "msg"\033[0m\n"); }
// Report all tests passed
#define SUCCESS(msg) if (!check_fail) {\
	iprintf("\033[32mSuccess: "msg"\033[37m\n");\
	return 0;\
} else {\
	iprintf("\033[31mFailed: "msg"\033[37m\n");\
	return 1;\
}

#define FASTEND 1
#define FAST(f) if (f && FASTEND) { return 1; }

extern int check_fail;

#include "program.h"
#include "vars.h"
#include "tokens.h"
#include "strs.h"
#include "arrays.h"
#include "system.h"
#include "runner.h"

#include <string.h>
#include <stdlib.h>

struct ptc* run_code(char* code);
void free_code(struct ptc* ptc);

int token_code(char* code, char* expected, int size);
