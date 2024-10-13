#include "graphics/sdl2/display_sdl.h"

#include "common.h"
#include "console.h"
#include "display.h"
#include "sdl2/sprite.h"
#include "sdl2/tilemap.h"
#include "resources.h"
#include "strs.h"
#include "system.h"

#include <GLES2/gl2.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

// TODO:CODE:HIGH remove!!! temporary while rebuilding entire file!
#pragma GCC diagnostic warning "-Wunused-variable"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-but-set-variable"

// This is fine for vertex shader, I don't really need anything specific here.
// This is a 2D identity function.
const char* vert_shader_src =
"#version 100\n"
"attribute vec2 pos;"
// Pass these to fragment shader
"attribute vec2 in_col;"
"attribute vec2 in_tex;"
"varying vec2 col;"
"varying vec2 tex;"
"uniform vec2 translation;"
"void main() {"
"	gl_Position = vec4( pos.x + translation.x, pos.y + translation.y, 0, 1 );"
"	col = in_col;"
"	tex = in_tex;"
"}"
;

// Needs my palette code.
const char* frag_shader_src =
"#version 100\n"
"varying mediump vec2 col;"
"varying mediump vec2 tex;"
"uniform sampler2D col_src;"
"uniform sampler2D chr_src;"
"void main() {"
"	highp vec4 c = texture2D(chr_src, tex.xy);"
"	highp vec4 a = texture2D(col_src, vec2(c.a + col.x / 16.0, col.y));"
"	gl_FragColor = a.rgba;"
"}"
;

#define TEXTURE_WIDTH_PIXELS 256
#define TEXTURE_WIDTH_BYTES (TEXTURE_WIDTH_PIXELS / 2)
#define CHR_WIDTH_BYTES 4


// TODO:CODE:MED
// maybe separate SDL init and GL init?
// For now they are very interrelated, though...
void init_gl(struct display* d){
	// Set up GLES 2 (widest range of support; WebGL, mobile, etc)
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );

	d->window = SDL_CreateWindow("SBC", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT * SCREEN_COUNT, SDL_WINDOW_OPENGL);
	if (!d->window){
		ABORT("Failed to create SDL window");
	}
	// Initialize context
	d->gl_context = SDL_GL_CreateContext(d->window);
	if (!d->gl_context){
		ABORT("Failed to create GL context");
	}

	// Build shaders (we really only need the one; it's for HW-accel'd palettes.)
	GLuint program = glCreateProgram();
	// First, the vertex shader
	GLint status;
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertex_shader, 1, &vert_shader_src, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE){
		GLsizei out_size;
		GLchar log[256];
		glGetShaderInfoLog(vertex_shader, 256, &out_size, log);
		printf("%s\n", log);
		ABORT("Failed to compile vertex shader");
	}

	// Attach vertex shader to program
	glAttachShader(program, vertex_shader);

	// Next, build fragment shader
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(frag_shader, 1, &frag_shader_src, NULL);
	glCompileShader(frag_shader);
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE){
		GLsizei out_size;
		GLchar log[256];
		glGetShaderInfoLog(frag_shader, 256, &out_size, log);
		printf("%s\n", log);
		ABORT("Failed to compile fragment shader");
	}

	// Attach fragment shader
	glAttachShader(program, frag_shader);

	// Link and continue
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE){
		GLsizei out_size;
		GLchar log[256];
		glGetProgramInfoLog(program, 256, &out_size, log);
		printf("%s\n", log);
		ABORT("Failed to link shaders");
	}

	// This is used for everything, so don't both detaching until the end
	glUseProgram(program);

	// Destroy local copies
	glDeleteShader(vertex_shader);
	glDeleteShader(frag_shader);

	// Get the position information
	d->gl_pos = glGetAttribLocation(program, "pos");
	d->gl_col = glGetAttribLocation(program, "in_col");
	d->gl_tex = glGetAttribLocation(program, "in_tex");
	if ((d->gl_pos | d->gl_col | d->gl_tex) == -1){
		ABORT("Failed to find shader attributes");
	}

	d->gl_col_tex = glGetUniformLocation(program, "col_src");
	d->gl_chr_tex = glGetUniformLocation(program, "chr_src");
	d->gl_trans = glGetUniformLocation(program, "translation");
	if ((d->gl_col_tex | d->gl_chr_tex | d->gl_trans) == -1){
		ABORT("Failed to find shader uniform");
	}

	glUniform1i(d->gl_col_tex, 0);
	glUniform1i(d->gl_chr_tex, 1);

	// https://stackoverflow.com/questions/1617370/how-to-use-alpha-transparency-in-opengl
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// TODO:CODE:HIGH
	// delete program object at end of main/after close?
	// close GL context somehow?
	// research GL memory management
}


