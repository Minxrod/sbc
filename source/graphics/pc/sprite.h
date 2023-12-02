#pragma once
#include <SFML/Graphics.h>

struct sprite_info;

struct sprite_array {
	/// List of all sprite vertices 
	sfVertexArray* va;
};

struct sprite_array init_sprite_array(void);

void free_sprite_array(struct sprite_array s);

void add_sprite(struct sprite_array sa, struct sprite_info* s);
