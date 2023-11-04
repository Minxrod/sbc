#include "tokens.h"

#include "ptc.h"
#include "program.h"
#include "strs.h" // for character id functions

#include <stdio.h>
#include <stdint.h>
#include <string.h>

const int MAX_SPECIAL_NAME_SIZE = 8;

const char* commands =
"PRINT   LOCATE  COLOR   DIM     FOR     TO      STEP    NEXT    "
"IF      THEN    ELSE    [ENDIF] GOTO    GOSUB   ON      RETURN  "
"END     STOP    "
"CLS     VISIBLE ACLS    VSYNC   WAIT    "
"INPUT   LINPUT  "
"APPEND  "
"BEEP    BGCLIP  BGCLR   BGCOPY  BGFILL  BGMCLEARBGMPLAY BGMPRG  BGMSET  BGMSETD BGMSETV "
"BGMSTOP BGMVOL  BGOFS   BGPAGE  BGPUT   BGREAD  BREPEAT CHRINIT CHRREAD CHRSET  CLEAR   "
"COLINIT COLREAD COLSET  CONT    DATA    DELETE  DTREAD  "
"EXEC    GBOX    GCIRCLE GCLS    GCOLOR  GCOPY   GDRAWMD GFILL   GLINE   "
"GPAGE   GPAINT  GPSET   GPRIO   GPUTCHR ICONCLR ICONSET "
"KEY     LIST    LOAD    NEW     PNLSTR  PNLTYPE "
"READ    REBOOT  RECVFILERENAME  RESTORE RSORT   RUN     SAVE    SENDFILESORT    "
"SPANGLE SPANIM  SPCHR   SPCLR   SPCOL   SPCOLVECSPHOME  SPOFS   SPPAGE  SPREAD  SPSCALE "
"SPSET   SPSETV  SWAP    TMREAD  ";

const char* functions =
"ABS     ASC     ATAN    BGCHK   BGMCHK  BGMGETV BTRIG   BUTTON  CHKCHR  CHR$    COS     "
"DEG     EXP     FLOOR   GSPOIT  HEX$    ICONCHK INKEY$  INSTR   LEFT$   LEN     LOG     "
"MID$    PI      POW     RAD     RIGHT$  RND     SGN     SIN     SPCHK   SPGETV  SPHIT   "
"SPHITRC SPHITSP SQR     STR$    SUBST$  TAN     VAL     ";

const char* operations =
"AND     NOT     OR      XOR     ";

const char* bc_conv_operations = 
"+       ,       -       *       /       ;       =       (-)     "
"==      !=      <       >       <=      >=      "
"%       "
"AND     OR      NOT     XOR     !       "
"(       )       [       ]       ";

const char* sysvars = 
"TRUE    FALSE   CANCEL  VERSION "
"TIME$   DATE$   MAINCTNLMAINCNTH"
"FREEVAR FREEMEM PRGNAME$PACKAGE$RESULT  "
"TCHST   TCHX    TCHY    TCHTIME "
"CSRX    CSRY    TABSTEP "
"SPHITNO SPHITX  SPHITY  SPHITT  "
"KEYBOARDFUNCNO  ICONPUSEICONPAGEICONPMAX"
"ERL     ERR     "
"MEM     ";

const char* labels = "LABEL   ";

const char* comments = "REM     ";

// Scans for a specific instruction. Special purpose function to handle the
// various instruction lengths.
idx bc_scan(struct program* code, idx index, u8 find){
	// search for find in code->data
	while (index < code->size){ 
		u8 cur = code->data[index];
		if (cur == find){
			return index;
		}
		//debug! 
		u8 len = code->data[++index];
//		iprintf("%d: %c,%d %.*s\n", (int)index-1, cur >= 32 ? cur : '?', len, len, &code->data[index+1]);
		if (cur == BC_STRING || cur == BC_LABEL || cur == BC_LABEL_STRING || cur == BC_DATA){
			index++;
			index += len + (len & 1);
		} else if (cur == BC_WIDE_STRING){
			index++;
			index += sizeof(u16) * len;
		} else if (cur == BC_DIM || cur == BC_VARIABLE_NAME){
			cur = len;
			++index;
			if (cur >= 'A'){
			} else {
				index += len + (len & 1);
			}
		} else if (cur == BC_NUMBER){
			index += 6-1; // change if number syntax gets modified?
		} else {
			index += 2-1;
		}
	}
	return BC_SCAN_NOT_FOUND;
}


