#pragma once

#include "common.h"

// behold, "tests"
// Expect true
#define ASSERT(a, msg) if ((check_fail = !(a))) { printf("\033[31mFailed: "msg"\033[0m\n"); return 1; }
// Expect false
#define DENY(a, msg) if ((check_fail = a)) { printf("\033[31mFailed: "msg"\033[0m\n"); return 1; }
// Expect true; failure is permitted to continue (no return)
#define CHECK(a, msg)  if ((check_fail = !(a))) { printf("\033[31mFailed (Continued): "msg"\033[0m\n"); }
// Report all tests passed
#define SUCCESS(msg) if (!check_fail) {\
	printf("\033[32mSuccess: "msg"\033[37m\n");\
	return 0;\
} else {\
	printf("\033[31mFailed: "msg"\033[37m\n");\
	return 1;\
}

#define FASTEND 1
#define FAST(f) if (f && FASTEND) { return 1; }

// Quick checks
// TODO:CODE:LOW Add line numbers here somehow
#define CHECK_VAR_INT(var,num) {ASSERT(test_var(&p->vars, var, VAR_NUMBER)->value.number == INT_TO_FP(num), var" == "#num); }
#define CHECK_VAR_NUM(var,num) {ASSERT(test_var(&p->vars, var, VAR_NUMBER)->value.number == num, var" == "#num" (fixp)"); }
#define CHECK_VAR_STR(var,str) { \
	iprintf("%s\n", (char*)str_at(test_var(&p->vars, var, VAR_STRING)->value.ptr, 0));\
	ASSERT(str_comp(test_var(&p->vars, var, VAR_STRING)->value.ptr, str), var" == "#str);\
}

extern int check_fail;

#include "interpreter/vars.h"
//#include "interpreter.h"
//#include "system.h"
//#include "program.h"
struct ptc;

#include <string.h>
#include <stdlib.h>

struct ptc* run_code(char* code);
struct ptc* run_code_lowmem(char* code);
struct ptc* run_code_keys(char* code, char* keys, int len);
struct ptc* run_code_opts(char* code, int opts);

bool check_code_error(char* code, enum err_code expected);

void free_code(struct ptc* ptc);

int token_code(char* code, char* expected, int size);
