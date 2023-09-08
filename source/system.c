#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "system.h"
#include "vars.h"
#include "resources.h"

struct ptc* init_system(int var, int str, int arr){
	struct ptc* ptc = calloc(sizeof(struct ptc), 1);
	if (ptc == NULL){
		iprintf("Error allocating memory!");
		abort();
	}
//	*ptc = (const struct ptc){0}; // see if this reduces stack
	
	// init vars memory
	init_mem_var(&ptc->vars, var);
	init_mem_str(&ptc->strs, str, STRING_CHAR);
	init_mem_arr(&ptc->arrs, var, arr);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// init various ptc items
	// TODO:CODE use subsytem initialization functions?
	ptc->console.tabstep = 4;
	ptc->res.visible = VISIBLE_ALL;
	init_input(&ptc->input);
	
	init_resource(&ptc->res);
	
	//allocate ptc struct here?
	return ptc;
}

void free_system(struct ptc* p){
	free_resource(&p->res);
	free_mem_arr(&p->arrs);
	free_mem_str(&p->strs);
	free_mem_var(&p->vars);
	free(p);
}

void cmd_acls(struct ptc* p){
	// TODO:IMPL
	p->stack.stack_i = 0;
}

void cmd_visible(struct ptc* p){
	// TODO:IMPL
	p->stack.stack_i = 0;
}

void cmd_vsync(struct ptc* p){
	// TODO:IMPL
	p->stack.stack_i = 0;
}

void cmd_wait(struct ptc* p){
	// TODO:IMPL
	p->stack.stack_i = 0;
}

void cmd_clear(struct ptc* p){
	reset_var(&p->vars);
	reset_str(&p->strs);
	reset_arr(&p->arrs);
}

#ifdef ARM9
void system_draw(struct ptc* p){
	u16* map = p->res.bg_upper;
	for (int y = 0; y < CONSOLE_HEIGHT; ++y){
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			u16 t = to_char(con_text_getc(&p->console, x, y));
			t |= (con_col_get(&p->console, x, y) & COL_FG_MASK) << 12; //TODO:CODE 12 should be a constant?
			*map = t;
			map++;
		}
	}
	// TODO:IMPL background tile/color
}
#endif

#ifdef PC
#include <SFML/Graphics.h>
#include "pc/tilemap.h"

void system_draw(sfRenderWindow* rw, struct ptc* p){
	// TODO:IMPL Implement VISIBLE
	// TODO:IMPL Every system except text
	
	sfRenderStates rs = sfRenderStates_default();
	sfShader* shader = p->res.shader;
	rs.texture = p->res.chr_tex[0];
	
	sfShader_setTextureUniform(shader, "colors", p->res.col_tex);
	sfShader_setCurrentTextureUniform(shader, "texture");
	sfShader_setFloatUniform(shader, "colbank", 0); //TODO change
	rs.shader = shader;
	
	// TODO:CODE No dynamic allocations here
	struct tilemap console_map;
	console_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT);
	
	for (int x = 0; x < CONSOLE_WIDTH; ++x){
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			tile(&console_map, x, y, to_char(con_text_getc(&p->console, x, y)), 0, 0);
			// TODO:IMPL background tile/color[
//			iprintf("Color?:%d,%d,%d\n", x,y,p->console.color[x][y] & COL_FG_MASK);
			palette(&console_map, x, y, con_col_get(&p->console, x, y) & COL_FG_MASK);
		}
	}
	
	sfRenderWindow_drawVertexArray(rw, console_map.va, &rs);
	free_tilemap(&console_map);
//	sfTexture_destroy(col_tex);
}
#endif
