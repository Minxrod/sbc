#include "strfuncs.h"

#include "system.h"
#include "error.h"

void func_len(struct ptc* p){
	FUNC_ARGCHECK(1);
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
	FUNC_ARGCHECK(3);
	struct value_stack* s = &p->stack;
	struct stack_entry* len_e = stack_pop(s);
	struct stack_entry* start_e = stack_pop(s);
	struct stack_entry* str_e = stack_pop(s);
	
	if (str_e->type & VAR_STRING && start_e->type & VAR_NUMBER && len_e->type & VAR_NUMBER){
		void* s = VALUE_STR(str_e);
		u16 start = VALUE_INT(start_e);
		u16 len = VALUE_INT(len_e);
		//TODO:ERR: start, len must be postiive
		
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
		// TODO:IMPL Determine type(s) of strings to get types for copying
		str_copy_buf(str_at(s, start), str_at(dest, 0), 0, len);
		
		stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = dest}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}
