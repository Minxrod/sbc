#include "operators.h"

#include "system.h"
#include "ptc.h"

#include <stdio.h>
#include <stdlib.h>

void op_add(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {x + y}});
	} else if (a->type & b->type & VAR_STRING){
		struct string* x, * y, * z;
		
		x = VALUE_STR(a);
		y = VALUE_STR(b);
		z = get_new_str(&p->strs);
		
		str_concat(x, y, z);
		
		stack_push(s, (struct stack_entry){VAR_STRING, {.ptr = z}});
	} else {
		iprintf("Types not correct!!!\n");
		abort();
	}
}

void op_comma(struct ptc* p){
	struct stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){STACK_OP, {.number = OP_COMMA}});
}

void op_semi(struct ptc* p){
	struct stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){STACK_OP, {.number = OP_SEMICOLON}});
}


void op_mult(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {x * y >> 12}});
	}
}

void op_div(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {x / y << 12}});
	}
}

void op_sub(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {x - y}});
	}
}

void op_assign(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & VAR_NUMBER){
		if (a->type & VAR_VARIABLE){
			s32 x = VALUE_NUM(b);
			
			*(s32*)a->value.ptr = x;
		} else {
			iprintf("Can't assign to literal!\n");
			abort();
		}
	} else if (a->type & b->type & VAR_STRING){
		if (a->type & VAR_VARIABLE){
			// A = variable containing pointer to struct string*
			struct string** str = (struct string**)a->value.ptr;
			// a->value.ptr is a struct string*. The variable stored value would be... struct string**.
			// to assign to s, need to take the stack string b, which could contain:
			if ((*str)->type == STRING_EMPTY){
				// select new struct string* to store within variable (?)
				*str = get_new_str(&p->strs);
			}
			str_copy(b->value.ptr, *str);
		} else {
			iprintf("Can't assign to literal!\n");
			abort();
		}
		
//		iprintf("String assignment not yet implemented\n");
//		abort();
	} else {
		iprintf("Invalid variable type for assignment\n");
		abort();
	}
}


