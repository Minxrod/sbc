#include "display_nds.h"

#include "system.h"

#include <nds/arm9/sprite.h> // For SpriteEntry struct
#include <nds/arm9/video.h> // For VRAM setup

void init_display(struct ptc* p){
	p->display.oam_buf = calloc_log("init_display", sizeof(OAMTable), 1);
}

void free_display(struct display* d){
	free_log("free_display", d->oam_buf);
}

void display_draw_all(struct ptc* p){
//	struct display* d = &p->display;
//	(void)d;
#ifdef SBC_PROFILE
	int x = p->console.x;
	int y = p->console.y;
	for (int i = 0; i < PROFILER_TIMERS; ++i){
		con_putn_at(&p->console, 23, i, i << FIXPOINT);
		con_putn_at(&p->console, 26, i, timerTicks2usec(p->time.profile[i]) << FIXPOINT);
		con_put(&p->console, to_wide(' '));
	}
	p->console.x = x;
	p->console.y = y;
#endif
	
	start_time(NULL);
	// Copy color palettes
	u16* destinations[6] = {
		(u16*)BG_PALETTE,
		(u16*)SPRITE_PALETTE,
		(u16*)VRAM_G,
		(u16*)BG_PALETTE_SUB,
		(u16*)SPRITE_PALETTE_SUB,
		(u16*)VRAM_I,
	};
	
	vramSetBankG(VRAM_G_LCD);
	vramSetBankI(VRAM_I_LCD);
	
	for (int i = 0; i < 2*COL_BANKS; ++i){
		DC_FlushRange(p->res.col[i], COL_SIZE);
		dmaCopyWords(3, p->res.col[i], destinations[i], COL_SIZE);
	}
	
	vramSetBankG(VRAM_G_SPRITE_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	check_time(&p->time, 13); //~91
	
	// Set screen
	
	// =============
	// Upper screen
	// =============
	display_sprite(p, 0);
	display_graphics(p, 0);
	display_background(p, 0);
	display_console(p);
	display_cursor(p);
	
	// update sprites and GRP
	DC_FlushRange(p->display.oam_buf, sizeof(OAMTable)); // Flush cache
	dmaCopyWords(3, p->display.oam_buf, (u32*)OAM, sizeof(OAMTable));
	
	// =============
	// Lower screen
	// =============
	#ifndef NDEBUG
		return; // Allows lower screen to be used for debugging
	#endif
	display_sprite(p, 1);
	display_graphics(p, 1);
	display_background(p, 1);
	display_panel_background(p);
	display_panel_keys(p);
	display_panel_console(p);
	display_icon(p);
	
	// update sprites and GRP
	DC_FlushRange(p->display.oam_buf, sizeof(OAMTable)); // Flush cache
	dmaCopyWords(3, p->display.oam_buf, (u32*)OAM_SUB, sizeof(OAMTable));

}

void display_console(struct ptc* p){
	struct console* con = &p->console;
	u16* fg = p->res.scr[0];
	u16* bg = p->res.scr[1];
	
	// Console background
	for (int y = 0; y < CONSOLE_HEIGHT; ++y){
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			u8 bg_pal = (con_col_get(con, x, y) & COL_BG_MASK) >> 4;
			u16 bg_tile = to_tiledata(bg_pal ? 15 : 0, bg_pal, 0, 0);
			
			*bg++ = bg_tile;
		}
	}
	
	// Console foreground
	if (!(p->res.visible & VISIBLE_CONSOLE)){
		videoBgDisable(0);
		return; // don't bother copying data if disabled
	} else {
		videoBgEnable(0);
	}
	
	for (int y = 0; y < CONSOLE_HEIGHT; ++y){
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			u16 fg_tile = to_tiledata(to_char(con_text_getc(con, x, y)), con_col_get(con, x, y) & COL_FG_MASK, 0, 0);
			
			*fg++ = fg_tile;
		}
	}
	
}

void display_panel_console(struct ptc* p){
	struct console* con;
	if (p->panel.type >= PNL_KYA){
		con = p->panel.keys_text;
	} else {
		con = p->panel.text;
	}
	
	u16* fg = p->res.scr[SCR_BANKS];
	for (int y = 0; y < CONSOLE_HEIGHT; ++y){
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			u16 fg_tile = to_tiledata(to_char(con_text_getc(con, x, y)), con_col_get(con, x, y) & COL_FG_MASK, 0, 0);
			
			*fg++ = fg_tile;
		}
	}
}

