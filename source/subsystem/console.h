#pragma once
/**
 * @file console.h
 * @brief This manages the console commands.
 * 
 * This file contains information and code for the operation of text console.
 * No rendering of any kind is done for the text console here.
 *
 * 
 * 
 */

#include "common.h"

#define CONSOLE_WIDTH 32
#define CONSOLE_HEIGHT 24

struct ptc;

#define COL_FG_MASK 0x0f
#define COL_BG_MASK 0xf0

/**
 * Console structure
 * 
 * This structure only contains the console data itself.
 * Rendering is handled using character resources in a different location.
 */
struct console {
	uint_fast8_t x; // range [0,CONSOLE_WIDTH)
	uint_fast8_t y; // range [0,CONSOLE_HEIGHT)
	uint_fast8_t tabstep; //valid range is 1-16
	u16 text[CONSOLE_HEIGHT][CONSOLE_WIDTH];
	// low 4 bits: fg color; high 4: bg color
	uint_fast8_t col;
	uint_fast8_t color[CONSOLE_HEIGHT][CONSOLE_WIDTH];
	bool test_mode;
};

struct console* init_console(void);
void free_console(struct console*);

void con_put(struct console* c, u16 w);
void con_puts(struct console* c, const void* s);
void con_putn(struct console* c, fixp n);
void con_putn_at(struct console* c, int x, int y, fixp n);

void con_advance(struct console* c);
void con_tab(struct console* c);
void con_newline(struct console* c, bool scroll);

// PTC commands, etc.
void cmd_cls(struct ptc* p);
void cmd_print(struct ptc* p);
void cmd_color(struct ptc* p);
void cmd_locate(struct ptc* p);
void cmd_input(struct ptc* p);
void cmd_linput(struct ptc* p);
// PTC functions, etc.
void cmd_chkchr(struct ptc* p);
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
