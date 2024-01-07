#pragma once

#include "common.h"

#define MAX_SPRITES 100

struct sprites {
	int page;
	
	struct sprite_info {
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
		s8 home_x;
		/// Sprite home y component (pixels)
		s8 home_y;
		
		/// Struct for sprite position.
		struct {
			/// Sprite current x position (pixels)
			fixp x;
			/// Sprite current y position (pixels)
			fixp y;
			/// Change in x position (pixels/frame)
			fixp dx;
			/// Change in y position (pixels/frame)
			fixp dy;
			/// Remaining time to move sprite (frames)
			/// If -1, sprite is done moving.
			int time;
		} pos;
		
		/// Struct for sprite scaling.
		struct {
			/// Current scale multiplier [0.0-2.0]
			fixp s;
			/// Change in scale over time (step/frame)
			fixp ds;
			/// Remaining time to scale sprite (frames)
			/// If 0, sprite is done resizing.
			int time;
		} scale;
		
		/// Struct for sprite rotation
		struct {
			/// Current sprite angle (degrees)
			fixp a;
			/// Change in angle (degrees/frame)
			fixp da;
			/// Remaining time rotate sprite (frames)
			/// If 0, sprite is done rotating.
			int time;
		} angle;
		
		/// Struct for sprite collision detection.
		struct {
			/// Displacement x (pixels)
			fixp dx;
			/// Displacement y (pixels)
			fixp dy;
			/// Hitbox x coordinate (pixels)
			fixp x;
			/// Hitbox y coordinate (pixels)
			fixp y;
			/// Width of hitbox (pixels)
			fixp w;
			/// Height of hitbox (pixels)
			fixp h;
			/// Should hitbox change with sprite scale?
			bool scale_adjust;
			/// Collision mask
			u8 mask;
		} hit;
		
		/// Struct for sprite animation
		struct {
			/// Number of characters to animate through (number of animation frames)
			int chrs;
			/// Time to stay on one character (frames)
			int frames_per_chr;
			/// How many times remaining to loop
			int loop;
			/// Should animation loop forever?
			bool loop_forever;
			/// Current animation frame
			int current_frame;
			/// Current animation character
			int current_chr;
		} anim;
		
		/// Sprite variables (used by `SPSETV`, `SPGETV`)
		fixp vars[8];
	} info[2][MAX_SPRITES];
	
	fixp sphitno;
	fixp sphitx;
	fixp sphity;
	fixp sphitt;
};

void init_sprites(struct sprites* s);
void free_sprites(struct sprites* s);

struct sprite_info init_sprite_info(int id,int chr,int pal,bool horiz,bool vert,int prio,int w, int h);

void step_sprites(struct sprites* s);
bool is_hit(struct sprite_info* a, struct sprite_info* b);

struct ptc;

void cmd_sppage(struct ptc* p);
void cmd_spset(struct ptc* p);
void cmd_spclr(struct ptc* p);
void cmd_sphome(struct ptc* p);

void cmd_spofs(struct ptc* p);
void cmd_spchr(struct ptc* p);
void cmd_spanim(struct ptc* p);
void cmd_spangle(struct ptc* p);
void cmd_spscale(struct ptc* p);

void cmd_spread(struct ptc* p);
void cmd_spsetv(struct ptc* p);
void func_spgetv(struct ptc* p);
void func_spchk(struct ptc* p);

void cmd_spcol(struct ptc* p);
void cmd_spcolvec(struct ptc* p);
void func_sphit(struct ptc* p);
void func_sphitsp(struct ptc* p);
void func_sphitrc(struct ptc* p);
