#include "common.h"

#include "interpreter.h"

#include "program.h"
#include "resources.h"
#include "sdl2/display_sdl.h"
#include "system.h"
#include "error.h"
#include "input.h"

#include <SDL2/SDL_keyboard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#ifdef __linux
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#endif

#include "extension/sbc_blockalloc.h" // to allow pointers to be limited to 32 bits for compatibility

enum emu_key_mode {
	MODE_KEYBOARD,
	MODE_BUTTON,
	MODE_COUNT,
};

// UDLR, ABXY, LR SELECT START, and a special 'toggle mode' key
#define KEY_COUNT 13

#define SDL_ERROR(msg)\
do {\
	iprintf(msg": %s\n", SDL_GetError());\
	return 1;\
} while(0);

int main(int argc, char** argv){
	init_memory(MAX_MEMORY);
	if (0 > SDL_Init( SDL_INIT_VIDEO )){
		SDL_ERROR("Failed to initialze SDL");
	}

/*	char* window_name = "SBC";
	if (argc >= 2){
		// Load .PTC file as program
		window_name = argv[1];
	}*/

	struct sbc* ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT, false);

	init_gl(&ptc->display);
	init_display_2(ptc); // has to be after because GL context needs to exist first...
	// TODO:CODE:LOW ordering is dumb how can I fix this

	// THREAD MODEL
	// WINDOW                    PROGRAM
	//  Event -> Input ==>        BUTTON, KEYBOARD, etc.
	//  Stop -> Load Program =>   struct sbc
	//  Rendering <==             struct sbc ~ display state

	// Launch the program thread
	struct launch_info info = {ptc, NULL, argc >= 2 ? argv[1] : NULL};
	thrd_t prog_thread;
	if (thrd_success != thrd_create(&prog_thread, launch_system, &info)){
		printf("Failed to create the program thread!\n");
		abort();
	}

	int key_mode = 0;
	int keys[MODE_COUNT][KEY_COUNT];
	FILE* file = fopen("resources/config.txt", "r");
	if (!file){
		printf("Failed to read config file!\n");
		abort();
	}
	for (int m = 0; m < MODE_COUNT; ++m){
		for (int i = 0; i < KEY_COUNT; ++i){
			int r = fscanf(file, "%d", &keys[m][i]);
			if (!r || r == EOF){
				printf("Failed to read key %d\n", 1+i);
				abort();
			}
		}
	}

	// TODO:CODE:LOW split this into an update function (one frame) etc.
	// For emscripten.
	SDL_Event event;
	bool running = true;
	while (running && ptc->exec.error != ERR_SHUTDOWN){
		int b = 0; // Button codes (can be modified by some typed sequences)
		while (SDL_PollEvent(&event)){
			if (event.type == SDL_QUIT){
				running = false;
			} else if (event.type == SDL_KEYDOWN){
				s32 unicode = event.key.keysym.sym;
				if (unicode < 0x10000 && is_char(unicode) && !(unicode == '\b' || unicode == '\r' || unicode == '\x1b')){
					if (unicode < 128){
						set_inkey(&ptc->input, to_wide(unicode));
					} else {
						set_inkey(&ptc->input, unicode);
					}
				}
			}
		}

		// various frame updates
		const u8* keyboard = SDL_GetKeyboardState(NULL);
		for (int i = 0; i < BUTTON_COUNT; ++i){
			if (keyboard[keys[key_mode][i]]){
				b |= 1 << i;
			}
		}
		set_input(ptc, b);

		if (false){// && sfKeyboard_isKeyPressed(keys[key_mode][12])){
			// change input mode
			key_mode = (key_mode + 1) % MODE_COUNT;
		}

//		sfVector2i pos = sfMouse_getPosition((sfWindow*)window);
//		sfVector2u window_size = sfRenderWindow_getSize(window);
/*		if (pos.y >= (int)window_size.y / 2){
			// TODO:IMPL:LOW this code supposes the panel size matches the screen size. How to decouple this?
			// TODO:CODE:LOW detecting what screen is used could be better.
			// Need a mapping from window coordinates into touch coordinates.
			int tchx = SCREEN_WIDTH * pos.x / window_size.x;
			int tchy = SCREEN_HEIGHT * pos.y / (window_size.y / SCREEN_COUNT) - SCREEN_HEIGHT * (SCREEN_COUNT - 1);
			if (tchx < SCREEN_WIDTH && tchx >= 0 && tchy < SCREEN_HEIGHT && tchy >= 0){
	//			iprintf("%d,%d\n", tchx, tchy);
				set_touch(&ptc->input, sfMouse_isButtonPressed(0), tchx, tchy);
				press_key(ptc, sfMouse_isButtonPressed(0), tchx, tchy);
			}
		}*/

		step_sprites(&ptc->sprites);
		step_background(&ptc->background);
		inc_time(&ptc->time);

		display_draw_all(ptc);
	}

	// Done with GUI
	SDL_DestroyWindow(ptc->display.window);

	// this causes program to stop execution after instruction finishes
	ptc->exec.error = ERR_SHUTDOWN;
	iprintf("Waiting for program thread to terminate\n");
	thrd_join(prog_thread, NULL);

	free_system(ptc);

	free_memory();
	return 0;
}
