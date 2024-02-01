#pragma once
/// 
/// @file
/// @brief Defines the system struct, which contains the state of all 
/// interpreter (sub)systems.
/// 

#include "interpreter.h"

#include "subsystem/resources.h"
#include "subsystem/console.h"
#include "subsystem/graphics.h"
#include "subsystem/panel.h"
#include "subsystem/background.h"
#include "subsystem/sprites.h"
#include "input.h"
#include "timers.h"

#include "graphics/display.h"

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
	// upper screen stuff
	struct console console;
	// background stuff
	struct background background;
	// sprites stuff
	struct sprites sprites;
	// graphics pages
	struct graphics graphics;
	// lower screen stuff
	struct panel panel;
	
	// graphics and resources
	struct resources res;
	// generic stuff
	struct input input;
	// timing
	struct timers time;
	
	// actual system stuff
	/// The value/variable stack 
	struct runner exec;
	
	struct value_stack stack;
	
	struct call_stack calls;
	
	struct variables vars;
	
	struct strings strs;
	
	struct arrays arrs;
	
	// rendering information (system-specific)
	struct display display;
};

struct ptc* init_system(int var, int str, int arr);
void free_system(struct ptc*);

void cmd_acls(struct ptc*);
void cmd_visible(struct ptc*);
void cmd_vsync(struct ptc*);
void cmd_wait(struct ptc*);
void cmd_clear(struct ptc*);
void cmd_swap(struct ptc*);
