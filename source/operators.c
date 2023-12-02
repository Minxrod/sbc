#include "operators.h"

#include "system.h"
#include "ptc.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void op_add(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {x + y}});
	} else if (a->type & b->type & VAR_STRING){
		struct string* x, * y, * z;
		
		// The value_str function checks if 
		x = value_str(a);
		y = value_str(b);
		z = get_new_str(&p->strs);
		z->uses = 1;
		
		str_concat(x, y, z);
		
		stack_push(s, (struct stack_entry){VAR_STRING, {.ptr = z}});
	} else {
		p->exec.error = ERR_OP_DIFFERENT_TYPES;
	}
}

void op_comma(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){STACK_OP, {.number = OP_COMMA}});
}

void op_semi(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){STACK_OP, {.number = OP_SEMICOLON}});
}

void op_mult(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {(int64_t)x * y >> FIXPOINT}});
	} else if (a->type & VAR_STRING && b->type & VAR_NUMBER){
		s32 count = FP_TO_INT(VALUE_NUM(b)); //TODO:TEST:LOW Does this round down?
		struct string* x, * y;
		
		x = value_str(a);
		
		y = get_new_str(&p->strs);
		y->uses = 1;
		if (count * str_len(x) > MAX_STRLEN){
			p->exec.error = ERR_STRING_TOO_LONG;
			return;
		}
		
		for (int i = 0; i < count; ++i){
			str_concat(y, x, y);
		}
		
		stack_push(s, (struct stack_entry){VAR_STRING, {.ptr = y}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_div(struct ptc* p){
	//TODO:ERR:MID Check for overflow, division by zero
	s32 x, y;
	
	x = STACK_REL_NUM(-2);
	y = STACK_REL_NUM(-1);
	
	p->stack.stack_i -= 2;
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {(((int64_t)x << FIXPOINT) / y)}});
}

void op_sub(struct ptc* p){
	//TODO:ERR:MID Check for overflow
	s32 x, y;
	
	x = STACK_REL_NUM(-2);
	y = STACK_REL_NUM(-1);
	
	p->stack.stack_i -= 2;
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {x - y}});
}

void op_negate(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_NUMBER){
		s32 x;
		
		x = VALUE_NUM(a);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {-x}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPE;
	}
}

void op_assign(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	// a = b
	
	if (a->type & b->type & VAR_NUMBER){
		if (a->type & VAR_VARIABLE){
			s32 x = VALUE_NUM(b);
			
			*(s32*)a->value.ptr = x;
		} else {
			p->exec.error = ERR_OP_ASSIGN_TO_LITERAL;
		}
	} else if (a->type & b->type & VAR_STRING){
		if (a->type & VAR_VARIABLE){
			// A = variable containing pointer to struct string*
			struct string** dest = (struct string**)a->value.ptr;
			struct string* src = value_str(b);
			
			if (src == *dest)
				return; // no change
			if (*dest == NULL || **(char**)dest == BC_STRING){
				// if dest is currently NOT writable string
				if (src->type == STRING_CHAR){
					// shallow copy alloc'd string, track use
					(*dest) = src;
					(*dest)->uses++;
				} else if (src->type == BC_STRING){
					(*dest) = src;
					// no uses: read only
				} else {
					iprintf("Unknown source string type!\n");
					abort();
				}
			} else if (**(char**)dest == STRING_CHAR){
				// dest is writable: replace it here
				(*dest)->uses--;
				if (src->type == STRING_CHAR){
					*dest = src;
					if (src)
						(*dest)->uses++;
				} else if (src->type == BC_STRING){
					// replace with string
					(*dest) = src;
				} else {
					iprintf("Unknown source string type!\n");
					abort();
				}
			} else {
				iprintf("Unknown destination string type!\n");
				abort();
			}
		} else {
			p->exec.error = ERR_OP_ASSIGN_TO_LITERAL;
		}
	} else {
		p->exec.error = ERR_OP_ASSIGN_INVALID_TYPE;
	}
}

void op_equal(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x == y)}});
	} else if (a->type & b->type & VAR_STRING){
		struct string* x, * y;
		
		x = value_str(a);
		y = value_str(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(str_comp(x, y))}});
	} else {
		p->exec.error = ERR_OP_DIFFERENT_TYPES;
	}
}

void op_inequal(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x != y)}});
	} else {
		struct string* x, * y;
		
		x = value_str(a);
		y = value_str(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(!str_comp(x, y))}});
	}
}

void op_less(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x < y)}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_greater(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x > y)}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_less_equal(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x <= y)}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_greater_equal(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x >= y)}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_modulo(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		//TODO:ERR:MED Check for y=0
		stack_push(s, (struct stack_entry){VAR_NUMBER, {(INT_TO_FP(FP_TO_INT(x) % FP_TO_INT(y)))}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_and(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		fixp x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(FP_TO_INT(x) & FP_TO_INT(y))}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_or(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		fixp x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(FP_TO_INT(x) | FP_TO_INT(y))}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_xor(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		fixp x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(FP_TO_INT(x) ^ FP_TO_INT(y))}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_not(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_NUMBER){
		fixp x;
		
		x = VALUE_NUM(a);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(~FP_TO_INT(x))}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

void op_logical_not(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_NUMBER){
		fixp x;
		
		x = VALUE_NUM(a);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x == 0)}});
	} else {
		p->exec.error = ERR_OP_INVALID_TYPES;
	}
}

