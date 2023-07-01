#include "runner.h"
#include "system.h"

// TODO: Move these files to a subdirectory?
#include "console.h"
#include "operators.h"

#include "tokens.h"
#include "ptc.h"

#include <stdio.h>

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
			ptc_commands[(u32)data](p);
		} else if (instr == BC_OPERATOR){
			print_name(bc_conv_operations, data);
			
			if ((u8)data > sizeof(ptc_operators)/sizeof(ptc_operators[0])){
				return;
			}
			ptc_operators[(u32)data](p);
		} else if (instr == BC_FUNCTION){
			print_name(functions, data);
		} else if (instr == BC_NUMBER){
			s32 number = 0;
			
			number |= (char)code->data[index++] << 24;
			number |= (unsigned char)code->data[index++] << 16;
			number |= (unsigned char)code->data[index++] << 8;
			number |= (unsigned char)code->data[index++];
			
			p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_NUMBER, {number}};
			iprintf("num=%.12f", number / 4096.0);
		} else if (instr == BC_VARIABLE_NAME){
			iprintf("name=");
			//TODO: 
			enum types t;
			void* x;
			struct named_var* v;
			
			if (code->data[index+(u8)data-1] == '$'){
				t = VAR_STRING;
				v = get_var(&p->vars, &code->data[index], data, t);
				if (!v) return;
				x = v->value.ptr;
			} else {
				t = VAR_NUMBER;
				v = get_var(&p->vars, &code->data[index], data, t);
				if (!v) return;
				x = (void*)&v->value.number;
			}
			
			p->stack.entry[p->stack.stack_i++] = (struct stack_entry){VAR_VARIABLE | t, {.ptr = x}};
			for (size_t i = 0; i < (u32)data; ++i){
				iprintf("%c", code->data[index++]);
			}
			if (index % 2) index++;
			//debug
			stack_print(&p->stack);
		} else {
			iprintf("Unknown BC: %c %d", instr, data);
		}
		iprintf("\n");
	}
	/*
	for (u32 i = 0; i < p->stack.stack_i; ++i){
		iprintf("%d:%d\n", p->stack.entry[i].type, p->stack.entry[i].value.number);
	}
	*/
}
