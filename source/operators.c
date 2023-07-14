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
		z->uses = 1;
		
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

void op_negate(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* a = stack_pop(s);
	
	if (a->type & VAR_NUMBER){
		s32 x;
		
		x = VALUE_NUM(a);
		
		stack_push(s, (struct stack_entry){VAR_NUMBER, {-x}});
	}
}

void op_assign(struct ptc* p){
	struct stack* s = &p->stack;
	struct stack_entry* b = stack_pop(s);
	struct stack_entry* a = stack_pop(s);
	// a = b
	
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
			//TODO: if B is variable: shallow copy
			//TODO: if B is RO literal: copy read only
			// A = variable containing pointer to struct string*
			struct string** dest = (struct string**)a->value.ptr;
			struct string* src = VALUE_STR(b);
			
			if (b->type & VAR_VARIABLE){
				// OK: has persistence
			} else {
				// will lose usage on assignment
				src->uses--;
			}
			
			if (src == *dest)
				return; // no change
			if (*dest == NULL || *(char*)dest == BC_STRING){
				// if dest is currently NOT writable string
				if (src == NULL){
					(*dest) = NULL; // assign empty string, done
				} else if (src->type == STRING_CHAR){
					// shallow copy alloc'd string, track use
					(*dest) = src;
					(*dest)->uses++;
				} else if (src->type == BC_STRING){
					(*dest) = src;
					// no uses: read only
				}
			} else if (*(char*)dest == STRING_CHAR){
				// dest is writable: replace it here
				(*dest)->uses--;
				if (src == NULL || src->type == STRING_CHAR){
					*dest = src;
					(*dest)->uses++;
				} else if (src->type == BC_STRING){
					// replace with string
					(*dest) = src;
				}
			}
			
/*			// a->value.ptr is a struct string*. The variable stored value would be... struct string**.
			// to assign to s, need to take the stack string b, which could contain:
			if ((*str) == NULL || (*str)->type == STRING_EMPTY){
				// destination does not exist: shallow copy to a
				*str = b->value.ptr;
				(*str)->uses++; // this string is now used once more
			} else if ((*str)->type == BC_STRING){ 
				// destination is RO: must allocate string
				
			} else if ((*str)->uses > 1){
				// destination exists and is used more than once
				(*str)->uses--; //remove one string usage of destination
				// select new struct string* to store within variable (?)
				// and write to new string
				*str = get_new_str(&p->strs);
				str_copy(b->value.ptr, *str);
			} else {
				// string not empty and not used twice: used once
				// just overwrite destination
				str_copy(b->value.ptr, *str);
			}*/
		} else {
			iprintf("Can't assign to literal!\n");
			abort();
		}
	} else {
		iprintf("Invalid variable type for assignment\n");
		abort();
	}
}


