#include "sprites.h"

#include "system.h"
#include "vars.h"
#include "error.h"

void init_sprites(struct sprites* s){
	*s = (struct sprites){0};
}

struct sprite_info init_sprite_info(int id,int chr,int pal,bool horiz,bool vert,int prio,int w, int h){
	return (struct sprite_info){
		true,
		id,chr,pal,w,h,prio,horiz,vert,
		0,0,
		{ // position
			0,0,0,0,-1
		},{ // scale
			INT_TO_FP(1),0,0
		},{ // angle
			0,0,0
		},{ // collision
			0,0,0,0,INT_TO_FP(w),INT_TO_FP(h),false,0xff
		},{ // animation
			1,0,0,false,0,0
		},{ // sprite vars
			0,0,0,0,0,0,0,0
		}
	};
}

void free_sprites(struct sprites* s){
//	free(s);
	(void)s;
}

// Calculate sprite collisions using the separating axis theorem
// If you project the sprite's vertices onto the normals of both sprites
// and they don't overlap, then they aren't colliding. Otherwise, they are.
// TODO:TEST:MED This really needs a lot of testing.
bool is_hit(struct sprite_info* a, struct sprite_info* b){
	if (!a->active || !b->active)
		return false; //can't hit inactive sprites
	
	if (!(a->hit.mask & b->hit.mask))
		return false; //can't hit different groups
	
	// TODO:CODE:NONE Rewrite to use a vector struct?
	// Note: These are vectors, so the numeric values shouldn't matter too much
	fixp normals[3][2] = {
		{INT_TO_FP(1), 0},
		{0, INT_TO_FP(1)},
		{0, 0}
	};
	
	if (!a->hit.dx && !a->hit.dy){
		normals[2][0] = -a->pos.dy;
		normals[2][1] = a->pos.dx;
	} else {
		normals[2][0] = -a->hit.dy;
		normals[2][1] = a->hit.dx;
	}
	
	if (!b->hit.dx && !b->hit.dy){
		normals[2][0] -= -b->pos.dy;
		normals[2][1] -= b->pos.dx;
	} else {
		normals[2][0] -= -b->hit.dy;
		normals[2][1] -= b->hit.dx;
	}
	
	// TODO:PERF:LOW Check if reducing to regular AABB case is faster for sprites
	// with only trivial vectors
	fixp a_verts[8][2] = {
		// vertices of a
		{a->pos.x + a->hit.x, a->pos.y + a->hit.y},
		{a->pos.x + a->hit.x + a->hit.w, a->pos.y + a->hit.y},
		{a->pos.x + a->hit.x + a->hit.w, a->pos.y + a->hit.y + a->hit.h},
		{a->pos.x + a->hit.x, a->pos.y + a->hit.y + a->hit.h},
		// vertices of a + combined collision vectors of a+b
		{a->pos.x + a->hit.x + normals[2][1], a->pos.y + a->hit.y - normals[2][0]},
		{a->pos.x + a->hit.x + a->hit.w + normals[2][1], a->pos.y + a->hit.y - normals[2][0]},
		{a->pos.x + a->hit.x + a->hit.w + normals[2][1], a->pos.y + a->hit.y + a->hit.h - normals[2][0]},
		{a->pos.x + a->hit.x + normals[2][1], a->pos.y + a->hit.y + a->hit.h - normals[2][0]},
	};
	
	fixp b_verts[4][2] = {
		{b->pos.x + b->hit.x, b->pos.y + b->hit.y},
		{b->pos.x + b->hit.x + b->hit.w, b->pos.y + b->hit.y},
		{b->pos.x + b->hit.x + b->hit.w, b->pos.y + b->hit.y + b->hit.h},
		{b->pos.x + b->hit.x, b->pos.y + b->hit.y + b->hit.h},
	};
	
	for (idx i = 0; i < sizeof(normals)/sizeof(normals[0]); ++i){
		if (!normals[i][0] && !normals[i][1]){
			continue;
		}
		
		//TODO:PERF:LOW Check if verts can be swapped and see which is faster
		// Get the bounds of the projected coordinates
		int64_t min = 0x7fffffffffffffff;
		int64_t max = 0x8000000000000000;
		for (idx j = 0; j < sizeof(b_verts)/sizeof(b_verts[0]); ++j){
			// Fixpoint becomes 40.24 here
			int64_t d = (int64_t)normals[i][0] * (int64_t)b_verts[j][0] + (int64_t)normals[i][1] * (int64_t)b_verts[j][1];
			
			if (d < min) min = d;
			if (d > max) max = d;
		}
		
		// Check if vertices of other sprite overlap the first projected sprite
		bool in_range = false;
		bool on_left = false;
		bool on_right = false;
		for (idx j = 0; j < sizeof(a_verts)/sizeof(a_verts[0]); ++j){
			// Fixpoint becomes 40.24 here
			// Same scale as min and max, so the math still works
			int64_t d = (int64_t)normals[i][0] * (int64_t)a_verts[j][0] + (int64_t)normals[i][1] * (int64_t)a_verts[j][1];
			
			if (d < min) on_left = true;
			if (d > max) on_right = true;
			if (min <= d && d <= max) in_range = true;
			if (on_left && on_right) in_range = true;
		}
		
		if (!in_range) return false;
	}
	// No separating axis was found
	return true;
}

void cmd_sppage(struct ptc* p){
	int page = STACK_INT(0);
	
	p->sprites.page = page;
}

void cmd_spclr(struct ptc* p){
	if (p->stack.stack_i == 1){
		// SPCLR id
		int index = STACK_INT(0);
		// disable sprite: no values matter if inactive
		p->sprites.info[p->sprites.page][index].active = false;
	} else {
		// SPCLR
		for (int i = 0; i < MAX_SPRITES; ++i){
			p->sprites.info[p->sprites.page][i].active = false;
		}
	}
}

void cmd_spset(struct ptc* p){
	fixp id = STACK_INT(0);
	fixp chr = STACK_INT(1);
	fixp pal = STACK_INT(2);
	bool horiz_flip = STACK_INT(3) != 0;
	bool vert_flip = STACK_INT(4) != 0;
	fixp prio = STACK_INT(5);
	fixp width = INT_TO_FP(16);
	fixp height = INT_TO_FP(16);
	if (p->stack.stack_i == 8){
		// spset id,chr,pal,h,v,prio,w,h
		width = STACK_INT(6);
		height = STACK_INT(7);
	}
	// TODO:ERR:MED bounds checking
	// TODO:IMPL:HIGH Finish initializing the rest of the important properties
	p->sprites.info[p->sprites.page][id] = init_sprite_info(id,chr,pal,horiz_flip,vert_flip,prio,width,height);
}

void cmd_spofs(struct ptc* p){
	// TODO:ERR:MED bounds checking
	fixp id = STACK_INT(0);
	fixp x = STACK_NUM(1);
	fixp y = STACK_NUM(2);
	if (p->stack.stack_i == 3){
		// spofs id,x,y
		p->sprites.info[p->sprites.page][id].pos.x = x;
		p->sprites.info[p->sprites.page][id].pos.y = y;
	} else {
		ERROR(ERR_UNIMPLEMENTED);
	}
}
