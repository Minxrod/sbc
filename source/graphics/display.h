#pragma once

#ifdef SFML
#include "graphics/pc/display_pc.h"
#endif
#ifdef ARM9
#include "graphics/nds/display_nds.h"
#endif
#ifdef SDL2
#include "graphics/sdl2/display_sdl.h"
#endif

struct ptc;
struct display;

void init_display(struct ptc*);
void free_display(struct display*);

void display_draw_all(struct ptc* p);

void display_console(struct ptc* p);
void display_panel_console(struct ptc* p);
void display_background(struct ptc* p, int screen, int layer);
void display_panel_background(struct ptc* p);
void display_sprite(struct ptc* p, int screen, int prio);
void display_panel_keys(struct ptc* p);
void display_icon(struct ptc* p);
void display_cursor(struct ptc* p);
void display_graphics(struct ptc* p, int screen, int prio);

enum sbc_tex {
	SBC_TEX_BGF_UPPER,
	SBC_TEX_BGD_UPPER,
	SBC_TEX_BGU_UPPER,
	SBC_TEX_SPU_UPPER,
	SBC_TEX_SPS_UPPER,
	SBC_TEX_BGF_LOWER,
	SBC_TEX_BGD_LOWER,
	SBC_TEX_BGU_LOWER,
	SBC_TEX_SPD_LOWER,
	SBC_TEX_SPS_LOWER,
};

/// Color palette index into col_tex
enum sbc_col {
	SBC_COL_BG_UPPER,
	SBC_COL_SPR_UPPER,
	SBC_COL_GRP_UPPER,
	SBC_COL_BG_LOWER,
	SBC_COL_SPR_LOWER,
	SBC_COL_GRP_LOWER,
};
