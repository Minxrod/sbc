#include "flow.h"

#include "common.h"
#include "ptc.h"
#include "system.h"
#include "program.h"
#include "error.h"

#include "tokens.h"

#include <stdlib.h>
#include <stdio.h>

void cmd_for(struct ptc* p){
	// current stack consists of one var ptr
	// Note: Don't pop from stack - variable is needed for initial assignment later
	struct stack_entry* e = &p->stack.entry[0];
	
	p->calls.entry[p->calls.stack_i].type = CALL_FOR;
	//note: this points to just after FOR
	u32 index = p->exec.index;
	do {
		index = bc_scan(p->exec.code, index, BC_OPERATOR);
	} while (index != BC_SCAN_NOT_FOUND && p->exec.code->data[index+1] != OP_ASSIGN);
//	iprintf("\nIndex of OP_ASSIGN: %ld", index);
	if (index == BC_SCAN_NOT_FOUND){
		p->exec.error = ERR_MISSING_OP_ASSIGN_FOR;
		return;
	}
	
	p->calls.entry[p->calls.stack_i].address = index+2;
	p->calls.entry[p->calls.stack_i].var_type = e->type;
	p->calls.entry[p->calls.stack_i].var = e->value.ptr;
	
	p->calls.stack_i++;
}

void cmd_to(struct ptc* p){
	(void)p;
}

void cmd_step(struct ptc* p){
	(void)p;
}

/// PTC command marking the end of a `FOR` loop.
/// 
/// Format: 
/// * `NEXT [variable]`
/// 
/// Arguments:
/// * variable - Optional variable for iteration
/// 
/// @param a Arguments
void cmd_next(struct ptc* p){
	struct program for_condition;
	
	// get NEXT variable if needed
	struct stack_entry* e = NULL;
	if (p->stack.stack_i){
		// variable ptr
		e = stack_pop(&p->stack);
	}
	
	// get call stack top / number of elements
	s32 stack_i = p->calls.stack_i;
	if (!stack_i){
		// empty stack
		p->exec.error = ERR_NEXT_WITHOUT_FOR;
		return;
	}
	while(true){
		if (p->calls.entry[--stack_i].type == CALL_FOR){
			// stack points to FOR call - is it correct?
			if (e == NULL || p->calls.entry[stack_i].var == e->value.ptr){
				break;
			}
		}
		if (stack_i < 0){
			// did not find FOR anywhere in stack
			p->exec.error = ERR_NEXT_WITHOUT_FOR;
			return;
		}
	}
	// this is the FOR call we are processing.
	struct call_entry* c = &p->calls.entry[stack_i];
	u32 addr = c->address; //points to just after FOR
	// march address forward until hitting B command
	addr = bc_scan(p->exec.code, addr, BC_BEGIN_LOOP);
	struct program* code = p->exec.code;
	struct runner temp = p->exec; // copy code state (not stack)
	
	// addr points to loop condition setup
	for_condition.size = addr - c->address;
	for_condition.data = &code->data[c->address];
	run(&for_condition, p);
	p->exec = temp; //restore program state
	
	// now stack should contain (END, [STEP])
	s32* current = (s32*)c->var;
	s32 end;
	s32 step;
	if (p->stack.stack_i == 1){
		step = INT_TO_FP(1);
		end = stack_pop(&p->stack)->value.number;
	} else {
		step = stack_pop(&p->stack)->value.number; 
		end = stack_pop(&p->stack)->value.number;
	}
	(*current) += step;
	s32 val = *current;
	// check if need to loop
	if ((step < 0 && end > val) || (step >= 0 && end < val)){
		// loop ends
		// REMOVE ENTRY stack_i from the stack
		// TODO:IMPL copy down all further entries
		p->calls.stack_i--; // decrease stack count
	} else {
		// loop continues, jump back to this point
		p->exec.index = c->address;
	}
	// NEXT should clear stack when done
	p->stack.stack_i = 0;
}

// IF uses the value on the stack to determine where to jump to next, either the
// THEN/GOTO block or the ELSE block.
void cmd_if(struct ptc* p){
	// current stack consists of one item (should be numeric)
	struct stack_entry* e = stack_pop(&p->stack);
	// TODO:ERR type check
	u32 index = p->exec.index;
	if (e->value.number != 0){
		// true: proceed to next instruction as normal
	} else {
		// false: proceed to ELSE or ENDIF
		while ((index = bc_scan(p->exec.code, index, BC_COMMAND)) != BC_SCAN_NOT_FOUND){
			if (p->exec.code->data[index+1] == CMD_ELSE || p->exec.code->data[index+1] == CMD_ENDIF){
				break; // found ELSE or ENDIF
			}
			// continue search past this instruction
			index += 2;
		}
		
		if (index == BC_SCAN_NOT_FOUND){
			p->exec.error = ERR_MISSING_ELSE_AND_ENDIF;
			return;
		} else {
			index += 2; // move to instruction past ELSE or ENDIF
		}
	}
	char* label = &p->exec.code->data[index];
	if (label[0] == BC_LABEL_STRING){
		// GOTO label
		index = 0;
		while ((index = bc_scan(p->exec.code, index, BC_LABEL)) != BC_SCAN_NOT_FOUND){
			// found index: check correctness
			// TODO:PERF fast search/cache label locations?
//			iprintf("%d\n", index);
			if (str_comp(&p->exec.code->data[index], label)){
				// this is the index, jump to here
				break;
			}
			index += 2;
		}
		if (index == BC_SCAN_NOT_FOUND){
			p->exec.error = ERR_LABEL_NOT_FOUND;
			return;
		}
	}
	p->exec.index = index;
	// IF should clear stack [only contains condition]
	p->stack.stack_i = 0;
}

