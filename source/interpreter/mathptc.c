#include "mathptc.h"

#include "common.h"
#include "error.h"
#include "system.h"

#include <stdlib.h>
#include <math.h>

void func_rnd(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	int r;
	int max;

	max = FP_TO_INT(VALUE_NUM(a));
	r = rand();
	if (max == 0) {
		max = 1;
	}
	if (max < 0){
		max = -max;
		r %= max;
		r = -r;
	} else {
		r %= max;
	}

	stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(r)}});
}

// Approximately 3.141 in 20.12FP
#define FIXP_PI 12867

void func_pi(struct ptc* p){
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {FIXP_PI}});
}

void func_floor(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(FP_TO_INT(VALUE_NUM(a)))}});
}

fixp func_log_internal(fixp value){
	return round(4096*log(value/4096.0));
}

void func_log(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	fixp log_a = VALUE_NUM(a);
	if (log_a <= 0){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
//	iprintf("%d\n", log_a);
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {func_log_internal(log_a)}});
}

fixp func_atan_internal(fixp value){
	fixp res = floor(4096*(atan(value/4096.0)));
	return res;
}

fixp func_atan2_internal(fixp y, fixp x){
	fixp res = floor(4096*(atan2(y/4096.0, x/4096.0)));
	return res;
}

void func_pow(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* e = stack_pop(s);
	struct stack_entry* b = stack_pop(s);
	
	fixp pow_b = VALUE_NUM(b);
	fixp pow_e = VALUE_NUM(e);

	float f = powf(pow_b/4096.0f, pow_e/4096.0f);
	iprintf("%f", f);

	if (isinf(f)){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	} else if (f >= 524288.0f || f <= -524288.0f) {
		ERROR(ERR_OVERFLOW);
	}
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {4096*f}});
}

fixp func_rad_internal(fixp deg){
	return (int64_t)deg * FIXP_PI / INT_TO_FP(180);
}

fixp func_deg_internal(fixp rad){
	return (int64_t)rad * INT_TO_FP(180) / FIXP_PI;
}

void func_rad(struct ptc* p){
	struct stack_entry* deg = stack_pop(&p->stack);
	
	STACK_RETURN_NUM(func_rad_internal(VALUE_NUM(deg)));
}

void func_deg(struct ptc* p){
	struct stack_entry* rad = stack_pop(&p->stack);
	
	STACK_RETURN_NUM(func_deg_internal(VALUE_NUM(rad)));
}

// separate to make testing easier
fixp func_sin_internal(fixp value){
	fixp res = floor(4096*(sin(value/4096.0)));
	return res;
}

// TODO:PERF:MED Replace with libnds fixedpoint versions?
fixp func_cos_internal(fixp value){
	fixp res = floor(4096*(cos(value/4096.0)));
	return res;
}

fixp func_tan_internal(fixp value){
	fixp res = floor(4096*(tan(value/4096.0)));
	return res;
}

void func_sin(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	fixp angle = VALUE_NUM(a);
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {func_sin_internal(angle)}});
}

void func_cos(struct ptc* p){
	struct stack_entry* rad = stack_pop(&p->stack);
	
	STACK_RETURN_NUM(func_cos_internal(VALUE_NUM(rad)));
}

void func_tan(struct ptc* p){
	struct stack_entry* rad = stack_pop(&p->stack);
	
	STACK_RETURN_NUM(func_tan_internal(VALUE_NUM(rad)));
}

void func_atan(struct ptc* p){
	if (p->exec.argcount == 1){
		struct stack_entry* ratio = stack_pop(&p->stack);
		
		STACK_RETURN_NUM(func_atan_internal(VALUE_NUM(ratio)));
	} else {
		struct stack_entry* x = stack_pop(&p->stack);
		struct stack_entry* y = stack_pop(&p->stack);
		
		STACK_RETURN_NUM(func_atan2_internal(VALUE_NUM(y), VALUE_NUM(x)));
	}
}

void func_abs(struct ptc* p){
	struct stack_entry* num = stack_pop(&p->stack);
	
	fixp n = VALUE_NUM(num);
	// Check added because signed integer overflow is undefined behavior
	if ((u32)n == 0x80000000){
		STACK_RETURN_NUM(n); // this is the same behavior as PTC
	}
	n = n < 0 ? -n : n;
	STACK_RETURN_NUM(n);
}

void func_sgn(struct ptc* p){
	struct stack_entry* num = stack_pop(&p->stack);
	
	fixp n = VALUE_NUM(num);
	fixp r = n < 0 ? -INT_TO_FP(1) : n > 0 ? INT_TO_FP(1) : 0;
	
	STACK_RETURN_NUM(r);
}

fixp func_sqr_internal(fixp value){
	fixp res = floor(4096*(sqrt(value/4096.0)));
	return res;
}

fixp func_exp_internal(fixp value){
	fixp res = floor(4096*(exp(value/4096.0)));
	return res;
}

void func_sqr(struct ptc* p){
	struct stack_entry* val = stack_pop(&p->stack);
	
	STACK_RETURN_NUM(func_sqr_internal(VALUE_NUM(val)));
}

void func_exp(struct ptc* p){
	struct stack_entry* val = stack_pop(&p->stack);
	
	STACK_RETURN_NUM(func_exp_internal(VALUE_NUM(val)));
}


