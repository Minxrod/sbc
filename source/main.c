#include "common.h"

#include "interpreter.h"

#include "program.h"
#include "resources.h"
#include "system.h"
#include "error.h"

// Common main stuff

#ifdef ARM9_BUILD
#include <nds.h>
#include <fat.h>

#include "system.h"
#include "program.h"
#include "interpreter/tokens.h"

#include <stdio.h>

void init(void){
	//regular bank setup
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);
	
	vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_MAIN_SPRITE, VRAM_C_SUB_BG, VRAM_D_SUB_SPRITE);
	
	// https://mtheall.com/vram.html#T0=2&NT0=1024&MB0=0&TB0=2&S0=3&T1=2&NT1=1024&MB1=4&TB1=4&S1=3&T2=2&NT2=1024&MB2=8&TB2=6&S2=3&T3=2&NT3=1024&MB3=12&TB3=6&S3=3
	int con_fg = bgInit(0, BgType_Text4bpp, BgSize_T_512x512, 0, 2);
	int con_bg = bgInit(1, BgType_Text4bpp, BgSize_T_512x512, 4, 4);
	int bg0 = bgInit(2, BgType_Text4bpp, BgSize_T_512x512, 8, 6);
	int bg1 = bgInit(3, BgType_Text4bpp, BgSize_T_512x512, 12, 6);
	bgSetPriority(con_fg, 0);
	bgSetPriority(con_bg, 3);
	bgSetPriority(bg0, 1);
	bgSetPriority(bg1, 2);
	
	int con_fgl = bgInitSub(0, BgType_Text4bpp, BgSize_T_512x512, 0, 2);
	int con_bgl = bgInitSub(1, BgType_Text4bpp, BgSize_T_512x512, 4, 4);
	int bg0l = bgInitSub(2, BgType_Text4bpp, BgSize_T_512x512, 8, 6);
	int bg1l = bgInitSub(3, BgType_Text4bpp, BgSize_T_512x512, 12, 6);
	bgSetPriority(con_fgl, 0);
	bgSetPriority(con_bgl, 1);
	bgSetPriority(bg0l, 2);
	bgSetPriority(bg1l, 3);
	
	oamInit(&oamMain, SpriteMapping_1D_128, true);
	oamInit(&oamSub, SpriteMapping_1D_128, true);
	
	windowEnable(WINDOW_0);
	windowEnableSub(WINDOW_0);
	bgWindowEnable(bg0l, WINDOW_0);
	bgWindowEnable(bg1l, WINDOW_0);
	bgWindowEnable(bg0, WINDOW_0);
	bgWindowEnable(bg1, WINDOW_0);
	bgWindowEnable(con_fg, WINDOW_0 | WINDOW_OUT);
	bgWindowEnable(con_bg, WINDOW_0 | WINDOW_OUT);
	bgWindowEnable(con_fgl, WINDOW_0 | WINDOW_OUT);
	bgWindowEnable(con_bgl, WINDOW_0 | WINDOW_OUT);
	oamWindowEnable(&oamMain, WINDOW_0 | WINDOW_OUT);
	oamWindowEnable(&oamSub, WINDOW_0 | WINDOW_OUT);
	
	// Uncomment this to make everything half-resolution (not useful but looks cool)
/*	REG_MOSAIC = 0x1111;
	REG_MOSAIC_SUB = 0x1111;
	for (int i = 0; i < 7; ++i){
		bgMosaicEnable(i);
	}*/
	
	// extended palette mode
//	vramSetBankF(VRAM_F_BG_EXT_PALETTE);
//	vramSetBankG(VRAM_G_SPRITE_EXT_PALETTE);
//	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
//	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
}

// Global so that it can be used during interrupt
struct ptc* ptc;

#define KEY(T) if (b & KEY_##T){ conv_b |= BUTTON_##T; }

void frame_update(){
	// scan buttons
	scanKeys();
	// set touch here
	touchPosition touch;
	touchRead(&touch);
	
	// apply inputs
	int b = keysCurrent();
	int conv_b = 0;
	// somewhat silly way to map buttons correctly
	KEY(A)
	KEY(B)
	KEY(X)
	KEY(Y)
	KEY(UP)
	KEY(LEFT)
	KEY(RIGHT)
	KEY(DOWN)
	KEY(SELECT)
	KEY(START)
	KEY(L)
	KEY(R)
	
	set_input(&ptc->input, conv_b);
	
	set_touch(&ptc->input, b & KEY_TOUCH, touch.px, touch.py);
	press_key(ptc, b & KEY_TOUCH, touch.px, touch.py);
	// increment frame count

	step_background(&ptc->background);
	step_sprites(&ptc->sprites);
	inc_time(&ptc->time);
	
	// draw
	display_draw_all(ptc);
}

int main(void){
	init();
	fatInitDefault();
#ifndef NDEBUG
	consoleDemoInit(); // Uses VRAM C but I guess this is fine as a debug tool
#endif
	
	ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT, false);
	
	struct launch_info info = {ptc, NULL, NULL};
	
	// set this after creating system to ensure resources are loaded
	irqSet(IRQ_VBLANK, frame_update);
	
	launch_system(&info);
	
	// make failure conditions readable
	do {
		scanKeys();
	} while (!(keysCurrent() & KEY_TOUCH));
	
	return 0;
}

#endif
#ifdef ARM9
void v__(void); //prevent empty translation unit
#endif

