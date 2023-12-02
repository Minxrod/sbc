#include "panel.h"

#include "common.h"
#include "error.h"
#include "console.h"
#include "system.h"

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

enum key_sprite_size {
	KEY_SPR_16_16,
	KEY_SPR_16_32,
	KEY_SPR_32_32,
};

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

// x-coordinate, sprite size
u8 keyboard_pos[][2]={
	{0, KEY_SPR_32_32},
	{24, KEY_SPR_16_32},
	{40, KEY_SPR_16_32},
	{56, KEY_SPR_16_32},
	{72, KEY_SPR_16_32},
	{88, KEY_SPR_16_32},
	{104, KEY_SPR_16_32},
	{120, KEY_SPR_16_32},
	{136, KEY_SPR_16_32},
	{152, KEY_SPR_16_32},
	{168, KEY_SPR_16_32},
	{184, KEY_SPR_16_32},
	{200, KEY_SPR_16_32},
	{216, KEY_SPR_16_32},
	{232, KEY_SPR_16_32},
};

void set_keyboard(struct panel* p, enum pnltype type){
	// TODO:PERF:LOW Check previous state to change less if possible
	// Set BG layout (load layout)
	(void)p;
	(void)type;
	// Set sprites locations
	
	
}
