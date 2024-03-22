#include "common.h"

#ifndef ARM9_BUILD
#ifdef ARM9
#include <nds.h>
#include <fat.h>
#endif

#include <limits.h>

#include "test_util.h"

int test_vars(void);
int test_strs(void);
int test_arrs(void);
int test_tokens(void);
int test_label(void);

int test_math(void);

// int test_compile(void);
int test_transpile(void);

int test_int_vars(void);
int test_int_code(void);
int test_int_math(void);
int test_int_func(void);
int test_int_operator(void);

int test_input(void);
int test_console(void);
int test_background(void);
int test_sprite(void);
int test_graphics(void);
int test_resources(void);

int run_tests(int argc, char** argv){
#ifdef ARM9
	// Needed for file loading...
	fatInitDefault();
#endif
	// Select tests to run (default: all)
	char* test_list = NULL; // FAST checks for null
	if (argc == 2){
		iprintf("Selected tests: %s\n", argv[1]);
		test_list = argv[1];
	} else if (argc >= 2){
		iprintf("usage: test 'test_one test_two ...'\nif no list is provided, runs every test\n");
		return 1;
	}
	
	// assumptions
	static_assert(CHAR_BIT == 8);
	
	FAST(test_vars);
	FAST(test_strs);
	FAST(test_arrs);
	FAST(test_tokens);
	FAST(test_label);
	
	FAST(test_math);
	
	FAST(test_int_vars);
	FAST(test_int_code);
	FAST(test_int_operator);
	FAST(test_int_math);
	FAST(test_int_func);
	
	FAST(test_input);
	FAST(test_console);
	FAST(test_background);
	FAST(test_sprite);
	FAST(test_graphics);
	FAST(test_resources);
	
	return 0;
}

int main(int argc, char** argv){
#ifdef ARM9
	consoleDemoInit();
#endif
#ifdef EZ80
	dbg_printf("Can you hear me now?\n");
#endif
	
	int res = run_tests(argc, argv);
	
#ifdef ARM9
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		
//		iprintf("Continue?\n");
		int keys = keysDown();
		if (keys & KEY_START)
			break;
	}
#endif
	return res;
}
#endif
