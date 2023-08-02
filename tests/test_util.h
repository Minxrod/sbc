#pragma once

// behold, "tests"
// Expect true
#define ASSERT(a, msg) if ((check_fail = !(a))) { printf("\033[31mFailed: "msg"\033[0m\n"); return 1; }
// Expect false
#define DENY(a, msg) if ((check_fail = a)) { printf("\033[31mFailed: "msg"\033[0m\n"); return 1; }
// Expect true; failure is permitted to continue (no return)
#define CHECK(a, msg)  if ((check_fail = !(a))) { printf("\033[31mFailed (Continued): "msg"\033[0m\n"); }
// Report all tests passed
#define SUCCESS(msg) if (!check_fail) {\
	printf("\033[32mSuccess: "msg"\033[0m\n");\
	return 0;\
} else {\
	printf("\033[31mFailed: "msg"\033[0m\n");\
	return 1;\
}

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

void run_code(char* code, struct ptc* ptc);
void free_code(struct ptc* ptc);
