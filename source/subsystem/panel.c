#include "panel.h"

#include "common.h"
#include "error.h"
#include "console.h"
#include "system.h"

#include <string.h>
#include <assert.h>

// x,y,w,h,chr,keycode
const s16 keyboard_pos[][6]={
// The primary keyboard keys [1-68]
// First row
	{0, 48, 32, 32, 280, 1}, // Escape (sprite 0)
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
	{224, 120, 32, 32, 300, 60}, // Enter (sprite 59)
// Fifth row (all special keys)
	{0, 144, 16, 16, 313, 61}, // Caps lock
	{24, 144, 16, 16, 508, 62}, // KYA select
	{40, 144, 16, 16, 509, 63}, // KYM select
	{56, 144, 16, 16, 510, 64}, // KYK select
	{80, 144, 32, 16, 496, 65}, // Spacebar left (sprite 64)
	{112, 144, 16, 16, 495, 65},
	{128, 144, 16, 16, 495, 65},
	{144, 144, 16, 16, 495, 65},
	{160, 144, 16, 16, 495, 65},
	{176, 144, 16, 16, 494, 65}, // Spacebar right (sprite 69)
	{200, 144, 16, 16, 309, 66}, // Insert
	{216, 144, 16, 16, 310, 67}, // Delete
	{240, 144, 16, 16, 311, 68}, // Search (sprite 72)
// Function keys + exit
	{0, -1, 16, 16, 505, 101}, // Function Key 1 Left (sprite 73)
	{16, -1, 32, 16, 499, 101}, // Function Key Right
	{48, -1, 16, 16, 504, 102}, // Function Key 2 Left
	{64, -1, 32, 16, 499, 102}, // Function Key Right
	{96, -1, 16, 16, 503, 103}, // Function Key 3 Left
	{112, -1, 32, 16, 499, 103}, // Function Key Right
	{144, -1, 16, 16, 502, 104}, // Function Key 4 Left
	{160, -1, 32, 16, 499, 104}, // Function Key Right
	{192, -1, 16, 16, 501, 105}, // Function Key 5 Left
	{208, -1, 32, 16, 499, 105}, // Function Key Right (sprite 82)
	{240, 0, 16, 16, 315, 106}, // Exit button (sprite 83)
// Bottom bar keys
	{0, 168, 32, 32, 304, 70}, // Help key (sprite 84)
	{40, 168, 32, 32, 256, 71}, // RUN/STOP Key Left
	{72, 168, 16, 32, 260, 71}, // RUN/STOP Key Right
	{88, 168, 16, 32, 262, 72}, // EDIT Key Left
	{104, 168, 32, 32, 264, 72}, // EDIT Key Right (sprite 88)
// Icon (index=90)
	{144, 168, 16, 16, -1, 80}, // Icon page up (316) (sprite 89)
	{144, 180, 16, 16, -1, 81}, // Icon page down (317)
	{160, 168, 32, 32, -1, 90}, // Icon 0 (sprite 91)
	{184, 168, 32, 32, -1, 91}, // Icon 1
	{208, 168, 32, 32, -1, 92}, // Icon 2
	{232, 168, 32, 32, -1, 93}, // Icon 3 (sprite 94)
};

void init_panel(struct ptc* p){
	p->panel.type = PNL_KYA;
	p->panel.text = alloc_console();
	p->panel.keys_text = alloc_console();
	p->panel.shift = 0;
	p->panel.cursor = PNL_INSERT;
	set_panel_bg(p, p->panel.type);
	
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
		p->panel.keys[i].prio = 1;
		// Collisions handled by tiles, essentially
/*		p->panel.keys[i].hit.x = INT_TO_FP(1);
		p->panel.keys[i].hit.y = INT_TO_FP(1);
		if (k < 70){
			p->panel.keys[i].hit.w = INT_TO_FP(14);
			p->panel.keys[i].hit.h = INT_TO_FP(22);
		} else if (k >= 70 && k <= 79){
		} else if (k >= 90 && k <= 99){
			p->panel.keys[i].hit.w = INT_TO_FP(22);
		}
		p->panel.keys[i].hit.h = INT_TO_FP(22);*/
		p->panel.keys[i].vars[0] = k;
		if (c == -1){
			p->panel.keys[i].active = false;
		}
	}
}

void free_panel(struct ptc* p){
	free_console(p->panel.text);
	free_console(p->panel.keys_text);
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
			set_panel_bg(p, p->panel.type);
	}
}

// TODO:TEST:MED Test this function, specifically edge behavior
void cmd_pnlstr(struct ptc* p){
	unsigned int x, y;
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

void cmd_key(struct ptc* p){
	int key_id;
	STACK_INT_RANGE(0,1,5,key_id);
	void* key_str = STACK_STR(1);
	
	// updates visual and data
	set_function_key(p, key_id, key_str);
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
	// ICONCLR [id]
	if (p->stack.stack_i){
		int id;
		STACK_INT_RANGE(0,0,3,id);
		p->panel.keys[ICON_START+id].active = false;
	} else {
		for (int i = ICON_START; i < PANEL_KEYS; ++i){
			p->panel.keys[i].active = false;
		}
	}
}

void func_iconchk(struct ptc* p){
	int icon = get_pressed_key(p);
	if (icon >= 90 && p->panel.keys[p->panel.id_pressed].active){
		stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(icon - 90)}});
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

