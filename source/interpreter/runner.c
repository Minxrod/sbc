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
#include "vars.h"

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

void ptc_func_stub(struct ptc* p){
	// Stub function consumes arguments provided and returns zero
	p->stack.stack_i -= p->exec.argcount;
	STACK_RETURN_INT(0);
}

void ptc_err(struct ptc* p){
	// The "I haven't done this yet" function
	ERROR(ERR_UNIMPLEMENTED);
}

const ptc_call ptc_bgmstub = ptc_stub;

DTCM_DATA const ptc_call ptc_commands[] = {
	cmd_print, cmd_locate, cmd_color, ptc_err, // dim
	cmd_for, cmd_to, cmd_step, cmd_next,
	cmd_if, cmd_then, cmd_else, cmd_endif,
	cmd_goto, cmd_gosub, cmd_on, cmd_return,
	cmd_end, cmd_stop,
	cmd_cls, cmd_visible, cmd_acls, cmd_vsync, cmd_wait,
	cmd_input, cmd_linput,
	cmd_append, ptc_stub, //BEEP
	cmd_bgclip, cmd_bgclr, cmd_bgcopy, cmd_bgfill, ptc_bgmstub, //BGMCLEAR 
	ptc_bgmstub, ptc_bgmstub, ptc_bgmstub, ptc_bgmstub, ptc_bgmstub, ptc_bgmstub, ptc_bgmstub, //BGMVOL
	cmd_bgofs, cmd_bgpage, cmd_bgput, cmd_bgread, cmd_brepeat, cmd_chrinit, cmd_chrread, //CHRREAD
	cmd_chrset, cmd_clear, cmd_colinit, cmd_colread, cmd_colset, ptc_err, //CONT
	ptc_stub, ptc_err, cmd_dtread, cmd_exec, cmd_gbox, //GBOX
	cmd_gcircle, cmd_gcls, cmd_gcolor, cmd_gcopy, cmd_gdrawmd, cmd_gfill, cmd_gline, // GLINE, 
	cmd_gpage, cmd_gpaint, cmd_gpset, cmd_gprio, cmd_gputchr, cmd_iconclr, cmd_iconset, //ICONSET, 
	cmd_key, ptc_err, cmd_load, cmd_new, //NEW, 
	cmd_pnlstr, cmd_pnltype, cmd_read, ptc_err, ptc_err, ptc_err, //RENAME, 
	cmd_restore, cmd_rsort, cmd_run, cmd_save, ptc_err, cmd_sort, cmd_spangle, //SPANGLE, 
	cmd_spanim, cmd_spchr, cmd_spclr, cmd_spcol, ptc_err, cmd_sphome, cmd_spofs, cmd_sppage, //SPPAGE,
	cmd_spread, cmd_spscale, cmd_spset, cmd_spsetv, cmd_swap, //SWAP, 
	cmd_tmread, //TMREAD,
	ptc_stub, ptc_stub, // TALKSTOP
	cmd_poke, cmd_pokeh, cmd_pokeb, cmd_memcopy, cmd_memfill
};

DTCM_DATA const ptc_call ptc_operators[] = {
	op_add, op_comma, op_sub, op_mult, op_div, op_semi, op_assign, op_negate,
	op_equal, op_inequal, op_less, op_greater, op_less_equal, op_greater_equal,
	op_modulo,
	op_and, op_or, op_xor, op_not, op_logical_not
};

DTCM_DATA const ptc_call ptc_functions[] = {
	func_abs, func_asc, func_atan, func_bgchk, ptc_func_stub, ptc_err, func_btrig, func_button,
	func_chkchr, func_chr, func_cos, func_deg, func_exp, func_floor, func_gspoit, func_hex, func_iconchk, //FUNC_ICONCHK
	func_inkey, func_instr, func_left, func_len, func_log, func_mid, func_pi, func_pow, func_rad, //FUNC_RAD
	func_right, func_rnd, func_sgn, func_sin, func_spchk, func_spgetv, func_sphit, ptc_err, //FUNC_SPHITRC
	func_sphitsp, func_sqr, func_str, func_subst, func_tan, func_val, //FUNC_VAL
	ptc_func_stub, // FUNC_TALKCHK
	func_peek, func_peekh, func_peekb, func_addr, func_ptr, // FUNC_PTR
};

