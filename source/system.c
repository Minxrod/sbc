#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "system.h"
#include "vars.h"

struct ptc* system_init(){
	struct ptc* ptc = malloc(sizeof(struct ptc));
	if (ptc == NULL){
		iprintf("Error allocating memory!");
		abort();
	}
	*ptc = (const struct ptc){0};
	
	// init vars memory
	init_mem_var(&ptc->vars, 16);
	init_mem_str(&ptc->strs, 32, STRING_CHAR);
	init_mem_arr(&ptc->arrs, 16, 64);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// init various ptc items
	// TODO: subsytem initialization functions?
	ptc->console.tabstep = 4;
	
	//allocate ptc struct here?
	return ptc;
}

