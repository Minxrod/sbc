#include "data.h"

#include "interpreter/strs.h"
#include "interpreter/tokens.h"
#include "system.h"
#include "program.h"
#include "error.h"
#include "ptc.h"

// Returns number of characters read OR READ_ONE_ERR if an error occurs
int read_one_u8(struct ptc* p, u8* src, size_t len, struct stack_entry* dest){
	// skip leading spaces
	size_t i = 0;
	while (i < len && src[i] == ' '){
		i++;
	}
	if (i >= len){
		return READ_ONE_ERR;
	}
	size_t start = i;
	if (dest->type == (VAR_VARIABLE | VAR_STRING)){
		// Create new string
		struct string* s = get_new_str(&p->strs);
		// Remove a use if variable had a string aleady
		if (*(void**)dest->value.ptr != NULL && **(char**)dest->value.ptr == STRING_CHAR){
			(*(struct string**)dest->value.ptr)->uses--;
		}
		
		// create destination and copy characters
		for (; i < len && src[i] != ','; ++i){
			s->ptr.s[i - start] = src[i];
		}
		s->len = i - start;
		
		// Assign new string to variable
		(*(struct string**)dest->value.ptr) = s;
		s->uses = 1;
	} else if (dest->type == (VAR_VARIABLE | VAR_NUMBER)){
		u8 conversion[16]; // this buffer size is very arbitrary
		bool neg = false;
		if (src[i] == '-'){
			neg = true;
			i++;
		}
		
		// Read number!
		for (; i < len && i - start < 16; ++i){
			u8 c = src[i];
			if (is_number(c) || c == '-'){
				conversion[i - start] = c;
			} else if (c == ','){ 
				break; // stop reading here
			} else {
				return READ_ONE_ERR;
			}
		}
		// Convert!
		// Note; This only works with the off-by-one error of '-' cases 
		// because str_to_num ignores invalid characters entirely
		*((fixp*)dest->value.ptr) = u8_to_num(conversion, i - start) * (neg ? -1 : 1);
	} else {
		return READ_ONE_ERR;
	}
	return i - start;
}

// Finds and sets the data index to the next available slot
void find_data(struct ptc* p){
	p->exec.data_index = bc_scan(p->exec.code, p->exec.data_index, BC_DATA);
	if (p->exec.data_index == BC_SCAN_NOT_FOUND){
		ERROR(ERR_OUT_OF_DATA);
	} else {
		p->exec.data_offset = 0;
	}
}

void cmd_read(struct ptc* p){
	// Get variables from stack and call read() into each of them (using DATA string as source)
	// Get pointer, offset for data
	// TODO:CODE:LOW Constant for 2
	u8* data_block = (u8*)&p->exec.code->data[p->exec.data_index]; //points to BC_DATA
	u8 block_size = data_block[1];
	u8* data_src;
	
	if (data_block[0] != BC_DATA){
		find_data(p);
		data_block = (u8*)&p->exec.code->data[p->exec.data_index];
		if (p->exec.error) return; // ran out of DATA
	}
	
	iprintf("READ from: src=%d size=%d\n", p->exec.data_index, block_size);
	// Iterate over variables on stack
	for (int i = 0; i < p->stack.stack_i; ++i){
		data_src = &data_block[2 + p->exec.data_offset];
		
		int read = read_one_u8(p, data_src, block_size - p->exec.data_offset, ARG(i));
		if (read == READ_ONE_ERR){
			ERROR(ERR_READ_FAILURE); //TODO:ERR:LOW Return better error if the type was not assignable
		}
		p->exec.data_offset += read;
		if (data_block[2 + p->exec.data_offset] == ','){
			p->exec.data_offset++;
		}
		if (p->exec.data_offset >= block_size){
			// search for next data block
			find_data(p);
			data_block = (u8*)&p->exec.code->data[p->exec.data_index];
			if (p->exec.error) return; // ran out of DATA
		}
	}
}

void cmd_restore(struct ptc* p){
	(void)p;
}
