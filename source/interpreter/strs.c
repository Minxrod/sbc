#include "strs.h"
#include "ptc.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

//Note: Should be const, but need to pass void* into this.
char* single_char_strs_src = "S\1\x00S\1\x01S\1\x02S\1\x03S\1\x04S\1\x05S\1\x06S\1\x07S\1\x08S\1\x09S\1\x0aS\1\x0bS\1\x0cS\1\x0dS\1\x0eS\1\x0f"
"S\1\x10S\1\x11S\1\x12S\1\x13S\1\x14S\1\x15S\1\x16S\1\x17S\1\x18S\1\x19S\1\x1aS\1\x1bS\1\x1cS\1\x1dS\1\x1eS\1\x1f"
"S\1\x20S\1\x21S\1\x22S\1\x23S\1\x24S\1\x25S\1\x26S\1\x27S\1\x28S\1\x29S\1\x2aS\1\x2bS\1\x2cS\1\x2dS\1\x2eS\1\x2f"
"S\1\x30S\1\x31S\1\x32S\1\x33S\1\x34S\1\x35S\1\x36S\1\x37S\1\x38S\1\x39S\1\x3aS\1\x3bS\1\x3cS\1\x3dS\1\x3eS\1\x3f"
"S\1\x40S\1\x41S\1\x42S\1\x43S\1\x44S\1\x45S\1\x46S\1\x47S\1\x48S\1\x49S\1\x4aS\1\x4bS\1\x4cS\1\x4dS\1\x4eS\1\x4f"
"S\1\x50S\1\x51S\1\x52S\1\x53S\1\x54S\1\x55S\1\x56S\1\x57S\1\x58S\1\x59S\1\x5aS\1\x5bS\1\x5cS\1\x5dS\1\x5eS\1\x5f"
"S\1\x60S\1\x61S\1\x62S\1\x63S\1\x64S\1\x65S\1\x66S\1\x67S\1\x68S\1\x69S\1\x6aS\1\x6bS\1\x6cS\1\x6dS\1\x6eS\1\x6f"
"S\1\x70S\1\x71S\1\x72S\1\x73S\1\x74S\1\x75S\1\x76S\1\x77S\1\x78S\1\x79S\1\x7aS\1\x7bS\1\x7cS\1\x7dS\1\x7eS\1\x7f"
"S\1\x80S\1\x81S\1\x82S\1\x83S\1\x84S\1\x85S\1\x86S\1\x87S\1\x88S\1\x89S\1\x8aS\1\x8bS\1\x8cS\1\x8dS\1\x8eS\1\x8f"
"S\1\x90S\1\x91S\1\x92S\1\x93S\1\x94S\1\x95S\1\x96S\1\x97S\1\x98S\1\x99S\1\x9aS\1\x9bS\1\x9cS\1\x9dS\1\x9eS\1\x9f"
"S\1\xa0S\1\xa1S\1\xa2S\1\xa3S\1\xa4S\1\xa5S\1\xa6S\1\xa7S\1\xa8S\1\xa9S\1\xaaS\1\xabS\1\xacS\1\xadS\1\xaeS\1\xaf"
"S\1\xb0S\1\xb1S\1\xb2S\1\xb3S\1\xb4S\1\xb5S\1\xb6S\1\xb7S\1\xb8S\1\xb9S\1\xbaS\1\xbbS\1\xbcS\1\xbdS\1\xbeS\1\xbf"
"S\1\xc0S\1\xc1S\1\xc2S\1\xc3S\1\xc4S\1\xc5S\1\xc6S\1\xc7S\1\xc8S\1\xc9S\1\xcaS\1\xcbS\1\xccS\1\xcdS\1\xceS\1\xcf"
"S\1\xd0S\1\xd1S\1\xd2S\1\xd3S\1\xd4S\1\xd5S\1\xd6S\1\xd7S\1\xd8S\1\xd9S\1\xdaS\1\xdbS\1\xdcS\1\xddS\1\xdeS\1\xdf"
"S\1\xe0S\1\xe1S\1\xe2S\1\xe3S\1\xe4S\1\xe5S\1\xe6S\1\xe7S\1\xe8S\1\xe9S\1\xeaS\1\xebS\1\xecS\1\xedS\1\xeeS\1\xef"
"S\1\xf0S\1\xf1S\1\xf2S\1\xf3S\1\xf4S\1\xf5S\1\xf6S\1\xf7S\1\xf8S\1\xf9S\1\xfaS\1\xfbS\1\xfcS\1\xfdS\1\xfeS\1\xff";
char* empty_str_src = "S\0";

