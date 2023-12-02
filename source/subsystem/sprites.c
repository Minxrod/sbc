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
			0,0,0,0,w,h,false,0xff
		},{ // animation
			1,0,0,false,0,0
		},{ // sprite vars
			0,0,0,0,0,0,0,0
		}
	};
}

void free_sprites(struct sprites* s){
	free(s);
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