GLuint gen_chr_texture(const u8* const src, const size_t chr_count){
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glGenerateMipmap(GL_TEXTURE_2D);
	if (glGetError()) ABORT("");

	u8 array[chr_count * CHR_WIDTH * CHR_HEIGHT];
	for (size_t i = 0; i < chr_count; ++i){
		int x = i % 32;
		int y = i / 32;
		for (int cy = 0; cy < CHR_HEIGHT; ++cy){
			for (int cx = 0; cx < CHR_WIDTH; cx += 2){
				const u8 c = src[CHR_UNIT_SIZE * i + cx / 2 + 4 * cy];
				const int pixel_index = TEXTURE_WIDTH_PIXELS * (CHR_HEIGHT * y + cy) + CHR_WIDTH * x + cx;
				array[pixel_index + 0] = c & 0x0F;
				array[pixel_index + 1] = c >> 4;
			}
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, TEXTURE_WIDTH_PIXELS, chr_count / 4, 0, GL_ALPHA, GL_UNSIGNED_BYTE, array);

	// TODO:CODE:LOW why is it size / 4 for height? Determine full expression?
	return texture;
}

GLuint gen_col_texture(const u16* const src){
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glGenerateMipmap(GL_TEXTURE_2D);
	if (glGetError()) ABORT("");

	u8 array[4*256*COL_BANKS*SCREEN_COUNT]; // no {0} because it will all be generated by the end
	for (size_t i = 0; i < 256*COL_BANKS*SCREEN_COUNT; ++i){
		u16 s = src[i];
//		s = ((s & 0xff00) >> 8) | ((s & 0x00ff) << 8);

		array[4*i+0] = (s & 0x001f) << 3; //TODO:IMPL:LOW Adjust values to match PTC
		array[4*i+1] = ((s & 0x03e0) >> 2) | ((s & 0x8000) >> 13);
		array[4*i+2] = ((s & 0x7c00) >> 7);
		if ((i / 256) % 3 != 2){
			// BG, SP
			array[4*i+3] = (i % 16) ? 255 : 0;
		} else {
			// GRP
			array[4*i+3] = (i % 256) ? 255 : 0;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, COL_BANKS * SCREEN_COUNT, 0, GL_RGBA, GL_UNSIGNED_BYTE, array);

	return texture;
}

void regen_col_tex(GLuint texture, const u16* const src){
	u8 array[4*256*COL_BANKS*SCREEN_COUNT]; // no {0} because it will all be generated by the end
	for (size_t i = 0; i < 256*COL_BANKS*SCREEN_COUNT; ++i){
		u16 s = src[i];
//		s = ((s & 0xff00) >> 8) | ((s & 0x00ff) << 8);

		array[4*i+0] = (s & 0x001f) << 3; //TODO:IMPL:LOW Adjust values to match PTC
		array[4*i+1] = ((s & 0x03e0) >> 2) | ((s & 0x8000) >> 13);
		array[4*i+2] = ((s & 0x7c00) >> 7);
		if ((i / 256) % 3 != 2){
			// BG, SP
			array[4*i+3] = (i % 16) ? 255 : 0;
		} else {
			// GRP
			array[4*i+3] = (i % 256) ? 255 : 0;
		}
	}
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, COL_BANKS * SCREEN_COUNT, GL_RGBA, GL_UNSIGNED_BYTE, array);
}

// Regenerate a single bank
void regen_chr_tex(GLuint texture, int bank, const u8* const src){
	u8 array[256 * CHR_WIDTH * CHR_HEIGHT];
	for (size_t i = 0; i < 256; ++i){
		int x = i % 32;
		int y = i / 32;
		for (int cy = 0; cy < CHR_HEIGHT; ++cy){
			for (int cx = 0; cx < CHR_WIDTH; cx += 2){
				const u8 c = src[CHR_UNIT_SIZE * i + cx / 2 + 4 * cy];
				const int pixel_index = TEXTURE_WIDTH_PIXELS * (CHR_HEIGHT * y + cy) + CHR_WIDTH * x + cx;
				array[pixel_index + 0] = c & 0x0F;
				array[pixel_index + 1] = c >> 4;
			}
		}
	}
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 64 * bank, 256, 64, GL_ALPHA, GL_UNSIGNED_BYTE, array);
}