DTCM_DATA const ptc_call ptc_sysvars[] = {
	sys_true, sys_false, sys_cancel, sys_version,
	sys_time, sys_date, sys_maincntl, sys_maincnth, //MAINCNTH
	sys_freevar, sys_freemem, ptc_err, ptc_err, sys_result, //RESULT
	sys_tchst, sys_tchx, sys_tchy, sys_tchtime, //TCHTIME
	sys_csrx, sys_csry, sys_tabstep,
	sys_sphitno, ptc_err, ptc_err, ptc_err,
	sys_keyboard, sys_funcno,
	ptc_err, ptc_err, ptc_err, // ICONPMAX
	ptc_err, ptc_err, // ERR
	sys_mem,
	sys_memsafe
};

DTCM_DATA const ptc_call ptc_sysvars_valid[] = {
	NULL, NULL, NULL, NULL, // VERSION
	NULL, NULL, NULL, NULL, // MAINCNTH
	NULL, NULL, NULL, NULL, NULL, // RESULT
	NULL, NULL, NULL, NULL, // TCHTIME
	NULL, NULL, syschk_tabstep,
	NULL, NULL, NULL, NULL, // SPHITT
	NULL, NULL, NULL, NULL, NULL, // ICONPMAX
	NULL, NULL, // ERR
	syschk_mem,
	syschk_memsafe,
};


/// Debug function for checking command/function names from IDs
static inline void print_name(const char* names, int data){
	char name[9] = {0};
	for (size_t i = 0; i < 8; ++i){
		name[i] = names[8*data + i];
	}
	iprintf("cmd=%s", name);
}

struct var_name {
	char* name;
	int len;
	int type;
};

struct var_name read_var_name(struct bytecode* b, idx index){
	struct var_name result;
	u8 instr = b->data[index];
	assert(instr == BC_VARIABLE_NAME || instr == BC_ARRAY_NAME || instr == BC_DIM);
	u8 data = b->data[index+1];
	if (data < 'A'){ // data byte contains length of name
		result.name = (char*)b->data + index + 2;
		bool is_str = b->data[index + data - 1 + 2] == '$';
		result.len = is_str ? data-1 : data;
		result.type = is_str ? VAR_STRING : VAR_NUMBER;
	} else { // data byte contains name (single letter optimization)
		result.name = (char*)b->data + index + 1;
		result.len = 1;
		result.type = VAR_NUMBER;
	}
	return result;
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
	r->code = code;
	p->stack.stack_i = 0;
	
	while (r->index < r->code.size && !p->exec.error){
		// get one instruction and execute it
		start_time(&p->time);
		char instr = r->code.data[r->index++];
		char data = r->code.data[r->index++];
		
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
				iprintf("%.*s", data, &r->code.data[r->index]);
				// push string pointer to the stack
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_STRING, {.ptr = (void*)&r->code.data[r->index-2]}};
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
				
			case BC_SYSVAR_VALIDATE:
				print_name(sysvars, data);
				if ((u8)data >= sizeof(ptc_sysvars_valid)/sizeof(ptc_sysvars_valid[0])){
					r->error = ERR_PTC_SYSVAR_INVALID;
					break;
				}
				if (ptc_sysvars_valid[(u32)data]){
					ptc_sysvars_valid[(u32)data](p);
				} else {
					r->error = ERR_UNIMPLEMENTED;
				}
				break;
				
