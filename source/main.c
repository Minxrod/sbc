#include <nds.h>

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
}
