#include "runner.h"
#include "system.h"

#include "operators.h"
#include "sysvars.h"
#include "error.h"

#include "tokens.h"
#include "ptc.h"
#include "program.h"
#include "flow.h"
#include "data.h"
#include "input.h"

#include "mathptc.h"
#include "strfuncs.h"

#include "subsystem/console.h"
#include "subsystem/background.h"
#include "subsystem/panel.h"
#include "subsystem/resources.h"
#include "subsystem/sprites.h"
#include "subsystem/graphics.h"

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

void ptc_err(struct ptc* p){
	// The "I haven't done this yet" function
	ERROR(ERR_UNIMPLEMENTED);
}


DTCM_DATA const ptc_call ptc_commands[] = {
	cmd_print, cmd_locate, cmd_color, ptc_err, // dim
	cmd_for, cmd_to, cmd_step, cmd_next,
	cmd_if, cmd_then, cmd_else, cmd_endif,
	cmd_goto, cmd_gosub, cmd_on, cmd_return,
	cmd_end, cmd_stop,
	cmd_cls, cmd_visible, cmd_acls, cmd_vsync, cmd_wait,
	cmd_input, cmd_linput,
	ptc_err, ptc_stub, //BEEP
	cmd_bgclip, cmd_bgclr, cmd_bgcopy, cmd_bgfill, ptc_err, //BGMCLEAR 
	ptc_err, ptc_err, ptc_err, ptc_err, ptc_err, ptc_stub, ptc_err, //BGMVOL
	cmd_bgofs, cmd_bgpage, cmd_bgput, cmd_bgread, cmd_brepeat, cmd_chrinit, ptc_err, //CHRREAD
	ptc_err, cmd_clear, ptc_err, ptc_err, ptc_err, ptc_err, //CONT
	ptc_stub, ptc_err, cmd_dtread, cmd_exec, cmd_gbox, //GBOX
	ptc_err, cmd_gcls, cmd_gcolor, ptc_err, ptc_err, cmd_gfill, cmd_gline, // GLINE, 
	cmd_gpage, ptc_err, cmd_gpset, ptc_err, ptc_err, cmd_iconclr, cmd_iconset, //ICONSET, 
	ptc_err, ptc_err, ptc_err, ptc_err, //NEW, 
	cmd_pnlstr, cmd_pnltype, cmd_read, ptc_err, ptc_err, ptc_err, //RENAME, 
	cmd_restore, ptc_err, ptc_err, ptc_err, ptc_err, ptc_err, ptc_err, //SPANGLE, 
	ptc_err, ptc_err, cmd_spclr, ptc_err, ptc_err, ptc_err, cmd_spofs, cmd_sppage, //SPPAGE,
	ptc_err, ptc_err, cmd_spset, ptc_err, cmd_swap, //SWAP, 
	ptc_err //TMREAD,
};

DTCM_DATA const ptc_call ptc_operators[] = {
	op_add, op_comma, op_sub, op_mult, op_div, op_semi, op_assign, op_negate,
	op_equal, op_inequal, op_less, op_greater, op_less_equal, op_greater_equal,
	op_modulo,
	op_and, op_or, op_xor, op_not, op_logical_not
};

DTCM_DATA const ptc_call ptc_functions[] = {
	ptc_err, func_asc, ptc_err, ptc_err, ptc_err, ptc_err, func_btrig, func_button,
	ptc_err, func_chr, ptc_err, ptc_err, ptc_err, func_floor, ptc_err, ptc_err, func_iconchk, //FUNC_ICONCHK
	func_inkey, func_instr, func_left, func_len, func_log, func_mid, func_pi, func_pow, ptc_err, //FUNC_RAD
	func_right, func_rnd, ptc_err, func_sin, ptc_err, ptc_err, ptc_err, ptc_err, //FUNC_SPHITRC
	ptc_err, ptc_err, func_str, func_subst, ptc_err, func_val, //FUNC_VAL
};

DTCM_DATA const ptc_call ptc_sysvars[] = {
	sys_true, sys_false, sys_cancel, sys_version,
	ptc_err, sys_date, sys_maincntl, sys_maincnth, //MAINCNTH
	ptc_err, ptc_err, ptc_err, ptc_err, ptc_err, //RESULT
	sys_tchst, sys_tchx, sys_tchy, sys_tchtime, //TCHTIME
	sys_csrx, sys_csry, sys_tabstep,
};

