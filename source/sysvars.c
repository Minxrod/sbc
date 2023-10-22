#include "sysvars.h"

#include "system.h"
#include "interpreter/stack.h"

#include <time.h>

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

void sys_date(struct ptc* p){
	struct value_stack* s = &p->stack;
	time_t t = time(NULL);
	struct tm* tm = localtime(&t);
	
	int day = tm->tm_mday;
	int month = tm->tm_mon;
	int year = tm->tm_year + 1900;
	
	struct string* str = get_new_str(&p->strs);
	str->uses = 1;
	str->len = 10;
	
	str->ptr.s[0] = year / 1000 + '0';
	str->ptr.s[1] = (year / 100) % 10 + '0';
	str->ptr.s[2] = (year / 10) % 10 + '0';
	str->ptr.s[3] = year % 10 + '0';
	str->ptr.s[4] = '/';
	str->ptr.s[5] = month / 10 + '0';
	str->ptr.s[6] = month % 10 + '0';
	str->ptr.s[7] = '/';
	str->ptr.s[8] = day / 10 + '0';
	str->ptr.s[9] = day % 10 + '0';
	
	stack_push(s, (struct stack_entry){VAR_STRING, .value.ptr = str});
}