#ifdef PC
//this is the computer-only file

#include <SFML/System.h>
#include <SFML/Graphics.h>
#include <SFML/Window.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "interpreter.h"

#include "program.h"
#include "resources.h"
#include "system.h"
#include "error.h"

#include "extension/sbc_blockalloc.h" // to allow pointers to be limited to 32 bits for compatibility

enum emu_key_mode {
	MODE_KEYBOARD,
	MODE_BUTTON,
	MODE_COUNT,
};

int main(int argc, char** argv){
	init_memory(MAX_MEMORY);
//	struct program program = {0};
	
	char* window_name = "SBC";
	if (argc >= 2){
		// Load .PTC file as program
		window_name = argv[1];
//		load_prg_alloc(&program, argv[1]);
	}
	// Auto-inputs
	FILE* inputs = NULL;
	if (argc >= 3){
		inputs = fopen(argv[2], "r");
	}
	
	struct ptc* ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT, false);
	
	// https://gist.github.com/def-/fee8bb041719337c8812
	// used as convenient reference
	sfRenderWindow* window;
	
	window = sfRenderWindow_create((sfVideoMode){SCREEN_WIDTH, SCREEN_HEIGHT*SCREEN_COUNT, 32}, window_name, sfResize | sfClose, NULL);
	sfRenderWindow_setFramerateLimit(window, FRAMERATE);
	if (!window){
		printf("Failed to create the render window!\n");
		abort();
	}
	ptc->display.rw = window;
	
	// THREAD MODEL
	// WINDOW                    PROGRAM
	//  Event -> Input ==>        BUTTON, KEYBOARD, etc.
	//  Stop -> Load Program =>   struct ptc
	//  Rendering <==             struct ptc ~ display state
	
	// Launch the program thread
	struct launch_info info = {ptc, NULL, argc >= 2 ? argv[1] : NULL};
	thrd_t prog_thread;
	if (thrd_success != thrd_create(&prog_thread, launch_system, &info)){
		printf("Failed to create the program thread!\n");
		abort();
	}
	
	int key_mode = 0;
	int keys[MODE_COUNT][13];
	FILE* file = fopen("resources/config.txt", "r");
	if (!file){
		printf("Failed to read config file!\n");
		abort();
	}
	for (int m = 0; m < MODE_COUNT; ++m){
		for (int i = 0; i < 13; ++i){
			int r = fscanf(file, "%d", &keys[m][i]);
			if (!r || r == EOF){
				printf("Failed to read key %d\n", 1+i);
				abort();
			}
		}
	}
	
	sfEvent event;
	while (sfRenderWindow_isOpen(window)){
		int b = 0; // Button codes (can be modified by some typed sequences)
		while (sfRenderWindow_pollEvent(window, &event)){
			if (event.type == sfEvtClosed){
				sfRenderWindow_close(window);
			}
			
			if (event.type == sfEvtTextEntered){
				if (event.text.unicode <= 128 && key_mode == MODE_KEYBOARD){
					if (!(event.text.unicode == '\b' || event.text.unicode == '\r'))
					{
						set_inkey(&ptc->input, to_wide(event.text.unicode));
					}
				} else if (event.text.unicode >= 12289 && event.text.unicode <= 12540){
					if (to_char(event.text.unicode) >= 0xa1){
						set_inkey(&ptc->input, event.text.unicode - 12289);
					}
				}
			}
		}
		
		char input;
		if (inputs){
			//read 128 units per frame
			bool success = true;
			while(success){
				if ((input = getc(inputs)) != EOF){
					success = set_inkey(&ptc->input, input);
					if (!success){
						ungetc(input, inputs);
					}
				} else {
					fclose(inputs);
					inputs = NULL;
					success = false;
					break;
				}
			}
		}
		
		// various frame updates
		for (int i = 0; i < 12; ++i){
			b |= sfKeyboard_isKeyPressed(keys[key_mode][i]) << i;
		}
		set_input(&ptc->input, b);
		if (check_pressed(&ptc->input, BUTTON_ID_SELECT)){
			// TODO:CODE:MED Move this somewhere else...?
			// TODO:CODE:MED Thread-safety??
			ptc->exec.error = ERR_BREAK;
		}
		
		if (sfKeyboard_isKeyPressed(keys[key_mode][12])){
			// change input mode
			key_mode = (key_mode + 1) % MODE_COUNT;
		}
		
		// TODO:IMPL:LOW Adjust for scaled window
		sfVector2i pos = sfMouse_getPosition((sfWindow*)window);
		if (pos.x >= 0 && pos.x < 256 && pos.y >= 192 && pos.y < 192+192){
			set_touch(&ptc->input, sfMouse_isButtonPressed(0), pos.x, pos.y - 192);
			press_key(ptc, sfMouse_isButtonPressed(0), pos.x, pos.y - 192);
		} else {
//			set_touch(&ptc->input, sfMouse_isButtonPressed(0), pos.x, pos.y - 192);
		}
		step_sprites(&ptc->sprites);
		step_background(&ptc->background);
		inc_time(&ptc->time);
		
		display_draw_all(ptc);
	}
	
	//TODO:CODE:MED Signal thread to die on exit
	sfRenderWindow_destroy(window);
	// tell thread to die before freeing system

	free_system(ptc);
	
	free_memory();
	return 0;
}
#endif

#ifdef EZ80
void ignore();
#endif
