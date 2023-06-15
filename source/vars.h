#pragma once

#include "common.h"

enum var_types {
	VAR_NUMBER,
	VAR_STRING,
	VAR_ARRAY_NUMBER,
	VAR_ARRAY_STRING,
	VAR_ARRAY2D_NUMBER,
	VAR_ARRAY2D_STRING,
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

struct variable {
	u32 type;
	union value value;
};

struct named_var {
	u32 type;
	char name[16];
	union value value;
};


// number can be stored directly

enum string_types {
	STRING_EMPTY = 's',
	STRING_CHAR = 'S',
	STRING_WIDE = 'W',
};

extern const int MAX_STRLEN;
// string meta
// type can be 'S' or 'W'
struct string {
	char type;
	u8 len;
	union data {
		u8* s;
		u16* w;
	} data;
};

// dim_2 = 0xffffffff -> 1d array else 2d array
// value* either points to array of numbers or array of pointers to strings
struct array {
	u32 dim_1_size;
	u32 dim_2_size;
	union value* value;
};

