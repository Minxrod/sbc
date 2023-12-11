#include "panel.h"

#include "common.h"
#include "error.h"
#include "console.h"
#include "system.h"

#include <string.h>

void init_panel(struct ptc* p){
//	struct panel* panel = calloc(sizeof(struct panel), 1);
	p->panel.type = PNL_KYA;
	p->panel.text = init_console();
	set_keyboard(p, p->panel.type);
}

void free_panel(struct ptc* p){
	free_console(p->panel.text);
}

void cmd_pnltype(struct ptc* p){
	void* type = value_str(ARG(0));
	
	if (str_comp(type, "S\3OFF")){
		p->panel.type = PNL_OFF;
	} else if (str_comp(type, "S\3PNL")){
		p->panel.type = PNL_PNL;
	} else if (str_comp(type, "S\3KYA")){
		p->panel.type = PNL_KYA;
	} else if (str_comp(type, "S\3KYM")){
		p->panel.type = PNL_KYM;
	} else if (str_comp(type, "S\3KYK")){
		p->panel.type = PNL_KYK;
	} else {
		ERROR(ERR_INVALID_ARGUMENT_VALUE);
	}
}

void cmd_pnlstr(struct ptc* p){
	(void)p;
}


// Actual characters associated to the sprites
char* keyboard_chr[6]={
	"\0\0001234567890-+=\0$\"QWERTYUIOP@*()\t!ASDFGHJKL;:<>\0'ZXCVBNM,./%\r\0\0\0\0 \0\0\0", //kya
	"\0\0\0\0#\0\0&\0^\\~\0\x7f|\0\0\0qwertyuiop`\0[]\t\0asdfghjkl\0\0{}\0\0zxcvbnm\0\0?_\r\0\0\0\0 \0\0\0", //shift_kya
	"\0\0\xF1\xF2\xF0\xF3\x0A\xE5\x1A\xE4\xE6\xE7\x10\x07\x14\0\x03\x96\x98\x91\x99\xEC\xEE\xEF\xED\x0E\x0F\x05\x06"
	"\x04\x0B\x1B\t\x95\x92\x93\x94\x15\x17\x16\xF4\xF6\xF7\xF5\xEB\xEA\xE8\0\xFF\x9A\x90\x9B\xFC\xFD\xFE\x97\x9C\x9D\x9E\x9F\r\0\0\0\0 \0\0\0", //kigou
	"\0\0\0\x80\0\0\0\xE1\0\xE0\xE2\xE3\x11\x08\0\0\0\0\xF8\x1E\xFA\x01\x02\x18\x19\x0C\x12\x13\0\0\xE9\0\t"
	"\0\x1D\0\x1C\0\x81\x82\x83\x84\x85\x86\x87\0\0\0\0\xF9\x1F\xFB\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F\r\0\0\0\0 \0\0\0", //shift_kigou
	"\0\0\xB1\xB2\xB3\xB4\xB5\xC5\xC6\xC7\xC8\xC9\x2D\x2B\x3D\0\xA0\xA5\xB6\xB7\xB8\xB9\xBA\xCA\xCB\xCC\xCD\xCE\xA2"
	"\xA3\xDD\xA1\t\xBB\xBC\xBD\xBE\xBF\xCF\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xA4\0\xC0\xC1\xC2\xC3\xC4\xD7\xD8\xD9\xDA\xDB\xDC\xA6\r\0\0\0\0 \0\0\0", //kana
//note: shift_kana uses up to two characters (null-terminated)
	"\0 \0 \xA7\0\xA8\0\xA9\0\xAA\0\xAB\0\0 \0 \0 \0 \0 \xCA\xDF\xCB\xDF\xCC\xDF\0 "
	"\xAF\0\xB3\xDE\xB6\xDE\xB7\xDE\xB8\xDE\xB9\xDE\xBA\xDE\xCA\xDE\xCB\xDE\xCC\xDE\xCD\xDE\xCE\xDE\xCD\xDF\xCE\xDF\0 \0 "
	"\t\0\xBB\xDE\xBC\xDE\xBD\xDE\xBE\xDE\xBF\xDE\x31\0\x32\0\x33\0\x34\0\x35\0\xAC\0\xAD\0\xAE\0\0 "
	"\0 \xC0\xDE\xC1\xDE\xC2\xDE\xC3\xDE\xC4\xDE\x36\0\x37\0\x38\0\x39\0\x30\0\0 \0 \r\0\0 \0 \0 \0  \0\0 \0 \0 " //shift_kana
};

