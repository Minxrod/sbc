#include "tokens.h"

#include "ptc.h"
#include "program.h"
#include "strs.h" // for character id functions
#include "error.h"
#include "interpreter/label.h" // for adding label entry
#include "data.h" // for BC_DATA_DELIM

#include "system.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

#define MAX_SPECIAL_NAME_SIZE 8

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
"EXEC    FILES   GBOX    GCIRCLE GCLS    GCOLOR  GCOPY   GDRAWMD GFILL   GLINE   "
"GPAGE   GPAINT  GPSET   GPRIO   GPUTCHR ICONCLR ICONSET "
"KEY     LIST    LOAD    NEW     PNLSTR  PNLTYPE "
"READ    REBOOT  RECVFILERENAME  RESTORE RSORT   RUN     SAVE    SENDFILESORT    "
"SPANGLE SPANIM  SPCHR   SPCLR   SPCOL   SPCOLVECSPHOME  SPOFS   SPPAGE  SPREAD  SPSCALE "
"SPSET   SPSETV  SWAP    TMREAD  "
// not "supported" really, but some programs still used this to do things like detect region
"TALK    TALKSTOP"
"POKE    POKEH   POKEB   MEMCOPY MEMFILL ";

const char* functions =
"ABS     ASC     ATAN    BGCHK   BGMCHK  BGMGETV BTRIG   BUTTON  CHKCHR  CHR$    COS     "
"DEG     EXP     FLOOR   GSPOIT  HEX$    ICONCHK INKEY$  INSTR   LEFT$   LEN     LOG     "
"MID$    PI      POW     RAD     RIGHT$  RND     SGN     SIN     SPCHK   SPGETV  SPHIT   "
"SPHITRC SPHITSP SQR     STR$    SUBST$  TAN     VAL     "
// not implemented but still useful
"TALKCHK "
"PEEK    PEEKH   PEEKB   ADDR    PTR$    ";

const char* operations =
"AND     OR      XOR     NOT     ";

const char* bc_conv_operations = 
"+       ,       -       *       /       ;       =       (-)     "
"==      !=      <       >       <=      >=      "
"%       "
"AND     OR      XOR     NOT     !       "
"(       )       [       ]       ";

const char* sysvars = 
"TRUE    FALSE   CANCEL  VERSION "
"TIME$   DATE$   MAINCNTLMAINCNTH"
"FREEVAR FREEMEM PRGNAME$PACKAGE$RESULT  "
"TCHST   TCHX    TCHY    TCHTIME "
"CSRX    CSRY    TABSTEP "
"SPHITNO SPHITX  SPHITY  SPHITT  "
"KEYBOARDFUNCNO  ICONPUSEICONPAGEICONPMAX"
"ERL     ERR     "
"MEM$    "
// extension
"MEMSAFE ";

const char* labels = "LABEL   ";

const char* comments = "REM     ";

// * = any (don't check/special case)
// N = number
// n = number var
// S = string
// s = string var
// A = array
// a = array list [only SORT,RSORT should use this?]
// v = variable list
// E = both
// L = label (includes string)
// l = label list
// 0 = Nothing (no argument) (must be first in list)
// ; = semicolon
// < = comma
// , = separates valid formations
// Note that everything is ordered from short to long.
const char* cmd_format[] = {
	"*","NN","N,NN","*", //DIM
	"*","*","*","0,n", //NEXT
	"N","*","*","*", //ENDIF
	"L,l","L,l","N","0", //RETURN
	"0","0", //STOP
	"0","*","0","N","N", //WAIT
	"v,S;v","s,S;s", //LINPUT
	"S","0,N,NN,NNN,NNNN", //BEEP
	"NNNN","0,N","NNNNNNN","NNNNNN,NNNNNS,NNNNNNNNN","0,N", //BGMCLEAR
	// BGMPLAY confirmed to max out at nine string arguments
	// Weird bug: going over causes the MML to be parsed incorrect?
	"N,S,NN,SS,NNN,SSS,SSSS,SSSSS,SSSSSS,SSSSSSS,SSSSSSSS,SSSSSSSSS", //BGMPLAY
	"NS,NNS,NNNNNS,NNNNNNS","NS,NSS,NSSS,NSSSS,NSSSSS,NSSSSSS,NSSSSSSS,NSSSSSSSS,NSSSSSSSS,NSSSSSSSSS",//BGMSET
	"NL","NNN","0,N,NN","N,NN", //BGMVOL
	"NNN,NNNN","N","NNNN,NNNS,NNNNNNN","NNNn,NNNs,NNNnnnn","N,NNN","S","SNs", //CHRREAD
	"SNS","0","0,S,SN","SNnnn","SNS","0", //CONT
	"*","S","Snnn","S","0,S,SS,SSS,SSSS,SSSSS","NNNN,NNNNN", //GBOX
	"NNN,NNNN,NNNNNN","0,N","N","NNNNNNN,NNNNNNNN","N","NNNN,NNNNN","NNNN,NNNNN",//GLINE
	"N,NNN","NN,NNN,NNNN","NN,NNN","N","NNSNNN","0,N","NN",//ICONSET
	"NS","","S,SN","0",//NEW
	"NNS,NNSN","S","v","","","SS",//RENAME
	"L","NNa","0","S","","NNa","NN,NNN,NNNN",//SPANGLE
	"NNN,NNNN","NN,NNNNNN","0,N","NNNNNN,NNNNNNN","NNN","NNN","NNN,NNNN","N",//SPPAGE
	"NN,NNN,NNNN,NNNNN,NNNNNN","NN,NNN","NNNNNN,NNNNNNNN","NNN","nn,ss",//SWAP
	"Snnn",//TMREAD
	"S", "0", // TALK, TALKSTOP
	"NN","NN","NN","NNN","NNN", //MEMFILL
};