			case BC_NUMBER:
				{
				fixp number = 0;
				
				number |= (fixp)((unsigned char)r->code.data[r->index++] << 24);
				number |= (unsigned char)r->code.data[r->index++] << 16;
				number |= (unsigned char)r->code.data[r->index++] << 8;
				number |= (unsigned char)r->code.data[r->index++];
				
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {number}};
				iprintf("num=%.12f", number / 4096.0);
				}
				check_time(&p->time, 6);
				break;
				
			case BC_VARIABLE_ID_SMALL:;
				int id; // Note: Can also be set via BC_VARIABLE_ID
				id = (u8)data;
			bc_variable_id_shared:
				{
				iprintf("id=%d ", (u8)id);
				struct named_var* v = &p->vars.vars[id];
				iprintf("dtype=%d", v->type);
				if (!r->argcount){
					// values
					p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_VARIABLE | v->type, {.ptr = &v->value}};
				} else {
					// array element
					idx a = (r->argcount == 2) ? STACK_REL_INT(-2) : STACK_REL_INT(-1);
					idx b = (r->argcount == 2) ? (idx)STACK_REL_INT(-1) : ARR_DIM2_UNUSED;
					p->stack.stack_i -= r->argcount;
					
					union value* entry = get_arr_entry_via_ptr(&p->vars, &v->value, a, b, v->type);
					if (!entry){
						r->error = p->vars.error;
						break;
					}
					// Note: must clear array bit when given only an entry
					p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_VARIABLE | (v->type & ~VAR_ARRAY), {.ptr = entry}};
				}
				}
				break;
				
			case BC_VARIABLE_ID:
				{
				// TODO:TEST:MED Check that ID is calculated correctly
				// Note: id variable declared in BC_VARIABLE_ID_SMALL
				id = (u8)data;
				id |= (u8)r->code.data[r->index++] << 8;
				id |= (u8)r->code.data[r->index++] << 16;
				goto bc_variable_id_shared;
				}
				break;
				
			case BC_ARRAY_NAME:;
				enum types t = VAR_ARRAY; // type of variable
				void* x;                  // pointer to variable value
