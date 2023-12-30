#include <stdlib.h>
#include <stdio.h>

#include "interpreter/vars.h"
#include "common.h"
#include "system.h"
#include "resources.h"
#include "stack.h"

#include "interpreter.h"
#include "program.h"
#include "graphics/display.h"

struct ptc* init_system(int var, int str, int arr){
	iprintf("init_system calloc: %d\n", (int)sizeof(struct ptc));
	struct ptc* ptc = calloc(sizeof(struct ptc), 1);
	if (ptc == NULL){
		iprintf("Error allocating memory!\n");
		abort();
	}
//	*ptc = (const struct ptc){0}; // see if this reduces stack
	
	// init vars memory
	init_mem_var(&ptc->vars, var);
	init_mem_str(&ptc->strs, str, STRING_CHAR);
//	iprintf("%p %p %p %p\n", (void*)ptc->vars.vars, (void*)ptc->strs.strs, ptc->strs.str_data, NULL);
//	return ptc;
	init_mem_arr(&ptc->arrs, var, arr);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// init various ptc items
	// TODO:CODE:LOW use subsytem initialization functions?
	ptc->console.tabstep = 4;
	ptc->res.visible = VISIBLE_ALL;
	init_input(&ptc->input);
	init_sprites(&ptc->sprites);
	init_graphics(&ptc->graphics);
	
	init_resource(&ptc->res);
	init_display(ptc); // needs resources as well
	
	// must occur after resources as it depend on SCR
	init_panel(ptc);
	
	return ptc;
}

void free_system(struct ptc* p){
	free_panel(p);
	free_display(&p->display);
	free_resource(&p->res);
	free_mem_arr(&p->arrs);
	free_mem_str(&p->strs);
	free_mem_var(&p->vars);
	free(p);
}

//https://smilebasicsource.com/forum/thread/docs-ptc-acls
char acls_code[] = 
"VISIBLE 1,1,1,1,1,1:ICONCLR\r"
"COLOR 0,0:CLS:'GDRAWMD FALSE\r"
"FOR P=1 TO 0 STEP -1\r"
" GPAGE P,P,P:GCOLOR 0:GCLS:'GPRIO 3\r"
" BGPAGE P:BGOFS 0,0,0:BGOFS 1,0,0\r"
" BGCLR:BGCLIP 0,0,31,23\r"
" SPPAGE P:SPCLR\r"
"NEXT\r"
"FOR I=0 TO 255\r"
" 'COLINIT \"BG\", I:COLINIT \"SP\", I\r"
" 'COLINIT \"GRP\",I\r"
"NEXT\r";

char acls_bytecode[2*sizeof acls_code];
u8 acls_bc_lines[12];

void cmd_acls(struct ptc* p){
	// copy vars and use a temp variables for this snippet
	struct runner cur_exec = p->exec; // copy code state
	struct variables temp_vars = {0};
	init_mem_var(&temp_vars, 4); //P,I
	struct variables vars = p->vars;
	p->vars = temp_vars;
	
	struct program acls_program = { sizeof(acls_code), acls_code };
	struct bytecode acls_bc = init_bytecode(sizeof(acls_code));
	
	tokenize(&acls_program, &acls_bc);
	
	run(acls_bc, p);
	p->exec = cur_exec;
	
	// Restore proper program variable state
	p->vars = vars;
}

void cmd_visible(struct ptc* p){
	// TODO:IMPL:MED
	p->stack.stack_i = 0;
}

void cmd_vsync(struct ptc* p){
	int delay = STACK_INT(0);
	
	int64_t start_time = get_time(&p->time);
	while (get_time(&p->time) - start_time < delay){
		// wait for 10ms before checking again
		thrd_sleep(&(struct timespec){.tv_nsec=(1e7)}, NULL);
	}
}

void cmd_wait(struct ptc* p){
	int delay = STACK_INT(0);
	
	// wait for duration of delay
	int big = delay / 60;
	int small = delay % 60;
	
	int64_t nsec = 1e9l * small / 60l;
	
	thrd_sleep(&(struct timespec){.tv_sec = big, .tv_nsec=nsec}, NULL);
}

void cmd_clear(struct ptc* p){
	reset_var(&p->vars);
	reset_str(&p->strs);
	reset_arr(&p->arrs);
}

