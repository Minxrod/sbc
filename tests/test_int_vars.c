#include "test_util.h"

#include "vars.h"
#include "system.h"
#include "program.h"
#include "tokens.h"
#include "runner.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void run_code(char* code, struct ptc* ptc){
	struct program p = {
		strlen(code), code,
	};
	struct program o = {
		0, calloc(strlen(code), 2),
	};
	
	// compile program
	tokenize(&p, &o);
	// init vars memory
	init_mem_var(&ptc->vars, 16);
	// run code
	run(&o, ptc);
	
	free(o.data);
//	free(ptc->vars.vars);
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
	
	
	iprintf("test_int_vars successful\n");
	return 0;
}
