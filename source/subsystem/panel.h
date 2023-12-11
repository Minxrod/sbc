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
	
	// TODO:IMPL:LOW Function keys need separate string storage
	void* func_keys[5];
	
	// TODO:IMPL:MED function keys
	// TODO:IMPL:HIGH keyboard
	
	struct console* text;
	
	struct sprite_info keys[PANEL_KEYS];
};

void init_panel(struct ptc*);
void free_panel(struct ptc*);

void cmd_pnltype(struct ptc* p);
void cmd_pnlstr(struct ptc* p);

void set_keyboard(struct ptc* p, enum pnltype type);
