#include <stdlib.h>
#include <stdio.h>

#include "interpreter/vars.h"
#include "common.h"
#include "system.h"
#include "resources.h"

#include "interpreter.h"
#include "program.h"


struct ptc* init_system(int var, int str, int arr){
	iprintf("init_system calloc: %d\n", (int)sizeof(struct ptc));
	struct ptc* ptc = calloc(sizeof(struct ptc), 1);
	if (ptc == NULL){
		iprintf("Error allocating memory!\n");
		abort();
	}
//	*ptc = (const struct ptc){0}; // see if this reduces stack
	
	// init vars memory
	init_mem_var(&ptc->vars, var);
	init_mem_str(&ptc->strs, str, STRING_CHAR);
	iprintf("%p %p %p %p\n", (void*)ptc->vars.vars, (void*)ptc->strs.strs, ptc->strs.str_data, NULL);
//	return ptc;
	init_mem_arr(&ptc->arrs, var, arr);
	ptc->vars.strs = &ptc->strs;
	ptc->vars.arrs = &ptc->arrs;
	// init various ptc items
	// TODO:CODE:LOW use subsytem initialization functions?
	ptc->console.tabstep = 4;
	ptc->res.visible = VISIBLE_ALL;
	init_input(&ptc->input);
	init_sprites(&ptc->sprites);
	
	init_resource(&ptc->res);
	
	// must occur after resources as it depend on SCR
	init_panel(ptc);
	
	return ptc;
}

void free_system(struct ptc* p){
	free_panel(p);
	free_resource(&p->res);
	free_mem_arr(&p->arrs);
	free_mem_str(&p->strs);
	free_mem_var(&p->vars);
	free(p);
}

//https://smilebasicsource.com/forum/thread/docs-ptc-acls
char acls_code[] = 
"VISIBLE 1,1,1,1,1,1:'ICONCLR\r"
"COLOR 0,0:CLS:'GDRAWMD FALSE\r"
"FOR P=1 TO 0 STEP -1\r"
" GPAGE P,P,P:GCOLOR 0:GCLS:'GPRIO 3\r"
" BGPAGE P:BGOFS 0,0,0:BGOFS 1,0,0\r"
" BGCLR:BGCLIP 0,0,31,23\r"
" SPPAGE P:SPCLR\r"
"NEXT\r"
"FOR I=0 TO 255\r"
" 'COLINIT \"BG\", I:COLINIT \"SP\", I\r"
" 'COLINIT \"GRP\",I\r"
"NEXT\r";

char acls_bytecode[2*sizeof acls_code];

void cmd_acls(struct ptc* p){
	// copy vars and use a temp variables for this snippet
	struct runner cur_exec = p->exec; // copy code state
	struct variables temp_vars = {0};
	init_mem_var(&temp_vars, 4); //P,I
	struct variables vars = p->vars;
	p->vars = temp_vars;
	
	struct program acls_program = { sizeof acls_code, acls_code };
	struct program acls_bc = { 0, acls_bytecode };
	
	tokenize(&acls_program, &acls_bc);
	
	run(&acls_bc, p);
	p->exec = cur_exec;
	
	// Restore proper program variable state
	p->vars = vars;
}

void cmd_visible(struct ptc* p){
	// TODO:IMPL:MED
	p->stack.stack_i = 0;
}

void cmd_vsync(struct ptc* p){
	int delay = STACK_INT(0);
	
	int64_t start_time = get_time(&p->time);
	while (get_time(&p->time) - start_time < delay){
		// wait for 10ms before checking again
		thrd_sleep(&(struct timespec){.tv_nsec=(1e7)}, NULL);
	}
}

void cmd_wait(struct ptc* p){
	int delay = STACK_INT(0);
	
	// wait for duration of delay
	int big = delay / 60;
	int small = delay % 60;
	
	thrd_sleep(&(struct timespec){.tv_sec = big, .tv_nsec=(1e7*small/60*1e2)}, NULL);
}

void cmd_clear(struct ptc* p){
	reset_var(&p->vars);
	reset_str(&p->strs);
	reset_arr(&p->arrs);
}

#ifdef ARM9
void system_draw(struct ptc* p){
	//TODO:CODE:MED Move to separate function/file for NDS rendering?
	//TODO:IMPL:MED background tile/color
	u16* map = p->res.bg_upper;
	for (int y = 0; y < CONSOLE_HEIGHT; ++y){
		for (int x = 0; x < CONSOLE_WIDTH; ++x){
			u16 t = to_char(con_text_getc(&p->console, x, y));
			t |= (con_col_get(&p->console, x, y) & COL_FG_MASK) << 12; //TODO:CODE:NONE 12 should be a constant?
			*map = t;
			map++;
		}
	}
}
#endif

#ifdef PC
#include <SFML/Graphics.h>
#include "graphics/pc/tilemap.h"
#include "graphics/pc/graphic.h"
#include "graphics/pc/sprite.h"