/// Debug function for checking command/function names from IDs
static inline void print_name(const char* names, int data){
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
void run(struct bytecode code, struct ptc* p) {
	struct runner* r = &p->exec;
	r->index = 0;
	r->data_index = 0;
	r->data_offset = 0;
	r->code = code; // TODO:CODE:MED Since this is a value copy, don't use code anymore for consistency
	p->stack.stack_i = 0;
	
	while (r->index < r->code.size && !p->exec.error){
		// get one instruction and execute it
		start_time(&p->time);
		char instr = code.data[r->index++];
		char data = code.data[r->index++];
		
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
				check_time(&p->time, 0);
				break;
			
			case BC_LABEL_STRING:
			case BC_STRING:
				iprintf("len=%d ", data);
				iprintf("%.*s", data, &code.data[r->index]);
				// push string pointer to the stack
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_STRING, {.ptr = (void*)&code.data[r->index-2]}};
				r->index += data + (data & 1); // to next instruction
				check_time(&p->time, 1);
				break;
			
			case BC_COMMAND:
			case BC_COMMAND_FIRST:
				print_name(commands, data);
				//run command using current stack
				if ((u8)data >= sizeof(ptc_commands)/sizeof(ptc_commands[0])){
					r->error = ERR_PTC_COMMAND_INVALID;
					break;
				}
				if (ptc_commands[(u32)data]){
					ptc_commands[(u32)data](p);
//					iprintf("End command; %d?<%d %d ", r->index, r->code.size, (int)r->error);
					print_name(commands, data);
				} else {
					r->error = ERR_UNIMPLEMENTED;
					break;
				}
				// FOR,TO,STEP use stack for BC_BEGIN_LOOP so don't clear
				// ON ignores value to pass it to GOTO/GOSUB, so don't clear
				if (data != CMD_FOR && data != CMD_TO && data != CMD_STEP && data != CMD_ON)
					p->stack.stack_i=0; //clear stack after most commands
				check_time(&p->time, 2);
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
				check_time(&p->time, 3);
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
				check_time(&p->time, 4);
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
				check_time(&p->time, 5);
				break;
				
			case BC_NUMBER:
				{
				fixp number = 0;
				
				number |= (char)code.data[r->index++] << 24;
				number |= (unsigned char)code.data[r->index++] << 16;
				number |= (unsigned char)code.data[r->index++] << 8;
				number |= (unsigned char)code.data[r->index++];
				
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {number}};
				iprintf("num=%.12f", number / 4096.0);
				}
				check_time(&p->time, 6);
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
					name = (char*)&code.data[r->index];
					len = code.data[r->index+(u8)data-1] == '$' ? data-1 : data;
					t = code.data[r->index+(u8)data-1] == '$' ? VAR_STRING : VAR_NUMBER; 
				} else {
					name = &data;
					len = 1;
					t = VAR_NUMBER;
				}
				
				if (!r->argcount){
					struct named_var* v = get_var(&p->vars, name, len, t);
					if (!v){
						r->error = p->vars.error;
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
					if (!val){
						r->error = p->vars.error;
						break;
					}
					x = t & VAR_NUMBER ? (void*)&val->number : &val->ptr;
				}
				
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_VARIABLE | t, {.ptr = x}};
				if (data < 'A'){
					iprintf("%.*s", data, &code.data[r->index]);
					r->index += data + (data & 1); // to next instruction
				} else {
					iprintf("%c", code.data[r->index-1]);
				}
				//debug
	//			stack_print(&p->stack);
				check_time(&p->time, 7);
				break;
			
			case BC_DIM:
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
					r->error = p->vars.error;
				} else {
					char* x = (char*)&code.data[r->index];
					iprintf("%.*s", data, x);
					r->index += data + (data & 1); // to next instruction
					
					enum types t = x[(u8)data-1] == '$' ? VAR_STRING : VAR_NUMBER;
					u32 len = t & VAR_NUMBER ? data : data-1;
					
					get_new_arr_var(&p->vars, x, len, t | VAR_ARRAY, a, b);
					r->error = p->vars.error;
				}
				
				iprintf(" dim=%d,%d", (int)a, (int)b);
				}
				check_time(&p->time, 8);
				break;
			
			case BC_ARGCOUNT:
				r->argcount = data;
				
				iprintf("argc=%d", (int)r->argcount);
				check_time(&p->time, 9);
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
				fixp* current = (fixp*)p->calls.entry[p->calls.stack_i-1].var;
				fixp end;
				fixp step;
				if (p->stack.stack_i == 1){
					step = INT_TO_FP(1);
					end = STACK_REL_NUM(-1);
				} else {
					step = STACK_REL_NUM(-1);
					end = STACK_REL_NUM(-2);
				}
				fixp val = *current;
				iprintf("val:%d end:%d step:%d\n", val, end, step);
				if ((step < 0 && end > val) || (step >= 0 && end < val)){
					// if val + step will never reach end, then skip to NEXT
					// treat valid NEXT as first in a line
					int nest = 1;
					// Scan for NEXT only at start of line
					// TODO:TEST:MED Check behavior of cases with ::NEXT
					idx index = r->index;
					while (nest > 0){
						index = bc_scan(p->exec.code, index, BC_COMMAND_FIRST);
						if (index == BC_SCAN_NOT_FOUND){
							p->exec.error = ERR_FOR_WITHOUT_NEXT;
							break;
						}
						// Found BC_COMMAND; check if NEXT or FOR
//						instr = p->exec.code.data[index];
						data = p->exec.code.data[index+1];
						if (data == CMD_NEXT){
							// TODO:IMPL:HIGH Check variable for a given NEXT to see if it counts or not
							// find NEXT, go back one instruction, execute statement if it is VAR?
							// Found a NEXT
							nest--;
						} else if (data == CMD_FOR){
							// Found a FOR, so we need to search for another NEXT
							nest++;
						}
						index += 2; // advance past this command
					}
					// index now points to one past the NEXT, the correct location
					r->index = index;
				} else {
					// execution continues as normal into the loop
				}
				// clear the FOR arguments off the stack
				p->stack.stack_i = 0;
				}
				check_time(&p->time, 10);
				break;
				
			case BC_DATA:
			case BC_LABEL:
				// ignore these!
				r->index += data + (data & 1); // to next instruction
				check_time(&p->time, 11);
				break;
				
			default:
				iprintf("Unknown BC: %c %d", instr, data);
				r->error = ERR_INVALID_BC;
				check_time(&p->time, 12);
				break;
		}
		
		if (instr != BC_ARGCOUNT){
			r->argcount = 0; // zero after use to avoid reading vars as arrays
		}
		
		iprintf("\n");
	}
	
