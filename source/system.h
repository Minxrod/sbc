#pragma once

#include "common.h"
#include "stack.h"
#include "strs.h"
#include "arrays.h"
#include "runner.h"
#include "resources.h"
#include "input.h"

#include "console.h"
#include "graphics.h"

// note that VAR_LIMIT should be a multiple of 2 to ensure that
// the current hash algorithm works
#define VAR_LIMIT 2048
#define STR_LIMIT 4096
#define ARR_LIMIT 262144

#ifdef EZ80
#undef VAR_LIMIT
#undef STR_LIMIT
#undef ARR_LIMIT
#define VAR_LIMIT 64
#define STR_LIMIT 32
#define ARR_LIMIT 2048
#endif

// note: members here are stored directly (why?)
struct ptc {
	// generic stuff
	struct input input;
	// upper screen stuff
	struct console console;
//	struct background background;
//	struct sprites sprites;
	// graphics pages
	struct graphics graphics;
	// lower screen stuff
//	struct panel panel;
	
	// graphics and resources
	struct resources res;
	
	// actual system stuff
	/// The value/variable stack 
	struct runner exec;
	
	struct value_stack stack;
	
	struct call_stack calls; 
	
	struct variables vars;
	
	struct strings strs;
	
	struct arrays arrs;
};

struct ptc* init_system(int var, int str, int arr);
void free_system(struct ptc*);

void cmd_acls(struct ptc*);
void cmd_visible(struct ptc*);
void cmd_vsync(struct ptc*);
void cmd_wait(struct ptc*);
void cmd_clear(struct ptc*);

#ifdef ARM9
void system_draw(struct ptc* p);
#endif
#ifdef PC
struct sfRenderWindow;

void system_draw(struct sfRenderWindow*, struct ptc*);
#endif

