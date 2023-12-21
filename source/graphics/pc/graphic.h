#pragma once

struct sfTexture;

struct ptc;

struct graphic {
	int width;
	
	int height;
	
	int char_width;
	
	int char_height;
	
	int block_width;
	
	int block_height;
	
	/// Rendered graphics texture
	struct sfTexture* texture;
	
	/// Sprite to render texture to
	struct sfSprite* sprite;
};

struct graphic init_graphic(int w, int h);

void draw_graphic(struct graphic* g, struct ptc* p, int screen);

void free_graphic(struct graphic* g);
