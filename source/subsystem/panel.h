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

/// Manages the specialized lower screen 'panel' state. This includes the
/// virtual keyboard, lower screen text printing, and the icon system.
struct panel {
	/// Current panel type.
	enum pnltype {
		PNL_OFF,
		PNL_PNL,
		PNL_KYA,
		PNL_KYM,
		PNL_KYK
	} type;
	
	/// Stores the text data for strings stored in each function key.
	u16 func_keys[5][256];
	/// Stores the length of the strings stored in each function key.
	u16 func_keys_len[5];
	
	/// User text console for the lower screen.
	struct console* text;
	
	/// Stores text for function keys
	/// and shortcut keys (not implemented currently)
	struct console* keys_text;
	
	/// Sprite data for the panel. This includes both keyboard and icon sprites.
	struct sprite_info keys[PANEL_KEYS];
	
	/// The "key id" of the current key pressed.
	///
	/// See get_pressed_key() for details.
	u8 key_pressed;
	/// The sprite ID of the current key pressed.
	/// This is an index into the keys array.
	u8 id_pressed;
	/// The time the key being pressed has been held, in frames.
	int pressed_time;
	
	/// Value indicating the status of the SHIFT key and the caps lock key
	/// of the virtual keyboard.
	int shift;
	/// Value indicating whether text entry is in INSERT or REPLACE mode.
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

void refresh_panel(struct ptc*);
void set_panel_bg(struct ptc* p, enum pnltype type);
void set_function_key(struct ptc* p, int key, const void* string);
void press_key(struct ptc* p, bool t, int x, int y);
void offset_key(struct ptc* p, int id, int d);

/// Returns the currently pressed key's key ID.
/// 
/// The ranges of return values fall into these ranges:
///  -   1- 69: Keyboard key, corresponding to the same value of KEYBOARD
///  -  90- 93: Icon, where 90=icon 0 and so on.
///  - 101-105: Function key, where 101 corresponds to FUNCNO=1 and so on.
/// 
/// @param p System struct
/// @return Sprite ID of pressed key.
int get_pressed_key(struct ptc* p);
