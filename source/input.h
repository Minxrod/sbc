#pragma once

#include "common.h"

#if defined(PC) || defined(ARM9)
// This only matters on PC
#include <threads.h>
#endif

#include <stdbool.h>

#define BUTTON_COUNT 12
// ^ v < > A B X Y L R START SELECT
#define BUTTON_ID_UP 0
#define BUTTON_ID_DOWN 1
#define BUTTON_ID_LEFT 2
#define BUTTON_ID_RIGHT 3
#define BUTTON_ID_A 4
#define BUTTON_ID_B 5
#define BUTTON_ID_X 6
#define BUTTON_ID_Y 7
#define BUTTON_ID_L 8
#define BUTTON_ID_R 9
#define BUTTON_ID_START 10
#define BUTTON_ID_SELECT 11

#define BUTTON_UP (1 << BUTTON_ID_UP)
#define BUTTON_DOWN (1 << BUTTON_ID_DOWN)
#define BUTTON_LEFT (1 << BUTTON_ID_LEFT)
#define BUTTON_RIGHT (1 << BUTTON_ID_RIGHT)
#define BUTTON_A (1 << BUTTON_ID_A)
#define BUTTON_B (1 << BUTTON_ID_B)
#define BUTTON_X (1 << BUTTON_ID_X)
#define BUTTON_Y (1 << BUTTON_ID_Y)
#define BUTTON_L (1 << BUTTON_ID_L)
#define BUTTON_R (1 << BUTTON_ID_R)
#define BUTTON_START (1 << BUTTON_ID_START)
#define BUTTON_SELECT (1 << BUTTON_ID_SELECT)

#define INKEY_BUF_SIZE 256

struct input {
	/// Current button state
	uint_fast16_t button;
	/// Previous button state
	uint_fast16_t old_button;
	///
	struct button_time {
		int start;
		int repeat;
		int frame;
	} times[12];
	
	uint_fast16_t tchx, tchy, tchtime;
	// Derived from tchx,tchy
	u8 keyboard;
	
#if defined(PC)
	mtx_t inkey_mtx;
	mtx_t touch_mtx;
#endif
	u16 inkey_buf[INKEY_BUF_SIZE];
	u16 current_write;
	u16 current_base;
};

void init_input(struct input* i);

void set_input(struct input* i, int b);
void set_repeat(struct input* i, int button, int start, int repeat);

bool set_inkey(struct input* i, u16 k);
u16 get_inkey(struct input* i);

// Should also set keyboard, inkey!
void set_touch(struct input* i, bool t, u8 x, u8 y);

bool check_pressed(struct input* i, int id);
bool check_pressed_manual(struct input* i, int id, int start, int repeat);

struct ptc;

void cmd_brepeat(struct ptc* p);
void func_btrig(struct ptc* p);
void func_button(struct ptc* p);
void func_inkey(struct ptc* p);
