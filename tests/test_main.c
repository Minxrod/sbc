#include "common.h"

#ifndef ARM9_BUILD
#ifdef ARM9
#include <nds.h>
#include <fat.h>
#endif

#include "test_util.h"

int test_vars();
int test_strs();
int test_arrs();
int test_tokens();

int test_math();

int test_int_vars();
int test_int_code();
int test_int_math();
int test_int_func();

int test_input();
int test_console();
int test_background();
int test_sprite();

int run_tests(){
#ifdef ARM9
	// Needed for file loading...
	fatInitDefault();
#endif
	
#ifndef EZ80
	FAST(test_vars());
	FAST(test_strs());
	FAST(test_arrs());
	FAST(test_tokens());
	
	FAST(test_math());
	
	FAST(test_int_vars());
	FAST(test_int_code());
	FAST(test_int_math());
	FAST(test_int_func());
	
#endif
	FAST(test_input());
	FAST(test_console());
	FAST(test_background());
	FAST(test_sprite());
	
	return 0;
}

int main(){
#ifdef ARM9
	consoleDemoInit();
#endif
#ifdef EZ80
	dbg_printf("Can you hear me now?\n");
#endif
	
	int res = run_tests();
	
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