char* single_char_strs;
char* empty_str;

char* hex_digits = "0123456789ABCDEF";

bool is_lower(const char c){
	return 'a' <= c && c <= 'z';
}

bool is_upper(const char c){
	return 'A' <= c && c <= 'Z';
}

bool is_number(const char c){
	return '0' <= c && c <= '9';
}

bool is_alpha(const char c){
	return is_upper(c) || is_lower(c);
}

bool is_alphanum(const char c){
	return is_alpha(c) || is_number(c);
}

bool is_name_start(const char c){
	return is_alpha(c) || c == '_';
}

bool is_name(const char c){
	return is_alphanum(c) || c == '_';
}

bool is_varname(const char c){
	return is_name(c) || c == '$';
}

// Used as index into var table directly.
u32 name_hash(const char* name, const u32 len, const u32 hmax){
	assert((hmax & (hmax - 1)) == 0); // check hmax is power of 2
	u32 hash = name[0]; // prevents single-char conflicts
	for (u32 i=1; i<len; ++i){
		hash += name[i]*i*179;
	}
	return hash & (hmax - 1); // % hmax
}

bool namecmp(const char* a, const u32 len, const char b[16]){
	assert(len <= 16);
	for (u32 i = 0; i < len; ++i){
		if (a[i] != b[i]){
			return false;
		}
	}
	if (len == 16)
		return true;
	else {
		return b[len] == '\0';
	}
}

/// Allocate memory for strs
void init_mem_str(struct strings* s, const uint_fast16_t str_count, const enum string_type str_type){
#ifdef PC
	// dynamic alloc places these special strings into the correct address space
	single_char_strs = malloc_log("special_strings", 256*3+2);
	empty_str = single_char_strs + 768;
	memcpy(single_char_strs, single_char_strs_src, 768);
	memcpy(empty_str, empty_str_src, 2);
#else
	single_char_strs = single_char_strs_src;
	empty_str = empty_str_src;
#endif
	
	s->strs_max = str_count;
	s->strs = calloc_log("init_mem_str", str_count, sizeof(struct string));
	s->type = str_type;
	
	if (str_type == STRING_CHAR){
		s->str_data = malloc_log("init_mem_str data", str_count * sizeof(u8) * MAX_STRLEN);
	} else if (str_type == STRING_WIDE) {
		s->str_data = calloc_log("init_mem_str data", str_count, sizeof(u16) * MAX_STRLEN);
	} else {
		iprintf("Invalid string type!\n");
		abort();
	}
}

void reset_str(struct strings* s){
	for (uint_fast16_t i = 0; i < s->strs_max; ++i){
		s->strs[i].uses = 0; // free string by marking it unused
	}
}

/// Free memory for strs
void free_mem_str(struct strings* s){
	free_log("free_mem_str", s->strs);
	free_log("free_mem_str data", s->str_data);
}

/// Finds a usable string slot.
/// Must exist at least once string slot to function correctly.
struct string* get_new_str(struct strings* s){
	struct string* strs = s->strs;
	size_t i = 0;
	while (strs[i].uses > 0){
		++i;
		if (i >= s->strs_max){
			return NULL;
		}
	}
	strs[i].type = s->type;
	strs[i].uses = 0;
	if (s->type == STRING_CHAR){
		strs[i].ptr.s = &((u8*)s->str_data)[i*MAX_STRLEN];
	} else if (s->type == STRING_WIDE){
		strs[i].ptr.w = &((u16*)s->str_data)[i*MAX_STRLEN];
	} else {
		iprintf("String type not set/Invalid type set!");
		abort();
	}
	// length is currently unspecified; assign in code using this result
	
