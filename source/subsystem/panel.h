#pragma once

#include "sprites.h"

struct ptc;
struct console;

//TODO:PERF:LOW It's not this many but close?
#define PANEL_KEYS 100

struct panel {
	/// Current panel type
	enum pnltype {
		PNL_OFF,
		PNL_PNL,
		PNL_KYA,
		PNL_KYM,
		PNL_KYK
	} type;
	
	u16 func_keys[5][256];
	u16 func_keys_len[5];
	
	// TODO:IMPL:MED function keys
	// TODO:IMPL:HIGH keyboard
	
	struct console* text;
	
	struct sprite_info keys[PANEL_KEYS];
	
	u8 key_pressed;
	u8 id_pressed;
	
	// TODO:IMPL:MED Last pressed timer
};

void init_panel(struct ptc*);
void free_panel(struct ptc*);

void cmd_pnltype(struct ptc* p);
void cmd_pnlstr(struct ptc* p);

void cmd_key(struct ptc* p);

void cmd_iconset(struct ptc* p);
void cmd_iconclr(struct ptc* p);
void func_iconchk(struct ptc* p);
void sys_iconpage(struct ptc* p);
void sys_iconpmax(struct ptc* p);

void set_keyboard(struct ptc* p, enum pnltype type);
void set_function_key(struct ptc* p, int key, const void* string);
void press_key(struct ptc* p, bool t, int x, int y);
void offset_key(struct ptc* p, int id, int d);
