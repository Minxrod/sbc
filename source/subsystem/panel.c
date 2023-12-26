#include "panel.h"

#include "common.h"
#include "error.h"
#include "console.h"
#include "system.h"

#include <string.h>
#include <assert.h>

// x,y,w,h,chr,keycode
s16 keyboard_pos[][6]={
// The primary keyboard keys [1-68]
// First row
	{0, 48, 32, 32, 280, 1}, // Escape
	{24, 48, 16, 32, 384, 2},
	{40, 48, 16, 32, 386, 3},
	{56, 48, 16, 32, 388, 4},
	{72, 48, 16, 32, 390, 5},
	{88, 48, 16, 32, 392, 6},
	{104, 48, 16, 32, 394, 7},
	{120, 48, 16, 32, 396, 8},
	{136, 48, 16, 32, 398, 9},
	{152, 48, 16, 32, 400, 10},
	{168, 48, 16, 32, 402, 11},
	{184, 48, 16, 32, 404, 12},
	{200, 48, 16, 32, 406, 13},
	{216, 48, 16, 32, 408, 14},
	{232, 48, 32, 32, 276, 15}, // Backspace
// Second row
	{0, 72, 16, 32, 410, 16},
	{16, 72, 16, 32, 412, 17},
	{32, 72, 16, 32, 414, 18},
	{48, 72, 16, 32, 416, 19},
	{64, 72, 16, 32, 418, 20},
	{80, 72, 16, 32, 420, 21},
	{96, 72, 16, 32, 422, 22},
	{112, 72, 16, 32, 424, 23},
	{128, 72, 16, 32, 426, 24},
	{144, 72, 16, 32, 428, 25},
	{160, 72, 16, 32, 430, 26},
	{176, 72, 16, 32, 432, 27},
	{192, 72, 16, 32, 434, 28},
	{208, 72, 16, 32, 436, 29},
	{224, 72, 16, 32, 438, 30},
	{240, 72, 16, 32, 440, 31},
// Third row
	{0, 96, 32, 32, 284, 32}, // Tab
	{24, 96, 16, 32, 442, 33},
	{40, 96, 16, 32, 444, 34},
	{56, 96, 16, 32, 446, 35},
	{72, 96, 16, 32, 448, 36},
	{88, 96, 16, 32, 450, 37},
	{104, 96, 16, 32, 452, 38},
	{120, 96, 16, 32, 454, 39},
	{136, 96, 16, 32, 456, 40},
	{152, 96, 16, 32, 458, 41},
	{168, 96, 16, 32, 460, 42},
	{184, 96, 16, 32, 462, 43},
	{200, 96, 16, 32, 464, 44},
	{216, 96, 16, 32, 466, 45},
	{232, 96, 16, 32, 468, 46},
// Fourth row
	{0, 120, 32, 32, 296, 47}, // Shift
	{32, 120, 16, 32, 470, 48},
	{48, 120, 16, 32, 472, 49},
	{64, 120, 16, 32, 474, 50},
	{80, 120, 16, 32, 476, 51},
	{96, 120, 16, 32, 478, 52},
	{112, 120, 16, 32, 480, 53},
	{128, 120, 16, 32, 482, 54},
	{144, 120, 16, 32, 484, 55},
	{160, 120, 16, 32, 486, 56},
	{176, 120, 16, 32, 488, 57},
	{192, 120, 16, 32, 490, 58},
	{208, 120, 16, 32, 492, 59},
	{224, 120, 32, 32, 300, 60}, // Enter
// Fifth row (all special keys)
	{0, 144, 16, 16, 313, 61}, // Caps lock
	{24, 144, 16, 16, 508, 62}, // KYA select
	{40, 144, 16, 16, 509, 63}, // KYM select
	{56, 144, 16, 16, 510, 64}, // KYK select
	{80, 144, 32, 16, 496, 65}, // Spacebar left
	{112, 144, 16, 16, 495, 65},
	{128, 144, 16, 16, 495, 65},
	{144, 144, 16, 16, 495, 65},
	{160, 144, 16, 16, 495, 65},
	{176, 144, 16, 16, 494, 65}, // Spacebar right
	{200, 144, 16, 16, 309, 66}, // Insert
	{216, 144, 16, 16, 310, 67}, // Delete
	{240, 144, 16, 16, 311, 68}, // Search
// Function keys + exit
	{0, -1, 16, 16, 505, 101}, // Function Key 1 Left
	{16, -1, 32, 16, 499, 101}, // Function Key Right
	{48, -1, 16, 16, 504, 102}, // Function Key 2 Left
	{64, -1, 32, 16, 499, 102}, // Function Key Right
	{96, -1, 16, 16, 503, 103}, // Function Key 3 Left
	{112, -1, 32, 16, 499, 103}, // Function Key Right
	{144, -1, 16, 16, 502, 104}, // Function Key 4 Left
	{160, -1, 32, 16, 499, 104}, // Function Key Right
	{192, -1, 16, 16, 501, 105}, // Function Key 5 Left
	{208, -1, 32, 16, 499, 105}, // Function Key Right
	{240, 0, 16, 16, 315, 106}, // Exit button
// Bottom bar keys
	{0, 168, 32, 32, 304, 70}, // Help key
	{40, 168, 32, 32, 256, 71}, // RUN/STOP Key Left
	{72, 168, 16, 32, 260, 71}, // RUN/STOP Key Right
	{88, 168, 16, 32, 262, 72}, // EDIT Key Left
	{104, 168, 32, 32, 264, 72}, // EDIT Key Right
// Icon (index=90)
	{144, 168, 16, 16, -1, 80}, // Icon page up (316)
	{144, 180, 16, 16, -1, 81}, // Icon page down (317)
	{160, 168, 32, 32, -1, 90}, // Icon page down
	{184, 168, 32, 32, -1, 91}, // Icon page down
	{208, 168, 32, 32, -1, 92}, // Icon page down
	{232, 168, 32, 32, -1, 93}, // Icon page down
};

