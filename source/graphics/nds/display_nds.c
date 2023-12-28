#include "display_nds.h"

#include "system.h"

#include <nds/arm9/sprite.h> // For SpriteEntry struct
#include <nds/arm9/video.h> // For VRAM setup

void init_display(struct ptc* p){
	(void)p;
}

void free_display(struct display* d){
	(void)d;
}

void display_draw_all(struct ptc* p){
//	struct display* d = &p->display;
//	(void)d;
	
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
	
	for (u16 i = 0; i < 2*COL_BANKS; ++i){
		for (u16 j = 0; j < COL_SIZE/sizeof(u16); ++j){
			destinations[i][j] = p->res.col[i][j];
		}
	}
	
	vramSetBankG(VRAM_G_SPRITE_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	
	// Set screen
	for (int screen = 0; screen <= 1; ++screen){
		// Draw single screen contents
		for (int prio = 3; prio >= 0; --prio){
			display_draw_graphics(p, screen, prio);
			display_draw_background(p, screen, prio);
			display_draw_sprite(p, screen, prio);
			display_draw_text(p, screen, prio);
		}
	}
}

void display_draw_text(struct ptc* p, int screen, int prio){
	if (!prio) return;
	
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
	/*
	if (screen == 0){
		map = p->res.scr[SCR_BANKS * screen + 1];
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			for (int x = 0; x < CONSOLE_WIDTH; ++x){
				u8 pal = (con_col_get(con, x, y) & COL_BG_MASK) >> 4;
				u16 t = to_tiledata(pal ? 15 : 0, pal, 0, 0);
				*map++ = t;
			}
		}
	}*/
}

void display_draw_background(struct ptc* p, int screen, int prio){
	// TODO:IMPL:MED BGCLIP bounds
	bg_scroll* bg = screen ? BG_OFFSET_SUB : BG_OFFSET;
	// BG0 = 2 BG1 = 3
	for (int i = 2; i <= 3; ++i){
		bg[i].x = FP_TO_INT(p->background.ofs[screen][i-2].x);
		bg[i].y = FP_TO_INT(p->background.ofs[screen][i-2].y);
	}
	(void)prio;
}

void display_draw_sprite(struct ptc* p, int screen, int prio){
	// This priority doesn't actually mean anything; we just want this to run
	// once as we can set all sprites in one go due to the hardware supporting
	// priority ordering already. We use prio=3 as it runs first.
	if (prio != 3) return;
	
	SpriteEntry* oam;
	u32* oam_dest;
	struct sprite_info* s;
	
	// TODO:CODE:MED Move this OAM array to display
	u32 oam_buf[sizeof(OAMTable)/sizeof(u32)] = {0};
	oam = (SpriteEntry*)&oam_buf;
	for (int i = 0; i < SPRITE_COUNT; ++i){
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
			
			u8 size = 0;
			u8 shape = 0;
			// TODO:PERF:MED This is terrible, there has to be a better way to calculate this
			// Or maybe just save the result on SPSET sprite creation?
			if (s[i].w == 8){ 
				if (s[i].h == 8){
					size = OBJSIZE_8;
					shape = OBJSHAPE_SQUARE;
				} else if (s[i].h == 16){
					size = OBJSIZE_8;
					shape = OBJSHAPE_TALL;
				} else if (s[i].h == 32){
					size = OBJSIZE_16;
					shape = OBJSHAPE_TALL;
				}
			} else if (s[i].w == 16){
				if (s[i].h == 8){
					size = OBJSIZE_8;
					shape = OBJSHAPE_WIDE;
				} else if (s[i].h == 16){
					size = OBJSIZE_16;
					shape = OBJSHAPE_SQUARE;
				} else if (s[i].h == 32){
					size = OBJSIZE_32;
					shape = OBJSHAPE_TALL;
				}
			} else if (s[i].w == 32){
				if (s[i].h == 8){
					size = OBJSIZE_16;
					shape = OBJSHAPE_WIDE;
				} else if (s[i].h == 16){
					size = OBJSIZE_32;
					shape = OBJSHAPE_WIDE;
				} else if (s[i].h == 32){
					size = OBJSIZE_32;
					shape = OBJSHAPE_SQUARE;
				} else if (s[i].h == 64){
					size = OBJSIZE_64;
					shape = OBJSHAPE_TALL;
				}
			} else if (s[i].w == 64){
				if (s[i].h == 32){
					size = OBJSIZE_64;
					shape = OBJSHAPE_WIDE;
				} else if (s[i].h == 64){
					size = OBJSIZE_64;
					shape = OBJSHAPE_SQUARE;
				}
			}
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
	
	
	// Create OAM
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
	
	for (u32 i = 0; i < sizeof(SpriteEntry)*SPRITE_COUNT/sizeof(u32); i += 1){
		((u32*)oam_dest)[i] = ((u32*)oam)[i];
	}
}

void display_draw_graphics(struct ptc* p, int screen, int prio){
	// TODO:IMPL:LOW Set graphics priority of sprites
	if (prio != 3) return;
//	if (prio != 4) return;
	// Note that VRAM can only be read in 16bit or higher units
	u32* ptr = (u32*)(screen ? VRAM_GRP_CHR + VRAM_LOWER_OFS : VRAM_GRP_CHR);
	
	for (u32 i = 0; i < GRP_SIZE/sizeof(u32); ++i){
		ptr[i] = ((u32*)p->res.grp[p->graphics.info[screen].displaypage])[i];
	}
}