void display_background(struct ptc* p, int screen){
	bg_scroll* bg = screen ? BG_OFFSET_SUB : BG_OFFSET;
	// BG0 = 2 BG1 = 3
	struct bg_clip clip = p->background.clip[screen];
	if (screen == 0){
		// TODO:PERF:NONE write directly to memory here instead of using functions?
		if (p->res.visible & VISIBLE_BG0){
			videoBgEnable(2);
		} else {
			videoBgDisable(2);
		}
		if (p->res.visible & VISIBLE_BG1){
			videoBgEnable(3);
		} else {
			videoBgDisable(3);
		}
		windowSetBounds(WINDOW_0, clip.x1, clip.y1, clip.x2, clip.y2);
	} else { // screen 1
		if (p->res.visible & VISIBLE_BG0){
			videoBgEnableSub(2);
		} else {
			videoBgDisableSub(2);
		}
		if (p->res.visible & VISIBLE_BG1){
			videoBgEnableSub(3);
		} else {
			videoBgDisableSub(3);
		}
		windowSetBoundsSub(WINDOW_0, clip.x1, clip.y1, clip.x2, clip.y2);
	}
	
	for (int i = 0; i <= 1; ++i){
		bg[2+i].x = FP_TO_INT(p->background.ofs[screen][i].x);
		bg[2+i].y = FP_TO_INT(p->background.ofs[screen][i].y);
	}
}

void display_panel_background(struct ptc* p){
	if (p->res.visible & VISIBLE_PANEL){
		videoBgEnableSub(1);
	} else {
		videoBgDisableSub(1);
	}
	// no draws, because BG layers are stored in VRAM already
	// and the panel background layer never scrolls or anything
}

void display_sprite(struct ptc* p, int screen){
	// display_sprites clears OAM buffer
	SpriteEntry* oam = (SpriteEntry*)p->display.oam_buf;
	for (int i = 0; i < SPRITE_COUNT; ++i){
		oam[i].isRotateScale = false;
		oam[i].isHidden = true;
//		oam[i].isMosaic = true;
	}
	// render in display_panel_keys instead
	if (screen == 1 && p->panel.type >= PNL_KYA) return;
	// VISIBLE has disabled sprites
	if (screen == 1 && !(p->res.visible & VISIBLE_PANEL)) return;
	if (!(p->res.visible & VISIBLE_SPRITE)) return;
	
	SpriteRotation* oam_rot = (SpriteRotation*)p->display.oam_buf;
	struct sprite_info* s;
	
	s = p->sprites.info[screen];
	
	// TODO:PERF:NONE Check if splitting into two loops (one for scaled, one for not-scaled is faster?
	for (int i = 0; i < MAX_SPRITES; ++i){
		if (s[i].active){
			oam[i].isHidden = false;
			oam[i].gfxIndex = get_sprite_chr(&s[i]) + 512 * screen;
			oam[i].palette = s[i].pal;
			oam[i].priority = s[i].prio;
			oam[i].colorMode = OBJCOLOR_16;
			oam[i].hFlip = s[i].flip_x;
			oam[i].vFlip = s[i].flip_y;
			fixp sc = s[i].scale.s; // 0.0 to 2.0
			int x = FP_TO_INT(s[i].pos.x - s[i].home_x);
			int y = FP_TO_INT(s[i].pos.y - s[i].home_y);
			if (x < -128 || x >= 256 || y <= -64 || y >= 192){
				oam[i].isHidden = true;
				continue;
			}
			oam[i].x = x;
			oam[i].y = y;
			
			uint_fast8_t shape = (s[i].w > s[i].h) + 2 * (s[i].w < s[i].h);
			uint_fast16_t size = spr_calc_size(shape, s[i].w, s[i].h);
			oam[i].size = size;
			oam[i].shape = shape;
			
			if (i < 32){ // rotation groups
				oam[i].x -= s[i].w / 2;
				oam[i].y -= s[i].h / 2;
				oam[i].isRotateScale = true;
				oam[i].isSizeDouble = true;
				oam[i].rotationIndex = i;
				int sh1 = 8;
				// TODO:IMPL:MED Determine offsets for correct rotation
				oam_rot[i].hdx = ((int)cosLerp(degreesToAngle(FP_TO_INT(s[i].angle.a))) << sh1) / sc;
				oam_rot[i].vdx = ((int)sinLerp(degreesToAngle(FP_TO_INT(s[i].angle.a))) << sh1) / sc;
				oam_rot[i].hdy = ((int)-sinLerp(degreesToAngle(FP_TO_INT(s[i].angle.a))) << sh1) / sc;
				oam_rot[i].vdy = ((int)cosLerp(degreesToAngle(FP_TO_INT(s[i].angle.a))) << sh1) / sc;
			}
		}
	}
}

