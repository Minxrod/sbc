#include "display.h"

#include <stdlib.h>

#include "system.h"

void init_display(struct ptc* p){
	struct display* d = &p->display;
	struct resources* r = &p->res;
	
	d->console_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT);
	d->console_bg_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT);
	d->panel_text_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT);
	d->panel_bg_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT);
	
	d->graphic = init_graphic(GRP_WIDTH, GRP_HEIGHT);
	
	d->background_map = init_tilemap(BG_WIDTH, BG_HEIGHT);
//	d->foreground_map = init_tilemap(BG_WIDTH, BG_HEIGHT);
	
//	d->sprites = init_sprite_array();
//	d->panel_keys = init_sprite_array();
	
	// SFML stuff
	d->view = sfView_createFromRect((sfFloatRect){0, 0, 256, 192});
	sfView_setViewport(d->view, (sfFloatRect){0, 0, 1, 0.5f});
	
	// actual draw commands
	d->rs = sfRenderStates_default();
	if (!sfShader_isAvailable()){
		iprintf("Error: Shaders are unavailable!\n");
		abort();
	} else {
		if (!(d->shader = sfShader_createFromFile(NULL, NULL, "resources/bgsp.frag"))){
			iprintf("Error: Shader failed to load!\n");
			abort();
		}
	}
	
	// Generate PC textures here
	for (int page = 0; page <= 1; ++page){
		d->chr_tex[0+5*page] = gen_chr_texture(r->chr[0+CHR_BANKS*page], 512); //BGF
		d->chr_tex[1+5*page] = gen_chr_texture(r->chr[4+CHR_BANKS*page], 1024); //BGD
		d->chr_tex[2+5*page] = gen_chr_texture(r->chr[8+CHR_BANKS*page], 1024); //BGU
		d->chr_tex[3+5*page] = gen_chr_texture(r->chr[12+CHR_BANKS*page], 2048); // SPU or SPD
		d->chr_tex[4+5*page] = gen_chr_texture(r->chr[20+CHR_BANKS*page], 512); //SPS
	}
	// Generates all colors at once
	d->col_tex = gen_col_texture(r->col_banks);
}

sfTexture* gen_col_texture(u16* src){
	u8 array[4*256*6]; // no {0} because it will all be generated by the end
	
	for (size_t i = 0; i < 256*6; ++i){
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
//		iprintf("%ld:(%d,%d,%d,%d)",i,array[4*i+0],array[4*i+1],array[4*i+2],array[4*i+3]);
	}
//	iprintf("\n");
	
	sfTexture* tex = sfTexture_create(256, 6);
	if (!tex){
		return NULL;
	}
	sfTexture_updateFromPixels(tex, array, 256, 6, 0, 0);
	return tex;
}

sfTexture* gen_chr_texture(u8* src, size_t size){
	u8 array[4*size*64];
	for (size_t i = 0; i < size; ++i){
		size_t x = i % 32;
		size_t y = i / 32;
		for (int cx = 0; cx < 8; ++cx){
			for (int cy = 0; cy < 8; ++cy){
				u8 c = src[32*i+cx/2+4*cy];
				array[4*(8*x+cx+256*(8*y+cy))] = ((cx & 1) ? (c >> 4) & 0x0f : c & 0x0f);
			}
		}
	}
	
	sfTexture* tex = sfTexture_create(256, size/4);
	if (!tex){
		return NULL;
	}
	sfTexture_updateFromPixels(tex, array, 256, size/4, 0, 0);
	return tex;
}

void free_display(struct display* d){
	free_tilemap(&d->console_map);
	free_tilemap(&d->console_bg_map);
	free_tilemap(&d->panel_bg_map);
	free_tilemap(&d->panel_text_map);
	
	free_graphic(&d->graphic);
	
	free_tilemap(&d->background_map);
	free_tilemap(&d->foreground_map);
	
//	free_sprite_array(d->sprites);
//	free_sprite_array(d->panel_keys);
	
	sfView_destroy(d->view);
	
	// Destroy textures here
	for (int i = 0; i < 10; ++i){
		sfTexture_destroy(d->chr_tex[i]);
	}
	sfTexture_destroy(d->col_tex);
	sfShader_destroy(d->shader);
}

