#pragma once
#ifndef SBC_DISPLAY
#define SBC_DISPLAY

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GLES2/gl2.h>

#include "tilemap.h"
#include "sprite.h"

#include <stdbool.h>

struct display {
	struct tilemap console_map;
	struct tilemap console_bg_map;

	struct tilemap panel_text_map;
	struct tilemap panel_bg_map;

//	struct graphic graphic;

	struct tilemap background_map;
	struct tilemap foreground_map;

	/// Sprites with texture of 512 pixels (SPU, SPD+SPK)
	struct sprite_array sprites;
	/// Sprites with texture of 128 pixels (SPS)
	struct sprite_array sprites_small;
//	struct sprite_array panel_keys;

	// Main window
	SDL_Window* window;
	// Window GL context
	SDL_GLContext* gl_context;

	// Resource textures
	GLuint chr_tex[12];
	GLuint col_tex;
	GLuint grp_tex;

	GLuint shader;

	/// Shader information inputs
	// Shader Vertex information
	GLint gl_pos;
	GLint gl_trans;

	// Shader fragment information inputs
	GLint gl_col;
	GLint gl_tex;

	GLint gl_col_tex;
	GLint gl_chr_tex;
};

void init_gl(struct display* d);
struct sbc;
void init_display_2(struct sbc* p);

#endif // SBC_DISPLAY
