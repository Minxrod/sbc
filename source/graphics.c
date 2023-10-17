#include "graphics.h"

#include "system.h"
#include "error.h"

#include <math.h>

/// Convert screen coordinates to GRP coordinates
u16 grp_index(uint_fast8_t x, uint_fast8_t y){
	int_fast8_t cx = x / 64;
	int_fast8_t cy = y / 64;
	int_fast8_t tx = (x / 8) % 8;
	int_fast8_t ty = (y / 8) % 8;
	int_fast8_t px = x % 8;
	int_fast8_t py = y % 8;
	return px + py * 8 + tx * 64 + ty * 512 + cx * 4096 + cy * 16384;
}

u8* grp_drawpage(struct ptc* p, u8 page){
	return p->res.grp[page];
}

void cmd_gpage(struct ptc* p){
	//TODO:ERR:MED argument range
	//TODO:IMPL:HIGH 3-argument form
	struct stack_entry* e = stack_pop(&p->stack);
	p->graphics.screen = FP_TO_INT(VALUE_NUM(e));
	p->stack.stack_i = 0;
}

void cmd_gcolor(struct ptc* p){
	//TODO:ERR:MED argument range
	struct stack_entry* e = stack_pop(&p->stack);
	p->graphics.color = FP_TO_INT(VALUE_NUM(e));
}

void cmd_gcls(struct ptc* p){
	//TODO:ERR:MED argument range
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

void cmd_gfill(struct ptc* p){
	//GFILL x1 y1 x2 y2 [c]
	//TODO:CODE:NONE Determine what type is appropriate here?
	//TODO:ERR:LOW Check argument clamping accuracy
	int x1 = STACK_INT(0);
	int y1 = STACK_INT(1);
	int x2 = STACK_INT(2);
	int y2 = STACK_INT(3);
	u8 color;
	
	x1 = x1 < 0 ? 0 : x1 > GRP_WIDTH ? GRP_WIDTH : x1;
	y1 = y1 < 0 ? 0 : y1 > GRP_HEIGHT ? GRP_HEIGHT : y1;
	x2 = x2 < 0 ? 0 : x2 > GRP_WIDTH ? GRP_WIDTH : x2;
	y2 = y2 < 0 ? 0 : y2 > GRP_HEIGHT ? GRP_HEIGHT : y2;
	
	if (p->stack.stack_i == 5){
		color = STACK_INT(4); //TODO:ERR:MED Argument ranges
	} else if (p->stack.stack_i == 4){
		color = p->graphics.color;
	} else {
		ERROR(ERR_WRONG_ARG_COUNT);
	}
	
	u8* page = grp_drawpage(p, p->graphics.drawpage);
	
	for (int_fast16_t x = x1; x <= x2; ++x){
		for (int_fast16_t y = y1; y <= y2; ++y){
			page[grp_index(x, y)] = color;
		}
	}
	p->stack.stack_i = 0;
}

void cmd_gbox(struct ptc* p){
	int x1 = STACK_INT(0);
	int y1 = STACK_INT(1);
	int x2 = STACK_INT(2);
	int y2 = STACK_INT(3);
	u8 color;
	
	x1 = x1 < 0 ? 0 : x1 > GRP_WIDTH ? GRP_WIDTH : x1;
	y1 = y1 < 0 ? 0 : y1 > GRP_HEIGHT ? GRP_HEIGHT : y1;
	x2 = x2 < 0 ? 0 : x2 > GRP_WIDTH ? GRP_WIDTH : x2;
	y2 = y2 < 0 ? 0 : y2 > GRP_HEIGHT ? GRP_HEIGHT : y2;
	
	if (p->stack.stack_i == 5){
		color = STACK_INT(4); //TODO:ERR:MED Argument ranges
	} else if (p->stack.stack_i == 4){
		color = p->graphics.color;
	} else {
		ERROR(ERR_WRONG_ARG_COUNT);
	}
	
	u8* page = grp_drawpage(p, p->graphics.drawpage);
	
	int x = x1;
	int y = y1;
	for (int_fast16_t x = x1; x <= x2; ++x){
		page[grp_index(x, y)] = color;
	}
	for (int_fast16_t y = y1; y <= y2; ++y){
		page[grp_index(x, y)] = color;
	}
	x = x2;
	y = y2;
	for (int_fast16_t x = x1; x <= x2; ++x){
		page[grp_index(x, y)] = color;
	}
	for (int_fast16_t y = y1; y <= y2; ++y){
		page[grp_index(x, y)] = color;
	}
	p->stack.stack_i = 0;
}

// Notes about GLINE:
// oooo        <--this           ooooo
//     oooo                          ooooo
//         oooo   not this-->            oooo
void cmd_gline(struct ptc* p){
	int x1 = STACK_INT(0);
	int y1 = STACK_INT(1);
	int x2 = STACK_INT(2);
	int y2 = STACK_INT(3);
	u8 color;
	
	if (p->stack.stack_i == 5){
		color = STACK_INT(4); //TODO:ERR:MED Argument ranges
	} else if (p->stack.stack_i == 4){
		color = p->graphics.color;
	} else {
		ERROR(ERR_WRONG_ARG_COUNT);
	}
	
	u8* page = grp_drawpage(p, p->graphics.drawpage);
	
	// Draw a line :)
	if (abs(x2 - x1) >= abs(y2 - y1)){
		// Line has horizontal component
		// x1 -> x2
		fixp y_slope = ((((int64_t)(y2 - y1) << 32) / (x2 - x1)) >> (32 - FIXPOINT)) & 0xFFFFFFFF;
		// 8.12 FP I guess
		if (x1 > x2){
			int temp = y1;
			y1 = y2;
			y2 = temp;
			temp = x1;
			x1 = x2;
			x2 = temp;
		}
		for (int x = x1; x <= x2; ++x){
			int y = FP_TO_INT(y_slope*(x-x1)) + y1;
			
			page[grp_index(x,y)] = color;
		}
	} else {
		// X-range <= Y-range
		fixp x_slope = ((((int64_t)(x2 - x1) << 32) / (y2 - y1)) >> (32 - FIXPOINT)) & 0xFFFFFFFF;
		// 8.12 FP I guess
		if (y1 > y2){
			int temp = y1;
			y1 = y2;
			y2 = temp;
			temp = x1;
			x1 = x2;
			x2 = temp;
		}
		for (int y = y1; y <= y2; ++y){
			int x = FP_TO_INT(x_slope*(y-y1)) + x1;
			
			page[grp_index(x,y)] = color;
		}
	}
	
	p->stack.stack_i = 0;
}

void cmd_gpset(struct ptc* p){
	int x = STACK_INT(0);
	int y = STACK_INT(1);
	u8 color;
	
	if (p->stack.stack_i == 3){
		color = STACK_INT(2); //TODO:ERR:MED Argument ranges
	} else if (p->stack.stack_i == 2){
		color = p->graphics.color;
	} else {
		ERROR(ERR_WRONG_ARG_COUNT);
	}
	
	u8* page = grp_drawpage(p, p->graphics.drawpage);
	
	//TODO:ERR:MED Argument ranges
	page[grp_index(x,y)] = color;
	
	p->stack.stack_i = 0;
}
