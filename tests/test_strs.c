#include "strs.h"

int test_strs(){
	/*
	string creation tests
	*/
	// Create string slots
	{
		struct strings strs;
		const u32 max = 8;
		
		init_mem_str(&strs, max, STRING_CHAR);
		
		ASSERT(strs.strs_max == max, "[init_mem_str] Correct string count");
		ASSERT(strs.str_data != NULL, "[init_mem_str] Successful memory allocation");
	}
	
	// Get unused string slots
	{
		struct strings strs;
		const u32 max = 8;
		
		init_mem_str(&strs, max, STRING_CHAR);
		
		struct string* new_str = get_new_str(&strs);
		
		ASSERT(strs.strs == new_str, "[get_new_str] Get string memory");
		new_str->uses += 1; // Mark string as used
		new_str = get_new_str(&strs);
		
		ASSERT(&strs.strs[1] == new_str, "[get_new_str] Get fresh string memory");
	}
	
	// String conversion
	{
		
	}
	
	printf("test_strs success\n");
	return 0;
}