// Note: Expects null-terminated string
int tok_in_str_index(const char* str, const char* data, struct token* tok){
	int index;
	size_t start = 0;
	do {
		index = start / MAX_SPECIAL_NAME_SIZE;
		for (size_t i = 0; i < tok->len; ++i){
			if (data[tok->ofs + i] != *(str + i)){
				index = -1;
				break;
			}
		}
		if (index < 0){
			str += MAX_SPECIAL_NAME_SIZE;
			start += MAX_SPECIAL_NAME_SIZE;
		} else {
			char c = *(str + tok->len);
			if (tok->len < MAX_SPECIAL_NAME_SIZE && c != ' ' && c != '\0'){
				//not a proper match; try again
				index = -1;
				str += MAX_SPECIAL_NAME_SIZE;
				start += MAX_SPECIAL_NAME_SIZE;
			} else {
				break; //have a valid match: exit
			}
		}
	} while (*str != '\0');
	
	return index;
}

void print_token(struct tokenizer* state, struct token t){
	if (t.type != command && t.type != function && t.type != operation && t.type != sysvar){
		iprintf("ofs:%d len:%d type:%d ", (int)t.ofs, t.len, t.type);
		if (t.prio) {
			iprintf("prio:%02d ", t.prio);
		} else {
			iprintf("        ");
		}
		for (size_t c = 0; c < t.len; ++c){
			char ch = state->source->data[t.ofs + c];
			if (ch == '\r'){
				iprintf("\\r");
			} else {
				iprintf("%c", ch);
			}
		}
	} else {
		// op, func, cmd, sysvar
		const char* names = t.type == command ? commands : t.type == function ? functions : t.type == sysvar ? sysvars : bc_conv_operations;
		char name[9] = {0};
		for (size_t i = 0; i < 8; ++i){
			name[i] = names[8*t.ofs + i];
		}
		iprintf("id:%d type:%d cmd:%s", (int)t.ofs, t.type, name);
		if (t.prio) {
			iprintf("prio:%02d ", t.prio);
		} else {
			iprintf("        ");
		}
	}
	iprintf("\n");
}

void tokenize(struct program* src, struct program* out){
	struct tokenizer state = {0};
	state.source = src;
	state.output = out;
	state.cursor = 0;
	state.is_comment = 0;
	state.state = TKR_NONE;
	
	while (state.cursor < state.source->size){
		tok_none(&state);
		switch(state.state){
			case TKR_NONE:
				break;
			case TKR_CONVERT:
				tok_convert(&state);
				break;
			default:
				iprintf("Tokenizer crash at: %d Char: %c \n", (int)state.cursor, state.source->data[state.cursor]);
				goto crash;
		}
	}
	crash:
	
	iprintf("Current tokens:\n");
	for (size_t i = 0; i < 100; ++i){
		if (state.tokens[i].len == 0)
			break;
		print_token(&state, state.tokens[i]);
	}
}

void tok_convert(struct tokenizer* state){
	// convert the tokens into bytecode
	// this can be done per-line
	
	tok_prio(state);
	
	tok_eval(state);
	
	tok_test(state);
	
	tok_code(state);
}

// * = any (don't check/special case)
// N = number
// n = number var
// S = string
// s = string var
// a = array [only SORT,RSORT should use this?]
// E = both
// L = label (includes string)
// 0 = Nothing (no argument) (must be first in list)
// ; = semicolon
// < = comma
// , = separates valid formations
const char* cmd_format[] = {
	"*","NN","N,NN","*", //DIM
	"*","*","*","0,n", //NEXT
	"N","","*","0", //ENDIF
	"L,l","L,l","N","0", //RETURN
	"0","0", //STOP
	"0","NNNNNN","0","N","N", //WAIT
	"*","S;s,s", //LINPUT
	"","0,N,NN,NNN,NNNN", //BEEP
	"NNNN","","","NNNNNN","", //BGMCLEAR
	"","","","","","","", //BGMVOL
	"NNN,NNNN","N","NNNN,NNNNNNN","","NNN","S","", //CHRREAD
	"","0","","","","0", //CONT
	"","","Snnn","","NNNN,NNNNN", //GBOX
	"","0,N","N","","","NNNN,NNNNN","NNNN,NNNNN",//GLINE
	"N,NNN","","NN,NNN","","","","NN",//ICONSET
	"","","","",//NEW
	"NNS","S","*","","","",//RENAME
	"L","","0","","","","",//SPANGLE
	"","","","","","","","",//SPPAGE
	"","","","","",//SWAP
	"",//TMREAD
};

const char* func_format[] = {
	"N","S","N,NN","","","","0","0,N",//BUTTON
	"NN","N","N","N","","N","NN,NNN","N,NN","",//ICONCHK
	"0","","","S","","SNN","0","NN","N",//RAD
	"","N","N","N","","","","",//SPHITRC
	"","N","N","","","S",//VAL
};

