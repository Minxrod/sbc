#include "test_util.h"

#include "interpreter.h"

#include "system.h"
#include "program.h"
#include "ptc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Test code (needs to persist after exec for reading some test results)

int test_int_vars(){
	{
		char* code = "A=5\rB=8\rC=A+B\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(5), "[assign] A=5");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(8), "[assign] B=8");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(13), "[arithmetic] C=13");
		
		free_code(p);
	}
	
	// Numbers storage test I
	{
		char* code = "A=1.36\rB=2.78\rC=0.001\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == 5570, "[decimal] A=1.36");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == 11386, "[decimal] B=2.78");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == 4, "[decimal] C=0.001");
		
		free_code(p);
	}
	
	// Numbers storage test II
	{
		char* code = "A=8.979\rB=9.24\rC=18.186\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == 36777, "[decimal] A=8.979");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == 37847, "[decimal] B=9.24");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == 74489, "[decimal] C=18.186");

		free_code(p);
	}
	
	{
		char* code = "A$=\"~Wow!~\"\r";
		char* str2 = "S\006~Wow!~";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		struct named_var* v = get_var(&p->vars, "A", 1, VAR_STRING);
		char* s = (char*)v->value.ptr;
		
		ASSERT(*s == BC_STRING, "[assign] Correct string type");
		ASSERT(str_len(s) == 6, "[assign] Correct string length");
		ASSERT(str_comp(s, str2), "[assign] Assign string");
		
		free_code(p);

	}
	
	{
		char* code = "A$=\"ABC\"+\"DEFGH\"\rB$=\"DEFGH\"\r?A$,B$\r";
		char* str2 = "S\010ABCDEFGH";
		
		struct ptc* p = run_code(code);
		
		struct named_var* v = get_var(&p->vars, "A", 1, VAR_STRING);
		struct string* s = (struct string*)v->value.ptr;
		
		ASSERT(s->type == STRING_CHAR, "[concat] Correct string type");
		ASSERT(s->len == 8, "[concat] Correct string length");
		ASSERT(s->uses == 1, "[concat] Correct string usage count")
		ASSERT(str_comp(s, str2), "[concat] Correct string value");
		
		free_code(p);
	}
	
	// Test that exists to troubleshoot test.c not working when it worked here
	// (missing init code was the reason)
	{
		char* code = "B=5\r"
		"C=8\r"
		"A=B+C\r"
		"?A,B,C\r"
		"B$=\"Abc\"+\"deF\"\r"
		"?B$,\"1234\"\r";
		char* str2 = "S\006AbcdeF";
		
		struct ptc* p = run_code(code);
		
		struct named_var* v = get_var(&p->vars, "B", 1, VAR_STRING);
		struct string* s = (struct string*)v->value.ptr;
		
		ASSERT(s->type == STRING_CHAR, "[concat] Correct string type");
		ASSERT(s->len == 6, "[concat] Correct string length");
		ASSERT(s->uses == 1, "[concat] Connect string usages");
		ASSERT(str_comp(s, str2), "[concat] Correct string value");
		
		free_code(p);
	}
	
	// Extrmemely basic unary op test
	{
		char* code = "A=-1\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == -4096, "[decimal] A=-1");
		
		free_code(p);
	}
	
	// Extrmemely basic unary op test II
	{
		char* code = "A=---1+--1+-1\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == -4096, "[decimal] A=-1");
		
		free_code(p);
	}
	
	// Extrmemely basic unary op test II
	{
		char* code = "A=-(-(-1))+(--1+-1*--1)\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == -4096, "[decimal] A=-1 (but more complicated)");
		
		free_code(p);
	}
	
	// More unary tests
	{
		char* code = "A=---1-(----1)\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == -8192, "[decimal] A=-2");
		
		free_code(p);
	}
	
	// Order of non-associative ops test
	{
		char* code = "A=2-3-5-8\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == -INT_TO_FP(14), "[decimal] A=-14");
		
		free_code(p);
	}
	
	// Array declaration code test
	{
		char* code = "DIM A[16]\rDIM B[4,4]\r";
		// run program
		struct ptc* p = run_code(code);
		
		// check result for correctness
		struct named_var* a = test_var(&p->vars, "A", VAR_NUMBER | VAR_ARRAY);
		ASSERT(a != NULL, "[dim] A exists");
		ASSERT(a->value.ptr != NULL, "[dim] A is allocated");
		ASSERT(arr_size(a->value.ptr, ARR_DIM1) == 16, "[dim] A has correct size");
		ASSERT(arr_size(a->value.ptr, ARR_DIM2) == ARR_DIM2_UNUSED, "[dim] A has correct size II");
		
		struct named_var* b = test_var(&p->vars, "B", VAR_NUMBER | VAR_ARRAY);
		ASSERT(b != NULL, "[dim] B exists");
		ASSERT(b->value.ptr != NULL, "[dim] B is allocated");
		ASSERT(b->value.ptr != a->value.ptr, "[dim] B is not A");
		ASSERT(arr_size(b->value.ptr, ARR_DIM1) == 4, "[dim] B has correct size");
		ASSERT(arr_size(b->value.ptr, ARR_DIM2) == 4, "[dim] B has correct size II");
		
		free_code(p);
	}
	
	// Array write and read test
	{
		char* code = "DIM A[3]\rA[0]=7\rA(1)=8\rA[2)=9\rA=4\r";
		// run program
		struct ptc* p = run_code(code);
		// check DIM for correctness
		struct named_var* a = test_var(&p->vars, "A", VAR_NUMBER | VAR_ARRAY);
		ASSERT(a != NULL, "[dim] A exists");
		ASSERT(a->value.ptr != NULL, "[dim] A is allocated");
		ASSERT(arr_size(a->value.ptr, ARR_DIM1) == 3, "[dim] A has correct size");
		ASSERT(arr_size(a->value.ptr, ARR_DIM2) == ARR_DIM2_UNUSED, "[dim] A has correct size II");
		// check values correct
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(4), "[arrays] Var A=4, separate from arr A");
		ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 0, ARR_DIM2_UNUSED)->number == INT_TO_FP(7), "[arrays] A[0]=7");
		ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 1, ARR_DIM2_UNUSED)->number == INT_TO_FP(8), "[arrays] A[1]=8");
		ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 2, ARR_DIM2_UNUSED)->number == INT_TO_FP(9), "[arrays] A[2]=9");
		
		free_code(p);
	}
	
	// 2D Array write and read test
	{
		char* code = "DIM A[2,2]\rA[0,0]=7\rA(1,0)=8\rA[0,1)=9\rA(1,1]=10\r";
		// run program
		struct ptc* p = run_code(code);
		// check DIM for correctness
		struct named_var* a = test_var(&p->vars, "A", VAR_NUMBER | VAR_ARRAY);
		ASSERT(a != NULL, "[dim] A exists");
		ASSERT(a->value.ptr != NULL, "[dim] A is allocated");
		ASSERT(arr_size(a->value.ptr, ARR_DIM1) == 2, "[dim] A has correct size");
		ASSERT(arr_size(a->value.ptr, ARR_DIM2) == 2, "[dim] A has correct size II");
		// check values correct
		ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 0, 0)->number == INT_TO_FP(7), "[arrays] A[0,0]=7");
		ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 1, 0)->number == INT_TO_FP(8), "[arrays] A[1,0]=8");
		ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 0, 1)->number == INT_TO_FP(9), "[arrays] A[0,1]=9");
		ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, 1, 1)->number == INT_TO_FP(10), "[arrays] A[1,1]=10");
		
		free_code(p);
	}
	
	// String array manipulation
	{
		char* code = "DIM A$[3]\rA$[0]=\"A\"\rA$[1]=\"B\"\rA$[2]=A$[1]+A$[0]\r";
		char* strA = "S\001A";
		char* strB = "S\001B";
		char* strBA = "S\002BA";
		
		// run code
		struct ptc* p = run_code(code);
		// check results
		struct named_var* a = test_var(&p->vars, "A", VAR_STRING | VAR_ARRAY);
		ASSERT(a != NULL, "[dim] A exists");
		ASSERT(a->value.ptr != NULL, "[dim] A is allocated");
		ASSERT(arr_size(a->value.ptr, ARR_DIM1) == 3, "[dim] A has correct size");
		ASSERT(arr_size(a->value.ptr, ARR_DIM2) == ARR_DIM2_UNUSED, "[dim] A has correct size II");
		
		struct string* s1 = (struct string*)get_arr_entry(&p->vars, "A", 1, VAR_STRING | VAR_ARRAY, 0, ARR_DIM2_UNUSED)->ptr;
		struct string* s2 = (struct string*)get_arr_entry(&p->vars, "A", 1, VAR_STRING | VAR_ARRAY, 1, ARR_DIM2_UNUSED)->ptr;
		struct string* s3 = (struct string*)get_arr_entry(&p->vars, "A", 1, VAR_STRING | VAR_ARRAY, 2, ARR_DIM2_UNUSED)->ptr;
		
		ASSERT(s1->type == BC_STRING, "[array] Correct string type 1");
		ASSERT(str_len(s1) == 1, "[array] Correct string length");
		ASSERT(str_comp(s1, strA), "[array] Assign string");
		
		ASSERT(s2->type == BC_STRING, "[array] Correct string type 2");
		ASSERT(str_len(s2) == 1, "[array] Correct string length");
		ASSERT(str_comp(s2, strB), "[array] Assign string");
		
		ASSERT(s3->type == STRING_CHAR, "[array] Correct string type 3");
		ASSERT(s3->len == 2, "[array] Correct string length");
		ASSERT(s3->uses == 1, "[array] Correct string uses");
		ASSERT(str_comp(s3, strBA), "[array] Assign string");
		
		free_code(p);
	}
	
	// String reference counting
	{
		// note: to force string alloc, add an empty string to end
		char* code = "A$=\"AB\"+\"C\"\rB$=A$\rC$=B$\rD$=C$+\"D\"\rC$=\"A\"+\"\"\rE$=MID$(A$,0,1)\r";
		char* strABC = "S\003ABC";
		char* strABCD = "S\004ABCD";
		char* strA = "S\001A";
		
		// run code
		struct ptc* p = run_code(code);
		// check results
		struct named_var* a = test_var(&p->vars, "A", VAR_STRING);
		struct named_var* d = test_var(&p->vars, "D", VAR_STRING);
		struct named_var* c = test_var(&p->vars, "C", VAR_STRING);
		struct named_var* e = test_var(&p->vars, "E", VAR_STRING);
		
		ASSERT(a != NULL, "[str_uses] A exists");
		ASSERT(str_comp(a->value.ptr, strABC), "[str_uses] Correct string A");
		ASSERT(((struct string*)a->value.ptr)->uses == 2, "[str_uses] Correct usage count A");
		
		ASSERT(d != NULL, "[str_uses] D exists");
		ASSERT(str_comp(d->value.ptr, strABCD), "[str_uses] Correct string D");
		ASSERT(((struct string*)d->value.ptr)->uses == 1, "[str_uses] Correct usage count D");
		
		ASSERT(c != NULL, "[str_uses] C exists");
		ASSERT(((struct string*)c->value.ptr)->type == STRING_CHAR, "[str_uses] Correct string type C");
		ASSERT(str_comp(c->value.ptr, strA), "[str_uses] Correct string C");
		ASSERT(((struct string*)c->value.ptr)->uses == 1, "[str_uses] Correct usage count C");
		
		ASSERT(e != NULL, "[str_uses] E exists");
		ASSERT(str_comp(e->value.ptr, strA), "[str_uses] Correct string E");
		ASSERT(((struct string*)e->value.ptr)->uses == 1, "[str_uses] Correct usage count E");
		
		free_code(p);
	}
	
	{
		char* code = "A=5\rDIM B[7]\rB[3]=6\rC$=\"ABCDEF\"\rCLEAR\r";
		struct ptc* p = run_code(code);
		
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(0), "[clear] Clears variable A correctly");
		ASSERT(get_arr_entry(&p->vars, "B", 1, VAR_NUMBER | VAR_ARRAY, 3, ARR_DIM2_UNUSED)->number == INT_TO_FP(0), "[clear] Clears variable B[] correctly");
		ASSERT(test_var(&p->vars, "C", VAR_STRING)->value.ptr == empty_str, "[clear] Clears string C$ correctly");
		
		free_code(p);
	}
	
	// Chunk of SAMPLE5 that broke
	{
		char* code = "SPC = 8\rDAYW = 8:DRAWD = 30\r"
		"BGL = (256 - (DRAWD - 1) * DAYW - SPC * 2) / 2\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(test_var(&p->vars, "SPC", VAR_NUMBER)->value.number == INT_TO_FP(8), "[vars] SPC=8");
		ASSERT(test_var(&p->vars, "DAYW", VAR_NUMBER)->value.number == INT_TO_FP(8), "[vars] DAYW=8");
		ASSERT(test_var(&p->vars, "DRAWD", VAR_NUMBER)->value.number == INT_TO_FP(30), "[vars] DRAWD=30");
		ASSERT(test_var(&p->vars, "BGL", VAR_NUMBER)->value.number == INT_TO_FP(4), "[vars] BGL=4");
		
		free_code(p);
	}
	
	// Check for duplicate definition error
	{
		ASSERT(check_code_error("DIM A[6]\rDIM A[12]\r", ERR_DUPLICATE_DIM), "[dim] Duplicate definition error");
	}
	
	// Out of memory (variable creation)
	{
		{
			struct ptc* p = run_code_lowmem("A=0\rB=1\rC=2\rD=3\r");
			ASSERT(p->exec.error == ERR_NONE, "[vars] Variables created successfully");
			free_code(p);
		}
		{
			struct ptc* p = run_code_lowmem("A=0\rB=1\rC=2\rD=3\rE=4\r");
			ASSERT(p->exec.error == ERR_OUT_OF_MEMORY, "[vars] Variable creation error when out of memory");
			free_code(p);
		}
	}
	
	SUCCESS("test_int_vars success");
}