void display_draw_all(struct sbc* p){
	// Regeneration of changed textures
	if (p->res.regen_col){
		regen_col_tex(p->display.col_tex, p->res.col_banks);
		p->res.regen_col = false;
	}
	// 1 bit indicates to increase the texture ID and reset the bank
	// TODO:CODE:MED it would be nice to encode all the offset/CHR texture stuff into some generated code
	// so that the resource counts could change in the future...
	uint64_t tex_inc_mask = 0x280888 | ((uint64_t)0x280888 << CHR_BANKS);
	int tex_src = 0;
	int bank = 0;
	for (int i = 0; i < CHR_BANKS * SCREEN_COUNT; ++i){
		// Check if textures need regen
		if (p->res.regen_chr[i]) {
			regen_chr_tex(p->display.chr_tex[tex_src], bank, p->res.chr[i]);
			p->res.regen_chr[i] = false;
		}
		// Advance texture locations
		if (tex_inc_mask & 1){
			bank = 0;
			tex_src++;
		} else {
			++bank;
		}
		tex_inc_mask >>= 1;
//		iprintf("tex_src=%d\n", tex_src);
		assert(tex_src < 10 || i == CHR_BANKS*SCREEN_COUNT-1);
		assert(bank < 8);
	}

	// =============
	// Lower screen
	// =============
	glViewport(0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, p->display.col_tex);
	glClear(GL_COLOR_BUFFER_BIT);
	// lowest prio
	display_graphics(p, 0, 3);
	display_sprite(p, 0, 3);
	display_background(p, 0, 1);
	display_graphics(p, 0, 2);
	display_sprite(p, 0, 2);
	display_background(p, 0, 0);
	display_graphics(p, 0, 1);
	display_sprite(p, 0, 1);
	display_console(p);
	display_graphics(p, 0, 0);
	display_sprite(p, 0, 0);
	display_cursor(p);
	// highest prio

	// =============
	// Lower screen
	// =============
//	sfView_setViewport(d->view, (sfFloatRect){0,  1.0 / SCREEN_COUNT, 1,  1.0 / SCREEN_COUNT});
//	sfRenderWindow_setView(d->rw, d->view);
	// lowest prio

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	display_graphics(p, 1, 3);
	display_sprite(p, 1, 3);
	display_background(p, 1, 1);
	display_graphics(p, 1, 2);
	display_sprite(p, 1, 2);
	display_background(p, 1, 0);
	display_graphics(p, 1, 1);
	display_sprite(p, 1, 1);
	display_panel_background(p);
	display_panel_keys(p); // TODO:TEST:LOW Check that panel priority here is actually correct
	display_panel_console(p);
	display_graphics(p, 1, 0);
	display_sprite(p, 1, 0);
	display_icon(p);

	SDL_GL_SwapWindow(p->display.window);
}

GLuint gen_grp_texture(void){
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (glGetError()) ABORT("");

	u8* data = calloc(GRP_WIDTH, GRP_HEIGHT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, GRP_WIDTH, GRP_HEIGHT, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
	free(data);
	return texture;
}

void init_display(struct sbc* p){
	p->display.console_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT, 128);
	p->display.console_bg_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT, 256);
	p->display.panel_text_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT, 128);
	p->display.panel_bg_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT, 256);
	p->display.background_map = init_tilemap(SCREEN_WIDTH / CHR_WIDTH + 1, SCREEN_HEIGHT / CHR_HEIGHT + 1, 256);
	p->display.sprites = init_sprite_array(2048/4);
	p->display.sprites_small = init_sprite_array(512/4);
}

