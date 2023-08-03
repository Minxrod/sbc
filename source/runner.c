#include "runner.h"
#include "system.h"

// TODO: Move these files to a subdirectory?
#include "console.h"
#include "operators.h"
#include "sysvars.h"
#include "error.h"

#include "tokens.h"
#include "ptc.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define BC_SCAN_NOT_FOUND UINT_MAX

// Scans for a specific instruction. Special purpose function to handle the
// various instruction lengths.
u32 bc_scan(struct program* code, u32 index, u8 find){
	// search for find in code->data
	while (index < code->size){ 
		u8 cur = code->data[index];
		if (cur == find){
			return index;
		}
		if (cur == BC_STRING){
			cur = code->data[++index];
			index ++;
			index += cur + (cur & 1);
		} else if (cur == BC_WIDE_STRING){
			cur = code->data[++index];
			index ++;
			index += sizeof(u16) * cur;
		} else if (cur == BC_DIM || cur == BC_VARIABLE_NAME){
			cur = code->data[++index];
			if (cur >= 'A'){
				++index;
			} else {
				index += cur + (cur & 1);
			}
		} else if (cur == BC_NUMBER){
			index += 6; // TODO: may need to change if number syntax gets modified
		} else {
			index += 2;
		}
	}
	return BC_SCAN_NOT_FOUND;
}

typedef void(*ptc_call)(struct ptc*);

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
	iprintf("\nIndex of OP_ASSIGN: %d", index);
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
	++p;
}

void cmd_step(struct ptc* p){
	p++;
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
		e = stack_pop(&p->stack)->value.ptr;
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
		} else if (stack_i < 0){
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
		step = 1<<12;
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
		// TODO: copy down all further entries
		p->calls.stack_i--; // decrease stack count
	} else {
		// loop continues, jump back to this point
		p->exec.index = c->address;
	}
}

// IF uses the value on the stack to determine where to jump to next, either the
// THEN/GOTO block or the ELSE block.
void cmd_if(struct ptc* p){
	// current stack consists of one item (should be numeric)
	struct stack_entry* e = stack_pop(&p->stack);
	// TODO type check
	
	if (e->value.number != 0){
		// true: proceed to next instruction as normal
	} else {
		// false: proceed to ELSE or ENDIF
		u32 index = p->exec.index;
		do {
			index = bc_scan(p->exec.code, index, BC_COMMAND);
		} while (index != BC_SCAN_NOT_FOUND &&
			p->exec.code->data[index+1] != CMD_ELSE &&
			p->exec.code->data[index+1] != CMD_ENDIF);
		
		if (index == BC_SCAN_NOT_FOUND){
			p->exec.error = ERR_MISSING_ELSE_AND_ENDIF;
			return;
		} else {
			index += 2; // move to instruction past ELSE or ENDIF
			p->exec.index = index;
		}
	}
}

// This command should never exist
void cmd_then(struct ptc* p){
	p++;
}

// When hitting an ELSE instruction (only hit from THEN block)
// jump to the next ENDIF.
// TODO: Comment style ELSE should not break...
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
	p++;
}


const ptc_call ptc_commands[] = {
	cmd_print, cmd_locate, cmd_color, NULL, // dim
	cmd_for, cmd_to, cmd_step, cmd_next,
	cmd_if, cmd_then, cmd_else, cmd_endif,
};

const ptc_call ptc_operators[] = {
	op_add,
	op_comma,
	op_sub,
	op_mult,
	op_div,
	op_semi,
	op_assign,
	op_negate,
};

const ptc_call ptc_functions[] = {
	op_add,
};

const ptc_call ptc_sysvars[] = {
	sys_true, sys_false, sys_cancel, sys_version,
};

/// Debug function for checking command/function names from IDs
void print_name(const char* names, int data){
	char name[9] = {0};
	for (size_t i = 0; i < 8; ++i){
		name[i] = names[8*data + i];
	}
	iprintf("cmd=%s", name);
}

