#pragma once

// TODO:CODE:HIGH rename this to not be confusing with the other file called common
#include "common.h"

struct sprite_info;

struct sprite_array {
	/// List of all sprite vertices 
	vertex_array vertices;
	quad_array quads;
	/// Current size of resizables
	int size; // in tiles
	/// Current maximum size of resizables (before needing a realloc)
	int capacity; // in tiles
	/// Texture height
	int tex_height;
};

struct sprite_array init_sprite_array(int tex_height);

/// Resets the sprite array's contents without affect it's capacity
/// (This reduces re-allocations when the object is reused)
void reset_sprite_array(struct sprite_array*);

struct display;
void draw_sprite_array(struct display* d, struct sprite_array* s);

void add_tile(struct sprite_array* s, int x, int y, int xstepx, int ystepx, int xstepy, int ystepy, int pal, int chr);

void free_sprite_array(struct sprite_array* s);

void add_sprite(struct sprite_array* sa, struct sprite_info* s);
