#include "common.h"

#include "interpreter.h"

#include "program.h"
#include "resources.h"
#include "system.h"
#include "error.h"

// Common main stuff

struct launch_info {
	struct ptc* p;
	struct program* prg;
};

struct program launcher = {
	13, "LINPUT CODE$\r"
};


const char* bench_begin = "ACLS:CLEAR\r";
const char* bench_mid = "FOR J=1 TO 5\r"
"\r"
"M=POW(10,J)\r"
"\r"
"T=MAINCNTL\r"
"FOR I=1 TO M\r";
const char* bench_end = "NEXT\r"
"S=MAINCNTL-T\r"
"\r"
"?M,S\r"
"IF S>=1000 THEN @END\r"
"NEXT\r"
"@END\r";

int sbc_benchmark(struct launch_info* info){
	char src[1024] = {0};
	char pre[100];
	char code[100];
	char post[100];
	struct program p = { 0, src };
	struct bytecode bc = init_bytecode();
	
	FILE* f = fopen("resources/sbccompat.csv","r"); // Note: This is not a csv file actually
	FILE* result = fopen("sbccompat_out", "w");
	while (!feof(f)){
		fgets(pre, 100, f);
		fgets(code, 100, f);
		fgets(post, 100, f);
		for (int i = 0; i < 100; ++i){
			if (pre[i] == '\\') pre[i] = '\r';
			if (code[i] == '\\') code[i] = '\r';
			if (post[i] == '\\') post[i] = '\r';
			if (pre[i] == '\n') pre[i] = '\r';
			if (code[i] == '\n') code[i] = '\r';
			if (post[i] == '\n') post[i] = '\r';
		}
		sprintf(src, "%s%s%s%s%s%s%n", bench_begin, pre, bench_mid, code, bench_end, post, (int*)&p.size);
//		fprintf(f2, "%s", src);
		info->p->exec.error = ERR_NONE; // reset for next run
		info->p->exec.error = tokenize_full(&p, &bc, info->p, TOKOPT_VARIABLE_IDS);
		if (info->p->exec.error == ERR_NONE){
			run(bc, info->p);
		}
		
		if (info->p->exec.error == ERR_NONE){
			fixp num = test_var(&info->p->vars, "S", VAR_NUMBER)->value.number;
			fixp lev = test_var(&info->p->vars, "J", VAR_NUMBER)->value.number;
			int perf = FP_TO_INT(num);
			for (int i = FP_TO_INT(lev); i < 5; ++i){
				perf *= 10;
			}
			fprintf(result, "%s %d\n", code, perf);
		} else {
			fprintf(result, "%s %s\n", code, error_messages[info->p->exec.error]);
		}
	}
	fclose(f);
	fclose(result);
	free_bytecode(bc);
	return 0;
}

int system_launch(void* launch_info){
	struct launch_info* info = (struct launch_info*)launch_info;
	struct bytecode bc = init_bytecode();
	
//	sbc_benchmark(launch_info);
	
	if (info->prg->size){
		info->p->exec.error = tokenize_full(info->prg, &bc, info->p, TOKOPT_VARIABLE_IDS);
		if (info->p->exec.error == ERR_NONE){
			run(bc, info->p);
		}
		
		// Display error status
		iprintf("Error: %s\n", error_messages[info->p->exec.error]);
		// TODO:CODE:HIGH This includes a const cast away. What's a good way to avoid this?
		const struct string err_status = {
			STRING_CHAR, strlen(error_messages[info->p->exec.error]), 0,
			{.s = (u8*)error_messages[info->p->exec.error]}
		};
		con_puts(&info->p->console, &err_status);
		con_newline(&info->p->console, true);
		return info->p->exec.error;
	}
	
	u8 direct_cmd[32+1] = {0};
	struct program prog = {0, (char*)direct_cmd};
	con_puts(&info->p->console, "S\45Small BASIC Computer            READY");
	con_newline(&info->p->console, true);
	bool running = true;
	while (running){
		// Prompt for command
		// TODO:IMPL:LOW Redesign to remove prompt symbol
		info->p->exec.error = ERR_NONE;
		tokenize(&launcher, &bc);
		run(bc, info->p);
		
		// Get program name from output
		void* cmd = get_var(&info->p->vars, "CODE", 4, VAR_STRING)->value.ptr;
		str_char_copy(cmd, direct_cmd);
		prog.size = str_len(cmd);
		direct_cmd[prog.size++] = '\r';
		
		// Execute small program
		info->p->exec.error = ERR_NONE;
		info->p->exec.error = tokenize(&prog, &bc);
		if (info->p->exec.error == ERR_NONE){
			run(bc, info->p);
		}
		
		// Display error status
		iprintf("Error: %s\n", error_messages[info->p->exec.error]);
		// TODO:CODE:HIGH This includes a const cast away. What's a good way to avoid this?
		const struct string err_status = {
			STRING_CHAR, strlen(error_messages[info->p->exec.error]), 0,
			{.s = (u8*)error_messages[info->p->exec.error]}
		};
		con_puts(&info->p->console, &err_status);
		con_newline(&info->p->console, true);
	}
	
//	info->p->exec.error = tokenize(info->prg, &bc);
//	free_log("system_launch", info->prg->data);
	// only needs BC, not source
	
	free_bytecode(bc);
	return info->p->exec.error;
}


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
	bgInit(0, BgType_Text4bpp, BgSize_T_512x512, 0, 2);
	bgInit(1, BgType_Text4bpp, BgSize_T_512x512, 4, 4);
	bgInit(2, BgType_Text4bpp, BgSize_T_512x512, 8, 6);
	bgInit(3, BgType_Text4bpp, BgSize_T_512x512, 12, 6);
	
	bgInitSub(0, BgType_Text4bpp, BgSize_T_512x512, 0, 2);
	bgInitSub(1, BgType_Text4bpp, BgSize_T_512x512, 4, 4);
	bgInitSub(2, BgType_Text4bpp, BgSize_T_512x512, 8, 6);
	bgInitSub(3, BgType_Text4bpp, BgSize_T_512x512, 12, 6);
	
	oamInit(&oamMain, SpriteMapping_1D_128, true);
	oamInit(&oamSub, SpriteMapping_1D_128, true);
	
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
	
	struct program program = {0};
	ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT);
	
	struct launch_info info = {ptc, &program};
	
	// set this after creating system to ensure resources are loaded
	
