#include "data.h"

#include "common.h"

#include "interpreter/strs.h"
#include "interpreter/tokens.h"
#include "system.h"
#include "program.h"
#include "error.h"
#include "ptc.h"

// Returns number of characters read OR READ_ONE_ERR if an error occurs
int read_one_u8(struct ptc* p, const u8* src, size_t len, struct stack_entry* dest){
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
		value_str(dest);
		
		// create destination and copy characters
		for (; i < len && src[i] != BC_DATA_DELIM; ++i){
			s->ptr.s[i - start] = src[i];
		}
		s->len = i - start;
		
		// Assign new string to variable
		(*(struct string**)dest->value.ptr) = s;
		s->uses = 1;
	} else if (dest->type == (VAR_VARIABLE | VAR_NUMBER)){
		u8 conversion[16] = {0}; // this buffer size is very arbitrary
		
		// Read number!
		for (; i < len && i - start < 16; ++i){
			u8 c = src[i];
			if (is_number(c) || c == '-' || c == '.'){
				conversion[i - start] = c;
			} else if (c == BC_DATA_DELIM){ 
				break; // stop reading here
			} else {
				return READ_ONE_ERR;
			}
			iprintf(":%c", conversion[i - start]);
		}
		// Convert!
		*((fixp*)dest->value.ptr) = u8_to_num(conversion, i - start);
	} else {
		return READ_ONE_ERR;
	}
	return i - start;
}

// Finds and sets the data index to the next available slot
void find_data(struct ptc* p){
	assert(p->exec.code.data[p->exec.data_index] == BC_DATA); // assumes index already points to valid data
	int ofs = p->exec.data_offset;
	// Note: Instruction is of form BC_DATA [length] [`length` characters] [null if length % 2 is odd]
	// To skip past this
	p->exec.data_index = bc_scan(p->exec.code, p->exec.data_index + 2 + ofs + (ofs & 1), BC_DATA);
	p->exec.data_offset = 0;
}

/// Offset into data block where data actually starts
#define DATA_DATA_OFS 2

void cmd_read(struct ptc* p){
	// Get variables from stack and call read() into each of them (using DATA string as source)
	// Get pointer, offset for data
	if (p->exec.data_index == BC_SCAN_NOT_FOUND){
		ERROR(ERR_OUT_OF_DATA);
	}
	u8* data_block = (u8*)&p->exec.code.data[p->exec.data_index]; //points to instruct that can be BC_DATA
//	u8 block_size = data_block[1];
	u8* data_src;
	u8 block_size;
	
	if (data_block[0] != BC_DATA){
		p->exec.data_offset = 0;
		p->exec.data_index = bc_scan(p->exec.code, p->exec.data_index, BC_DATA);
		if (p->exec.data_index == BC_SCAN_NOT_FOUND){
			ERROR(ERR_OUT_OF_DATA);
		}
		data_block = (u8*)&p->exec.code.data[p->exec.data_index];
	}
	
	iprintf("READ from: ");
	// Iterate over variables on stack
	for (int i = 0; i < p->stack.stack_i; ++i){
		block_size = data_block[1];
		iprintf("src=%d size=%d", p->exec.data_index, block_size);
		iprintf(" data=\"%.*s\"", block_size, &data_block[DATA_DATA_OFS + p->exec.data_offset]);
		data_src = &data_block[DATA_DATA_OFS + p->exec.data_offset];
		
		int read = read_one_u8(p, data_src, block_size - p->exec.data_offset, ARG(i));
		if (read == READ_ONE_ERR){
			ERROR(ERR_READ_FAILURE);
		}
		p->exec.data_offset += read;
		iprintf(" read:%d", p->exec.data_offset);
		if (data_block[2 + p->exec.data_offset] == BC_DATA_DELIM){
			p->exec.data_offset++;
		}
		iprintf(" pos:%d\n", p->exec.data_offset);
		if (p->exec.data_offset >= block_size){
			// search for next data block
			find_data(p);
			data_block = (u8*)&p->exec.code.data[p->exec.data_index];
		}
	}
	iprintf("\n");
}

void cmd_restore(struct ptc* p){
	void* label = value_str(ARG(0));
	idx index = search_label(p, label); // sets error on failure
	p->exec.data_index = bc_scan(p->exec.code, index, BC_DATA);
	p->exec.data_offset = 0;
	iprintf("Data at %d: %.*s", index, p->exec.code.data[index+1], &p->exec.code.data[index+2]);
}
