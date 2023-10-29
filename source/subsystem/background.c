#include "background.h"

#include "system.h"
#include "error.h"

struct background* init_background(){
	struct background* b = calloc(sizeof(struct background), 1);
	// everything defaults to zero for now
	return b;
}

void free_background(struct background* b){
	free(b);
}

u16* bg_page(struct ptc* p, u8 page, u8 layer){
	return p->res.scr[2*page+layer];
}

u16 bg_index(uint_fast8_t x, uint_fast8_t y){
	// TODO: Replace these with named constants?
	int_fast8_t cx = x / 32;
	int_fast8_t cy = y / 32;
	int_fast8_t tx = x % 32;
	int_fast8_t ty = y % 32;
	return tx + ty * 32 + cx * 1024 + cy * 2048;
}


void cmd_bgpage(struct ptc* p){
	// TODO:ERR:MED bounds checking
	p->background.page = STACK_INT(0);
}

void cmd_bgclr(struct ptc* p){
	// TODO:ERR:MED bounds checking
	if (p->stack.stack_i == 0){
		// clear both layers on current page
		for (int l = 0; l <= 1; ++l){
			u16* bg = bg_page(p, p->background.page, l);
			//TODO:PERF:NONE this can be faster
			for (uint_fast8_t y = 0; y < BG_HEIGHT; ++y){
				for (uint_fast8_t x = 0; x < BG_HEIGHT; ++x){
					bg[bg_index(x,y)] = 0;
				}
			}
		}
	} else {
		// clear specific layer
		u16* bg = bg_page(p, p->background.page, STACK_INT(0));
		//TODO:PERF:NONE this can be faster
		for (uint_fast8_t y = 0; y < BG_HEIGHT; ++y){
			for (uint_fast8_t x = 0; x < BG_HEIGHT; ++x){
				bg[bg_index(x,y)] = 0;
			}
		}
	}
}

// BGPUT l,x,y,td
// BGPUT l,x,y,td$
// BGPUT l,x,y,c,p,h,v
void cmd_bgput(struct ptc* p){
	// TODO:ERR:MED bounds checking
	// TODO:IMPL:HIGH other argument forms
	uint_fast8_t layer, x, y;
	u16 tiledata;
	layer = STACK_INT(0);
	x = STACK_INT(1);
	y = STACK_INT(2);
	if (p->stack.stack_i == 4){
		if (ARG(3)->type & VAR_NUMBER){
			tiledata = STACK_INT(3);
		} else {
			ERROR(ERR_UNIMPLEMENTED);
		}
	} else {
		ERROR(ERR_UNIMPLEMENTED);
	}
	
	// valid args: put tile
	u16* bg = bg_page(p, p->background.page, layer);
	
	bg[bg_index(x,y)] = tiledata;
}

void cmd_bgfill(struct ptc* p){
	ERROR(ERR_UNIMPLEMENTED);
}

void cmd_bgofs(struct ptc* p){
	ERROR(ERR_UNIMPLEMENTED);
}

void cmd_bgclip(struct ptc* p){
	ERROR(ERR_UNIMPLEMENTED);
}

void cmd_bgread(struct ptc* p){
	ERROR(ERR_UNIMPLEMENTED);
}

void cmd_bgcopy(struct ptc* p){
	ERROR(ERR_UNIMPLEMENTED);
}