	return &strs[i];
}

int digit_value(const u16 c){
	if (c >= to_wide('0') && c <= to_wide('9')){
		return c - to_wide('0');
	} else if (c >= to_wide('A') && c <= to_wide('F')){
		return c - to_wide('A') + 10;
	}
	return -1;
}

int64_t u8_to_number(u8* data, const int len, const int base, const bool allow_decimal){
	const struct string str = {
		.type = STRING_CHAR, .len = len, .ptr.s = data
	};
	return str_to_number(&str, base, allow_decimal);
}

// More general number parsing
int64_t str_to_number(const void* str, const int base, const bool allow_decimal){
	size_t len = str_len(str);
	
	unsigned int number = 0;
	unsigned int fraction = 0;
	unsigned int maximum = 1;
	
	bool is_negative = false;
	
	for (size_t i = 0; i < len; ++i){
		u16 c = str_at_wide(str, i);
		int digit = digit_value(c);
		if (digit >= 0 && digit < base){
			number *= base;
			number += digit_value(c);
		} else if (c == to_wide('.') && allow_decimal){
			++i;
			for (size_t k = i; k < len; ++k){
				digit = digit_value(str_at_wide(str, k));
				if (digit >= 0 && digit < base){
					fraction *= base;
					maximum *= base;
					fraction += digit;
				} else {
					// Stop reading on invalid character (useful for VAL)
					// Note: second period is not allowed
					break;
				}
			}
			break;
		} else if (!i && c == to_wide('-')){
			is_negative = true;
		} else {
			// Stop reading on invalid character (useful for VAL)
			break;
		}
		if (number >= 524288){
			break;
		}
	}
	
	int64_t combined;
	if (is_negative){
		combined = INT_TO_FP(-number);
		combined -= INT_TO_FP(1) * fraction / maximum;
	} else {
		combined = ((int64_t)number * 4096) + 4096 * fraction / maximum;
	}
	
//	printf("Number := %f Combined := %d\n", (double)number, combined);
	return combined;
}

int64_t u8_to_num(u8* data, const idx len){
	return u8_to_number(data, len, 10, true);
}

// Returns STR_TYPE_8 if string is u8 type
// Returns STR_TYPE_16 if string is u16 type
int str_type(const void* str){
	assert(str);
	
	switch (*(char*)str){
		case STRING_CHAR:
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
		case STRING_INLINE_CHAR:
			return STR_TYPE_8;
		case BC_WIDE_STRING:
		case STRING_WIDE:
		case STRING_INLINE_WIDE:
			return STR_TYPE_16;
		default:
			iprintf("Unimplemented/Not a valid string type! (String type)\n");
			abort();
	}
}

void fixp_to_str(const fixp num, void* dest){
	if (str_type(dest) == STR_TYPE_16){
		// this is a bit stupid, but we can just...
		// change the string type to the smaller one.
		// there's guaranteed to be enough space, after all.
		((struct string*)dest)->type = STRING_CHAR;
	}
	fixp_to_char(num, (u8*)str_at(dest, 0));
	str_set_len(dest, strlen((char*)str_at(dest, 0)));
}

// Convert 20.12 fixed point number to string
// Using PTC rounding rules
// Note: This function null-terminates the string instead of storing the length.
void fixp_to_char(const fixp n, u8* str){
//	u16* begin = str;
	if (n < 0){
		(*str++) = '-';
	}
	fixp num = n < 0 ? -n : n;
	
	u32 integer = num >> FIXPOINT;
	const u32 decimal_mask = (INT_TO_FP(1) - 1); //default: 0x00000fff
	u32 decimal = ((((num & decimal_mask) + 2) * 1000) >> FIXPOINT);
	u32 divisor = 100000;
	bool trailing = false;
	// Loop until divisor is zero (to get all digits)
	if (integer){
		while (divisor > 0){
			// integer > divisor:
			if (divisor > integer){
				divisor /= 10;
				if (trailing)
					*str++ = '0';
				continue;
			}
			
			*str = '0';
			while (integer >= divisor){
				integer -= divisor;
				*str += 1;
			}
			divisor /= 10;
			str++;
			trailing = true;
		}
	} else {
		// zero is special case
		*str++ = '0';
	}
	// Only add decimal if it exists
	if (decimal){
		*str++ = '.';
		// loop until number is gone (does not keep trailing zeros)
		divisor = 100;
		while (decimal > 0){
			// integer > divisor:
			*str = '0';
			if (divisor > decimal){
				divisor /= 10;
				str++;
				continue;
			}
			
			while (decimal >= divisor){
				decimal -= divisor;
				*str += 1;
			}
			divisor /= 10;
			str++;
		}
	}
	// Null-terminate
	*str = '\0';
}

