#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <threads.h>

#include "common.h"
#include "system.h"
#include "resources.h"
#include "stack.h"
#include "ptc.h"

#include "interpreter.h"
#include "program.h"

struct ptc* init_system(int var, int str, int arr){
	srand(time(NULL));
	
	struct ptc* ptc = calloc_log("init_system", sizeof(struct ptc), 1);
	if (ptc == NULL){
		iprintf("Error allocating memory!\n");
		abort();
	}
//	*ptc = (const struct ptc){0}; // see if this reduces stack
	init_resource(&ptc->res);
//	iprintf("%zd\n", sizeof(struct ptc));
	
	// init vars memory
	init_mem_var(&ptc->vars, var);
	init_mem_str(&ptc->strs, str, STRING_CHAR);
//	iprintf("%p %p %p %p\n", (void*)ptc->vars.vars, (void*)ptc->strs.strs, ptc->strs.str_data, NULL);
//	return ptc;
	init_mem_arr(&ptc->arrs, var, arr);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// init various ptc items
	// TODO:CODE:MED use subsytem initialization functions?
	ptc->console.tabstep = 4;
	ptc->console.sys_tabstep = INT_TO_FP(4);
	ptc->res.visible = VISIBLE_ALL;
	ptc->background.clip[0].x1 = 0;
	ptc->background.clip[0].y1 = 0;
	ptc->background.clip[0].x2 = 255;
	ptc->background.clip[0].y2 = 191;
	ptc->background.clip[1].x1 = 0;
	ptc->background.clip[1].y1 = 0;
	ptc->background.clip[1].x2 = 255;
	ptc->background.clip[1].y2 = 191;
	init_input(&ptc->input);
	init_sprites(&ptc->sprites);
	init_graphics(&ptc->graphics);
	
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
	free_log("free_system", p);
}

// modified from
// https://smilebasicsource.com/forum/thread/docs-ptc-acls
char acls_code[] = 
"VISIBLE 1,1,1,1,1,1:ICONCLR\r"
"COLOR 0,0:CLS:GDRAWMD FALSE\r"
"FOR P=1 TO 0 STEP -1\r"
" GPAGE P,P,P:GCOLOR 0:GCLS:GPRIO 3\r"
" BGPAGE P:BGOFS 0,0,0:BGOFS 1,0,0\r"
" BGCLR:BGCLIP 0,0,31,23\r"
" SPPAGE P:SPCLR\r"
" COLINIT \"BG\":COLINIT \"SP\"\r"
" COLINIT \"GRP\"\r"
"NEXT\r";

u8 acls_bytecode[2*sizeof(acls_code)];
u8 acls_bc_lines[12];
struct labels acls_labels = {0};

void cmd_acls(struct ptc* p){
	// copy vars and use a temp variables for this snippet
	struct runner cur_exec = p->exec; // copy code state
	struct variables temp_vars = {0};
	init_mem_var(&temp_vars, 4); //P,I
	struct variables vars = p->vars;
	p->vars = temp_vars;
	
	struct program acls_program = { sizeof(acls_code)-1, acls_code };
	struct bytecode acls_bc = {
		0, acls_bytecode, acls_bc_lines, init_labels(0)
	};
	
	p->exec.error = tokenize(&acls_program, &acls_bc);
	run(acls_bc, p);
	if (p->exec.error) return;
	p->exec = cur_exec;
	
	// Restore proper program variable state
	p->vars = vars;
	free_mem_var(&temp_vars);
}