void run(struct program* code, struct ptc* p) {
	struct runner* r = &p->exec;
	r->index = 0;
	r->code = code;
	p->stack.stack_i = 0;
	
	while (r->index < code->size && !p->exec.error){
		// get one instruction and execute it
		char instr = code->data[r->index++];
		char data = code->data[r->index++];
		
		iprintf("%c ", instr);
		
		switch (instr){
			case BC_SMALL_NUMBER:
				iprintf("val=%d", data);
				// TODO: small decimals should also be of this form?
				if (data >= 0 && data <= 99){
					p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {((u32)data) << 12}};
				} else {
					r->error = ERR_NUM_INVALID;
				}
				break;
			
			case BC_STRING:
				iprintf("len=%d ", data);
				iprintf("%.*s", data, &code->data[r->index]);
				// push string pointer to the stack
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_STRING, {.ptr = (void*)&code->data[r->index-2]}};
				r->index += data + (data & 1); // to next instruction
				break;
			
			case BC_COMMAND:
				print_name(commands, data);
				//run command using current stack
				if ((u8)data >= sizeof(ptc_commands)/sizeof(ptc_commands[0])){
					r->error = ERR_PTC_COMMAND_INVALID;
				}
				ptc_commands[(u32)data](p);
				break;
				
			case BC_OPERATOR:
				print_name(bc_conv_operations, data);
				
				if ((u8)data >= sizeof(ptc_operators)/sizeof(ptc_operators[0])){
					r->error = ERR_PTC_OPERATOR_INVALID;
				}
				ptc_operators[(u32)data](p);
				break;
				
			case BC_FUNCTION:
				print_name(functions, data);
				
				if ((u8)data >= sizeof(ptc_functions)/sizeof(ptc_functions[0])){
					r->error = ERR_PTC_FUNCTION_INVALID;
				}
				ptc_functions[(u32)data](p);
				break;
				
			case BC_SYSVAR:
				print_name(sysvars, data);
				if ((u8)data >= sizeof(ptc_sysvars)/sizeof(ptc_sysvars[0])){
					r->error = ERR_PTC_FUNCTION_INVALID;
				}
				ptc_sysvars[(u32)data](p);
				break;
				
			case BC_NUMBER:
				{
				s32 number = 0;
				
				number |= (char)code->data[r->index++] << 24;
				number |= (unsigned char)code->data[r->index++] << 16;
				number |= (unsigned char)code->data[r->index++] << 8;
				number |= (unsigned char)code->data[r->index++];
				
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {number}};
				iprintf("num=%.12f", number / 4096.0);
				}
				break;
				
			case BC_VARIABLE_NAME:
				// this is both array accesses and regular vars
				// difference determined by whether or not ARGCOUNT was set
				// before reading variable
				iprintf("name=");
				
				// type of variable
				enum types t;
				// pointer to variable value
				void* x;
				// variable struct
				char* name;
				// length of variable name
				u8 len;
				if (data < 'A'){
					name = &code->data[r->index];
					len = code->data[r->index+(u8)data-1] == '$' ? data-1 : data;
					t = code->data[r->index+(u8)data-1] == '$' ? VAR_STRING : VAR_NUMBER; 
				} else {
					name = &data;
					len = 1;
					t = VAR_NUMBER;
				}
				
				if (!r->argcount){
					struct named_var* v = get_var(&p->vars, name, len, t);
					if (!v){
						r->error = ERR_VARIABLE_CREATION_FAIL;
						break;
					}
					x = t & VAR_NUMBER ? (void*)&v->value.number : &v->value.ptr;
				} else {
					s32 a;
					s32 b = ARR_DIM2_UNUSED;
					// TODO: check for strings here before reading
					if (r->argcount == 2){
						struct stack_entry* y = stack_pop(&p->stack);
						b = VALUE_NUM(y) >> 12;
					}
					struct stack_entry* z = stack_pop(&p->stack);
					a = VALUE_NUM(z) >> 12;
					
					union value* val = get_arr_entry(&p->vars, name, len, t | VAR_ARRAY, a, b);
					x = t & VAR_NUMBER ? (void*)&val->number : &val->ptr;
				}
				
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_VARIABLE | t, {.ptr = x}};
				if (data < 'A'){
					iprintf("%.*s", data, &code->data[r->index]);
					r->index += data + (data & 1); // to next instruction
				} else {
					iprintf("%c", code->data[r->index-1]);
				}
				//debug
	//			stack_print(&p->stack);
				break;
			
			case BC_DIM:
				//TODO: error checking for strings here
				{
				u32 a;
				u32 b = ARR_DIM2_UNUSED;
				if (r->argcount == 2){
					b = stack_pop(&p->stack)->value.number >> 12;
				}
				a = stack_pop(&p->stack)->value.number >> 12;
				
				iprintf("name=");
				if (data >= 'A'){
					iprintf("%c ", data);
					get_new_arr_var(&p->vars, &data, 1, VAR_NUMBER | VAR_ARRAY, a, b);
				} else {
					char* x = &code->data[r->index];
					iprintf("%.*s", data, x);
					r->index += data + (data & 1); // to next instruction
					
					enum types t = x[(u8)data-1] == '$' ? VAR_STRING : VAR_NUMBER;
					u32 len = t & VAR_NUMBER ? data : data-1;
					
					get_new_arr_var(&p->vars, x, len, t | VAR_ARRAY, a, b);
				}
				
				iprintf(" dim=%d,%d", a, b);
				}
				break;
			
			case BC_ARGCOUNT:
				r->argcount = data;
				
				iprintf("argc=%d", r->argcount);
				break;
			
			case BC_BEGIN_LOOP:
				{
				// execute FOR condition check
				// this will always occur when the stack is prepared already
				// call_entry: Var ptr 
				// stack: end, [step]
				// TODO: check if stack has entry!
				s32* current = (s32*)p->calls.entry[p->calls.stack_i-1].var;
				s32 end;
				s32 step;
				if (p->stack.stack_i == 1){
					step = 1;
					end = stack_pop(&p->stack)->value.number;
				} else {
					step = stack_pop(&p->stack)->value.number; 
					end = stack_pop(&p->stack)->value.number;
				}
				s32 val = *current;
				if ((step < 0 && end > val) || (step >= 0 && end < val)){
					// if val + step will never reach end, then skip to NEXT
					// treat valid NEXT as first in a line
					int nest = 1;
					while (nest > 0){
						instr = code->data[r->index++];
						data = code->data[r->index++];
						if (instr == BC_COMMAND){
							if (data == CMD_NEXT){
								// TODO: Variable check...
								// find NEXT, go back one instruction, execute statement if it is VAR?
								// Found a NEXT
								nest--;
							} else if (data == CMD_FOR){
								// Found a FOR, so we need to search for another NEXT
								nest++;
							}
						}
					}
					// index now points to one past the NEXT, the correct location
				} else {
					// execution continues as normal into the loop
				}
				}
				break;
				
			default:
				iprintf("Unknown BC: %c %d", instr, data);
				r->error = ERR_INVALID_BC;
				break;
		}
		
		if (instr != BC_ARGCOUNT){
			r->argcount = 0; // zero after use to avoid reading vars as arrays
		}
		
		iprintf("\n");
	}
	/*
	for (u32 i = 0; i < p->stack.stack_i; ++i){
		iprintf("%d:%d\n", p->stack.entry[i].type, p->stack.entry[i].value.number);
	}
	*/
}
