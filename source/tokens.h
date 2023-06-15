#pragma once

#include "common.h"

static const u32 LINE_MAX = 100;

extern const char* commands;
extern const char* functions;
extern const char* operations;
extern const char* bc_conv_operations;

// Program source loaded from ex. PTC file?
struct program {
	// Size of program (characters/bytes)
	u32 size;
	// Program data
	char* data;
};

enum tokenizer_state {
	TKR_NONE,
	TKR_NAME,
	TKR_PRINT,
	TKR_NUMBER,
	TKR_ARGS,
	TKR_CONVERT,
	TKR_EXPR,
};

struct pair {
	char name[16];
	u32 value;
};

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
		newline
	} type;
	u32 ofs;
	u8 len;
	u16 prio;
};

struct tokenizer {
	// Program info
	struct program* source;
	// Current tokenizer state
	enum tokenizer_state state;
	// Current character
	u32 cursor;
	
	// Tokens for a line
	struct token tokens[100]; //max possible per line
	// Current token index
	u32 token_i;
	
	// Values obtained while parsing
	// TODO: dynamic size?
	// Label addresses
	u32 label;
	struct pair labels[4096];
	// Variable indices
	u32 var;
	struct pair vars[4096];
	
	struct program* output;
};

void tokenize(struct program* src, struct program* output);

void tok_none(struct tokenizer* state);

void tok_print(struct tokenizer* state);

void tok_expr(struct tokenizer* state);

void tok_name(struct tokenizer* state);

void tok_string(struct tokenizer* state);

void tok_number(struct tokenizer* state);

void tok_convert(struct tokenizer* state);

void tok_prio(struct tokenizer* state);

void tok_eval(struct tokenizer* state);

void tok_code(struct tokenizer* state);

