#include "tilemap.h"

#include <SFML/Graphics.h>

struct tilemap tilemap_init(int w, int h){
	struct tilemap map = {w, h, sfVertexArray_create()};
	
	sfVertexArray_setPrimitiveType(map.va, sfQuads);
	sfVertexArray_resize(map.va, w*h*4);
	
	for (int y = 0; y < h; ++y){
		for (int x = 0; x < w; ++x){
			sfVertexArray_getVertex(map.va, 4*(x+w*y))->position = (sfVector2f){x*8, y*8};
			sfVertexArray_getVertex(map.va, 4*(x+w*y)+1)->position = (sfVector2f){x*8+8, y*8};
			sfVertexArray_getVertex(map.va, 4*(x+w*y)+2)->position = (sfVector2f){x*8+8, y*8+8};
			sfVertexArray_getVertex(map.va, 4*(x+w*y)+3)->position = (sfVector2f){x*8, y*8+8};
			//vertex color = palette info
			sfVertexArray_getVertex(map.va, 4*(x+w*y))->color = sfBlack;
			sfVertexArray_getVertex(map.va, 4*(x+w*y)+1)->color = sfBlack;
			sfVertexArray_getVertex(map.va, 4*(x+w*y)+2)->color = sfBlack;
			sfVertexArray_getVertex(map.va, 4*(x+w*y)+3)->color = sfBlack;
		}
	}
	
	return map;
}

sfVector2f chr_texCoords(int chr, int xofs, int yofs){
	int x = chr % 32;
	int y = chr / 32;
	return (sfVector2f){8*(x+xofs), 8*(y+yofs)};
}

//sets a single tile, by means of setting the texture coordinates for the tile
void tile(struct tilemap* map, int x, int y, int chr, bool h, bool v){
	sfVertexArray_getVertex(map->va, 4 * (x + map->width * y) + 0)->texCoords = chr_texCoords(chr, h, v);
	sfVertexArray_getVertex(map->va, 4 * (x + map->width * y) + 1)->texCoords = chr_texCoords(chr, !h, v);
	sfVertexArray_getVertex(map->va, 4 * (x + map->width * y) + 2)->texCoords = chr_texCoords(chr, !h, !v);
	sfVertexArray_getVertex(map->va, 4 * (x + map->width * y) + 3)->texCoords = chr_texCoords(chr, h, !v);
}

// sets the color palette of a tile
void palette(struct tilemap* map, int x, int y, int pal){
	sfColor col = {pal,0,0,255};
	sfVertexArray_getVertex(map->va, 4 * (x + map->width * y) + 0)->color = col;
	sfVertexArray_getVertex(map->va, 4 * (x + map->width * y) + 1)->color = col;
	sfVertexArray_getVertex(map->va, 4 * (x + map->width * y) + 2)->color = col;
	sfVertexArray_getVertex(map->va, 4 * (x + map->width * y) + 3)->color = col;
}

