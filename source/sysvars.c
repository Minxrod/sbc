#include "sysvars.h"

#include "system.h"
#include "stack.h"

// To create a read-only entry:
// Push only a literal value to the stack
// To create a writable entry: Push a VAR_VARIABLE type entry
void sys_true(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, .value.number = INT_TO_FP(1)});
}

void sys_false(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, .value.number = INT_TO_FP(0)});
}

void sys_cancel(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, .value.number = -INT_TO_FP(1)});
}

// All compatibility testing performed on a USA v2.2 copy of Petit Computer.
// Thus the VERSION returned will be 2.2, unless I ever find an alternate
// version to test against. Most behavior should be identical anyways, and
// realistically I dont think most programs used this.
void sys_version(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, .value.number = INT_TO_FP(0x2020)});
}
