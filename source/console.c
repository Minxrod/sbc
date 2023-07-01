#include "console.h"

#include "system.h"

#include <stdio.h>

void cmd_print(struct ptc* p){
//	struct stack* s = &p->stack;
	struct console* c = &p->console;
	
	u32 i = 0;
	while (i < p->stack.stack_i){
		struct stack_entry* e = &p->stack.entry[i];
		
		//TODO: Print to the actual console and not the logs/debug console
		if (e->type & VAR_NUMBER){
			s32 x = VALUE_NUM(e);
			iprintf("%.3f", x / 4096.0);
		} else if (e->type & VAR_STRING) {
			//TODO: Check string type before printing!
			iprintf("len=%d ", *((unsigned char*)e->value.ptr+1));
			iprintf("%.*s", *((unsigned char*)e->value.ptr+1), (char*)e->value.ptr+2);
		} else if (e->type & VAR_VARIABLE) {
			//TODO: Convert variable to value type!
			iprintf("name=%.*s", *((unsigned char*)e->value.ptr+1), (char*)e->value.ptr+2);
			
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
