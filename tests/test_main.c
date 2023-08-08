#ifdef ARM9
#include <nds.h>
#endif

#include "test_util.h"

#include "test_vars.h"
#include "test_strs.h"
#include "test_arrs.h"
#include "test_tokens.h"

#include "test_int_vars.h"
#include "test_int_code.h"

#include "test_console.h"

int tests(){
	FAST(test_vars());
	FAST(test_strs());
	FAST(test_arrs());
	FAST(test_tokens());
	
	FAST(test_int_vars());
	FAST(test_int_code());
	
	FAST(test_console());
	
	return 0;
}

int main(){
	#ifdef ARM9
	consoleDemoInit();
	#endif
	
	int res = tests();
	
	#ifdef ARM9
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		
		int keys = keysDown();
		if (keys & KEY_START)
			break;
	}
	#endif
	return res;
}