void init_display_2(struct sbc* p){
	struct resources* r = &p->res;
	struct display* d = &p->display;
	for (int page = 0; page < SCREEN_COUNT; ++page){
		d->chr_tex[0+5*page] = gen_chr_texture(r->chr[0+CHR_BANKS*page], 512); //BGF
		d->chr_tex[1+5*page] = gen_chr_texture(r->chr[4+CHR_BANKS*page], 1024); //BGD
		d->chr_tex[2+5*page] = gen_chr_texture(r->chr[8+CHR_BANKS*page], 1024); //BGU
		d->chr_tex[3+5*page] = gen_chr_texture(r->chr[12+CHR_BANKS*page], 2048); // SPU or SPD
		d->chr_tex[4+5*page] = gen_chr_texture(r->chr[20+CHR_BANKS*page], 512); //SPS
	}
	d->col_tex = gen_col_texture(r->col_banks);
	d->grp_tex = gen_grp_texture();
}

void free_display(struct display* d){
	for (int i = 0; i < SCREEN_COUNT; ++i){
		glDeleteTextures(10, d->chr_tex);
	}
	free_tilemap(&d->console_map);
	free_tilemap(&d->console_bg_map);
	free_tilemap(&d->panel_text_map);
	free_tilemap(&d->panel_bg_map);
	free_tilemap(&d->background_map);
	free_sprite_array(&d->sprites);
	free_sprite_array(&d->sprites_small);
}

/// Draws the upper screen text console foreground and background.
///
/// Display of the foreground can be toggled with `VISIBLE`.
/// The color background cannot be disabled.
void display_console(struct sbc* p){
	struct display* d = &p->display;

//	glEnable(GL_TEXTURE_2D);

	// Update the tilemaps
	for (int x = 0; x < CONSOLE_WIDTH; ++x){
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			tile(&d->console_map, x, y, to_char(con_text_getc(&p->console, x, y)), 0, 0);
			palette(&d->console_map, x, y, con_col_get(&p->console, x, y) & COL_FG_MASK);

			u8 c = (con_col_get(&p->console, x, y) & COL_BG_MASK) >> 4;
			tile(&d->console_bg_map, x, y, c ? 15 : 0, 0, 0);
			palette(&d->console_bg_map, x, y, c);
		}
	}

	// Background color layer
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, p->display.chr_tex[SBC_TEX_BGD_UPPER]);
	draw_tilemap(d, &d->console_bg_map);
	// Text layer
	if (p->res.visible & VISIBLE_CONSOLE){
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, p->display.chr_tex[SBC_TEX_BGF_UPPER]);

		draw_tilemap(d, &d->console_map);
	}
}

void display_panel_console(struct sbc* p){
	struct display* d = &p->display;
	// Update the tilemaps
	for (int x = 0; x < CONSOLE_WIDTH; ++x){
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			struct console* c = p->panel.keys_text; // function key text
			if (p->panel.type == PNL_OFF || p->panel.type == PNL_PNL){
				// lower screen text console
				c = p->panel.text;
			}
			tile(&d->panel_text_map, x, y, to_char(con_text_getc(c, x, y)), 0, 0);
			palette(&d->panel_text_map, x, y, con_col_get(c, x, y) & COL_FG_MASK);
		}
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, p->display.chr_tex[SBC_TEX_BGF_LOWER]);
	draw_tilemap(d, &d->panel_text_map);
//	draw_va(d, SBC_COL_BG_LOWER, SBC_TEX_BGF_LOWER, d->panel_text_map.va);
}

