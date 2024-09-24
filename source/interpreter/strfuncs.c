#include "strfuncs.h"

#include "strs.h"
#include "system.h"
#include "error.h"
#include "stack.h"

#include <string.h>

void func_len(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	void* x = value_str(a);
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(str_len(x))}});
}

void func_mid(struct ptc* p){
	struct value_stack* s = &p->stack;
//	struct stack_entry* len_e = stack_pop(s);
//	struct stack_entry* start_e = stack_pop(s);
//	struct stack_entry* str_e = stack_pop(s);
	
	// Must be done first to ensure uses are kept correctly
	struct string* dest = get_new_str(&p->strs);
	
	int start;
	int len;
	
	void* str = STACK_REL_STR(-3);
	STACK_REL_INT_MIN(-2,0,start);
	STACK_REL_INT_MIN(-1,0,len);
	s->stack_i -= 3;
	
	s16 max_len = (s16)str_len(str) - (s16)start;
	if (max_len <= 0){
		stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = empty_str}});
		return;
	} else if (max_len <= (s16)len){
		len = max_len;
	}
	
	dest->uses = 1;
	dest->len = len;
	
	u8 type = str_type(str) | (str_type(dest) << 1);
	str_copy_buf(str_at(str, start), str_at(dest, 0), type, len);

	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
}

void func_val(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	u8 num_str[MAX_STRLEN];
	void* str = value_str(a);
	str_char_copy(str, num_str);
	int len = str_len(str);
//	bool negate = false;
	fixp v;

	u8 first_char = num_str[0];
	if (first_char == '&'){
		u8 second_char = num_str[1];
		int base = 0;
		if (second_char == 'H'){
			base = 16; // parse as hex
		} else if (second_char == 'B'){
			base = 2; // parse as binary
		} else {
			ERROR(ERR_SYNTAX);
		}
		v = u8_to_number(num_str + 2, len-2, base, false);
	} else {
		v = u8_to_num(num_str, len);
	}
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, {v}});
}

void func_str(struct ptc* p){
	fixp n = STACK_REL_NUM(-1);
	
	struct string* s = get_new_str(&p->strs);
	
	fixp_to_str(n, s);
	s->uses = 1;
	
	p->stack.stack_i -= 1;
	
	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = s}});
}

void func_instr(struct ptc* p){
	struct value_stack* s = &p->stack;
	int start = 0;
	if (p->exec.argcount == 3){
		struct stack_entry* startv = stack_pop(s);
		start = VALUE_INT(startv);
	}
	struct stack_entry* substring = stack_pop(s);
	struct stack_entry* string = stack_pop(s);
	void* str = value_str(string);
	void* sub = value_str(substring);
	
	int len_string = str_len(str);
	int len_substring = str_len(sub);
	// empty string always matches
//	iprintf("%d,%s,%s\n", start, (char*)str_at(str,0), (char*)str_at(sub,0));
	if (len_substring == 0) {
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(0)}});
		return;
	}
	
	int i;
	int j = 0;
	for (i = start; i < len_string; ++i){
		if (len_substring - j > len_string - i){
			// remaining string to search for is too long: can't find it
			break;
		}
		if (j == len_substring) break;
		
		if (str_at_wide(str,i) == str_at_wide(sub,j)){
			j++;
		} else {
			// match failed
			i -= j; // go back by number of matched chars
			j = 0; // reset check index
		}
//		iprintf("%d,%d\n",i,j);
	}
	// if string has been entirely matched, return match position
	if (j == len_substring){
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(i - len_substring)}});
	} else {
		stack_push(s, (struct stack_entry){VAR_NUMBER, {-INT_TO_FP(1)}});
	}
}

void func_chr(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	int c = VALUE_INT(a);
	if (c < 0 || c > 255){
		ERROR(ERR_OUT_OF_RANGE);
	}
	void* str = &single_char_strs[3*c];
	
	stack_push(s, (struct stack_entry){VAR_STRING, {.ptr = str}});
}

void func_subst(struct ptc* p){
	// Note: must be allocated here to ensure use count of str, repl are included
	struct string* dest = get_new_str(&p->strs);

	// argument validation
	int start;
	int count;
	void* str = STACK_REL_STR(-4);
	int len = str_len(str);
	void* repl = STACK_REL_STR(-1);
	STACK_REL_INT_RANGE(-3,0,len,start); // between 0 and length of string inclusive
	count = STACK_REL_INT(-2);
	p->stack.stack_i -= 4;
	if (count < 0 || count > len){
		count = len; // replaces all if less than max
	}
	int repl_len = str_len(repl);
	// actual copying
	dest->uses = 1;
	if (start + repl_len > MAX_STRLEN) ERROR(ERR_STRING_TOO_LONG);

	str_copy_buf(str_at(str, 0), dest->ptr.s, str_type(str) | (str_type(dest) << STR_COPY_DEST), start);
//	iprintf("%s\n",dest->ptr.s);
	str_copy_buf(str_at(repl, 0), str_at(dest, start), str_type(repl) | (str_type(dest) << STR_COPY_DEST), str_len(repl));
//	iprintf("%s\n",dest->ptr.s);
	int remaining = str_len(str)-start-count;
	if (remaining > 0)
		str_copy_buf(str_at(str, start+count), str_at(dest, start + str_len(repl)), str_type(str) | (str_type(dest) << STR_COPY_DEST), remaining);
//	iprintf("%s\n",dest->ptr.s);
	
	dest->len = start + str_len(repl) + (remaining > 0 ? remaining : 0);
	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
}

