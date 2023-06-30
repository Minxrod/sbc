#pragma once

#include "common.h"

#define MAX_STRLEN 256

// string meta
// type can be 'S' or 'W'
enum string_type {
	STRING_EMPTY = 'e',
	STRING_CHAR = 's',
	STRING_WIDE = 'w',
};

// string table info
struct string {
	/// Type of string
	char type;
	/// Length of string (characters)
	u8 len;
	/// Number of references to this string
	u32 uses;
	/// Pointer to string data
	union {
		/// 8-bit char string pointer
		u8 **s;
		/// 16-bit wide string pointer
		u16 **w;
	} ptr;
};

struct strings {
	/// Max strings allocated
	u32 strs_max;
	/// Next string data offset
	/// TODO: Use as start point to search
	u32 str_next;
	/// String info array
	struct string* strs;
	/// String data
	void* str_data;
};

// allocate str table and memory for `str_count` max strings
void init_mem_str(struct strings* s, int str_count, enum string_type str_type);

struct string* get_new_str(struct strings* s);

//Convert char to wide char
u16 to_wide(u8 c);

// Copy a string of variable type to a wide string buffer
void str_copy(void* src, u16* dest);

// Compare two strings for equality
void str_comp(void* str1, void* str2);

// Concatenate two strings
void str_concat(void* str1, void* str2, void* dest);
