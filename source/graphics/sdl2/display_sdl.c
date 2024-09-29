#include "graphics/sdl2/display_sdl.h"

#include "common.h"
#include "display.h"
#include "resources.h"
#include "system.h"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

// TODO:CODE:HIGH remove!!! temporary while rebuilding entire file!
#pragma GCC diagnostic warning "-Wunused-variable"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-but-set-variable"

#define TEXTURE_WIDTH_PIXELS 256
#define TEXTURE_WIDTH_BYTES (TEXTURE_WIDTH_PIXELS / 2)
#define CHR_WIDTH_BYTES 4

void display_draw_all(struct sbc* p){
	// lowest prio
/*	display_graphics(p, 0, 3);
	display_sprite(p, 0, 3);
	display_background(p, 0, 1);
	display_graphics(p, 0, 2);
	display_sprite(p, 0, 2);
	display_background(p, 0, 0);
	display_graphics(p, 0, 1);
	display_sprite(p, 0, 1);
*/	display_console(p);
	/*display_graphics(p, 0, 0);
	display_sprite(p, 0, 0);
	display_cursor(p);*/
	// highest prio

	// =============
	// Lower screen
	// =============
//	sfView_setViewport(d->view, (sfFloatRect){0,  1.0 / SCREEN_COUNT, 1,  1.0 / SCREEN_COUNT});
//	sfRenderWindow_setView(d->rw, d->view);
	// lowest prio

/*	display_graphics(p, 1, 3);
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
	display_icon(p);*/
	(void)p;
}


SDL_Surface* gen_chr_texture(const u8* const src, const size_t size){
	SDL_Surface* surface = SDL_CreateRGBSurface(0, TEXTURE_WIDTH_PIXELS, size / 4, 4, 0, 0, 0, 0);
	if (!surface){
		iprintf("Error creating SDL surface: %s\n", SDL_GetError());
		return NULL;
	}
	if (SDL_SetColorKey(surface, SDL_TRUE, 0)){
		iprintf("Error setting transparency: %s\n", SDL_GetError());
		return NULL;
	}
/*	if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND)){
		iprintf("Error creating SDL surface: %s\n", SDL_GetError());
		return NULL;
	}*/
	u8* array = (u8*)surface->pixels;
	for (size_t i = 0; i < size; ++i){
		int x = i % 32;
		int y = i / 32;
		for (int cy = 0; cy < CHR_HEIGHT; ++cy){
			for (int cx = 0; cx < CHR_WIDTH; cx += 2){
				u8 c = src[CHR_UNIT_SIZE * i + cx / 2 + 4 * cy];
				c = (c << 4) | (c >> 4);
//				iprintf("%d,%d,%d,%d\n", x, y, cx, cy);
				array[TEXTURE_WIDTH_BYTES * (CHR_HEIGHT * y + cy) + CHR_WIDTH_BYTES * x + cx / 2] = c;
//				array[4*(8*x+cx+256*(8*y+cy))] = ((cx & 1) ? (c >> 4) & 0x0f : c & 0x0f);
			}
		}
	}

	// TODO:CODE:LOW why is it size / 4 for height? Determine full expression?
	return surface;
}

void set_pal16(SDL_Surface* surface, const u16* const src, int pal){
	// TODO:PERF:MED Precompute colors array for all palettes, only pass pointers to each palette.
	// Still involves a copy but this is software anyways, the color palettes are small enough to be OK.
	// I hope.
	SDL_Color colors[16];
	const int src_pal_offset = 16 * pal;
	for (int i = 0; i < 16; ++i){
		u16 s = src[i + src_pal_offset];
//		s = ((s & 0xff00) >> 8) | ((s & 0x00ff) << 8);

		colors[i].r = (s & 0x001f) << 3; //TODO:IMPL:LOW Adjust values to match PTC
		colors[i].g = ((s & 0x03e0) >> 2) | ((s & 0x8000) >> 13);
		colors[i].b = ((s & 0x7c00) >> 7);
		colors[i].a = i ? 255 : 0;
	}

	if (SDL_SetPaletteColors(surface->format->palette, colors, 0, 16)){
		iprintf("Failed to set texture palette %s\n", SDL_GetError());
	}
}


void init_display(struct sbc* p){
	struct resources* r = &p->res;
	struct display* d = &p->display;
	for (int page = 0; page < SCREEN_COUNT; ++page){
		d->chr_tex[0+5*page] = gen_chr_texture(r->chr[0+CHR_BANKS*page], 512); //BGF
		d->chr_tex[1+5*page] = gen_chr_texture(r->chr[4+CHR_BANKS*page], 1024); //BGD
		d->chr_tex[2+5*page] = gen_chr_texture(r->chr[8+CHR_BANKS*page], 1024); //BGU
		d->chr_tex[3+5*page] = gen_chr_texture(r->chr[12+CHR_BANKS*page], 2048); // SPU or SPD
		d->chr_tex[4+5*page] = gen_chr_texture(r->chr[20+CHR_BANKS*page], 512); //SPS
	}
}

void free_display(struct display* d){
	for (int i = 0; i < SCREEN_COUNT; ++i){
		SDL_FreeSurface(d->chr_tex[i]);
	}
}

// Get CHR source rectangle from a texture (assumed 256*(64*banks))
SDL_Rect get_chr_src_rect(int chr){
	int x = chr % 32;
	int y = chr / 32;
	return (SDL_Rect){CHR_WIDTH_BYTES * x, CHR_HEIGHT * y, CHR_WIDTH, CHR_HEIGHT};
}

