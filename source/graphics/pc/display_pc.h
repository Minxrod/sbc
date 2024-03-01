#pragma once

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

void init_display(struct ptc*);
void free_display(struct display*);

void display_draw_all(struct ptc* p);
void display_draw_text(struct ptc* p, int screen, int prio);
void display_draw_background(struct ptc* p, int screen, int prio);
void display_draw_sprite(struct ptc* p, int screen, int prio);
void display_draw_graphics(struct ptc* p, int screen, int prio);
void display_draw_panel(struct ptc* p, int screen, int prio);