const char func_return[] = "NNNNNNNN"
"NSNNNNNSN"
"SNSNNSNNN"
"SNNNNNNN"
"NNSSNN";

const char* op_format[] = {
	"NN,SS","*","NN","NN,SN","NN","*", //;
	"nN,sS","N", //(-)
	"NN,SS","NN,SS","NN","NN","NN","NN", //<=
	"NN",//%
	"NN","NN","N","NN","N",//!
	"*","*","*","*"
};

/// char* stack is a string of length stack_len
/// valid is a null-terminated string
/// true indicates the command is valid
bool check_cmd(const char* stack, int stack_len, const char* valid){
	if ((valid[0] == '0' || valid[0] == '*') && stack_len == 0) return true;
	
	int valid_len = strlen(valid);
	int stack_i = 0;
	bool is_valid = true;
	for (int valid_i = 0; valid_i < valid_len;){
		char s = stack[stack_i++];
		char v = valid[valid_i++];
//		iprintf("%c?=%c\n", s, v);
		if (v == ',') {
			if (is_valid) return true;
			stack_i = 0;
			is_valid = true;
			continue;
		}
		if (stack_i > stack_len) return false;
		
		//TODO:PERF:NONE Check if refactoring this to use fallthrough is faster
		switch (v){
			case 'N':
				if (s == 'N' || s == 'n') break;
				is_valid = false;
				break;
				
			case 'n':
				if (s == 'n') break;
				is_valid = false;
				break;
				
			case 'S':
				if (s == 'S' || s == 's') break;
				is_valid = false;
				break;
				
			case 's':
				if (s == 's') break;
				is_valid = false;
				break;
				
			case 'L':
				if (s == 'S' || s == 's' || s == 'L') break;
				is_valid = false;
				break;
				
			case 'l':
				if (s == 'S' || s == 's' || s == 'L') {
					valid_i--; // read as many labels as needed
					break;
				}
				is_valid = false;
				break;
				
			case '*':
				return true;
				
			default:
				iprintf("Invalid format string is always valid.\n");
				return true;
		}
	}
	if (stack_i != stack_len) return false;
	return true;
}

void tok_test(struct tokenizer* state){
	// This is argument count and argument type validation.
	char stack[40];
	u8 stack_i = 0;
	int argc = 0;
	
	iprintf("Validation:\n");
	for (u8 i = 0; i < state->token_i; ++i){
		iprintf("Stack: %.*s\n", stack_i, stack);
		iprintf("%d: ", i);
		print_token(state, state->tokens[i]);
		switch (state->tokens[i].type){
			case number:
				stack[stack_i++] = 'N';
				break;
			
			case string:
				stack[stack_i++] = 'S';
				break;
				
			case command:
				{
				u8 cmd = state->tokens[i].ofs;
				const char* valid = cmd_format[cmd];
				if (!check_cmd(stack, stack_i, valid)){
					// Set error state on invalid command
					print_token(state, state->tokens[i]);
					iprintf("Stack: %d %s\n", stack_i, stack);
					abort();
				}
				// relies on variable carrying through for assignment
				if (cmd != CMD_FOR){
					stack_i = 0;
				}
				}
				break;
				
			case operation:
				{
				u8 prio = state->tokens[i].prio;
				const char* valid;
				bool is_valid = true;
				if (state->tokens[i].ofs == OP_COMMA){
					stack[stack_i++] = '<';
				} else if (state->tokens[i].ofs == OP_SEMICOLON){
					stack[stack_i++] = ';';
				} else if (prio % 8 != 6){
					// binary
					valid = op_format[state->tokens[i].ofs];
					is_valid = check_cmd(&stack[stack_i-2], 2, valid);
					stack_i -= 2; // +2 -1
					// note: any binary op will have the type of the first argument
					if (state->tokens[i].ofs != OP_ASSIGN)
						stack[stack_i++] &= 0x5f;  //binary op returns value type of first argument. unless it's =, that doesn't return anything.
				} else {
					// unary: all of these take numbers, so
					valid = "N";
					is_valid = check_cmd(&stack[stack_i-1], 1, valid);
					// +1 -1
					stack_i--;
					stack[stack_i++] = 'N';  //any unary op returns number
				}
				if (!is_valid){
					// Set error state on invalid command
					print_token(state, state->tokens[i]);
					iprintf("Stack: %d %s\n", stack_i, stack);
					abort();
				}
				}
				break;
				
			case function:
				if (argc){
					stack_i -= argc;
				}
				u8 func = state->tokens[i].ofs;
				const char* valid = func_format[func];
				bool is_valid = check_cmd(&stack[stack_i], argc, valid);
				if (!is_valid){
					// Set error state on invalid command
					print_token(state, state->tokens[i]);
					iprintf("Stack: %d %s\n", stack_i, stack);
					abort();
				}
				stack[stack_i++] = func_return[func];
				argc = 0;
				break;
				
			case name:
				if (argc){
					// process argc counts first
					const char* valid = "N,NN";
					bool is_valid = check_cmd(&stack[stack_i-argc], argc, valid);
					if (!is_valid){
						// arguments to array access are bad
						iprintf("Bad array arguments!\n");
						iprintf("Stack: %d %s\n", stack_i, stack);
						abort();
					}
					stack_i -= argc;
					argc = 0;
				}
				u8 last = state->tokens[i].len;
				if (state->source->data[state->tokens[i].ofs + last - 1] == '$'){
					stack[stack_i++] = 's';
				} else {
					stack[stack_i++] = 'n';
				}
				break;
				
			case sysvar:
				{
				u8 id = state->tokens[i].ofs;
				if (id == SYS_TIME$ || id == SYS_DATE$ || id == SYS_PRGNAME$
					|| id == SYS_PACKAGE$ || id == SYS_MEM$){
						stack[stack_i++] = 's';
					} else {
						stack[stack_i++] = 'n';
					}
				}
				break;
				
			case label_string:
				stack[stack_i++] = 'L';
				break;
				
			case arg_count:
				argc = state->tokens[i].len;
				break;
				
			default:
				break;
		}
	}
}

