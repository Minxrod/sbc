#pragma once
/**
 * @file vars.h
 * 
 * Variable management struct and functions.
 * 
 */

#include "common.h"
#include "error.h"

/**
 * Variable and value types. Used for both @sa stack_entry and @sa named_var.
 */
enum types {
	/// Special value denoting an empty variable slot.
	/// Cannot be combined with any other type.
	VAR_EMPTY=0,
	/// Indicates a numeric 20.12 fixed-point value.
	/// @warn Only one of VAR_NUMBER and VAR_STRING should be selected.
	VAR_NUMBER=1,
	/// Indicates a string of maximum 256 characters.
	/// @warn Only one of VAR_NUMBER and VAR_STRING should be selected.
	VAR_STRING=2,
	/// Denotes that the value contained is a variable pointer.
	/// Can be combined with VAR_NUMBER or VAR_STRING.
	VAR_VARIABLE=4,
	/// Denotes that the value contained is an array type.
	/// Can be combined with VAR_NUMBER or VAR_STRING.
	VAR_ARRAY=8,
	/// This one is silly. It marks an operator such as , or ;.
	STACK_OP=16,
};

//TODO:CODE:LOW Should these be moved to stack.h?
#define VALUE_NUM(v) (v->type & VAR_VARIABLE ? *(s32*)v->value.ptr : v->value.number)
#define VALUE_INT(v) FP_TO_INT(VALUE_NUM(v))
#define VALUE_STR(v) (v->type & VAR_VARIABLE ? *(void**)v->value.ptr : (void*)v->value.ptr)
struct stack_entry;
void* value_str(struct stack_entry* e);

// Convenience definitions
#define STACK_INT(i) VALUE_INT(ARG(i))
#define STACK_NUM(i) VALUE_NUM(ARG(i))
#define STACK_STR(i) VALUE_STR(ARG(i))

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

// This file defines the data structures used to store variable information.

// Notes on vars:
// There are 6 types of variables.
// Number - 20.12 fixed point
// String - 0-256 character sequence
// Array - One of the above types, but an array.

// Contains either a pointer to string/array data or a number
// Or pointer to variable (this takes the form of a pointer to number or pointer to string/array)
union value {
	fixp number;
	void* ptr;
};

struct named_var {
	u8 type;
	char name[16];
	union value value;
};

struct strings;
struct arrays;

struct variables {
	/// Max variables in use
	uint_fast16_t vars_max;
	/// Strings table
	struct strings* strs;
	/// Arrays table
	struct arrays* arrs;
	/// Variable table
	struct named_var* vars;
	/// Error status for variable accesses
	enum err_code error;
};

int var_name_hash(char*, u32, u32);
bool namecmp(char* a, u32 len, char b[16]);

// allocate var table for `var_count` variables
void init_mem_var(struct variables* v, uint_fast16_t var_count);
void reset_var(struct variables* v);
void free_mem_var(struct variables* v);

// Expects null-terminated name
struct named_var* test_var(struct variables* v, char*, enum types type);
// Name and length required

/**
 * Gets a pointer to a variable using the variable name.
 * The name itself does not determine the variable type. It must be passed separately.
 */
struct named_var* get_var(struct variables* v, char*, u32, enum types type);
// Gets a pointer to a value within an array
union value* get_arr_entry(struct variables* v, char*, u32, enum types type, u32 ix, u32 iy);

struct named_var* get_new_arr_var(struct variables* v, char* name, u32 len, enum types type, u32 dim1, u32 dim2);

