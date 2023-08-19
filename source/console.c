#include "console.h"

#include "system.h"
#include "ptc.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// @warning Does not handle newlines!
void con_advance(struct console* c){
	c->x++;
	if (c->x >= CONSOLE_WIDTH){
		c->y++;
		c->x = 0;
	}
}

void con_newline(struct console* c){
	c->x = 0;
	c->y++;
	// TODO:PERF optimize (scroll all at once?)
	while (c->y >= CONSOLE_HEIGHT){
		c->y--;
		// newline: scroll console up
		// TODO:PERF optimize this (memcpy?)
		for (u32 i = 1; i < CONSOLE_HEIGHT; ++i){
			for (u32 j = 0; j < CONSOLE_WIDTH; ++j){
				c->text[i-1][j] = c->text[i][j];
				c->color[i-1][j] = c->color[i][j];
			}
		}
		for (u32 j = 0; j < CONSOLE_WIDTH; ++j){
			c->text[CONSOLE_HEIGHT-1][j] = 0;
			c->color[CONSOLE_HEIGHT-1][j] = c->col;
		}
	}
}

void con_init(struct console* c){
	c->tabstep = 4;
}

void con_put(struct console* c, u16 w){
	c->text[c->y][c->x] = w;
	c->color[c->y][c->x] = c->col;
	
	c->x++;
	if (c->x >= CONSOLE_WIDTH){
		con_newline(c);
	}
}

//TODO:PERF optimize via copying multiple lines at once for large strings?
//TODO:PERF memcpy instead of manual copy? probably nicer on cache...?
//TODO:PERF con_puts write directly to console via str_wide_copy?
//(color still separate here)

void con_puts(struct console* c, void* s){
	u16 buf[256];
	str_wide_copy(s, buf);
	// now the string is already wide chars for printing
	u32 len = str_len(s);
	for (size_t i = 0; i < len; ++i){
		con_put(c, buf[i]);
	}
}

void cmd_print(struct ptc* p){
//	struct stack* s = &p->stack;
	struct console* c = &p->console;
	u8 buf[16]; //S#-524287.999\0 max length is 2+12 chars
	buf[0]='S';
	
	u32 i = 0;
	while (i < p->stack.stack_i){
		struct stack_entry* e = &p->stack.entry[i];
		
		if (e->type & VAR_NUMBER){
			s32 x = VALUE_NUM(e);
			str_num(x, &buf[2]);
			buf[1] = strlen((char*)&buf[2]); //TODO:CODE remove this bit
			
			con_puts(c, buf);
		} else if (e->type & VAR_STRING) {
			con_puts(c, VALUE_STR(e));
		} else if (e->type & STACK_OP) { 
			if (e->value.number == OP_COMMA){
				// tab
				do {
					con_advance(c);
				} while (c->x % c->tabstep != 0);
				if (c->y == CONSOLE_HEIGHT){
					con_newline(c);
				}
			} else if (e->value.number == OP_SEMICOLON){
				// do nothing lol
			} else {
				// error
				p->exec.error = ERR_PRINT_INVALID_OP;
				return;
			}
		} else {
			// what
			p->exec.error = ERR_PRINT_INVALID_STACK;
			return;
		}
		i++;
	}
	if (p->stack.stack_i && p->stack.entry[p->stack.stack_i-1].type == STACK_OP){
		// no newline!
	} else {
		// newline!
		con_newline(c);
	}
	
	p->stack.stack_i = 0; //PRINT consumes all stack items
}

void cmd_color(struct ptc* p){
	struct console* c = &p->console;
	// TODO:ERR check arguments types, quantity
	if (p->stack.stack_i == 2){
		c->col = 0;
		c->col |= VALUE_NUM(stack_get(&p->stack, 0)) >> 12; // FG
		c->col |= (VALUE_NUM(stack_get(&p->stack, 1)) >> 12) << 4; // BG
	} else if (p->stack.stack_i == 1){
		c->col &= ~COL_FG_MASK; // clear FG value
		c->col |= (VALUE_NUM(stack_get(&p->stack, 0)) >> 12); // FG
	} else {
		p->exec.error = ERR_WRONG_ARG_COUNT;
	}
	p->stack.stack_i = 0;
}

void cmd_locate(struct ptc* p){
	struct console* c = &p->console;
	// TODO:ERR check arguments types, quantity
	if (p->stack.stack_i == 2){
		c->x = VALUE_NUM(stack_get(&p->stack, 0)) >> 12; // BG
		c->y = VALUE_NUM(stack_get(&p->stack, 1)) >> 12; // FG
	} else {
		p->exec.error = ERR_WRONG_ARG_COUNT;
	}
	p->stack.stack_i = 0;
}

void cmd_cls(struct ptc* p){
	struct console* c = &p->console;
	// TODO:ERR check arguments types, quantity
	for (int i = 0; i < CONSOLE_HEIGHT; ++i){
		for (int j = 0; j < CONSOLE_WIDTH; ++j){
			c->text[i][j] = 0;
			c->color[i][j] = 0;
		}
	}
}

#define INPUT_RETURN '\r'
#define INPUT_BACKSPACE '\b'
#define INPUT_UP 256
#define INPUT_DOWN 257
#define INPUT_LEFT 258
#define INPUT_RIGHT 259

