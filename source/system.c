#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "system.h"
#include "vars.h"

struct ptc* init_system(int var, int str, int arr){
	struct ptc* ptc = malloc(sizeof(struct ptc));
	if (ptc == NULL){
		iprintf("Error allocating memory!");
		abort();
	}
	*ptc = (const struct ptc){0};
	
	// init vars memory
	init_mem_var(&ptc->vars, var);
	init_mem_str(&ptc->strs, str, STRING_CHAR);
	init_mem_arr(&ptc->arrs, var, arr);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// init various ptc items
	// TODO: subsytem initialization functions?
	ptc->console.tabstep = 4;
	ptc->console.test_mode = false;
	ptc->g.visible = VISIBLE_ALL;
	
	init_resource(&ptc->res);
	
	//allocate ptc struct here?
	return ptc;
}

void free_system(struct ptc* p){
	free_mem_arr(&p->arrs);
	free_mem_str(&p->strs);
	free_mem_var(&p->vars);
	free_resource(&p->res);
	free(p);
}

void cmd_acls(struct ptc* p){
	// TODO: Implement
	p->stack.stack_i = 0;
}

void cmd_visible(struct ptc* p){
	// TODO: Actually work
	p->stack.stack_i = 0;
}

void cmd_vsync(struct ptc* p){
	// TODO: Actually work
	p->stack.stack_i = 0;
}

void cmd_wait(struct ptc* p){
	// TODO: Actually work
	p->stack.stack_i = 0;
}

#ifdef PC
#include <SFML/Graphics.h>
#include "pc/tilemap.h"

void system_draw(sfRenderWindow* rw, struct ptc* p){
	// TODO: visible
	// TODO: Every system except text
	
	sfShader* shader;
	sfRenderStates rs = sfRenderStates_default();
	rs.texture = gen_chr_texture(p->res.chr[0], 256);
	if (!sfShader_isAvailable()){
		iprintf("Error: Shaders are unavailable!");
		abort();
	} else {
		if (!(shader = sfShader_createFromFile(NULL, NULL, "resources/bgsp.frag"))){
			iprintf("Error: Shader failed to load!");
			abort();
		}
	}
	static sfTexture* col_tex = 0;
	if (!col_tex)
		col_tex = gen_col_texture(p->res.col[0]);
	sfShader_setTextureUniform(shader, "colors", col_tex);
	sfShader_setCurrentTextureUniform(shader, "texture");
	sfShader_setFloatUniform(shader, "colbank", 0); //TODO change
	rs.shader = shader;
	
	struct tilemap map;
	map = tilemap_init(32, 24);
	
	for (int x = 0; x < 32; ++x){
		for (int y = 0; y < 24; ++y){
			tile(&map, x, y, to_char(con_text_getc(&p->console, x, y)), 0, 0);
			// TODO: color palette
			// TODO: background tile/color[
//			iprintf("Color?:%d,%d,%d\n", x,y,p->console.color[x][y] & COL_FG_MASK);
			palette(&map, x, y, p->console.color[x][y] & COL_FG_MASK);
		}
	}
	
	sfRenderWindow_drawVertexArray(rw, map.va, &rs);
//	sfTexture_destroy(col_tex);
}
#endif