void set_panel_bg(struct ptc* p, enum pnltype type){
	// Set BG layout (load layout)
	// Note that only a portion of the grid is ever needed
	u16* panel_bg = p->res.scr[SCR_BANKS+1];
	if (type == PNL_OFF){
		memset((u8*)panel_bg, 0, PANEL_WIDTH*PANEL_HEIGHT*2);
	} else if (type == PNL_PNL){
		load_file((u8*)panel_bg, "resources/pnlPANEL.NSCR", 36, PANEL_WIDTH*PANEL_HEIGHT*2);
	} else {
		load_file((u8*)panel_bg, "resources/pnlKEY.NSCR", 36, PANEL_WIDTH*PANEL_HEIGHT*2);
	}
}

void set_function_key(struct ptc* p, int key, const void* string){
	assert(1 <= key && key <= 5);
	--key;
	p->panel.func_keys_len[key] = str_len(string);
	str_wide_copy(string, p->panel.func_keys[key]);
	struct console* c = p->panel.keys_text;
	c->x = 1+key*6;
	for (idx i = 0; i < 5; ++i){
		if (i == 4 && str_len(string) > 4){
			con_put(c, to_wide('.'));
		} else if (i < str_len(string)){
			con_put(c, str_at_wide(string, i));
		} else {
			con_put(c, to_wide(' '));
		}
	}
}

// This is used to calculate the sprite ID of the touched location.
// This is a 32x24 grid of characters corresponding to sprite ID at that location.
// \xff (-1) indicates no sprite currently is placed there.
// TODO:CODE:LOW Find a way to generate this from some sort of layout file instead
// (then use this as a test case?)
const char* key_map = 
"\111\111\112\112\112\112\113\113\114\114\114\114\115\115\116\116\116\116\117\117\120\120\120\120\121\121\122\122\122\122\123\123"
"\111\111\112\112\112\112\113\113\114\114\114\114\115\115\116\116\116\116\117\117\120\120\120\120\121\121\122\122\122\122\123\123"
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
"\00\00\00\01\01\02\02\03\03\04\04\05\05\06\06\07\07\10\10\11\11\12\12\13\13\14\14\15\15\16\16\16"
"\00\00\00\01\01\02\02\03\03\04\04\05\05\06\06\07\07\10\10\11\11\12\12\13\13\14\14\15\15\16\16\16"
"\00\00\00\01\01\02\02\03\03\04\04\05\05\06\06\07\07\10\10\11\11\12\12\13\13\14\14\15\15\16\16\16"
"\17\17\20\20\21\21\22\22\23\23\24\24\25\25\26\26\27\27\30\30\31\31\32\32\33\33\34\34\35\35\36\36"
"\17\17\20\20\21\21\22\22\23\23\24\24\25\25\26\26\27\27\30\30\31\31\32\32\33\33\34\34\35\35\36\36"
"\17\17\20\20\21\21\22\22\23\23\24\24\25\25\26\26\27\27\30\30\31\31\32\32\33\33\34\34\35\35\36\36"
"\37\37\37\40\40\41\41\42\42\43\43\44\44\45\45\46\46\47\47\50\50\51\51\52\52\53\53\54\54\55\55\xff"
"\37\37\37\40\40\41\41\42\42\43\43\44\44\45\45\46\46\47\47\50\50\51\51\52\52\53\53\54\54\55\55\xff"
"\37\37\37\40\40\41\41\42\42\43\43\44\44\45\45\46\46\47\47\50\50\51\51\52\52\53\53\54\54\55\55\xff"
"\56\56\56\56\57\57\60\60\61\61\62\62\63\63\64\64\65\65\66\66\67\67\70\70\71\71\72\72\73\73\73\73"
"\56\56\56\56\57\57\60\60\61\61\62\62\63\63\64\64\65\65\66\66\67\67\70\70\71\71\72\72\73\73\73\73"
"\56\56\56\56\57\57\60\60\61\61\62\62\63\63\64\64\65\65\66\66\67\67\70\70\71\71\72\72\73\73\73\73"
"\74\74\xff\75\75\76\76\77\77\xff\100\100\100\100\101\101\102\102\103\103\104\104\105\105\xff\106\106\107\107\xff\110\110"
"\74\74\xff\75\75\76\76\77\77\xff\100\100\100\100\101\101\102\102\103\103\104\104\105\105\xff\106\106\107\107\xff\110\110"
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
"\124\124\124\124\xff\125\125\125\125\126\126\127\127\130\130\130\130\xff\xff\xff\133\133\133\134\134\134\135\135\135\136\136\136"
"\124\124\124\124\xff\125\125\125\125\126\126\127\127\130\130\130\130\xff\xff\xff\133\133\133\134\134\134\135\135\135\136\136\136"
"\124\124\124\124\xff\125\125\125\125\126\126\127\127\130\130\130\130\xff\xff\xff\133\133\133\134\134\134\135\135\135\136\136\136"
;

