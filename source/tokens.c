#include "tokens.h"
#include "ptc.h"

#include <stdio.h>
#include <stdint.h>

const int MAX_SPECIAL_NAME_SIZE = 8;
const char* commands =
"PRINT   LOCATE  COLOR   ";

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
"+       -       *       /       %       "
"AND     OR      NOT     XOR     -       !       "
">       >=      <       <=      ==      !=      "
"=       ,       ;       ";

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

// Note: Expects null-terminated string
bool tok_in_str(const char* str, const char* dat, struct token* tok){
	bool good;
	do {
		good = true;
		for (size_t i = 0; i < tok->len; ++i){
			if (dat[tok->ofs + i] != *str){
				good = false;
			}
			str++;
			if (!good) break;
		}
		if (good) break;
	} while (*str != '\0');
	if (is_alpha(dat[tok->ofs + tok->len]))
		good = false; // Substring continues
	return good;
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
			break;
		}
	} while (*str != '\0');
	char c = *(str + tok->len);
	if (tok->len < MAX_SPECIAL_NAME_SIZE && c != ' ' && c != '\0')
		return -1; // not a proper match
	
	return index;
}


void print_token(struct tokenizer* state, struct token t){
	iprintf("ofs:%d len:%d type:%d ", t.ofs, t.len, t.type);
	if (t.prio) {
		iprintf("prio:%2d ", t.prio);
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
	iprintf("\n");
}

void tokenize(struct program* src, struct program* out){
	struct tokenizer state;
	state.source = src;
	state.output = out;
	state.cursor = 0;
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
				iprintf("Tokenizer crash at: %d Char: %c \n", state.cursor, state.source->data[state.cursor]);
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
	// reordered tokens for conversion
	struct token result[100];
	size_t result_values; // base of current expression
	size_t result_i; // first unused entry
};

void tok_code(struct tokenizer* state){
	// assume the order is fixed
	char* data = state->output->data;
	u32* size = &state->output->size;
	
	for (size_t i = 0; i < state->token_i; ++i){
		if (state->tokens[i].type == command){
			// convert token to command
			if (state->source->data[state->tokens[i].ofs] == '?'){
				data[(*size)++] = BC_COMMAND;
				data[(*size)++] = (char)CMD_PRINT;
			} else {
				int j = tok_in_str_index(commands, state->source->data, &state->tokens[i]);
				iprintf("command: %d, %d\n", j, state->tokens[i].ofs);
				if (j >= 0){
					data[(*size)++] = BC_COMMAND;
					data[(*size)++] = (char)(j);
				} else {
					iprintf("Invalid command: %d, %d\n", j, state->tokens[i].ofs);
				}
			}
		} else if (state->tokens[i].type == number){
			// oops i need to convert strings to numbers - do that later...
			if (state->tokens[i].len == 1){
				data[(*size)++] = BC_SMALL_NUMBER;
				data[(*size)++] = state->source->data[state->tokens[i].ofs] - '0';
			} else if (state->tokens[i].len == 2){ 
				data[(*size)++] = BC_SMALL_NUMBER;
				data[(*size)++] = 10*(state->source->data[state->tokens[i].ofs] - '0') + (state->source->data[state->tokens[i].ofs+1] - '0');
			} else {
				double number = 0;
				size_t decimals = 0;
				for (size_t j = 0; j < state->tokens[i].len; ++j){
					char c = state->source->data[state->tokens[i].ofs + j];
					number *= 10;
					if (is_number(c)){
						number += c - '0';
						if (decimals > 0) {
							++decimals;
						}
					} else if (c == '.'){
						number /= 10;
						++decimals;
					}
				}
				for (size_t j = 0; j < decimals-1; ++j)
					number /= 10;
					
				iprintf("Number := %f", number);
				int fp = (int)(number * 4096.0);
				
				// large number
				data[(*size)++] = BC_NUMBER;
				data[(*size)++] = 0x0; //ignored
				data[(*size)++] = (fp >> 24) & 0xff;
				data[(*size)++] = (fp >> 16) & 0xff;
				data[(*size)++] = (fp >> 8) & 0xff;
				data[(*size)++] = (fp >> 0) & 0xff;
			}
		} else if (state->tokens[i].type == string){
			data[(*size)++] = BC_STRING;
			data[(*size)++] = state->tokens[i].len;
			for (int j = 0; j < state->tokens[i].len; ++j){
				data[(*size)++] = state->source->data[state->tokens[i].ofs + j];
			}
			if (*size % 2){
				(*size)++; // pad one null to keep instructions aligned
			}
		} else if (state->tokens[i].type == operation) {
			data[(*size)++] = BC_OPERATOR;
			
			data[(*size)++] = tok_in_str_index(bc_conv_operations, state->source->data, &state->tokens[i]);
			
		} else if (state->tokens[i].type == function){
			data[(*size)++] = BC_FUNCTION;
			
			data[(*size)++] = tok_in_str_index(functions, state->source->data, &state->tokens[i]);
		} else if (state->tokens[i].type == name) {
			//TODO: actual ID calculation
			//Find variable if it exists in table
			//Create variable if it does not exist
			//Get ID based on this index
			//Optional: Maybe have a small name cache?
			data[(*size)++] = BC_VARIABLE_NAME;
			data[(*size)++] = state->tokens[i].len;
			for (int j = 0; j < state->tokens[i].len; ++j){
				data[(*size)++] = state->source->data[state->tokens[i].ofs + j];
			}
			if (*size % 2){
				(*size)++; // pad one null to keep instructions aligned
			}
		} else {
			iprintf("Unknown token: ");
			print_token(state, state->tokens[i]);
		}
	}
	state->token_i = 0;
}

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
void tok_eval(struct tokenizer* state){
	struct eval e = {0};
	
	for (size_t i = 0; i < state->token_i; ++i){
		u16 prio = state->tokens[i].prio;
		iprintf("[tok_eval]");
		print_token(state, state->tokens[i]);
		
		if (state->tokens[i].type == operation && prio % 8 == 0) {
			// ()[] removed
		} else if (state->tokens[i].type == operation && prio == 1) {
			// comma or semicolon
			tok_eval_clean_stack(&e, prio);
			e.result[e.result_i++] = state->tokens[i];
		} else if (state->tokens[i].type == command || prio > 0){
			// operator, function or command
			if (!e.op_i || e.op_stack[e.op_i-1]->prio <= prio){
				e.op_stack[e.op_i++] = &state->tokens[i];
			} else {
				tok_eval_clean_stack(&e, prio);
				e.op_stack[e.op_i++] = &state->tokens[i];
			}
		} else if (state->tokens[i].type == newline){ 
			// ignore
		} else {
			e.result[e.result_i++] = state->tokens[i];
		}
	}
	
	//cleanup
//	e.op_i--;
//	e.value_i--;
	tok_eval_clean_stack(&e, 0);
	
	iprintf("Result size: %ld\n",e.result_i);
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
			char c = state->source->data[state->tokens[i].ofs];
			bool single = state->tokens[i].len == 1;
			switch (c){
				case ',':
				case ';':
					state->tokens[i].prio = nest + 1;
					break;
				case 'A':
				case 'O':
				case 'X':
					state->tokens[i].prio = nest + 2;
					break;
				case '<':
				case '>':
					state->tokens[i].prio = nest + 3;
					break;
				case '=':
					state->tokens[i].prio = single ? nest + 0 : nest + 3;
					break;
				case '!':
					state->tokens[i].prio = single ? nest + 6 : nest + 3;
					break;
				case '+':
					state->tokens[i].prio = nest + 4;
					break;
				case '-':
					if ((i>0 && state->tokens[i-1].type == number) ||
							state->tokens[i-1].type == name ||
							state->source->data[state->tokens[i-1].prio] % 8 == 0){
						state->tokens[i].prio = nest + 4;
					} else {
						state->tokens[i].prio = nest + 6;
					}
					break;
				case '*':
				case '/':
				case '%':
					state->tokens[i].prio = nest + 5;
					break;
				case 'N':
					state->tokens[i].prio = nest + 6;
					break;
				case '(':
				case '[':
					if (i>0 && state->tokens[i-1].type == name){
						state->tokens[i-1].prio = nest + 7;
					}
					nest += 8;
					state->tokens[i].prio = nest;
					break;
				case ']':
				case ')':
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

void tok_none(struct tokenizer* state){
	// This state represents: Whitespace and default state
	// This state can transition to:
	char c = state->source->data[state->cursor];
	
	if (is_name_start(c)){
		tok_name(state);
	} else if (c == '?'){
//		state->state = TKR_PRINT; //print requires special parsing...
		tok_single(state, command);
	} else if (c == '\r' || c == ':'){
		tok_single(state, newline);
		state->state = TKR_CONVERT;
	} else if (c == ' '){
		//whitespace
		state->cursor++;
	} else if (c == '"'){
		tok_string(state);
	} else if (is_number(c) || c == '.') {
		tok_number(state);
	} else if (c == ',' || c == ';' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
			c == '(' || c == ')' || c == '[' || c == ']'){
		tok_single(state, operation);
	} else if (c == '<' || c == '=' || c == '>' || c == '!'){
		tok_single(state, operation);
		// check for <= == >= !=
		if (state->source->data[state->cursor] == '='){
			state->tokens[state->token_i - 1].len++;
			state->cursor++;
		}
	} else {
		iprintf("Unknown transition from NONE on %c\n", c);
		state->cursor++;
	}
}

void tok_print(struct tokenizer* state){
	char c = '\r';
	do {
		tok_expr(state);
//?		state->cursor++;
	} while (c != '\r');
	state->state = TKR_CONVERT;
}

void tok_expr(struct tokenizer* state){
//	iprintf("Entered expr\n");
	char c;
	do {
		c = state->source->data[state->cursor];
		if (is_name_start(c)){
			tok_name(state);
		} else if (is_number(c)){
			tok_number(state);
		} else if (c == '"'){
			tok_string(state);
		} else if (c == ','){
			tok_single(state, operation);
		} else if (c != '\r'){
			//skip!
			iprintf("Unknown state %d\n", state->source->data[state->cursor]);
//			state->cursor++;
		}
	} while (c != '\r');
//	iprintf("Exited expr\n");
}

void tok_name(struct tokenizer* state){
	state->tokens[state->token_i].type = name;
	state->tokens[state->token_i].len = 0;
	state->tokens[state->token_i].ofs = state->cursor;
	do {
		state->tokens[state->token_i].len++;
		state->cursor++;
	} while (is_name(state->source->data[state->cursor]));
	//additional checking for special strings
	if (0 < tok_in_str_index(commands, state->source->data, &state->tokens[state->token_i])){
		state->tokens[state->token_i].type = command;
	} else if (0 < tok_in_str_index(functions, state->source->data, &state->tokens[state->token_i])){
		state->tokens[state->token_i].type = function;
	} else if (0 < tok_in_str_index(operations, state->source->data, &state->tokens[state->token_i])){
		state->tokens[state->token_i].type = operation;
	} else if (0 < tok_in_str_index(labels, state->source->data, &state->tokens[state->token_i])){
		state->tokens[state->token_i].type = label;
	} else if (0 < tok_in_str_index(comments, state->source->data, &state->tokens[state->token_i])){
		state->tokens[state->token_i].type = comment;
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

// I have taken one class on languages so let's see if I learned anything here
// 
// state		on				to			
// [default]	[A-Za-z_]		[alphanum]	
// [default]	[?]				[PRINT]
// [PRINT]		[]
// [operator]	[+-*/%<=>!]		[operator]	
// [alphanum]	[A-Za-z0-9_]	[alphanum]	
// [alphanum]	[+-*/%<=>!]		[operator]	
// 
// 
// 
// 
// 
// 
// 

// instructions:
//  [args] cmd
//  [args] func
//  sysvar
//  [args] op
//  label -> index into instr
//  strings -> index into strings
//  numbers -> encode
//  separator -> none, used to delimit special parsing cases I guess
//  comment -> discard or use as error message
//  variable -> index into vars

// #1 #1 + c
// f
// s
// o
// @
// "
// #
// :
// '
// v
