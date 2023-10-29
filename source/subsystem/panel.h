#pragma once

struct ptc;
struct console;

struct panel {
	/// Current panel type
	enum pnltype {
		PNL_OFF,
		PNL_PNL,
		PNL_KYA,
		PNL_KYM,
		PNL_KYK
	} type;
	
	// TODO:IMPL:MED function keys
	// TODO:IMPL:HIGH keyboard
	
	struct console* text;
};

void cmd_pnltype(struct ptc* p);
void cmd_pnlstr(struct ptc* p);
