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
	u8 color[CONSOLE_WIDTH][CONSOLE_HEIGHT];
};

void con_put(struct console* c, u16 w);

void con_advance(struct console* c);
void con_tab(struct console* c);
void con_newline(struct console* c);

void cmd_cls(struct ptc* p);

void cmd_print(struct ptc* p);

void cmd_color(struct ptc* p);

void cmd_locate(struct ptc* p);
