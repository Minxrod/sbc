#include "test_util.h"

#include "vars.h"
#include "system.h"
#include "program.h"
#include "tokens.h"
#include "runner.h"
#include "arrays.h"
#include "strs.h"
#include "ptc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Test code (needs to persist after exec for reading some test results)
char outcode[2048];

void run_code(char* code, struct ptc* ptc){
	struct program p = {
		strlen(code), code,
	};
	struct program o = {
		0, outcode,
	};
	
	// compile program
	tokenize(&p, &o);
	// init vars memory
	init_mem_var(&ptc->vars, 16);
	// init strs memory
	init_mem_str(&ptc->strs, 32, STRING_CHAR);
	init_mem_arr(&ptc->arrs, 16, 64);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// run code
	run(&o, ptc);
}

int test_int_vars(){
	{
		struct ptc ptc;
		char* code = "A=5\rB=8\rC=A+B\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == 5<<12, "[assign] A=5");
		ASSERT(test_var(&ptc.vars, "B", VAR_NUMBER)->value.number == 8<<12, "[assign] B=8");
		ASSERT(test_var(&ptc.vars, "C", VAR_NUMBER)->value.number == 13<<12, "[arithmetic] C=13");
		free(ptc.vars.vars);
	}
	
	// Numbers storage test I
	{
		struct ptc ptc;
		char* code = "A=1.36\rB=2.78\rC=0.001\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == 5570, "[decimal] A=1.36");
		ASSERT(test_var(&ptc.vars, "B", VAR_NUMBER)->value.number == 11386, "[decimal] B=2.78");
		ASSERT(test_var(&ptc.vars, "C", VAR_NUMBER)->value.number == 4, "[decimal] C=0.001");
		free(ptc.vars.vars);
	}
	
	// Numbers storage test II
	{
		struct ptc ptc;
		char* code = "A=8.979\rB=9.24\rC=18.186\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == 36777, "[decimal] A=8.979");
		ASSERT(test_var(&ptc.vars, "B", VAR_NUMBER)->value.number == 37847, "[decimal] B=9.24");
		ASSERT(test_var(&ptc.vars, "C", VAR_NUMBER)->value.number == 74489, "[decimal] C=18.186");
		free(ptc.vars.vars);
	}
	
	{
		struct ptc ptc;
		char* code = "A$=\"~Wow!~\"\r";
		char* str2 = "S\006~Wow!~";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		struct named_var* v = get_var(&ptc.vars, "A", 1, VAR_STRING);
		char* s = (char*)v->value.ptr;
		
		ASSERT(*s == BC_STRING, "[assign] Correct string type");
		ASSERT(str_len(s) == 6, "[assign] Correct string length");
		ASSERT(str_comp(s, str2), "[assign] Assign string");
		
		free(ptc.vars.vars);
	}
	
	{
		struct ptc ptc;
		char* code = "A$=\"ABC\"+\"DEFGH\"\rB$=\"DEFGH\"\r?A$,B$\r";
		char* str2 = "S\010ABCDEFGH";
		
		run_code(code, &ptc);
		
		struct named_var* v = get_var(&ptc.vars, "A", 1, VAR_STRING);
		struct string* s = (struct string*)v->value.ptr;
		
		ASSERT(s->type == STRING_CHAR, "[concat] Correct string type");
		ASSERT(s->len == 8, "[concat] Correct string length");
		ASSERT(str_comp(s, str2), "[concat] Correct string value");
		
		free(ptc.vars.vars);
	}
	
	// Test that exists to troubleshoot test.c not working when it worked here
	// (missing init code was the reason)
	{
		struct ptc ptc;
		char* code = "B=5\r"
		"C=8\r"
		"A=B+C\r"
		"?A,B,C\r"
		"B$=\"Abc\"+\"deF\"\r"
		"?B$,\"1234\"\r";
		char* str2 = "S\006AbcdeF";
		
		run_code(code, &ptc);
		
		struct named_var* v = get_var(&ptc.vars, "B", 1, VAR_STRING);
		struct string* s = (struct string*)v->value.ptr;
		
		ASSERT(s->type == STRING_CHAR, "[concat] Correct string type");
		ASSERT(s->len == 6, "[concat] Correct string length");
		ASSERT(str_comp(s, str2), "[concat] Correct string value");
		
		free(ptc.vars.vars);
	}
	
	// Extrmemely basic unary op test
	{
		struct ptc ptc;
		char* code = "A=-1\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == -4096, "[decimal] A=-1");
		free(ptc.vars.vars);
	}
	
	// Extrmemely basic unary op test II
	{
		struct ptc ptc;
		char* code = "A=---1+--1+-1\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == -4096, "[decimal] A=-1");
		free(ptc.vars.vars);
	}
	
	// Extrmemely basic unary op test II
	{
		struct ptc ptc;
		char* code = "A=-(-(-1))+(--1+-1*--1)\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == -4096, "[decimal] A=-1 (but more complicated)");
		free(ptc.vars.vars);
	}
	
	// More unary tests
	{
		struct ptc ptc;
		char* code = "A=---1-(----1)\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == -8192, "[decimal] A=-2");
		free(ptc.vars.vars);
	}
	
	// Order of non-associative ops test
	{
		struct ptc ptc;
		char* code = "A=2-3-5-8\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == -(14<<12), "[decimal] A=-14");
		free(ptc.vars.vars);
	}
	
	// Array declaration code test
	{
		struct ptc ptc;
		char* code = "DIM A[16]\rDIM B[4,4]\r";
		// run program
		run_code(code, &ptc);
		
		// check result for correctness
		struct named_var* a = test_var(&ptc.vars, "A", VAR_NUMBER | VAR_ARRAY);
		ASSERT(a != NULL, "[dim] A exists");
		ASSERT(a->value.ptr != NULL, "[dim] A is allocated");
		ASSERT(arr_size(a->value.ptr, ARR_DIM1) == 16, "[dim] A has correct size");
		ASSERT(arr_size(a->value.ptr, ARR_DIM2) == ARR_DIM2_UNUSED, "[dim] A has correct size II");
		
		struct named_var* b = test_var(&ptc.vars, "B", VAR_NUMBER | VAR_ARRAY);
		ASSERT(b != NULL, "[dim] B exists");
		ASSERT(b->value.ptr != NULL, "[dim] B is allocated");
		ASSERT(b->value.ptr != a->value.ptr, "[dim] B is not A");
		ASSERT(arr_size(b->value.ptr, ARR_DIM1) == 4, "[dim] B has correct size");
		ASSERT(arr_size(b->value.ptr, ARR_DIM2) == 4, "[dim] B has correct size II");
		
		free(ptc.vars.vars);
	}
	
	// Array write and read test
	{
		struct ptc ptc;
		char* code = "DIM A[3]\rA[0]=7\rA(1)=8\rA[2)=9\rA=4\r";
		// run program
		run_code(code, &ptc);
		// check DIM for correctness
		struct named_var* a = test_var(&ptc.vars, "A", VAR_NUMBER | VAR_ARRAY);
		ASSERT(a != NULL, "[dim] A exists");
		ASSERT(a->value.ptr != NULL, "[dim] A is allocated");
		ASSERT(arr_size(a->value.ptr, ARR_DIM1) == 3, "[dim] A has correct size");
		ASSERT(arr_size(a->value.ptr, ARR_DIM2) == ARR_DIM2_UNUSED, "[dim] A has correct size II");
		// check values correct
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == 4<<12, "[arrays] Var A=4, separate from arr A");
		ASSERT(get_arr_entry(&ptc.vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 0, ARR_DIM2_UNUSED)->number == 7<<12, "[arrays] A[0]=7");
		ASSERT(get_arr_entry(&ptc.vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 1, ARR_DIM2_UNUSED)->number == 8<<12, "[arrays] A[1]=8");
		ASSERT(get_arr_entry(&ptc.vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 2, ARR_DIM2_UNUSED)->number == 9<<12, "[arrays] A[2]=9");
		
		free(ptc.vars.vars);
	}
	
	// 2D Array write and read test
	{
		struct ptc ptc;
		char* code = "DIM A[2,2]\rA[0,0]=7\rA(1,0)=8\rA[0,1)=9\rA(1,1]=10\r";
		// run program
		run_code(code, &ptc);
		// check DIM for correctness
		struct named_var* a = test_var(&ptc.vars, "A", VAR_NUMBER | VAR_ARRAY);
		ASSERT(a != NULL, "[dim] A exists");
		ASSERT(a->value.ptr != NULL, "[dim] A is allocated");
		ASSERT(arr_size(a->value.ptr, ARR_DIM1) == 2, "[dim] A has correct size");
		ASSERT(arr_size(a->value.ptr, ARR_DIM2) == 2, "[dim] A has correct size II");
		// check values correct
		ASSERT(get_arr_entry(&ptc.vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 0, 0)->number == 7<<12, "[arrays] A[0,0]=7");
		ASSERT(get_arr_entry(&ptc.vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 1, 0)->number == 8<<12, "[arrays] A[1,0]=8");
		ASSERT(get_arr_entry(&ptc.vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 0, 1)->number == 9<<12, "[arrays] A[0,1]=9");
		ASSERT(get_arr_entry(&ptc.vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 1, 1)->number == 10<<12, "[arrays] A[1,1]=10");
		
		free(ptc.vars.vars);
	}
	
	// String array manipulation
	{
		struct ptc ptc;
		char* code = "DIM A$[3]\rA$[0]=\"A\"\rA$[1]=\"B\"\rA$[2]=A$[1]+A$[0]\r";
		char* strA = "S\001A";
		char* strB = "S\001B";
		char* strBA = "S\002BA";
		
		// run code
		run_code(code, &ptc);
		// check results
		struct named_var* a = test_var(&ptc.vars, "A", VAR_STRING | VAR_ARRAY);
		ASSERT(a != NULL, "[dim] A exists");
		ASSERT(a->value.ptr != NULL, "[dim] A is allocated");
		ASSERT(arr_size(a->value.ptr, ARR_DIM1) == 3, "[dim] A has correct size");
		ASSERT(arr_size(a->value.ptr, ARR_DIM2) == ARR_DIM2_UNUSED, "[dim] A has correct size II");
		
		struct string* s1 = (struct string*)get_arr_entry(&ptc.vars, "A", 1, VAR_STRING | VAR_ARRAY, 0, ARR_DIM2_UNUSED)->ptr;
		struct string* s2 = (struct string*)get_arr_entry(&ptc.vars, "A", 1, VAR_STRING | VAR_ARRAY, 1, ARR_DIM2_UNUSED)->ptr;
		struct string* s3 = (struct string*)get_arr_entry(&ptc.vars, "A", 1, VAR_STRING | VAR_ARRAY, 2, ARR_DIM2_UNUSED)->ptr;
		
		ASSERT(s1->type == BC_STRING, "[array] Correct string type 1");
		ASSERT(str_len(s1) == 1, "[array] Correct string length");
		ASSERT(str_comp(s1, strA), "[array] Assign string");
		
		ASSERT(s2->type == BC_STRING, "[array] Correct string type 2");
		ASSERT(str_len(s2) == 1, "[array] Correct string length");
		ASSERT(str_comp(s2, strB), "[array] Assign string");
		
		ASSERT(s3->type == STRING_CHAR, "[array] Correct string type 3");
		ASSERT(s3->len == 2, "[array] Correct string length");
		ASSERT(str_comp(s3, strBA), "[array] Assign string");
		
		// check DIM for correctness
		// check values correct
	}
	
	SUCCESS("test_int_vars success");
}
