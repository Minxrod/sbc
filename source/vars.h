#pragma once

#include "common.h"

#define MAX_STRLEN 256

enum var_type {
	VAR_NUMBER,
	VAR_STRING,
	VAR_ARRAY_NUMBER,
	VAR_ARRAY_STRING,
	VAR_ARRAY2D_NUMBER,
	VAR_ARRAY2D_STRING,
	VAR_EMPTY,
};

// This file defines the data structures used to store variable information.

// Notes on vars:
// There are 6 types of variables.
// Number - 20.12 fixed point
// String - 0-256 character sequence
// 

// Contains either a pointer to string/array data or a number
union value {
	s32 number;
	void* ptr;
};

/*
struct variable {
	u32 type;
	union value value;
};
*/

struct named_var {
	u32 type;
	char name[16];
	union value value;
};


// number can be stored directly

enum string_type {
	STRING_EMPTY = 's',
	STRING_CHAR = 'S',
	STRING_WIDE = 'W',
};

// string meta
// type can be 'S' or 'W'
struct string {
	char type;
	u8 len;
	union data {
		struct string_data* s;
		struct wstring_data* w;
	} data;
};

struct string_data {
	u32 uses;
	u8 s[MAX_STRLEN];
};

struct wstring_data {
	u32 uses;
	u16 s[MAX_STRLEN];
};

// dim_2 = 0xffffffff -> 1d array else 2d array
// value* either points to array of numbers or array of pointers to strings
struct array {
	u32 dim_1_size;
	u32 dim_2_size;
	union value* value;
};

struct variables {
	/// Max variables in use
	u32 vars_max;
	/// Variable table
	struct named_var* vars;
};

struct strings {
	/// Max strings allocated
	u32 strs_max;
	/// Next string data offset
	u32 str_next;
	/// String info array
	struct string* strs;
	/// String data
	void* str_data;
};

int var_name_hash(char*, u32, u32);
bool namecmp(char* a, u32 len, char b[16]);

// allocate var table for `var_count` variables
void init_mem_var(struct variables* v, int var_count);
// allocate str table and memory for `str_count` max strings
void init_mem_str(struct strings* s, int str_count, enum string_type str_type);

struct named_var* get_var(struct variables* v, char*, u32, enum var_type type);
