#include "runner.h"
#include "system.h"

// TODO: Move these files to a subdirectory?
#include "console.h"
#include "operators.h"
#include "error.h"

#include "tokens.h"
#include "ptc.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>

//0-9
const s32 SMALL_NUMBERS[] = {
	0, 4096, 8192, 12288, 16384, 20480, 24576, 28672, 32768, 36864
};

typedef void(*ptc_call)(struct ptc*);

void cmd_for(struct ptc* p){
	// current stack consists of one var ptr
	struct stack_entry* e = &p->stack.entry[0];
	
	p->calls.entry[p->calls.stack_i].type = CALL_FOR;
	// Note: skips instruction OP_ASSIGN
	p->calls.entry[p->calls.stack_i].address = p->exec.index + 2;
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

void cmd_next(struct ptc* p){
	p++;
	// get call stack top
//	u32 top = p->calls.stack_i-1;
//	struct call_entry* e = &p->calls.entry[top];
	
	// check if need to loop
}


const ptc_call ptc_commands[] = {
	cmd_print,
	cmd_locate,
	cmd_color,
	NULL, // dim
	cmd_for,
	cmd_to,
	cmd_step,
	cmd_next,
	
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
	p->stack.stack_i = 0;
	
	while (r->index < code->size && !p->exec.error){
		// get one instruction and execute it
		char instr = code->data[r->index++];
		char data = code->data[r->index++];
		
		iprintf("%c ", instr);
		
		if (instr == BC_SMALL_NUMBER){
			iprintf("val=%d", data);
			
			if (data >= 0 && data <= 99){
				// push a SMALL_NUMBERS[data] to the stack
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {((u32)data) << 12}};
			} else {
				r->error = ERR_NUM_INVALID;
			}
		} else if (instr == BC_STRING){
			iprintf("len=%d ", data);
			iprintf("%.*s", data, &code->data[r->index]);
			// push string pointer to the stack
			p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_STRING, {.ptr = (void*)&code->data[r->index-2]}};
			r->index += data + (data & 1); // to next instruction
		} else if (instr == BC_COMMAND){
			print_name(commands, data);
			//run command using current stack
			if ((u8)data >= sizeof(ptc_commands)/sizeof(ptc_commands[0])){
				r->error = ERR_PTC_COMMAND_INVALID;
			}
			ptc_commands[(u32)data](p);
		} else if (instr == BC_OPERATOR){
			print_name(bc_conv_operations, data);
			
			if ((u8)data >= sizeof(ptc_operators)/sizeof(ptc_operators[0])){
				r->error = ERR_PTC_OPERATOR_INVALID;
			}
			ptc_operators[(u32)data](p);
		} else if (instr == BC_FUNCTION){
			print_name(functions, data);
			
			if ((u8)data >= sizeof(ptc_functions)/sizeof(ptc_functions[0])){
				r->error = ERR_PTC_FUNCTION_INVALID;
			}
		} else if (instr == BC_NUMBER){
			s32 number = 0;
			
			number |= (char)code->data[r->index++] << 24;
			number |= (unsigned char)code->data[r->index++] << 16;
			number |= (unsigned char)code->data[r->index++] << 8;
			number |= (unsigned char)code->data[r->index++];
			
			p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {number}};
			iprintf("num=%.12f", number / 4096.0);
		} else if (instr == BC_VARIABLE_NAME){
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
					iprintf("Error: Variable failed to allocate!\n");
					abort();
				}
				x = t & VAR_NUMBER ? (void*)&v->value.number : &v->value.ptr;
			} else {
				u32 a;
				u32 b = ARR_DIM2_UNUSED;
				// TODO: check for strings here before reading
				if (r->argcount == 2){
					b = stack_pop(&p->stack)->value.number >> 12;
				}
				a = stack_pop(&p->stack)->value.number >> 12;
				
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
		} else if (instr == BC_DIM){
			//TODO: error checking for strings here
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
		} else if (instr == BC_ARGCOUNT){
			r->argcount = data;
			
			iprintf("argc=%d", r->argcount);
		} else if (instr == BC_BEGIN_LOOP){
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
				
			} else {
				// execution continues as normal
			}
		} else {
			iprintf("Unknown BC: %c %d", instr, data);
			r->error = ERR_INVALID_BC;
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
