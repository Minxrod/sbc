#pragma once
///
/// @file console.h
/// @brief This manages the console commands.
/// 
/// This file contains information and code for the operation of text console.
/// No rendering of any kind is done for the text console here.
///
#include "common.h"

/// Width of the console, in text characters
#define CONSOLE_WIDTH 32
/// Height of the console, in text characters
#define CONSOLE_HEIGHT 24

struct ptc;

#define COL_FG_MASK 0x0f
#define COL_BG_MASK 0xf0

///
/// Console structure.
///
/// This manages the text console of SBC. This includes things like text color
/// and the cursor location.
///
/// This structure only contains the console data itself.
/// Rendering is handled using character resources in a different location.
///
struct console {
	/// The text cursor's x-coordinate.
	uint_fast8_t x; // range [0,CONSOLE_WIDTH)
	/// The text cursor's y-coordinate.
	uint_fast8_t y; // range [0,CONSOLE_HEIGHT)
	/// The current width of a tab.
	/// Updates from sys_tabstep on use.
	uint_fast8_t tabstep; //valid range is 1-16
	/// The width of a tab as assigned via system variables.
	fixp sys_tabstep; // used for actual assignment
	/// The contents of the text console.
	/// Stored as UCS2/UTF16 wide characters
	u16 text[CONSOLE_HEIGHT][CONSOLE_WIDTH];
	// low 4 bits: fg color; high 4: bg color
	/// The current text color. This color is used for newly printed text.
	///
	/// This includes both background and foreground colors.
	/// Use COL_BG_MASK and COL_FG_MASK to extract individual colors.
	uint_fast8_t col;
	/// The stored text color.
	/// Each text character has an assigned foreground and background color.
	uint_fast8_t color[CONSOLE_HEIGHT][CONSOLE_WIDTH];
	/// Enables "test mode," which is intended for automated testing.
	/// This disables the delay on waiting for characters within `shared_input`.
	bool test_mode;
};

/// Allocates a new `struct console` and initializes it.
///
/// @note Allocates memory - free with free_console.
/// @return New console struct.
struct console* init_console(void);

/// Frees memory allocated by `init_console`.
/// 
/// @param Console struct to free
void free_console(struct console*);

/// Places a wide character at the cursor position of the console,
/// and then advances the cursor.
/// 
/// @param c Console to write to
/// @param w Wide character to print.
void con_put(struct console* c, u16 w);

/// Writes a string to the text console, advancing the cursor to the end of the
/// string.
/// 
/// @param c Console to write to
/// @param s String to write
void con_puts(struct console* c, const void* s);

/// Writes a fixed-point number to the text console. The number is converted to
/// a decimal string and then printed.
/// 
/// @param c Console to write to
/// @param n Number to write
void con_putn(struct console* c, fixp n);

/// Writes a fixed-point number to a specific location within the text console.
///
/// @param c Console to write to
/// @param x x-coordinate to write number
/// @param y y-coordinate to write number
/// @param n Number to write
void con_putn_at(struct console* c, int x, int y, fixp n);

/// Advances the text cursor by one character.
/// 
/// @param c Console to advance cursor of
void con_advance(struct console* c);

/// Advances the text cursor to the next line. 
/// If the cursor reaches the bottom of the console and scroll is set, scrolls
/// the console to ensure cursor is on-screen. otherwise, cursor is set to just
/// off the console's edge.
void con_newline(struct console* c, bool scroll);

// PTC commands, etc.
void cmd_cls(struct ptc* p);
void cmd_print(struct ptc* p);
void cmd_color(struct ptc* p);
void cmd_locate(struct ptc* p);
void cmd_input(struct ptc* p);
void cmd_linput(struct ptc* p);
// PTC functions, etc.
void func_chkchr(struct ptc* p);
// PTC sysvars, etc.
void sys_csrx(struct ptc* p);
void sys_csry(struct ptc* p);
void sys_tabstep(struct ptc* p);

static inline u16 con_text_getc(struct console* c, u32 x, u32 y){
	return c->text[y][x];
}

static inline void con_text_setc(struct console* c, u32 x, u32 y, u16 w){
	c->text[y][x] = w;
}

static inline u8 con_col_get(struct console* c, u32 x, u32 y){
	return c->color[y][x];
}

static inline void con_col_set(struct console* c, u32 x, u32 y, u8 col){
	c->color[y][x] = col;
}
