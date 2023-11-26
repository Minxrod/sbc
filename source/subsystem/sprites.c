#include "sprites.h"

#include "system.h"
#include "vars.h"

void init_sprites(struct sprites* s){
	*s = (struct sprites){0};
}

struct sprite_info init_sprite_info(int id,int chr,int pal,int w, int h, int prio){
	return (struct sprite_info){
		true,
		id,chr,pal,w,h,prio,false,false,
		0,0,
		{ // position
			0,0,0,0,-1
		},{ // scale
			INT_TO_FP(1),0,0
		},{ // angle
			0,0,0
		},{ // collision
			0,0,0,0,0,0,false,0xff
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


