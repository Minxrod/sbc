#include "flow.h"

#include "common.h"
#include "label.h"
#include "ptc.h"
#include "system.h"
#include "program.h"
#include "error.h"

#include "tokens.h"
#include "vars.h"

#include <stdlib.h>
#include <stdio.h>

void cmd_for(struct ptc* p){
	// current stack consists of one var ptr
	// Old design: [VAR] [FOR] [START] [ASSIGNMENT]
	// New design: [VAR] [START] [ASSIGNMENT] [FOR]
	// This works the same way, but it relies on the additional assumption that stack entries
	// are never erased until a new entry is added.
	struct stack_entry* e = &p->stack.entry[0];
	
	p->calls.entry[p->calls.stack_i].type = CALL_FOR;
	//note: this points to just after FOR
	u32 index = p->exec.index - 2;
	do {
		index = bc_scan(p->exec.code, index + 2, BC_OPERATOR);
	} while (index != BC_SCAN_NOT_FOUND && p->exec.code.data[index+1] != OP_ASSIGN);
//	iprintf("\nIndex of OP_ASSIGN: %u", index);
	if (index == BC_SCAN_NOT_FOUND){
		p->exec.error = ERR_MISSING_OP_ASSIGN_FOR;
		return;
	}
	
	if (p->calls.stack_i == CALL_STACK_SIZE-1){
		ERROR(ERR_OUT_OF_MEMORY);
	}
	// Note: instruction index is limited to [0,MAX_SOURCE_SIZE) currently
	// Line size (and distance to BC_BEGIN_LOOP) is in [0,100) and usually small
	// Store offset as part of index here, and save the searching on every NEXT command
	idx to_start = index + 2;
	idx to_size = bc_scan(p->exec.code, to_start, BC_BEGIN_LOOP);
	if (to_size == BC_SCAN_NOT_FOUND) {
		ERROR(ERR_FOR_MISSING_BEGIN_LOOP);
	}

	// stores beginning of TO [STEP] expression and size of expression
	p->calls.entry[p->calls.stack_i].address = to_start | ((to_size - to_start) << 24);
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
	// get NEXT variable if needed
	struct stack_entry* e = NULL;
	if (p->stack.stack_i){
		// variable ptr
		e = stack_pop(&p->stack);
	}
	
	// get call stack top / number of elements
	s32 stack_i = p->calls.stack_i;
	if (stack_i <= 0){
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
		if (stack_i <= 0){
			// did not find FOR anywhere in stack
			ERROR(ERR_NEXT_WITHOUT_FOR);
		}
	}
	// this is the FOR call we are processing.
	struct call_entry* c = &p->calls.entry[stack_i];
	u32 addr = c->address; //points to just after FOR
	idx to_start = addr & 0x00ffffff;
	idx to_size = addr >> 24;

	struct bytecode code = p->exec.code;
	struct runner temp = p->exec; // copy code state (not stack)
	
	// addr points to loop condition setup
	struct bytecode for_condition;
	for_condition.size = to_size;
	for_condition.data = code.data + to_start;
	run(for_condition, p);
	p->exec = temp; //restore program state
	
	// now stack should contain (END, [STEP])
	s32* current = (s32*)c->var;
	s32 end = STACK_NUM(0);
	s32 step;
	if (p->stack.stack_i == 1){
		step = INT_TO_FP(1);
	} else {
		step = STACK_NUM(1);
	}
	int64_t overflow_check= (int64_t)*current + step;
	if (overflow_check > INT_MAX || overflow_check < INT_MIN){
		ERROR(ERR_OVERFLOW);
	}
	(*current) = overflow_check;
	s32 val = *current;
	// check if need to loop
	if ((step < 0 && end > val) || (step >= 0 && end < val)){
		// loop ends
		// Remove entry stack_i from the stack
		p->calls.stack_i--; // decrease stack count: we will copy one past this down
		for (u32 i = stack_i; i < p->calls.stack_i; ++i){
			p->calls.entry[i] = p->calls.entry[i+1];
		}
	} else {
		// loop continues, jump back to this point
		p->exec.index = to_start;
	}
	// NEXT should clear stack when done
	p->stack.stack_i = 0;
}