/// Converts PTC's extended ASCII to UTF16/UCS2 
u16 to_wide(u8 c){
	if (c < 0x21){
		return c;
	}
	if (c < 0x80){
		if (c == 0x22) return 0x201d;
		if (c == 0x27) return 0x2019;
		if (c == 0x5c) return 0xffe5;
		return (c - 0x20) | 0xff00;
	}
	if (c < 0xa1){
		return c;
	}
	if (c < 0xe0){
		if (c == 0xb0) return 0xff70;
		const u8 katakana[] = "\x02\x0c\x0d\x01\xfb\xf2\xa1\xa3\xa5\xa7\xa9\xe3\xe5\xe7"
		"\xc3?\xa2\xa4\xa6\xa8\xaa\xab\xad\xaf\xb1\xb3\xb5\xb7\xb9\xbb\xbd\xbf"
		"\xc1\xc4\xc6\xc8\xca\xcb\xcc\xcd\xce\xcf\xd2\xd5\xd8\xdb\xde\xdf\xe0"
		"\xe1\xe2\xe4\xe6\xe8\xe9\xea\xeb\xec\xed\xef\xf3\x9b\x9c";
		return katakana[c - 0xa1] | 0x3000;
	}
	// c >= 0xe0
	return c;
}

// "".join([chr(x) if x>=0 else '?' for x in [ktk.find(chr(i)) for i in range(0,256)]])
const u8 katakana[] = "?\x03\x00?????????\x01\x02?????????????????????????????????????????????????"
"\x0f???????????????????????????????????????????????????????????????????????????????????????????" /*trigraph oops*/ "=>"
"????\x06\x10\x07\x11\x08\x12\t\x13\n\x14\x15?\x16?\x17?\x18?\x19?\x1a?\x1b?\x1c?\x1d?\x1e?\x1f? ?"
"\x0e!?\"?#?$%&\'()??*??+??,?" /*split trigraph, again*/ "?-??./012\x0b""3\x0c""4\r56789:?;??\x05<???????\x04????";

//TODO:TEST:LOW Check that this validates all characters correctly
int to_char(u16 w){
	u8 wh = (w & 0xff00) >> 8;
	u8 wl = w & 0x00ff;
	if (wh == 0){
		return wl;
	} else if (wh == 0x20){
		if (wl == 0x1d) return 0x22;
		if (wl == 0x19) return 0x27;
		// invalid returns -1
	} else if (wh == 0xff){
		if (wl == 0x70) return 0xb0;
		if (wl == 0xe5) return 0x5c;
		return wl + 0x20; // TODO:IMPL:LOW returns some false values?
	} else if (wh == 0x30){
		return katakana[wl] + 0xa1;
	}
	iprintf("Error converting u16 to u8 char: 0x%x\n", w);
	return -1;
}

inline bool is_char(u16 w){
	return to_char(w) != -1;
}