void display_draw_all(struct ptc* p){
	// SFML stuff
	struct display* d = &p->display;
	struct resources* r = &p->res;
//	d->view = sfView_createFromRect((sfFloatRect){0, 0, 256, 192});
//	sfView_setViewport(d->view, (sfFloatRect){0, 0, 1, 0.5f});
//	sfRenderWindow_setView(d->rw, d->view);
	
	// actual draw commands
	d->rs = sfRenderStates_default();
	sfShader* shader = p->display.shader;

	d->rs.shader = shader;
	
	sfShader_setFloatUniform(shader, "colbank", 0);
	
	sfRenderWindow_clear(d->rw, sfBlack);
	
	// TODO:PERF:MED Reuse texture memory instead of destroying and regenerating
	for (int page = 0; page <= 1; ++page){
		if (r->regen_chr[0 + CHR_BANKS * page] || r->regen_chr[1 + CHR_BANKS * page]){
			sfTexture_destroy(d->chr_tex[0+5*page]);
			d->chr_tex[0+5*page] = gen_chr_texture(r->chr[0+CHR_BANKS*page], 512); //BGF
			r->regen_chr[0 + CHR_BANKS * page] = false;
			r->regen_chr[1 + CHR_BANKS * page] = false;
		}
		if (r->regen_chr[4 + CHR_BANKS * page] ||
			r->regen_chr[5 + CHR_BANKS * page] ||
			r->regen_chr[6 + CHR_BANKS * page] ||
			r->regen_chr[7 + CHR_BANKS * page])
		{
			sfTexture_destroy(d->chr_tex[1+5*page]);
			d->chr_tex[1+5*page] = gen_chr_texture(r->chr[4+CHR_BANKS*page], 1024); //BGD
			r->regen_chr[4 + CHR_BANKS * page] = false;
			r->regen_chr[5 + CHR_BANKS * page] = false;
			r->regen_chr[6 + CHR_BANKS * page] = false;
			r->regen_chr[7 + CHR_BANKS * page] = false;
		}
		if (r->regen_chr[8 + CHR_BANKS * page] ||
			r->regen_chr[9 + CHR_BANKS * page] ||
			r->regen_chr[10 + CHR_BANKS * page] ||
			r->regen_chr[11 + CHR_BANKS * page])
		{
			sfTexture_destroy(d->chr_tex[2+5*page]);
			d->chr_tex[2+5*page] = gen_chr_texture(r->chr[8+CHR_BANKS*page], 1024); //BGU
			r->regen_chr[8 + CHR_BANKS * page] = false;
			r->regen_chr[9 + CHR_BANKS * page] = false;
			r->regen_chr[10 + CHR_BANKS * page] = false;
			r->regen_chr[11 + CHR_BANKS * page] = false;
		}
		if (r->regen_chr[12 + CHR_BANKS * page] ||
			r->regen_chr[13 + CHR_BANKS * page] ||
			r->regen_chr[14 + CHR_BANKS * page] ||
			r->regen_chr[15 + CHR_BANKS * page] ||
			r->regen_chr[16 + CHR_BANKS * page] ||
			r->regen_chr[17 + CHR_BANKS * page] ||
			r->regen_chr[18 + CHR_BANKS * page] ||
			r->regen_chr[19 + CHR_BANKS * page])
		{
			sfTexture_destroy(d->chr_tex[3+5*page]);
			d->chr_tex[3+5*page] = gen_chr_texture(r->chr[12+CHR_BANKS*page], 2048); // SPU or SPD
			r->regen_chr[12 + CHR_BANKS * page] = false;
			r->regen_chr[13 + CHR_BANKS * page] = false;
			r->regen_chr[14 + CHR_BANKS * page] = false;
			r->regen_chr[15 + CHR_BANKS * page] = false;
			r->regen_chr[16 + CHR_BANKS * page] = false;
			r->regen_chr[17 + CHR_BANKS * page] = false;
			r->regen_chr[18 + CHR_BANKS * page] = false;
			r->regen_chr[19 + CHR_BANKS * page] = false;
		}
		if (r->regen_chr[20 + CHR_BANKS * page] ||
			r->regen_chr[21 + CHR_BANKS * page])
		{
			sfTexture_destroy(d->chr_tex[4+5*page]);
			d->chr_tex[4+5*page] = gen_chr_texture(r->chr[20+CHR_BANKS*page], 512); //SPS
			r->regen_chr[20 + CHR_BANKS * page] = false;
			r->regen_chr[21 + CHR_BANKS * page] = false;
		}
	}
	
	if (r->regen_col){
		sfTexture_destroy(d->col_tex);
		d->col_tex = gen_col_texture(r->col_banks);
		r->regen_col = false;
	}
	
	sfShader_setTextureUniform(shader, "colors", d->col_tex);
	sfShader_setCurrentTextureUniform(shader, "texture");
	
	// Set screen
	for (int screen = 0; screen <= 1; ++screen){
		// Set part of screen to draw to
		sfView_setViewport(d->view, (sfFloatRect){0, screen * 0.5f, 1, 0.5f});
		sfRenderWindow_setView(d->rw, d->view);
		
/*		sfCircleShape* s = sfCircleShape_create();
		sfCircleShape_setRadius(s, 128);
///		sfCircleShape_setPosition(s, (sfVector2f){0,0});
		
		// set the shape color to green
		sfCircleShape_setFillColor(s, (sfColor){64, 250, 42, 255});
		sfRenderStates rs = sfRenderStates_default();
		sfRenderWindow_drawCircleShape(d->rw, s, &rs);
		sfCircleShape_destroy(s);*/
		
		// Draw single screen contents
		for (int prio = 3; prio >= 0; --prio){
			display_draw_graphics(p, screen, prio);
			display_draw_background(p, screen, prio);
			display_draw_sprite(p, screen, prio);
			display_draw_text(p, screen, prio);
		}
	}
	
	sfRenderWindow_display(d->rw);
}

