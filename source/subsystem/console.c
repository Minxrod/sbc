#include "console.h"

#include "system.h"
#include "ptc.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <threads.h>

/// @warning Does not handle newlines!
void con_advance(struct console* c){
	c->x++;
	if (c->x >= CONSOLE_WIDTH){
		c->y++;
		c->x = 0;
	}
}

void con_scroll(struct console* c){
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
	struct console* c = calloc_log("init_console", sizeof(struct console), 1);
	c->sys_tabstep = INT_TO_FP(4);
	c->tabstep = 4;
	return c;
}

void free_console(struct console* c){
	free_log("free_console", c);
}

void con_put(struct console* c, u16 w){
	assert(c->x < CONSOLE_WIDTH);
	assert(c->y < CONSOLE_HEIGHT);
	c->text[c->y][c->x] = w;
	c->color[c->y][c->x] = c->col;
	
	c->x++;
	if (c->x >= CONSOLE_WIDTH){
		con_newline(c, false);
	}
}

void con_puts(struct console* c, const void* s){
	if (c->y >= CONSOLE_HEIGHT){
		con_scroll(c);
	}
	u32 len = str_len(s);
	for (size_t i = 0; i < len; ++i){
		con_put(c, str_at_wide(s, i));
		if (i < len-1 && c->y == CONSOLE_HEIGHT){
			con_scroll(c);
		}
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
	struct console* c = &p->console;
	
	u32 i = 0;
	while (i < p->stack.stack_i){
		struct stack_entry* e = &p->stack.entry[i];
		
		if (e->type & VAR_NUMBER){
			con_putn(c, VALUE_NUM(e));
		} else if (e->type & VAR_STRING) {
			con_puts(c, value_str(e));
		} else if (e->type & STACK_OP) { 
			if (e->value.number == OP_COMMA){
				// tab
				if (c->x == 0 && c->y == CONSOLE_HEIGHT){
					con_scroll(c);
				}
				do {
					con_put(c, to_wide(' '));
				} while (c->x % c->tabstep != 0);
			} else if (e->value.number == OP_SEMICOLON){
				// do nothing lol
			} else {
				ERROR(ERR_PRINT_INVALID_OP); // should never happen
			}
		} else {
			ERROR(ERR_PRINT_INVALID_STACK); // should probably never happen
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
			c->color[i][j] = c->col;
			p->panel.text->text[i][j] = 0; // CLS also clears lower screen
			p->panel.text->color[i][j] = 0; // CLS also clears lower screen
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

u16* shared_input(struct ptc* p){
	// TODO:IMPL:LOW Use BREPEAT values if set
	// TODO:IMPL:LOW Set color when entering text
	// TODO:IMPL:LOW Remove color when backspacing
	// TODO:IMPL:MED Blinking text cursor
	// TODO:TEST:MED Write tests for these
	struct console* con = &p->console;
	
	uint_fast16_t inkey;
	uint_fast8_t keyboard;
	u16* output = con->text[con->y]; // start of current line
//	u8* outcol = con->color[con->y];
	uint_fast8_t out_index = 0;
	uint_fast8_t out_index_max = 0;
	int old_panel = p->panel.type;
	p->console.cursor_visible = true;
	// enable typing during INPUT/LINPUT if not already active
	p->panel.type = PNL_KYA;
	set_panel_bg(p, PNL_KYA);
	do {
		// wait for a frame to pass
		int t = get_time(&p->time);
		// This test_mode check isn't ideal, but it allows test
		// cases to run without waiting for an update from another thread
		inkey = get_inkey(&p->input); // these are buffered so can be placed before
		keyboard = get_pressed_key(p);
		while (!inkey && !con->test_mode && t == get_time(&p->time)){
			// sleep for user input
			struct timespec tspec = {.tv_nsec=1000000000/600};
			thrd_sleep(&tspec, NULL);
		}
		// Check for special keys
		// redraw on first frame of hold and first frame of release
		// TODO:IMPL:LOW Pressing L+R inverts the selection again for the duration held.
		// it should keep the shift state.
		if (((p->input.button ^ p->input.old_button) & BUTTON_L) ||
		    ((p->input.button ^ p->input.old_button) & BUTTON_R)){
			p->panel.shift ^= PNL_SHIFT;
			refresh_panel(p);
		} else if ((keyboard == 15 || check_pressed(&p->input, BUTTON_ID_Y))){
			// Copy characters back
			if (out_index > 0){
				for (int i = out_index-1; i < (int)out_index_max; ++i){
					output[i] = output[i+1];
				}
				output[--out_index_max] = 0;
				--out_index;
			}
		} else if (keyboard == 67){
			// Copy characters back
			if (out_index < out_index_max){
				for (int i = out_index; i < (int)out_index_max; ++i){
					output[i] = output[i+1];
				}
				output[--out_index_max] = 0;
			}
		} else if (inkey == '\r'){
			// don't add this one
		} else if (inkey && out_index_max < CONSOLE_WIDTH){
			if ((p->panel.cursor & ~PNL_INSERT)){ // REPLACE mode
				output[out_index++] = inkey;
				if (out_index > out_index_max)
					++out_index_max;
			} else { // INSERT mode
				// Copy characters past insertion point forward
				for (int i = out_index_max-1; i >= (int)out_index; --i){
					output[i+1] = output[i];
				}
				output[out_index++] = inkey;
				++out_index_max;
			}
		} else if (check_pressed(&p->input, BUTTON_ID_LEFT)){
			out_index -= out_index > 0;
		} else if (check_pressed(&p->input, BUTTON_ID_RIGHT)){
			out_index += out_index < out_index_max;
		}
		p->console.x = out_index;
	} while (p->exec.error == ERR_NONE && inkey != '\r'
			&& !check_pressed_manual(&p->input, BUTTON_ID_A, 15, 4));
	p->panel.type = old_panel;
	p->console.cursor_visible = false;
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
			continue;
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

void func_chkchr(struct ptc* p){
	int x, y;
	int c = -1;
	x = STACK_REL_INT(-2);
	y = STACK_REL_INT(-1);
	p->stack.stack_i -= 2;
	
	if (x >= 0 && x < CONSOLE_WIDTH && y >= 0 && y < CONSOLE_HEIGHT){
		c = to_char(con_text_getc(&p->console,x,y));
	}
	
	STACK_RETURN_INT(c);
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
	struct value_stack* s = &p->stack;
	
	stack_push(s, (struct stack_entry){VAR_NUMBER | VAR_VARIABLE, .value.ptr = &p->console.sys_tabstep});
}

void syschk_tabstep(struct ptc* p){
	int cur_tabstep = FP_TO_INT(p->console.sys_tabstep);
	if (cur_tabstep < 1) cur_tabstep = 1;
	if (cur_tabstep > 16) cur_tabstep = 16;
	p->console.tabstep = cur_tabstep;
}
