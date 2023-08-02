#include "test_util.h"

int check_fail;

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
	init_mem_str(&ptc->strs, 32, STRING_CHAR);
	init_mem_arr(&ptc->arrs, 16, 64);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// init various ptc items
	ptc->console.tabstep = 4;
	// run code
	run(&o, ptc);
}

void free_code(struct ptc* ptc){
	free(ptc->vars.vars);
	free(ptc->vars.strs->strs);
	free(ptc->vars.strs->str_data);
	free(ptc->vars.arrs->arr_data);
}
