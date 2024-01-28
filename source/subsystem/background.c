#include "background.h"

#include "system.h"
#include "error.h"

#include <assert.h>
#include <string.h>

struct background* init_background(void){
	struct background* b = calloc_log("init_background", sizeof(struct background), 1);
	// everything defaults to zero for now
	return b;
}

void free_background(struct background* b){
	free_log("free_background", b);
}

void step_background(struct background* b){
	// TODO:CODE:NONE pages constant
	for (int p = 0; p < 2; ++p){
		for (int l = 0; l < 2; ++l){
			struct bg_offset* bg = &b->ofs[p][l];
			if (bg->time > 0){
				bg->x += bg->step_x;
				bg->y += bg->step_y;
				bg->time--;
			}
		}
	}
}

u16* bg_page(struct ptc* p, u8 page, u8 layer){
	return p->res.scr[SCR_BANKS*page+2+layer];
}

#define BG_CHUNK_WIDTH 32
#define BG_CHUNK_HEIGHT 32
#define BG_CHUNK_SIZE (BG_CHUNK_WIDTH * BG_CHUNK_HEIGHT)

u16 bg_index(uint_fast8_t x, uint_fast8_t y){
	int_fast8_t cx = x / BG_CHUNK_WIDTH;
	int_fast8_t cy = y / BG_CHUNK_HEIGHT;
	int_fast8_t tx = x % BG_CHUNK_WIDTH;
	int_fast8_t ty = y % BG_CHUNK_HEIGHT;
	return tx + ty * BG_CHUNK_WIDTH + cx * BG_CHUNK_SIZE + cy * BG_CHUNK_SIZE * (BG_WIDTH / BG_CHUNK_WIDTH);
}

u16 bg_tile(struct ptc* p, uint_fast8_t page, uint_fast8_t l, uint_fast8_t x, uint_fast8_t y){
	return bg_page(p,page,l)[bg_index(x,y)];
}

void cmd_bgpage(struct ptc* p){
	STACK_INT_RANGE(0,0,1,p->background.page);
}

void cmd_bgclr(struct ptc* p){
	if (p->stack.stack_i == 0){
		// clear both layers on current page
		for (int l = 0; l <= 1; ++l){
			u16* bg = bg_page(p, p->background.page, l);
			memset(bg, 0, SCR_SIZE);
		}
	} else {
		// clear specific layer
		int layer;
		STACK_INT_RANGE(0,0,1,layer);
		u16* bg = bg_page(p, p->background.page, layer);
		memset(bg, 0, SCR_SIZE);
	}
}

// BGPUT l,x,y,td
// BGPUT l,x,y,td$
// BGPUT l,x,y,c,p,h,v
void cmd_bgput(struct ptc* p){
	// TODO:IMPL:HIGH other argument forms
	uint_fast8_t layer, x, y;
	u16 tiledata;
	STACK_INT_RANGE(0,0,1,layer);
	x = STACK_INT(1) % BG_WIDTH; //TODO:TEST:MED check that this works for negatives
	y = STACK_INT(2) % BG_HEIGHT;
	if (p->stack.stack_i == 4){
		if (ARG(3)->type & VAR_NUMBER){
			tiledata = STACK_INT(3) & 0xffff;
		} else {
			ERROR(ERR_UNIMPLEMENTED);
		}
	} else {
		int chr, pal, h, v;
		STACK_INT_RANGE(3,0,1023,chr);
		STACK_INT_RANGE(4,0,15,pal);
		STACK_INT_RANGE(5,0,1,h);
		STACK_INT_RANGE(6,0,1,v);
		
		tiledata = to_tiledata(chr, pal, h, v);
	}
	
	// valid args: put tile
	u16* bg = bg_page(p, p->background.page, layer);
	
	bg[bg_index(x,y)] = tiledata;
}