void press_key(struct ptc* ptc, bool t, int x, int y){
	struct panel* p = &ptc->panel;
	if (!t){
		p->key_pressed = 0;
		p->id_pressed = -1;
		p->pressed_time = 0;
		return; // no touch = no press
	} else if (t && p->key_pressed){ // touch and hold
		char c = key_map[x/8 + 32*(y/8)];
		if (p->keys[(int)c].vars[0] != p->key_pressed){
			p->key_pressed = 0;
			p->id_pressed = -1;
		} else {
			++p->pressed_time;
		}
	} else if (p->pressed_time > 0){ // was holding, but moved away from key (t && !last_pressed && timer is still active)
//		return;
	} else {
		//press keyboard keys (touching, no previous hold, no previous timer)
		char c = key_map[x/8 + 32*(y/8)];
		if (c != '\xff'){
			p->id_pressed = c;
			p->key_pressed = p->keys[(int)c].vars[0];
			p->pressed_time = 1;
		} else {
			// no key pressed
			p->pressed_time = 0;
		}
	}
	
	// TODO:IMPL:MED Icon pages (check here before panel check)
	if (p->type == PNL_OFF || p->type == PNL_PNL) return;
	
	int pressed_key = get_pressed_key(ptc);
	bool refresh = false;
	
	if (pressed_key){
		if (pressed_key == 1){
			ptc->exec.error = ERR_BREAK;
		} else if (pressed_key == 47){
			// shift
			p->shift ^= PNL_SHIFT;
			refresh = true;
		} else if (pressed_key <= 60 || pressed_key == 65){
			int shift = !!(p->shift & PNL_SHIFT) ^ !!(p->shift & PNL_CAPS_LOCK);
			int source_key_map = 2 * (p->type - 2) + (shift);
			int source_key = pressed_key;
			if (source_key_map == 5) source_key *= 2;
			char c = keyboard_chr[source_key_map][source_key];
			if (c){
				set_inkey(&ptc->input, to_wide(c));
				// Katakana shift keyboard can type two characters at once
				if (source_key_map == 5){
					char c2 = keyboard_chr[source_key_map][source_key+1];
					if (c2){
						set_inkey(&ptc->input, to_wide(c2));
					}
				}
				if (p->shift & PNL_SHIFT){
					p->shift &= ~PNL_SHIFT;
					refresh = true;
				}
			}
		} else if (pressed_key >= 61 && pressed_key <= 64){
			if (pressed_key == 61) p->shift ^= PNL_CAPS_LOCK;
			else if (pressed_key == 62) p->type = PNL_KYA;
			else if (pressed_key == 63) p->type = PNL_KYM;
			else if (pressed_key == 64) p->type = PNL_KYK;
			
			refresh = true;
		} else if (pressed_key == 66){
			p->cursor ^= PNL_INSERT;
		} else if (pressed_key >= 101 && pressed_key <= 105){
			int key = pressed_key - 101;
			for (int i = 0; i < p->func_keys_len[key]; ++i){
				set_inkey(&ptc->input, p->func_keys[key][i]);
			}
		} else if (pressed_key >= 70 && pressed_key <= 72){
			ptc->exec.error = ERR_BUTTON_SIGNAL;
			ptc->exec.error_info[0] = pressed_key;
		}
	}
	
	if (refresh){
		refresh_panel(ptc);
	}
}

void refresh_panel(struct ptc* ptc){
	struct panel* p = &ptc->panel;
	int shift = !!(p->shift & PNL_SHIFT) ^ !!(p->shift & PNL_CAPS_LOCK);
	int source_key_chr = (2 * (p->type - 2) + shift) * 2;
	memcpy(ptc->res.chr[18+CHR_BANKS], ptc->res.key_chr[source_key_chr], CHR_SIZE);
	memcpy(ptc->res.chr[19+CHR_BANKS], ptc->res.key_chr[source_key_chr + 1], CHR_SIZE);
	
	ptc->res.regen_chr[18+CHR_BANKS] |= true;
	ptc->res.regen_chr[19+CHR_BANKS] |= true;
}

int get_pressed_key(struct ptc* ptc){
	struct panel* p = &ptc->panel;
	if (!p->key_pressed || !check_repeat(p->pressed_time, 30, 4))
		return 0;
	return p->key_pressed;
}

void offset_key(struct ptc* p, int id, int d){
	if (id<PANEL_KEYS){
		for (int i = id; p->panel.keys[i].vars[0] == p->panel.key_pressed && i<PANEL_KEYS; ++i){
			p->panel.keys[i].pos.x += d;
			p->panel.keys[i].pos.y += d;
		}
	}
	if (id>0){
		for (int i = id-1; p->panel.keys[i].vars[0] == p->panel.key_pressed && i>0; --i){
			p->panel.keys[i].pos.x += d;
			p->panel.keys[i].pos.y += d;
		}
	}
}
