#pragma once

#include "vars.h"

#define VALUE_STACK_MAX 256
#define CALL_STACK_MAX 256

/**
 * Value stack. Each entry is a tagged union specifying the same set of types
 * from vars.h.
 * 
 */
struct value_stack {
	u32 stack_i;
	struct stack_entry {
		u32 type;
		union value value;
	} entry[VALUE_STACK_MAX];
};

struct stack_entry* stack_get(struct value_stack* s, int index);

void stack_push(struct value_stack* s, struct stack_entry value);
struct stack_entry* stack_pop(struct value_stack* s);

void stack_print(struct value_stack* s);

enum call_type {
	CALL_FOR,
	CALL_GOSUB,
};

struct call_stack {
	u32 stack_i;
	struct call_entry {
		enum call_type type; // FOR or GOSUB
		u32 address; // instruction index
		u32 var_type;
		void* var; // s32* or struct string**
	} entry[256];
};
