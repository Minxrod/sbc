#include "display_nds.h"

#include "system.h"

#include <nds/arm9/sprite.h> // For SpriteEntry struct

void init_display(struct ptc* p){
	(void)p;
}

void free_display(struct display* d){
	(void)d;
}

void display_draw_all(struct ptc* p){
	struct display* d = &p->display;
	(void)d;
	
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
		con = p->panel.text;
	}
	
	//TODO:IMPL:MED background tile/color
	for (int y = 0; y < CONSOLE_HEIGHT; ++y){
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			u16 t = to_char(con_text_getc(con, x, y));
			t |= (con_col_get(con, x, y) & COL_FG_MASK) << 12; //TODO:CODE:NONE 12 should be a constant?
			*map = t;
			map++;
		}
	}
}

void display_draw_background(struct ptc* p, int screen, int prio){
	// TODO:IMPL:MED set scroll values?
	(void)p;
	(void)screen;
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
	
	u32 oam_buf[sizeof(OAMTable)/sizeof(u32)] = {0};
	oam = (SpriteEntry*)&oam_buf;
	
	s = p->sprites.info[screen];
	if (screen == 0){
		oam_dest = (u32*)OAM;
	} else {
		oam_dest = (u32*)OAM_SUB;
		if (p->panel.type != PNL_OFF && p->panel.type != PNL_PNL){
			// keyboard
			s = p->panel.keys;
			if (p->panel.key_pressed){
				offset_key(p, p->panel.id_pressed, INT_TO_FP(1));
			}
		}
	}
	
	for (int i = 0; i < 100; ++i){
		oam[i].isRotateScale = false; //TODO:IMPL:MED For scaled sprites later
		oam[i].isHidden = !s[i].active;
		if (s[i].active){
			oam[i].gfxIndex = s[i].chr;
			oam[i].palette = s[i].pal;
			oam[i].priority = s[i].prio;
			oam[i].colorMode = OBJCOLOR_16;
			oam[i].hFlip = s[i].flip_x;
			oam[i].vFlip = s[i].flip_y;
			oam[i].x = FP_TO_INT(s[i].pos.x);
			oam[i].y = FP_TO_INT(s[i].pos.y);
			// debug
			
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
	
//	(void)oam_dest;
	for (u32 i = 0; i < sizeof(SpriteEntry)*128/sizeof(u32); i += 1){
		((u32*)oam_dest)[i] = ((u32*)oam)[i];
	}
}

void display_draw_graphics(struct ptc* p, int screen, int prio){
	(void)p;
	(void)screen;
	(void)prio;
}
