#include "operators.h"

#include "strs.h"
#include "system.h"
#include "ptc.h"
#include "error.h"
#include "vars.h"
#include "variables.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

void op_add(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	assert(a->type & b->type & (VAR_NUMBER | VAR_STRING)); // must share a type
	
	if (a->type & VAR_NUMBER){
		fixp x = VALUE_NUM(a);
		fixp y = VALUE_NUM(b);
		dfixp z = (dfixp)x + y;
		if (z < INT_MIN || z > INT_MAX){
			ERROR(ERR_OVERFLOW);
		}
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {x + y}});
	} else {
		// The value_str function checks uses to get the next string
		struct string* z = get_new_str(&p->strs);
		struct string* x = value_str(a);
		struct string* y = value_str(b);
		z->uses = 1;
		
		if (str_len(x) + str_len(y) > MAX_STRLEN){
			ERROR(ERR_STRING_TOO_LONG);
		}
		
		str_concat(x, y, z);
		
		stack_push(s, (struct stack_entry){VAR_STRING, {.ptr = z}});
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
		fixp x = VALUE_NUM(a);
		fixp y = VALUE_NUM(b);
		dfixp z = (dfixp)x * y >> FIXPOINT;
		if (z < INT_MIN || z > INT_MAX){
			ERROR(ERR_OVERFLOW);
		}
	
		stack_push(s, (struct stack_entry){VAR_NUMBER, {z}});
	} else if (a->type & VAR_STRING && b->type & VAR_NUMBER){
		int count = FP_TO_INT(VALUE_NUM(b));
		if (count < 0) count = 0;
		
		// must be done first to keep uses correct
		struct string* y = get_new_str(&p->strs);
		struct string* x = value_str(a);
		
		y->uses = 1;
		y->len = 0; // start empty
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
	fixp x = STACK_REL_NUM(-2);
	fixp y = STACK_REL_NUM(-1);
	
	if (y == 0) ERROR(ERR_DIVIDE_BY_ZERO);
	dfixp z = ((dfixp)x << FIXPOINT) / y;
	if (z < INT_MIN || z > INT_MAX){
		ERROR(ERR_OVERFLOW);
	}
	
	p->stack.stack_i -= 2;
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {z}});
}

void op_sub(struct ptc* p){
	fixp x = STACK_REL_NUM(-2);
	fixp y = STACK_REL_NUM(-1);
	dfixp z = (dfixp)x - y;
	if (z < INT_MIN || z > INT_MAX){
		ERROR(ERR_OVERFLOW);
	}
	
	p->stack.stack_i -= 2;
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {x - y}});
}

void op_negate(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	fixp x = VALUE_NUM(a);

	stack_push(s, (struct stack_entry){VAR_NUMBER, {-x}});
}

void op_assign(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	// a = b
	
	assert(a->type & b->type & (VAR_NUMBER | VAR_STRING)); // must share a type
	
	if (a->type & VAR_NUMBER){
		*(fixp*)a->value.ptr = VALUE_NUM(b);
	} else {
		// A = variable containing pointer to struct string*
		struct string** dest = (struct string**)a->value.ptr;
		struct string* new_alloc = get_new_str(&p->strs); // creates copies for strings from source
		// if new_alloc is unused, there is no problem since uses is not set
		struct string* src = value_str(b);
		assert(dest); // var must exist
		assert(*dest); // must contain a valid string already
		// char dest_old_type = (*dest)->type;
		
		if (is_dyn_str(*dest)){
			// dest is dynamic: we replace it here, so reduce uses count
			(*dest)->uses--;
		}
		// TODO:IMPL:LOW TOKOPT for storing read-only BC_STRING
		// (saves memory but breaks on bytecode replacement)
		if (src->type == STRING_CHAR){
			*dest = src;
			(*dest)->uses++;
		} else if (src->type == BC_STRING){
			if (!new_alloc) {
				ERROR(ERR_OUT_OF_MEMORY); // failed to alloc string
			}
			str_copy(src, new_alloc);
			++new_alloc->uses;
			*dest = new_alloc;
		} else if (src->type == STRING_WIDE){
			if (src == &p->res.mem_str){
				// For MEM specifically, we need to copy the characters to a new string
				// TODO:IMPL:LOW Handle extra characters better
				str_copy(src, new_alloc);
				++new_alloc->uses;
				*dest = new_alloc;
			} else { // just assign a reference
				*dest = src;
				(*dest)->uses++;
			}
		} else {
			ABORT("Unknown source string type!\n");
		}
	}
}

void op_equal(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	assert(a->type & b->type & (VAR_NUMBER | VAR_STRING)); // must share a type
	
	if (a->type & VAR_NUMBER){
		fixp x = VALUE_NUM(a);
		fixp y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x == y)}});
	} else {
		struct string* x = value_str(a);
		struct string* y = value_str(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(str_comp(x, y))}});
	}
}

void op_inequal(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	assert(a->type & b->type & (VAR_NUMBER | VAR_STRING)); // must share a type
	
	if (a->type & VAR_NUMBER){
		fixp x = VALUE_NUM(a);
		fixp y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x != y)}});
	} else {
		struct string* x;
		struct string* y;
		
		x = value_str(a);
		y = value_str(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(!str_comp(x, y))}});
	}
}

void op_less(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_NUMBER){
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(VALUE_NUM(a) < VALUE_NUM(b))}});
	} else {
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(str_lex(value_str(a), value_str(b)) < 0)}});
	}
}

void op_greater(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_NUMBER){
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(VALUE_NUM(a) > VALUE_NUM(b))}});
	} else {
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(str_lex(value_str(a), value_str(b)) > 0)}});
	}
}

void op_less_equal(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	

	if (a->type & VAR_NUMBER){
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(VALUE_NUM(a) <= VALUE_NUM(b))}});
	} else {
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(str_lex(value_str(a), value_str(b)) <= 0)}});
	}
}

void op_greater_equal(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_NUMBER){
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(VALUE_NUM(a) >= VALUE_NUM(b))}});
	} else {
		stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(str_lex(value_str(a), value_str(b)) >= 0)}});
	}
}

void op_modulo(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);

	fixp x = VALUE_NUM(a);
	fixp y = VALUE_NUM(b);
	
	if (y == 0) ERROR(ERR_DIVIDE_BY_ZERO);
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, {x % y}});
}

void op_and(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	fixp x = VALUE_NUM(a);
	fixp y = VALUE_NUM(b);
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(FP_TO_INT(x) & FP_TO_INT(y))}});
}

void op_or(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	fixp x = VALUE_NUM(a);
	fixp y = VALUE_NUM(b);

	stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(FP_TO_INT(x) | FP_TO_INT(y))}});
}

void op_xor(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);

	fixp x = VALUE_NUM(a);
	fixp y = VALUE_NUM(b);

	stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(FP_TO_INT(x) ^ FP_TO_INT(y))}});
}

void op_not(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	fixp x = VALUE_NUM(a);
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(~FP_TO_INT(x))}});
}

void op_logical_not(struct ptc* p){
	struct value_stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	fixp x = VALUE_NUM(a);
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(x == 0)}});
}
