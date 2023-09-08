#include "graphics.h"

#include "system.h"
#include "error.h"

/// Convert screen coordinates to GRP coordinates
u16 grp_index(int_fast8_t x, int_fast8_t y){
	int_fast8_t cx = x / 64;
	int_fast8_t cy = y / 64;
	int_fast8_t tx = (x / 8) % 8;
	int_fast8_t ty = (y / 8) % 8;
	int_fast8_t px = x % 8;
	int_fast8_t py = y % 8;
	return px + py * 8 + tx * 64 + ty * 512 + cx * 2048 + cy * 16384;
}

u8* grp_drawpage(struct ptc* p, u8 page){
	return p->res.grp[page];
}

void cmd_gpage(struct ptc* p){
	//TODO:ERR argument count
	//TODO:ERR argument range
	//TODO:IMPL 3-argument form
	struct stack_entry* e = stack_pop(&p->stack);
	p->graphics.screen = FP_TO_INT(VALUE_NUM(e));
	p->stack.stack_i = 0;
}

void cmd_gcolor(struct ptc* p){
	ARGCHECK(1);
	
	//TODO:ERR argument range
	struct stack_entry* e = stack_pop(&p->stack);
	p->graphics.color = FP_TO_INT(VALUE_NUM(e));
}

void cmd_gcls(struct ptc* p){
	//TODO:IMPL
	//TODO:ERR argument range, count
	u8 color;
	if (p->stack.stack_i == 1){
		struct stack_entry* e = stack_pop(&p->stack);
		color = FP_TO_INT(VALUE_NUM(e));
	} else if (p->stack.stack_i == 0){
		color = p->graphics.color;
	} else {
		ERROR(ERR_WRONG_ARG_COUNT);
	}
	
	u8* page = grp_drawpage(p, p->graphics.drawpage);
	
	for (u16 i = 0; i < GRP_SIZE; ++i){
		page[i] = color;
	}
}
