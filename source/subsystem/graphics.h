#pragma once

#include "common.h"
#include "subsystem/resources.h"

#include <assert.h>

struct ptc;

// TODO:CODE:NONE Determine whether 'screen' or 'page' should be used
struct graphics {
	u8 screen;
	u8 color;
	bool drawmode;
	
	// Information stored for two pages
	struct {
		u8 drawpage;
		u8 displaypage;
		
		u8 prio;
	} info[2];
};

void init_graphics(struct graphics* g);

/// Convert screen coordinates to GRP coordinates
static inline u16 grp_index(uint_fast8_t x, uint_fast8_t y){
	int_fast8_t cx = x / 64;
	int_fast8_t cy = y / 64;
	int_fast8_t tx = (x / 8) % 8;
	int_fast8_t ty = (y / 8) % 8;
	int_fast8_t px = x % 8;
	int_fast8_t py = y % 8;
	return px + py * 8 + tx * 64 + ty * 512 + cx * 4096 + cy * 16384;
}

// Note: No error checking here
static inline void grp_pixel(u8* page, int x, int y, u8 color, bool drawmode){
	if (x >= 0 && x < GRP_WIDTH){
		if (y >= 0 && y < GRP_HEIGHT){
			if (!drawmode){
				page[grp_index(x,y)] = color;
			} else {
				page[grp_index(x,y)] ^= color;
			}
		}
	}
}

// Need VERY complete tests for these
// TODO:TEST:MED GLINE
// TODO:TEST:MED GCIRCLE
// Need tests in general for edge cases of
// TODO:TEST:MED GBOX
// TODO:TEST:MED GFILL
void cmd_gpage(struct ptc* p);
void cmd_gcolor(struct ptc* p);
void cmd_gcls(struct ptc* p);
void cmd_gfill(struct ptc* p);
void cmd_gbox(struct ptc* p);
void cmd_gline(struct ptc* p);
void cmd_gpset(struct ptc* p);
void cmd_gdrawmd(struct ptc* p);
void cmd_gcircle(struct ptc* p);
void cmd_gprio(struct ptc* p);
void cmd_gputchr(struct ptc* p);

// TODO:TEST:MED gspoit
void func_gspoit(struct ptc* p);