// IF uses the value on the stack to determine where to jump to next, either the
// THEN/GOTO block or the ELSE block.
void cmd_if(struct ptc* p){
	// current stack consists of one item (should be numeric)
	fixp value = STACK_NUM(0);
//	struct stack_entry* e = stack_pop(&p->stack);
	u32 index = p->exec.index;
	bool jump_ok = true;
	if (value){
		// true: proceed to next instruction as normal
	} else {
		// false: proceed to ELSE or ENDIF
		while ((index = bc_scan(p->exec.code, index, BC_COMMAND)) != BC_SCAN_NOT_FOUND){
			if (p->exec.code.data[index+1] == CMD_ELSE || p->exec.code.data[index+1] == CMD_ENDIF){
				break; // found ELSE or ENDIF
			}
			// continue search past this instruction
			index += 2;
		}
		
		if (index == BC_SCAN_NOT_FOUND){
			p->exec.error = ERR_MISSING_ELSE_AND_ENDIF;
			return;
		}
		jump_ok = p->exec.code.data[index+1] == CMD_ELSE;
		index += 2; // move to instruction past ELSE or ENDIF
	}
	// If there's a label AND it's not followed by a GOSUB
	char* label = (char*)&p->exec.code.data[index];
	if (jump_ok && label[0] == BC_LABEL_STRING){
		int len = label[1];
		int instr = label[2+len+(len & 1)];
		int cmd = label[2+len+(len & 1)+1];
		if (instr == BC_COMMAND && (cmd == CMD_GOSUB || cmd == CMD_GOTO)){
			// Don't jump because it is handled by next instruction
		} else {
			// GOTO label
			index = search_label(p, label);
		}
	}
	p->exec.index = index;
}

void cmd_then(struct ptc* p){
	p->exec.error = ERR_COMMAND_EXISTS;
}

// When hitting an ELSE instruction (only hit from THEN block)
// jump to the next ENDIF.
// TODO:IMPL:LOW Comment style ELSE should not break...
void cmd_else(struct ptc* p){
	u32 index = p->exec.index - 2; // start from immediately after the else
	do {
		index = bc_scan(p->exec.code, index + 2, BC_COMMAND);
	} while (index != BC_SCAN_NOT_FOUND &&
	p->exec.code.data[index+1] != CMD_ENDIF);
	
	if (index == BC_SCAN_NOT_FOUND){
		ERROR(ERR_MISSING_ELSE_AND_ENDIF);
	}
	index += 2; // move to instruction past ELSE or ENDIF
	p->exec.index = index;
}

void cmd_endif(struct ptc* p){
	(void)p;
}

void cmd_goto_gosub(struct ptc* p, bool push_return){
	// stack should contain pointer to label string (string type, with subtype BC_LABEL_STRING)
	assert(p->stack.stack_i);
	struct stack_entry* e = ARG(0);
	void* label = NULL;
	assert(e->type & (VAR_NUMBER | VAR_STRING));
	if (e->type & VAR_NUMBER){
		// Rest of stack contains labels in order
		int label_i = VALUE_INT(e);
//		iprintf("%d,%d\n", (int)label_index, (int)p->stack.stack_i);
		if (label_i < 0 || label_i+1 >= (int)p->stack.stack_i){
			return; // No jump: number is out of range
		}
		if (p->stack.entry[label_i+1].type & STACK_OP){
			ERROR(ERR_SYNTAX); // label is missing from this slot
		}
		label = p->stack.entry[label_i+1].value.ptr;
	} else {
		label = value_str(e); //value_str
	}
	// Search code for label
	u32 index = search_label(p, label);
	
	if (push_return){
		if (p->calls.stack_i == CALL_STACK_SIZE-1){
			ERROR(ERR_OUT_OF_MEMORY);
		}
		p->calls.entry[p->calls.stack_i].type = CALL_GOSUB;
		p->calls.entry[p->calls.stack_i].address = p->exec.index;
		p->calls.stack_i++;
	}
	
	p->exec.index = index;
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
	p->exec.index = p->exec.code.size;
}

void cmd_stop(struct ptc* p){
	p->exec.error = ERR_BREAK;
}

void cmd_cont(struct ptc* p){
	// if this is executing, it's from DIRECT mode. how to handle this?
	(void)p;
}
