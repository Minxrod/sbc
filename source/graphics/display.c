#include "display.h"

#if !defined(SDL2) && !defined(SFML) && !defined(ARM9)
void init_display(struct sbc* s){(void)s;}
void free_display(struct display* d){(void)d;}
#endif
