#ifdef ARM9
void v(void); //prevent empty translation unit
#endif

#ifndef ARM9
//this is the computer-only file

#include <SFML/System.h>
#include <SFML/Graphics.h>
#include <SFML/Window.h>

#include <stdio.h>
#include <stdlib.h>

/*#include "tokens.h"
#include "runner.h"
#include "program.h"*/
#include "resources.h"
#include "system.h"

#include "tilemap.h"

int main(int argc, char** argv){
	if (argc == 2){
		// TODO: Load file as program
	} else {
		// Todo: Load default program
	}
	
	struct ptc* ptc = system_init();
	
	// https://gist.github.com/def-/fee8bb041719337c8812
	// used as convenient reference
	sfRenderWindow* window;
	
	window = sfRenderWindow_create((sfVideoMode){256,384,32}, argv[0], sfResize | sfClose, NULL);
	if (!window){
		printf("Failed to create the render window!");
		abort();
	}
	
	FILE* f = fopen("BGF0.PTC", "rb");
	fread(ptc->res.chr[0], sizeof(u8), 0x30, f); //header skip
	size_t rc = fread(ptc->res.chr[0], sizeof(u8), CHR_SIZE, f);
	if (rc < CHR_SIZE){
		iprintf("fuck\n");
		abort();
	}
	fclose(f);
	
	sfEvent event;
	while (sfRenderWindow_isOpen(window)){
		while (sfRenderWindow_pollEvent(window, &event)){
			if (event.type == sfEvtClosed){
				sfRenderWindow_close(window);
			}
			// TODO: Get keyboard and mouse/touch input
		}
		
		sfRenderWindow_clear(window, sfBlack);
		
//		sfRenderWindow_drawSprite(window, spr, NULL);
		
//		sfRenderWindow_drawVertexArray(window, map.va, &rs);
		system_draw(window, ptc);
		
		sfRenderWindow_display(window);
	}
	
	sfRenderWindow_destroy(window);
	
//	struct ptc* ptc;
	
	
	
/*	// initialize system buffers, etc.
	struct ptc* ptc = system_init();
	
	struct program program;
	program.data = code;
	for (program.size = 0; code[program.size] != '\0'; ++program.size);
//	struct program_code code
	
	struct program output;
	output.data = out;
	output.size = 0;
	
	tokenize(&program, &output);
	
	run(&output, ptc);
	
//	struct named_var* v = get_var(&ptc.vars, "A", 1, VAR_NUMBER);
//	iprintf("%d", v->value.number);
	
	// measure size of program file size vs original's memory vs remake's memory use
	iprintf("file:%d ptc:%d sbc:%d\n", program.size, program.size*2, output.size);
	
	return 0;*/
	
	// TODO: Clean up memory on exit
}


#endif


/*#include <nds.h>

#include "tokens.h"

#include <stdio.h>

void init(void){
	//regular bank setup
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);
	
	vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_MAIN_SPRITE, VRAM_C_SUB_BG, VRAM_D_SUB_SPRITE);
	
	// extended palette mode
	vramSetBankF(VRAM_F_BG_EXT_PALETTE);
	vramSetBankG(VRAM_G_SPRITE_EXT_PALETTE);
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
}

char* code = "?\"Hello world!\"\r?12345";

char out[4096] = {0};

int main(void){
	struct program program;
	program.data = code;
	program.size = 16;
	struct program code;
	code.data = out;
	code.size = 0;
	
	//init();
	consoleDemoInit();
	
	tokenize(&program, &code);
	
	while(1) {
	
		swiWaitForVBlank();
		scanKeys();
		
		int keys = keysDown();
		if (keys & KEY_START)
			break;
	}
	
	return 0;
}*/
