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
	// TODO optimize (scroll all at once?)
	while (c->y >= CONSOLE_HEIGHT){
		c->y--;
		// newline: scroll console up
		// TODO optimize this (memcpy?)
		for (u32 i = 1; i < CONSOLE_HEIGHT; ++i){
			for (u32 j = 0; j < CONSOLE_WIDTH; ++i){
				c->text[i-1][j] = c->text[i][j];
				c->color[i-1][j] = c->color[i][j];
			}
		}
	}
}

void con_init(struct console* c){
	c->tabstep = 4;
}

void con_put(struct console* c, u16 w){
	c->text[c->x][c->y] = w;
	c->color[c->x][c->y] = c->col;
	
	c->x++;
	if (c->x >= CONSOLE_WIDTH){
		con_newline(c);
	}
}

//TODO: optimize via copying multiple lines at once for large strings?
//TODO: memset instead of manual copy? probably nicer on cache...?
//TODO: con_puts write directly to console via str_wide_copy?
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
			buf[1] = strlen((char*)&buf[2]); //TODO: remove this bit
			
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
	if (p->stack.entry[p->stack.stack_i-1].type == STACK_OP){
		// no newline!
	} else {
		// newline!
		con_newline(c);
	}
	
	p->stack.stack_i = 0; //PRINT consumes all stack items
}

void cmd_color(struct ptc* p){
	struct console* c = &p->console;
	// TODO: check arguments types, quantity
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
	// TODO: check arguments types, quantity
	if (p->stack.stack_i == 2){
		// TODO: Use stack_get or whatever instead
		c->x = VALUE_NUM(stack_get(&p->stack, 0)) >> 12; // BG
		c->y = VALUE_NUM(stack_get(&p->stack, 1)) >> 12; // FG
	} else {
		p->exec.error = ERR_WRONG_ARG_COUNT;
	}
	p->stack.stack_i = 0;
}

void cmd_cls(struct ptc* p){
	struct console* c = &p->console;
	// TODO: check arguments types, quantity
	for (int i = 0; i < CONSOLE_HEIGHT; ++i){
		for (int j = 0; j < CONSOLE_WIDTH; ++j){
			c->text[j][i] = 0;
			c->color[j][i] = 0;
		}
	}
}


inline u16 con_text_getc(struct console* c, u32 x, u32 y){
	return c->text[x][y];
}

inline void con_text_setc(struct console* c, u32 x, u32 y, u16 w){
	c->text[x][y] = w;
}