void init_panel(struct ptc* p){
//	struct panel* panel = calloc(sizeof(struct panel), 1);
	p->panel.type = PNL_KYA;
	p->panel.text = init_console();
	// TODO:PERF:NONE Check if making console dynamically sized saves space?
	p->panel.keys_text = init_console();
	set_keyboard(p, p->panel.type);
	
	set_function_key(p, 1, "S\5FILES");
	set_function_key(p, 2, "S\5LOAD\"");
	set_function_key(p, 3, "S\5SAVE\"");
	set_function_key(p, 4, "S\4CONT");
	set_function_key(p, 5, "S\3RUN");
	
	// Set sprites locations
	for (idx i = 0; i < sizeof(keyboard_pos) / sizeof(keyboard_pos[0]); ++i){
		int x,y,w,h,c,k;
		x = INT_TO_FP(keyboard_pos[i][0]);
		y = INT_TO_FP(keyboard_pos[i][1]);
		w = keyboard_pos[i][2];
		h = keyboard_pos[i][3];
		c = keyboard_pos[i][4];
		k = keyboard_pos[i][5];
		
		p->panel.keys[i] = init_sprite_info(i,c,0,0,0,0,w,h);
		p->panel.keys[i].pos.x = x;
		p->panel.keys[i].pos.y = y;
		p->panel.keys[i].hit.x = INT_TO_FP(1);
		p->panel.keys[i].hit.y = INT_TO_FP(1);
		if (k < 70){
			// TODO:IMPL:HIGH Identify wide keys and change hitbox accordingly
			p->panel.keys[i].hit.w = INT_TO_FP(14);
			p->panel.keys[i].hit.h = INT_TO_FP(22);
		} else if (k >= 70 && k <= 79){
			// TODO:IMPL:MED Determine hitbox sizes for other keys
		} else if (k >= 90 && k <= 99){
			p->panel.keys[i].hit.w = INT_TO_FP(22);
		}
		p->panel.keys[i].hit.h = INT_TO_FP(22);
		p->panel.keys[i].vars[0] = k;
		if (c == -1){
			p->panel.keys[i].active = false;
		}
	}
}

void free_panel(struct ptc* p){
	free_console(p->panel.text);
}

void cmd_pnltype(struct ptc* p){
	void* type = value_str(ARG(0));
	
	enum pnltype old_type = p->panel.type;
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
	if (old_type != p->panel.type){
		if ((old_type >= PNL_KYA) != (p->panel.type >= PNL_KYA))
			set_keyboard(p, p->panel.type);
	}
}

// TODO:TEST:MED Test this function, specifically edge behavior
void cmd_pnlstr(struct ptc* p){
	int x, y;
	STACK_INT_RANGE_SILENT(0,0,CONSOLE_WIDTH-1,x);
	STACK_INT_RANGE_SILENT(1,0,CONSOLE_HEIGHT-1,y);
	void* str = value_str(ARG(2));
	u8 c = 0;
	if (p->stack.stack_i == 4){
		STACK_INT_RANGE(3,0,15,c);
	}
	p->panel.text->col = c;
	p->panel.text->x = x;
	p->panel.text->y = y;
	int len = str_len(str);
	for (int i = 0; i < len; ++i){
		con_put(p->panel.text, str_at_wide(str, i));
		if (p->panel.text->y != y) return; // stop printing at edge of screen
	}
}

