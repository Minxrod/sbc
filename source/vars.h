#pragma once
/**
 * @file vars.h
 * 
 * Variable management struct and functions.
 * 
 */

#include "common.h"

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
	/// This one is stupid, and should be refactored away eventually.
	STACK_OP=32,
};

#define VALUE_NUM(v) v->type & VAR_VARIABLE ? *(s32*)v->value.ptr : v->value.number
#define VALUE_STR(v) v->type & VAR_VARIABLE ? *(struct string**)v->value.ptr : (struct string*)v->value.ptr


// This file defines the data structures used to store variable information.

// Notes on vars:
// There are 6 types of variables.
// Number - 20.12 fixed point
// String - 0-256 character sequence
// Array - One of the above types, but an array.

// Contains either a pointer to string/array data or a number
// Or pointer to variable (this takes the form of a pointer to number or pointer to string/array)
union value {
	s32 number;
	void* ptr;
};

struct named_var {
	u32 type;
	char name[16];
	union value value;
};

struct strings;
struct arrays;

struct variables {
	/// Max variables in use
	u32 vars_max;
	/// Strings table
	struct strings* strs;
	/// Arrays table
	struct arrays* arrs;
	/// Variable table
	struct named_var* vars;
};

int var_name_hash(char*, u32, u32);
bool namecmp(char* a, u32 len, char b[16]);

// allocate var table for `var_count` variables
void init_mem_var(struct variables* v, int var_count);

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

