#include "test_util.h"

#include "error.h"
#include "system.h"
#include "program.h"

#include <assert.h>
#include <string.h>

int check_fail;

// This is a global so as to use less stack space for very limited systems
//char outcode[2048];
//u8 outcode_lines[128];

/// Fully paramtereized execution function. Intended to be used for executing test code.
/// Can be supplied with memory limits and input sequences.
/// keys is not null-terminated - length must be provided.
/// This is because \0 can be typed and is distinct from typing nothing.
struct ptc* run_code_conditions(char* code, const char* keys, int key_len, int var_limit, int str_limit, int arr_limit, int opts){
//	assert(strlen(code) < 1024); // prevents exceeding outcode buffer
//	memset(outcode, 0x0f, 2048); // identify errors of failing to write
	// init system
	MEM_CASE // only the start here!
	struct ptc* ptc = init_system(var_limit, str_limit, arr_limit);
	ptc->res.search_path = "tests/data/"; // override standard search path with test path
	ptc->console.test_mode = true;
	// compile program p into bytecode in o
	init_mem_prg(&ptc->exec.prg, MAX_SOURCE_SIZE);
    strcpy(ptc->exec.prg.data, code);
	ptc->exec.prg.size = strlen(code);
	struct bytecode o = init_bytecode();
	ptc->exec.error = tokenize_full(&ptc->exec.prg, &o, ptc, opts);
	
	// buffer inkeys
	if (keys){
		for (int i = 0; i < key_len; ++i)
			set_inkey(&ptc->input, to_wide(keys[i]));
	}
	// run code
	run(o, ptc);
	
	if (ptc->exec.error){
		iprintf("Error: %s\n", error_messages[ptc->exec.error]);
	}
	
	// 
	return ptc;
}

/// Creates a system with very low memory to test out-of-memory conditions.
struct ptc* run_code_lowmem(char* code){
	return run_code_conditions(code, NULL, 0, 4, 2, 4, TOKOPT_NONE);
}

struct ptc* run_code_keys(char* code, char* keys, int len){
	return run_code_conditions(code, keys, len, VAR_LIMIT, STR_LIMIT, ARR_LIMIT, TOKOPT_NONE);
}

// Run program with enabled opts
struct ptc* run_code_opts(char* code, int opts){
	return run_code_conditions(code, NULL, 0, VAR_LIMIT, STR_LIMIT, ARR_LIMIT, opts);
}

// reduces duplication to have this
struct ptc* run_code(char* code){
	return run_code_keys(code, NULL, 0);
}

void free_code(struct ptc* ptc){
	free_bytecode(ptc->exec.code); // must be done here to preserve string memory
	free_log("free_mem_prg", ptc->exec.prg.data);
	free_system(ptc);
	MEM_CASE_END // done here to clean up block
}

// Return value of 0 indicates success
bool check_code_error(char* code, enum err_code expected){
	struct ptc* p = run_code(code);
	
	bool result = p->exec.error == expected;
	
	free_code(p);
	
	return result;
}

#include "common.h"
#include <stdio.h>

int token_code(char* code, char* expected, int size){
	MEM_CASE
	struct program p = { strlen(code), code };
	struct bytecode o = init_bytecode();
	
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
	free_bytecode(o);
	MEM_CASE_END
	return !error;
}