void cmd_visible(struct ptc* p){
	if (p->stack.stack_i != 6){
		ERROR(ERR_SYNTAX);
	}
	u8 visible_flags = 0;
	for (int i = 0; i < 6; ++i){
		struct stack_entry* e = stack_get(&p->stack, i);
		if (e->type & VAR_NUMBER){
			int bit;
			STACK_INT_RANGE(i,0,1,bit);
			if (bit){
				visible_flags |= bit << i;
			} else {
				visible_flags &= ~(1 << i) & 0x3f;
			}
			iprintf("%d\n", bit);
		} else if (e->type == STACK_OP && e->value.number == OP_COMMA){
			// keep previous value if 
			visible_flags |= (p->res.visible & (1 << i));
		} else if (e->type & VAR_STRING){
			ERROR(ERR_TYPE_MISMATCH);
		} else { // invalid argument, shouldn't usually happen
			ERROR(ERR_SYNTAX);
		}
	}
	p->res.visible = visible_flags;
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
//	int delay = STACK_INT(0);
	
	// wait for duration of delay
//	int big = delay / 60;
//	int small = delay % 60;
	
//	int64_t nsec = 1e9l * small / 60l;

	int delay = STACK_INT(0);
	
	int64_t start_time = get_time(&p->time);
	
	//thrd_sleep(&(struct timespec){.tv_sec = big, .tv_nsec=nsec}, NULL);
	while (get_time(&p->time) - start_time < delay){
		// wait for 10ms before checking again
		thrd_sleep(&(struct timespec){.tv_nsec=(1e7)}, NULL);
	}
}

void cmd_clear(struct ptc* p){
	reset_var(&p->vars);
	reset_str(&p->strs);
	reset_arr(&p->arrs);
}

// TODO:CODE:LOW Move the variable functions (CLEAR, etc. to elsewhere?)
void cmd_swap(struct ptc* p){
	// Note: Both are variables and types are already the same via tok_test check
	// Note that swap is swapping values, but those values depend on types
	struct stack_entry* a = ARG(0);
	struct stack_entry* b = ARG(1);
	union value c;
	memcpy(&c, a->value.ptr, sizeof(union value)); // copies from pointed-to number or string
	memcpy(a->value.ptr, b->value.ptr, sizeof(union value));
	memcpy(b->value.ptr, &c, sizeof(union value));
}

#define MAX_SORT_ARRAYS 32

static inline void _sort_swap(union value* data, union value** extra, int i, int j){
	union value temp = data[i];
	data[i] = data[j];
	data[j] = temp;
	for (int k = 0; k < MAX_SORT_ARRAYS; ++k){
		if (extra[k]){
			temp = extra[k][i];
			extra[k][i] = extra[k][j];
			extra[k][j] = temp;
		} else {
			return;
		}
	}
}

typedef bool(*comparison)(union value, union value);

// based on
// https://en.wikipedia.org/wiki/Quicksort?useskin=monobook#cite_ref-:2_13-2
int _partition(union value* data, union value** extra, int start, int end, comparison comp){
	if (end - start <= 1){
		return start; // already sorted
	}
	
	union value pivot = data[start+(end - start)/2]; // select a random pivot
	--start;
	++end;
	
	while (true){
		do {
			++start;
		} while (comp(data[start], pivot));
		do {
			--end;
		} while (comp(pivot, data[end]));
		if (start >= end) return start;
		
		_sort_swap(data, extra, start, end);
	}
}

bool fixp_sort_comp(union value a, union value b){
	return a.number < b.number;
}

bool fixp_rsort_comp(union value a, union value b){
	return a.number > b.number;
}

bool str_sort_comp(union value a, union value b){
	int size_a = str_len(a.ptr);
	int size_b = str_len(b.ptr);
	int size = size_a < size_b ? size_a : size_b;
	for (int i = 0; i < size; ++i){
		u16 chr_a = str_at_wide(a.ptr, i);
		u16 chr_b = str_at_wide(b.ptr, i);
		if (chr_a < chr_b){
			return true;
		} else if (chr_b < chr_a){
			return false;
		}
	}
	return size_a < size_b;
}

bool str_rsort_comp(union value a, union value b){
	int size_a = str_len(a.ptr);
	int size_b = str_len(b.ptr);
	int size = size_a < size_b ? size_a : size_b;
	for (int i = 0; i < size; ++i){
		u16 chr_a = str_at_wide(a.ptr, i);
		u16 chr_b = str_at_wide(b.ptr, i);
		if (chr_a == chr_b) continue;
		return chr_a > chr_b;
	}
	return size_a > size_b;
}