// none of these values will exceed 100[?]
struct eval {
	//value and op could be combined maybe
	struct token* value_stack[100];
	u8 value_i;
	struct token* op_stack[100];
	u8 op_i;
	/// Arguments stack
	u8 argc_stack[64];
	u8 argc_i;
	// reordered tokens for conversion
	struct token result[100];
	u8 result_i; // first unused entry
};

s32 tok_to_num(struct tokenizer* state, struct token* t){
	s32 number = str_to_num((u8*)&state->source->data[t->ofs], t->len);
	iprintf("Number := %f", (double)number);
	return number;
}

void tok_code(struct tokenizer* state){
	// assume the order is fixed
	char* data = state->output->data;
	idx* size = &state->output->size;
	
	for (size_t i = 0; i < state->token_i; ++i){
		switch (state->tokens[i].type){
			case command:
				if (state->tokens[i].ofs == CMD_DATA){
					// Special rules for DATA: write token contents as string
					// TODO:ERR:MED Don't allow whitespace within unquoted DATA strings.
					iprintf("DATA tok_code %d\n", state->token_i);
					data[(*size)++] = BC_DATA;
					char* data_size = &data[(*size)++]; // write as copying characters
					*data_size = 0;
					i++;
					while (i < state->token_i){
						struct token t = state->tokens[i];
						switch (t.type){
							case command:
							case function:
							case operation:
							case sysvar:
								// need to "de-convert" these
								{
								const char* names = t.type == command ? commands : t.type == function ? functions : t.type == sysvar ? sysvars : bc_conv_operations;
								for (size_t j = 0; j < 8; ++j){
									char c = names[8*t.ofs+j];
									if (c != ' '){
										data[(*size)++] = c;
										(*data_size)++;
									} else {
										break;
									}
								}
								}
								break;
							
							case newline:
								// Don't write these into the string
								// Break the outer while to prevent moving past newline
								// (this is handled by the outer FOR loop)
								break;
							
							default:
								*data_size += t.len;
								for (size_t j = 0; j < t.len; ++j){
									data[(*size)++] = state->source->data[t.ofs + j];
								}
						}
						++i;
					}
					if (*size % 2){
						data[(*size)++] = 0; // pad one null to keep instructions aligned
					}
					
					// End of DATA special rules
				} else {
					// convert token to command
	//				iprintf("command: %d, %d\n", j, state->tokens[i].ofs);
					data[(*size)++] = BC_COMMAND;
					data[(*size)++] = state->tokens[i].ofs;
				}
				break;
			
			case number:
				// oops i need to convert strings to numbers - do that later...
				if (state->tokens[i].len == 1){
					data[(*size)++] = BC_SMALL_NUMBER;
					data[(*size)++] = state->source->data[state->tokens[i].ofs] - '0';
				} else if (state->tokens[i].len == 2){ 
					data[(*size)++] = BC_SMALL_NUMBER;
					data[(*size)++] = 10*(state->source->data[state->tokens[i].ofs] - '0') + (state->source->data[state->tokens[i].ofs+1] - '0');
				} else {
					s32 fp = tok_to_num(state, &state->tokens[i]);
					
					// large number
					data[(*size)++] = BC_NUMBER;
					data[(*size)++] = 0x0; //ignored
					data[(*size)++] = (fp >> 24) & 0xff;
					data[(*size)++] = (fp >> 16) & 0xff;
					data[(*size)++] = (fp >> 8) & 0xff;
					data[(*size)++] = (fp >> 0) & 0xff;
				}
				break;
			
			case string:
				data[(*size)++] = BC_STRING;
				data[(*size)++] = state->tokens[i].len;
				for (int j = 0; j < state->tokens[i].len; ++j){
					data[(*size)++] = state->source->data[state->tokens[i].ofs + j];
				}
				if (*size % 2){
					data[(*size)++] = 0; // pad one null to keep instructions aligned
				}
				break;
				
			case operation:
				data[(*size)++] = BC_OPERATOR;
				
//				int op = tok_in_str_index(bc_conv_operations, state->source->data, &state->tokens[i]);
				int op = state->tokens[i].ofs;
				if (state->tokens[i].prio % 8 == 6 && op == OP_SUBTRACT){
					op = OP_NEGATE;
				}
				data[(*size)++] = op;
				
				iprintf("%d", data[*size-1]);
				break;
				
			case function:
				data[(*size)++] = BC_FUNCTION;
				
				data[(*size)++] = state->tokens[i].ofs;
				break;
				
			case name:
			case dim_arr:
				//TODO:PERF:NONE actual ID calculation
				//Find variable if it exists in table
				//Create variable if it does not exist
				//Get ID based on this index
				//Note: Small name becomes 
				data[(*size)++] = state->tokens[i].type == name ? BC_VARIABLE_NAME : BC_DIM;
				if (state->tokens[i].len == 1){
					// Name is a single char: replace length (always less than 'A')
					data[(*size)++] = state->source->data[state->tokens[i].ofs];
				} else {
					data[(*size)++] = state->tokens[i].len;
					for (int j = 0; j < state->tokens[i].len; ++j){
						data[(*size)++] = state->source->data[state->tokens[i].ofs + j];
					}
					if (*size % 2){
						data[(*size)++] = 0; // pad one null to keep instructions aligned
					}
				}
				break;
				
			case arg_count:
				data[(*size)++] = BC_ARGCOUNT;
				data[(*size)++] = state->tokens[i].len;
				break;
				
			case loop_begin:
				data[(*size)++] = BC_BEGIN_LOOP;
				data[(*size)++] = 0;
				break;
				
			case sysvar:
				data[(*size)++] = BC_SYSVAR;
				data[(*size)++] = state->tokens[i].ofs;
				break;
				
			case label:
			case label_string:
				data[(*size)++] = state->tokens[i].type == label ? BC_LABEL : BC_LABEL_STRING;
				data[(*size)++] = state->tokens[i].len;
				for (int j = 0; j < state->tokens[i].len; ++j){
					data[(*size)++] = state->source->data[state->tokens[i].ofs + j];
				}
				if (*size % 2){
					data[(*size)++] = 0; // pad one null to keep instructions aligned
				}
				break;
				
			default:
				iprintf("Unknown token: ");
				print_token(state, state->tokens[i]);
				//TODO:ERR:MED Code generation errors?
		}
	}
	state->token_i = 0;
}

