#pragma once

#include "common.h"

enum types {
	VAR_NUMBER=1,
	VAR_STRING=2,
	VAR_VARIABLE=4,
	VAR_ARRAY=8,
	VAR_EMPTY=16,
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

struct variables {
	/// Max variables in use
	u32 vars_max;
	/// Strings table
	struct strings* strs;
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
struct named_var* get_var(struct variables* v, char*, u32, enum types type);
