#pragma once

#include <stdbool.h>

#include <SFML/Graphics.h>

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
	/// VertexArray to store the polygons used by SFML for rendering.
	sfVertexArray* va;
};

/// Initializes the tilemap.
///
/// @note Returns tilemap struct itself by value, for it is small.
struct tilemap tilemap_init(int w, int h);

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

