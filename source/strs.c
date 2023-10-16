#include "strs.h"
#include "ptc.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//Note: Should be const, but need to pass void* into this.
char* single_char_strs = 
"S\1\x00S\1\x01S\1\x02S\1\x03S\1\x04S\1\x05S\1\x06S\1\x07S\1\x08S\1\x09S\1\x0aS\1\x0bS\1\x0cS\1\x0dS\1\x0eS\1\x0f"
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
void init_mem_str(struct strings* s, uint_fast16_t str_count, enum string_type str_type){
	s->strs_max = str_count;
	iprintf("calloc=%d\n", (int)str_count * (int)sizeof(struct string));
	s->strs = calloc(str_count, sizeof(struct string));
	iprintf("CRASH?");
	s->type = str_type;
	
	s->empty = (struct string){STRING_EMPTY, 0, 0, {NULL}};
	if (str_type == STRING_CHAR){
		iprintf("calloc=%d\n", (int)sizeof(u8) * MAX_STRLEN * (int) str_count);
		s->str_data = malloc(str_count * sizeof(u8) * MAX_STRLEN);
		iprintf("CRASH?");
	} else if (str_type == STRING_WIDE) {
		s->str_data = calloc(str_count, sizeof(u16) * MAX_STRLEN);
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

int digit_value(char c){
	if (c >= '0' && c <= '9'){
		return c - '0';
	} else if (c >= 'A' && c <= 'F'){
		return c - 'A' + 10;
	}
	return -1;
}

// More general number parsing
fixp str_to_number(u8* data, idx len, int base, bool allow_decimal){
	unsigned int number = 0;
	unsigned int fraction = 0;
	unsigned int maximum = 1;
	
	for (size_t i = 0; i < len; ++i){
		char c = data[i];
		int digit = digit_value(c);
		if (digit >= 0 && digit < base){
			number *= base;
			number += digit_value(c);
		} else if (c == '.' && allow_decimal){
			++i;
			for (size_t k = i; k < len; ++k){
				digit = digit_value(data[k]);
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
		} else {
			// Stop reading on invalid character (useful for VAL)
			break;
		}
	}
	//TODO:ERR:MED Check overflow
	
	fixp combined = number * 4096 + 4096 * fraction / maximum;
	
//	iprintf("Number := %f", (double)number);
	return combined;

}

//NOTE: Doesn't handle negatives
fixp str_to_num(u8* data, idx len){
	return str_to_number(data, len, 10, true);
}

// Convert 20.12 fixed point number to string
// Using PTC rounding rules
// TODO:CODE:LOW rename
void str_num(s32 num, u8* str){
//	u16* begin = str;
	if (num < 0){
		(*str++) = '-';
		num = -num;
	}
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
	// TODO:CODE:LOW String length?
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
		//TODO:CODE:LOW Handle invalid characters better?
		iprintf("Error converting u16 to u8 char (unimplemented): %x\n", w);
		abort();
	}
}

/// Copy n characters from source buffer to destination buffer.
/// Copy behavior is determined by the type information in types.
/// 
/// @note src and dest should be either u8* or u16*
void str_copy_buf(const void* src, void* dest, const u8 types, const u16 count){
	///TODO:PERF:LOW profile this and see if swapping loop/condition order is better?
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
			((u16*)dest)[i] = to_wide(((u16*)src)[i]);
		}
	}
}

/// Copies a string from src (any valid string data, determined by first byte) to dest
/// @note strings are NOT null-terminated. Length should be stored if needed.
void str_wide_copy(const void* src, u16* dest){
	struct string* s = (struct string*)src;
	if (!src) return; // nothing to copy
	
	switch (*(char*)src){
		case STRING_EMPTY:
			return;
		case STRING_CHAR:
			str_copy_buf(s->ptr.s, dest, STR_COPY_DEST_16, s->len);
			return;
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
			str_copy_buf(&((u8*)src)[2], dest, STR_COPY_DEST_16, ((u8*)src)[1]);
			return;
		default:
			iprintf("Unimplemented/Not a valid string type! (Copy u16)\n");
			abort();
	}
}

/// Copies a string from src (any valid string data, determined by first byte) to dest
/// @note strings are NOT null-terminated. Length should be stored if needed.
void str_char_copy(const void* src, u8* dest){
	struct string* s = (struct string*)src;
	if (!src) return; // nothing to copy
	
	switch (*(char*)src){
		case STRING_EMPTY:
			return;
		case STRING_CHAR:
			str_copy_buf(s->ptr.s, dest, STR_COPY_DEST_8, s->len);
			return;
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
			str_copy_buf(&((u8*)src)[2], dest, STR_COPY_DEST_8, ((u8*)src)[1]);
			return;
		default:
			iprintf("Unimplemented/Not a valid string type! (Copy u8)\n");
			abort();
	}
}

// Gets a pointer to a character in the string at the given index.
void* str_at(const void* src, const u16 index){
	struct string* s = (struct string*)src;
	
	switch (*(char*)src){
		case STRING_EMPTY:
			return NULL;
		case STRING_CHAR:
			return &s->ptr.s[index];
		case BC_LABEL_STRING:
		case BC_LABEL:
		case BC_STRING:
			return &((u8*)src)[2+index];
		default:
			iprintf("Unimplemented/Not a valid string type! (str_at)\n");
			abort();
	}
}

u32 str_len(const void* src){
	struct string* s = (struct string*)src;
	if (!src) return 0;
	
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

bool str_comp(const void* str1, const void* str2){
	u16 cmp1[256];
	u16 cmp2[256];
	
	if (str1 == str2) return true;
	
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
		//TODO:ERR:MED error status
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

