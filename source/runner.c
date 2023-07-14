#include "runner.h"
#include "system.h"

// TODO: Move these files to a subdirectory?
#include "console.h"
#include "operators.h"

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

const ptc_call ptc_commands[] = {
	cmd_print,
	cmd_locate,
	cmd_color,
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

/// Debug function for checking command/function names from IDs
void print_name(const char* names, int data){
	char name[9] = {0};
	for (size_t i = 0; i < 8; ++i){
		name[i] = names[8*data + i];
	}
	iprintf("cmd=%s", name);
}

void run(struct program* code, struct ptc* p) {
	u32 index = 0;
	p->stack.stack_i = 0;
	u32 argcount = 0;
	
	while (index < code->size){
		// get one instruction and execute it
		char instr = code->data[index++];
		char data = code->data[index++];
		
		iprintf("%c ", instr);
		
		if (instr == BC_SMALL_NUMBER){
			iprintf("val=%d", data);
			if (data >= 0 && data <= 99){
				// push a SMALL_NUMBERS[data] to the stack
				p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {((u32)data) << 12}};
			}
		} else if (instr == BC_STRING){
			iprintf("len=%d ", data);
			// push string pointer to the stack
			p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_STRING, {.ptr = (void*)&code->data[index-2]}};
			for (size_t i = 0; i < (u32)data; ++i){
				iprintf("%c", code->data[index++]);
			}
			if (index % 2) index++;
		} else if (instr == BC_COMMAND){
			print_name(commands, data);
			//run command using current stack
			if ((u8)data >= sizeof(ptc_commands)/sizeof(ptc_commands[0])){
				iprintf("Error: Unimplemented command!\n");
				return;
			}
			ptc_commands[(u32)data](p);
		} else if (instr == BC_OPERATOR){
			print_name(bc_conv_operations, data);
			
			if ((u8)data >= sizeof(ptc_operators)/sizeof(ptc_operators[0])){
				iprintf("Error: Unimplemented operator!\n");
				return;
			}
			ptc_operators[(u32)data](p);
		} else if (instr == BC_FUNCTION){
			print_name(functions, data);
			
			/*if ((u8)data >= sizeof(ptc_functions)/sizeof(ptc_functions[0])){
				iprintf("Error: Unimplemented function!\n");
				return;
			}*/
		} else if (instr == BC_NUMBER){
			s32 number = 0;
			
			number |= (char)code->data[index++] << 24;
			number |= (unsigned char)code->data[index++] << 16;
			number |= (unsigned char)code->data[index++] << 8;
			number |= (unsigned char)code->data[index++];
			
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
				name = &code->data[index];
				len = code->data[index+(u8)data-1] == '$' ? data-1 : data;
				t = code->data[index+(u8)data-1] == '$' ? VAR_STRING : VAR_NUMBER; 
			} else {
				name = &data;
				len = 1;
				t = VAR_NUMBER;
			}
			
			if (!argcount){
				struct named_var* v = get_var(&p->vars, name, len, t);
				if (!v){
					iprintf("Error: Variable failed to allocate!\n");
					abort();
				}
				x = t & VAR_NUMBER ? (void*)&v->value.number : &v->value.ptr;
			} else {
				u32 a;
				u32 b = ARR_DIM2_UNUSED;
				if (argcount == 2){
					b = stack_pop(&p->stack)->value.number >> 12;
				}
				a = stack_pop(&p->stack)->value.number >> 12;
				
				union value* val = get_arr_entry(&p->vars, name, len, t | VAR_ARRAY, a, b);
				x = t & VAR_NUMBER ? (void*)&val->number : &val->ptr;
			}
			
			p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_VARIABLE | t, {.ptr = x}};
			if (data < 'A'){
				for (size_t i = 0; i < (u32)data; ++i){
					iprintf("%c", code->data[index++]);
				}
				if (index % 2) index++;
			} else {
				iprintf("%c", code->data[index-1]);
			}
			//debug
//			stack_print(&p->stack);
		} else if (instr == BC_DIM){
			//TODO: error checking for strings here
			u32 a;
			u32 b = ARR_DIM2_UNUSED;
			if (argcount == 2){
				b = stack_pop(&p->stack)->value.number >> 12;
			}
			a = stack_pop(&p->stack)->value.number >> 12;
			
			iprintf("name=");
			if (data >= 'A'){
				iprintf("%c ", data);
				get_new_arr_var(&p->vars, &data, 1, VAR_NUMBER | VAR_ARRAY, a, b);
			} else {
				for (size_t i = 0; i < (u32)data; ++i){
					iprintf("%c ", code->data[index++]);
				}
				if (index % 2) index++;
				
				enum types t = code->data[index+(u8)data-1] == '$' ? VAR_STRING : VAR_NUMBER;
				
				get_new_arr_var(&p->vars, &code->data[index], data, t | VAR_ARRAY, a, b);
			}
			
			iprintf("dim=%d,%d", a, b);
		} else if (instr == BC_ARGCOUNT){
			argcount = data;
			
			iprintf("argc=%d", argcount);
		} else {
			iprintf("Unknown BC: %c %d", instr, data);
		}
		if (instr != BC_ARGCOUNT){
			argcount = 0; // zero after use to avoid reading vars as arrays
		}
		
		iprintf("\n");
	}
	/*
	for (u32 i = 0; i < p->stack.stack_i; ++i){
		iprintf("%d:%d\n", p->stack.entry[i].type, p->stack.entry[i].value.number);
	}
	*/
}
