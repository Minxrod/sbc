#include "sprites.h"

#include "common.h"
#include "resources.h"
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
			1,0,0,false,0,-1
		},{ // sprite vars
			0,0,0,0,0,0,0,0
		}
	};
}

void step_sprites(struct sprites* s){
	// Note: Code adapted from PTC-EmkII Sprites.cpp (Sprites::update())
	for (int p = 0; p < SCREEN_COUNT; ++p){
		struct sprite_info* info = s->info[p]; //array of sprites
		for (int i = 0; i < MAX_SPRITES; ++i){
			struct sprite_info* spr = &info[i];
			if (!spr->active){ continue; } // only update live sprites
			if (spr->scale.time >= 0){
				spr->scale.s += spr->scale.ds;
				spr->scale.time--;
			}
			if (spr->angle.time >= 0){
				// TODO:IMPL:MED interpolation direction?
				spr->angle.a += spr->angle.da % INT_TO_FP(360);
				spr->angle.time--;
			}
			if (spr->pos.time >= 0){
				spr->pos.x += spr->pos.dx;
				spr->pos.y += spr->pos.dy;
				spr->pos.time--;
			}
			// TODO:PERF:NONE Check if this is slow.
			if (spr->anim.loop_forever || spr->anim.loop > 0){
				spr->anim.current_frame++;
				
				if (spr->anim.current_frame == spr->anim.frames_per_chr){ //single frame end
					spr->anim.current_frame = 0;
					int chr_step = spr->w * spr->h / 256;
					if (chr_step < 1) chr_step = 1;
					spr->anim.current_chr += chr_step;
					
					if ((spr->anim.current_chr - spr->chr) / chr_step == spr->anim.chrs){ //loop complete
						if (!spr->anim.loop_forever){
							spr->anim.loop--;
						}
						if (!spr->anim.loop_forever && spr->anim.loop == 0){ //all loops complete
							spr->anim.current_chr -= chr_step;
							//no more animation; frame remains last frame
							break;
						}
						spr->anim.current_chr = spr->chr;
					}
				}
			}
		}
	}
}

