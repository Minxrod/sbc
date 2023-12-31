#pragma once

#include "common.h"

#define TOKENS_LINE_MAX 100

extern const char* commands;
extern const char* functions;
extern const char* operations;
extern const char* bc_conv_operations;
extern const char* sysvars;

enum tokenizer_state {
	TKR_NONE,
	TKR_NAME,
	TKR_PRINT,
	TKR_NUMBER,
	TKR_ARGS,
	TKR_CONVERT,
	TKR_EXPR,
};

#define LABEL_UNKNOWN ((u32)-1)

struct token {
	enum type {
		number,
		string,
		name,
		operation,
		command,
		function,
		label,
		comment,
		newline,
		dim_arr,
		arg_count,
		loop_begin,
		sysvar,
		label_string,
		first_of_line_command,
	} type;
	idx ofs;
	u8 len;
	u16 prio;
};

struct program;

struct tokenizer {
	// Program info
	struct program* source;
	// Current tokenizer state
	enum tokenizer_state state;
	// Current character
	u32 cursor;
	
	// Tokens for a line
	struct token tokens[TOKENS_LINE_MAX]; //max possible per line
	// Current token index
	uint_fast8_t token_i;
	
	bool is_comment;
	// Values obtained while parsing
	
	struct bytecode* output;
	
	uint_fast16_t lines_processed;
};

int tokenize(struct program* src, struct bytecode* output);

int tok_none(struct tokenizer* state);

void tok_name(struct tokenizer* state);

void tok_string(struct tokenizer* state);

void tok_number(struct tokenizer* state);

int tok_convert(struct tokenizer* state);

void tok_prio(struct tokenizer* state);

void tok_eval(struct tokenizer* state);

int tok_code(struct tokenizer* state);

/// Validate argument sequence
bool check_cmd(const char* stack, int stack_len, const char* valid);

/// Returns error code
int tok_test(struct tokenizer* state);
