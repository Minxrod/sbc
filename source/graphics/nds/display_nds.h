#pragma once

struct ptc;

struct display {
	/// Location to use to hold temporary sprite data
	void* oam_buf;
};

// c.f. gbatek (product, square modified tables added)
// https://www.problemkaputt.de/gbatek.htm#lcdobjoamattributes
//          Size  Square   Horizontal  Vertical  Square(m) Product
//          0     8x8      16x8        8x16      16*8      128
//          1     16x16    32x8        8x32      16*16     256
//          2     32x32    32x16       16x32     16*32     512
//          3     64x64    64x32       32x64     16*64     1024,2048
static inline int spr_calc_size(int shape, int w, int h){
	// We can convert products to sizes with this
	int size = (shape ? w : 16) * h;
	size >>= 8; // 0 1 2 4|8
	size = (size > 3) ? 3 : size; // 0 1 2 3
	return size;
}

void init_display(struct ptc*);
void free_display(struct display*);

void display_draw_all(struct ptc* p);

void display_console(struct ptc* p);
void display_panel_console(struct ptc* p);
void display_background(struct ptc* p, int screen);
void display_panel_background(struct ptc* p);
void display_sprite(struct ptc* p, int screen);
void display_panel_keys(struct ptc* p);
void display_icon(struct ptc* p);
void display_cursor(struct ptc* p);
void display_graphics(struct ptc* p, int screen);