//	iprintf("%d\n", r->index);
	/*
	for (u32 i = 0; i < p->stack.stack_i; ++i){
		iprintf("%d:%d\n", p->stack.entry[i].type, p->stack.entry[i].value.number);
	}
	*/
}

void cmd_exec(struct ptc* p){
	// EXEC filename
	char filename_buf[257];
	void* filename = value_str(ARG(0));
	str_char_copy(filename, (u8*)filename_buf);
	filename_buf[str_len(filename)] = '\0';
	
	// Load program into memory and tokenize into bc
	// TODO:CODE:LOW This doesn't /look/ safe, but it should be.
	// The bytecode is designed to always compile smaller or equal than the program
	// source code in UCS2 (u16 chars). This shouldn't ever destroy the code
	// being currently tokenized.
	// TODO:TEST:MED This absolutely needs some test cases
	iprintf("\nold=%d ", p->exec.code.size);
	struct program prog = { 0, (char*)&p->exec.code.data[524288]};
	if (!load_prg(&prog, filename_buf)){
		// Failed to load, potentially destroyed code
		ERROR(ERR_FILE_LOAD_FAILED);
	}
	
	// tokenize updates all relevant exec.code values
	p->exec.error = tokenize(&prog, &p->exec.code);
	iprintf("new=%d\n", p->exec.code.size);
	// error set if needed; otherwise, execute
	// EXEC is called from run, so running happens naturally. Just reset the necessary values.
	p->exec.index = 0; // beginning of program
	p->exec.data_index = 0;
	p->exec.data_offset = 0;
	p->exec.argcount = 0;
}