void quick_sort(union value* data, union value** extra, int start, int end, comparison comp){
	// https://en.wikipedia.org/wiki/Quicksort?useskin=monobook#Hoare_partition_scheme
	if (end - start <= 1) return;
	
	int p = _partition(data, extra, start, end, comp);
	quick_sort(data, extra, start, p, comp);
	quick_sort(data, extra, p + 1, end, comp);
}

/// @note Does not correctly handle permuting the same array as is being sorted!
void _cmd_sort_shared(struct ptc* p, comparison comp){
	int start;
	int size;
	union value* extra[MAX_SORT_ARRAYS] = {0};
	if (p->stack.stack_i > MAX_SORT_ARRAYS+3){
		ERROR(ERR_SYNTAX); // 32 is max number of arrays
	}
	// SORT start, size, arr, [arr1, arr2, ...]
	// TODO:ERR:MED Check that arrays are not 2D
	union value* data = *(void**)stack_get(&p->stack, 2)->value.ptr;
//	iprintf("%p\n", (void*)data);
	int array_min_size = arr_size(data, ARR_DIM1);
	for (int k = 3; k < p->stack.stack_i; ++k){
		extra[k-3] = *(union value**)stack_get(&p->stack, k)->value.ptr;
		if ((int)arr_size(extra[k-3], ARR_DIM1) < array_min_size){
			array_min_size = arr_size(extra[k-3], ARR_DIM1);
		}
		if (extra[k-3] == data){
			// This does not work in this implemtation but would be valid in PTC
			// TODO:IMPL:NONE Implement this...? (Is it possible with remaining memory?)
			ERROR(ERR_SORT_SAME_ARRAY);
		}
		extra[k-3] += 2; // move past dimension info
	}
//	iprintf("%d\n", array_min_size);
	STACK_INT_RANGE(0,0,array_min_size-1,start);
	STACK_INT_RANGE(1,0,array_min_size-start,size);
	
	// now, begin sort
	// +2 to skip dimension info stored in array memory
	quick_sort(data+2, extra, start, start + size - 1, comp);
}

void cmd_sort(struct ptc* p){
	_cmd_sort_shared(p, ARG(2)->type & VAR_STRING ? str_sort_comp : fixp_sort_comp);
}

void cmd_rsort(struct ptc* p){
	_cmd_sort_shared(p, ARG(2)->type & VAR_STRING ? str_rsort_comp : fixp_rsort_comp);
}

const char* bench_begin = "ACLS:CLEAR\r";
const char* bench_mid = "FOR J=1 TO 5\r"
"\r"
"M=POW(10,J)\r"
"\r"
"T=MAINCNTL\r"
"FOR I=1 TO M\r";
const char* bench_end = "NEXT\r"
"S=MAINCNTL-T\r"
"\r"
"?M,S\r"
"IF S>=1000 THEN @END\r"
"NEXT\r"
"@END\r";

int sbc_benchmark(struct launch_info* info){
	char src[1024] = {0};
	char pre[100];
	char code[100];
	char post[100];
	struct program p = { 0, src };
	struct bytecode bc = info->p->exec.code;
	
	FILE* f = fopen("resources/sbccompat.txt","r");
	if (!f){
		iprintf("Failed to open benchmark tests file!\n");
		return 1;
	}
	FILE* result = fopen("sbccompat_out", "w");
	if (!result){
		iprintf("Failed to open benchmark results file!\n");
		fclose(f);
		return 1;
	}
	int index = 0;
	while (!feof(f)){
		fgets(pre, 100, f);
		fgets(code, 100, f);
		fgets(post, 100, f);
		for (int i = 0; i < 100; ++i){
			if (pre[i] == '\\') pre[i] = '\r';
			if (code[i] == '\\') code[i] = '\r';
			if (post[i] == '\\') post[i] = '\r';
			if (pre[i] == '\n') pre[i] = '\r';
			if (code[i] == '\n') code[i] = '\r';
			if (post[i] == '\n') post[i] = '\r';
		}
		sprintf(src, "%s?%d\r%s%s%s%s%s%n", bench_begin, index, pre, bench_mid, code, bench_end, post, (int*)&p.size);
//		fprintf(f2, "%s", src);
		info->p->exec.error = ERR_NONE; // reset for next run
		info->p->exec.error = tokenize_full(&p, &bc, info->p, TOKOPT_VARIABLE_IDS);
		if (info->p->exec.error == ERR_NONE){
			run(bc, info->p);
		}
		
		if (info->p->exec.error == ERR_NONE){
			fixp num = test_var(&info->p->vars, "S", VAR_NUMBER)->value.number;
			fixp lev = test_var(&info->p->vars, "J", VAR_NUMBER)->value.number;
			int perf = FP_TO_INT(num);
			for (int i = FP_TO_INT(lev); i < 5; ++i){
				perf *= 10;
			}
			fprintf(result, "%d\n", perf);
		} else {
			fprintf(result, "%s\n", error_messages[info->p->exec.error]);
		}
		++index;
	}
	fclose(f);
	fclose(result);
//	free_bytecode(bc);
	return 0;
}