// Remove operators until priority reaches prio
// eval state struct
// prio to clean to
void tok_eval_clean_stack(struct eval* e, u16 prio){
	while (e->op_i && e->op_stack[e->op_i-1]->prio >= prio){
//		u16 op_prio = e->op_stack[e->op_i-1]->prio;
		
		e->result[e->result_i++] = *e->op_stack[--e->op_i];
	}
}

// once again based on
// https://en.wikipedia.org/wiki/Shunting_yard_algorithm
// This one is infinitely less bad than than the PTC-EmkII attempt. :)
// That one was awful. Go look at it if you want to laugh at bad code.
// I'm still amazed it worked in the end.
void tok_eval(struct tokenizer* state){
	struct eval e = {0};
	bool is_for = false;
	bool is_dim = false;
	bool is_if = false;
	bool implicit_commas = true;
	
	iprintf("[tok_eval] Entry with %d unit(s)\n", state->token_i);
	if (state->token_i && state->tokens[0].type == command && state->tokens[0].ofs == CMD_DATA){
		iprintf("DATA special case\n");
		for (size_t j = 0; j < state->token_i; ++j){
			print_token(state, state->tokens[j]);
		}
		state->state = TKR_NONE; // Important: Tells tokenizer to read a new line
		return;
	}
	
	for (size_t i = 0; i < state->token_i; ++i){
		u16 prio = state->tokens[i].prio;
//		char first = state->source->data[state->tokens[i].ofs];
		char op = state->tokens[i].ofs;
		iprintf("[tok_eval]");
		print_token(state, state->tokens[i]);
		
		if (state->tokens[i].type == operation && prio % 8 == 0) {
			// operation of prio 0: ( ) [ ] =
			if (op == OP_ASSIGN){
				tok_eval_clean_stack(&e, prio);
				e.op_stack[e.op_i++] = &state->tokens[i];
			} else if (op == OP_OPEN_BRACKET || op == OP_OPEN_PAREN){
				e.argc_i++;
				// down a level; default # args is 1 (no commas, not "()")
				e.argc_stack[e.argc_i] = 1;
				if (i){
					u8 prev_type = state->tokens[i-1].type;
					if (i && prev_type != name && prev_type != function && prev_type != dim_arr){
						e.argc_stack[e.argc_i] |= 0x80; // special indicator for "doesn't NEED argcount"
					}
				}
			} else { // assumed ) or ]
				// handle 0-args PI(), BUTTON() case
				if (state->tokens[i-1].type == operation && state->tokens[i-1].ofs == OP_OPEN_PAREN){
					e.argc_stack[e.argc_i] = 0;
				}
				// TODO:PERF:NONE Don't create argcount for DIM command
				// (unnecessary, implied by stack size?)
				// also might just keep this way for simplicity
				if (!(e.argc_stack[e.argc_i] & 0x80)){
					state->tokens[i].type = arg_count;
					state->tokens[i].len = e.argc_stack[e.argc_i--];
					tok_eval_clean_stack(&e, prio);
					e.op_stack[e.op_i++] = &state->tokens[i];
				} else {
					e.argc_i--;
				}
			}
		} else if (state->tokens[i].type == operation && prio == 1) {
			// comma or semicolon at lowest nest level
			e.argc_stack[e.argc_i]++;
			tok_eval_clean_stack(&e, prio);
			if (implicit_commas){
				// don't push commas unless they are prior to another comma or at the end of the line
				if (state->tokens[i].ofs == OP_SEMICOLON){
					e.result[e.result_i++] = state->tokens[i];
				} else { // must be a comma here
					if (state->tokens[i+1].type == newline || state->tokens[i+1].prio % 8 == 1){
						e.result[e.result_i++] = state->tokens[i];
					}
				}
			} else {
				// commas and semicolons should be pushed.
				e.result[e.result_i++] = state->tokens[i];
			}
		} else if (state->tokens[i].type == command || prio > 0){
			// operator, function or command
			if (state->tokens[i].type == command){
				// These are all of the special cases
				// (many are flow control commands with unique parsing needs)
				if (state->tokens[i].ofs == CMD_DIM){
				// command is DIM: set array-creation-mode for following tokens
					is_dim = true;
					continue; // do not compile as instruction
				} else if (state->tokens[i].ofs == CMD_PRINT){
					// Command is PRINT: semicolons are implied if omitted;
					// commas are always added
					implicit_commas = false;
				} else if (state->tokens[i].ofs == CMD_FOR){
					// Command is FOR: add instruction at end of FOR setup to
					// properly execute the loop
					is_for = true;
					// TODO:PERF:NONE TO and STEP are unneeded instructions after a FOR
					// Do not compile those
				} else if (state->tokens[i].ofs == CMD_IF){
					// Command is IF: add normally, but needs to indicate that
					// ENDIF should be added once line end is hit
					is_if = true;
				} else if (state->tokens[i].ofs == CMD_THEN){
					// THEN should be discarded: default IF behavior will be to
					// continue to next instruction, which can ignore THEN
					tok_eval_clean_stack(&e, 0);
					continue;
				} else if (state->tokens[i].ofs == CMD_ELSE){
					// ELSE should be placed immediately
					// similar to commas within a command, it is used as a
					// separator between TRUE/FALSE paths of the IF.
					tok_eval_clean_stack(&e, 0);
					e.result[e.result_i++] = state->tokens[i];
					continue;
				} else {
					// Any other "normal" command
					is_dim = false;
					// Note: is_for will not be always be reset because FOR
					// needs to be kept to the newline, including through other
					// commands TO and STEP
					if (is_for && state->tokens[i].ofs != CMD_TO && state->tokens[i].ofs != CMD_STEP) {
						e.result[e.result_i++] = (struct token){.type=loop_begin, .ofs=0, .len=1, .prio=0};
						is_for = false;
					}
				}
			} else if (state->tokens[i].type == name && prio == 7 && is_dim){
				// arrays with prio 7: flag the type for array creation
				// if name and prio 7 then => array; mark as array if DIM else
				// unnecessary and just read element
				state->tokens[i].type = dim_arr;
			}
			
			if (!e.op_i || e.op_stack[e.op_i-1]->prio < prio + (prio % 8 == 6)){
				// if the current operator is higher prio to that
				// on the stack top, push it to the stack
			} else {
				// the current operator is lower or same priority than the stack
				// remove arguments from the stack, then push current operator
				tok_eval_clean_stack(&e, prio);
			}
			// DON'T push a comma, as this is implied from argument ordering/structure
			if (state->tokens[i].type != operation || state->tokens[i].ofs != OP_COMMA)
				e.op_stack[e.op_i++] = &state->tokens[i];
			else
				e.argc_stack[e.argc_i]++;
		} else if (state->tokens[i].type == newline){ 
			// dim ends
			tok_eval_clean_stack(&e, 0);
			is_dim = false;
			implicit_commas = true;
			if (is_for){
				e.result[e.result_i++] = (struct token){.type=loop_begin, .ofs=0, .len=1, .prio=0};
				is_for = false;
			}
		} else if (state->tokens[i].type == label){
			// label defined
			if (i != 0){
				// label is an argument to something like goto
				// convert to string and use as @
				state->tokens[i].type = label_string;
			}
			e.result[e.result_i++] = state->tokens[i];
		} else {
			// values
			e.result[e.result_i++] = state->tokens[i];
		}
	}
	//cleanup
	tok_eval_clean_stack(&e, 0);
	
	if (is_if){
		e.result[e.result_i++] = (struct token){.type=command, .ofs=CMD_ENDIF, .len=1, .prio=0};
	}
	
//	if (state->tokens[0].ofs == CMD_DATA) iprintf("DATA special case WHAT?=%d\n", state->token_i);
	iprintf("Result size: %d\n",(int)e.result_i);
	for (size_t i = 0; i < e.result_i; ++i){
		if (e.result[i].len == 0)
			break;
		print_token(state, e.result[i]);
	}
	
	for (size_t i = 0; i < e.result_i; ++i){
		state->tokens[i] = e.result[i];
	}
	state->token_i = e.result_i;
	state->state = TKR_NONE;
}

