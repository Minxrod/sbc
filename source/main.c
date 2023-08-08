#ifdef ARM9
void v(void); //prevent empty translation unit
#endif

#ifndef ARM9
//this is the computer-only testing file

#include "tokens.h"
#include "runner.h"
#include "system.h"
#include "program.h"

#include <stdio.h>

char* code = "B=5\r"
"C=8\r"
"A=B+C\r"
"?A,B,C\r"
"B$=\"Abc\"+\"deF\"\r"
"?B$,\"1234\"\r";

char out[4096];

int main(void){
	// initialize system buffers, etc.
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
	
	return 0;
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
