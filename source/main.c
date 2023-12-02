#include "common.h"

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
	
	// extended palette mode
//	vramSetBankF(VRAM_F_BG_EXT_PALETTE);
//	vramSetBankG(VRAM_G_SPRITE_EXT_PALETTE);
//	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
//	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
}

// Global so that it can be used during interrupt
struct ptc* ptc;

void frame_update(){
	scanKeys();
	// set buttons here
	keysCurrent();
	
	set_input(&ptc->input, keysCurrent());
	// draw
	system_draw(ptc);
}

int main(void){
	init();
	fatInitDefault();
	consoleDemoInit(); // Uses VRAM C but I guess this is fine as a debug tool
//	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_512x512, 0, 2, false, true); 
	struct program program;
	ptc = init_system(VAR_LIMIT, STR_LIMIT, ARR_LIMIT);
	// set this after creating system to ensure resources are loaded
	
	prg_load(&program, "SAMPLE6.PTC");
	iprintf("program malloc: %d\n", 2*program.size);
	struct program bc = {0, malloc(2*program.size)};
	tokenize(&program, &bc);
//	free(program.data);
	irqSet(IRQ_VBLANK, frame_update);
	// only needs BC, not source
	run(&bc, ptc);
	
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
#include <threads.h>

#include "interpreter.h"

#include "program.h"
#include "resources.h"
#include "system.h"
#include "error.h"

struct launch_info {
	struct ptc* p;
	struct program* prg;
};

int system_launch(void* launch_info){
	struct launch_info* info = (struct launch_info*)launch_info;
	
	// TODO:IMPL:MED Check for escapes, pauses, etc.
	struct program bc = {0, malloc(2*info->prg->size)};
	tokenize(info->prg, &bc);
	free(info->prg->data);
	// only needs BC, not source
	run(&bc, info->p);
	
	if (info->p->exec.error){
		iprintf("Error: %s\n", error_messages[info->p->exec.error]);
		con_puts(&info->p->console, "S\5Error");
		con_newline(&info->p->console, true);
	} else {
		con_puts(&info->p->console, "S\2OK");
		con_newline(&info->p->console, true);
	}
	
	return info->p->exec.error;
}

int main(int argc, char** argv){
//	srand(time(NULL));
	struct program program;
	
	char* window_name = "SBC";
	if (argc >= 2){
		// Load .PTC file as program
		window_name = argv[1];
		prg_load(&program, argv[1]);
	} else {
		// Load default program: TODO:IMPL:LOW load an actual launcher program
		prg_load(&program, "programs/SAMPLE2.PTC");
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
	}
	
	sfEvent event;
	while (sfRenderWindow_isOpen(window)){
		while (sfRenderWindow_pollEvent(window, &event)){
			if (event.type == sfEvtClosed){
				sfRenderWindow_close(window);
			}
			
			if (event.type == sfEvtTextEntered){
				if (event.text.unicode <= 128){
					set_inkey(&ptc->input, to_wide(event.text.unicode));
				} else if (event.text.unicode >= 12289 && event.text.unicode <= 12540){
					//match = {12290: '¡', 12296: '¢', 12297: '£', 12289: '¤', 12539: '¥', 12530: '¦', 12449: '§', 12451: '¨', 12453: '©', 12455: 'ª', 12457: '«', 12515: '¬', 12517: '\xad', 12519: '®', 12483: '¯', 12540: '°', 12450: '±', 12452: '²', 12454: '³', 12456: '´', 12458: 'µ', 12459: '¶', 12461: '·', 12463: '¸', 12465: '¹', 12467: 'º', 12469: '»', 12471: '¼', 12473: '½', 12475: '¾', 12477: '¿', 12479: 'À', 12481: 'Á', 12484: 'Â', 12486: 'Ã', 12488: 'Ä', 12490: 'Å', 12491: 'Æ', 12492: 'Ç', 12493: 'È', 12494: 'É', 12495: 'Ê', 12498: 'Ë', 12501: 'Ì', 12504: 'Í', 12507: 'Î', 12510: 'Ï', 12511: 'Ð', 12512: 'Ñ', 12513: 'Ò', 12514: 'Ó', 12516: 'Ô', 12518: 'Õ', 12520: 'Ö', 12521: '×', 12522: 'Ø', 12523: 'Ù', 12524: 'Ú', 12525: 'Û', 12527: 'Ü', 12531: 'Ý', 12443: 'Þ', 12444: 'ß'}
					//"".join([match[x + min(match)] if x + min(match) in match else '?' for x in range(0,256)])
//					u8* text_to_key = (u8*)"¤¡?????¢£?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????Þß????§±¨²©³ª´«µ¶?·?¸?¹?º?»?¼?½?¾?¿?À?Á?¯Â?Ã?Ä?ÅÆÇÈÉÊ??Ë??Ì??Í??Î??ÏÐÑÒÓ¬Ô\xadÕ®Ö×ØÙÚÛ?Ü??¦Ý???????¥°????";
					//printf("%04x,%02x\n", event.text.unicode, text_to_key[event.text.unicode - 12289]);
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
		
		// TODO:CODE:NONE SFML 2.6, use scan codes?
		int b = 0;
		for (int i = 0; i < 12; ++i){
			b |= sfKeyboard_isKeyPressed(keys[i]) << i;
		}
		set_input(&ptc->input, b);
		
		sfVector2i pos = sfMouse_getPosition((sfWindow*)window);
		set_touch(&ptc->input, sfMouse_isButtonPressed(0), pos.x, pos.y);
		
		sfRenderWindow_clear(window, sfBlack);
		
		system_draw(window, ptc);
		
		sfRenderWindow_display(window);
		inc_time(&ptc->time);
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
