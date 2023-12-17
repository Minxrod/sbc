#pragma once

#include "common.h"

struct ptc;

struct graphics {
	u8 screen;
	u8 drawpage;
	u8 displaypage;
	
	u8 color;
};

void init_graphics(struct graphics* g);

u16 grp_index(uint_fast8_t x, uint_fast8_t y);

//TODO:TEST:LOW Need VERY complete tests for these
// GLINE, GCIRCLE
// Need tests in general for edge cases of
// GFILL, GBOX, GPSET
void cmd_gpage(struct ptc* p);
void cmd_gcolor(struct ptc* p);
void cmd_gcls(struct ptc* p);
void cmd_gfill(struct ptc* p);
void cmd_gbox(struct ptc* p);
void cmd_gline(struct ptc* p);
void cmd_gpset(struct ptc* p);