//				char* name;               // variable struct
//				uint_fast8_t len;         // length of variable name
				goto name_shared;
				
			case BC_VARIABLE_NAME:
				iprintf("name=");
				// this is both array accesses and regular vars
				// difference determined by whether or not ARGCOUNT was set
				// before reading variable
				t = 0;
			name_shared:;
				struct var_name var = read_var_name(&r->code, r->index-2);
				var.type |= t;
				
				if (!r->argcount){
					struct named_var* v = get_var(&p->vars, var.name, var.len, var.type);
					if (!v){
						r->error = p->vars.error;
						break;
					}
					x = &v->value;
				} else {
					idx a = (r->argcount == 2) ? STACK_REL_INT(-2) : STACK_REL_INT(-1);
					idx b = (r->argcount == 2) ? (idx)STACK_REL_INT(-1) : ARR_DIM2_UNUSED;
					p->stack.stack_i -= r->argcount;
					
					union value* val = get_arr_entry(&p->vars, var.name, var.len, var.type | VAR_ARRAY, a, b);
					if (!val){
						r->error = p->vars.error;
						break;
					}
					x = val;
				}
				
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_VARIABLE | var.type, {.ptr = x}};
				// debug printing
				if (data < 'A'){
					iprintf("%.*s", data, &r->code.data[r->index]);
					r->index += data + (data & 1); // to next instruction
				} else {
					iprintf("%c", r->code.data[r->index-1]);
				}
				//debug
				check_time(&p->time, 7);
				break;
			
			case BC_DIM:
				{
				idx a = (r->argcount == 2) ? STACK_REL_INT(-2) : STACK_REL_INT(-1);
				idx b = (r->argcount == 2) ? (idx)STACK_REL_INT(-1) : ARR_DIM2_UNUSED;
				p->stack.stack_i -= r->argcount;
				
				iprintf("name=");
				if (data >= 'A'){
					iprintf("%c ", data);
					get_new_arr_var(&p->vars, &data, 1, VAR_NUMBER | VAR_ARRAY, a, b);
					r->error = p->vars.error;
				} else {
					char* x = (char*)&r->code.data[r->index];
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
				} else if (!p->stack.stack_i){
					p->exec.error = ERR_BEGIN_LOOP_NO_END;
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
//					idx index = r->index;
					struct idx_pair result = {0, r->index};
					while (nest > 0){
						result = bc_scan_pair(r->code, result.index, BC_COMMAND_FIRST);
						if (result.index == BC_SCAN_NOT_FOUND){
							p->exec.error = ERR_FOR_WITHOUT_NEXT;
							break;
						}
						// Found BC_COMMAND; check if NEXT or FOR
//						instr = p->exec.code.data[index];
						data = r->code.data[result.index+1];
						if (data == CMD_NEXT){
							iprintf("prev=%d[%s] index=%d[%s]\n", result.prev, r->code.data + result.prev, result.index, r->code.data + result.index);
							if (result.prev == BC_SCAN_NOT_FOUND){
								// Found generic NEXT
								nest--;
							} else {
								// Specific NEXT potientially found
								u8 var_check = r->code.data[result.prev];
								// Check for variable for a given NEXT to see if it counts or not
								if (var_check == BC_VARIABLE_NAME || var_check == BC_VARIABLE_ID || var_check == BC_VARIABLE_ID_SMALL){
									// have variable: check that it matches stored NEXT variable
									struct named_var* v;
									if (var_check == BC_VARIABLE_NAME){
										// get var by name
										struct var_name var = read_var_name(&r->code, result.prev);
										v = get_var(&p->vars, var.name, var.len, var.type);
									} else if (var_check == BC_VARIABLE_ID_SMALL){
										// get var by ID
										v = &p->vars.vars[r->code.data[result.prev + 1]];
									} else {
										// larger var ID
										v = &p->vars.vars[r->code.data[result.prev + 1] | (r->code.data[result.prev + 2] << 8) | (r->code.data[result.prev + 3] << 16)];
									}
									// verify variable matches
									if (current == &v->value.number){
										result.index += 2; // past NEXT [var]
										break; // ignore nesting?
									}
									// ignore otherwise? unclear
								} else {
									// treat as generic next
									nest--;
								}
							}
						} else if (data == CMD_FOR){
							// Found a FOR, so we need to search for another NEXT
							nest++;
						}
						result.index += 2; // advance past this command
					}
					// index now points to one past the NEXT, the correct location
					r->index = result.index;
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
				iprintf("len=%d ", data);
				for (int i = 0; i < (int)data; ++i){
					char c = r->code.data[r->index + i];
					iprintf("%c", c ? c : ',');
				}
				
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
	iprintf("\nold=%d ", p->exec.code.size);
//	struct program prog = { 0, (char*)&p->exec.code.data[524288] };
	if (!(p->exec.prg.size = check_load_res((u8*)p->exec.prg.data, p->res.search_path, filename_buf, TYPE_PRG))){
		// Failed to load, potentially destroyed code
		// TODO:IMPL:LOW Set RESULT here instead
		ERROR(ERR_FILE_LOAD_FAILED);
	}
	
	// tokenize updates all relevant exec.code values
	p->exec.error = tokenize(&p->exec.prg, &p->exec.code);
	iprintf("new=%d\n", p->exec.code.size);
	// error set if needed; otherwise, execute
	// EXEC is called from run, so running happens naturally. Just reset the necessary values.
	p->exec.index = 0; // beginning of program
	p->exec.data_index = 0;
	p->exec.data_offset = 0;
	p->exec.argcount = 0;
//	p->exec.prg = prog;
}

void cmd_run(struct ptc* p){
	// tokenize updates all relevant exec.code values
	p->exec.error = tokenize_full(&p->exec.prg, &p->exec.code, p, TOKOPT_NONE);
	iprintf("new=%d\n", p->exec.code.size);
	// error set if needed; otherwise, execute
	// EXEC is called from run, so running happens naturally. Just reset the necessary values.
	p->exec.index = 0; // beginning of program
	p->exec.data_index = 0;
	p->exec.data_offset = 0;
	p->exec.argcount = 0;
}
