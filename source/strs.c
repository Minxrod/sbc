#include "strs.h"
#include "ptc.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/// Allocate memory for strs
void init_mem_str(struct strings* s, int str_count, enum string_type str_type){
	s->strs_max = str_count;
	s->strs = calloc(str_count, sizeof(struct string));
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
	
	return &strs[i];
}

u16 to_wide(u8 c){
	if (c < 0x21){
		return c;
	} else if (c >= 0x21 && c < 0x28){
		//TODO: a few keyboard symbols
		abort();
	} else if (c >= 28 && c < 0x80){
		return c | 0xff00;
	} else if (c >= 80 && c < 0xa1){
		return c;
	} else if (c >= 0xa1 && c < 0xe0){
		//TODO: katakana
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
				dest[i] = to_wide(*(s->ptr.s)[i]);
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

