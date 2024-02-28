#include "graphic.h"

#include "graphics.h"
#include "system.h"

#include <SFML/Graphics.h>

struct graphic init_graphic(int w, int h){
	struct graphic grp = {w, h, 8, 8, 4, 3, sfTexture_create(GRP_WIDTH, GRP_HEIGHT), sfSprite_create()};
	if (!grp.texture){
		iprintf("Failed to create texture!\n");
		abort();
	}
	if (!grp.sprite){
		iprintf("Failed to create sprite!\n");
		abort();
	}
	
	sfSprite_setTexture(grp.sprite, grp.texture, true);
	
	return grp;
}

void free_graphic(struct graphic* g){
	sfSprite_destroy(g->sprite);
	sfTexture_destroy(g->texture);
}

void draw_graphic(struct graphic* g, struct ptc* p, int screen){
	u8 grp[GRP_SIZE*4];
	for (int y = 0; y < GRP_HEIGHT; ++y){
		for (int x = 0; x < GRP_WIDTH; ++x){
			u8 col = p->res.grp[p->graphics.info[screen].displaypage][grp_index(x, y)];
			grp[(x+(y*GRP_WIDTH))*4] = col;
		}
	}
	
	sfTexture_updateFromPixels(g->texture, grp, GRP_WIDTH, GRP_HEIGHT, 0, 0);
}