// This is the DIRECT mode prompt.
// Pretty basic, but it works.
// TODO:IMPL:LOW Redesign to remove prompt symbol
struct program launcher = {
	13, "LINPUT CODE$\r"
};

enum launch_state {
	LAUNCH_PROMPT,
	LAUNCH_EDIT,
	LAUNCH_RUN,
	LAUNCH_BENCH,
	LAUNCH_AUTOLOAD,
};

// this is practically the main function, barring some platform-specific intialization
int launch_system(void* launch_info){
	u8 err_msg[66] = {STRING_INLINE_CHAR}; // 2 lines + metadata
	struct launch_info* info = (struct launch_info*)launch_info;
	struct ptc* p = info->p;
	
	// DIRECT setup
	struct bytecode prompt_bc = init_bytecode_size(launcher.size, 1, 0); // TODO:CODE:NONE This isn't guaranteed to be safe, but I'm also not sure if the program can be made easily dangerous
	tokenize_full(&launcher, &prompt_bc, p, TOKOPT_NONE);
	
	// editor setup
	// TODO:PERF:MED Reduce memory fragmentation by loading program into bytecode memory...?
	struct program editor = {0};
	load_prg_alloc(&editor, "programs/EDITOR.PTC"); // TODO:CODE:MED move EDITOR to resources
	struct bytecode editor_bc = init_bytecode_size(editor.size, 500, 64); // TODO:CODE:NONE This isn't guaranteed to be safe, but I'm also not sure if the program can be made easily dangerous
	tokenize_full(&editor, &editor_bc, p, TOKOPT_VARIABLE_IDS);
	free_log("editor temp load", editor.data);
	
	// for user programs / "RUN" setup
	struct bytecode bc = init_bytecode_size(MAX_SOURCE_SIZE, MAX_LINES, MAX_LABELS);
	if (!bc.labels.entry || !bc.data){
		iprintf("Failed to allocate bytecode...\n");
		return -1;
	}
	init_mem_prg(&p->exec.prg, MAX_SOURCE_SIZE);
	if (!p->exec.prg.data){
		iprintf("Failed to allocate program...\n");
		return -1;
	}
	
	// Setup complete
	con_puts(&p->console, "S\45Small BASIC Computer            READY");
	con_newline(&p->console, true);
	bool running = true;
	
	int state = LAUNCH_PROMPT;
	if (info->prg_filename){
		state = LAUNCH_AUTOLOAD;
	}
	
	// TODO:IMPL:LOW Add configuration method for optimizations
	int opts = TOKOPT_NONE;
	while (running){
		p->exec.error = ERR_NONE; // prepare for next execution
		p->calls.stack_i = 0; // TODO:IMPL:MED Determine a better way to handle this
		int old_state = state;
		iprintf("MAIN state=%d\n", state);
		switch (state){
			case LAUNCH_PROMPT: // DIRECT mode
				{
				run(prompt_bc, p); // get prompt
				
				void* cmd = get_var(&p->vars, "CODE", 4, VAR_STRING)->value.ptr;
				if (str_comp(cmd, "S\011REM BENCH")){
					state = LAUNCH_BENCH;
					break;
				} else if (str_comp(cmd, "S\3RUN")){
					state = LAUNCH_RUN;
					break;
				}
				// check status
				if (p->exec.error == ERR_BUTTON_SIGNAL){
					int sig = p->exec.error_info[0];
					if (sig == 72){ // EDIT
						state = LAUNCH_EDIT;
					}
					p->exec.error = ERR_NONE; // clear signal
					p->exec.error_info[0] = 0; // clear signal info
					break;
				}
				
				// execute single line
				// create small program
				u8 direct_cmd[32+1] = {0};
				struct program prog = {0, (char*)direct_cmd};
				str_char_copy(cmd, direct_cmd);
				prog.size = str_len(cmd);
				direct_cmd[prog.size++] = '\r';
				
				// prepare execution
				p->exec.error = ERR_NONE;
				p->calls.stack_i = 0; // TODO:IMPL:MED Determine a better way to handle this
				p->exec.error = tokenize_full(&prog, &bc, p, TOKOPT_NONE);
				if (p->exec.error == ERR_NONE){
					run(bc, p);
				}
				}
				break;
				
			case LAUNCH_EDIT: // EDIT mode
				p->exec.error = ERR_NONE;
				get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(0);
				run(editor_bc, p);
				int sig = p->exec.error_info[0];
				// TODO:IMPL:LOW style buttons corrcetly for each mode
				if (p->exec.error == ERR_BUTTON_SIGNAL){
					if (sig == 71){ // RUN/DIRECT
						state = LAUNCH_PROMPT;
						// copy program to memory
						p->exec.error = ERR_NONE;
						p->exec.error_info[0] = 0; // clear signal info
						get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(3);
						run(editor_bc, p);
						// reset screen
						get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(2);
						run(editor_bc, p);
					}
				} else if (p->exec.error == ERR_USER_SIGNAL){
					if (sig == 1){ // run program
						state = LAUNCH_RUN;
						p->exec.error = ERR_NONE;
						p->exec.error_info[0] = 0; // clear signal info
						// copy program to memory
						get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(3);
						run(editor_bc, p);
						// reset screen
						get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(2);
						run(editor_bc, p);
					}
				} else {
					state = LAUNCH_PROMPT;
					// crash to console to allow reading error message
				}
				// set this to allow EDITOR to be correctly started via LOAD/RUN or EXEC, if wanted
				get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(1);
				break;
				
			case LAUNCH_RUN: // 'RUN' mode
				p->exec.error = tokenize_full(&p->exec.prg, &bc, p, opts);
				if (p->exec.error == ERR_NONE){
					run(bc, p);
				}
				state = LAUNCH_PROMPT; // program terminated: return to DIRECT mode
				break;
				
			case LAUNCH_BENCH: // benchmark tests
				p->exec.code = bc; // ensure it has correct resource
				sbc_benchmark(launch_info);
				state = LAUNCH_PROMPT; // return to DIRECT when done
				break;
				
			case LAUNCH_AUTOLOAD: // auto load program
				load_prg(&p->exec.prg, info->prg_filename);
				state = LAUNCH_RUN;
				break;
				
			default:
				return p->exec.error;
		}
		
		(void)old_state;
		// Display error status (for any given execution)
		iprintf("Error: %s\n", error_messages[p->exec.error]);
		
		strcpy((char*)err_msg + 2, error_messages[p->exec.error]);
		err_msg[1] = strlen(error_messages[p->exec.error]);
		strcpy((char*)err_msg + 2 + err_msg[1], p->exec.error_info);
		err_msg[1] += strlen(p->exec.error_info);
		
		con_puts(&p->console, &err_msg);
		con_newline(&p->console, true);
		// clear error status for next execution
		p->exec.error = ERR_NONE;
		p->exec.error_info[0] = '\0'; // remove error string
	}
	
	free_log("free exec.prg.data", p->exec.prg.data);
	free_bytecode(bc);
	return p->exec.error;
}