void cmd_then(struct ptc* p){
	p->exec.error = ERR_COMMAND_EXISTS;
}

// When hitting an ELSE instruction (only hit from THEN block)
// jump to the next ENDIF.
// TODO:IMPL Comment style ELSE should not break...
void cmd_else(struct ptc* p){
	u32 index = p->exec.index;
	do {
		index = bc_scan(p->exec.code, index, BC_COMMAND);
	} while (index != BC_SCAN_NOT_FOUND &&
	p->exec.code->data[index+1] != CMD_ENDIF);
	
	if (index == BC_SCAN_NOT_FOUND){
		p->exec.error = ERR_MISSING_ELSE_AND_ENDIF;
		return;
	} else {
		index += 2; // move to instruction past ELSE or ENDIF
		p->exec.index = index;
	}
}

void cmd_endif(struct ptc* p){
	(void)p;
}

void cmd_goto_gosub(struct ptc* p, bool push_return){
	// stack should contain pointer to label string (string type, with subtype BC_LABEL_STRING)
	// TODO:ERR Check that stack has entries
	struct stack_entry* e = &p->stack.entry[0];
///	struct stack_entry* e = stack_pop(&p->stack);
	char* label;
	if (e->type & VAR_NUMBER){
		// Rest of stack contains labels in order
		s32 label_index = FP_TO_INT(VALUE_NUM(e)); //TODO:TEST Do fractional values round down for this one?
//		iprintf("%d,%d\n", (int)label_index, (int)p->stack.stack_i);
		if (label_index < 0 || label_index+1 >= (int)p->stack.stack_i){
			p->stack.stack_i = 0;
			return; // No jump: number is out of range
		}
		label = (char*)p->stack.entry[label_index+1].value.ptr;
	} else if (e->type & VAR_STRING){
		label = (char*)e->value.ptr;
	} else {
		// Typeless variable? something went wrong
		p->stack.stack_i = 0;
		ERROR(ERR_UNKNOWN_TYPE);
	}
	if (label[0] == BC_LABEL_STRING){
		// Search code for label
		u32 index = 0;
		while ((index = bc_scan(p->exec.code, index, BC_LABEL)) != BC_SCAN_NOT_FOUND){
			// found index: check correctness
//			iprintf("%c,%s", p->exec.code->data[index], label);
			// TODO:PERF fast search/cache label locations?
			if (str_comp(&p->exec.code->data[index], label)){
				// this is the index, jump to here
				break;
			}
			u8 len = p->exec.code->data[index+1]; //TODO:CODE Advance by length as a function
			index += len + (len & 1);
		}
		// TODO:IMPL check when label does not exist
		if (index == BC_SCAN_NOT_FOUND){
			ERROR(ERR_LABEL_NOT_FOUND);
		}
		
		if (push_return){
			p->calls.entry[p->calls.stack_i].type = CALL_GOSUB;
			p->calls.entry[p->calls.stack_i].address = p->exec.index;
			p->calls.stack_i++;
		}
		
		p->exec.index = index;
	} else {
		// TODO:IMPL Implement actual strings as arguments (should be similar)
		p->exec.error = ERR_UNIMPLEMENTED;
	}
	// GOTO needs to clear stack of labels
	p->stack.stack_i = 0;
}

void cmd_goto(struct ptc* p){
	cmd_goto_gosub(p, false);
}

void cmd_gosub(struct ptc* p){
	cmd_goto_gosub(p, true);
}

void cmd_on(struct ptc* p){
	(void)p;
}

void cmd_return(struct ptc* p){
	s32 stack_i = p->calls.stack_i;
	if (!stack_i){
		// empty stack
		p->exec.error = ERR_RETURN_WITHOUT_GOSUB;
		return;
	}
	while(true){
		if (p->calls.entry[--stack_i].type == CALL_GOSUB){
			break;
		}
		if (stack_i < 0){
			// did not find GOSUB anywhere in stack
			p->exec.error = ERR_RETURN_WITHOUT_GOSUB;
			return;
		}
	}
	// Here, stack_i points to RETURN.
	p->exec.index = p->calls.entry[stack_i].address;
	p->calls.stack_i = stack_i; // Reduce stack
}

void cmd_end(struct ptc* p){
	p->exec.index = p->exec.code->size;
}

void cmd_stop(struct ptc* p){
	(void)p;
}
