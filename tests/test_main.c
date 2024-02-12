#include "common.h"

#ifndef ARM9_BUILD
#ifdef ARM9
#include <nds.h>
#include <fat.h>
#endif

#include "test_util.h"

int test_vars(void);
int test_strs(void);
int test_arrs(void);
int test_tokens(void);
int test_label(void);

int test_math(void);

int test_int_vars(void);
int test_int_code(void);
int test_int_math(void);
int test_int_func(void);

int test_input(void);
int test_console(void);
int test_background(void);
int test_sprite(void);
int test_graphics(void);
int test_resources(void);

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
	FAST(test_label());
	
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
	FAST(test_graphics());
	FAST(test_resources());
	
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