void display_draw_text(struct ptc* p, int screen, int prio){
	if (prio) return;
	struct display* d = &p->display;
	
	for (int x = 0; x < CONSOLE_WIDTH; ++x){
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			// console 
			if (screen == 0){
				tile(&d->console_map, x, y, to_char(con_text_getc(&p->console, x, y)), 0, 0);
				palette(&d->console_map, x, y, con_col_get(&p->console, x, y) & COL_FG_MASK);
				
				u8 c = (con_col_get(&p->console, x, y) & COL_BG_MASK) >> 4;
				tile(&d->console_bg_map, x, y, c ? 15 : 0, 0, 0);
				palette(&d->console_bg_map, x, y, c);
			} else {
				// panel
				struct console* c = p->panel.keys_text;
				if (p->panel.type == PNL_OFF || p->panel.type == PNL_PNL){
					c = p->panel.text;
				}
				tile(&d->panel_text_map, x, y, to_char(con_text_getc(c, x, y)), 0, 0);
				palette(&d->panel_text_map, x, y, con_col_get(c, x, y) & COL_FG_MASK);
				
				/*if (p->panel.type != PNL_OFF && p->panel.type != PNL_PNL){
					u16 td = bg_tile(p,1,2,x,y);
					tile(&d->panel_bg_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
					palette(&d->panel_bg_map, x, y, 0);
				}*/
			}
		}
	}
	
	if (screen == 0){
		// Background color layer
		d->rs.texture = d->chr_tex[1];
		sfShader_setFloatUniform(d->shader, "colbank", 0);
		sfShader_setBoolUniform(d->shader, "grp_mode", false);
		sfRenderWindow_drawVertexArray(d->rw, d->console_bg_map.va, &d->rs);
		// Text layer
		d->rs.texture = d->chr_tex[0];
		sfShader_setFloatUniform(d->shader, "colbank", 0);
		sfShader_setBoolUniform(d->shader, "grp_mode", false);
		sfRenderWindow_drawVertexArray(d->rw, d->console_map.va, &d->rs);
	} else {
		// Text layer
		d->rs.texture = d->chr_tex[5];
		sfShader_setFloatUniform(d->shader, "colbank", 3);
		sfShader_setBoolUniform(d->shader, "grp_mode", false);
		sfRenderWindow_drawVertexArray(d->rw, d->panel_text_map.va, &d->rs);
	}
}

