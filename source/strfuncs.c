#include "strfuncs.h"

#include "system.h"
#include "error.h"

void func_len(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_STRING){
		void* x = VALUE_STR(a);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(str_len(x))}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void func_mid(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* len_e = stack_pop(s);
	struct stack_entry* start_e = stack_pop(s);
	struct stack_entry* str_e = stack_pop(s);
	
	if (str_e->type & VAR_STRING && start_e->type & VAR_NUMBER && len_e->type & VAR_NUMBER){
		void* s = VALUE_STR(str_e);
		u16 start = VALUE_INT(start_e);
		u16 len = VALUE_INT(len_e);
		//TODO:ERR:MED start, len must be postiive
		
		s16 max_len = (s16)str_len(s) - (s16)start;
		if (max_len <= 0){
			stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = NULL}});
			return;
		} else if (max_len <= (s16)len){
			len = max_len;
		}
		
		struct string* dest = get_new_str(&p->strs);
		dest->uses = 1;
		dest->len = len;
		// TODO:IMPL:LOW Determine type(s) of strings to get types for copying
		str_copy_buf(str_at(s, start), str_at(dest, 0), 0, len);
		
		stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void func_val(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_STRING){
		void* str = VALUE_STR(a);
		void* str_begin = str_at(str, 0);
		int len = str_len(str);
		bool negate = false;
		fixp v;
		
		u8 first_char = *(u8*)str_begin;
		if (first_char == '-'){
			negate = true;
			str_begin = str_at(str, 1);
			len--;
			v = str_to_num(str_begin, str_len(str));
		} else if (first_char == '&'){
			u8 second_char = *(u8*)str_at(str, 1);
			str_begin = str_at(str, 2);
			if (second_char == 'H'){
				// parse as hex
				v = str_to_number(str_begin, len-2, 16, false);
			} else if (second_char == 'B'){
				// parse as binary
				v = str_to_number(str_begin, len-2, 2, false);
			} else {
				ERROR(ERR_SYNTAX);
			}
		} else {
			v = str_to_num(str_begin, str_len(str));
		}
		// TODO:IMPL:LOW This doesn't work for u16 strings
		if (negate) v = -v;
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {v}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
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
	void* str = VALUE_STR(string);
	void* sub = VALUE_STR(substring);
	
	int len_string = str_len(str);
	int len_substring = str_len(sub);
	// empty string always matches
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
			// match failed, try again at next char
			j = 0;
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

void cmd_dtread(struct ptc* p){
	//TODO:ERR:MED Syntax error on invalid characters, wrong length
	// Note: Invalid dates are OK! 9999/99/99 works and gives 9999, 99, 99.
	void* date_str = VALUE_STR(ARG(0));
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
