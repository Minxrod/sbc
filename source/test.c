#ifndef ARM9
//this is the computer-only testing file

#include "tokens.h"
#include "runner.h"
#include "system.h"

#include <stdio.h>

char* code = "B=5\r"
"C=8\r"
"A=B+C\r"
"?A,B,C\r";

/*
"?\"Hello world!\",23\r"
"?12345.67\r"
"COLOR 0\r"
"LOCATE 3,3\r"
"PRINT 3+5*8,3\r"
"?\"HI!\"+\"56\";5*8+3/SIN(6)\r"
"?ATAN(6,3)\r"
"?5,PI(),5\r"
"?PI(1,2,4,8)\r"
"?AA,B,C\r"
"A=B+C\r"
"PRINT a,b\r"
"A(8,B)=C+56*7+8\r"
;*/

char out[4096];

int main(void){
	// initialize system buffers, etc.
	system_init();
	
	struct program program;
	program.data = code;
	for (program.size = 0; code[program.size] != '\0'; ++program.size);
//	struct program_code code
	
	struct program output;
	output.data = out;
	output.size = 0;
	
	tokenize(&program, &output);
	
	struct ptc ptc;
	
	init_mem_var(&ptc.vars, 2048);
	
	run(&output, &ptc);
	
//	struct named_var* v = get_var(&ptc.vars, "A", 1, VAR_NUMBER);
//	iprintf("%d", v->value.number);
	
	// measure size of program file size vs original's memory vs remake's memory use
	iprintf("file:%d ptc:%d sbc:%d\n", program.size, program.size*2, output.size);
	
	return 0;
}

#endif
