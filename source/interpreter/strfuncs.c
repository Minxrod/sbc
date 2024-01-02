#include "strfuncs.h"

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
	
	u16 start, len;
	
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
	// TODO:IMPL:LOW Determine type for destination string
	u8 type = str_type(str);
	str_copy_buf(str_at(str, start), str_at(dest, 0), type, len);
	
	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
}

void func_val(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	void* str = value_str(a);
	void* str_begin = str_at(str, 0);
	int len = str_len(str);
	bool negate = false;
	fixp v;
	
	u8 first_char = *(u8*)str_begin;
	if (first_char == '-'){
		negate = true;
		str_begin = str_at(str, 1);
		len--;
		v = u8_to_num(str_begin, str_len(str));
	} else if (first_char == '&'){
		u8 second_char = *(u8*)str_at(str, 1);
		str_begin = str_at(str, 2);
		if (second_char == 'H'){
			// parse as hex
			v = u8_to_number(str_begin, len-2, 16, false);
		} else if (second_char == 'B'){
			// parse as binary
			v = u8_to_number(str_begin, len-2, 2, false);
		} else {
			ERROR(ERR_SYNTAX);
		}
	} else {
		v = u8_to_num(str_begin, str_len(str));
	}
	// TODO:IMPL:LOW This doesn't work for u16 strings
	if (negate) v = -v;
	
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
	iprintf("%d,%s,%s\n", start, (char*)str_at(str,0), (char*)str_at(sub,0));
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
		iprintf("%d,%d\n",i,j);
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
	struct value_stack* s = &p->stack;
	struct stack_entry* replace_v = stack_pop(s);
	struct stack_entry* count_v = stack_pop(s);
	struct stack_entry* start_v = stack_pop(s);
	struct stack_entry* string_v = stack_pop(s);
	
	// Note: must be allocated here to ensure use values of str, repl are noted
	struct string* dest = get_new_str(&p->strs);
	
	int start = VALUE_INT(start_v);
	int count = VALUE_INT(count_v);
	void* str = value_str(string_v);
	void* repl = value_str(replace_v);
	
	dest->uses = 1;
//	dest->len = 0;
	
	//TODO:ERR:HIGH Proper bounds checks!
	
	str_copy_buf(str_at(str, 0), dest->ptr.s, STR_COPY_SRC_8 | STR_COPY_DEST_8, start);
//	iprintf("%s\n",dest->ptr.s);
	str_copy_buf(str_at(repl, 0), &dest->ptr.s[start], STR_COPY_SRC_8 | STR_COPY_DEST_8, str_len(repl));
//	iprintf("%s\n",dest->ptr.s);
	int remaining = str_len(str)-start-count;
	if (remaining > 0)
		str_copy_buf(str_at(str, start+count), &dest->ptr.s[start+str_len(repl)], STR_COPY_SRC_8 | STR_COPY_DEST_8, remaining);
//	iprintf("%s\n",dest->ptr.s);
	
	dest->len = start + str_len(repl) + (remaining > 0 ? remaining : 0);
	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
}

void func_asc(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* str_v = stack_pop(s);
	
	//TODO:TEST:LOW Check behavior on various strings
	fixp chr = INT_TO_FP(to_char(str_at_wide(VALUE_STR(str_v), 0)));
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {chr}});
}

void func_right(struct ptc* p){
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
	} else if (count > max_len){
		count = max_len;
	}
	
	dest->uses = 1;
	dest->len = count;
	// TODO:IMPL:LOW Determine type for destination string
	uint_fast8_t type = str_type(str); //< source type
	str_copy_buf(str_at(str, max_len - count), str_at(dest, 0), type, dest->len);
	
	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
}

// TODO:CODE:NONE Consider combining right and left?
void func_left(struct ptc* p){
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
	} else if (count > max_len){
		count = max_len;
	}
	
	dest->uses = 1;
	dest->len = count;
	// TODO:IMPL:LOW Determine type for destination string
	uint_fast8_t type = str_type(str); //< source type
	str_copy_buf(str_at(str, 0), str_at(dest, 0), type, count);
	
	stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
}

void cmd_dtread(struct ptc* p){
	//TODO:ERR:MED Syntax error on invalid characters, wrong length
	// Note: Invalid dates are OK! 9999/99/99 works and gives 9999, 99, 99.
	void* date_str = value_str(ARG(0));
	fixp* year = ARG(1)->value.ptr;
	fixp* month = ARG(2)->value.ptr;
	fixp* day = ARG(3)->value.ptr;
	
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
	p->stack.stack_i = 0;
}
