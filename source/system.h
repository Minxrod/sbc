#pragma once

#include "common.h"
#include "stack.h"
#include "strs.h"
#include "arrays.h"
#include "runner.h"
#include "resources.h"

#include "console.h"

// note that VAR_LIMIT should be a multiple of 2 to ensure that
// the current hash algorithm works
#define VAR_LIMIT 2048
#define STR_LIMIT 4096
#define ARR_LIMIT 262144

// note: members here are stored directly (why?)
struct ptc {
	// upper screen stuff
	struct console console;
//	struct background background;
//	struct sprites sprites;
//	struct graphics graphics;
	// lower screen stuff
//	struct panel panel;
	
	// graphics and resources
	struct resources res;
	
	struct graphics g;
	
	// actual system stuff
	/// The value/variable stack 
	struct runner exec;
	
	struct value_stack stack;
	
	struct call_stack calls; 
	
	struct variables vars;
	
	struct strings strs;
	
	struct arrays arrs;
};

struct ptc* system_init();

#ifdef PC
struct sfRenderWindow;

void system_draw(struct sfRenderWindow*, struct ptc*);
#endif

