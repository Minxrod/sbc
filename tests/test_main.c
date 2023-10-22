#include "common.h"

#ifndef ARM9_BUILD
#ifdef ARM9
#include <nds.h>
#include <fat.h>
#endif

#include "test_util.h"

#ifndef EZ80
#include "test_vars.h"
#include "test_strs.h"
#include "test_arrs.h"
#include "test_tokens.h"

#include "test_math.h"

#include "test_int_vars.h"
#include "test_int_code.h"
#endif

#include "test_input.h"
#include "test_console.h"

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
	
#endif
	FAST(test_input());
	FAST(test_console());
	
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
