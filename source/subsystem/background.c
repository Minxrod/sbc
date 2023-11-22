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

u16 bg_tile(struct ptc* p, uint_fast8_t page, uint_fast8_t l, uint_fast8_t x, uint_fast8_t y){
	return bg_page(p,page,l)[bg_index(x,y)];
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
	x = STACK_INT(1) % BG_WIDTH; //TODO:TEST:MED check that this works for negatives
	y = STACK_INT(2) % BG_HEIGHT;
	if (p->stack.stack_i == 4){
		if (ARG(3)->type & VAR_NUMBER){
			tiledata = STACK_INT(3);
		} else {
			ERROR(ERR_UNIMPLEMENTED);
		}
	} else {
		//TODO:IMPL:MED this should be a function probably? implement error checks only once
		int chr = STACK_INT(3);
		int pal = STACK_INT(4);
		int h = STACK_INT(5);
		int v = STACK_INT(6);
		//TODO:ERR:MED bounds checking...
		//TODO:TEST:LOW tests for this form?
		tiledata = (chr & 0x3ff) | ((h & 0x1) << 10) | ((v & 0x1) << 11) | ((pal & 0xf) << 12);
	}
	
	// valid args: put tile
	u16* bg = bg_page(p, p->background.page, layer);
	
	bg[bg_index(x,y)] = tiledata;
}

//BGFILL layer, x1, y1, x2, y2, chr, pal, h, v
//BGFILL layer, x1, y1, x2, y2, tile
//BGFILL layer, x1, y1, x2, y2, tile$
void cmd_bgfill(struct ptc* p){
	// TODO:ERR:MED bounds checking layer
	// TODO:IMPL:HIGH other forms
	uint_fast8_t layer;
	int x1, x2, y1, y2, temp;
	u16 tiledata;
	layer = STACK_INT(0);
	x1 = STACK_INT(1);
	y1 = STACK_INT(2);
	x2 = STACK_INT(3);
	y2 = STACK_INT(4);
	if (p->stack.stack_i == 6){
		if (ARG(5)->type & VAR_NUMBER){
			tiledata = STACK_INT(5);
		} else {
			ERROR(ERR_UNIMPLEMENTED);
		}
	} else {
		ERROR(ERR_UNIMPLEMENTED);
	}
	// bounds checking!
	if (x1 < 0) { x1 = 0; }
	if (x2 < 0) { x2 = 0; }
	if (x1 > BG_WIDTH-1) { x1 = BG_WIDTH-1; }
	if (x2 > BG_WIDTH-1) { x2 = BG_WIDTH-1; }
	
	if (y1 < 0) { y1 = 0; }
	if (y2 < 0) { y2 = 0; }
	if (y1 > BG_HEIGHT-1) { y1 = BG_HEIGHT-1; }
	if (y2 > BG_HEIGHT-1) { y2 = BG_HEIGHT-1; }
	
	if (x1 > x2){
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y1 > y2){
		temp = y1;
		y1 = y2;
		y2 = temp;
	}
	
	// valid args: put tile
	u16* bg = bg_page(p, p->background.page, layer);
	
	for (int y = y1; y <= y2; ++y){
		for (int x = x1; x <= x2; ++x){
			bg[bg_index(x,y)] = tiledata;
		}
	}
}

void cmd_bgofs(struct ptc* p){
	// TODO:ERR:MED check bounds? (maybe only for layer?)
	// TODO:ERR:HIGH interpolation form
	uint_fast8_t layer;
	fixp x, y;
	layer = STACK_INT(0);
	x = STACK_NUM(1);
	y = STACK_NUM(2);
	
	if (p->stack.stack_i == 3){
		p->background.ofs[p->background.page][layer].x = x;
		p->background.ofs[p->background.page][layer].y = y;
		p->background.ofs[p->background.page][layer].time = 0;
	} else {
		ERROR(ERR_UNIMPLEMENTED);
	}
}

void cmd_bgclip(struct ptc* p){
	// TODO:ERR:MED check bounds
	uint_fast8_t x1 = STACK_INT(0);
	uint_fast8_t y1 = STACK_INT(1);
	uint_fast8_t x2 = STACK_INT(2);
	uint_fast8_t y2 = STACK_INT(3);
	
	p->background.clip[p->background.page].x1 = x1;
	p->background.clip[p->background.page].y1 = y1;
	p->background.clip[p->background.page].x2 = x2;
	p->background.clip[p->background.page].y2 = y2;
}

void cmd_bgread(struct ptc* p){
	ERROR(ERR_UNIMPLEMENTED);
}

void cmd_bgcopy(struct ptc* p){
	ERROR(ERR_UNIMPLEMENTED);
}

