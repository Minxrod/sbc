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

// optimization options ("opt"s)
enum tokenizer_opts {
	TOKOPT_NONE = 0, // for readability if passing none
	TOKOPT_NO_LABELS = 1, // Doesn't generate L#... expressions for labels
	TOKOPT_VARIABLE_IDS = 2, // Generates IDs instead of named variables
};

// Optimizations that are allowed without access to a system pointer
#define TOKOPT_SYSTEMLESS (TOKOPT_NO_LABELS)

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
		base_number,
		array_name,
	} type;
	idx ofs;
	u8 len;
	u16 prio;
};

//struct program;
//struct ptc;

struct tokenizer {
	// Flags for the optimizer
	enum tokenizer_opts opts;
	// Current tokenizer state
	enum tokenizer_state state;
	// Tokens for a line
	struct token tokens[TOKENS_LINE_MAX]; //max possible per line
	// Current token index
	uint_fast8_t token_i;
	// Flag to indicate a comment is being parsed
	bool is_comment;
	
	
	// Program info
	struct program* source;
	// Current character within program source
	u32 cursor;
	
	// Output bytecode data, labels, line info
	struct bytecode* output;
	// Line count
	uint_fast16_t lines_processed;
	// System pointer
	struct ptc* system;
};

int tokenize(struct program* src, struct bytecode* output);

int tokenize_full(struct program* src, struct bytecode* output, void* system, int opts);

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

void tok_base_number(struct tokenizer* state);
