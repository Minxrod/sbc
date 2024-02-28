#pragma once

struct ptc;
struct display;

#ifdef PC
#include "graphics/pc/display_pc.h"
#endif
#ifdef ARM9
#include "graphics/nds/display_nds.h"
#endif
#ifdef NONE
struct display {
};
#endif

void init_display(struct ptc*);
void free_display(struct display*);

void display_draw_all(struct ptc* p);
void display_draw_text(struct ptc* p, int screen, int prio);
void display_draw_background(struct ptc* p, int screen, int prio);
void display_draw_sprite(struct ptc* p, int screen, int prio);
void display_draw_graphics(struct ptc* p, int screen, int prio);
void display_draw_panel(struct ptc* p, int screen, int prio);

