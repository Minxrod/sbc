#include "display_nds.h"

#include "system.h"

#include <nds/arm9/sprite.h> // For SpriteEntry struct
#include <nds/arm9/video.h> // For VRAM setup

void init_display(struct ptc* p){
	p->display.oam_buf = calloc(sizeof(OAMTable), 1);
}

void free_display(struct display* d){
	free(d->oam_buf);
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
	
//	start_time(NULL);
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
//	check_time(&p->time, 13); //~91
	
	// Set screen
	for (int screen = 0; screen <= 1; ++screen){
#ifndef NDEBUG
			if (screen) return; // Allows lower screen to be used for debugging
#endif
//		for (int prio = 3; prio >= 0; --prio){
//		start_time(NULL);
		display_draw_graphics(p, screen, 0);
//		check_time(&p->time, 14); // 1368
//		start_time(NULL);
		display_draw_background(p, screen, 0);
//		check_time(&p->time, 15);
//		start_time(NULL);
		display_draw_sprite(p, screen, 0);
//		check_time(&p->time, 16); //168
//		start_time(NULL);
		display_draw_text(p, screen, 0);
//		check_time(&p->time, 17); //529
	}
	
}

void display_draw_text(struct ptc* p, int screen, int prio){
	if (prio) return;
	
	u16* map;
	struct console* con;
	map = p->res.scr[SCR_BANKS * screen];
	if (screen == 0){ // upper screen
		con = &p->console;
	} else { // lower screen
		if (p->panel.type >= PNL_KYA){
			con = p->panel.keys_text;
		} else {
			con = p->panel.text;
		}
	}
	
	for (int y = 0; y < CONSOLE_HEIGHT; ++y){
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			u16 t = to_tiledata(to_char(con_text_getc(con, x, y)), con_col_get(con, x, y) & COL_FG_MASK, 0, 0);
			*map = t;
			map++;
		}
	}
	
	if (screen == 0){
		map = p->res.scr[SCR_BANKS * screen + 1];
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			for (int x = 0; x < CONSOLE_WIDTH; ++x){
				u8 pal = (con_col_get(con, x, y) & COL_BG_MASK) >> 4;
				u16 t = to_tiledata(pal ? 15 : 0, pal, 0, 0);
				*map++ = t;
			}
		}
	}
}

void display_draw_background(struct ptc* p, int screen, int prio){
	if (prio) return;
	// TODO:IMPL:MED BGCLIP bounds
	bg_scroll* bg = screen ? BG_OFFSET_SUB : BG_OFFSET;
	// BG0 = 2 BG1 = 3
	for (int i = 2; i <= 3; ++i){
		bg[i].x = FP_TO_INT(p->background.ofs[screen][i-2].x);
		bg[i].y = FP_TO_INT(p->background.ofs[screen][i-2].y);
	}
}

void display_draw_sprite(struct ptc* p, int screen, int prio){
	// This priority doesn't actually mean anything; we just want this to run
	// once as we can set all sprites in one go due to the hardware supporting
	// priority ordering already.
	if (prio) return;
	
	SpriteEntry* oam;
	u32* oam_dest;
	struct sprite_info* s;
	
	oam = (SpriteEntry*)p->display.oam_buf;
	for (int i = 0; i < SPRITE_COUNT; ++i){
		oam[i].isRotateScale = false;
		oam[i].isHidden = true;
	}
	
	s = p->sprites.info[screen];
	if (screen == 0){
		oam_dest = (u32*)OAM;
	} else {
		oam_dest = (u32*)OAM_SUB;
		if (p->panel.type != PNL_OFF && p->panel.type != PNL_PNL){
			// keyboard
			s = p->panel.keys;
//			if (p->panel.key_pressed){
//				offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
//			}
		}
		if (p->panel.key_pressed){
			offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
		}
	}
	
	for (int i = 0; i < MAX_SPRITES; ++i){
		if (s[i].active){
			oam[i].isHidden = false;
			oam[i].gfxIndex = s[i].chr;
			oam[i].palette = s[i].pal;
			oam[i].priority = s[i].prio;
			oam[i].colorMode = OBJCOLOR_16;
			oam[i].hFlip = s[i].flip_x;
			oam[i].vFlip = s[i].flip_y;
			oam[i].x = FP_TO_INT(s[i].pos.x);
			oam[i].y = FP_TO_INT(s[i].pos.y);
			
			uint_fast8_t shape = (s[i].w > s[i].h) + 2 * (s[i].w < s[i].h);
			uint_fast16_t size = spr_calc_size(shape, s[i].w, s[i].h);
			oam[i].size = size;
			oam[i].shape = shape;
		}
	}
	
	if (screen == 1){
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
			offset_key(p, p->panel.id_pressed, -INT_TO_FP(1));
		}
	}
	
	// Create OAM entries for GRP
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
	
	// TODO:IMPL:LOW Text cursor sprite
	
	dmaCopyWords(3, oam, oam_dest, sizeof(OAMTable));
}

void display_draw_graphics(struct ptc* p, int screen, int prio){
	// TODO:IMPL:LOW Set graphics priority of sprites
	if (prio) return;
//	if (prio != 4) return;
	// Note that VRAM can only be read in 16bit or higher units (but libnds memcpy is fine if aligned)
	u8* src = p->res.grp[p->graphics.info[screen].displaypage];
	u8* dest = (u8*)(screen ? VRAM_GRP_CHR + VRAM_LOWER_OFS : VRAM_GRP_CHR);
	
	DC_FlushRange(src, GRP_SIZE); // Flush cache https://libnds.devkitpro.org/dma_8h.html
	dmaCopyWords(3, src, dest, GRP_SIZE); // DMA W 1368
//	dmaCopy(src, dest, GRP_SIZE); // DMA HW 1730
//	memcpy(dest, src, GRP_SIZE); // memcpy 2568
}
