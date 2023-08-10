#include "resources.h"

#include "system.h"
#include "error.h"

#include <stdlib.h>

#ifdef PC
#include <SFML/Graphics.h>
#endif

#ifdef ARM9
#define VRAM_BASE (0x06000000)
#define VRAM_BG_BASE (VRAM_BASE + 0x0)
#define VRAM_BG_CHR (VRAM_BG_BASE + 0x8000)
#define VRAM_BG_SCR (VRAM_BG_BASE + 0x0)

#define VRAM_SP_BASE (VRAM_BASE + 0x00200000)
#define VRAM_SP_CHR (VRAM_SP_BASE)
#define VRAM_GRP_CHR (VRAM_SP_BASE + CHR_SIZE*10)

#define VRAM_LOWER_OFS (0x00400000)
#endif

void resource_init(struct resources* r){
#ifdef ARM9
	// Assign pointers into VRAM as needed
	for (int lower = 0; lower < 1; ++lower){
		for (int i = 0; i < 12; ++i){
			r->chr[i + CHR_BANKS * lower] = (u8*)(VRAM_BG_CHR + CHR_SIZE * i + VRAM_LOWER_OFS * lower);
		}
		for (int i = 0; i < 10; ++i){
			r->chr[12 + i + CHR_BANKS * lower] = (u8*)(VRAM_SP_CHR + CHR_SIZE * i + VRAM_LOWER_OFS * lower);
		}
		// TODO: May need RAM copies
		for (int i = 0; i < 1; ++i){
			r->scr[i + 2 * lower] = (u16*)(VRAM_BG_SCR + SCR_SIZE * i + VRAM_LOWER_OFS * lower);
		}
		//TODO: COL
	}
	for (int i = 0; i < 4; ++i){
		r->grp[i] = calloc(256, 192); // TODO: replace with constants
	}
#endif
#ifdef PC
	// allocate memory for resources (needs ~512K)
	// this will serve as "emulated VRAM"
	u8* all_banks = calloc(CHR_SIZE, CHR_BANKS * 2);
	// guarantee contiguous regions (useful for generating textures)
	for (int lower = 0; lower < 1; ++lower){
		for (int i = 0; i < CHR_BANKS; ++i){
			r->chr[i + CHR_BANKS * lower] = &all_banks[i * CHR_SIZE + CHR_BANKS * lower];
		}
		for (int i = 0; i < 1; ++i){
			r->scr[i + 2 * lower] = calloc(1, SCR_SIZE);
		}
		//TODO: COL
		for (int i = 0; i < 3; ++i){
			r->col[i + 3 * lower] = calloc(1, COL_SIZE);
		}
	}
	for (int i = 0; i < 4; ++i){
		r->grp[i] = calloc(256, 192); // TODO: replace with constants
	}
#endif
}

// Returns a data pointer for the resource
// Returns NULL, if the resource name was invalid
void* get_resource(struct ptc* p, char* name, int len){
	char c = name[0]; //category
	char id = name[1]; //id help for SCU/SPU
	char t = name[2]; //type (within category)
	// TODO: Mem, maybe? or not?
	
	char bank, upper;
	if (len == 5){
		bank = name[3] - '0';
		upper = name[4] == 'U';
	} else if (len == 4){
		if (name[3] >= '0' && name[3] < '7'){
			bank = name[3] - '0';
			upper = 0; //TODO: depends on BG,SP,GRP page
		} else if (name[3] == 'U'){
			bank = 0;
			upper = 1;
		} else if (name[3] == 'L'){
			bank = 0;
			upper = 0;
		} else {
			p->exec.error = ERR_INVALID_RESOURCE_TYPE;
			return NULL;
		}
	} else if (len == 3){
		bank = 0;
		upper = 0; //TODO: depends on BG,SP,GRP page
	} else {
		p->exec.error = ERR_INVALID_RESOURCE_TYPE;
		return NULL;
	}
	if (c == 'B'){ //BGU, BGF, BGD
		if (t == 'U'){
			return p->res.chr[8+CHR_BANKS*upper+bank];
		} else if (t == 'F'){
			return p->res.chr[0+CHR_BANKS*upper+bank];
		} else if (t == 'D'){
			return p->res.chr[4+CHR_BANKS*upper+bank];
		}
	} else if (c == 'S'){ //SPU, SPS, SPD, SCU
		if (t == 'U'){
			if (id == 'C'){
				return p->res.scr[2+bank+4*upper];
			} else if (id == 'P'){
				if (!upper)
					return p->res.chr[12+bank];
			}
		} else if (id == 'S'){
			return p->res.chr[20+bank+CHR_BANKS*upper];
		} else if (id == 'D'){
			return p->res.chr[12+bank+CHR_BANKS*upper];
		} else if (id == 'K'){ // yeah, this one normally isn't accessible
			return p->res.chr[16+bank+CHR_BANKS*upper];
		}
	} else if (c == 'C'){ //COL
		return p->res.col[3*upper+bank];
	} else if (c == 'G'){ //GRP
		// TODO: Does page matter here?
		return p->res.grp[(int)bank];
	}
	p->exec.error = ERR_INVALID_RESOURCE_TYPE;
	return NULL;
}

#ifdef PC
sfTexture* gen_chr_texture(u8* src, size_t size){
	u8 array[4*256*64] = {0};
	for (size_t i = 0; i < size; ++i){
		size_t x = i % 32;
		size_t y = i / 32;
		for (int cx = 0; cx < 8; ++cx){
			for (int cy = 0; cy < 8; ++cy){
				u8 c = src[32*i+cx/2+4*cy];
				array[4*(8*x+cx+256*(8*y+cy))] = ((cx & 1) ? (c >> 4) & 0x0f : c & 0x0f) << 4;
				// NOTE: needed to set opacity to max I guess?
				// TODO: remove when shaders or software renderer implemented
				array[4*(8*x+cx+256*(8*y+cy))+3] = 255; //((cx & 1) ? (c >> 4) & 0x0f : c & 0x0f);
			}
		}
	}
	
	sfTexture* tex = sfTexture_create(256, 64);
	if (!tex){
		return NULL;
	}
	sfTexture_updateFromPixels(tex, array, 256, 64, 0, 0);
	return tex;
}

#endif
