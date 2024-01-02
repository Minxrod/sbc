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

void con_scroll(struct console* c){
	// TODO:PERF:LOW optimize (scroll all at once?)
	while (c->y >= CONSOLE_HEIGHT){
		c->y--;
		// newline: scroll console up
		// TODO:PERF:LOW optimize this (memcpy?)
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

void con_newline(struct console* c, bool scroll){
	c->x = 0;
	c->y++;
	if (scroll) con_scroll(c);
}

struct console* init_console(void){
	iprintf("console calloc: %d\n", (int)sizeof(struct console));
	struct console* c = calloc(sizeof(struct console), 1);
	c->tabstep = 4;
	return c;
}

void free_console(struct console* c){
	free(c);
}

void con_put(struct console* c, u16 w){
	if (c->y == 24) { con_scroll(c); }
	c->text[c->y][c->x] = w;
	c->color[c->y][c->x] = c->col;
	
	c->x++;
	if (c->x >= CONSOLE_WIDTH){
		con_newline(c, false);
	}
}

//TODO:PERF:LOW optimize via copying multiple lines at once for large strings?
//TODO:PERF:LOW con_puts write directly to console via str_wide_copy?
//(color still separate here)

void con_puts(struct console* c, const void* s){
	u32 len = str_len(s);
	for (size_t i = 0; i < len; ++i){
		con_put(c, str_at_wide(s, i));
	}
}

void con_putn(struct console* c, fixp n){
	u8 buf[16]; ///S#-524287.999\0 max length is 2+12 chars
	buf[0] = STRING_INLINE_CHAR;
	fixp_to_str(n, buf);
	
	con_puts(c, buf);
}

void con_putn_at(struct console* c, int x, int y, fixp n){
	c->x = x;
	c->y = y;
	con_putn(c, n);
}

void cmd_print(struct ptc* p){
//	struct stack* s = &p->stack;
	struct console* c = &p->console;
//	u8 buf[16]; 
//	buf[0]=STRING_INLINE_CHAR;
	
	u32 i = 0;
	while (i < p->stack.stack_i){
		struct stack_entry* e = &p->stack.entry[i];
		
		if (e->type & VAR_NUMBER){
			con_putn(c, VALUE_NUM(e));
		} else if (e->type & VAR_STRING) {
			con_puts(c, VALUE_STR(e));
		} else if (e->type & STACK_OP) { 
			if (e->value.number == OP_COMMA){
				// tab
				do {
					con_put(c, to_wide(' '));
				} while (c->x % c->tabstep != 0);
				if (c->y == CONSOLE_HEIGHT){
					con_newline(c, false);
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
		con_newline(c, true);
	}
	
	p->stack.stack_i = 0; //PRINT consumes all stack items
}

void cmd_color(struct ptc* p){
	struct console* c = &p->console;
	if (p->stack.stack_i == 2){
		c->col = 0;
		c->col |= STACK_INT(0); // FG
		c->col |= STACK_INT(1) << 4; // BG
	} else if (p->stack.stack_i == 1){
		c->col &= ~COL_FG_MASK; // clear FG value
		c->col |= STACK_INT(0); // FG
	}
	p->stack.stack_i = 0;
}

void cmd_locate(struct ptc* p){
	struct console* c = &p->console;
	if (p->stack.stack_i == 2){
		//LOCATE is a silent failure on out of range
		s32 x = FP_TO_INT(VALUE_NUM(stack_get(&p->stack, 0)));
		s32 y = FP_TO_INT(VALUE_NUM(stack_get(&p->stack, 1)));
		if (x >= 0 && y >= 0 && x < CONSOLE_WIDTH && y < CONSOLE_HEIGHT){
			c->x = x;
			c->y = y;
		}
	}
	p->stack.stack_i = 0;
}

void cmd_cls(struct ptc* p){
	struct console* c = &p->console;
	for (int i = 0; i < CONSOLE_HEIGHT; ++i){
		for (int j = 0; j < CONSOLE_WIDTH; ++j){
			c->text[i][j] = 0;
			c->color[i][j] = 0;
		}
	}
	c->x = 0;
	c->y = 0;
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

u16* shared_input(struct ptc* p){
	// TODO:IMPL:LOW Use BREPEAT values if set
	// TODO:IMPL:LOW Set color when entering text
	// TODO:IMPL:LOW Remove color when backspacing
	// TODO:IMPL:MED Blinking text cursor
	// TODO:IMPL:MED Delete, insert functionality
	// TODO:IMPL:MED Left/right movement of cursor
	// TODO:TEST:MED Write tests for these
	struct console* con = &p->console;
	uint_fast16_t inkey;
	uint_fast8_t keyboard;
	u16* output = con->text[con->y]; // start of current line
	uint_fast8_t out_index = 0;
	do {
		inkey = get_inkey(&p->input);
		keyboard = get_pressed_key(p);
		// wait for a frame to pass
		int t = get_time(&p->time);
		// This test_mode check isn't ideal, but it allows test
		// cases to run without waiting for an update from another thread
		while (!inkey && !con->test_mode && t == get_time(&p->time)){
			// sleep for user input
			struct timespec tspec = {.tv_nsec=1000000000/600};
			thrd_sleep(&tspec, NULL);
		}
		// Check for special keys
		if ((keyboard == 15 || check_pressed_manual(&p->input, BUTTON_ID_Y, 15 ,4))){
			if (out_index > 0){
				output[--out_index] = 0;
			}
		} else if (inkey == '\r'){
			// don't add this one
		} else if (inkey && out_index < CONSOLE_WIDTH){
			output[out_index++] = inkey;
		}
	} while (p->exec.error == ERR_NONE && inkey != '\r'
			&& keyboard != 60
			&& !check_pressed_manual(&p->input, BUTTON_ID_A, 15, 4));
	return output;
}

void con_prompt(struct console* con, void* prompt_str){
	// Displays prompt string
	if (prompt_str) // only display string if it exists
		con_puts(con, prompt_str);
	con_put(con, to_wide('?'));
	if (con->x) con_newline(con, true); // only newline if necessary for user to have full line of input
}

void con_reprompt(struct console* con, void* prompt_str){
	con_newline(con, true); // from user entering the line.
	con_puts(con, "S\20?Redo from start");
	con_newline(con, true);
	con_prompt(con, prompt_str);
}

void cmd_input(struct ptc* p){
	struct console* con = &p->console;
	// INPUT [prompt;]var[,var...]
	// Argument validation here
	if (!p->stack.stack_i){
		ERROR(ERR_NO_ARGUMENTS);
	}
	void* prompt_str = NULL;
	u8 index = 0;
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
	
	// Initial prompt
	con_prompt(con, prompt_str);
	
	// Prompt user for input
	u16* output;
//	u8 out_index;
	bool valid = false;
	while (!valid){
		valid = true;
		output = shared_input(p);
		// scan commas
		int commas = 0;
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			commas += con_text_getc(con, x, con->y) == to_wide(',');
		}
		if (len != commas + 1){
			valid = false;
			con_reprompt(con, prompt_str);
			continue; //TODO:IMPL:MED ?Redo from start
		}
		u8 conversion_copy[CONSOLE_WIDTH];
		int prev_i = 0;
		int out_i = 0;
		
		for (int i = 0; i < len; ){
			struct stack_entry* e = ARG(index+i);
			u8 c = to_char(output[out_i]);
			conversion_copy[out_i++] = c;
			if (e->type == (VAR_NUMBER | VAR_VARIABLE)){
				// validate first entry is numeric
				if (is_number(c) || c == '.' || c == '-'){
				} else if (c == ',' || c == '\0' || c == ' ' || out_i >= CONSOLE_WIDTH){
					// convert from previous to out_i
					if (out_i - prev_i - 1 == 0){
						valid = false;
						break;
					}
					fixp n = u8_to_num(&conversion_copy[prev_i], out_i - prev_i - 1);
					prev_i = out_i;
					*(fixp*)e->value.ptr = n; //store result to variable
					++i;
					
				} else {
					// invalid character
					valid = false;
					break;
				}
			} else if (e->type == (VAR_STRING | VAR_VARIABLE)){
				if (c == ',' || c == '\0' || out_i >= CONSOLE_WIDTH){
					struct string* s = get_new_str(&p->strs);
					
					if (*(void**)e->value.ptr != NULL && **(char**)e->value.ptr == STRING_CHAR){
						(*(struct string**)e->value.ptr)->uses--;
					}
					(*(struct string**)e->value.ptr) = s;
					
					// TODO:TEST:LOW Check that wide strings work here
					bool require_wide = false;
					for (int j = prev_i; j < out_i; ++j){
						if (!is_char(output[j])) require_wide = true;
					}
					
					s->uses = 1;
					s->len = out_i - prev_i - 1;
					for (int j = prev_i; j < out_i; ++j){
						if (require_wide){
							s->ptr.w[j - prev_i] = output[j];
						} else {
							s->ptr.s[j - prev_i] = conversion_copy[j];
						}
					}
					prev_i = out_i;
					++i;
				}
			} else {
				// invalid argument type
				p->exec.error = ERR_UNKNOWN_TYPE;
				p->stack.stack_i = 0;
				return;
			}
		}
		// Note that this has to be after reading the characters, or the positions get messed up
		if (!valid){
			con_reprompt(con, prompt_str);
		}
	}
	con_newline(con, true); // from user entering the line successfully.
	p->stack.stack_i = 0;
}

void cmd_linput(struct ptc* p){
	struct console* con = &p->console;
	// INPUT [prompt;]var$
	void* prompt_str = NULL;
	struct stack_entry* e;
	if (p->stack.stack_i == 3){
		// check prompt string
		prompt_str = ARG(0)->value.ptr;
		e = ARG(2);
	} else {
		e = ARG(0);
	}
	
	con_prompt(con, prompt_str);
	
	u16* output = shared_input(p);
	
	// Now store the result to a new string
	struct string* s = get_new_str(&p->strs);
	
	if (*(void**)e->value.ptr != NULL && **(char**)e->value.ptr == STRING_CHAR){
		(*(struct string**)e->value.ptr)->uses--;
	}
	(*(struct string**)e->value.ptr) = s;
	
	// TODO:IMPL:LOW Wide string support?
	s->uses = 1;
	s->len = 0;
	for (int j = 0; j < CONSOLE_WIDTH; ++j){
		u8 c = to_char(output[j]);
		if (!c) break;
		s->ptr.s[j] = c;
		s->len++;
	}
	con_newline(con, true); // from user entering the line successfully.
}

void sys_csrx(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, .value.number = INT_TO_FP(p->console.x)});
}

void sys_csry(struct ptc* p){
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){VAR_NUMBER, .value.number = INT_TO_FP(p->console.y)});
}

void sys_tabstep(struct ptc* p){
	// TODO:IMPL:HIGH figure out how to validate assignments here?
	ERROR(ERR_UNIMPLEMENTED);
}
