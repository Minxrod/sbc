#include "sprite.h"

#include <math.h>
#include <stdlib.h>

#include "sprites.h"
#include "common.h"
#include "display.h"

/// Capacity of resizable array in tiles
#define DEFAULT_TILE_CAPACITY 64

struct sprite_array init_sprite_array(int tex_height){
	struct sprite_array s = {
		.vertices = malloc(sizeof(vertex) * VERTICES_PER_TILE * DEFAULT_TILE_CAPACITY),
		.quads = malloc(sizeof(quad) * INDICES_PER_TILE * DEFAULT_TILE_CAPACITY),
		.size = 0,
		.capacity = DEFAULT_TILE_CAPACITY,
		.tex_height = tex_height,
	};
	// TODO:CODE:LOW handle alloc failures better
	assert(s.vertices);
	assert(s.quads);
	return s;
}

void reset_sprite_array(struct sprite_array* s){
	s->size = 0;
}

void free_sprite_array(struct sprite_array* s){
	free(s->vertices);
	free(s->quads);
}

///
/// @param xstepx X-coord step for tile X step
/// @param ystepx Y-coord step for tile X step
void add_tile(struct sprite_array* s, int x, int y, int xstepx, int ystepx, int xstepy, int ystepy, int pal, int chr){
	assert(s->size <= s->capacity);
	if (s->size == s->capacity){
		s->capacity *= 2;
		// Increase capacity
		void* vertices = realloc(s->vertices, sizeof(vertex) * VERTICES_PER_TILE * s->capacity);
		void* quads = realloc(s->quads, sizeof(quad) * INDICES_PER_TILE * s->capacity);
		assert(vertices);
		assert(quads);
		s->vertices = vertices;
		s->quads = quads;
	}

	int vertex_i = s->size * VERTICES_PER_TILE;
	int quad_i = s->size;
	// TODO:IMPL:HIGH coordiantes need to be normalized (copy tilemap)
	s->vertices[vertex_i + 0][VERTEX_X] = norm_x(FP_TO_INT(x));
	s->vertices[vertex_i + 0][VERTEX_Y] = norm_y(FP_TO_INT(y));
	s->vertices[vertex_i + 0][VERTEX_PALETTE] = pal;
	s->vertices[vertex_i + 0][VERTEX_U] = CHR_WIDTH * (chr % 32) / (float)SCREEN_WIDTH;
	s->vertices[vertex_i + 0][VERTEX_V] = CHR_HEIGHT * (chr / 32) / (float)s->tex_height;

	s->vertices[vertex_i + 1][VERTEX_X] = norm_x(FP_TO_INT(x + xstepx));
	s->vertices[vertex_i + 1][VERTEX_Y] = norm_y(FP_TO_INT(y + ystepx));
	s->vertices[vertex_i + 1][VERTEX_PALETTE] = pal;
	s->vertices[vertex_i + 1][VERTEX_U] = CHR_WIDTH * (chr % 32 + 1) / (float)SCREEN_WIDTH;
	s->vertices[vertex_i + 1][VERTEX_V] = CHR_HEIGHT * (chr / 32) / (float)s->tex_height;

	s->vertices[vertex_i + 2][VERTEX_X] = norm_x(FP_TO_INT(x + xstepx + xstepy));
	s->vertices[vertex_i + 2][VERTEX_Y] = norm_y(FP_TO_INT(y + ystepx + ystepy));
	s->vertices[vertex_i + 2][VERTEX_PALETTE] = pal;
	s->vertices[vertex_i + 2][VERTEX_U] = CHR_WIDTH * (chr % 32 + 1) / (float)SCREEN_WIDTH;
	s->vertices[vertex_i + 2][VERTEX_V] = CHR_HEIGHT * (chr / 32 + 1) / (float)s->tex_height;

	s->vertices[vertex_i + 3][VERTEX_X] = norm_x(FP_TO_INT(x + xstepy));
	s->vertices[vertex_i + 3][VERTEX_Y] = norm_y(FP_TO_INT(y + ystepy));
	s->vertices[vertex_i + 3][VERTEX_PALETTE] = pal;
	s->vertices[vertex_i + 3][VERTEX_U] = CHR_WIDTH * (chr % 32) / (float)SCREEN_WIDTH;
	s->vertices[vertex_i + 3][VERTEX_V] = CHR_HEIGHT * (chr / 32 + 1) / (float)s->tex_height;

	s->quads[quad_i][0] = vertex_i + 0;
	s->quads[quad_i][1] = vertex_i + 1;
	s->quads[quad_i][2] = vertex_i + 2;
	s->quads[quad_i][3] = vertex_i + 0;
	s->quads[quad_i][4] = vertex_i + 2;
	s->quads[quad_i][5] = vertex_i + 3;

	++s->size;
}


void rotate_xy(fixp* x, fixp* y, const fixp a){
	fixp ox = *x;
	double angle = a/4096.0;
	*x = *x * cos(angle) + *y * sin(angle);
	*y = ox *-sin(angle) + *y * cos(angle);
}

