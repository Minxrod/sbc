#pragma once
#ifndef SBC_DISPLAY
#define SBC_DISPLAY

#include "graphics/pc/tilemap.h"
#include "graphics/pc/sprite.h"
#include "graphics/pc/graphic.h"

#include <SFML/Graphics.h>

struct display {
	struct tilemap console_map;
	struct tilemap console_bg_map;
	
	struct tilemap panel_text_map;
	struct tilemap panel_bg_map;
	
	struct graphic graphic;
	
	struct tilemap background_map;
	struct tilemap foreground_map;
	
	struct sprite_array sprites;
	struct sprite_array panel_keys;
	
	sfView* view;
	sfRenderStates rs;
	sfShader* shader;
	sfRenderWindow* rw;
	
	// Resource textures
	struct sfTexture* chr_tex[12];
	struct sfTexture* col_tex;
};

/// Texture indices for CHR resources
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

#endif
