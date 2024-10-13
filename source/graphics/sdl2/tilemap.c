#include "tilemap.h"

#include "resources.h"
#include "display.h"


struct tilemap init_tilemap(int w, int h, int tex_height){
	// 4 vertices per tile
	const int vertex_size = sizeof(vertex) * VERTICES_PER_TILE * w * h;
	const int quads_size = sizeof(quad) * w * h;
	struct tilemap map = {
		w, h,
		.vertices = malloc(vertex_size), .vertices_size = vertex_size,
		.quads = malloc(quads_size), .quads_size = quads_size,
		.texture_height = tex_height,
	};
	
	for (int y = 0; y < h; ++y){
		for (int x = 0; x < w; ++x){
			const int vert_i = VERTICES_PER_TILE * (x + w * y);
			vertex coords = {norm_x(CHR_WIDTH * x), norm_y(CHR_HEIGHT * y), 0, 0, 0, 0};
			memcpy(&map.vertices[vert_i + 0], &coords, sizeof(coords));
			coords[VERTEX_X] += CHR_WIDTH * FLOAT_PIXEL_WIDTH;
			memcpy(&map.vertices[vert_i + 1], &coords, sizeof(coords));
			coords[VERTEX_Y] -= CHR_HEIGHT * FLOAT_PIXEL_HEIGHT;
			memcpy(&map.vertices[vert_i + 2], &coords, sizeof(coords));
			coords[VERTEX_X] -= CHR_WIDTH * FLOAT_PIXEL_WIDTH; // back to original X
			memcpy(&map.vertices[vert_i + 3], &coords, sizeof(coords));
			const int index_i = (x + w * y);
			map.quads[index_i][0] = vert_i + 0;
			map.quads[index_i][1] = vert_i + 1;
			map.quads[index_i][2] = vert_i + 2;
			map.quads[index_i][3] = vert_i + 0;
			map.quads[index_i][4] = vert_i + 2;
			map.quads[index_i][5] = vert_i + 3;
		}
	}
	
	return map;
}

void draw_tilemap(struct display* d, struct tilemap* map){
	// Create buffers (TODO:CODE:LOW generate with the map itself...?)
	GLuint vertices;
	GLuint indices;
	glGenBuffers(1, &vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferData(GL_ARRAY_BUFFER, map->vertices_size, map->vertices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, map->quads_size, map->quads, GL_DYNAMIC_DRAW);

	// Rendering
	// Setup vertex attributes
	glEnableVertexAttribArray(d->gl_pos);
	glEnableVertexAttribArray(d->gl_col);
	glEnableVertexAttribArray(d->gl_tex);

	//Set vertex data
//	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexAttribPointer(d->gl_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_X * sizeof(GLfloat)));
	glVertexAttribPointer(d->gl_col, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_PALETTE * sizeof(GLfloat)));
	glVertexAttribPointer(d->gl_tex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_U * sizeof(GLfloat)));

	//Set index data and render
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glDrawElements(GL_TRIANGLES, INDICES_PER_TILE * map->width * map->height, GL_UNSIGNED_INT, NULL);

	// Cleanup (TODO:CODE:NONE why is this needed?)
	glDisableVertexAttribArray(d->gl_pos);
	glDisableVertexAttribArray(d->gl_col);
	glDisableVertexAttribArray(d->gl_tex);

	glDeleteBuffers(1, &vertices);
	glDeleteBuffers(1, &indices);
}

void free_tilemap(struct tilemap* map){
	free(map->vertices);
	free(map->quads);
}

//sets a single tile, by means of setting the texture coordinates for the tile
void tile(struct tilemap* map, int x, int y, int chr, bool h, bool v){
	const int vert_i = VERTICES_PER_TILE * (x + map->width * y);
	// TODO:CODE:MED bank texture size constants or something?
	map->vertices[vert_i + 0][VERTEX_U] = CHR_WIDTH * ((chr % 32) + h) / (float)SCREEN_WIDTH;
	map->vertices[vert_i + 0][VERTEX_V] = CHR_HEIGHT * ((chr / 32) + v) / (float)map->texture_height;
	map->vertices[vert_i + 1][VERTEX_U] = CHR_WIDTH * ((chr % 32) + !h) / (float)SCREEN_WIDTH;
	map->vertices[vert_i + 1][VERTEX_V] = CHR_HEIGHT * ((chr / 32) + v) / (float)map->texture_height;
	map->vertices[vert_i + 2][VERTEX_U] = CHR_WIDTH * ((chr % 32) + !h) / (float)SCREEN_WIDTH;
	map->vertices[vert_i + 2][VERTEX_V] = CHR_HEIGHT * ((chr / 32) + !v) / (float)map->texture_height;
	map->vertices[vert_i + 3][VERTEX_U] = CHR_WIDTH * ((chr % 32) + h) / (float)SCREEN_WIDTH;
	map->vertices[vert_i + 3][VERTEX_V] = CHR_HEIGHT * ((chr / 32) + !v) / (float)map->texture_height;
}

// sets the color palette of a tile
void palette(struct tilemap* map, int x, int y, int pal){
	const int vert_i = VERTICES_PER_TILE * (x + map->width * y);
	map->vertices[vert_i + 0][VERTEX_PALETTE] = pal;
	map->vertices[vert_i + 1][VERTEX_PALETTE] = pal;
	map->vertices[vert_i + 2][VERTEX_PALETTE] = pal;
	map->vertices[vert_i + 3][VERTEX_PALETTE] = pal;
}
