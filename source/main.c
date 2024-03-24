#include "common.h"

#include "interpreter.h"

#include "program.h"
#include "resources.h"
#include "system.h"
#include "error.h"

// Common main stuff

/// Information struct containing program source and the system to run it on.
///
/// Used for launching the main thread on PC.
struct launch_info {
	/// Pointer to system struct
	struct ptc* p;
	/// Pointer to program source.
	struct program* prg;
	/// Name of program to autoboot
	char* prg_filename;
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
	struct bytecode bc = info->p->exec.code;
	
	FILE* f = fopen("resources/sbccompat.txt","r");
	if (!f){
		iprintf("Failed to open benchmark tests file!\n");
		return 1;
	}
	FILE* result = fopen("sbccompat_out", "w");
	if (!result){
		iprintf("Failed to open benchmark results file!\n");
		fclose(f);
		return 1;
	}
	int index = 0;
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
		sprintf(src, "%s?%d\r%s%s%s%s%s%n", bench_begin, index, pre, bench_mid, code, bench_end, post, (int*)&p.size);
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
			fprintf(result, "%d\n", perf);
		} else {
			fprintf(result, "%s\n", error_messages[info->p->exec.error]);
		}
		++index;
	}
	fclose(f);
	fclose(result);
//	free_bytecode(bc);
	return 0;
}

// This is the DIRECT mode prompt.
// Pretty basic, but it works.
// TODO:IMPL:LOW Redesign to remove prompt symbol
struct program launcher = {
	13, "LINPUT CODE$\r"
};

enum launch_state {
	LAUNCH_PROMPT,
	LAUNCH_EDIT,
	LAUNCH_RUN,
	LAUNCH_BENCH,
	LAUNCH_AUTOLOAD,
};