// Calculate sprite collisions using the separating axis theorem
// If you project the sprite's vertices onto the normals of both sprites
// and they don't overlap, then they aren't colliding. Otherwise, they are.
// TODO:TEST:MED This really needs a lot of testing.
bool is_hit(struct sprite_info* a, struct sprite_info* b){
	if (!a->active || !b->active){
		return false; //can't hit inactive sprites
	}
	if (!(a->hit.mask & b->hit.mask)){
		return false; //can't hit different groups
	}
	// TODO:IMPL:LOW Doesn't implement adjust for scale flag?
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

// Gets a character index for this sprite (in 4-character sprite units)
int get_sprite_chr(struct sprite_info* s){
	int chr = s->chr;
//	if (s->anim.loop_forever || s->anim.loop)
	if (s->anim.current_chr >= 0)
		chr = s->anim.current_chr;
	return chr;
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
	int width = 16;
	int height = 16;
	if (p->stack.stack_i == 8){
		// spset id,chr,pal,h,v,prio,w,h
		width = STACK_INT(6);
		height = STACK_INT(7);
	}
	// TODO:ERR:MED bounds checking
	// TODO:TEST:MED Validate that this initialization is correct
	p->sprites.info[p->sprites.page][id] = init_sprite_info(id,chr,pal,horiz_flip,vert_flip,prio,width,height);
}

void cmd_spofs(struct ptc* p){
	int id;
	fixp x, y;
	int time;
	STACK_INT_RANGE(0,0,99,id);
	x = STACK_NUM(1) & 0xfffff000; // TODO:TEST:LOW Check that these values are correct
	y = STACK_NUM(2) & 0xfffff000;
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	if (!s->active) ERROR(ERR_ILLEGAL_FUNCTION_CALL);

	if (p->stack.stack_i == 3){
		// spofs id,x,y
		s->pos.x = x;
		s->pos.y = y;
		s->pos.time = -1;
	} else {
		STACK_INT_MIN(3,0,time);
//		iprintf("x=%d y=%d t=%d\n", x, y, time);
		if (time){
			s->pos.dx = (x - s->pos.x) / time;
			s->pos.dy = (y - s->pos.y) / time;
//			iprintf("dx=%d dy=%d\n", s->pos.dx, s->pos.dy);
			s->pos.time = time - 1; // TODO:CODE:NONE rework step to use time and time == 0 instead?
		} else {
			s->pos.x = x;
			s->pos.y = y;
			s->pos.time = -1;
		}
	}
}

void cmd_spsetv(struct ptc* p){
	// SPSETV id,ix,val
	int id,ix;
	fixp val;
	STACK_INT_RANGE(0,0,99,id);
	STACK_INT_RANGE(1,0,7,ix);
	val = STACK_NUM(2);
	if (!p->sprites.info[p->sprites.page][id].active){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	p->sprites.info[p->sprites.page][id].vars[ix] = val;
}

void func_spgetv(struct ptc* p){
	// SPGETV id,ix
	int id,ix;
	STACK_REL_INT_RANGE(-2,0,99,id);
	STACK_REL_INT_RANGE(-1,0,7,ix);
	p->stack.stack_i -= 2;
	
	if (!p->sprites.info[p->sprites.page][id].active){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {p->sprites.info[p->sprites.page][id].vars[ix]}});
}


/// PTC command to set the collision information for a sprite.
/// 
/// Format: 
/// * `SPCOL id,x,y,w,h,scale[,group]`
/// 
/// Arguments:
/// * id: Sprite control number
/// * x: Hitbox x
/// * y: Hitbox y
/// * w: Hitbox width
/// * h: Hitbox height
/// * scale: Flag to adjust collision for `SPSCALE`?
/// * group: Collision mask
/// 
/// @param p System struct
void cmd_spcol(struct ptc* p){
	int id, x, y, w, h;
	bool scale;
	uint_fast8_t group = 0xff;
	STACK_INT_RANGE(0,0,99,id);
	STACK_INT_RANGE(1,-128,127,x);
	STACK_INT_RANGE(2,-128,127,y);
	STACK_INT_RANGE(3,0,255,w);
	STACK_INT_RANGE(4,0,255,h);
	STACK_INT_RANGE(5,0,1,scale);
	if (p->stack.stack_i == 7){
		STACK_INT_RANGE(6,0,255,group);
	}
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	if (!s->active){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	} else {
		s->hit.x = INT_TO_FP(x);
		s->hit.y = INT_TO_FP(y);
		s->hit.w = INT_TO_FP(w);
		s->hit.h = INT_TO_FP(h);
		s->hit.scale_adjust = scale;
		s->hit.mask = group;
	}
}

void cmd_sphome(struct ptc* p){
	int id;
	int_fast8_t x, y;
	STACK_INT_RANGE(0,0,99,id);
	x = STACK_INT(1) & 0xff; // TODO:TEST:MED check that this is correct
	y = STACK_INT(2) & 0xff;
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	if (!s->active){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	} else {
		s->home_x = x;
		s->home_y = y;
	}
}

void cmd_spscale(struct ptc* p){
	int id, scale, time;
	STACK_INT_RANGE(0,0,31,id);
	STACK_INT_RANGE(1,0,200,scale);
	time = p->stack.stack_i == 3 ? STACK_NUM(2) : 0;
	if (time < 0){
		ERROR(ERR_OUT_OF_RANGE);
	}
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	if (!s->active){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	if (p->stack.stack_i == 2){
		s->scale.s = INT_TO_FP(scale)/100;
		s->scale.time = -1;
	} else {
		STACK_INT_MIN(2,0,time);
		s->scale.ds = ((INT_TO_FP(scale)/100) - s->scale.s) / time;
		s->scale.time = time ? time : -1;
	}
}

void func_sphit(struct ptc* p){
	int id, start;
	start = 0;
	if (p->exec.argcount == 1){
		STACK_REL_INT_RANGE(-1,0,99,id);
		p->stack.stack_i -= 1;
	} else {
		STACK_REL_INT_RANGE(-2,0,99,id);
		STACK_REL_INT_RANGE(-1,0,99,start);
		p->stack.stack_i -= 2;
	}
	int hit = -1;
	struct sprite_info* this = &p->sprites.info[p->sprites.page][id];
	for (int i = start; i < MAX_SPRITES; ++i){
		if (i == id) continue; // skip own sprite
		struct sprite_info* other = &p->sprites.info[p->sprites.page][i];
		if (is_hit(this, other)){
			hit = i;
			p->sprites.sphitno = hit;
			break;
		}
	}
	// TODO:IMPL:MED SPHITX, SPHITY, SPHITT
	STACK_RETURN_INT(hit != -1);
}

void func_sphitsp(struct ptc* p){
	int id, other_id;
	STACK_REL_INT_RANGE(-1,0,99,id);
	STACK_REL_INT_RANGE(-2,0,99,other_id);
	
	struct sprite_info* this = &p->sprites.info[p->sprites.page][id];
	struct sprite_info* other = &p->sprites.info[p->sprites.page][other_id];
	
	if (is_hit(this, other)){
		STACK_RETURN_INT(1);
	} else {
		STACK_RETURN_INT(0);
	}
	
	// TODO:TEST:MED SPHITX, SPHITY, SPHITT, SPHITNO affected by this?
}

void cmd_spangle(struct ptc* p){
	int id, angle; //, time, direction;
	STACK_INT_RANGE(0,0,99,id);
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	if (!s->active) {
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	
	angle = (STACK_INT(1) % 360);
//	if (angle < 0) angle += 360;
	
	if (p->stack.stack_i == 2){
		s->angle.a = INT_TO_FP(angle);
		s->angle.time = -1;
	} else {
		int dir = 1;
		int time;
		STACK_INT_MIN(2,0,time);
		if (p->stack.stack_i == 4){
			dir = STACK_INT(3);
			if (dir != 1 && dir != -1){
				ERROR(ERR_ILLEGAL_FUNCTION_CALL); // TODO:ERR:LOW Check correct error code
			}
		}
		
		struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
//		s->angle.a = ;
		if (time){
			if (dir < 0){
				// 45 -> 90 step 45/time
				s->angle.da = (INT_TO_FP(angle) - s->angle.a) / time;
			} else {
				// 45 -> 90 step 315/time
				s->angle.da = (s->angle.a - INT_TO_FP(angle)) / time;
			}
		} else {
			s->angle.a = INT_TO_FP(angle);
		}
		s->angle.time = time ? time : -1;
	}
}

void cmd_spchr(struct ptc* p){
	int id, chr;
	STACK_INT_RANGE(0,0,99,id);
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	STACK_INT_RANGE(1,0,(p->sprites.page ? 117 : 511),chr);
	if (p->stack.stack_i == 2){
		// SPCHR id, chr
		s->chr = chr;
	} else {
		// SPCHR id, chr, pal, h, v, prio
		int pal, prio;
		bool h, v;
		STACK_INT_RANGE(2,0,15,pal);
		STACK_INT_RANGE(3,0,1,h);
		STACK_INT_RANGE(4,0,1,v);
		STACK_INT_RANGE(5,0,3,prio);
		
		s->chr = chr;
		s->pal = pal;
		s->flip_x = h;
		s->flip_y = v;
		s->prio = prio;
	}
}

void func_spchk(struct ptc* p){
	int id;
	STACK_REL_INT_RANGE(-1,0,99,id);
	p->stack.stack_i -= 1;
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	
	int result = s->pos.time >= 0;
	result |= (s->angle.time >= 0) << 1;
	result |= (s->scale.time >= 0) << 2;
	result |= (s->anim.loop_forever || s->anim.loop > 0) << 3;
	STACK_RETURN_INT(result);
}

void cmd_spread(struct ptc* p){
	int id;
	STACK_INT_RANGE(0,0,99,id);
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	// TODO:TEST:LOW Test ranges of values for correctness
	// TODO:TEST:LOW Test interpolation values
	if (p->stack.stack_i >= 2){
		fixp* x = (fixp*)ARG(1)->value.ptr;
		*x = INT_TO_FP(FP_TO_INT(s->pos.x));
	}
	if (p->stack.stack_i >= 3){
		fixp* y = (fixp*)ARG(2)->value.ptr;
		*y = INT_TO_FP(FP_TO_INT(s->pos.y));
	}
	if (p->stack.stack_i >= 4){
		fixp* a = (fixp*)ARG(3)->value.ptr;
		*a = s->angle.a;
	}
	if (p->stack.stack_i >= 5){
		fixp* sc = (fixp*)ARG(4)->value.ptr;
		*sc = s->scale.s;
	}
	if (p->stack.stack_i == 6){
		fixp* c = (fixp*)ARG(5)->value.ptr;
		*c = s->chr;
	}
}

void cmd_spanim(struct ptc* p){
	// SPANIM id chrs time [loop]
	int id, chrs, time;
	// TODO:ERR:LOW Determine errors
	STACK_INT_RANGE(0,0,99,id);
	STACK_INT_MIN(1,0,chrs);
	STACK_INT_MIN(2,0,time);
	
	struct sprite_info* s = &p->sprites.info[p->sprites.page][id];
	s->anim.chrs = chrs;
	s->anim.frames_per_chr = time;
	s->anim.current_frame = 0;
	s->anim.current_chr = s->chr;
	
	if (p->stack.stack_i == 3){
		s->anim.loop = 0;
		s->anim.loop_forever = true;
	} else {
		int loop;
		STACK_INT_MIN(3,0,loop);
		s->anim.loop = loop;
		s->anim.loop_forever = loop == 0;
	}
}

void sys_sphitno(struct ptc* p){
	STACK_RETURN_INT(p->sprites.sphitno);
}

