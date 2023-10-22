#include "test_vars.h"

#include "test_util.h"

#include "interpreter/vars.h"

#include <stdio.h>

int test_vars(){
	
	/*
	namecmp tests
	*/
	// Obvious true test
	{
		char* name1 = "ABC";
		int name1_len = 3;
		char name2[16] = {'A','B','C',0};
		
		ASSERT(namecmp(name1, name1_len, name2), "[namecmp] Simple name match");
	}
	
	// Obvious false test
	{
		char* name1 = "XYZ";
		int name1_len = 3;
		char name2[16] = {'A','B','C',0};
		
		DENY(namecmp(name1, name1_len, name2), "[namecmp] Simple name no match");
	}
	
	// Name1 longer
	{
		char* name1 = "ABCD";
		int name1_len = 4;
		char name2[16] = {'A','B','C',0};
		
		DENY(namecmp(name1, name1_len, name2), "[namecmp] Longer name1 same start no match");
	}
	
	// Name1 longer
	{
		char* name1 = "AB";
		int name1_len = 4;
		char name2[16] = {'A','B','C',0};
		
		DENY(namecmp(name1, name1_len, name2), "[namecmp] Shorter name1 same start no match");
	}
	
	// Max length same
	{
		char* name1 = "ABCDEFGHIJKLMNO";
		int name1_len = 16;
		char name2[16] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O'};
		
		ASSERT(namecmp(name1, name1_len, name2), "[namecmp] Same name length 16");
	}
	
	// Max length difference
	{
		char* name1 = "ABCDEFGHIJKLMNO";
		int name1_len = 16;
		char name2[16] = {'X','Y','Z','D','E','F','G','H','I','J','K','L','M','N','O'};
		
		DENY(namecmp(name1, name1_len, name2), "[namecmp] Different name length 16");
	}
	
	// Min length same
	{
		char* name1 = "A";
		int name1_len = 1;
		char name2[16] = {'A',0};
		
		ASSERT(namecmp(name1, name1_len, name2), "[namecmp] Same name length 1");
	}
	
	/*
	get_var tests
	*/
	
	// Get a new variable
	{
		struct variables v;
		init_mem_var(&v, 8);
		
		char* name1 = "A";
		
		struct named_var* var = get_var(&v, name1, 1, VAR_NUMBER);
		ASSERT(var->type == VAR_NUMBER, "[get_var] Create var with type number");
		ASSERT(var->value.number == 0, "[get_var] Create var with value 0");
		
		free_mem_var(&v);
	}
	
	// Get and write to a new variable, then read it again
	{
		struct variables v;
		init_mem_var(&v, 8);
		
		char* name1 = "A";
		
		struct named_var* var = get_var(&v, name1, 1, VAR_NUMBER);
		var->value.number = INT_TO_FP(123);
		
		var = get_var(&v, name1, 1, VAR_NUMBER);
		
		ASSERT(var->type == VAR_NUMBER, "[get_var] Get var with type number");
		ASSERT(var->value.number == INT_TO_FP(123), "[get_var] Check value is 123");
		
		free_mem_var(&v);
	}
	
	// Read and write multiple variables
	{
		struct variables v;
		init_mem_var(&v, 8);
		
		char* name1 = "A";
		char* name2 = "AB";
		
		struct named_var* var;
		var = get_var(&v, name1, 1, VAR_NUMBER);
		var->value.number = INT_TO_FP(123);
		
		var = get_var(&v, name2, 2, VAR_NUMBER);
		var->value.number = INT_TO_FP(256);
		
		var = get_var(&v, name1, 1, VAR_NUMBER);
		ASSERT(var->type == VAR_NUMBER, "[get_var] Read var with type number");
		ASSERT(var->value.number == INT_TO_FP(123), "[get_var] Check value is 123");
		
		var = get_var(&v, name2, 2, VAR_NUMBER);
		ASSERT(var->type == VAR_NUMBER, "[get_var] Read var with type number");
		ASSERT(var->value.number == INT_TO_FP(256), "[get_var] Check value is 256");
		
		free_mem_var(&v);
	}
	
	// Check that names can be shared between differing types (as different vars)
	{
		struct variables v;
		struct strings s;
		v.strs = &s;
		
		init_mem_var(&v, 8);
		init_mem_str(&s, 16, STRING_CHAR);
		
		char* name1 = "A";
		char* name2 = "A$";
		
		struct named_var *var1, *var2;
		
		var1 = get_var(&v, name1, 1, VAR_NUMBER);
		var2 = get_var(&v, name2, 1, VAR_STRING);
		
		ASSERT(var1 != var2, "[get_var] Same name different types are different vars");
		
		free_mem_str(&s);
		free_mem_var(&v);
	}
	
	// Check that default initialization of array works as expected
	{
		struct variables v;
		struct arrays a;
		v.arrs = &a;
		
		init_mem_var(&v, 8);
		init_mem_arr(&a, 8, 32);
		
		char* name1 = "A";
		
		struct named_var *var1;
		union value* val1;
		val1 = get_arr_entry(&v, name1, 1, VAR_NUMBER | VAR_ARRAY, 3, ARR_DIM2_UNUSED);
		// must be after or else array has no memory pointer
		var1 = get_var(&v, name1, 1, VAR_NUMBER | VAR_ARRAY);
		
		ASSERT(var1 != NULL, "[get_var] Array variable exists from get_var");
		ASSERT(var1->type == (VAR_NUMBER | VAR_ARRAY), "[get_var] Variable has array type");
		ASSERT(var1->value.ptr != NULL, "[get_var] Variable has pointer to array data");
		
		ASSERT(arr_size(var1->value.ptr, ARR_DIM1) == 10, "[get_var] Default array size is correct");
		ASSERT(arr_size(var1->value.ptr, ARR_DIM2) == ARR_DIM2_UNUSED, "[get_var] Default array size is correct");
		ASSERT(val1->number == 0, "[get_arr_entry] Initalized value is zero");
		
		free_mem_arr(&a);
		free_mem_var(&v);
	}
	
	// Create an array via get_new_arr_var; 2-dimensional array
	{
		struct variables v;
		struct arrays a;
		v.arrs = &a;
		
		init_mem_var(&v, 8);
		init_mem_arr(&a, 8, 32);
		
		char* name1 = "ARR";
		
		struct named_var *var1;
		var1 = get_new_arr_var(&v, name1, 3, VAR_NUMBER | VAR_ARRAY, 6, 4);
		// must be after or else array has no memory pointer	
		ASSERT(var1 != NULL, "[get_new_arr_var] Array variable exists");
		ASSERT(var1->type == (VAR_NUMBER | VAR_ARRAY), "[get_new_arr_var] Variable has array type");
		ASSERT(var1->value.ptr != NULL, "[get_new_arr_var] Variable has pointer to array data");
		
		ASSERT(arr_size(var1->value.ptr, ARR_DIM1) == 6, "[get_new_arr_var] Default array size is correct");
		ASSERT(arr_size(var1->value.ptr, ARR_DIM2) == 4, "[get_new_arr_var] Default array size is correct");
		
		free_mem_arr(&a);
		free_mem_var(&v);
	}
	
	SUCCESS("test_vars success");
}