void display_draw_background(struct ptc* p, int screen, int prio){
	// TODO:IMPL:MED Background scrolling
	struct display* d = &p->display;
	if (prio == 1 || prio == 2){
		for (int x = 0; x < BG_WIDTH; ++x){
			for (int y = 0; y < BG_HEIGHT; ++y){
				// BG0 and BG1
				u16 td = bg_tile(p,screen,prio-1,x,y);
				tile(&d->background_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
				palette(&d->background_map, x, y, (td & 0xf000) >> 12);
			}
		}
		d->rs.texture = d->chr_tex[2+5*screen];
		sfShader_setFloatUniform(d->shader, "colbank", 0+3*screen);
		sfShader_setBoolUniform(d->shader, "grp_mode", false);
		sfRenderWindow_drawVertexArray(d->rw, d->background_map.va, &d->rs);
	}
	
	if (p->panel.type != PNL_OFF && screen == 1 && prio == 1){
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			for (int y = 0; y < CONSOLE_HEIGHT; ++y){
				// Panel BG map
				u16 td = p->res.scr[SCR_BANKS+1][x+CONSOLE_WIDTH*y];
				tile(&d->panel_bg_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
				palette(&d->panel_bg_map, x, y, (td & 0xf000) >> 12);
			}
		}
		d->rs.texture = d->chr_tex[1+5*screen];
		sfShader_setFloatUniform(d->shader, "colbank", 0+3*screen);
		sfShader_setBoolUniform(d->shader, "grp_mode", false);
		sfRenderWindow_drawVertexArray(d->rw, d->panel_bg_map.va, &d->rs);
	}
}

void display_draw_sprite(struct ptc* p, int screen, int prio){
	struct display* d = &p->display;
	// TODO:PERF:LOW Re-use sprite array to prevent frequent allocations?
	struct sprite_array sprites = init_sprite_array();
	
	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
	}
	if (screen == 0 || p->panel.type == PNL_OFF){
		for (int i = 0; i < MAX_SPRITES; ++i){
			if (p->sprites.info[screen][i].active && p->sprites.info[screen][i].prio == prio){
				add_sprite(sprites, &p->sprites.info[screen][i]);
			}
		}
	} else {
		if (p->panel.type != PNL_OFF && p->panel.type != PNL_PNL){
			for (int i = 0; i < KEYBOARD_KEYS; ++i){
				struct sprite_info* key = &p->panel.keys[i];
				// active is a good check to see if the sprite is correctly defined, for now
				if (key->active){
					add_sprite(sprites, key);
				}
			}
		}
	}
	if (screen == 1){
		// Icons are drawn on any panel setting
		for (int i = 0; i < ICON_KEYS; ++i){
			struct sprite_info* key = &p->panel.keys[ICON_PAGE_START+i];
			// active is a good check to see if the sprite is correctly defined, for now
			if (key->active){
				add_sprite(sprites, key);
			}
		}
	}
	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, -INT_TO_FP(1));
	}
	
	d->rs.texture = d->chr_tex[3+5*screen];
	sfShader_setFloatUniform(d->shader, "colbank", 1+3*screen);
	sfShader_setBoolUniform(d->shader, "grp_mode", false);
	sfRenderWindow_drawVertexArray(d->rw, sprites.va, &d->rs);
	
	free_sprite_array(sprites);
}

void display_draw_graphics(struct ptc* p, int screen, int prio){
	struct display* d = &p->display;
	if (screen == 0 || (p->panel.type == PNL_OFF || p->panel.type == PNL_PNL)){
		if (prio == p->graphics.info[screen].prio){
			draw_graphic(&d->graphic, p, screen);
			
			sfShader_setFloatUniform(d->shader, "colbank", 2+3*screen);
			sfShader_setBoolUniform(d->shader, "grp_mode", true);
			sfRenderWindow_drawSprite(d->rw, d->graphic.sprite, &d->rs);
		}
	}
}