void display_background(struct sbc* p, int screen, int layer){
	// Check if these should even be rendered
	if (!(p->res.visible & VISIBLE_BG0) && layer == 0) return;
	if (!(p->res.visible & VISIBLE_BG1) && layer == 1) return;
	struct display* d = &p->display;

	// TODO:IMPL:MED BGCLIP bounds
	// It's trickier than you'd think due to weird wrapping rules...
	// How can this be done...?
	// Add it to the shader?
//	sfView_setViewport(d->view, (sfFloatRect){0, screen * 0.5f, 1, 0.5f});
//	sfRenderWindow_setView(d->rw, d->view);
	// 0 - 7 -> 0
	// tile unit to start at
	int start_x = (uint32_t)(FP_TO_INT(p->background.ofs[screen][layer].x) / 8) % BG_WIDTH;
	int start_y = (uint32_t)(FP_TO_INT(p->background.ofs[screen][layer].y) / 8) % BG_HEIGHT;
	// offset within tile
	float ofs_x = (uint32_t)(FP_TO_INT(p->background.ofs[screen][layer].x)) % 8;
	float ofs_y = (uint32_t)(FP_TO_INT(p->background.ofs[screen][layer].y)) % 8;

	// at most 33 tiles can be displayed in x direction
	// 25 in y direction
	for (int x = 0; x < SCREEN_WIDTH / CHR_WIDTH + 1; ++x){
		for (int y = 0; y < SCREEN_HEIGHT / CHR_HEIGHT + 1; ++y){
			u16 td = bg_tile(p,screen,layer,(x + start_x) % BG_WIDTH,(y + start_y) % BG_HEIGHT);
			tile(&d->background_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
			palette(&d->background_map, x, y, (td & 0xf000) >> 12);
		}
	}

	// TODO:IMPL:MED translation
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, p->display.chr_tex[screen ? SBC_TEX_BGU_LOWER : SBC_TEX_BGU_UPPER]);
	glUniform2f(d->gl_trans, ofs_x * FLOAT_PIXEL_WIDTH, ofs_y * FLOAT_PIXEL_HEIGHT);
	draw_tilemap(d, &d->background_map);
	glUniform2f(d->gl_trans, 0, 0);
}

void display_panel_background(struct sbc* p){
	if (SCREEN_COUNT < 2) return;
	struct display* d = &p->display;
	if (p->panel.type == PNL_OFF) return;
	if (!(p->res.visible & VISIBLE_PANEL)) return;

	for (int x = 0; x < CONSOLE_WIDTH; ++x){
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			// Panel BG map
			u16 td = p->res.scr[SCR_BANKS+1][x+CONSOLE_WIDTH*y];
			tile(&d->panel_bg_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
			palette(&d->panel_bg_map, x, y, (td & 0xf000) >> 12);
		}
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, p->display.chr_tex[SBC_TEX_BGD_LOWER]);
	draw_tilemap(d, &d->panel_bg_map);
}

void display_sprite(struct sbc* p, int screen, int prio){
	// Don't display over panel
	if (screen == 1 && (p->panel.type != PNL_OFF || !(p->res.visible & VISIBLE_PANEL))) return;
	if (!(p->res.visible & VISIBLE_SPRITE)) return;

	struct display* d = &p->display;
	struct sprite_array* s = screen ? &d->sprites_small : &d->sprites;
	reset_sprite_array(s);

	for (int i = 0; i < MAX_SPRITES; ++i){
		if (p->sprites.info[screen][i].active && p->sprites.info[screen][i].prio == prio){
			add_sprite(s, &p->sprites.info[screen][i]);
		}
	}
	// TODO:IMPL:HIGH regen_chr, regen_col

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, p->display.chr_tex[screen ? SBC_TEX_SPS_LOWER : SBC_TEX_SPU_UPPER]);
	draw_sprite_array(d, s);
}

void display_panel_keys(struct sbc* p){
	if (SCREEN_COUNT < 2) return;
	// Only render keyboard when enabled
	if (p->panel.type == PNL_OFF || p->panel.type == PNL_PNL) return;
	if (!(p->res.visible & VISIBLE_PANEL)) return;

	struct display* d = &p->display;
	struct sprite_array* sprites = &d->sprites;
	reset_sprite_array(sprites);

	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
	}

	for (int i = 0; i < KEYBOARD_KEYS; ++i){
		struct sprite_info* key = &p->panel.keys[i];
		// active is a good check to see if the sprite is correctly defined, for now
		if (key->active){
			add_sprite(sprites, key);
		}
	}

	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(-1));
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, p->display.chr_tex[SBC_TEX_SPD_LOWER]);
	draw_sprite_array(d, sprites);
}

