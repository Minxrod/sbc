#pragma once

#include "common.h"
#include "stack.h"
#include "strs.h"
#include "arrays.h"
#include "runner.h"

#include "console.h"

#define BG_WIDTH 64
#define BG_HEIGHT 64

#define SPRITE_MAX 100

#define GRAPHICS_WIDTH 256
#define GRAPHICS_HEIGHT 192

struct bg {
	s32 x;
	s32 y;
	u16 tiles[BG_WIDTH][BG_HEIGHT];
};

struct spr {
	/// Is the sprite currently enabled?
	bool active;
	/// Sprite ID (control number)
	u8 id;
	/// Character code [0-511]
	u16 chr;
	/// Palette [0-15]
	u8 pal;
	/// Width of sprite (pixels)
	u8 w;
	/// Height of sprite (pixels)
	u8 h;
	/// Sprite draw priority [0-3]
	u8 prio;
	/// Is sprite flipped horizontally?
	bool flip_x;
	/// Is sprite flipper vertically?
	bool flip_y;
	
	/// Sprite home x component (pixels)
	s32 home_x;
	/// Sprite home y component (pixels)
	s32 home_y;
	
	/// Struct for sprite position.
	struct {
		/// Sprite current x position (pixels)
		s32 x;
		/// Sprite current y position (pixels)
		s32 y;
		/// Change in x position (pixels/frame)
		s32 dx;
		/// Change in y position (pixels/frame)
		s32 dy;
		/// Remaining time to move sprite (frames)
		/// If -1, sprite is done moving.
		s32 time;
	} pos;
	
	/// Struct for sprite scaling.
	struct {
		/// Current scale multiplier [0.0-2.0]
		s32 s;
		/// Change in scale over time (step/frame)
		s32 ds;
		/// Remaining time to scale sprite (frames)
		/// If 0, sprite is done resizing.
		u32 time;
	} scale;
	
	/// Struct for sprite rotation
	struct {
		/// Current sprite angle (degrees)
		s32 a;
		/// Change in angle (degrees/frame)
		s32 da;
		/// Remaining time rotate sprite (frames)
		/// If 0, sprite is done rotating.
		u32 time;
	} angle;
	
	/// Struct for sprite collision detection.
	struct {
		/// Displacement x (pixels)
		s32 dx;
		/// Displacement y (pixels)
		s32 dy;
		/// Hitbox x coordinate (pixels)
		s32 x;
		/// Hitbox y coordinate (pixels)
		s32 y;
		/// Width of hitbox (pixels)
		s32 w;
		/// Height of hitbox (pixels)
		s32 h;
		/// Should hitbox change with sprite scale?
		bool scale_adjust;
		/// Collision mask
		u8 mask;
	} hit;
	
	/// Struct for sprite animation
	struct {
		/// Number of characters to animate through (number of animation frames)
		u32 chrs;
		/// Time to stay on one character (frames)
		u32 frames_per_chr;
		/// How many times remaining to loop
		u32 loop;
		/// Should animation loop forever?
		bool loop_forever;
		/// Current animation frame
		u32 current_frame;
		/// Current animation character
		u32 current_chr;
	} anim;
	
	/// Sprite variables (used by `SPSETV`, `SPGETV`)
	s32 vars[8];
};

struct background {
	struct bg layer[2];
};

struct sprites {
	struct spr s[100];
};

struct grp {
	// arranged as character data
	u8 data[GRAPHICS_WIDTH*GRAPHICS_WIDTH];
};

struct graphics {
	struct grp page[4];
};

struct panel {
	struct console text;
	struct bg background;
	struct sprites keys;
};

// note: members here are stored directly (why?)
struct ptc {
	// upper screen stuff
	struct console console;
//	struct background background;
//	struct sprites sprites;
//	struct graphics graphics;
	// lower screen stuff
//	struct panel panel;
	
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

