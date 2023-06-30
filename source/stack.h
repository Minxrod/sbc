#pragma once

#include "vars.h"

enum stack_flags {
	STACK_NUMBER = 1,
	STACK_STRING = 2,
	STACK_VARIABLE = 4,
	STACK_ARRAY = 8,
	STACK_OP = 16,
};

struct stack {
	u32 stack_i;
	struct stack_entry {
		u32 type;
		union value value; 
	} entry[256];
};

struct stack_entry* stack_get(struct stack* s, int index);

void stack_push(struct stack* s, struct stack_entry value);
struct stack_entry* stack_pop(struct stack* s);