int system_launch(void* launch_info){
	u8 err_msg[66] = {STRING_INLINE_CHAR}; // 2 lines + metadata
	struct launch_info* info = (struct launch_info*)launch_info;
	struct ptc* p = info->p;
	
	// DIRECT setup
	struct bytecode prompt_bc = init_bytecode_size(launcher.size, 1, 0); // TODO:CODE:NONE This isn't guaranteed to be safe, but I'm also not sure if the program can be made easily dangerous
	tokenize_full(&launcher, &prompt_bc, p, TOKOPT_NONE);
	
	// editor setup
	// TODO:PERF:MED Reduce memory fragmentation by loading program into bytecode memory...?
	struct program editor = {0};
	load_prg_alloc(&editor, "programs/EDITOR.PTC"); // TODO:CODE:MED move EDITOR to resources
	struct bytecode editor_bc = init_bytecode_size(editor.size, 500, 64); // TODO:CODE:NONE This isn't guaranteed to be safe, but I'm also not sure if the program can be made easily dangerous
	tokenize_full(&editor, &editor_bc, p, TOKOPT_VARIABLE_IDS);
	free_log("editor temp load", editor.data);
	
	// for user programs / "RUN" setup
	struct bytecode bc = init_bytecode_size(MAX_SOURCE_SIZE, MAX_LINES, MAX_LABELS);
	if (!bc.labels.entry || !bc.data){
		iprintf("Failed to allocate bytecode...\n");
		return -1;
	}
	init_mem_prg(&p->exec.prg, MAX_SOURCE_SIZE);
	if (!p->exec.prg.data){
		iprintf("Failed to allocate program...\n");
		return -1;
	}
	
	// Setup complete
	con_puts(&p->console, "S\45Small BASIC Computer            READY");
	con_newline(&p->console, true);
	bool running = true;
	
	int state = LAUNCH_PROMPT;
	if (info->prg_filename){
		state = LAUNCH_AUTOLOAD;
	}
	
	// TODO:IMPL:LOW Add configuration method for optimizations
	int opts = TOKOPT_VARIABLE_IDS;
	while (running){
		p->exec.error = ERR_NONE; // prepare for next execution
		p->calls.stack_i = 0; // TODO:IMPL:MED Determine a better way to handle this
		int old_state = state;
		iprintf("MAIN state=%d\n", state);
		switch (state){
			case LAUNCH_PROMPT: // DIRECT mode
				{
				run(prompt_bc, p); // get prompt
				
				void* cmd = get_var(&p->vars, "CODE", 4, VAR_STRING)->value.ptr;
				if (str_comp(cmd, "S\011REM BENCH")){
					state = LAUNCH_BENCH;
					break;
				} else if (str_comp(cmd, "S\3RUN")){
					state = LAUNCH_RUN;
					break;
				}
				// check status
				if (p->exec.error == ERR_BUTTON_SIGNAL){
					int sig = p->exec.error_info[0];
					if (sig == 72){ // EDIT
						state = LAUNCH_EDIT;
					}
					p->exec.error = ERR_NONE; // clear signal
					p->exec.error_info[0] = 0; // clear signal info
					break;
				}
				
				// execute single line
				// create small program
				u8 direct_cmd[32+1] = {0};
				struct program prog = {0, (char*)direct_cmd};
				str_char_copy(cmd, direct_cmd);
				prog.size = str_len(cmd);
				direct_cmd[prog.size++] = '\r';
				
				// prepare execution
				p->exec.error = ERR_NONE;
				p->calls.stack_i = 0; // TODO:IMPL:MED Determine a better way to handle this
				p->exec.error = tokenize_full(&prog, &bc, p, TOKOPT_NONE);
				if (p->exec.error == ERR_NONE){
					run(bc, p);
				}
				}
				break;
				
			case LAUNCH_EDIT: // EDIT mode
				p->exec.error = ERR_NONE;
				get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(0);
				run(editor_bc, p);
				int sig = p->exec.error_info[0];
				if (p->exec.error == ERR_BUTTON_SIGNAL){
					if (sig == 71){ // RUN/DIRECT
						state = LAUNCH_PROMPT;
						// copy program to memory
						p->exec.error = ERR_NONE;
						p->exec.error_info[0] = 0; // clear signal info
						get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(3);
						run(editor_bc, p);
						// reset screen
						get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(2);
						run(editor_bc, p);
					}
				} else if (p->exec.error == ERR_USER_SIGNAL){
					if (sig == 1){ // run program
						state = LAUNCH_RUN;
						p->exec.error = ERR_NONE;
						p->exec.error_info[0] = 0; // clear signal info
						// copy program to memory
						get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(3);
						run(editor_bc, p);
						// reset screen
						get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(2);
						run(editor_bc, p);
					}
				} else {
					state = LAUNCH_PROMPT;
					// crash to console to allow reading error message
				}
				// set this to allow EDITOR to be correctly started via LOAD/RUN or EXEC, if wanted
				get_var(&p->vars, "EDITOR_MAGIC", 12, VAR_NUMBER)->value.number = INT_TO_FP(1);
				break;
				
			case LAUNCH_RUN: // 'RUN' mode
				p->exec.error = tokenize_full(&p->exec.prg, &bc, p, opts);
				if (p->exec.error == ERR_NONE){
					run(bc, p);
				}
				state = LAUNCH_PROMPT; // program terminated: return to DIRECT mode
				break;
				
			case LAUNCH_BENCH: // benchmark tests
				p->exec.code = bc; // ensure it has correct resource
				sbc_benchmark(launch_info);
				state = LAUNCH_PROMPT; // return to DIRECT when done
				break;
				
			case LAUNCH_AUTOLOAD: // auto load program
				load_prg(&p->exec.prg, info->prg_filename);
				state = LAUNCH_RUN;
				break;
				
			default:
				return p->exec.error;
		}
		
		(void)old_state;
		// Display error status (for any given execution)
		iprintf("Error: %s\n", error_messages[p->exec.error]);
		
		strcpy((char*)err_msg + 2, error_messages[p->exec.error]);
		err_msg[1] = strlen(error_messages[p->exec.error]);
		strcpy((char*)err_msg + 2 + err_msg[1], p->exec.error_info);
		err_msg[1] += strlen(p->exec.error_info);
		
		con_puts(&p->console, &err_msg);
		con_newline(&p->console, true);
		// clear error status for next execution
		p->exec.error = ERR_NONE;
		p->exec.error_info[0] = '\0'; // remove error string
	}
	
	free_log("free exec.prg.data", p->exec.prg.data);
	free_bytecode(bc);
	return p->exec.error;
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
	
	ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT);
	
	struct launch_info info = {ptc, NULL, NULL};
	
	// set this after creating system to ensure resources are loaded
	irqSet(IRQ_VBLANK, frame_update);
	
	system_launch(&info);
	
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
	struct launch_info info = {ptc, NULL, argc >= 2 ? argv[1] : NULL};
	thrd_t prog_thread;
	if (thrd_success != thrd_create(&prog_thread, system_launch, &info)){
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
	
	free_system(ptc);
	
	free_memory();
	return 0;
}
#endif

#ifdef EZ80
void ignore();
#endif
