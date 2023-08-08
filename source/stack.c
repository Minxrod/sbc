#include "stack.h"

#include <stdio.h>
#include <stddef.h>

struct stack_entry* stack_get(struct value_stack* s, int index){
	return &s->entry[index];
}

void stack_push(struct value_stack* s, struct stack_entry value){
	//TODO: Handle full stack
	s->entry[s->stack_i++] = value;
}

struct stack_entry* stack_pop(struct value_stack* s){
	//TODO: Handle empty stack
	return &s->entry[--s->stack_i];
}

void stack_print(struct value_stack* s){
	iprintf("STACK:\n");
	for (size_t i = 0; i < s->stack_i; ++i){
		iprintf("%d:%d\n", (int)s->entry[i].type, (int)s->entry[i].value.number);
	}
}