// take one line (to /r) and calculate priorities of tokens
void tok_prio(struct tokenizer* state){
	int nest = 0;
	for (size_t i = 0; i < state->token_i; i += 1){
		if (state->tokens[i].type == operation){
			u8 op_id = state->tokens[i].ofs;
			switch (op_id){
				case OP_ASSIGN:
					state->tokens[i].prio = nest + 0;
					break;
				case OP_COMMA:
				case OP_SEMICOLON:
					state->tokens[i].prio = nest + 1;
					break;
				case OP_AND:
				case OP_OR:
				case OP_XOR:
					state->tokens[i].prio = nest + 2;
					break;
				case OP_LESS:
				case OP_LESS_EQUAL:
				case OP_GREATER_EQUAL:
				case OP_GREATER:
				case OP_EQUAL:
				case OP_NOT_EQUAL:
					state->tokens[i].prio = nest + 3;
					break;
				case OP_ADD:
					state->tokens[i].prio = nest + 4;
					break;
				case OP_SUBTRACT:
					if ((i>0 && state->tokens[i-1].type == number) ||
							state->tokens[i-1].type == name ||
							state->tokens[i-1].type == sysvar || 
							(state->tokens[i-1].type == operation && (
								state->tokens[i-1].ofs > OP_OPEN_PAREN
							))){
						state->tokens[i].prio = nest + 4;
					} else {
						state->tokens[i].ofs = OP_NEGATE;
						state->tokens[i].prio = nest + 6;
					}
					break;
				case OP_MULTIPLY:
				case OP_DIVIDE:
				case OP_MODULO:
					state->tokens[i].prio = nest + 5;
					break;
				case OP_NOT:
				case OP_LOGICAL_NOT:
					state->tokens[i].prio = nest + 6;
					break;
				case OP_OPEN_BRACKET:
				case OP_OPEN_PAREN:
					if (i>0 && state->tokens[i-1].type == name){
						state->tokens[i-1].prio = nest + 7;
					}
					nest += 8;
					state->tokens[i].prio = nest;
					break;
				case OP_CLOSE_BRACKET:
				case OP_CLOSE_PAREN:
					state->tokens[i].prio = nest;
					nest -= 8;
					break;
			}
		} else if (state->tokens[i].type == function){
			state->tokens[i].prio = nest + 7;
		} else {
			state->tokens[i].prio = 0;
		}
	}
}

