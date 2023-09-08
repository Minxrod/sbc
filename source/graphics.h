#pragma once

#include "common.h"

struct ptc;

struct graphics {
	// TODO:CODE Could these be better stored in one u8?
	u8 screen;
	u8 drawpage;
	u8 displaypage;
	
	u8 color;
	// TODO:PERF Save draw pointer based on GPAGE
//	u8* drawgrp;
};

void init_graphics(struct graphics* g);

u16 grp_index(int_fast8_t x, int_fast8_t y);

void cmd_gpage(struct ptc* p);
void cmd_gcolor(struct ptc* p);
void cmd_gcls(struct ptc* p);