/// Copy n characters from source buffer to destination buffer.
/// Copy behavior is determined by the type information in types.
/// 
/// @note src and dest should be either u8* or u16*
void str_copy_buf(const void* src, void* dest, const u8 types, const u16 count){
	assert(src);
	assert(dest);
	// TODO:PERF:LOW profile this and see if swapping loop/condition order is better?
	for (size_t i = 0; i < count; ++i){
		if (types == 0){
			// u8 -> u8
			((u8*)dest)[i] = ((u8*)src)[i];
		} else if (types == STR_COPY_SRC_16){
			// u16 -> u8
			((u8*)dest)[i] = to_char(((u16*)src)[i]);
		} else if (types == STR_COPY_DEST_16){
			// u8 -> u16
			((u16*)dest)[i] = to_wide(((u8*)src)[i]);
		} else if (types == (STR_COPY_DEST_16 | STR_COPY_SRC_16)){
			// u16 -> u16
			((u16*)dest)[i] = ((u16*)src)[i];
		}
	}
}

/// Copies a string from src (any valid string data, determined by first byte) to dest
/// @note strings are NOT null-terminated. Length should be stored if needed.
void str_wide_copy(const void* src, u16* dest){
	assert(src);
	assert(dest);
	
	struct string* s = (struct string*)src;
	if (!src) return; // nothing to copy
	
	switch (*(char*)src){
		case STRING_CHAR:
			str_copy_buf(s->ptr.s, dest, STR_COPY_SRC_8 | STR_COPY_DEST_16, s->len);
			return;
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
		case STRING_INLINE_CHAR:
			str_copy_buf(&((u8*)src)[2], dest, STR_COPY_DEST_16, ((u8*)src)[1]);
			return;
		case STRING_WIDE:
			str_copy_buf(s->ptr.w, dest, STR_COPY_DEST_16 | STR_COPY_SRC_16, s->len);
			return;
		case STRING_INLINE_WIDE:
		default:
			iprintf("Unimplemented/Not a valid string type! (Copy u16)\n");
			abort();
	}
}

/// Copies a string from src (any valid string data, determined by first byte) to dest
/// @note strings are NOT null-terminated. Length should be stored if needed.
void str_char_copy(const void* src, u8* dest){
	assert(src);
	assert(dest);
	
	struct string* s = (struct string*)src;
	if (!src) return; // nothing to copy
	
	switch (*(char*)src){
		case STRING_CHAR:
			str_copy_buf(s->ptr.s, dest, STR_COPY_DEST_8, s->len);
			return;
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
			str_copy_buf(&((u8*)src)[2], dest, STR_COPY_DEST_8, ((u8*)src)[1]);
			return;
		case STRING_WIDE:
			str_copy_buf(s->ptr.w, dest, STR_COPY_SRC_16 | STR_COPY_DEST_8, s->len);
			return;
		default:
			iprintf("Unimplemented/Not a valid string type! (Copy u8)\n");
			abort();
	}
}

// Gets a pointer to a character in the string at the given index.
void* str_at(const void* src, const u16 index){
	assert(src);
	
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_CHAR:
			return s->ptr.s + index;
		case STRING_WIDE:
			return s->ptr.w + index;
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
		case STRING_INLINE_CHAR:
			return ((u8*)src) + 2 + index;
		case BC_WIDE_STRING:
		case STRING_INLINE_WIDE:
			return ((u16*)src) + 1 + index;
		default:
			iprintf("Unimplemented/Not a valid string type! (str_at)\n");
			abort();
	}
}

// Gets the wide character value in the string at the given index.
u16 str_at_wide(const void* src, const u16 index){
	assert(src);
	
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_CHAR:
			return to_wide(s->ptr.s[index]);
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
		case STRING_INLINE_CHAR:
			return to_wide(((u8*)src)[2+index]);
		case STRING_INLINE_WIDE:
			return ((u16*)src)[1+index];
		case STRING_WIDE:
			return s->ptr.w[index];
		default:
			iprintf("Unimplemented/Not a valid string type! (str_at_wide)\n");
			abort();
	}
}

// Gets the small character value in the string at the given index.
u16 str_at_char(const void* src, const u16 index){
	assert(src);
	
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_CHAR:
			return s->ptr.s[index];
		case STRING_WIDE:
			return s->ptr.w[index];
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
		case STRING_INLINE_CHAR:
			return ((u8*)src)[2+index];
		case STRING_INLINE_WIDE:
			return ((u16*)src)[1+index];
		default:
			iprintf("Unimplemented/Not a valid string type! (str_at_char)\n");
			abort();
	}
}

