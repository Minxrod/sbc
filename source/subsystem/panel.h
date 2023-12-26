#pragma once

#include "sprites.h"

struct ptc;
struct console;

#define KEYBOARD_KEYS 89
#define ICON_KEYS 6
#define ICON_PAGE_START (KEYBOARD_KEYS)
#define ICON_START (KEYBOARD_KEYS + 2)
#define PANEL_KEYS (KEYBOARD_KEYS + ICON_KEYS)

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
	
	struct console* text;
	
	// Stores text for function keys and shortcut keys
	struct console* keys_text;
	
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
void sys_iconpuse(struct ptc* p);

void set_keyboard(struct ptc* p, enum pnltype type);
void set_function_key(struct ptc* p, int key, const void* string);
void press_key(struct ptc* p, bool t, int x, int y);
void offset_key(struct ptc* p, int id, int d);
