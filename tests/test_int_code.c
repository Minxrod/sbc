#include "test_int_code.h"
#include "test_util.h"

#include "interpreter.h"

#include "ptc.h"
#include "error.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int test_int_code(){
	// Code searching
	{
		char* code = "FOR I=0 TO 9\r\rNEXT\r";
		char buf[64];
		struct program p = {
			strlen(code),
			code,
		};
		struct program o = {0, buf};
		
		tokenize(&p, &o);
		
		u32 i = bc_scan(&o, 0, BC_OPERATOR);
		ASSERT(i == 6, "[bc_scan] Find index of operator");
	}
	
	// Code searching (not faked)
	{
		char* code = "?\"O=\"+\"O=\"\r";
		char buf[64];
		struct program p = {
			strlen(code),
			code,
		};
		struct program o = {0, buf};
		
		tokenize(&p, &o);
		
		u32 i = bc_scan(&o, 0, BC_OPERATOR);
		ASSERT(i == 8, "[bc_scan] Find index of operator without being in string");
	}
	
	// Code searching (Variable name)
	{
		char* code = "V\3ABC\0L\1A\0";
		struct program o = {8, code};
		u32 i = bc_scan(&o, 0, BC_LABEL);
		ASSERT(i == 6, "[bc_scan] Find index of label past variable name");
	}
	
	// Actual FOR loop
	{
		char* code = "FOR I=0 TO 9\r\rNEXT\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(10), "[for] I=10");
		
		free_code(p);
	}
	
	// FOR loop with content
	{
		char* code = "DIM A[20]\rFOR I=0 TO 19\rA[I]=I\rNEXT\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(20), "[for] I=20");
		for (int i = 0; i < 20; ++i){
			ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, i, ARR_DIM2_UNUSED)->number == (INT_TO_FP(i)), "[for] A[i]=i");
		}
		
		free_code(p);
	}
	
	// IF construct
	{
		char* code = "IF TRUE THEN A=0:B=1 ELSE B=0:A=1\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(0), "[if] A=0");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(1), "[if] B=1");
		
		free_code(p);
	}
	
	// IF construct
	{
		char* code = "IF FALSE THEN A=0:B=1 ELSE B=0:A=-1\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == -(INT_TO_FP(1)), "[if] A=1");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(0), "[if] B=0");
		
		free_code(p);
	}
	
	// GOTO test
	{
		char* code = "@TEST\rI=I+1\rIF I<5 GOTO @TEST\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(5), "[goto] I=5");
		free_code(p);
	}
	
	// ON GOTO test
	{
		char* code = "ON 2 GOTO @1,@2,@3,@4,@5\r@1\rI=I+1\r@2\rI=I+1\r@3\rI=I+1\r@4\rI=I+1\r@5\rI=I+1\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(3), "[goto] I=3");
		free_code(p);
	}
	
	// END, GOSUB test
	{
		char* code = "FOR I=0 TO 3\rGOSUB @TEST\rNEXT\rEND\r@TEST\rJ=J+I+1\rRETURN\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "J", VAR_NUMBER)->value.number == INT_TO_FP(10), "[goto] J=10");
		free_code(p);
	}
	
	// DATA, READ test
	// READ one number
	{
		char* code = "DATA 523\rREAD D\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(523), "[data] D=523");
		free_code(p);
	}
	
	// READ one number
	{
		char* code = "DATA 532,7\rREAD D,B\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(532), "[data] D=532");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(7), "[data] B=7");
		free_code(p);
	}
	
	// READ one number
	{
		char* code = "DATA 532,7\rREAD D$\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(str_comp(test_var(&p->vars, "D", VAR_STRING)->value.ptr, "S\003532"), "[data] D$=\"532\"");
		free_code(p);
	}
	
	// READ one number
	{
		char* code = "READ D\rDATA 532,7\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(532), "[data] D=532 (DATA past start)");
		free_code(p);
	}
	
	{
		//TODO:TEST Create READ string containing comma
	}
	
	// INKEY$ test
	{
		char* code = "A$=INKEY$()\r";
		struct ptc* p = run_code_keys(code, "B", 1);
		// Check inkey string generation method works
		ASSERT(str_comp(test_var(&p->vars, "A", VAR_STRING)->value.ptr, "S\1B"), "[input] A$=\"B\"");
		free_code(p);
	}
	
	// LEN tests
	{
		char* code = "A=LEN(\"\")\rB=LEN(\"ABC\")\rC=LEN(\"Aaa!\"*8)\rD=LEN(\"@\"*256)\r";
		struct ptc* p = run_code(code);
		// Check various lengths
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(0), "[len] Length is 0");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(3), "[len] Length is 3");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(32), "[len] Length is 32");
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(256), "[len] Length is 256");
		free_code(p);
	}
	
	// MID$ tests
	{
		char* code = "A$=\"Test String! :)\"\rB$=MID$(A$,0,4)\rC$=MID$(A$,0,32)\rD$=MID$(A$,20,3)\rE$=MID$(A$,5,0)\r";
		char* a_str= "S\17Test String! :)";
		char* b_str= "S\4Test";
		char* c_str= "S\17Test String! :)";
		char* d_str= "S\0";
		char* e_str= "S\0";
		
		struct ptc* p = run_code(code);
		// Check various substrings
		ASSERT(str_comp(test_var(&p->vars, "A", VAR_STRING)->value.ptr, a_str), "[mid$] Base string A$");
		ASSERT(str_comp(test_var(&p->vars, "B", VAR_STRING)->value.ptr, b_str), "[mid$] B$");
		ASSERT(str_comp(test_var(&p->vars, "C", VAR_STRING)->value.ptr, c_str), "[mid$] C$");
		ASSERT(str_comp(test_var(&p->vars, "D", VAR_STRING)->value.ptr, d_str), "[mid$] D$");
		ASSERT(str_comp(test_var(&p->vars, "E", VAR_STRING)->value.ptr, e_str), "[mid$] E$");
		
		free_code(p);
	}
	
	// VAL tests...
	{
		char* code = "A=VAL(\"\")\rB=VAL(\"1 0\")\rC=VAL(\"00000000000123456\")\r"
		"D=VAL(\"0.0005\")\rE=VAL(\"AF58 B\")\rF=VAL(\"DBHUG77\")\r"
		"G=VAL(\"4565FGHG\")\rH=VAL(\" \")\rI=VAL(\"..\")\rJ=VAL(\"01.99 88.\")\r"
		"K=VAL(\"&H4FE4HR6 77 6\")\rL=VAL(\"111111 1\")\rM=VAL(\"524287.999994\")\r"
		"N=VAL(\"-0.0005\")\rO=VAL(\"0.6666\")\rP=VAL(\"MN230\")\r";
		
		struct ptc* p = run_code(code);
		// Check various substrings
		CHECK_VAR_INT("A",0);
		CHECK_VAR_INT("B",1);
		CHECK_VAR_INT("C",123456);
		CHECK_VAR_NUM("D",2);
		CHECK_VAR_INT("E",0);
		CHECK_VAR_INT("F",0);
		CHECK_VAR_INT("G",4565);
		CHECK_VAR_INT("H",0);
		CHECK_VAR_INT("I",0);
		CHECK_VAR_NUM("J",8151);
		CHECK_VAR_INT("K",20452);
		CHECK_VAR_INT("L",111111);
		CHECK_VAR_NUM("M",0x7fffffff);
		CHECK_VAR_NUM("N",-2);
		CHECK_VAR_NUM("O",2730);
		CHECK_VAR_INT("P",0);
		
		free_code(p);
	}
	
	// DTREAD tests
	{
		char* code = "DTREAD(\"2023/09/19\"),Y,M,D\r";
		
		struct ptc* p = run_code(code);
		// Check various substrings
		CHECK_VAR_INT("Y",2023);
		CHECK_VAR_INT("M",9);
		CHECK_VAR_INT("D",19);
		
		free_code(p);
	}
	
	// FOR stack stuff test
	{
		char* code = "FOR I=0 TO 1\rFOR J=0 TO 1\r?I,J\rIF J==0 THEN NEXT J\rNEXT I\rNEXT J\r";
		
		struct ptc* p = run_code(code);
		
		CHECK_VAR_INT("I",2); // after FOR ends, this is the result
		CHECK_VAR_INT("J",2);
		ASSERT(p->exec.error == ERR_NONE, "[for] Check NEXT VAR behavior");
		
		free_code(p);
	}
	
	SUCCESS("test_int_code success");
}
