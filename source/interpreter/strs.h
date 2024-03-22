#pragma once
///
/// @file
/// @brief The strings library.
/// 
/// This file defines the various string functions in use by SBC. This includes
/// the string data structures, types, and several string functions.
///
/// This file also contains some utility functions for determining character
/// type, as well as the name hash and comparison functions.
/// 
/// This strings library is designed to allow for several different
/// types of strings to be used, in a variety of data formats. This allows
/// the string that best fits a given scenario to be used easily.
/// 

#include "common.h"

#define MAX_STRLEN 256

#define ERR_NUMBER_OUT_OF_RANGE 0x0123456789abcdef

extern char* single_char_strs;
extern char* empty_str;
extern char* hex_digits;

// string meta
// type can be 'S' or 'W'
enum string_type {
	STRING_CHAR = 's',
	STRING_WIDE = 'w',
	STRING_INLINE_CHAR = '8',
	STRING_INLINE_WIDE = 'X',
};

// string table info
// Note that this table doesn't necessarily contain actual data.
struct string {
	/// Type of string
	char type;
	/// Length of string (characters)
	uint_fast16_t len;
	/// Number of references to this string
	uint_fast16_t uses;
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
	uint_fast16_t strs_max;
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
void init_mem_str(struct strings* s, const uint_fast16_t str_count, const enum string_type str_type);
void reset_str(struct strings* s);
void free_mem_str(struct strings* s);

struct string* get_new_str(struct strings* s);

bool is_lower(const char c);
bool is_upper(const char c);
bool is_number(const char c);
bool is_alpha(const char c);
bool is_alphanum(const char c);
bool is_name_start(const char c);
bool is_name(const char c);
bool is_varname(const char c);

u32 name_hash(const char* name, const u32 len, const u32 hmax);
bool namecmp(const char* a, const u32 len, const char b[16]);

//Convert char to wide char
u16 to_wide(const u8 c);

//Reverse char to wide char
u8 to_char(const u16 c);

//Convert a (hex) digit into an integer;
int digit_value(const u16 c);

// Checks if a given u16 can be converted to a u8 without loss
bool is_char(const u16 w);

int64_t u8_to_number(u8* data, const int len, const int base, const bool allow_decimal);

// Convert a string to a number, for any base in 2-16.
// (Only 2,10,16 support is required for PTC)
int64_t str_to_number(const void*, const int base, const bool allow_decimal);

// Convert number to string PTC style + sets length
void fixp_to_str(const fixp num, void* str);

// Convert number to u8* PTC style
void fixp_to_char(const fixp num, u8* str);

// Convert string to number PTC style
int64_t u8_to_num(u8* data, const idx len);

// Constants for types mask
// If set true, use u16, else u8 characters
#define STR_COPY_SRC_8 0
#define STR_COPY_DEST_8 0
#define STR_COPY_SRC_16 1
#define STR_COPY_DEST_16 2

// Copy from src buffer to destination buffer
void str_copy_buf(const void* src, void* dest, const u8 types, const u16 count);

// Get pointer to character at location
void* str_at(const void* src, const u16 index);

// Get character (wide) at current location
u16 str_at_wide(const void* src, const u16 index);

// Get character (small) at current location
u16 str_at_char(const void* src, const u16 index);

// Copy to u8 str
void str_char_copy(const void* src, u8* dest);

// Copy to u16 str
void str_wide_copy(const void* src, u16* dest);

/// Determines if the string uses 8-bit or 16-bit characters.
/// Intended for use in determining types for `str_copy_buf()`.
/// 
/// @param str String to get type of
/// @return STR_COPY_SRC_8 if string contains u8 characters.
/// STR_COPY_SRC_16 if string contains u16 characters.
int str_type(const void* str);

// Get string length
u32 str_len(const void* src);

// Set string length
void str_set_len(void* src, int len);

// Compare two strings for equality
bool str_comp(const void* str1, const void* str2);

/// Copy str1 to str2, reusing str2's existing buffer
/// @note Sets length of str2 to that of str1
/// @note Does not modify uses
void str_copy(const void* str1, void* str2);

// Concatenate two strings
bool str_concat(const void* str1, const void* str2, void* dest);