// x,y,w,h,chr
u16 keyboard_pos[][5]={
// The primary keyboard keys [1-68]
// First row
	{0, 48, 32, 32, 280}, // Escape
	{24, 48, 16, 32, 384},
	{40, 48, 16, 32, 386},
	{56, 48, 16, 32, 388},
	{72, 48, 16, 32, 390},
	{88, 48, 16, 32, 392},
	{104, 48, 16, 32, 394},
	{120, 48, 16, 32, 396},
	{136, 48, 16, 32, 398},
	{152, 48, 16, 32, 400},
	{168, 48, 16, 32, 402},
	{184, 48, 16, 32, 404},
	{200, 48, 16, 32, 406},
	{216, 48, 16, 32, 408},
	{232, 48, 32, 32, 276}, // Backspace
// Second row
	{0, 72, 16, 32, 410},
	{16, 72, 16, 32, 412},
	{32, 72, 16, 32, 414},
	{48, 72, 16, 32, 416},
	{64, 72, 16, 32, 418},
	{80, 72, 16, 32, 420},
	{96, 72, 16, 32, 422},
	{112, 72, 16, 32, 424},
	{128, 72, 16, 32, 426},
	{144, 72, 16, 32, 428},
	{160, 72, 16, 32, 430},
	{176, 72, 16, 32, 432},
	{192, 72, 16, 32, 434},
	{208, 72, 16, 32, 436},
	{224, 72, 16, 32, 438},
	{240, 72, 16, 32, 440},
// Third row
	{0, 96, 32, 32, 284}, // Tab
	{24, 96, 16, 32, 442},
	{40, 96, 16, 32, 444},
	{56, 96, 16, 32, 446},
	{72, 96, 16, 32, 448},
	{88, 96, 16, 32, 450},
	{104, 96, 16, 32, 452},
	{120, 96, 16, 32, 454},
	{136, 96, 16, 32, 456},
	{152, 96, 16, 32, 458},
	{168, 96, 16, 32, 460},
	{184, 96, 16, 32, 462},
	{200, 96, 16, 32, 464},
	{216, 96, 16, 32, 466},
	{232, 96, 16, 32, 468},
// Fourth row
	{0, 120, 32, 32, 296}, // Shift
	{32, 120, 16, 32, 470},
	{48, 120, 16, 32, 472},
	{64, 120, 16, 32, 474},
	{80, 120, 16, 32, 476},
	{96, 120, 16, 32, 478},
	{112, 120, 16, 32, 480},
	{128, 120, 16, 32, 482},
	{144, 120, 16, 32, 484},
	{160, 120, 16, 32, 486},
	{176, 120, 16, 32, 488},
	{192, 120, 16, 32, 490},
	{208, 120, 16, 32, 492},
	{224, 120, 32, 32, 300}, // Enter
// Fifth row (all special keys)
	{0, 144, 16, 16, 313}, // Caps lock
	{24, 144, 16, 16, 508}, // KYA select
	{40, 144, 16, 16, 509}, // KYM select
	{56, 144, 16, 16, 510}, // KYK select
	{80, 144, 32, 16, 496}, // Spacebar left
	{112, 144, 16, 16, 495},
	{128, 144, 16, 16, 495},
	{144, 144, 16, 16, 495},
	{160, 144, 16, 16, 495},
	{176, 144, 16, 16, 494}, // Spacebar right
	{200, 144, 16, 16, 309}, // Insert
	{216, 144, 16, 16, 310}, // Delete
	{240, 144, 16, 16, 311}, // Search
// Function keys + exit
	{0, 0, 16, 16, 505}, // Function Key 1 Left
	{16, 0, 32, 16, 499}, // Function Key Right
	{48, 0, 16, 16, 504}, // Function Key 2 Left
	{64, 0, 32, 16, 499}, // Function Key Right
	{96, 0, 16, 16, 503}, // Function Key 3 Left
	{112, 0, 32, 16, 499}, // Function Key Right
	{144, 0, 16, 16, 502}, // Function Key 4 Left
	{160, 0, 32, 16, 499}, // Function Key Right
	{192, 0, 16, 16, 501}, // Function Key 5 Left
	{208, 0, 32, 16, 499}, // Function Key Right
	{240, 0, 16, 16, 315},
// Bottom bar keys
	{0, 168, 32, 32, 304}, // Help key
	{40, 168, 32, 32, 256}, // RUN/STOP Key Left
	{72, 168, 16, 32, 260}, // RUN/STOP Key Right
	{88, 168, 16, 32, 262}, // EDIT Key Left
	{104, 168, 32, 32, 264}, // EDIT Key Right
// Icon (TODO:CODE:MED split into the icon subsystem)
	{144, 168, 16, 16, 316}, // Icon page up
	{144, 180, 16, 16, 317}, // Icon page down
};

void set_keyboard(struct ptc* p, enum pnltype type){
	// TODO:PERF:LOW Check previous state to change less if possible
	// Set BG layout (load layout)
	// Note that only a portion of the grid is ever needed
	if (type == PNL_OFF){
		memset(bg_page(p,1,2), 0, 32*24);
	} else if (type == PNL_PNL){
		load_file((u8*)bg_page(p,1,2), "resources/pnlPANEL.NSCR", 36, 32*24*2);
	} else {
		load_file((u8*)bg_page(p,1,2), "resources/pnlKEY.NSCR", 36, 32*24*2);
	}
	// Set sprites locations
	// TODO: run this once and only update CHR + POS as needed?
	for (unsigned int i = 0; i < sizeof(keyboard_pos) / sizeof(keyboard_pos[0]); ++i){
		int x,y,w,h,c;
		x = INT_TO_FP(keyboard_pos[i][0]);
		y = INT_TO_FP(keyboard_pos[i][1]);
		w = keyboard_pos[i][2];
		h = keyboard_pos[i][3];
		c = keyboard_pos[i][4];
		
		p->panel.keys[i] = init_sprite_info(i,c,0,0,0,1,w,h);
		p->panel.keys[i].pos.x = x;
		p->panel.keys[i].pos.y = y;
	}
	
}
