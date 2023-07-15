#include "console.h"

#include "system.h"

#include <stdio.h>
#include <stdlib.h>

void con_put(struct console* c, u16 w){
	c->text[c->x][c->y] = w;
	
	c->x++;
	if (c->x >= CONSOLE_WIDTH){
		c->y++;
		c->x = 0;
		if (c->y >= CONSOLE_HEIGHT){
			c->y--;
			// newline: scroll console up
			// TODO: memset here probably faster?
			for (u32 i = 1; i < CONSOLE_HEIGHT; ++i){
				for (u32 j = 0; j < CONSOLE_WIDTH; ++i){
					c->text[i-1][j] = c->text[i][j];
					c->color[i-1][j] = c->color[i][j];
				}
			}
		}
	}
}

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
	
	u32 i = 0;
	while (i < p->stack.stack_i){
		struct stack_entry* e = &p->stack.entry[i];
		
		//TODO: Print to the actual console and not the logs/debug console
		if (e->type & VAR_NUMBER){
			s32 x = VALUE_NUM(e);
			// TODO: convert to string!
			iprintf("%.3f", x / 4096.0);
		} else if (e->type & VAR_STRING) {
			//TODO: Check string type before printing!
			con_puts(c, VALUE_STR(e));
		} else {
			// what
			iprintf("What did you put on the stack? (type:%d)\n", e->type);
			abort();
		}
		iprintf("\n");
		i++;
	}
	c->text[c->x][c->y] = 0;
	p->stack.stack_i = 0; //PRINT consumes all stack items
}

void cmd_color(struct ptc* p){
	++p;
}

void cmd_locate(struct ptc* p){
	++p;
}
