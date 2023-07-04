#include "test_util.h"

#include "vars.h"
#include "system.h"

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
	}
	
	// Get and write to a new variable, then read it again
	{
		struct variables v;
		init_mem_var(&v, 8);
		
		char* name1 = "A";
		
		struct named_var* var = get_var(&v, name1, 1, VAR_NUMBER);
		var->value.number = 123<<12;
		
		var = get_var(&v, name1, 1, VAR_NUMBER);
		
		ASSERT(var->type == VAR_NUMBER, "[get_var] Get var with type number");
		ASSERT(var->value.number == 123<<12, "[get_var] Check value is 123");
	}
	
	// Read and write multiple variables
	{
		struct variables v;
		init_mem_var(&v, 8);
		
		char* name1 = "A";
		char* name2 = "AB";
		
		struct named_var* var;
		var = get_var(&v, name1, 1, VAR_NUMBER);
		var->value.number = 123<<12;
		
		var = get_var(&v, name2, 2, VAR_NUMBER);
		var->value.number = 256<<12;
		
		var = get_var(&v, name1, 1, VAR_NUMBER);
		ASSERT(var->type == VAR_NUMBER, "[get_var] Read var with type number");
		ASSERT(var->value.number == 123<<12, "[get_var] Check value is 123");
		
		var = get_var(&v, name2, 2, VAR_NUMBER);
		ASSERT(var->type == VAR_NUMBER, "[get_var] Read var with type number");
		ASSERT(var->value.number == 256<<12, "[get_var] Check value is 256");
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
		var2 = get_var(&v, name2, 2, VAR_STRING);
		
		ASSERT(var1 != var2, "[get_var] Same name different types are different vars");
	}
	
	printf("test_vars success\n");
	return 0;
}
