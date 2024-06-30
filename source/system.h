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
#include "extension/sbc_memory.h"
#include "input.h"
#include "timers.h"

#ifdef PC
#include "graphics/pc/display_pc.h"
#endif
#ifdef ARM9
#include "graphics/nds/display_nds.h"
#endif

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

/// The system struct. This contains the entire interpreter state.
///
// TODO:CODE:LOW change name to sbc at some point?
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
	// memory access system (system-specific)
	struct memory_api memapi;
};

struct ptc* init_system(int var, int str, int arr, bool headless);
void free_system(struct ptc*);

/// Information struct containing program source and the system to run it on.
///
/// Used for launching the main thread on PC.
struct launch_info {
	/// Pointer to system struct
	struct ptc* p;
	/// Pointer to program source.
	struct program* prg;
	/// Name of program to autoboot
	char* prg_filename;
};

// This is intended to accept a struct launch_info, but is void* for use in thrd_create
int launch_system(void*);
// Run a program on a system
int token_and_run(struct ptc* p, struct program* prg, struct bytecode* bc, int tokopts);

void cmd_acls(struct ptc*);
void cmd_visible(struct ptc*);
void cmd_vsync(struct ptc*);
void cmd_wait(struct ptc*);
void cmd_clear(struct ptc*);
void cmd_swap(struct ptc*);
void cmd_sort(struct ptc* p);
void cmd_rsort(struct ptc* p);
void cmd_files(struct ptc* p);
