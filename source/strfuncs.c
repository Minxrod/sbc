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
