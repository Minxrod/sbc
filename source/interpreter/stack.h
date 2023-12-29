#pragma once

#include "vars.h"

#include <assert.h>

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
		uint_fast8_t type; // Increase if more needed?
		union value value;
	} entry[VALUE_STACK_MAX];
};

// Inlining the stack functions improved speed from ~380 to ~360
// (Placing in ICTM changed to 370 instead; consider again if running out of space...?
static inline struct stack_entry* stack_get(struct value_stack* s, int index){
	return &s->entry[index];
}

static inline void stack_push(struct value_stack* s, struct stack_entry value){
	assert(s->stack_i < VALUE_STACK_MAX);
	s->entry[s->stack_i++] = value;
}

static inline struct stack_entry* stack_pop(struct value_stack* s){
	assert(s->stack_i > 0);
	return &s->entry[--s->stack_i];
}

//struct stack_entry* stack_get(struct value_stack* s, int index) ITCM_CODE;

//void stack_push(struct value_stack* s, struct stack_entry value) ITCM_CODE;
//struct stack_entry* stack_pop(struct value_stack* s) ITCM_CODE;

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
		uint_fast8_t var_type;
		void* var; // fixp* or struct string**
	} entry[CALL_STACK_MAX];
};
