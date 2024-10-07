#pragma once

#include <stdbool.h>

#include <GLES2/gl2.h>

// TODO:CODE:HIGH rename this to not be confusing with the other file called common
#include "common.h"

// = calloc_log("console history", 256, sizeof(**history));

/// Drawable tilemap class.
///
/// Used for a grid of renderable tiles, such as a console or BG layer.
///
/// @note Only intended to be drawn with the palette shader.
struct tilemap {
	/// Width of this tilemap, in tiles
	int width;
	/// Height of this tilemap, in tiles
	int height;
	/// Vertex array data to store the polygons used for rendering.
	/// The format is x, y, palette, bank (all floats)
	vertex_array vertices;
	/// Size of vertex array (bytes)
	int vertices_size;
	/// Indices for rendering.
	quad_array quads;
	/// Size of quad array (bytes)
	int quads_size;
	/// Height of texture in pixels (needed for normalizing texture coordinates)
	int texture_height;
};

/// Initializes the tilemap.
///
/// @note Returns tilemap struct itself by value, for it is small.
struct tilemap init_tilemap(int w, int h, int texture_height);
void free_tilemap(struct tilemap* map);

struct display;
void draw_tilemap(struct display* d, struct tilemap* map);

/// Sets a tile with the given information.
///
/// @param tilemap Tilemap object to modify
/// @param x Tile x location
/// @param y Tile y location
/// @param chr Character code
/// @param h true=Horizontally flip texture
/// @param v true=Vertically flip texture
void tile(struct tilemap*, int x, int y, int chr, bool h, bool v);
/// Sets the palette of the tile at (x,y).
///
/// @param tilemap Tilemap object to modify
/// @param x Tile x location
/// @param y Tile y location
/// @param pal Palette number
void palette(struct tilemap*, int x, int y, int pal);
