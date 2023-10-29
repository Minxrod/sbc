#include "runner.h"
#include "system.h"

// TODO:CODE:NONE Move these files to a subdirectory?
#include "console.h"
#include "operators.h"
#include "sysvars.h"
#include "error.h"

#include "tokens.h"
#include "ptc.h"
#include "program.h"
#include "flow.h"
#include "data.h"
#include "input.h"
#include "graphics.h"

#include "mathptc.h"
#include "strfuncs.h"

#include "subsystem/background.h"
#include "subsystem/panel.h"

#include <stdio.h>
#include <stdlib.h>

typedef void(*ptc_call)(struct ptc*);

// These are the tables that map command, function, etc. IDs into operations
// All functions take the ptc struct as an argument and return nothing
// Arguments are passed via p->stack struct; return values also go here.
// Errors are indicated via p->exec.error, constants are defined in error.h

void ptc_stub(struct ptc* p){
	// Stub function consumes all arguments and does nothing
	p->stack.stack_i = 0;
}

const ptc_call ptc_commands[] = {
	cmd_print, cmd_locate, cmd_color, NULL, // dim
	cmd_for, cmd_to, cmd_step, cmd_next,
	cmd_if, cmd_then, cmd_else, cmd_endif,
	cmd_goto, cmd_gosub, cmd_on, cmd_return,
	cmd_end, cmd_stop,
	cmd_cls, cmd_visible, cmd_acls, cmd_vsync, cmd_wait,
	cmd_input, cmd_linput,
	NULL, ptc_stub, //BEEP
	cmd_bgclip, cmd_bgclr, cmd_bgcopy, cmd_bgfill, NULL, //BGMCLEAR 
	NULL, NULL, NULL, NULL, NULL, ptc_stub, NULL, //BGMVOL
	cmd_bgofs, cmd_bgpage, cmd_bgput, cmd_bgread, NULL, NULL, NULL, //CHRREAD
	NULL, cmd_clear, NULL, NULL, NULL, NULL, //CONT
	ptc_stub, NULL, cmd_dtread, NULL, cmd_gbox, //GBOX
	NULL, cmd_gcls, cmd_gcolor, NULL, NULL, cmd_gfill, cmd_gline, // GLINE, 
	cmd_gpage, NULL, cmd_gpset, NULL, NULL, NULL, NULL, //ICONSET, 
	NULL, NULL, NULL, NULL, //NEW, 
	cmd_pnlstr, cmd_pnltype, cmd_read, NULL, NULL, NULL, //RENAME, 
	cmd_restore, NULL, NULL, NULL, NULL, NULL, NULL, //SPANGLE, 
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //SPPAGE,
	NULL, NULL, NULL, NULL, NULL, //SWAP, 
	NULL //TMREAD,
};

const ptc_call ptc_operators[] = {
	op_add, op_comma, op_sub, op_mult, op_div, op_semi, op_assign, op_negate,
	op_equal, op_inequal, op_less, op_greater, op_less_equal, op_greater_equal,
	op_modulo,
	op_and, op_or, op_xor, op_not,
};

const ptc_call ptc_functions[] = {
	NULL, NULL, NULL, NULL, NULL, NULL, func_btrig, func_button,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	func_inkey, NULL, NULL, func_len, NULL, func_mid, func_pi, NULL, NULL, //FUN_RAD
	NULL, func_rnd, NULL, func_sin, NULL, NULL, NULL, NULL, //FUN_SPHITRC
	NULL, NULL, NULL, NULL, NULL, func_val, //FUN_VAL
};

const ptc_call ptc_sysvars[] = {
	sys_true, sys_false, sys_cancel, sys_version,
	NULL, sys_date,
};

/// Debug function for checking command/function names from IDs
void print_name(const char* names, int data){
	char name[9] = {0};
	for (size_t i = 0; i < 8; ++i){
		name[i] = names[8*data + i];
	}
	iprintf("cmd=%s", name);
}

