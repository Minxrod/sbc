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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL2/SDL.h>
#endif

#include "extension/sbc_blockalloc.h" // to allow pointers to be limited to 32 bits for compatibility

enum emu_key_mode {
	MODE_KEYBOARD,
	MODE_BUTTON,
	MODE_COUNT,
};

// UDLR, ABXY, LR SELECT START, and a special 'toggle mode' key
#define KEY_COUNT 13

/// System struct!
struct sbc* ptc;

/// Program executor thread
/// runs separately at full speed while display runs once each frame
thrd_t prog_thread;
/// Launcher info for executor thread
/// This has to have a lifetime lasts for the thread to use it, so
/// I just made it global here
struct launch_info info;

// THREAD MODEL
// WINDOW                    PROGRAM
//  Event -> Input ==>        BUTTON, KEYBOARD, etc.
//  Stop -> Load Program =>   struct sbc
//  Rendering <==             struct sbc ~ display state


/// Button mappings
int key_mode = 0;
int keys[MODE_COUNT][KEY_COUNT];

void init(int argc, char** argv){
	init_memory(MAX_MEMORY);
	if (0 > SDL_Init( SDL_INIT_VIDEO )){
		ABORT("Failed to initialze SDL");
	}

	ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT, false);

	init_gl(&ptc->display);
	// TODO:CODE:LOW ordering is dumb how can I fix this
	init_display_2(ptc); // has to be after because GL context needs to exist first...

	// Launch the program thread
	info = (struct launch_info){ptc, NULL, argc >= 2 ? argv[1] : NULL};
	if (thrd_success != thrd_create(&prog_thread, launch_system, &info)){
		ABORT("Failed to create the program thread!\n");
	}

	FILE* file = fopen("resources/config.txt", "r");
	if (!file){
		ABORT("Failed to read config file!");
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
}

_Noreturn void deinit(void){
	// Done with GUI
	SDL_DestroyWindow(ptc->display.window);

	// this causes program to stop execution after instruction finishes
	ptc->exec.error = ERR_SHUTDOWN;
	iprintf("Waiting for program thread to terminate\n");
	thrd_join(prog_thread, NULL);

	free_system(ptc);

	free_memory();

	exit(0);
}

// Split off from main due to emscripten
void main_loop(void){
	if (ptc->exec.error == ERR_SHUTDOWN){
		deinit();
	}

	SDL_Event event;
	int b = 0; // Button codes (can be modified by some typed sequences)
	while (SDL_PollEvent(&event)){
		if (event.type == SDL_QUIT){
			deinit();  // does not return
		} else if (event.type == SDL_TEXTINPUT){
			s32 unicode = event.text.text[0];
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

	if (keyboard[keys[key_mode][12]]){
		// change input mode
		key_mode = (key_mode + 1) % MODE_COUNT;
	}

	int x;
	int y;
	int mouse_buttons = SDL_GetMouseState(&x, &y);
//		sfVector2i pos = sfMouse_getPosition((sfWindow*)window);
//		sfVector2u window_size = sfRenderWindow_getSize(window);
	if (y >= SCREEN_HEIGHT){
		// TODO:IMPL:LOW this code supposes the panel size matches the screen size. How to decouple this?
		// TODO:CODE:LOW detecting what screen is used could be better.
		// Need a mapping from window coordinates into touch coordinates.
		int tchx = SCREEN_WIDTH * x / SCREEN_WIDTH;
		int tchy = SCREEN_HEIGHT * y / SCREEN_HEIGHT - SCREEN_HEIGHT * (SCREEN_COUNT - 1);
		if (tchx < SCREEN_WIDTH && tchx >= 0 && tchy < SCREEN_HEIGHT && tchy >= 0){
//			iprintf("%d,%d\n", tchx, tchy);
			set_touch(&ptc->input, mouse_buttons & SDL_BUTTON(1), tchx, tchy);
			press_key(ptc, mouse_buttons & SDL_BUTTON(1), tchx, tchy);
		}
	}

	step_sprites(&ptc->sprites);
	step_background(&ptc->background);
	inc_time(&ptc->time);

	display_draw_all(ptc);
}

// structure more of less became this:
// https://wiki.libsdl.org/SDL2/README/emscripten
// for reasons described on that page
int main(int argc, char** argv){
	init(argc, argv);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
	while (true){
		main_loop();
	}
#endif
}
