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
#include <threads.h>

#include "tokens.h"
#include "runner.h"
#include "program.h"
#include "resources.h"
#include "system.h"

#include "tilemap.h"

struct launch_info {
	struct ptc* p;
	struct program* prg;
};

int system_launch(void* launch_info){
	struct launch_info* info = (struct launch_info*)launch_info;
	
	// TODO: Check for escapes, pauses, etc.
	struct program bc = {0, malloc(2*info->prg->size)};
	tokenize(info->prg, &bc);
	free(info->prg->data); // TODO: remove for debugging purposes?
	// only needs BC, not source
	run(&bc, info->p);
	return info->p->exec.error;
}

int main(int argc, char** argv){
	struct program program;
	char* window_name = "SBC";
	if (argc == 2){
		// TODO: Load file as program
		window_name = argv[1];
		prg_load(&program, argv[1]);
	} else {
		// Load default program: TODO load an actual launcher program
		prg_load(&program, "SAMPLE1.PTC");
	}
	
	struct ptc* ptc = system_init(VAR_LIMIT, STR_LIMIT, ARR_LIMIT);
	
	// https://gist.github.com/def-/fee8bb041719337c8812
	// used as convenient reference
	sfRenderWindow* window;
	
	window = sfRenderWindow_create((sfVideoMode){256,384,32}, window_name, sfResize | sfClose, NULL);
	if (!window){
		printf("Failed to create the render window!\n");
		abort();
	}
	
	// THREAD MODEL
	// WINDOW                    PROGRAM
	//  Event -> Input ==>        BUTTON, KEYBOARD, etc.
	//  Stop -> Load Program =>   struct ptc
	//  Rendering <==             struct ptc ~ display state
	
	// Launch the program thread
	struct launch_info info = {ptc, &program};
	thrd_t prog_thread;
	if (thrd_success != thrd_create(&prog_thread, system_launch, &info)){
		printf("Failed to create the program thread!\n");
		abort();
	}
	
	int keys[12];
	FILE* file = fopen("resources/config.txt", "r");
	if (!file){
		printf("Failed to read config file!\n");
		abort();
	}
	for (int i = 0; i < 12; ++i){
		int r = fscanf(file, "%d", &keys[i]);
		if (!r || r == EOF){
			printf("Failed to read key %d\n", 1+i);
			abort();
		}
		printf("%d\n", keys[i]);
	}
	
	sfEvent event;
	while (sfRenderWindow_isOpen(window)){
		while (sfRenderWindow_pollEvent(window, &event)){
			if (event.type == sfEvtClosed){
				sfRenderWindow_close(window);
			}
		}
		
		// TODO: SFML 2.6, use scan codes?
		int b = 0;
		for (int i = 0; i < 12; ++i){
			b |= sfKeyboard_isKeyPressed(keys[i]) << i;
		}
		set_input(&ptc->input, b);
		// TODO: Set buttons, touch, keys
		
		sfRenderWindow_clear(window, sfBlack);
		
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
