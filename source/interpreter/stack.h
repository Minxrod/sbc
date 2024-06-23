#pragma once
///
/// @file
/// @brief Stack definitions and convenience macros.
///

#include "vars.h"
#include "strs.h"

#include <assert.h>

// Get value of variable or literal
#define VALUE_NUM(v) (v->type & VAR_VARIABLE ? *(s32*)v->value.ptr : v->value.number)
#define VALUE_INT(v) FP_TO_INT(VALUE_NUM(v))
#define VALUE_STR(v) (v->type & VAR_VARIABLE ? *(void**)v->value.ptr : (void*)v->value.ptr)

// Convenience definitions
#define STACK_INT(i) VALUE_INT(ARG(i))
#define STACK_NUM(i) VALUE_NUM(ARG(i))
#define STACK_STR(i) value_str(ARG(i))

#define STACK_REL_INT(i) STACK_INT(p->stack.stack_i + i)
#define STACK_REL_NUM(i) STACK_NUM(p->stack.stack_i + i)
#define STACK_REL_STR(i) STACK_STR(p->stack.stack_i + i)

// Error checking definitions
#define STACK_INT_RANGE(i,min,max,dest) { int _v = STACK_INT(i); if (_v < min || _v > max) { ERROR(ERR_OUT_OF_RANGE); } dest = _v; }
#define STACK_INT_RANGE_SILENT(i,min,max,dest) { int _v = STACK_INT(i); if (_v < min || _v > max) { return; } dest = _v; }
#define STACK_INT_MIN(i,min,dest) { int _v = STACK_INT(i); if (_v < min) { ERROR(ERR_OUT_OF_RANGE); } dest = _v; }
#define STACK_REL_INT_RANGE(i,min,max,dest) { int _v = STACK_REL_INT(i); if (_v < min || _v > max) { ERROR(ERR_OUT_OF_RANGE); } dest = _v; }
#define STACK_REL_INT_RANGE_SILENT(i,min,max,dest) { int _v = STACK_REL_INT(i); if (_v < min || _v > max) { return; } dest = _v; }

#define STACK_REL_INT_MIN(i,min,dest) { int _v = STACK_REL_INT(i); if (_v < min) { ERROR(ERR_OUT_OF_RANGE); } dest = _v; }
#define STACK_REL_INT_MIN_SILENT(i,min,dest) { int _v = STACK_REL_INT(i); if (_v < min) { return; } dest = _v; }

#define STACK_RETURN_INT(val) { stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {INT_TO_FP((val))}}); return; }
#define STACK_RETURN_NUM(val) { stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {(val)}}); return; }
#define STACK_RETURN_STR(val) { stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = (val)}}); return; }

#define VALUE_STACK_SIZE 100
#define CALL_STACK_SIZE 256

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
	} entry[VALUE_STACK_SIZE];
};

// This function replaces the VALUE_STR macro in places where use count must be
// decreased for temporary values living on the stack.
static inline void* value_str(struct stack_entry* e){
	void* value = VALUE_STR(e);
	// Check for only VAR_STRING, and we only care about alloc'd values.
	// If the value is alloc'd and being read here, reduce usages as it was
	// removed from the stack.
	assert(value);
	if (e->type == VAR_STRING && ((struct string*)value)->type == STRING_CHAR){
		assert(((struct string*)value)->uses > 0);
		((struct string*)value)->uses--;
	}
	return value;
}

// Inlining the stack functions improved speed from ~380 to ~360
// (Placing in ICTM changed to 370 instead; consider again if running out of space...?

/// Reads an entry from the stack by index.
///
/// @param s Stack to read
/// @param index Stack entry to read from
/// @return Pointer to value in stack
static inline struct stack_entry* stack_get(struct value_stack* s, int index){
	assert(0 <= index && index < s->stack_i);
	return &s->entry[index];
}

/// Pushes an entry onto the top of the value stack
///
/// @param s Stack to modify
/// @param value Value to be add to the stack.
static inline void stack_push(struct value_stack* s, struct stack_entry value){
	assert(s->stack_i < VALUE_STACK_SIZE);
	s->entry[s->stack_i++] = value;
}

/// Pop an entry from the top of the value stack
///
/// @warning The entry is stored in the space that will be considered "available" for future stack pushes.
/// Therefore, do not write to the stack until you are done reading from this entry.
///
/// @param s Stack to pop from
/// @return Pointer to stack entry
// TODO:PERF:NONE see if value copy is faster?
static inline struct stack_entry* stack_pop(struct value_stack* s){
	assert(s->stack_i > 0);
	return &s->entry[--s->stack_i];
}

void stack_print(struct value_stack* s);

enum call_type {
	CALL_FOR,
	CALL_GOSUB,
};

/// Represents the call stack of the running BASIC program.
struct call_stack {
	/// Index to the first unused stack entry.
	uint_fast16_t stack_i;
	/// Represents an entry in the call stack.
	/// Depending on the type of call, stores either...
	/// * GOSUB - Return index
	/// * FOR - Loop info index + length, variable pointer
	struct call_entry {
		enum call_type type; // FOR or GOSUB
		u32 address; // combined instruction index and expression size for FOR
//		uint_fast8_t var_type;
		void* var; // fixp* or struct string**
	} entry[CALL_STACK_SIZE];
};
