#include "graphics.h"

#include "system.h"
#include "error.h"

#include <math.h>

void init_graphics(struct graphics* g){
	g->info[0].drawpage = 0;
	g->info[0].displaypage = 0;
	g->info[0].prio = 3;
	g->info[1].drawpage = 1;
	g->info[1].displaypage = 1;
	g->info[1].prio = 3;
	g->drawmode = 0;
}

u8* grp_drawpage(struct ptc* p){
	return p->res.grp[p->graphics.info[p->graphics.screen].drawpage];
}

void cmd_gpage(struct ptc* p){
	if (p->stack.stack_i == 1){
		STACK_INT_RANGE(0,0,1,p->graphics.screen);
	} else {
		STACK_INT_RANGE(0,0,1,p->graphics.screen);
		STACK_INT_RANGE(1,0,3,p->graphics.info[p->graphics.screen].drawpage);
		STACK_INT_RANGE(2,0,3,p->graphics.info[p->graphics.screen].displaypage);
	}
}

void cmd_gcolor(struct ptc* p){
	STACK_INT_RANGE(0,0,255,p->graphics.color);
}

void cmd_gcls(struct ptc* p){
	u8 color;
	if (p->stack.stack_i == 1){
		STACK_INT_RANGE(0,0,255,color);
	} else {
		color = p->graphics.color;
	}
	
	u8* page = grp_drawpage(p);
	
	for (u16 i = 0; i < GRP_SIZE; ++i){
		page[i] = color;
	}
}

void cmd_gfill(struct ptc* p){
	//GFILL x1 y1 x2 y2 [c]
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
		STACK_INT_RANGE(4,0,255,color);
	} else {
		color = p->graphics.color;
	}
	
	u8* page = grp_drawpage(p);
	
	for (int_fast16_t x = x1; x <= x2; ++x){
		for (int_fast16_t y = y1; y <= y2; ++y){
			grp_pixel(page,x,y,color,p->graphics.drawmode);
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
		int c = STACK_INT(4);
		if (c > 255 || c < 0){
			ERROR(ERR_OUT_OF_RANGE);
		}
		color = c;
	} else {
		color = p->graphics.color;
	}
	
	u8* page = grp_drawpage(p);
	
	int x = x1;
	int y = y1;
	for (int_fast16_t x = x1; x <= x2; ++x){
		grp_pixel(page,x,y,color,p->graphics.drawmode);
	}
	for (int_fast16_t y = y1; y <= y2; ++y){
		grp_pixel(page,x,y,color,p->graphics.drawmode);
	}
	x = x2;
	y = y2;
	for (int_fast16_t x = x1; x <= x2; ++x){
		grp_pixel(page,x,y,color,p->graphics.drawmode);
	}
	for (int_fast16_t y = y1; y <= y2; ++y){
		grp_pixel(page,x,y,color,p->graphics.drawmode);
	}
	p->stack.stack_i = 0;
}

static inline void draw_line(int x1, int y1, int x2, int y2, u8* page, u8 color, bool drawmode){
	// Draw a line :)
	if (abs(x2 - x1) >= abs(y2 - y1)){
		if (x1 == x2 && y1 == y2){
			grp_pixel(page,x1,y1,color,drawmode);
			return;
		}
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
			int y = FP_TO_INT(y_slope*(x-x1) + (1<<(FIXPOINT-1))) + y1;
			
			grp_pixel(page,x,y,color,drawmode);
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
			int x = FP_TO_INT(x_slope*(y-y1) + (1<<(FIXPOINT-1))) + x1;
			
			grp_pixel(page,x,y,color,drawmode);
		}
	}
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
		STACK_INT_RANGE(4,0,255,color);
	} else if (p->stack.stack_i == 4){
		color = p->graphics.color;
	} else {
		ERROR(ERR_WRONG_ARG_COUNT);
	}
	
	u8* page = grp_drawpage(p);
	
	// Draw a line :)
	draw_line(x1, y1, x2, y2, page, color, p->graphics.drawmode);
}