u32 str_len(const void* src){
	assert(src);
	
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_CHAR:
		case STRING_WIDE:
			return s->len;
		case BC_STRING:
		case BC_LABEL:
		case BC_LABEL_STRING:
		case BC_WIDE_STRING:
		case STRING_INLINE_CHAR:
		case STRING_INLINE_WIDE:
			return ((u8*)src)[1];
		default:
			iprintf("Unimplemented/Not a valid string type [type=%c]! (Length)\n", *(char*)src);
			abort();
	}
}

void str_set_len(void* src, int len){
	assert(src);
	assert(len >= 0 && len < 256);
	
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_CHAR:
			s->len = len;
			break;
		case STRING_INLINE_CHAR:
		case STRING_INLINE_WIDE:
			((u8*)src)[1] = len;
			break;
		case BC_LABEL:
		case BC_STRING:
		case BC_LABEL_STRING:
		case BC_WIDE_STRING:
		default:
			iprintf("Attempted to write to a read-only string type! (Set length)\n");
			abort();
	}
}

bool str_comp(const void* str1, const void* str2){
	assert(str1);
	assert(str2);
	
	if (str1 == str2) return true;
	
//	iprintf("lens %d==%d\n", str_len(str1), str_len(str2));
	u32 len = str_len(str1);
	if (len != str_len(str2)){
		return false;
	}

	for (u32 i = 0; i < len; ++i){
//		iprintf("%d==%d\n", cmp1[i], cmp2[i]);
		if (str_at_wide(str1, i) != str_at_wide(str2, i)){
			return false;
		}
	}
	return true;
}

int str_lex(const void* a, const void* b){
	int size_a = str_len(a);
	int size_b = str_len(b);
	int size = size_a < size_b ? size_a : size_b;
	for (int i = 0; i < size; ++i){
		u16 chr_a = str_at_wide(a, i);
		u16 chr_b = str_at_wide(b, i);
		if (chr_a < chr_b){
			return -1;
		}
		if (chr_b < chr_a){
			return 1;
		}
	}
	if (size_a < size_b){
		return -1;
	}
	if (size_b < size_a){
		return 1;
	}
	return 0;
}

// Copy str1 to str2
void str_copy(const void* src, void* src_dest){
	assert(src);
	assert(src_dest);
	
	switch (*(char*)src_dest){
		case STRING_CHAR:;
			u8* dest = ((struct string*)src_dest)->ptr.s;
			str_char_copy(src, dest);
			((struct string*)src_dest)->len = str_len(src);
			break;
		case STRING_WIDE:{
			u16* dest = ((struct string*)src_dest)->ptr.w;
			str_wide_copy(src, dest);
			((struct string*)src_dest)->len = str_len(src);
			break;
			}
		case BC_STRING:
		case BC_LABEL:
		case BC_LABEL_STRING:
		case BC_WIDE_STRING:
			iprintf("Error: Attempted to copy to read-only string!\n");
			abort();
		default:
			iprintf("Unimplemented/Not a valid string type! (Copy)\n");
			abort();
	}
}

/// Returns true if copy occurs
bool str_concat(const void* src1, const void* src2, void* dest){
	assert(src1);
	assert(src2);
	assert(dest);
	
	if (str_len(src1) + str_len(src2) > MAX_STRLEN){
		// Copy fails
		return false;
	}
	
	str_copy(src1, dest);
	struct string* d = (struct string*)dest;
	if (d->type == STRING_CHAR){
		// copy to after
		str_char_copy(src2, &(d->ptr.s[str_len(src1)]));
		d->len = str_len(src1) + str_len(src2);
		return true;
	} else if (d->type == STRING_WIDE){
		str_wide_copy(src2, &(d->ptr.w[str_len(src1)]));
		d->len = str_len(src1) + str_len(src2);
		return true;
	} else {
		iprintf("Error: Invalid destination type\n");
		abort();
	}
}