//only needed for operations and a few others
void tok_single(struct tokenizer* state, enum type type){
	state->tokens[state->token_i].type = type;
	state->tokens[state->token_i].ofs = state->cursor;
	state->tokens[state->token_i].len = 1;
	state->cursor++;
	state->token_i++;
}

void tok_with_condition(struct tokenizer* state, bool(*condition)(char)){
	state->tokens[state->token_i].type = name;
	state->tokens[state->token_i].len = 0;
	state->tokens[state->token_i].ofs = state->cursor;
	char c;
	do {
		state->tokens[state->token_i].len++;
		state->cursor++;
		c = state->source->data[state->cursor];
	} while (condition(c));
}

//TODO:CODE:LOW Rename this/merge with tokenizer loop
void tok_none(struct tokenizer* state){
	// This state represents: Whitespace and default state
	// This state can transition to:
	char c = state->source->data[state->cursor];
	
	if (c == '\r'){
		tok_single(state, newline);
		state->is_comment = false;
		state->state = TKR_CONVERT;
		// mark comment as ended
		return;
	}
	
	if (state->is_comment) {
		state->cursor++;
		return;
	}
	
	if (is_name_start(c)){
		tok_name(state);
	} else if (c == '?'){
//		tok_single(state, command);
		state->tokens[state->token_i].type = command;
		state->tokens[state->token_i].ofs = CMD_PRINT;
		state->cursor++;
		state->token_i++;
	} else if (c == ':'){
		tok_single(state, newline);
	} else if (c == ' '){
		//whitespace is ignored
		state->cursor++;
	} else if (c == '"'){
		tok_string(state);
	} else if (is_number(c) || c == '.') {
		tok_number(state);
	} else if (c == ',' || c == ';' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
			c == '(' || c == ')' || c == '[' || c == ']'){
		tok_single(state, operation);
		state->tokens[state->token_i-1].ofs = tok_in_str_index(bc_conv_operations, state->source->data, &state->tokens[state->token_i-1]);
	} else if (c == '<' || c == '=' || c == '>' || c == '!'){
		tok_single(state, operation);
		// check for <= == >= !=
		if (state->source->data[state->cursor] == '='){
			state->tokens[state->token_i - 1].len++;
			state->cursor++;
		}
		state->tokens[state->token_i-1].ofs = tok_in_str_index(bc_conv_operations, state->source->data, &state->tokens[state->token_i-1]);
	} else if (c == '@'){
		state->cursor++;
		tok_with_condition(state, is_name);
		state->tokens[state->token_i].type = label;
		state->token_i++;
	} else if (c == '\''){
		// read to newline
		state->cursor++;
		state->is_comment = true;
	} else {
		iprintf("Unknown transition from NONE on %c:%d\n", c, c);
		state->cursor++;
	}
}