void display_icon(struct sbc* p){
	struct display* d = &p->display;
	// Icons are basically always rendered
//	struct sprite_array icon_sprites = init_sprite_array();

	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
	}

	for (int i = 0; i < ICON_KEYS; ++i){
		struct sprite_info* key = &p->panel.keys[ICON_PAGE_START+i];
		// active is a good check to see if the sprite is correctly defined, for now
		if (key->active){
//			add_sprite(icon_sprites, key);
		}
	}

	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(-1));
	}
//	draw_va(d, SBC_COL_SPR_LOWER, SBC_TEX_SPD_LOWER, icon_sprites.va);

//	free_sprite_array(icon_sprites);
}

void display_cursor(struct sbc* p){
	if (!p->console.cursor_visible) return;
	if (p->time.time % FRAMERATE > (FRAMERATE / 2)) return;
	struct display* d = &p->display;

//	struct sprite_array cursor_sprite = init_sprite_array();
	struct sprite_info cursor = init_sprite_info(100,108,0,0,0,0,8,8);
	cursor.pos.x = INT_TO_FP(p->console.x * 8);
	cursor.pos.y = INT_TO_FP(p->console.y * 8);

//	add_sprite(cursor_sprite, &cursor);

//	draw_va(d, SBC_COL_SPR_UPPER, SBC_TEX_SPS_UPPER, cursor_sprite.va);

//	free_sprite_array(cursor_sprite);
}

void display_graphics(struct sbc* p, int screen, int prio){
	if (screen == 1 && !(p->panel.type == PNL_OFF || p->panel.type == PNL_PNL)) return;
	if (screen == 1 && !(p->res.visible & VISIBLE_PANEL)) return;
	if (!(p->res.visible & VISIBLE_GRAPHICS)) return;

	struct display* d = &p->display;
	if (prio == p->graphics.info[screen].prio){
		u8 grp[GRP_SIZE];
		for (int y = 0; y < GRP_HEIGHT; ++y){
			for (int x = 0; x < GRP_WIDTH; ++x){
				u8 col = p->res.grp[p->graphics.info[screen].displaypage][grp_index(x, y)];
				grp[x+(y*GRP_WIDTH)] = col;
			}
		}

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, d->grp_tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GRP_WIDTH, GRP_HEIGHT, GL_ALPHA, GL_UNSIGNED_BYTE, grp);

		glEnableVertexAttribArray(d->gl_pos);
		glEnableVertexAttribArray(d->gl_col);
		glEnableVertexAttribArray(d->gl_tex);

		vertex grp_vertices[6] = {
			{-1.f, 1.f, 0.f, 0.4, 0., 0.}, // 0
			{1.f, 1.f, 0.f, 0.4, 1., 0.}, // 1
			{1.f, -1.f, 0.f, 0.4, 1., 1.}, // 2
			{-1.f, 1.f, 0.f, 0.4, 0., 0.}, // 0
			{1.f, -1.f, 0.f, 0.4, 1., 1.}, // 2
			{-1.f, -1.f, 0.f, 0.4, 0., 1.}, // 3
		};
		GLuint grp_buf;
		glGenBuffers(1, &grp_buf);
		glBindBuffer(GL_ARRAY_BUFFER, grp_buf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(grp_vertices), grp_vertices, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(d->gl_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_X * sizeof(GLfloat)));
		glVertexAttribPointer(d->gl_col, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_PALETTE * sizeof(GLfloat)));
		glVertexAttribPointer(d->gl_tex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_U * sizeof(GLfloat)));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(d->gl_pos);
		glDisableVertexAttribArray(d->gl_col);
		glDisableVertexAttribArray(d->gl_tex);

		glDeleteBuffers(1, &grp_buf);
	}
}

