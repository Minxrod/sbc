#include "strs.h"
#include "ptc.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/// Allocate memory for strs
void init_mem_str(struct strings* s, int str_count, enum string_type str_type){
	s->strs_max = str_count;
	s->strs = calloc(str_count, sizeof(struct string));
	s->type = str_type;
	
	s->empty = (struct string){STRING_EMPTY, 0, 0, {NULL}};
	if (str_type == STRING_CHAR){
		s->str_data = calloc(str_count, sizeof(u8) * MAX_STRLEN);
	} else if (str_type == STRING_WIDE) {
		s->str_data = calloc(str_count, sizeof(u16) * MAX_STRLEN);
	} else {
		// ERROR TODO
		iprintf("Invalid string type\n");
		abort();
	}
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
	strs[i].uses = 1; // this is first usage
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

/// Converts PTC's extended ASCII to UTF16/UCS2 
u16 to_wide(u8 c){
	if (c < 0x21){
		return c;
	} else if (c >= 0x21 && c < 0x28){
		if (c == 0x22) return 0x201d;
		if (c == 0x27) return 0x2019;
		return (c - 0x20) | 0xff00;
	} else if (c >= 28 && c < 0x80){
		return c | 0xff00;
	} else if (c >= 80 && c < 0xa1){
		return c;
	} else if (c >= 0xa1 && c < 0xe0){
		//TODO: katakana
		iprintf("Error converting to wide char (Katakana)\n");
		abort();
	} else if (c >= 0xe0){
		return c;
	} else {
		iprintf("Error converting to wide char (unimplemented)\n");
		abort();
	}
}

/// Copies a string from src (any valid string data, determined by first byte) to dest
/// @note strings are NOT null-terminated. Length should be stored if needed.
void str_wide_copy(void* src, u16* dest){
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_EMPTY:
			return;
		case STRING_CHAR:
			for (size_t i = 0; i < s->len; ++i){
				// convert up to wide char
				dest[i] = to_wide((s->ptr.s)[i]);
			}
			return;
		case BC_STRING:
			for (size_t i = 0; i < ((u8*)src)[1]; ++i){
				dest[i] = to_wide(((u8*)src)[2+i]);
			}
			return;
		default:
			iprintf("Unimplemented/Not a valid string type!\n");
			abort();
	}
}

/// Copies a string from src (any valid string data, determined by first byte) to dest
/// @note strings are NOT null-terminated. Length should be stored if needed.
void str_char_copy(void* src, u8* dest){
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_EMPTY:
			return;
		case STRING_CHAR:
			for (size_t i = 0; i < s->len; ++i){
				// convert up to wide char
				dest[i] = (s->ptr.s)[i];
			}
			return;
		case BC_STRING:
			for (size_t i = 0; i < ((u8*)src)[1]; ++i){
				dest[i] = ((u8*)src)[2+i];
			}
			return;
		default:
			iprintf("Unimplemented/Not a valid string type!\n");
			abort();
	}
}

u32 str_len(void* src){
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_EMPTY:
			return 0;
		case STRING_CHAR:
			return s->len;
		case BC_STRING:
			return ((u8*)src)[1];
		default:
			iprintf("Unimplemented/Not a valid string type!\n");
			abort();
	}
}

bool str_comp(void* str1, void* str2){
	u16 cmp1[256];
	u16 cmp2[256];
	
	u32 len = str_len(str1);
	if (len != str_len(str2)){
		return false;
	}
	
	str_wide_copy(str1, cmp1);
	str_wide_copy(str2, cmp2);
	
	for (u32 i = 0; i < len; ++i){
		if (cmp1[i] != cmp2[i]){
			return false;
		}
	}
	return true;
}

// Copy str1 to str2
void str_copy(void* src, void* src_dest){
	switch (*(char*)src_dest){
		case STRING_EMPTY:
			iprintf("Error: Attempted to copy to empty string!\n");
			abort();
		case STRING_CHAR:;
			u8* dest = ((struct string*)src_dest)->ptr.s;
			str_char_copy(src, dest);
			((struct string*)src_dest)->len = str_len(src);
			break;
		case BC_STRING:
			iprintf("Error: Attempted to copy to read-only string!\n");
			abort();
		default:
			iprintf("Unimplemented/Not a valid string type!\n");
			abort();
	}
}