void tok_name(struct tokenizer* state){
	tok_with_condition(state, is_varname);
	//additional checking for special strings
	int index;
	if (0 <= (index = tok_in_str_index(commands, state->source->data, &state->tokens[state->token_i]))){
		state->tokens[state->token_i].type = command;
		state->tokens[state->token_i].ofs = index;
	} else if (0 <= (index = tok_in_str_index(functions, state->source->data, &state->tokens[state->token_i]))){
		state->tokens[state->token_i].type = function;
		state->tokens[state->token_i].ofs = index;
	} else if (0 <= (index = tok_in_str_index(bc_conv_operations, state->source->data, &state->tokens[state->token_i]))){
		// note that this is only the bitwise ops
		state->tokens[state->token_i].type = operation;
		state->tokens[state->token_i].ofs = index;
	} else if (0 <= tok_in_str_index(labels, state->source->data, &state->tokens[state->token_i])){
		state->tokens[state->token_i].type = label;
	} else if (0 <= tok_in_str_index(comments, state->source->data, &state->tokens[state->token_i])){
		state->tokens[state->token_i].type = comment;
	} else if (0 <= (index = tok_in_str_index(sysvars, state->source->data, &state->tokens[state->token_i]))){
		state->tokens[state->token_i].type = sysvar;
		state->tokens[state->token_i].ofs = index;
	}
	
	state->token_i++;
}

void tok_string(struct tokenizer* state){
//	iprintf("Entered string %d\n", state->cursor);
	if (state->source->data[state->cursor] == '"'){
		state->cursor++;
	}
	
	state->tokens[state->token_i].type = string;
	state->tokens[state->token_i].len = 0;
	state->tokens[state->token_i].ofs = state->cursor;
	while (state->source->data[state->cursor] != '"' &&
			state->source->data[state->cursor] != '\r'){
		state->tokens[state->token_i].len++;
		state->cursor++;
	}
	if (state->source->data[state->cursor] == '"'){
		state->cursor++;
	}
	state->token_i++;
//	iprintf("Exited string %d\n", state->token_i);
}

void tok_number(struct tokenizer* state){
	state->tokens[state->token_i].type = number;
	state->tokens[state->token_i].len = 0;
	state->tokens[state->token_i].ofs = state->cursor;
	do {
		state->tokens[state->token_i].len++;
		state->cursor++;
	} while (is_number(state->source->data[state->cursor]) || state->source->data[state->cursor] == '.');
	state->token_i++;
}

