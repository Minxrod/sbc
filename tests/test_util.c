#include "test_util.h"

#include "error.h"

int check_fail;

char outcode[2048];

struct ptc* run_code(char* code){
	struct program p = {
		strlen(code), code,
	};
	struct program o = {
		0, outcode,
	};
	
	// compile program
	tokenize(&p, &o);
	// init vars memory
	struct ptc* ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT);
	ptc->console.test_mode = true;
	// run code
	run(&o, ptc);
	
	if (ptc->exec.error){
		iprintf("Error: %s\n", error_messages[ptc->exec.error]);
	}
	
	return ptc;
}

struct ptc* run_code_keys(char* code, char* keys, int len){
	struct program p = {
		strlen(code), code,
	};
	struct program o = {
		0, outcode,
	};
	
	// compile program
	tokenize(&p, &o);
	// init vars memory
	struct ptc* ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT);
	ptc->console.test_mode = true;
	// buffer inkeys
	for (int i = 0; i < len; ++i)
		set_inkey(&ptc->input, to_wide(keys[i]));
	// run code
	run(&o, ptc);
	
	if (ptc->exec.error){
		iprintf("Error: %s\n", error_messages[ptc->exec.error]);
	}
	
	return ptc;
}

void free_code(struct ptc* ptc){
	free_system(ptc);
}

#include "common.h"
#include <stdio.h>

int token_code(char* code, char* expected, int size){
	struct program p = { strlen(code), code };
	struct program o = { 0, outcode };
	
	bool error = 0;
	// compile program
	tokenize(&p, &o);
	for (int i = 0; i < size; i+=1){
		char c = o.data[i];
		iprintf("%c:%d,", c > 32 ? c : '?', o.data[i]);
		if (o.data[i] != expected[i]){
			iprintf("Error\n");
			error = 1;
		}
	}
	iprintf("\n");
	return !error;
}
