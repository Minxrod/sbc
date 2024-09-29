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

/*void init_display(struct sbc*);
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
void display_graphics(struct sbc* p, int screen, int prio);*/

#endif