#define SWAP(a,b) { fixp temp = a; a = b; b = temp; }

void add_sprite(struct sprite_array* sa, struct sprite_info* s){
	// Adapted from SpriteArray.cpp in PTC-EmkII
	//Vertex order:
	//1--2
	//|  |
	//|  |
	//4--3
	// common values
	fixp scale = s->scale.s;
	int home_x = s->home_x;
	int home_y = s->home_y;
	
	// sprite width & height after scaling
	fixp w = s->w * scale;
	fixp h = s->h * scale;
	
	//determine scaled sprite bounds
	fixp x1 = -scale * home_x;
	fixp y1 = -scale * home_y;
	fixp x2 = -scale * home_x + w;
	fixp y2 = -scale * home_y;
	fixp x3 = -scale * home_x + w;
	fixp y3 = -scale * home_y + h;
	fixp x4 = -scale * home_x;
	fixp y4 = -scale * home_y + h;

	if (s->flip_x){
		SWAP(x1,x2);
		SWAP(y1,y2);
		SWAP(x4,x3);
		SWAP(y4,y3);
	}
	if (s->flip_y){
		SWAP(x1,x4);
		SWAP(y1,y4);
		SWAP(x2,x3);
		SWAP(y2,y3);
	}
	
	//rotate around s.home_x, s.home_y
	if (s->angle.a){
		fixp a = -s->angle.a * 3.14159 / 180;
		rotate_xy(&x1,&y1,a);
		rotate_xy(&x2,&y2,a);
		rotate_xy(&x3,&y3,a);
		rotate_xy(&x4,&y4,a);
	}
	
	//move rotated+scaled to correct location
	fixp x = s->pos.x; 
	fixp y = s->pos.y;
	x1 += x;
	y1 += y;
	x2 += x;
	y2 += y;
	x3 += x;
	y3 += y;
	x4 += x;
	y4 += y;
	
	//1--2
	//|
	//|
	//4
	fixp xcstep_tx = (x2 - x1) / s->w * 8;
	fixp ycstep_tx = (y2 - y1) / s->w * 8;
	fixp xcstep_ty = (x4 - x1) / s->h * 8;
	fixp ycstep_ty = (y4 - y1) / s->h * 8;

	fixp xc = x1;
	fixp yc = y1;
	int chr = 4 * get_sprite_chr(s);
	
//	int va_new_start = sfVertexArray_getVertexCount(va);
	// Round to integers
	int xcstep_txi = xcstep_tx;// >> FIXPOINT;
	int ycstep_txi = ycstep_tx;// >> FIXPOINT;
	int xcstep_tyi = xcstep_ty;// >> FIXPOINT;
	int ycstep_tyi = ycstep_ty;// >> FIXPOINT;
	for (int ty = 0; ty < s->h/8; ++ty){
		for (int tx = 0; tx < s->w/8; ++tx){
			int xci = xc;
			int yci = yc;
			add_tile(sa, xci, yci, xcstep_txi, ycstep_txi, xcstep_tyi, ycstep_tyi, s->pal, chr);

			xc += xcstep_tx;
			yc += ycstep_tx;
			++chr;
		}
		xc += -s->w / 8 * xcstep_tx;
		yc += -s->w / 8 * ycstep_tx;
		xc += xcstep_ty;
		yc += ycstep_ty;
	}
	
//	va_index.insert({s.id, {va_new_start , va.getVertexCount()}});
}

void draw_sprite_array(struct display* d, struct sprite_array* s){
	GLuint vertices;
	GLuint indices;
	glGenBuffers(1, &vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferData(GL_ARRAY_BUFFER, s->size * VERTICES_PER_TILE * sizeof(vertex), s->vertices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, s->size * sizeof(quad), s->quads, GL_DYNAMIC_DRAW);

	// Rendering
	// Setup vertex attributes
	glEnableVertexAttribArray(d->gl_pos);
	glEnableVertexAttribArray(d->gl_col);
	glEnableVertexAttribArray(d->gl_tex);

	//Set vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexAttribPointer(d->gl_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_X * sizeof(GLfloat)));
	glVertexAttribPointer(d->gl_col, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_PALETTE * sizeof(GLfloat)));
	glVertexAttribPointer(d->gl_tex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(VERTEX_U * sizeof(GLfloat)));

	//Set index data and render
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indices );
	glDrawElements( GL_TRIANGLES, s->size * sizeof(quad), GL_UNSIGNED_INT, NULL );

	// Cleanup (TODO:CODE:NONE why is this needed?)
	glDisableVertexAttribArray(d->gl_pos);
	glDisableVertexAttribArray(d->gl_col);
	glDisableVertexAttribArray(d->gl_tex);

	glDeleteBuffers(1, &vertices);
	glDeleteBuffers(1, &indices);
}