/// The big interpreter function.
/// 
/// Takes a chunk of bytecode stored in code and runs it.
/// Always reads from the start of the bytecode chunk passed.
/// While executing, stores the code pointer within the p struct.
/// 
/// @param code Program bytecode struct.
/// @param p PTC struct containing entire state of interpreter + system
/// 
void run(struct program* code, struct ptc* p) {
	struct runner* r = &p->exec;
	r->index = 0;
	r->data_index = 0;
	r->data_offset = 0;
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
				// TODO:IMPL:LOW small decimals should also be of this form?
				if ((u8)data <= 99){
					p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {((u32)data) << FIXPOINT}};
				} else {
					r->error = ERR_NUM_INVALID;
				}
				break;
			
			case BC_LABEL_STRING:
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
					break;
				}
				if (ptc_commands[(u32)data]){
					ptc_commands[(u32)data](p);
				} else {
					r->error = ERR_UNIMPLEMENTED;
				}
				// FOR,TO,STEP use stack for BC_BEGIN_LOOP so don't clear
				// ON ignores value to pass it to GOTO/GOSUB, so don't clear
				if (data != CMD_FOR && data != CMD_TO && data != CMD_STEP && data != CMD_ON)
					p->stack.stack_i=0; //clear stack after most commands
				break;
				
			case BC_OPERATOR:
				print_name(bc_conv_operations, data);
				
				if ((u8)data >= sizeof(ptc_operators)/sizeof(ptc_operators[0])){
					r->error = ERR_PTC_OPERATOR_INVALID;
					break;
				}
				if (ptc_operators[(u32)data]){
					ptc_operators[(u32)data](p);
				} else {
					r->error = ERR_UNIMPLEMENTED;
				}
				break;
				
			case BC_FUNCTION:
				print_name(functions, data);
				
				if ((u8)data >= sizeof(ptc_functions)/sizeof(ptc_functions[0])){
					r->error = ERR_PTC_FUNCTION_INVALID;
					break;
				}
				if (ptc_functions[(u32)data]){
					ptc_functions[(u32)data](p);
				} else {
					r->error = ERR_UNIMPLEMENTED;
				}
				break;
				
			case BC_SYSVAR:
				print_name(sysvars, data);
				if ((u8)data >= sizeof(ptc_sysvars)/sizeof(ptc_sysvars[0])){
					r->error = ERR_PTC_SYSVAR_INVALID;
					break;
				}
				if (ptc_sysvars[(u32)data]){
					ptc_sysvars[(u32)data](p);
				} else {
					r->error = ERR_UNIMPLEMENTED;
				}
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
					if (r->argcount == 2){
						struct stack_entry* y = stack_pop(&p->stack);
						b = FP_TO_INT(VALUE_NUM(y));
					}
					struct stack_entry* z = stack_pop(&p->stack);
					a = FP_TO_INT(VALUE_NUM(z));
					
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
				//TODO:ERR:MED error checking for strings here
				{
				u32 a;
				u32 b = ARR_DIM2_UNUSED;
				if (r->argcount == 2){
					struct stack_entry* y = stack_pop(&p->stack);
					b = FP_TO_INT(VALUE_NUM(y));
				}
				struct stack_entry* z = stack_pop(&p->stack);
				a = FP_TO_INT(VALUE_NUM(z));
				
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
				
				iprintf(" dim=%d,%d", (int)a, (int)b);
				}
				break;
			
			case BC_ARGCOUNT:
				r->argcount = data;
				
				iprintf("argc=%d", (int)r->argcount);
				break;
			
			case BC_BEGIN_LOOP:
				{
				// execute FOR condition check
				// this will always occur when the stack is prepared already
				// call_entry: Var ptr 
				// stack: end, [step]
				iprintf("\n");
				if (!p->calls.stack_i){
					p->exec.error = ERR_BEGIN_LOOP_FAIL;
					break;
				}
				s32* current = (s32*)p->calls.entry[p->calls.stack_i-1].var;
				s32 end;
				s32 step;
				if (p->stack.stack_i == 1){
					step = 1;
					end = STACK_REL_NUM(-1); //stack_pop(&p->stack)->value.number;
				} else {
					step = STACK_REL_NUM(-1); //stack_pop(&p->stack)->value.number; 
					end = STACK_REL_NUM(-2); //stack_pop(&p->stack)->value.number;
				}
				s32 val = *current;
				iprintf("val:%d end:%d step:%d\n", val, end, step);
				if ((step < 0 && end > val) || (step >= 0 && end < val)){
					// if val + step will never reach end, then skip to NEXT
					// treat valid NEXT as first in a line
					int nest = 1;
					// TODO:IMPL:HIGH
					// This loop doesn't work, I don't think?
					// It should scan for NEXT at line start
					idx index = r->index;
					while (nest > 0){
						do {
							index = bc_scan(p->exec.code, index, BC_OPERATOR);
						} while (index != BC_SCAN_NOT_FOUND && p->exec.code->data[index] != BC_COMMAND);
						if (index == BC_SCAN_NOT_FOUND){
							p->exec.error = ERR_FOR_WITHOUT_NEXT;
							break;
						}
						instr = p->exec.code->data[index];
						data = p->exec.code->data[index+1];
						if (instr == BC_COMMAND){
							if (data == CMD_NEXT){
								// TODO:IMPL:HIGH Check variable for a given NEXT to see if it counts or not
								// find NEXT, go back one instruction, execute statement if it is VAR?
								// Found a NEXT
								nest--;
							} else if (data == CMD_FOR){
								// Found a FOR, so we need to search for another NEXT
								nest++;
							}
						}
						index += 2; // advance past this command
					}
					// index now points to one past the NEXT, the correct location
				} else {
					// execution continues as normal into the loop
				}
				// clear the FOR arguments off the stack
				p->stack.stack_i = 0;
				}
				break;
				
			case BC_DATA:
			case BC_LABEL:
				// ignore these!
				r->index += data + (data & 1); // to next instruction
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