void cmd_iconset(struct ptc* p){
	// ICONSET id,icon
	int i, icon;
	STACK_INT_RANGE(0,0,3,i);
	STACK_INT_RANGE(1,0,64,icon);
	p->panel.keys[ICON_START+i].active = true;
	p->panel.keys[ICON_START+i].chr = icon*4;
}

void cmd_iconclr(struct ptc* p){
	ERROR(ERR_UNIMPLEMENTED);
}

void func_iconchk(struct ptc* p){
	// TODO:IMPL:MED Icon repeat interval
	if (p->panel.key_pressed > 90){
		stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(p->panel.key_pressed - 90)}});
	} else {
		stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {-INT_TO_FP(1)}});
	}
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

// TODO:CODE:NONE Rename to set_panel_bg or something?
void set_keyboard(struct ptc* p, enum pnltype type){
	// Set BG layout (load layout)
	// Note that only a portion of the grid is ever needed
	u16* panel_bg = p->res.scr[SCR_BANKS+1];
	if (type == PNL_OFF){
		memset((u8*)panel_bg, 0, 32*24*2); //TODO:CODE:LOW Replace these with constants
	} else if (type == PNL_PNL){
		load_file((u8*)panel_bg, "resources/pnlPANEL.NSCR", 36, 32*24*2);
	} else {
		load_file((u8*)panel_bg, "resources/pnlKEY.NSCR", 36, 32*24*2);
	}
}

void set_function_key(struct ptc* p, int key, const void* string){
	assert(1 <= key && key <= 5);
	key--;
	p->panel.func_keys_len[key] = str_len(string);
	str_wide_copy(string, p->panel.func_keys[key]);
	struct console* c = p->panel.keys_text;
	c->x = 1+key*6;
	for (idx i = 0; i < 5; ++i){
		if (i == 4 && str_len(string) > 4){
			con_put(c, to_wide('.'));
		} else if (i < str_len(string)){
			con_put(c, str_at_wide(string, i));
		}
	}
}

void press_key(struct ptc* ptc, bool t, int x, int y){
	struct panel* p = &ptc->panel;
	p->key_pressed = 0;
	p->id_pressed = -1;
	if (!t)
		return; // no touch = no press
	
	// TODO:PERF:NONE Don't recreate the sprite every time
	// Idea: Make this a cursor/possible to be a cursor?
	// Note: Width+height of zero leads to a 1-pixel hitbox
	struct sprite_info touch = init_sprite_info(0,0,0,0,0,0,0,0);
	touch.pos.x = INT_TO_FP(x);
	touch.pos.y = INT_TO_FP(y);
	
//	iprintf("%d,%d\n",x,y);
	int start = 0;
	if (p->type == PNL_OFF || p->type == PNL_PNL)
		start = KEYBOARD_KEYS; // skip to past keyboard keys and only check icons
	
	for (idx i = start; i < PANEL_KEYS; ++i){
		if (is_hit(&touch, &p->keys[i])){
//			iprintf("HIT:%d,%d\n",FP_TO_INT(p->keys[i].pos.x),FP_TO_INT(p->keys[i].pos.y));
			// found hit! save index
			p->id_pressed = i;
			p->key_pressed = p->keys[i].vars[0];
			break;
		}
	}
	
	// TODO:IMPL:HIGH Check panel state
	// TODO:IMPL:HIGH Keyboard repeat timings
	// TODO:IMPL:HIGH Keyboard modes;
	// TODO:IMPL:HIGH shift; caps lock
	if (p->key_pressed){
		if (p->key_pressed <= 60 || p->key_pressed == 65){
			char c = keyboard_chr[0][p->key_pressed];
			if (c)
				set_inkey(&ptc->input, to_wide(c));
		} else if (p->key_pressed == 61){
			ptc->res.regen_chr[18+CHR_BANKS] |= load_chr(ptc->res.chr[18+CHR_BANKS], "resources/SPD6.PTC");
			ptc->res.regen_chr[19+CHR_BANKS] |= load_chr(ptc->res.chr[19+CHR_BANKS], "resources/SPD7.PTC");
			// If files fail to load, just doesn't update keyboard
		}
	}
}

void offset_key(struct ptc* p, int id, int d){
	if (id<PANEL_KEYS){
		for (int i = id; p->panel.keys[i].vars[0] == p->panel.key_pressed; ++i){
			p->panel.keys[i].pos.x += d;
			p->panel.keys[i].pos.y += d;
		}
	}
	if (id>0){
		for (int i = id-1; p->panel.keys[i].vars[0] == p->panel.key_pressed; --i){
			p->panel.keys[i].pos.x += d;
			p->panel.keys[i].pos.y += d;
		}
	}
}
