#pragma once
///
/// @file
/// @brief Defines the tilemap background subsystem of PTC. 
///

#include "common.h"
#include "resources.h"

#include <assert.h>

struct ptc;

#define BG_LAYERS 2

///
/// Background management struct.
///
/// Contains information about the state of the background layers.
/// This includes the information for both screens.
/// 
struct background {
	int_fast8_t page;
	
	/// BG offset information.
	///
	/// Stores the scroll state of the BG layer, including interpolation data.
	struct bg_offset {
		fixp x;
		fixp y;
		
		fixp step_x;
		fixp step_y;
		
		fixp time;
	} ofs[SCREEN_COUNT][BG_LAYERS]; // index order: ofs[page][layer]
	
	/// BG clipping info.
	/// 
	/// Stores the per-screen visible region of the BG layer.
	struct bg_clip {
		uint_fast8_t x1;
		uint_fast8_t y1;
		uint_fast8_t x2;
		uint_fast8_t y2;
	} clip[SCREEN_COUNT];
	
};

/// Allocates memory for a background object
///
/// @return Newly-alloc'd background object
struct background* alloc_background(void);
/// Initializes background memory to it's initial state
///
/// @param b Background objecet to initialize
void init_background(struct background* b);
void free_background(struct background* b);

/// Steps the background interpolation state forward by one frame
///
/// @param b Pointer to background state
void step_background(struct background* b);

static inline u16 to_tiledata(u16 chr, u8 pal, bool h, bool v){
	assert((chr & 0x3ff) == chr);
	assert((pal & 0xf) == pal);
	
	return (chr & 0x3ff) | ((h & 0x1) << 10) | ((v & 0x1) << 11) | ((pal & 0xf) << 12);	
}

//u16* bg_page(struct ptc* p, u8 page, u8 layer);
// TODO:PERF:LOW Check if these are better as inlines
/// Calculates the index into a SCR resource from tile coordinates.
///
/// @param x X-coordiante in range [0,BG_WIDTH)
/// @param y Y-coordiante in range [0,BG_HEIGHT)
/// @return Index into a SCR array
u16 bg_index(uint_fast8_t x, uint_fast8_t y);
u16 bg_tile(struct ptc* p, uint_fast8_t page, uint_fast8_t l, uint_fast8_t x, uint_fast8_t y);

void cmd_bgpage(struct ptc* p);
void cmd_bgclr(struct ptc* p);
void cmd_bgput(struct ptc* p);
void cmd_bgfill(struct ptc* p);
void cmd_bgofs(struct ptc* p);
void cmd_bgread(struct ptc* p);
void cmd_bgclip(struct ptc* p);
void cmd_bgcopy(struct ptc* p);

void func_bgchk(struct ptc* p);
