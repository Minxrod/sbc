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
#if !defined(SDL2) && !defined(SFML) && !defined(ARM9)
struct display {
	int _dummy_struct_for_headless_tests;
};
// mark dummy as included: no other copies to be included later
#define SBC_DISPLAY
#endif

struct sbc;
struct display;

void init_display(struct sbc*);
void free_display(struct display*);

void display_draw_all(struct sbc* p);

void display_console(struct sbc* p);
void display_panel_console(struct sbc* p);
void display_background(struct sbc* p, int screen, int layer);
void display_panel_background(struct sbc* p);
void display_sprite(struct sbc* p, int screen, int prio);
void display_panel_keys(struct sbc* p);
void display_icon(struct sbc* p);
void display_cursor(struct sbc* p);
void display_graphics(struct sbc* p, int screen, int prio);

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
