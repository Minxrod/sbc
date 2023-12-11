#include "mathptc.h"

#include "error.h"
#include "system.h"

#include <stdlib.h>
#include <math.h>

void func_rnd(struct ptc* p){
	struct value_stack* s = &p->stack;
	// TODO:ERR:LOW Check number of arguments!
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

void func_pi(struct ptc* p){
	// Approximately 3.141 in 20.12FP
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {12867}}); 
}

// separate to make testing easier
fixp func_sin_internal(fixp value){
	fixp res = floor(4096*(sin(value/4096.0)));
	return res;
}

void func_sin(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	fixp angle = VALUE_NUM(a);
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {func_sin_internal(angle)}});
}

void func_floor(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(FP_TO_INT(VALUE_NUM(a)))}});
}

fixp func_log_internal(fixp value){
	return round(4096*log(value/4096));
}

void func_log(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	//TODO:ERR:LOW Bounds.
	fixp log_a = VALUE_NUM(a);
	iprintf("%d\n", log_a);
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {func_log_internal(log_a)}});
}

fixp func_cos_internal(fixp value){
	fixp res = floor(4096*(cos(value/4096.0)));
	return res;
}

fixp func_tan_internal(fixp value){
	fixp res = floor(4096*(tan(value/4096.0)));
	return res;
}

fixp func_atan_internal(fixp value){
	fixp res = floor(4096*(atan(value/4096.0)));
	return res;
}

fixp func_atan2_internal(fixp y, fixp x){
	fixp res = floor(4096*(atan2(y/4096.0, x/4096.0)));
	return res;
}