const char* func_format[] = {
	"N","S","N,NN","N","0,N","NN","0","0,N",//BUTTON
	"NN","N","N","N","","N","NN,NNN","N,NN","0",//ICONCHK
	"0","SS,SSN","SN","S","N","SNN","0","NN","N",//RAD
	"SN","N","N","N","N","NN","N,NN","",//SPHITRC
	"NN","N","N","SNNS","","S",//VAL
	"0",
	"N","N","N","S,SN","N" // PTR
};

const char func_return[] = "NNNNNNNN"
"NSNNNNNSN"
"SNSNNSNNN"
"SNNNNNNN"
"NNSSNN"
"N"
"NNNNS";

const char* op_format[] = {
	"NN,SS","*","NN","NN,SN","NN","*", //;
	"nN,sS","N", //(-)
	"NN,SS","NN,SS","NN,SS","NN,SS","NN,SS","NN,SS", //>=
	"NN",//%
	"NN","NN","NN","N","N",//!
	"*","*","*","*"
};


// Note: Expects null-terminated string
int tok_in_str_index(const char* str, const char* data, struct token* tok){
	int index;
	size_t start = 0;
	do {
		index = start / MAX_SPECIAL_NAME_SIZE;
		for (size_t i = 0; i < tok->len; ++i){
			// Allows matching lowercase forms of commands, etc.
			char c = data[tok->ofs + i];
			if (c >= 'a' && c <= 'z') {
				c = c - 'a' + 'A';
			}
			if (c != *(str + i)){
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
	if (t.type != command && t.type != function && t.type != operation && t.type != sysvar && t.type != first_of_line_command){
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
		const char* names = (t.type == command || t.type == first_of_line_command) ? commands : 
		                     t.type == function ? functions :
		                     t.type == sysvar ? sysvars :
		                     bc_conv_operations;
		char name[MAX_SPECIAL_NAME_SIZE+1] = {0};
		for (size_t i = 0; i < MAX_SPECIAL_NAME_SIZE; ++i){
			name[i] = names[MAX_SPECIAL_NAME_SIZE*t.cmd + i];
		}
		iprintf("id:%d type:%d cmd:%s", (int)t.cmd, t.type, name);
		if (t.prio) {
			iprintf("prio:%02d ", t.prio);
		} else {
			iprintf("        ");
		}
	}
	iprintf("\n");
}

int tokenize(struct program* src, struct bytecode* out){
	return tokenize_full(src, out, NULL, 0);
}

int tokenize_full(struct program* src, struct bytecode* out, void* system, int opts){
	struct tokenizer state = {0};
	state.source = src;
	state.output = out;
	state.state = TKR_NONE;
	state.opts = opts;
	state.system = system;
	if (!system){
		// Validate that system pointer is not used
		if ((opts & TOKOPT_SYSTEMLESS) != opts){
			return ERR_INVALID_OPTS;
		}
	} else {
		if (opts & TOKOPT_VARIABLE_IDS){
			state.system->vars.clear_values = true;
		}
	}
	iprintf("Selected opts: %x\n", state.opts);
	state.cursor = 0;
	state.is_comment = 0;
	state.lines_processed = 0;
	int error = ERR_NONE;
	// Re-initialize bc
	out->size = 0;
	reset_label(&out->labels);
	int old_cursor;
	while (state.cursor < state.source->size){
		old_cursor = state.cursor;
		error = tok_none(&state);
		if (error != ERR_NONE){
			if (opts & TOKOPT_STRICT_ERROR) break;
			state.output->data[state.output->size++] = BC_ERROR;
			state.output->data[state.output->size++] = error;
			iprintf("Error at: %d\n", state.output->size);
			while (state.source->data[state.cursor++] != '\r');
			error = ERR_NONE;
			state.token_i = 0;
			continue;
		}
		if (state.state == TKR_CONVERT){
			iprintf("line: %.*s\n", (int)state.cursor - state.tokens[0].ofs, &state.source->data[state.tokens[0].ofs]);
//			for (size_t i = 0; i < state.token_i; ++i){
//				iprintf("[line_token] ");
//				print_token(&state, state.tokens[i]);
//			}
			error = tok_convert(&state);
			if (error != ERR_NONE){
				if (opts & TOKOPT_STRICT_ERROR) break;
				state.output->data[state.output->size++] = BC_ERROR;
				state.output->data[state.output->size++] = error;
				error = ERR_NONE;
				state.token_i = 0;
				iprintf("Error at: %d\n", state.output->size);
			}
		}
	}
	if (error) {
		if (system){
			int line_view = state.cursor - old_cursor;
			line_view = line_view > 31 ? 31 : line_view;
			memcpy(state.system->exec.error_info, &state.source->data[old_cursor], line_view);
			state.system->exec.error_info[line_view+1] = '\0';
		}
		iprintf("tokenize error: %s\n", error_messages[error]);
	}
	
	iprintf("Current tokens:\n");
	for (size_t i = 0; i < 100; ++i){
		if (state.tokens[i].len == 0)
			break;
		print_token(&state, state.tokens[i]);
	}
	return error;
}

int tok_convert(struct tokenizer* state){
	// conversion of line returns to reading characters when done
	state->state = TKR_NONE;
	// convert the tokens into bytecode
	// this can be done per-line
	
	assert(state->token_i);
	if (state->tokens[0].type == label){
		// Add a label pointing to current index
		if (!add_label(&state->output->labels, &state->source->data[state->tokens[0].ofs], state->tokens[0].len, state->output->size)){
			return ERR_LABEL_ADD_FAILURE;
		}
		// option: Don't tokenize labels into code; rely only on table entries?
		if (state->opts & TOKOPT_NO_LABELS){
			// don't compile label into an actual statement
			// This also breaks the line_length calculation, but that doesn't actually do anything useful right now?
			state->token_i = 0; // mark tokens as used
			return ERR_NONE;
		}
	}
	
	tok_prio(state);
	
	tok_eval(state);
	
	int test_res = tok_test(state);
	if (test_res != ERR_NONE) return test_res;
	
	int line_begin = state->output->size;
	int code_res = tok_code(state);
	if (code_res != ERR_NONE) return code_res;
	int line_end = state->output->size;
	int line_length = line_end - line_begin;
	state->output->line_length[state->lines_processed++] = line_length;
	
	return ERR_NONE;
}

/// char* stack is a string of length stack_len
/// valid is a null-terminated string
/// true indicates the command is valid
bool check_cmd(const char* stack, int stack_len, const char* valid){
	assert(stack);
	assert(valid);
	// Check if only valid state is zero-arg and if zero arg is met 
	if (valid[0] == '0' && valid[1] == '\0') return stack_len == 0;
	// Check if anything goes and stack is empty
	if (stack_len == 0) return valid[0] == '*' || valid[0] == '0';
	assert(stack_len); // should be valid here
	
	iprintf("stack_len=%d\n", stack_len);
	int valid_len = strlen(valid);
	int stack_i = 0;
	bool is_valid = true;
	for (int valid_i = 0; valid_i < valid_len;){
		char s = stack[stack_i++];
		char v = valid[valid_i++];
		iprintf(" %d:%d s=%c ?= v=%c valid=%d\n", stack_i, valid_i, s, v, is_valid);
		if (v == ',') {
			if (is_valid && stack_i > stack_len) return true;
			stack_i = 0;
			is_valid = true;
			continue;
		}
		
		if (stack_i > stack_len){
			if (v != '\0' && v != 'l' && v != 'v'  && v != 'a'){
				return false; // sequence was incomplete
			}
			return is_valid;
		}
		
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
				if (s == 'S' || s == 's' || s == 'L' || s == '<') {
					valid_i--; // read as many labels as needed
					break;
				}
				is_valid = false;
				break;
				
			case 'v':
				if (s == 'n' || s == 's') {
					valid_i--; // read as many vars as needed
					break;
				}
				is_valid = false;
				break;
				
			case 'A':
				if (s == 'A') break; 
				is_valid = false;
				break;
				
			case 'a':
				if (s == 'A'){
					valid_i--;
					break;
				}
				is_valid = false;
				break;
				
			case '*':
				return true;
				
			case '0':
				is_valid = false;
				break;

			case ';':
				if (s == ';') break;
				is_valid = false;
				break;

			default:
				iprintf("Invalid format string.\n");
				return false;
		}
	}
	if (stack_i != stack_len) return false;
	return is_valid;
}

int tok_test(struct tokenizer* state){
	// This is argument count and argument type validation.
	char stack[40];
	u8 stack_i = 0;
	int argc = 0;
	stack[0] = 0; // Fixes undefined reads detected via -fanalyzer
	
	const char* valid; // Local used for some cases
	bool is_valid; // Used by some cases
	
	if (state->token_i && (state->tokens[0].type == command || state->tokens[0].type == first_of_line_command) && state->tokens[0].cmd == CMD_DATA)
		return ERR_NONE; // assumed valid
	// TODO:CODE:MED Move to tok_name/tok_data or something for validation instead of these special cases
	
	iprintf("Validation:\n");
	for (u8 i = 0; i < state->token_i; ++i){
		iprintf("Stack: %.*s\n", stack_i, stack);
		iprintf("%d: ", i);
		print_token(state, state->tokens[i]);
		switch (state->tokens[i].type){
			case number:
			case base_number:
				stack[stack_i++] = 'N';
				break;
			
			case string:
				stack[stack_i++] = 'S';
				break;
			
			case array_actual:
				// only intended for SORT, RSORT currently
				stack[stack_i++] = 'A';
				break;
			
			case first_of_line_command:
			case command:
				{
				u8 cmd = state->tokens[i].cmd;
				if (cmd == CMD_DATA) break; // parsing is done
				const char* valid = cmd_format[cmd];
				if (!check_cmd(stack, stack_i, valid)){
					// Set error state on invalid command
					print_token(state, state->tokens[i]);
					iprintf("Stack: %d %.*s\n", stack_i, stack_i, stack);
					return ERR_TEST_COMMAND;
				}
				// relies on variable carrying through...
				// FOR: for assignment
				// TO: for BC_BEGIN_LOOP
				if (cmd != CMD_FOR && cmd != CMD_TO){
					stack_i = 0;
				}
				}
				break;
				
			case operation:
				{
				u8 prio = state->tokens[i].prio;
				is_valid = true;
				if (state->tokens[i].cmd == OP_COMMA){
					stack[stack_i++] = '<';
				} else if (state->tokens[i].cmd == OP_SEMICOLON){
					stack[stack_i++] = ';';
				} else if (prio % 8 != 6){
					// binary
					if (stack_i < 2){
						is_valid = false;
					} else {
						valid = op_format[state->tokens[i].cmd];
						is_valid = check_cmd(&stack[stack_i-2], 2, valid);
						stack_i -= 2; // +2 -1
						// note: some binary ops will have the type of the first argument
						if (state->tokens[i].cmd >= OP_EQUAL && state->tokens[i].cmd <= OP_GREATER_EQUAL){
							// these ones don't
							stack[stack_i++] = 'N';
						} else if (state->tokens[i].cmd != OP_ASSIGN){
							stack[stack_i++] &= 0x5f;  //binary op returns value type of first argument. unless it's =, that doesn't return anything.
						}
					}
				} else {
					// unary: all of these take numbers, so
					if (stack_i < 1){
						is_valid = false;
					} else {
						valid = "N";
						is_valid = check_cmd(&stack[stack_i-1], 1, valid);
						// +1 -1
						stack_i--;
						stack[stack_i++] = 'N';  //any unary op returns number
					}
				}
				if (!is_valid){
					// Set error state on invalid command
					print_token(state, state->tokens[i]);
					iprintf("Stack: %d %s\n", stack_i, stack);
					return ERR_TEST_OPERATION;
				}
				}
				break;
				
			case function:
				if (argc){
					stack_i -= argc;
				}
				u8 func = state->tokens[i].cmd;
				valid = func_format[func];
				is_valid = check_cmd(&stack[stack_i], argc, valid);
				if (!is_valid){
					// Set error state on invalid command
					print_token(state, state->tokens[i]);
					iprintf("Stack: %d %s\n", stack_i, stack);
					return ERR_TEST_FUNCTION;
				}
				stack[stack_i++] = func_return[func];
				argc = 0;
				break;
				
			case array_name:
			case name:
				if (argc){
					// process argc counts first
					const char* arr_valid = "N,NN";
					is_valid = check_cmd(&stack[stack_i-argc], argc, arr_valid);
					if (!is_valid){
						// arguments to array access are bad
						iprintf("Bad array arguments!\n");
						iprintf("Stack: %d %s\n", stack_i, stack);
						return ERR_TEST_ARRAY;
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
				
			case dim_arr:
				if (argc){
					// process argc counts first
					const char* dim_valid = "N,NN";
					is_valid = check_cmd(&stack[stack_i-argc], argc, dim_valid);
					if (!is_valid){
						// arguments to array access are bad
						iprintf("Bad array arguments!\n");
						iprintf("Stack: %d %s\n", stack_i, stack);
						return ERR_TEST_ARRAY;
					}
					stack_i -= argc;
					argc = 0;
				}
				break;
				
			case sysvar:
				{
				u8 id = state->tokens[i].cmd;
				if (id == SYS_TIME || id == SYS_DATE || id == SYS_PRGNAME
					|| id == SYS_PACKAGE || id == SYS_MEM){
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
				
			case label: // doesn't get added to stack if it's an actual label
			case comment: // ignored
			case newline: // ignored
				break;
				
			case loop_begin:
				stack_i = 0; // consumes stack elements
				break;
				
			default:
				iprintf("Unknown command type=%d\n", state->tokens[i].type);
				return ERR_TEST_UNKNOWN;
		}
	}
	
	if (stack_i)
		return ERR_TEST_STACK;
	return ERR_NONE;
}

// none of these values will exceed 100[?]

int64_t tok_to_num(struct tokenizer* state, struct token* t){
	int64_t number = u8_to_num((u8*)&state->source->data[t->ofs], t->len);
	iprintf("Number := %f", (double)number/4096.0);
	return number;
}

int tok_data(struct tokenizer* state, struct token start_t, char* data, idx* size){
	// Special rules for DATA: write contents as string of sorts
	int start = start_t.ofs+4;
//	while (state->source->data[start] == ' '){
//		++start;
//	} // skip to first non-whitespace
	int end = state->cursor;

	data[(*size)++] = BC_DATA;
	char* data_size = &data[(*size)++]; // write as copying characters
	*data_size = 0;

	iprintf("Data [%d,%d)\n", start, end);
	enum bc_data_state {
		// Skip spaces until a different character is found
		BC_DATA_NONE,
		// Read characters until a comma, space, or newline is found
		BC_DATA_STRING,
		// Read characters until a closing quote is found
		BC_DATA_QUOTED,
		// Search spaces until delim or EOL
		BC_DATA_WAIT_NEXT,
	} data_state = BC_DATA_NONE;
	for (int j = start; j < end; ++j){
		char c = state->source->data[j];

		switch (data_state){
			case BC_DATA_NONE:
				if (c == '"'){
					data_state = BC_DATA_QUOTED;
					continue;
				}
				if (c == '\r' || c == ','){
					c = BC_DATA_DELIM;
					break;
				}
				if (c == ' ') continue; // keep searching
				data_state = BC_DATA_STRING;
				break;

			case BC_DATA_QUOTED:
				if (c == '"'){
					data_state = BC_DATA_NONE;
					continue;
				}
				if (c == '\r'){
					c = BC_DATA_DELIM;
					break;
				}
				// Add character to output
				break;

			case BC_DATA_WAIT_NEXT:
				if (c == ' ') continue;
				if (c == ',' || c == '\r'){
					data_state = BC_DATA_NONE;
					c = BC_DATA_DELIM;
					break;
				}
				c = BC_DATA_ERROR;
				break;

			case BC_DATA_STRING:
				// Unquoted string
				if (c == ' '){ // String ends
					data_state = BC_DATA_WAIT_NEXT;
					continue;
				}
				if (c == ',' || c == '\r'){ // String ends
					data_state = BC_DATA_NONE;
					c = BC_DATA_DELIM;
					break;
				}
				break;
		}

		// Add character to output
		data[(*size)++] = c;
		(*data_size)++;
	}

	if (*size % 2){
		data[(*size)++] = 0; // pad one null to keep instructions aligned
	}

	state->token_i = 0;
	return ERR_NONE;
	// End of DATA special rules
}

int tok_code(struct tokenizer* state){
	// assume the order is fixed
	char* data = (char*)state->output->data;
	idx* size = &state->output->size;
	uint64_t sys_validate = 0;
	
	for (size_t i = 0; i < state->token_i; ++i){
		switch (state->tokens[i].type){
			case command:
			case first_of_line_command:;
				bool first_of_line = state->tokens[i].type == first_of_line_command;
				state->tokens[i].type = command;
				if (state->tokens[i].cmd == CMD_DATA){
					tok_data(state, state->tokens[i], data, size);
				} else {
					// convert token to command
	//				iprintf("command: %d, %d\n", j, state->tokens[i].cmd);
//					(void)first_of_line;
					data[(*size)++] = first_of_line ? BC_COMMAND_FIRST : BC_COMMAND;
					data[(*size)++] = state->tokens[i].cmd;
				}
				break;
			
			case base_number:
				;
				int base = state->source->data[state->tokens[i].ofs+1] == 'H' ? 16 : 2;
				
				int64_t n = u8_to_number((u8*)&state->source->data[state->tokens[i].ofs+2], state->tokens[i].len-2, base, false);
				goto number_common;
				
			case number:
				n = tok_to_num(state, &state->tokens[i]);
				
			number_common:
//				iprintf("full: %lx\n", n);
				if (((n >> 31) & 0x1) != ((uint64_t)n >> 63)){
					if (n >= 0x080000000l && n <= 0x0ffffffff && state->tokens[i].type == number){
						// this is not an error for &H or &B, but is for number
						return ERR_OVERFLOW;
					}
				}
				if (n >= 0 && n < INT_TO_FP(100) && n == (n & (fixp)0xfffff000)){
					data[(*size)++] = BC_SMALL_NUMBER;
					data[(*size)++] = FP_TO_INT(n);
				} else {
					// TODO:PERF:LOW Store some numbers in less space
					// large number
					data[(*size)++] = BC_NUMBER;
					data[(*size)++] = 0x0; //ignored
					data[(*size)++] = ((u32)n >> 24) & 0xff;
					data[(*size)++] = ((u32)n >> 16) & 0xff;
					data[(*size)++] = ((u32)n >> 8) & 0xff;
					data[(*size)++] = ((u32)n >> 0) & 0xff;
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
				int op = state->tokens[i].cmd;
				if (state->tokens[i].prio % 8 == 6 && op == OP_SUBTRACT){
					op = OP_NEGATE;
				}
				data[(*size)++] = op;
				
				iprintf("%d", data[*size-1]);
				break;
				
			case function:
				data[(*size)++] = BC_FUNCTION;
				
				data[(*size)++] = state->tokens[i].cmd;
				break;
				
			case name:
			case array_name:
			case dim_arr:
			case array_actual:
				;
				// Note: This optimization relies on the assumption that single-char names map to IDs < 256.
				// This is because instructions are ALWAYS expected to compile smaller than the wide-char source
				// A [wide] -> (VA [regular var-name access] || iA [id-based access])
				// This can be broken if hash collisions lead to IDs greater than 256.
				bool is_array_type = state->tokens[i].type == array_name || state->tokens[i].type == array_actual;
				if ((state->opts & TOKOPT_VARIABLE_IDS) && state->tokens[i].type != dim_arr){
					// Get variable index in table
					int len = state->tokens[i].len;
					int t = state->source->data[state->tokens[i].ofs + len - 1] == '$' ? VAR_STRING : VAR_NUMBER;
					len -= t == VAR_STRING;
					if (is_array_type) t |= VAR_ARRAY;
					
					struct named_var* v = get_var(&state->system->vars, &state->source->data[state->tokens[i].ofs], len, t);
					if (state->system->vars.error != ERR_NONE){
						return state->system->vars.error;
					}
//					iprintf("var %p: type=%d name=%s data=%d\n", (void*)v, v->type, v->name, v->value.number);
					ptrdiff_t var_id = v - &state->system->vars.vars[0];
					// note: turns out pointer arithmetic essentially performs the index conversion for you
					// var_id contains the index into vars[] of this variable
					iprintf("%zd, %.*s\n", var_id, state->tokens[i].len, &state->source->data[state->tokens[i].ofs]);
					if (var_id < 256){
						data[(*size)++] = BC_VARIABLE_ID_SMALL;
						data[(*size)++] = (var_id & 0xff);
					} else {
						if (len == 1){
							// Length invariant failed. 
							return ERR_LENGTH_INVARIANT_BROKEN;
							// TODO:ERR:NONE Fallback to regular variable name structure instead?
							// (potential issues: CLEAR)
						}
						data[(*size)++] = BC_VARIABLE_ID;
						data[(*size)++] = (var_id & 0x0000ff);
						data[(*size)++] = (var_id & 0x00ff00) >> 8;
						data[(*size)++] = (var_id & 0xff0000) >> 16;
					}
				} else {
					u8 type = state->tokens[i].type == dim_arr ? BC_DIM : 
					          state->tokens[i].type == array_actual ? BC_ARRAY_NAME : 
					          BC_VARIABLE_NAME;
					data[(*size)++] = type;
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
				data[(*size)++] = state->tokens[i].cmd;
				// If sysvar is writable, add a check to ensure the value is limited to the valid range
				
				sys_validate |= ((uint64_t)1 << state->tokens[i].cmd);
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
				
			case comment:
				goto tok_code_exit;
				
			default:
				iprintf("Unknown token: ");
				print_token(state, state->tokens[i]);
				iprintf("\n");
				return ERR_UNKNOWN_TOKEN_TYPE;
		}
	}
	
tok_code_exit:
	// Validation must be placed at end to happen after assignment
/// Sysvars that allow assignment (and thus need validation)
#define SYSVAR_NEEDS_VALIDATE (\
		(1 << SYS_ICONPAGE) |\
		(1 << SYS_ICONPMAX) |\
		(1 << SYS_ICONPUSE) |\
		(1 << SYS_TABSTEP) |\
		(1 << SYS_MEM) |\
		((uint64_t)1 << SYS_MEMSAFE)\
	)

	if (sys_validate & SYSVAR_NEEDS_VALIDATE){
		for (int i = 0; i < 64; ++i){
			if (sys_validate & ((uint64_t)1 << i) & SYSVAR_NEEDS_VALIDATE){
				data[(*size)++] = BC_SYSVAR_VALIDATE;
				data[(*size)++] = i;
			}
		}
	}
	
	state->token_i = 0;
	return ERR_NONE;
}

/// Expression evaluation state.
///
/// Used for parsing expressions into executable order,
/// exclusively within tok_eval.
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
	bool is_sort = false;
	bool implicit_commas = true;
	
	iprintf("[tok_eval] Entry with %d unit(s)\n", state->token_i);
/*	if (state->token_i && state->tokens[0].type == command && state->tokens[0].cmd == CMD_DATA){
		iprintf("DATA special case\n");
		for (size_t j = 0; j < state->token_i; ++j){
			print_token(state, state->tokens[j]);
		}
		state->state = TKR_NONE; // Important: Tells tokenizer to read a new line
		return;
	}*/
	
	for (size_t i = 0; i < state->token_i; ++i){
		int prio = state->tokens[i].prio;
//		char first = state->source->data[state->tokens[i].ofs];
		int op = state->tokens[i].cmd;
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
					if (prev_type == name){
						state->tokens[i-1].type = array_name;
					} else if (prev_type != function && prev_type != dim_arr){
						e.argc_stack[e.argc_i] |= 0x80; // special indicator for "doesn't NEED argcount"
					}
				}
			} else { // assumed ) or ]
				// handle 0-args PI(), BUTTON() case
				if (state->tokens[i-1].type == operation && state->tokens[i-1].cmd == OP_OPEN_PAREN){
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
				// don't push commas unless they are prior to another comma or at the beginning/end of the line
				if (state->tokens[i].cmd == OP_SEMICOLON){
					e.result[e.result_i++] = state->tokens[i];
				} else { // must be a comma here
					if (!e.result_i || state->tokens[i+1].type == newline || (i>0 && state->tokens[i-1].prio % 8 == 1)){
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
				if (state->tokens[i].cmd == CMD_DIM){
				// command is DIM: set array-creation-mode for following tokens
					is_dim = true;
					continue; // do not compile as instruction
				} else if (state->tokens[i].cmd == CMD_PRINT){
					// Command is PRINT: semicolons are implied if omitted;
					// commas are always added
					implicit_commas = false;
				} else if (state->tokens[i].cmd == CMD_FOR){
					// Command is FOR: add instruction at end of FOR setup to
					// properly execute the loop
					is_for = true;
				} else if (state->tokens[i].cmd == CMD_IF){
					// Command is IF: add normally, but needs to indicate that
					// ENDIF should be added once line end is hit
					is_if = true;
				} else if (state->tokens[i].cmd == CMD_THEN){
					// THEN should be discarded: default IF behavior will be to
					// continue to next instruction, which can ignore THEN
					tok_eval_clean_stack(&e, 0);
					continue;
				} else if (state->tokens[i].cmd == CMD_ELSE){
					// ELSE should be placed immediately
					// similar to commas within a command, it is used as a
					// separator between TRUE/FALSE paths of the IF.
					tok_eval_clean_stack(&e, 0);
					e.result[e.result_i++] = state->tokens[i];
					continue;
				} else if (state->tokens[i].cmd == CMD_SORT || state->tokens[i].cmd == CMD_RSORT){
					// SORT, RSORT need arrays for arguments >2, so mark these functions
					// TODO:IMPL:NONE Determine a way to indicate arrays unambiguously (such as "A[]")
					// and make it an option
					is_sort = true;
				} else if (state->tokens[i].cmd == CMD_DATA){
					// DATA terminates the line - everything after it is parsed differently
					state->state = TKR_NONE; // Tell tokenizer to start new line
					tok_eval_clean_stack(&e, 0); // DATA must go after expression, so prior command(s) execute
					e.result[e.result_i++] = state->tokens[i]; // This marks the start of DATA for tok_code
					break;
				} else {
					// Any other "normal" command
					is_dim = false;
					// Note: is_for will not be always be reset because FOR
					// needs to be kept to the newline/end of FOR/TO/STEP command
					// including through other commands TO and STEP
					if (is_for && state->tokens[i].cmd != CMD_TO && state->tokens[i].cmd != CMD_STEP) {
						e.result[e.result_i++] = (struct token){.type=loop_begin, .ofs=0, .len=1, .prio=0};
						is_for = false;
					}
				}
				// This is a special case to mark commands that are located at
				// the start of a line (: counts for this purpose). This is needed to handle cases such as
				// FOR loops skipping to a NEXT, but only NEXTs that are at the beginning of the line.
				if (i == 0 || state->tokens[i-1].type == newline){
					state->tokens[i].type = first_of_line_command;
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
			if (state->tokens[i].type != operation || state->tokens[i].cmd != OP_COMMA)
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
		} else if (state->tokens[i].type == name && is_sort && e.argc_stack[0] >= 2){
			// convert variables to array when listed at end of SORT or RSORT
			// still need to treat them like other values though
			state->tokens[i].type = array_actual;
			e.result[e.result_i++] = state->tokens[i];
		} else {
			// values
			e.result[e.result_i++] = state->tokens[i];
		}
	}
	//cleanup
	tok_eval_clean_stack(&e, 0);
	
	if (is_if){
		e.result[e.result_i++] = (struct token){.type=command, .cmd=CMD_ENDIF, .ofs=0, .prio=0};
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
}

// take one line (to /r) and calculate priorities of tokens
void tok_prio(struct tokenizer* state){
	int nest = 0;
	for (size_t i = 0; i < state->token_i; i += 1){
		if (state->tokens[i].type == operation){
			u8 op_id = state->tokens[i].cmd;
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
								state->tokens[i-1].cmd > OP_OPEN_PAREN
							))){
						state->tokens[i].prio = nest + 4;
					} else {
						state->tokens[i].cmd = OP_NEGATE;
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
int tok_none(struct tokenizer* state){
	// This state represents: Whitespace and default state
	// This state can transition to:
	char c = state->source->data[state->cursor];
	
	if (c == '\r'){
		tok_single(state, newline);
		state->is_comment = false;
		state->state = TKR_CONVERT;
		// mark comment as ended
		return ERR_NONE;
	}
	
	if (state->is_comment) {
		state->cursor++;
		return ERR_NONE;
	}
	
	if (is_name_start(c)){
		return tok_name(state);
	} else if (c == '?'){
//		tok_single(state, command);
		state->tokens[state->token_i].type = command;
		state->tokens[state->token_i].cmd = CMD_PRINT;
		state->cursor++;
		state->token_i++;
	} else if (c == ':'){
		tok_single(state, newline);
	} else if (c == ' '){
		//whitespace is ignored
		state->cursor++;
	} else if (c == '"'){
		tok_string(state);
	} else if (c == '&') {
		tok_base_number(state);
	} else if (is_number(c) || c == '.') {
		tok_number(state);
	} else if (c == ',' || c == ';' || c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
			c == '(' || c == ')' || c == '[' || c == ']'){
		tok_single(state, operation);
		state->tokens[state->token_i-1].cmd = tok_in_str_index(bc_conv_operations, state->source->data, &state->tokens[state->token_i-1]);
	} else if (c == '<' || c == '=' || c == '>' || c == '!'){
		tok_single(state, operation);
		// check for <= == >= !=
		if (state->source->data[state->cursor] == '='){
			state->tokens[state->token_i - 1].len++;
			state->cursor++;
		}
		state->tokens[state->token_i-1].cmd = tok_in_str_index(bc_conv_operations, state->source->data, &state->tokens[state->token_i-1]);
	} else if (c == '@'){
		state->cursor++;
		tok_with_condition(state, is_name);
		if (state->tokens[state->token_i].len > 16){
			return ERR_LABEL_TOO_LONG;
		}
		state->tokens[state->token_i].type = label;
		state->token_i++;
	} else if (c == '\''){
		// read to newline
		state->cursor++;
		state->is_comment = true;
	} else {
		iprintf("Unknown transition from NONE on %c:%d\n", c, c);
		return ERR_UNKNOWN_TRANSITION;
	}
	return ERR_NONE;
}

int tok_label(struct tokenizer* state){
	if (state->source->data[state->cursor++] != ' '){
			return ERR_SYNTAX;
		}
		while (state->source->data[state->cursor] == ' '){
			state->cursor++;
		}
		if (!is_name(state->source->data[state->cursor])){
			return ERR_SYNTAX;
		}
		tok_with_condition(state, is_name);
		state->tokens[state->token_i].type = label;
		if (state->tokens[state->token_i].len > 16){
			return ERR_LABEL_TOO_LONG;
	}
	return ERR_NONE;
}

int tok_name(struct tokenizer* state){
	tok_with_condition(state, is_varname);
	//additional checking for special strings
	int index;
	int err = ERR_NONE;
	if (0 <= (index = tok_in_str_index(commands, state->source->data, &state->tokens[state->token_i]))){
		state->tokens[state->token_i].type = command;
		state->tokens[state->token_i].cmd = index; // overrides len
		if (index == CMD_DATA){
			// This runs to the end of the line similar to a comment
			// (and also has no effect when executed, like a comment)
			state->is_comment = true;
		}
	} else if (0 <= (index = tok_in_str_index(functions, state->source->data, &state->tokens[state->token_i]))){
		state->tokens[state->token_i].type = function;
		state->tokens[state->token_i].cmd = index; // overrides len
	} else if (0 <= (index = tok_in_str_index(bc_conv_operations, state->source->data, &state->tokens[state->token_i]))){
		// note that this is only the bitwise ops
		state->tokens[state->token_i].type = operation;
		state->tokens[state->token_i].cmd = index; // overrides len
	} else if (0 <= tok_in_str_index(labels, state->source->data, &state->tokens[state->token_i])){
		err = tok_label(state);
	} else if (0 <= tok_in_str_index(comments, state->source->data, &state->tokens[state->token_i])){
		state->tokens[state->token_i].type = comment;
		state->is_comment = true;
	} else if (0 <= (index = tok_in_str_index(sysvars, state->source->data, &state->tokens[state->token_i]))){
		state->tokens[state->token_i].type = sysvar;
		state->tokens[state->token_i].cmd = index; // overrides len
	}
	
	state->token_i++;
	return err;
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

void tok_base_number(struct tokenizer* state){
	state->tokens[state->token_i].type = base_number;
	state->tokens[state->token_i].len = 2;
	state->tokens[state->token_i].ofs = state->cursor;
	state->cursor += 2;
	do {
		state->tokens[state->token_i].len++;
		state->cursor++;
	} while (is_number(state->source->data[state->cursor])
	         || (state->source->data[state->cursor] >= 'A' && state->source->data[state->cursor] <= 'F'));
	state->token_i++;
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

