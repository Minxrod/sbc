#pragma once

#include "common.h"
#include <string.h> // strstr for FAST 

#ifdef PC
#include "extension/sbc_blockalloc.h"

// Initialize test environment (memory block)
// This is for tests that require use of dynamic memory
// Intended to only allocate once, and be "cleared" by MEM_CASE_END for re-use.
#define MEM_CASE do {\
	if (!sbc_membase) init_memory(MAX_MEMORY);\
} while(0);

// Clears test environment (memory block)
// This is for tests that require use of dynamic memory
// Note that this actually just resets sbc_memnext.
#define MEM_CASE_END do {\
	reset_memory();\
} while(0);

#endif // PC

#ifdef ARM9
#define MEM_CASE
#define MEM_CASE_END
#endif

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define LINE(x) LINE_STR(x)
#define LINE_STR(x) #x
#define INFO_STR __FILE__":" LINE(__LINE__)

// behold, "tests"
// Expect true
#define ASSERT(a, msg) if ((check_fail = !(a))) { printf("\033[31m"INFO_STR" Failed: "msg"\033[0m\n"); return 1; }
// Expect false
#define DENY(a, msg) if ((check_fail = a)) { printf("\033[31m"INFO_STR" Failed: "msg"\033[0m\n"); return 1; }
// Expect true; failure is permitted to continue (no return)
#define CHECK(a, msg)  if ((check_fail = !(a))) { printf("\033[31m"INFO_STR" Failed (Continued): "msg"\033[0m\n"); }
// Report all tests passed
#define SUCCESS(msg) do {\
	if (!check_fail) {\
		printf("\033[32mSuccess: "msg"\033[37m\n");\
		return 0;\
	}\
	printf("\033[31mFailed: "msg"\033[37m\n");\
	return 1;\
} while(0)

/// Determines if executable terminates on first failure or continues running
#define FASTEND 1
/// Requires char* test_list to contain list of tests 
#define FAST(f) { \
	if (!test_list || strstr(test_list, #f)){\
		iprintf("Entering "#f"\n");\
		if (f() && FASTEND) {\
			iprintf("Failure in "#f"\n");\
			return 1;\
		}\
	}\
}

// Quick checks
#define CHECK_VAR_INT(var,num) do {\
/*	iprintf(var"=%d exp=%d\n", test_var(&p->vars, var, VAR_NUMBER)->value.number, INT_TO_FP(num));*/\
	ASSERT(test_var(&p->vars, var, VAR_NUMBER)->value.number == INT_TO_FP(num), var" == "#num);\
} while(0)

#define CHECK_VAR_NUM(var,num) {ASSERT(test_var(&p->vars, var, VAR_NUMBER)->value.number == num, var" == "#num" (fixp)"); }
#define CHECK_VAR_STR(var,str) { \
/*	iprintf("check=%s\n", (char*)str_at(test_var(&p->vars, var, VAR_STRING)->value.ptr, 0));*/\
	ASSERT(str_comp(test_var(&p->vars, var, VAR_STRING)->value.ptr, str), var" == "#str);\
}

#define TEST_SEARCH_PATH "tests/data/"

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

int token_code(char* code, const char* expected, int size);
