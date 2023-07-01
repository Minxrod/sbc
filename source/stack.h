#pragma once

#include "vars.h"

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

void stack_print(struct stack* s);