u16 input_keys(struct ptc* p){
	struct input* i = &p->input;
	if (check_pressed_manual(i, BUTTON_ID_A, 15, 4)){
		return INPUT_RETURN;
	} else if (check_pressed_manual(i, BUTTON_ID_Y, 15, 4)){
		return INPUT_BACKSPACE;
	} else if (check_pressed_manual(i, BUTTON_ID_UP, 15, 4)){
		return INPUT_UP;
	} else if (check_pressed_manual(i, BUTTON_ID_DOWN, 15, 4)){
		return INPUT_DOWN;
	} else if (check_pressed_manual(i, BUTTON_ID_LEFT, 15, 4)){
		return INPUT_LEFT;
	} else if (check_pressed_manual(i, BUTTON_ID_RIGHT, 15, 4)){
		return INPUT_RIGHT;
	}
	// convert inputs
	return get_inkey(i); //returns 0 if no keys
}

void cmd_input(struct ptc* p){
	struct console* con = &p->console;
	// INPUT [prompt;]var[,var...]
	// Argument validation here
	if (!p->stack.stack_i){
		ERROR(ERR_NO_ARGUMENTS);
	}
	void* prompt_str = NULL;
	u8 index;
	if (p->stack.stack_i >= 2){
		// check for prompt
		if (ARG(1)->type == STACK_OP && ARG(1)->value.number == OP_SEMICOLON){
			// check prompt string
			if (ARG(0)->type & VAR_STRING){
				prompt_str = ARG(0)->value.ptr;
				index = 2; // variables start here
			} else {
				ERROR(ERR_TYPE_MISMATCH);
			}
		} else {
			// no prompt string
			index = 0;
		}
	}
	u8 len = p->stack.stack_i - index;
	// len: number of vars
	// TODO:ERR validate all are variables?
	
	// Display prompt
	if (prompt_str) // only display string if it exists
		con_puts(con, prompt_str);
	con_put(con, to_wide('?'));
	if (con->x) con_newline(con); // only newline if necessary for full line of input
	
	// Prompt user for input
	u16 inkey;
	// length: 32 TODO:CODE constant?
	u16* output;
	u8 out_index;
	bool valid = false;
	while (!valid){
		valid = true;
		output = con->text[con->y];
		out_index = 0;
		while ((inkey = get_inkey(&p->input)) != '\r'){
			if ((inkey == '\b' || check_pressed_manual(&p->input, BUTTON_ID_Y, 15, 4))
				&& out_index > 0){
				output[--out_index] = 0;
			} else if (inkey && out_index <= 31){
				output[out_index++] = inkey;
			} else {
				// sleep for user input?
				struct timespec tspec = {.tv_nsec=1000000000/60};
				thrd_sleep(&tspec, NULL);
			}
		}
		// scan commas
		int commas = 0;
		for (int x = 0; x < 32; ++x){
			commas += con_text_getc(con, x, con->y) == to_wide(',');
		}
		if (len != commas + 1){
			valid = false;
			continue; //TODO:IMPL ?Redo from start
		}
		// TODO:ERR Validate types
		u8 conversion_copy[32];
		int prev_i = 0;
		int out_i = 0;
		//TODO:CODE This loop is stupid? Fix it?
		for (int i = 0; i < len; ){
			struct stack_entry* e = ARG(index+i);
			u8 c = to_char(output[out_i]);
			if (e->type == (VAR_NUMBER | VAR_VARIABLE)){
				// validate first entry is numeric
				conversion_copy[out_i++] = c;
				if (is_number(c) || c == '.' || c == '-'){
				} else if (c == ',' || c == '\0' || c == ' ' || out_i >= 32){
					// convert from previous to out_i
					s32 n = str_to_num(&conversion_copy[prev_i], out_i - prev_i - 1);
					prev_i = out_i;
					*(s32*)e->value.ptr = n; //store result to variable
					++i;
					
				} else {
					// invalid character
					valid = false;
					break; //TODO:IMPL ?Redo from start
				}
			} else if (e->type == (VAR_STRING | VAR_VARIABLE)){
				conversion_copy[out_i++] = c;
				if (c == ',' || c == '\0' || c == ' ' || out_i >= 32){
					struct string* s = get_new_str(&p->strs);
					
					if (*(void**)e->value.ptr != NULL && **(char**)e->value.ptr == STRING_CHAR){
						(*(struct string**)e->value.ptr)->uses--;
					}
					(*(struct string**)e->value.ptr) = s;
					
					// TODO:IMPL Wide string support?
					s->uses = 1;
					s->len = out_i - prev_i - 1;
					for (int j = prev_i; j < out_i; ++j){
						s->ptr.s[j - prev_i] = conversion_copy[prev_i + j];
					}
					prev_i = out_i;
					++i;
				}
			} else {
				// invalid argument type
				p->exec.error = ERR_TYPE_MISMATCH; //TODO:CODE better error message?
				p->stack.stack_i = 0;
				return;
			}
		}
		con_newline(con); // from user entering the line.
		// Note that this has to be after reading the characters, or the positions get messed up
		if (!valid){
			con_puts(con, "S\20?Redo from start");
			con_newline(con);
			if (prompt_str) // only display string if it exists
				con_puts(con, prompt_str);
			con_put(con, to_wide('?'));
			if (con->x) con_newline(con); // only newline if necessary for full line of input
		}
	}
	p->stack.stack_i = 0;
}

void cmd_linput(struct ptc* p){
	++p;
}

inline u16 con_text_getc(struct console* c, u32 x, u32 y){
	return c->text[y][x];
}

inline void con_text_setc(struct console* c, u32 x, u32 y, u16 w){
	c->text[y][x] = w;
}

inline u8 con_col_get(struct console* c, u32 x, u32 y){
	return c->color[y][x];
}

inline void con_col_set(struct console* c, u32 x, u32 y, u8 col){
	c->color[y][x] = col;
}