void display_cursor(struct ptc* p){
	if (!p->console.cursor_visible) return;
	SpriteEntry* oam = (SpriteEntry*)p->display.oam_buf;
	
	oam[104].isHidden = false;
	oam[104].gfxIndex = 512+108;
//		oam[104].priority = 0; // TODO:TEST:NONE check correct priority?
	oam[104].colorMode = OBJCOLOR_16;
	oam[104].x = p->console.x * 8;
	oam[104].y = p->console.y * 8;
	oam[104].size = OBJSIZE_8;
	oam[104].shape = OBJSHAPE_SQUARE;
}

void display_panel_keys(struct ptc* p){
	if (p->panel.type == PNL_OFF || p->panel.type == PNL_PNL) return;
	if (!(p->res.visible & VISIBLE_PANEL)) return;
	
	SpriteEntry* oam = (SpriteEntry*)p->display.oam_buf;
	struct sprite_info* s = p->panel.keys;
	
	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
	}
	for (int i = 0; i < KEYBOARD_KEYS; ++i){
		if (s[i].active){
			oam[i].isHidden = false;
			oam[i].gfxIndex = s[i].chr;
			oam[i].palette = s[i].pal;
			oam[i].priority = s[i].prio;
			oam[i].colorMode = OBJCOLOR_16;
			oam[i].hFlip = s[i].flip_x;
			oam[i].vFlip = s[i].flip_y;
			oam[i].x = FP_TO_INT(s[i].pos.x - s[i].home_x);
			oam[i].y = FP_TO_INT(s[i].pos.y - s[i].home_y);
			
			uint_fast8_t shape = (s[i].w > s[i].h) + 2 * (s[i].w < s[i].h);
			uint_fast16_t size = spr_calc_size(shape, s[i].w, s[i].h);
			oam[i].size = size;
			oam[i].shape = shape;
		}
	}
	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(-1));
	}
}

void display_icon(struct ptc* p){
	SpriteEntry* oam = (SpriteEntry*)p->display.oam_buf;
	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
	}
	for (int i = 0; i < ICON_KEYS; ++i){
		struct sprite_info* icon = &p->panel.keys[ICON_PAGE_START+i];
		if (icon->active){
			oam[100+i].isHidden = false;
			oam[100+i].gfxIndex = icon->chr;
			oam[100+i].priority = icon->prio;
			oam[100+i].colorMode = OBJCOLOR_16;
			oam[100+i].x = FP_TO_INT(icon->pos.x);
			oam[100+i].y = FP_TO_INT(icon->pos.y);
			oam[100+i].size = OBJSIZE_32;
			oam[100+i].shape = OBJSHAPE_SQUARE;
		}
	}
	if (p->panel.key_pressed){
		offset_key(p, p->panel.id_pressed, INT_TO_FP(-1));
	}
}

void display_graphics(struct ptc* p, int screen){
	if (screen == 1 && (p->panel.type == PNL_OFF || p->panel.type == PNL_PNL)) return;
	if (screen == 1 && !(p->res.visible & VISIBLE_PANEL)) return;
	if (!(p->res.visible & VISIBLE_GRAPHICS)) return;
	
	SpriteEntry* oam = (SpriteEntry*)p->display.oam_buf;
	
	// Fill OAM entries for GRP
	for (int x = 0; x < SCREEN_WIDTH; x += 64){
		for (int y = 0; y < SCREEN_HEIGHT; y += 64){
			int i = x / 64 + y / 16;
			oam[111+i].isHidden = false;
			oam[111+i].gfxIndex = 640 + x / 2 + y * 2;
			oam[111+i].palette = 0;
			oam[111+i].priority = p->graphics.info[screen].prio;
			oam[111+i].colorMode = OBJCOLOR_256;
			oam[111+i].x = x;
			oam[111+i].y = y;
			oam[111+i].size = OBJSIZE_64;
			oam[111+i].shape = OBJSHAPE_SQUARE;
		}
	}
	// Note that VRAM can only be read in 16bit or higher units (but libnds memcpy is fine if aligned)
	u8* src = p->res.grp[p->graphics.info[screen].displaypage];
	u8* dest = (u8*)(screen ? VRAM_GRP_CHR + VRAM_LOWER_OFS : VRAM_GRP_CHR);
	
	DC_FlushRange(src, GRP_SIZE); // Flush cache https://libnds.devkitpro.org/dma_8h.html
	dmaCopyWords(3, src, dest, GRP_SIZE);
}
