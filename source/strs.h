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
// Note that this table doesn't necessarily contain actual data.
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
		u8 *s;
		/// 16-bit wide string pointer
		u16 *w;
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
	/// Empty string (init_mem_str fills this with correct values)
	struct string empty;
	/// String data (memory allocated for u8[MAX_STRLEN] or u16[MAX_STRLEN])
	void* str_data;
	/// Type of strings created via get_new_str
	char type;
};

// allocate str table and memory for `str_count` max strings
void init_mem_str(struct strings* s, int str_count, enum string_type str_type);
void free_mem_str(struct strings* s);

struct string* get_new_str(struct strings* s);

//Convert char to wide char
u16 to_wide(u8 c);

//Reverse char to wide char
u8 to_char(u16 c);

// Convert number to string PTC style
void str_num(s32 num, u8* str);

// Copy to u8 str
void str_char_copy(void* src, u8* dest);

// Copy to u16 str
void str_wide_copy(void* src, u16* dest);

// Get string length
u32 str_len(void* src);

// Compare two strings for equality
bool str_comp(void* str1, void* str2);

// Copy str1 to str2
void str_copy(void* str1, void* str2);

// Concatenate two strings
void str_concat(void* str1, void* str2, void* dest);
