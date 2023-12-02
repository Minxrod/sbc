#include "sprite.h"

#include <math.h>

#include "sprites.h"
#include "common.h"

struct sprite_array init_sprite_array(void){
	struct sprite_array s = { sfVertexArray_create() };
	sfVertexArray_setPrimitiveType(s.va, sfQuads);
	return s;
}

void free_sprite_array(struct sprite_array s){
	sfVertexArray_destroy(s.va);
}

#define SWAP(a,b) { fixp temp = a; a = b; b = temp; }

sfVector2f chr_texCoords2(int chr, int xofs, int yofs){
	int x = chr % 32;
	int y = chr / 32;
	return (sfVector2f){8*(x+xofs), 8*(y+yofs)};
}

void rotate_xy(fixp* x, fixp* y, const fixp a){
	fixp ox = *x;
	double angle = a/4096.0; 
	*x = *x * cos(angle) + *y * sin(angle);
	*y = ox *-sin(angle) + *y * cos(angle);
}

void add_sprite(struct sprite_array sa, struct sprite_info* s){
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
		fixp a = s->angle.a * 3.14159 / 180;
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
	sfColor c = {(unsigned char)(16*s->pal), 0, 0, 0};

	fixp xc = x1;
	fixp yc = y1;
	int chr = 4*s->chr;
	if (s->anim.loop_forever || s->anim.loop)
		chr = 4*s->anim.current_chr;
	
//	int va_new_start = sfVertexArray_getVertexCount(va);
	// Round to integers
	int xcstep_txi = xcstep_tx >> FIXPOINT;
	int ycstep_txi = ycstep_tx >> FIXPOINT;
	int xcstep_tyi = xcstep_ty >> FIXPOINT;
	int ycstep_tyi = ycstep_ty >> FIXPOINT;
	for (int ty = 0; ty < s->h/8; ++ty){
		for (int tx = 0; tx < s->w/8; ++tx){
			int xci = xc >> FIXPOINT;
			int yci = yc >> FIXPOINT;
			sfVertexArray_append(sa.va, (sfVertex){(sfVector2f){xci, yci}, c, chr_texCoords2(chr,0,0)});
			sfVertexArray_append(sa.va, (sfVertex){(sfVector2f){xci+xcstep_txi, yci+ycstep_txi}, c, chr_texCoords2(chr,1,0)});
			sfVertexArray_append(sa.va, (sfVertex){(sfVector2f){xci+xcstep_txi+xcstep_tyi, yci+ycstep_txi+ycstep_tyi}, c, chr_texCoords2(chr,1,1)});
			sfVertexArray_append(sa.va, (sfVertex){(sfVector2f){xci+xcstep_tyi, yci+ycstep_tyi}, c, chr_texCoords2(chr,0,1)});
			
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