//	prg_load(&program, "programs/PERFTS2.PTC");
//	struct bytecode bc = init_bytecode(program.size);
//	tokenize(&program, &bc);
//	free_log("main", program.data);
	irqSet(IRQ_VBLANK, frame_update);
	// only needs BC, not source
	system_launch(&info);
//	run(bc, ptc);
	
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


int main(int argc, char** argv){
//	srand(time(NULL));
	struct program program = {0};
	
	char* window_name = "SBC";
	if (argc >= 2){
		// Load .PTC file as program
		window_name = argv[1];
		prg_load(&program, argv[1]);
	} else {
		// Load default program: TODO:IMPL:LOW load an actual launcher program
//		prg_load(&program, "programs/SAMPLE7.PTC");
	}
	// Auto-inputs
	FILE* inputs = NULL;
	if (argc >= 3){
		inputs = fopen(argv[2], "r");
	}
	
	struct ptc* ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT);
	
	// https://gist.github.com/def-/fee8bb041719337c8812
	// used as convenient reference
	sfRenderWindow* window;
	
	window = sfRenderWindow_create((sfVideoMode){256,384,32}, window_name, sfResize | sfClose, NULL);
	sfRenderWindow_setFramerateLimit(window, 60);
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
	}
	
	sfEvent event;
	while (sfRenderWindow_isOpen(window)){
		int b = 0; // Button codes (can be modified by some typed sequences)
		while (sfRenderWindow_pollEvent(window, &event)){
			if (event.type == sfEvtClosed){
				sfRenderWindow_close(window);
			}
			
			if (event.type == sfEvtTextEntered){
				if (event.text.unicode <= 128){
					// TODO:CODE:LOW Check these as additional button presses instead?
					if (event.text.unicode == '\b'){
						b |= BUTTON_Y;
					} else if (event.text.unicode == '\r'){
						b |= BUTTON_A;
					} else {
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
			b |= sfKeyboard_isKeyPressed(keys[i]) << i;
		}
		set_input(&ptc->input, b);
		if (check_pressed(&ptc->input, BUTTON_ID_SELECT)){
			// TODO:CODE:MED Move this somewhere else...?
			// TODO:CODE:MED Thread-safety??
			ptc->exec.error = ERR_BREAK;
		}
		
		sfVector2i pos = sfMouse_getPosition((sfWindow*)window);
		if (pos.x >= 0 && pos.x < 256 && pos.y >= 192 && pos.y < 192+192){
			set_touch(&ptc->input, sfMouse_isButtonPressed(0), pos.x, pos.y - 192);
			press_key(ptc, sfMouse_isButtonPressed(0), pos.x, pos.y - 192);
		}
		step_sprites(&ptc->sprites);
		step_background(&ptc->background);
		inc_time(&ptc->time);
		
		display_draw_all(ptc);
	}
	
	//TODO:CODE:MED Signal thread to die on exit
	sfRenderWindow_destroy(window);
	
	free_system(ptc);
	
	return 0;
}
#endif

#ifdef EZ80
void ignore();
#endif
