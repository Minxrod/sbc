#include "test_vars.c"
#include "test_strs.c"
#include "test_arrs.c"
#include "test_tokens.c"

#include "test_int_vars.c"

int main(){
	test_vars();
	test_strs();
	test_arrs();
	test_int_vars();
	test_tokens();
	
	return 0;
}
