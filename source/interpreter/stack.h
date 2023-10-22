#pragma once

#include "vars.h"

#define VALUE_STACK_MAX 100
#define CALL_STACK_MAX 256

/**
 * Value stack. Each entry is a tagged union specifying the same set of types
 * from vars.h.
 * 
 */
struct value_stack {
	u8 stack_i; // would determine by VALUE_STACK_MAX?
	struct stack_entry {
		u8 type; // Increase if more needed?
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
	u16 stack_i;
	struct call_entry {
		enum call_type type; // FOR or GOSUB
		idx address; // instruction index
		u8 var_type;
		void* var; // fixp* or struct string**
	} entry[CALL_STACK_MAX];
};
