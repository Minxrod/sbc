#include "stack.h"

struct stack_entry* stack_get(struct stack* s, int index){
	return &s->entry[index];
}

void stack_push(struct stack* s, struct stack_entry value){
	//TODO: Handle full stack
	s->entry[s->stack_i++] = value;
}

struct stack_entry* stack_pop(struct stack* s){
	//TODO: Handle empty stack
	return &s->entry[--s->stack_i];
}

