#pragma once

#include "common.h"

#define CONSOLE_WIDTH 32
#define CONSOLE_HEIGHT 24

struct ptc;

/**
 * Console structure
 * 
 * This structure only contains the console data itself.
 * Rendering is handled using character resources in a different location.
 */
struct console {
	u16 x;
	u16 y;
	u32 tabstep;
	u16 text[CONSOLE_WIDTH][CONSOLE_HEIGHT];
	// low 4 bits: fg color; high 4: bg color
	u8 col;
	u8 color[CONSOLE_WIDTH][CONSOLE_HEIGHT];
};

void con_init(struct console* c);

void con_put(struct console* c, u16 w);
void con_puts(struct console* c, void* s);

void con_advance(struct console* c);
void con_tab(struct console* c);
void con_newline(struct console* c);

void cmd_cls(struct ptc* p);

void cmd_print(struct ptc* p);

void cmd_color(struct ptc* p);

void cmd_locate(struct ptc* p);

u16 con_text_getc(struct console* c, u32 x, u32 y);
void con_text_setc(struct console* c, u32 x, u32 y, u16 w);