void system_draw(sfRenderWindow* rw, struct ptc* p){
	// TODO:IMPL:HIGH Implement VISIBLE
	// TODO:IMPL:HIGH Every system
	
	// Prepare graphics here
	// TODO:CODE:LOW No dynamic allocations here
	struct tilemap console_map;
	console_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT);
	struct tilemap panel_text_map;
	panel_text_map = init_tilemap(CONSOLE_WIDTH, CONSOLE_HEIGHT);
	
	for (int x = 0; x < CONSOLE_WIDTH; ++x){
		for (int y = 0; y < CONSOLE_HEIGHT; ++y){
			// TODO:IMPL:MED background tile/color[
			tile(&console_map, x, y, to_char(con_text_getc(&p->console, x, y)), 0, 0);
			palette(&console_map, x, y, con_col_get(&p->console, x, y) & COL_FG_MASK);
			
			tile(&panel_text_map, x, y, to_char(con_text_getc(p->panel.text, x, y)), 0, 0);
			palette(&panel_text_map, x, y, con_col_get(p->panel.text, x, y) & COL_FG_MASK);
		}
	}
	
	// graphics
	struct graphic graphic = init_graphic(GRP_WIDTH, GRP_HEIGHT);
	draw_graphic(&graphic, p);
	
	// draw backgrounds
	struct tilemap background_map;
	background_map = init_tilemap(BG_WIDTH, BG_HEIGHT);
	struct tilemap foreground_map;
	foreground_map = init_tilemap(BG_WIDTH, BG_HEIGHT);
	struct tilemap panel_bg_map;
	panel_bg_map = init_tilemap(BG_WIDTH, BG_HEIGHT);
	
	for (int x = 0; x < BG_WIDTH; ++x){
		for (int y = 0; y < BG_HEIGHT; ++y){
			// BG0 and BG1
			u16 td = bg_tile(p,0,1,x,y);
			tile(&background_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
			palette(&background_map, x, y, (td & 0xf000) >> 12);
			td = bg_tile(p,0,0,x,y);
			tile(&foreground_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
			palette(&foreground_map, x, y, (td & 0xf000) >> 12);
			// panel
			td = bg_tile(p,1,2,x,y);
			tile(&panel_bg_map, x, y, td & 0x3ff, (td & 0x400) >> 10, (td & 0x800) >> 11);
			palette(&panel_bg_map, x, y, 0);
			// doesn't use any palette other than zero
		}
	}
	
	// draw sprites
	struct sprite_array sprites = init_sprite_array();
	for (int i = 0; i < MAX_SPRITES; ++i){
		if (p->sprites.info[0][i].active){
			add_sprite(sprites, &p->sprites.info[0][i]);
		}
	}
	
	struct sprite_array panel_keys = init_sprite_array();
	if (p->panel.type != PNL_OFF && p->panel.type != PNL_PNL){
		for (int i = 0; i < PANEL_KEYS; ++i){
			// active is a good check to see if the sprite is correctly defined, for now
			if (p->panel.keys[i].active){
				add_sprite(panel_keys, &p->panel.keys[i]);
			}
		}
	}
	
	// SFML stuff
	sfView* view;
	view = sfView_createFromRect((sfFloatRect){0, 0, 256, 192});
	sfView_setViewport(view, (sfFloatRect){0, 0, 1, 0.5f});
	sfRenderWindow_setView(rw, view);
	
	// actual draw commands
	sfRenderStates rs = sfRenderStates_default();
	sfShader* shader = p->res.shader;

	sfShader_setTextureUniform(shader, "colors", p->res.col_tex);
	sfShader_setCurrentTextureUniform(shader, "texture");
	rs.shader = shader;
	
	sfShader_setFloatUniform(shader, "colbank", 2);
	sfShader_setBoolUniform(shader, "grp_mode", true);
	sfRenderWindow_drawSprite(rw, graphic.sprite, &rs);
	
	rs.texture = p->res.chr_tex[2];
	sfShader_setFloatUniform(shader, "colbank", 0);
	sfShader_setBoolUniform(shader, "grp_mode", false);
	sfRenderWindow_drawVertexArray(rw, background_map.va, &rs);
	
	sfShader_setFloatUniform(shader, "colbank", 0);
	sfShader_setBoolUniform(shader, "grp_mode", false);
	sfRenderWindow_drawVertexArray(rw, foreground_map.va, &rs);
	
	rs.texture = p->res.chr_tex[3];
	sfShader_setFloatUniform(shader, "colbank", 1);
	sfShader_setBoolUniform(shader, "grp_mode", false);
	sfRenderWindow_drawVertexArray(rw, sprites.va, &rs);
	
	rs.texture = p->res.chr_tex[0];
	sfShader_setFloatUniform(shader, "colbank", 0);
	sfShader_setBoolUniform(shader, "grp_mode", false);
	sfRenderWindow_drawVertexArray(rw, console_map.va, &rs);
	
	// lower screen
	// Translate and set views to render on lower screen
	sfView_setViewport(view, (sfFloatRect){0, 0.5f, 1, 0.5f});
	sfRenderWindow_setView(rw, view);
	
	/*sfTransform tf = sfTransform_fromMatrix(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	);
	rs.transform = tf;*/
	
	// Panel render
	// BG layer
	rs.texture = p->res.chr_tex[6];
	sfShader_setFloatUniform(shader, "colbank", 3);
	sfShader_setBoolUniform(shader, "grp_mode", false);
	sfRenderWindow_drawVertexArray(rw, panel_bg_map.va, &rs);
	// Sprite layer
	rs.texture = p->res.chr_tex[8];
	sfShader_setFloatUniform(shader, "colbank", 4);
	sfShader_setBoolUniform(shader, "grp_mode", false);
	sfRenderWindow_drawVertexArray(rw, panel_keys.va, &rs);
	// Text layer
	rs.texture = p->res.chr_tex[5];
	sfShader_setFloatUniform(shader, "colbank", 3);
	sfShader_setBoolUniform(shader, "grp_mode", false);
	sfRenderWindow_drawVertexArray(rw, panel_text_map.va, &rs);
	
	free_tilemap(&console_map);
	free_graphic(&graphic);
	free_tilemap(&background_map);
	free_tilemap(&foreground_map);
	free_sprite_array(sprites);
	free_tilemap(&panel_bg_map);
	free_tilemap(&panel_text_map);
	
	sfView_destroy(view);
}
#endif
