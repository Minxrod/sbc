#include "tokens.h"

#include "ptc.h"
#include "program.h"

#include <stdio.h>
#include <stdint.h>

const int MAX_SPECIAL_NAME_SIZE = 8;

const char* commands =
"PRINT   LOCATE  COLOR   DIM     FOR     TO      STEP    NEXT    "
"IF      THEN    ELSE    [ENDIF] GOTO    GOSUB   ON      RETURN  "
"END     STOP    ";

/*
"ACLS    APPEND  "
"BEEP    BGCLIP  BGCLR   BGCOPY  BGFILL  BGMCLEARBGMPLAY BGMPRG  BGMSET  BGMSETD BGMSETV "
"BGMSTOP BGMVOL  BGOFS   BGPAGE  BGPUT   BGREAD  BREPEAT CHRINIT CHRREAD CHRSET  CLEAR   "
"CLS     COLINIT COLOR   COLREAD COLSET  CONT    DATA    DELETE  DIM     DTREAD  ELSE    "
"END     EXEC    FOR     GBOX    GCIRCLE GCLS    GCOLOR  GCOPY   GDRAWMD GFILL   GLINE   "
"GOSUB   GOTO    GPAGE   GPAINT  GPSET   GPRIO   GPUTCHR ICONCLR ICONSET IF      INPUT   "
"KEY     LINPUT  LIST    LOAD    LOCATE  NEW     NEXT    ON      PNLSTR  PNLTYPE "
"READ    REBOOT  RECVFILERENAME  RESTORE RETURN  RSORT   RUN     SAVE    SENDFILESORT    "
"SPANGLE SPANIM  SPCHR   SPCLR   SPCOL   SPCOLVECSPHOME  SPOFS   SPPAGE  SPREAD  SPSCALE "
"SPSET   SPSETV  STEP    STOP    SWAP    THEN    TMREAD  TO      VISIBLE VSYNC   WAIT    ";
*/

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
"ERL     ERR     ";

const char* labels = "LABEL   ";

const char* comments = "REM     ";

bool is_lower(const char c){
	return 'a' <= c && c <= 'z';
}

bool is_upper(const char c){
	return 'A' <= c && c <= 'Z';
}

bool is_number(const char c){
	return '0' <= c && c <= '9';
}

bool is_alpha(const char c){
	return is_upper(c) || is_lower(c);
}

bool is_alphanum(const char c){
	return is_alpha(c) || is_number(c);
}

bool is_name_start(const char c){
	return is_alpha(c) || c == '_';
}

bool is_name(const char c){
	return is_alphanum(c) || c == '_';
}

bool is_varname(const char c){
	return is_name(c) || c == '$';
}

// Scans for a specific instruction. Special purpose function to handle the
// various instruction lengths.
u32 bc_scan(struct program* code, u32 index, u8 find){
	// search for find in code->data
	while (index < code->size){ 
		u8 cur = code->data[index];
		if (cur == find){
			return index;
		}
		if (cur == BC_STRING || cur == BC_LABEL || cur == BC_LABEL_STRING){
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
	
	tok_code(state);
}

struct eval {
	//value and op could be combined maybe
	struct token* value_stack[100];
	size_t value_i;
	struct token* op_stack[100];
	size_t op_i;
	/// Arguments stack
	size_t argc_stack[64];
	size_t argc_i;
	// reordered tokens for conversion
	struct token result[100];
	size_t result_values; // base of current expression
	size_t result_i; // first unused entry
};

s32 tok_to_num(struct tokenizer* state, struct token* t){
	s32 number = 0;
	s32 fraction = 0;
	s32 maximum = 1;
	for (size_t j = 0; j < t->len; ++j){
		char c = state->source->data[t->ofs + j];
		if (is_number(c)){
			number *= 10;
			number += c - '0';
		} else if (c == '.'){
			++j;
			for (size_t k = j; k < t->len; ++k){
				c = state->source->data[t->ofs + k];
				fraction *= 10;
				maximum *= 10;
				fraction += c - '0';
			}
			break;
		}
	}
	number = number * 4096 + 4096 * fraction / maximum;
	
	iprintf("Number := %f", (double)number);
	return number;
}

void tok_code(struct tokenizer* state){
	// assume the order is fixed
	char* data = state->output->data;
	u32* size = &state->output->size;
	
	for (size_t i = 0; i < state->token_i; ++i){
		switch (state->tokens[i].type){
			case command:
				// convert token to command
				//TODO: ???
//				iprintf("command: %d, %d\n", j, state->tokens[i].ofs);
				data[(*size)++] = BC_COMMAND;
				data[(*size)++] = state->tokens[i].ofs;
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
				//TODO: actual ID calculation
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
				//TODO: Code generation errors?
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
				// down a level; default # args is 1 (no commas, not ()
				e.argc_stack[e.argc_i] = 1;
			} else { // assumed ) or ]
				// handle 0-args PI(), BUTTON() case
				if (state->tokens[i-1].type == operation && state->tokens[i-1].ofs == OP_OPEN_PAREN){
					e.argc_stack[e.argc_i] = 0;
				}
				// TODO: Don't create argcount for DIM command
				// (unnecessary, implied by stack size?)
				// also might just keep this way for simplicity
				state->tokens[i].type = arg_count;
				state->tokens[i].len = e.argc_stack[e.argc_i--];
				tok_eval_clean_stack(&e, prio);
				e.op_stack[e.op_i++] = &state->tokens[i];
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
					// TODO: TO and STEP are unneeded instructions after a FOR
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
			if (state->tokens[i].ofs != OP_COMMA)
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
		//TODO: copy comment text, instead of ignore?
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
	} else if (0 <= (index = tok_in_str_index(operations, state->source->data, &state->tokens[state->token_i]))){
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