void cmd_gpset(struct ptc* p){
	int x, y;
	STACK_INT_RANGE_SILENT(0,0,GRP_WIDTH-1,x);
	STACK_INT_RANGE_SILENT(1,0,GRP_HEIGHT-1,y);
	
	u8 color;
	
	if (p->stack.stack_i == 3){
		STACK_INT_RANGE(2,0,255,color);
	} else {
		color = p->graphics.color;
	}
	
	u8* page = grp_drawpage(p);
	
	grp_pixel(page,x,y,color,p->graphics.drawmode);
}

void cmd_gdrawmd(struct ptc* p){
	STACK_INT_RANGE(0,0,1,p->graphics.drawmode);
}

void cmd_gprio(struct ptc* p){
	STACK_INT_RANGE(0,0,3,p->graphics.info[p->graphics.screen].prio);
}

void cmd_gputchr(struct ptc* p){
	// GPUTCHR x,y,resource,chr,pal,size
	// TODO:PERF:LOW Check performance of this and see if it can be optimized
	int x_start, y_start, chr, pal, size;
	void* resource;
	x_start = STACK_INT(0);
	y_start = STACK_INT(1);
	resource = value_str(ARG(2));
	STACK_INT_RANGE(3,0,255,chr);
	STACK_INT_RANGE(4,0,15,pal);
	size = STACK_INT(5);
	if (size == 1 || size == 2 || size == 4 || size == 8){
		// TODO:IMPL:MED GPUTCHR copies palette to COL2 memory
		// pointer to base resource
		u8* src = str_to_resource(p, resource);
		// pointer to relevant character
		src = &src[32*chr]; // TODO:CODE:NONE constant for size of one character
		
		// pointer to destination page
		u8* dest = grp_drawpage(p);
		// copy to destination
		// TODO:CODE:NONE constants for character size
		for (int y = 0; y < 8; ++y){
			for (int x = 0; x < 8; x+=2){
				uint_fast8_t col1 = src[x/2+4*y] & 0x0f;
				uint_fast8_t col2 = src[x/2+4*y] >> 4;
				for (int i = 0; i < size; ++i){
					for (int j = 0; j < size; ++j){
						int xp = x_start + x*size + j;
						int yp = y_start + y*size + i;
						if (col1 && xp >= 0 && xp < GRP_WIDTH && yp && yp < GRP_HEIGHT){
							grp_pixel(dest, xp, yp, 16*pal+col1, p->graphics.drawmode);
						}
					}
				}
				for (int i = 0; i < size; ++i){
					for (int j = 0; j < size; ++j){
						int xp = x_start + x*size + j + size;
						int yp = y_start + y*size + i;
						if (col2 && xp >= 0 && xp < GRP_WIDTH && yp && yp < GRP_HEIGHT){
							grp_pixel(dest, xp, yp, 16*pal+col2, p->graphics.drawmode);
						}
					}
				}
			}
		}
		
	} else {
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
}

void func_gspoit(struct ptc* p){
	int page, x, y;
	u8* dest;
	if (p->exec.argcount == 2){
		dest = grp_drawpage(p);
		x = STACK_REL_INT(-2);
		y = STACK_REL_INT(-1);
		p->stack.stack_i -= 2;
	} else {
		STACK_REL_INT_RANGE(-3,0,3,page);
		x = STACK_REL_INT(-2);
		y = STACK_REL_INT(-1);
		p->stack.stack_i -= 3;
		dest = p->res.grp[page];
	}
	
	if (x >= GRP_WIDTH || x < 0 || y < 0 || y >= GRP_HEIGHT){
		STACK_RETURN_NUM(-INT_TO_FP(1));
		return;
	}
	STACK_RETURN_INT(dest[grp_index(x,y)]);
}

void cmd_gcopy(struct ptc* p){
	// GCOPY [srcpage] x1 y1 x2 y2 x3 y3 mode
	int page;
	u8* src, * dest;
	int x1, x2, x3;
	int y1, y2, y3;
	int temp;
	bool mode;
	if (p->stack.stack_i == 8){
		STACK_INT_RANGE(0,0,3,page);
		x1 = STACK_INT(1);
		y1 = STACK_INT(2);
		x2 = STACK_INT(3);
		y2 = STACK_INT(4);
		x3 = STACK_INT(5);
		y3 = STACK_INT(6);
		mode = STACK_INT(7);
	} else {
		page = p->graphics.info[p->graphics.screen].drawpage;
		x1 = STACK_INT(0);
		y1 = STACK_INT(1);
		x2 = STACK_INT(2);
		y2 = STACK_INT(3);
		x3 = STACK_INT(4);
		y3 = STACK_INT(5);
		mode = STACK_INT(6);
	}
	
	src = p->res.grp[page];
	dest = p->res.grp[p->graphics.info[p->graphics.screen].drawpage];
	
	// bounds checking!
	// TODO:TEST:LOW bounds check behavior
//	if (x1 < 0) { x1 = 0; }
//	if (x2 < 0) { x2 = 0; }
//	if (x3 < 0) { x3 = 0; }
//	if (x1 > GRP_WIDTH-1) { x1 = GRP_WIDTH-1; }
//	if (x2 > GRP_WIDTH-1) { x2 = GRP_WIDTH-1; }
//	if (x3 > GRP_WIDTH-1) { x3 = GRP_WIDTH-1; }
	
//	if (y1 < 0) { y1 = 0; }
//	if (y2 < 0) { y2 = 0; }
//	if (y3 < 0) { y3 = 0; }
//	if (y1 > GRP_HEIGHT-1) { y1 = GRP_HEIGHT-1; }
//	if (y2 > GRP_HEIGHT-1) { y2 = GRP_HEIGHT-1; }
//	if (y3 > GRP_HEIGHT-1) { y3 = GRP_HEIGHT-1; }
	
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
	
	// TODO:TEST:HIGH Check that this works in each direction, overlaps, offscreen
	iprintf("gcopy %d,%d,%d,%d,%d,%d\n", x1, y1, x2, y2, x3, y3);
	if (y3 < y1 || x3 < x1){
		for (int y = y1; y <= y2; ++y){
			for (int x = x1; x <= x2; ++x){
				u8 p;
				if (x >= 0 && y >= 0 && x < GRP_WIDTH && y < GRP_HEIGHT){
					p = src[grp_index(x,y)];
				} else {
					p = 0;
				}
				int dx, dy;
				dx = x - x1 + x3;
				dy = y - y1 + y3;
				if (mode || p){
					if (dx >= 0 && dy >= 0 && dx < GRP_WIDTH && dy < GRP_HEIGHT){
						dest[grp_index(dx,dy)] = p;
					}
				}
			}
		}
	} else {
		for (int y = y2; y >= y1; --y){
			for (int x = x2; x >= x1; --x){
				u8 p;
				if (x >= 0 && y >= 0 && x < GRP_WIDTH && y < GRP_HEIGHT){
					p = src[grp_index(x,y)];
				} else {
					p = 0;
				}
				int dx, dy;
				dx = x - x1 + x3;
				dy = y - y1 + y3;
				if (mode || p){
					if (dx >= 0 && dy >= 0 && dx < GRP_WIDTH && dy < GRP_HEIGHT){
						dest[grp_index(dx,dy)] = p;
					}
				}
			}
		}
	}
}

void cmd_gcircle(struct ptc* p){
	int x, y, r;
	u8 color = p->graphics.color;
	x = STACK_INT(0);
	y = STACK_INT(1);
	r = STACK_INT(2);
	if (p->stack.stack_i == 4){
		STACK_INT_RANGE(3,0,255,color);
	}
	u8* page = grp_drawpage(p);
	int drawmode = p->graphics.drawmode;
	// TODO:IMPL:LOW angle range version
	int angle_units = r + 16;
	// this implementation will be a bit slow it's just to have <something>
	for (int i = 0; i < angle_units; ++i){
		// TODO:CODE:NONE pi constant
		double angle = 2.0 * 3.141592653589 * i / angle_units;
		double angle2 = 2.0 * 3.141592653589 * (i + 1) / angle_units;
		int x1 = x + r * cos(angle);
		int y1 = y + r * sin(angle);
		int x2 = x + r * cos(angle2);
		int y2 = y + r * sin(angle2);
		
		draw_line(x1,y1,x2,y2,page,color,drawmode);
	}
}
