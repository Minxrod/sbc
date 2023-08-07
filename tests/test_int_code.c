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
	
	// Actual FOR loop
	{
		struct ptc ptc = {0};
		char* code = "FOR I=0 TO 9\r\rNEXT\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "I", VAR_NUMBER)->value.number == 10<<12, "[for] I=10");
		
		free_code(&ptc);
	}
	
	// FOR loop with content
	{
		struct ptc ptc = {0};
		char* code = "DIM A[20]\rFOR I=0 TO 19\rA[I]=I\rNEXT\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "I", VAR_NUMBER)->value.number == 20<<12, "[for] I=20");
		for (int i = 0; i < 20; ++i){
			ASSERT(get_arr_entry(&ptc.vars, "A", 1, VAR_NUMBER | VAR_ARRAY, i, ARR_DIM2_UNUSED)->number == (i << 12), "[for] A[i]=i");
		}
		
		free_code(&ptc);
	}
	
	// IF construct
	{
		struct ptc ptc = {0};
		char* code = "IF TRUE THEN A=0:B=1 ELSE B=0:A=1\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == 0<<12, "[if] A=0");
		ASSERT(test_var(&ptc.vars, "B", VAR_NUMBER)->value.number == 1<<12, "[if] B=1");
		
		free_code(&ptc);
	}
	
	// IF construct
	{
		struct ptc ptc = {0};
		char* code = "IF FALSE THEN A=0:B=1 ELSE B=0:A=-1\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "A", VAR_NUMBER)->value.number == -(1<<12), "[if] A=1");
		ASSERT(test_var(&ptc.vars, "B", VAR_NUMBER)->value.number == 0<<12, "[if] B=0");
		
		free_code(&ptc);
	}
	
	// GOTO test
	{
		struct ptc ptc = {0};
		char* code = "@TEST\rI=I+1\rIF I<5 GOTO @TEST\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "I", VAR_NUMBER)->value.number == 5<<12, "[goto] I=5");
	}
	
	// ON GOTO test
	{
		struct ptc ptc = {0};
		char* code = "ON 2 GOTO @1,@2,@3,@4,@5\r@1\rI=I+1\r@2\rI=I+1\r@3\rI=I+1\r@4\rI=I+1\r@5\rI=I+1\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "I", VAR_NUMBER)->value.number == 3<<12, "[goto] I=3");
	}
	
	// END, GOSUB test
	{
		struct ptc ptc = {0};
		char* code = "FOR I=0 TO 3\rGOSUB @TEST\rNEXT\rEND\r@TEST\rJ=J+I+1\rRETURN\r";
		// run program
		run_code(code, &ptc);
		// check output for correctness
		ASSERT(test_var(&ptc.vars, "J", VAR_NUMBER)->value.number == 10<<12, "[goto] J=10");
	}
	
	SUCCESS("test_int_code success");
}
