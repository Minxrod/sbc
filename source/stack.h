#pragma once

#include "vars.h"

enum stack_flags {
	STACK_NUMBER = 1,
	STACK_STRING = 2,
	STACK_VARIABLE = 4,
	STACK_ARRAY = 8,
};

struct stack {
	u32 stack_i;
	struct stack_entry {
		u32 type;
		union value value; 
	} entry[256];
};