void func_asc(struct ptc* p){
	void* str = STACK_REL_STR(-1);
	p->stack.stack_i -= 1;
	
	if (str_len(str) == 0) ERROR(ERR_SYNTAX); // for some reason
	fixp chr = INT_TO_FP(str_at_char(str, 0));
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {chr}});
}

// left or right
void _func_substring(struct ptc* p, bool right){
	struct value_stack* s = &p->stack;

	// Must be done first to ensure uses are kept correctly
	struct string* dest = get_new_str(&p->strs);

	int count;

	void* str = STACK_REL_STR(-2);
	STACK_REL_INT_MIN(-1,0,count);
	s->stack_i -= 2;

	int max_len = str_len(str);
	if (!count){
		stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = empty_str}});
		return;
	}
	if (count > max_len) count = max_len;

	dest->uses = 1;
	dest->len = count;

	uint_fast8_t type = str_type(str) | (str_type(dest) << 1); //< source type
	if (right){
		str_copy_buf(str_at(str, max_len - count), str_at(dest, 0), type, dest->len);
	} else {
		str_copy_buf(str_at(str, 0), str_at(dest, 0), type, count);
	}
	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
}

void func_right(struct ptc* p){
	_func_substring(p, true);
}

void func_left(struct ptc* p){
	_func_substring(p, false);
}

void func_hex(struct ptc* p){
	// HEX$ num
	// HEX$ num digits
	
	// Must be done first to ensure uses are kept correctly
	struct string* dest = get_new_str(&p->strs);
	
	int num;
	int digits = 0;
	if (p->exec.argcount == 2){
		num = STACK_REL_INT(-2);
		STACK_REL_INT_RANGE(-1,1,5,digits);
		p->stack.stack_i -= 2;
	} else {
		num = STACK_REL_INT(-1);
		p->stack.stack_i -= 1;
	}
	if (digits && (num >= (1 << (4*digits)) || num < -(1 << (4*digits-1)))){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	num &= 0x0fffff;
	if (digits){
		num &= 0x0fffff >> 4*(5-digits);
	}
	int len = 0;
	// once again we can just force the type to work here. there will always be enough space.
	dest->type = STRING_CHAR;
	sprintf((char*)dest->ptr.s, "%0*X%n", digits, num, &len);
	iprintf("[%d]%0X=%.*s", len, num, len, dest->ptr.s);
	
	dest->len = len;
	dest->uses = 1;
	
	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
}
#define DATE_STR_LEN 10

void cmd_dtread(struct ptc* p){
	// Note: Invalid dates are OK! 9999/99/99 works and gives 9999, 99, 99.
	void* date_str = value_str(ARG(0));
	fixp* year = ARG(1)->value.ptr;
	fixp* month = ARG(2)->value.ptr;
	fixp* day = ARG(3)->value.ptr;
	
	// validate date format
	if (str_len(date_str) != DATE_STR_LEN) ERROR(ERR_SYNTAX);
	const char format[] = "NNNN/NN/NN";
	for (int i = 0; i < DATE_STR_LEN; ++i){
		u8 c = str_at_char(date_str, i);
		if (format[i] == 'N' && !is_number(c)) ERROR(ERR_SYNTAX);
		if (format[i] == '/' && c != '/') ERROR(ERR_SYNTAX);
	}

	*year = (*((u8*)str_at(date_str,0)) - '0') * 1000;
	*year += (*((u8*)str_at(date_str,1)) - '0') * 100;
	*year += (*((u8*)str_at(date_str,2)) - '0') * 10;
	*year += (*((u8*)str_at(date_str,3)) - '0');
	*year <<= FIXPOINT;
	
	*month = (*((u8*)str_at(date_str,5)) - '0') * 10;
	*month += (*((u8*)str_at(date_str,6)) - '0') * 1;
	*month <<= FIXPOINT;
	
	*day = (*((u8*)str_at(date_str,8)) - '0') * 10;
	*day += (*((u8*)str_at(date_str,9)) - '0') * 1;
	*day <<= FIXPOINT;
}

#define TIME_STR_LEN 8

void cmd_tmread(struct ptc* p){
	void* time_str = value_str(ARG(0));
	fixp* hour = ARG(1)->value.ptr;
	fixp* min = ARG(2)->value.ptr;
	fixp* sec = ARG(3)->value.ptr;
	
	// validate time format
	if (str_len(time_str) != TIME_STR_LEN) ERROR(ERR_SYNTAX);
	const char format[] = "NN:NN:NN";
	for (int i = 0; i < TIME_STR_LEN; ++i){
		u8 c = str_at_char(time_str, i);
		if (format[i] == 'N' && !is_number(c)) ERROR(ERR_SYNTAX);
		if (format[i] == ':' && c != ':') ERROR(ERR_SYNTAX);
	}


	*hour = 10 * (str_at_char(time_str,0) - '0');
	*hour += 1 * (str_at_char(time_str,1) - '0');
	*hour <<= FIXPOINT;
	
	*min = 10 * (str_at_char(time_str,3) - '0');
	*min += 1 * (str_at_char(time_str,4) - '0');
	*min <<= FIXPOINT;
	
	*sec = 10 * (str_at_char(time_str,6) - '0');
	*sec += 1 * (str_at_char(time_str,7) - '0');
	*sec <<= FIXPOINT;
}
