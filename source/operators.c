#include "operators.h"

#include "system.h"
#include "ptc.h"

#include <stdio.h>
#include <stdlib.h>

void op_add(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & STACK_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){STACK_NUMBER, {x + y}});
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
	
	if (a->type & b->type & STACK_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){STACK_NUMBER, {x * y >> 12}});
	}
}

void op_div(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & STACK_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){STACK_NUMBER, {x / y << 12}});
	}
}

void op_sub(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & STACK_NUMBER){
		s32 x, y;
		
		x = VALUE_NUM(a);
		y = VALUE_NUM(b);
		
		stack_push(s, (struct stack_entry){STACK_NUMBER, {x - y}});
	}
}

void op_assign(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & b->type & STACK_NUMBER){
		if (a->type & STACK_VARIABLE){
			s32 x = VALUE_NUM(b);
			
			*(s32*)a->value.ptr = x;
		} else {
			iprintf("Can't assign to literal!\n");
			abort();
		}
	} else {
		iprintf("String assignment not yet implemented\n");
		abort();
	}
}