//BGFILL layer, x1, y1, x2, y2, chr, pal, h, v
//BGFILL layer, x1, y1, x2, y2, tile
//BGFILL layer, x1, y1, x2, y2, tile$
void cmd_bgfill(struct ptc* p){
	// TODO:IMPL:HIGH other forms
	uint_fast8_t layer;
	int x1, x2, y1, y2, temp;
	u16 tiledata;
	STACK_INT_RANGE(0,0,1,layer);
	x1 = STACK_INT(1);
	y1 = STACK_INT(2);
	x2 = STACK_INT(3);
	y2 = STACK_INT(4);
	if (p->stack.stack_i == 6){
		if (ARG(5)->type & VAR_NUMBER){
			tiledata = STACK_INT(5) & 0xffff;
		} else {
			ERROR(ERR_UNIMPLEMENTED);
		}
	} else {
		int chr, pal, h, v;
		STACK_INT_RANGE(5,0,1023,chr);
		STACK_INT_RANGE(6,0,15,pal);
		STACK_INT_RANGE(7,0,1,h);
		STACK_INT_RANGE(8,0,1,v);
		
		tiledata = to_tiledata(chr, pal, h, v);	
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
	int layer, time;
	fixp x, y;
	layer = STACK_INT(0);
	if (layer < 0 || layer > 1){
		ERROR(ERR_OUT_OF_RANGE);
	}
	x = STACK_NUM(1);
	y = STACK_NUM(2);
	
	struct bg_offset* b = &p->background.ofs[p->background.page][layer];
	if (p->stack.stack_i == 3){
		b->x = x;
		b->y = y;
		b->time = 0;
	} else {
		STACK_INT_MIN(3,0,time);
		if (time){
			b->step_x = (b->x - x) / time;
			b->step_y = (b->y - y) / time;
		} else {
			b->x = x;
			b->y = y;
		}
		b->time = time;
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
	// BGCOPY layer x1 y1 x2 y2 x3 y3
	uint_fast8_t layer;
	int x1, x2, y1, y2, x3, y3;
	int temp;
	STACK_INT_RANGE(0,0,1,layer);
	x1 = STACK_INT(1);
	y1 = STACK_INT(2);
	x2 = STACK_INT(3);
	y2 = STACK_INT(4);
	x3 = STACK_INT(5);
	y3 = STACK_INT(6);
	
	// bounds checking!
	// TODO:TEST:LOW bounds check behavior
	if (x1 < 0) { x1 = 0; }
	if (x2 < 0) { x2 = 0; }
	if (x3 < 0) { x3 = 0; }
	if (x1 > BG_WIDTH-1) { x1 = BG_WIDTH-1; }
	if (x2 > BG_WIDTH-1) { x2 = BG_WIDTH-1; }
	if (x3 > BG_WIDTH-1) { x3 = BG_WIDTH-1; }
	
	if (y1 < 0) { y1 = 0; }
	if (y2 < 0) { y2 = 0; }
	if (y3 < 0) { y3 = 0; }
	if (y1 > BG_HEIGHT-1) { y1 = BG_HEIGHT-1; }
	if (y2 > BG_HEIGHT-1) { y2 = BG_HEIGHT-1; }
	if (y3 > BG_HEIGHT-1) { y3 = BG_HEIGHT-1; }
	
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
	
	if (y3 < y1 || x3 < x1){
		for (int y = y1; y <= y2; ++y){
			for (int x = x1; x <= x2; ++x){
				bg[bg_index(x-x1+x3,y-y1+y3)] = bg[bg_index(x,y)];
			}
		}
	} else {
		for (int y = y2; y >= y1; --y){
			for (int x = x2; x >= x1; --x){
				bg[bg_index(x-x1+x3,y-y1+y3)] = bg[bg_index(x,y)];
			}
		}
	}
}

void func_bgchk(struct ptc* p){
	int layer;
	STACK_REL_INT_RANGE(-1,0,1,layer);
	p->stack.stack_i -= 1;
	
	STACK_RETURN_INT(p->background.ofs[p->background.page][layer].time > 0);
}
