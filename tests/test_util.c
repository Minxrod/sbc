#include "test_util.h"

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
	struct ptc* ptc = init_system(16, 32, 64);
	ptc->console.test_mode = true;
	// run code
	run(&o, ptc);
	
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