SDL_Rect get_chr_dest_rect(int x, int y){
	return (SDL_Rect){CHR_WIDTH * x, CHR_HEIGHT * y, CHR_WIDTH, CHR_HEIGHT};
}


/// Draws the upper screen text console foreground and background.
///
/// Display of the foreground can be toggled with `VISIBLE`.
/// The color background cannot be disabled.
void display_console(struct sbc* p){
	struct display* d = &p->display;

	for (int x = 0; x < CONSOLE_WIDTH; ++x){
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			u8 c = (con_col_get(&p->console, x, y) & COL_BG_MASK) >> 4;
			SDL_Rect src = get_chr_src_rect(c ? 15 : 0);
			SDL_Rect dest = get_chr_dest_rect(x, y); // same between BG and FG

			SDL_Surface* bgd = d->chr_tex[SBC_TEX_BGD_UPPER];
			set_pal16(bgd, p->res.col[SBC_COL_BG_UPPER], c);
			SDL_BlitSurface(bgd, &src, d->window_surface, &dest);

			SDL_Surface* bgf = d->chr_tex[SBC_TEX_BGF_UPPER];
			src = get_chr_src_rect(to_char(con_text_getc(&p->console, x, y)));
			set_pal16(bgf, p->res.col[SBC_COL_BG_UPPER], con_col_get(&p->console, x, y) & COL_FG_MASK);
			SDL_BlitSurface(bgf, &src, d->window_surface, &dest);
		}
	}

	// Background color layer
//	draw_va(d, SBC_COL_BG_UPPER, SBC_TEX_BGD_UPPER, d->console_bg_map.va);
	// Text layer
	if (p->res.visible & VISIBLE_CONSOLE){
//		draw_va(d, SBC_COL_BG_UPPER, SBC_TEX_BGF_UPPER, d->console_map.va);
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
//			tile(&d->panel_text_map, x, y, to_char(con_text_getc(c, x, y)), 0, 0);
//			palette(&d->panel_text_map, x, y, con_col_get(c, x, y) & COL_FG_MASK);
		}
	}

//	draw_va(d, SBC_COL_BG_LOWER, SBC_TEX_BGF_LOWER, d->panel_text_map.va);
}

void display_background(struct sbc* p, int screen, int layer){
	// Check if these should even be rendered
	if (!(p->res.visible & VISIBLE_BG0) && layer == 0) return;
	if (!(p->res.visible & VISIBLE_BG1) && layer == 1) return;
	struct display* d = &p->display;

	// TODO:IMPL:MED BGCLIP bounds
	// It's trickier than you'd think due to weird wrapping rules...
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
//			tile(&d->background_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
//			palette(&d->background_map, x, y, (td & 0xf000) >> 12);
		}
	}

	// translate layer
//	sfTransform_translate(&d->rs.transform, -ofs_x, -ofs_y);
/*	draw_va(d,
		screen ? SBC_COL_BG_LOWER : SBC_COL_BG_UPPER,
		screen ? SBC_TEX_BGU_LOWER : SBC_TEX_BGU_UPPER,
		d->background_map.va
	);*/
	// reset transform
//	d->rs.transform = sfTransform_Identity;
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
//			tile(&d->panel_bg_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
//			palette(&d->panel_bg_map, x, y, (td & 0xf000) >> 12);
		}
	}
//	draw_va(d, SBC_COL_BG_LOWER, SBC_TEX_BGD_LOWER, d->panel_bg_map.va);
}

void display_sprite(struct sbc* p, int screen, int prio){
	// Don't display over panel
	if (screen == 1 && (p->panel.type != PNL_OFF || !(p->res.visible & VISIBLE_PANEL))) return;
	if (!(p->res.visible & VISIBLE_SPRITE)) return;

	struct display* d = &p->display;
	// TODO:PERF:NONE re-use this to prevent extra allocations?
//	struct sprite_array sprites = init_sprite_array();

	for (int i = 0; i < MAX_SPRITES; ++i){
		if (p->sprites.info[screen][i].active && p->sprites.info[screen][i].prio == prio){
//			add_sprite(sprites, &p->sprites.info[screen][i]);
		}
	}

//	draw_va(d,
//		screen ? SBC_COL_SPR_LOWER : SBC_COL_SPR_UPPER,
//		screen ? SBC_TEX_SPS_LOWER : SBC_TEX_SPU_UPPER,
//		sprites.va
//	);

//	free_sprite_array(sprites);
}

void display_panel_keys(struct sbc* p){
	if (SCREEN_COUNT < 2) return;
	// Only render keyboard when enabled
	if (p->panel.type == PNL_OFF || p->panel.type == PNL_PNL) return;
	if (!(p->res.visible & VISIBLE_PANEL)) return;

	struct display* d = &p->display;
	// TODO:PERF:NONE re-use this to prevent extra allocations?
//	struct sprite_array sprites = init_sprite_array();

	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
	}

	for (int i = 0; i < KEYBOARD_KEYS; ++i){
		struct sprite_info* key = &p->panel.keys[i];
		// active is a good check to see if the sprite is correctly defined, for now
		if (key->active){
//			add_sprite(sprites, key);
		}
	}

	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(-1));
	}

//	draw_va(d, SBC_COL_SPR_LOWER, SBC_TEX_SPD_LOWER, sprites.va);

//	free_sprite_array(sprites);
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
//		draw_graphic(&d->graphic, p, screen);

//		sfShader_setFloatUniform(d->shader, "colbank", 2+3*screen);
//		sfShader_setBoolUniform(d->shader, "grp_mode", true);
//		sfRenderWindow_drawSprite(d->rw, d->graphic.sprite, &d->rs);
	}
}

