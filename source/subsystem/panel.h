#pragma once
///
/// @file
/// @brief Defines the panel subsystem of PTC, including the virtual keyboard,
/// icons, and lower screen text commands.
///
#include "sprites.h"

struct ptc;
struct console;

#define PANEL_WIDTH 32
#define PANEL_HEIGHT 24

#define KEYBOARD_KEYS 89
#define ICON_KEYS 6
#define ICON_PAGE_START (KEYBOARD_KEYS)
#define ICON_START (KEYBOARD_KEYS + 2)
#define PANEL_KEYS (KEYBOARD_KEYS + ICON_KEYS)

// shift
#define PNL_SHIFT 1
#define PNL_CAPS_LOCK 2
// cursor
#define PNL_INSERT 1

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
	int pressed_time;
	
	int mode;
	int shift;
	int cursor;
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

void set_panel_bg(struct ptc* p, enum pnltype type);
void set_function_key(struct ptc* p, int key, const void* string);
void press_key(struct ptc* p, bool t, int x, int y);
void offset_key(struct ptc* p, int id, int d);

/// Returns the currently pressed key's sprite ID.
/// 
/// The ranges of return values fall into these ranges:
///  -   1- 69: Keyboard key, corresponding to the same value of KEYBOARD
///  -  90- 93: Icon, where 90=icon 0 and so on.
///  - 101-105: Function key, where 101 corresponds to FUNCNO=1 and so on.
/// 
/// @param p System struct
/// @return Sprite ID of pressed key.
int get_pressed_key(struct ptc* p);
