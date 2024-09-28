#pragma once
#include <SDL2/SDL_surface.h>
#ifndef SBC_DISPLAY
#define SBC_DISPLAY

#include <SDL2/SDL.h>

struct display {
/*	struct tilemap console_map;
	struct tilemap console_bg_map;

	struct tilemap panel_text_map;
	struct tilemap panel_bg_map;

	struct graphic graphic;

	struct tilemap background_map;
	struct tilemap foreground_map;

	struct sprite_array sprites;
	struct sprite_array panel_keys;*/

	// Main window
	SDL_Window* window;
	// Surface of main window (blit destination)
	SDL_Surface* window_surface;

//	sfView* view;
//	sfRenderStates rs;
//	sfShader* shader;
//	sfRenderWindow* rw;

	// Resource textures
	SDL_Surface* chr_tex[12];
//	struct sfTexture* chr_tex[12];
//	struct sfTexture* col_tex;
};

#endif // SBC_DISPLAY
