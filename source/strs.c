#include "strs.h"
#include "ptc.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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
		// TODO:CODE Better error handling?
		iprintf("Invalid string type\n");
		abort();
	}
}

/// Free memory for strs
void free_mem_str(struct strings* s){
	free(s->strs);
	free(s->str_data);
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

s32 str_to_num(u8* data, u32 len){
	s32 number = 0;
	s32 fraction = 0;
	s32 maximum = 1;
	
	for (size_t i = 0; i < len; ++i){
		char c = data[i];
		if (is_number(c)){
			number *= 10;
			number += c - '0';
		} else if (c == '.'){
			++i;
			for (size_t k = i; k < len; ++k){
				c = data[k];
				fraction *= 10;
				maximum *= 10;
				fraction += c - '0';
			}
			break;
		}
	}
	//TODO:ERR Check overflow
	
	number = number * 4096 + 4096 * fraction / maximum;
	
//	iprintf("Number := %f", (double)number);
	return number;
}


// Convert 20.12 fixed point number to string
// Using PTC rounding rules
// TODO:CODE rename
void str_num(s32 num, u8* str){
//	u16* begin = str;
	if (num < 0){
		(*str++) = '-';
		num = -num;
	}
	u32 integer = num >> 12;
	u32 decimal = ((((num & 0x00000fff) + 2) * 1000) >> 12);
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
	// TODO:IMPL String length?
	// (str - begin)/ sizeof u8?;
}

/// Converts PTC's extended ASCII to UTF16/UCS2 
u16 to_wide(u8 c){
	if (c < 0x21){
		return c;
	} else if (c >= 0x21 && c < 0x28){
		if (c == 0x22) return 0x201d;
		if (c == 0x27) return 0x2019;
		return (c - 0x20) | 0xff00;
	} else if (c >= 0x28 && c < 0x80){
		return (c - 0x20) | 0xff00;
	} else if (c >= 0x80 && c < 0xa1){
		return c;
	} else if (c >= 0xa1 && c < 0xe0){
		if (c == 0xb0) return 0xff70;
		const u8 katakana[] = "\x02\x0c\x0d\x01\xfb\xf2\xa1\xa3\xa5\xa7\xa9\xe3\xe5\xe7"
		"\xc3?\xa2\xa4\xa6\xa8\xaa\xab\xad\xaf\xb1\xb3\xb5\xb7\xb9\xbb\xbd\xbf"
		"\xc1\xc4\xc6\xc8\xca\xcb\xcc\xcd\xce\xcf\xd2\xd5\xd8\xdb\xde\xdf\xe0"
		"\xe1\xe2\xe4\xe6\xe8\xe9\xea\xeb\xec\xed\xef\xf3\x9b\x9c";
		return katakana[c - 0xa1] | 0x3000;
	} else if (c >= 0xe0){
		return c;
	} else {
		iprintf("Error converting to wide char (unimplemented)\n");
		abort();
	}
}

u8 to_char(u16 w){
	u8 wh = (w & 0xff00) >> 8;
	u8 wl = w & 0x00ff;
	if (wh == 0){
		return wl;
	} else if (wh == 0x20){
		if (wl == 0x1d) return 0x22;
		if (wl == 0x19) return 0x27;
		abort();
	} else if (wh == 0xff){
		if (wl == 0x70) return 0xb0;
		return wl + 0x20;
	} else if (wh == 0x30){
		// "".join([chr(x) if x>=0 else '?' for x in [ktk.find(chr(i)) for i in range(0,256)]])
		const u8 katakana[] = "?\x03\x00?????????\x01\x02?????????????????????????????????????????????????"
		"\x0f???????????????????????????????????????????????????????????????????????????????????????????" /*trigraph oops*/ "=>"
		"????\x06\x10\x07\x11\x08\x12\t\x13\n\x14\x15?\x16?\x17?\x18?\x19?\x1a?\x1b?\x1c?\x1d?\x1e?\x1f? ?"
		"\x0e!?\"?#?$%&\'()??*??+??,?" /*split trigraph, again*/ "?-??./012\x0b""3\x0c""4\r56789:?;??\x05<???????\x04????";
		return katakana[wl] + 0xa1;
	} else {
		//TODO:IMPL Handle invalid characters better?
		iprintf("Error converting u16 to u8 char (unimplemented): %x\n", w);
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
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
			for (size_t i = 0; i < ((u8*)src)[1]; ++i){
				dest[i] = to_wide(((u8*)src)[2+i]);
			}
			return;
		default:
			iprintf("Unimplemented/Not a valid string type! (Copy u16)\n");
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
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
			for (size_t i = 0; i < ((u8*)src)[1]; ++i){
				dest[i] = ((u8*)src)[2+i];
			}
			return;
		default:
			iprintf("Unimplemented/Not a valid string type! (Copy u8)\n");
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
		case BC_LABEL:
		case BC_LABEL_STRING:
		case BC_WIDE_STRING:
			return ((u8*)src)[1];
		default:
			iprintf("Unimplemented/Not a valid string type! (Length)\n");
			abort();
	}
}

bool str_comp(void* str1, void* str2){
	u16 cmp1[256];
	u16 cmp2[256];
	
	if (str1 == str2) return true;
	if (str1 == NULL) return false;
	if (str2 == NULL) return false;
	
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

void str_concat(void* src1, void* src2, void* dest){
	if (str_len(src1) + str_len(src2) > MAX_STRLEN){
		// Copy fails
		//TODO:ERR error status
		return;
	}
	
	str_copy(src1, dest);
	struct string* d = (struct string*)dest;
	if (d->type == STRING_CHAR){
		// copy to after
		str_char_copy(src2, &(d->ptr.s[str_len(src1)]));
		d->len = str_len(src1) + str_len(src2);
	} else if (d->type == STRING_WIDE){
		iprintf("Unimplemented: Wide string destination\n");
		abort();
	} else {
		iprintf("Error: Invalid destination type\n");
		abort();
	}
}

