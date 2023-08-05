#include "test_vars.c"
#include "test_strs.c"
#include "test_arrs.c"
#include "test_tokens.c"

#include "test_int_vars.c"
#include "test_int_code.c"

#include "test_console.c"

int main(){
	FAST(test_vars());
	FAST(test_strs());
	FAST(test_arrs());
	FAST(test_tokens());
	
	FAST(test_int_vars());
	FAST(test_int_code());
	
	FAST(test_console());
	
	return 0;
}
