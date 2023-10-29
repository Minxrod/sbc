#pragma once

#include "common.h"

struct ptc;

struct background {
	int_fast8_t page;
	
	struct bg_offset {
		fixp x;
		fixp y;
		
		fixp step_x;
		fixp step_y;
		
		fixp time;
	} ofs[2][2];
};

struct background* init_background();
void free_background(struct background* b);

u16 bg_index(uint_fast8_t x, uint_fast8_t y);

void cmd_bgpage(struct ptc* p);
void cmd_bgclr(struct ptc* p);
void cmd_bgput(struct ptc* p);
void cmd_bgfill(struct ptc* p);
void cmd_bgofs(struct ptc* p);
void cmd_bgread(struct ptc* p);
void cmd_bgclip(struct ptc* p);
void cmd_bgcopy(struct ptc* p);

void func_bgchk(struct ptc* p);
