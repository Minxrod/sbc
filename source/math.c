#include "math.h"

#include "error.h"
#include "system.h"

#include <stdlib.h>

void func_rnd(struct ptc* p){
	struct value_stack* s = &p->stack;
	// TODO:ERR Check number of arguments!
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_NUMBER){
		s32 r, max;
		
		max = FP_TO_INT(VALUE_NUM(a));
		r = rand();
		if (max == 0)
			max = 1;
		if (max < 0){
			max = -max;
			r %= max;
			r = -r;
		} else {
			r %= max;
		}
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(r)}});
	} else {
		p->exec.error = ERR_FUNC_INVALID_TYPE;
	}	
}
