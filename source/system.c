#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "system.h"
#include "vars.h"

struct ptc* system_init(){
	struct ptc* ptc = malloc(sizeof(struct ptc));
	if (ptc == NULL){
		iprintf("Error allocating memory!");
		abort();
	}
	*ptc = (const struct ptc){0};
	
	// init vars memory
	init_mem_var(&ptc->vars, 16);
	init_mem_str(&ptc->strs, 32, STRING_CHAR);
	init_mem_arr(&ptc->arrs, 16, 64);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// init various ptc items
	// TODO: subsytem initialization functions?
	ptc->console.tabstep = 4;
	
	resource_init(&ptc->res);
	
	//allocate ptc struct here?
	return ptc;
}

#ifdef PC
#include <SFML/Graphics.h>
#include "tilemap.h"

void system_draw(sfRenderWindow* rw, struct ptc* p){
	// TODO: visible
	// TODO: Every system except text
	
	sfRenderStates rs = sfRenderStates_default();
	rs.texture = gen_chr_texture(p->res.chr[0], 256);
	
	struct tilemap map;
	map = tilemap_init(32, 24);
	
	for (int x = 0; x < 32; ++x){
		for (int y = 0; y < 24; ++y){
			tile(&map, x, y, (u8)con_text_getc(&p->console, x, y), 0, 0);
			// TODO: color palette
			// TODO: background tile/color
			palette(&map, x, y, 255);
		}
	}
	
	sfRenderWindow_drawVertexArray(rw, map.va, &rs);
}
#endif
