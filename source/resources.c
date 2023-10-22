#include "resources.h"

#include "system.h"
#include "error.h"

#include <stdlib.h>
#include <stdio.h>

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

#define VRAM_UPPER_PAL_BG 0x05000000
#define VRAM_UPPER_PAL_SP 0x05000200
#define VRAM_LOWER_PAL_BG 0x05000400
#define VRAM_LOWER_PAL_SP 0x05000600
#endif

void init_resource(struct resources* r){
#ifdef ARM9
	// Assign pointers into VRAM as needed
	for (int lower = 0; lower < 1; ++lower){
		for (int i = 0; i < 12; ++i){
			r->chr[i + CHR_BANKS * lower] = (u8*)(VRAM_BG_CHR + CHR_SIZE * i + VRAM_LOWER_OFS * lower);
		}
		for (int i = 0; i < 10; ++i){
			r->chr[12 + i + CHR_BANKS * lower] = (u8*)(VRAM_SP_CHR + CHR_SIZE * i + VRAM_LOWER_OFS * lower);
		}
		// TODO:IMPL:LOW May need RAM copies
		for (int i = 0; i < 1; ++i){
			r->scr[i + 2 * lower] = (u16*)(VRAM_BG_SCR + SCR_SIZE * i + VRAM_LOWER_OFS * lower);
		}
		r->col[0] = (u16*) VRAM_UPPER_PAL_BG;
		r->col[1] = (u16*) VRAM_UPPER_PAL_SP;
		r->col[2] = (u16*) 0; //TODO:IMPL:HIGH GRP
		r->col[3] = (u16*) VRAM_LOWER_PAL_BG;
		r->col[4] = (u16*) VRAM_LOWER_PAL_SP;
		r->col[5] = (u16*) 0; //TODO:IMPL:HIGH GRP
	}
	for (int i = 0; i < 4; ++i){
		iprintf("GRP%d calloc: %d\n", i, GRP_SIZE);
		r->grp[i] = calloc(GRP_SIZE, 1);
	}
	char* name;
	name = "resources/BGF0.PTC";
	FILE* f = fopen(name, "rb");
	if (!f){
		perror("Failed to load file: %s\n");
		abort();
	}
	fread(r->chr[0], sizeof(u8), HEADER_SIZE, f);
	fread(r->chr[0], sizeof(u8), CHR_SIZE, f);
	fclose(f);
	
	f = fopen("resources/COL0.PTC", "rb");
	if (!f){
		iprintf("Failed to load file: %s\n", name);
		abort();
	}
	fread(r->col[0], sizeof(u8), HEADER_SIZE, f);
	fread(r->col[0], sizeof(u8), COL_SIZE, f);
	fclose(f);
	
	r->bg_upper = (u16*)(VRAM_BG_BASE);
#endif
#ifdef PC
	// allocate memory for resources (needs ~512K)
	// this will serve as "emulated VRAM"
	r->all_banks = calloc(CHR_SIZE, CHR_BANKS * 2);
	r->col_banks = calloc(COL_SIZE, 6); //COL[0-2][U-L]
	// guarantee contiguous regions (useful for generating textures)
	for (int lower = 0; lower < 2; ++lower){
		for (int i = 0; i < CHR_BANKS; ++i){
			r->chr[i + CHR_BANKS * lower] = &r->all_banks[(i + CHR_BANKS * lower) * CHR_SIZE];
		}
		for (int i = 0; i < 2; ++i){
			r->scr[i + 2 * lower] = calloc(1, SCR_SIZE);
		}
		for (int i = 0; i < 3; ++i){
			r->col[i + 3 * lower] = &r->col_banks[(i + lower * 3) * COL_SIZE / 2];
		}
	}
	for (int i = 0; i < 4; ++i){
		r->grp[i] = calloc(GRP_SIZE, 1);
	}
	
	// Load default resource files
	const char* chr_files = 
	"BGF0BGF1BGF0BGF1BGD0BGD1BGD2BGD3BGU0BGU1BGU2BGU3"
	"SPU0SPU1SPU2SPU3SPU4SPU5SPU6SPU7SPS0SPS1"
	"BGF0BGF1BGF0BGF1BGD0BGD1BGD2BGD3BGU0BGU1BGU2BGU3"
	"SPD0SPD1SPD2SPD3SPD4SPD5SPD6SPD7SPS0SPS1";
	const char* col_files = "COL0COL1COL2COL0COL1COL2";
	
	char name[] = "resources/XXXX.PTC";
	
	// TODO:CODE:MED This is shared with NDS?
	for (int i = 0; i < CHR_BANKS*2; ++i){
		for (int j = 0; j < 4; ++j){
			name[10+j] = chr_files[4*i+j];
		}
		FILE* f = fopen(name, "rb");
		if (!f){
			iprintf("Failed to load file: %s\n", name);
			abort(); //TODO:ERR:LOW Handle gracefully? Fallback generated tiles?
		}
		fread(r->chr[i], sizeof(u8), HEADER_SIZE, f);
		fread(r->chr[i], sizeof(u8), CHR_SIZE, f);
		
		fclose(f);
	}
	
	for (int i = 0; i < 6; ++i){
		for (int j = 0; j < 4; ++j){
			name[10+j] = col_files[4*i+j];
		}
		FILE* f = fopen(name, "rb");
		if (!f){
			iprintf("Failed to load file: %s\n", name);
			abort(); //TODO:ERR:LOW Handle gracefully? Fallback generated tiles?
		}
		fread(r->col[i], sizeof(u8), HEADER_SIZE, f);
		fread(r->col[i], sizeof(u8), COL_SIZE, f);
		
		fclose(f);
	}
	
	// Generate PC textures here
	r->chr_tex[0] = gen_chr_texture(r->chr[0], 512);
	r->col_tex = gen_col_texture(r->col[0]);
	
	if (!sfShader_isAvailable()){
		iprintf("Error: Shaders are unavailable!\n");
		abort();
	} else {
		if (!(r->shader = sfShader_createFromFile(NULL, NULL, "resources/bgsp.frag"))){
			iprintf("Error: Shader failed to load!\n");
			abort();
		}
	}
#endif
}

void free_resource(struct resources* r){
#ifdef PC
	// Free memory here
	free(r->all_banks);
	free(r->col_banks);
	for (int i = 0; i < 4; ++i){
		free(r->grp[i]);
	}
	for (int i = 0; i < 4; ++i){
		free(r->scr[i]);
	}
	
	// Destroy textures here
	sfTexture_destroy(r->chr_tex[0]);
	sfTexture_destroy(r->col_tex);
	sfShader_destroy(r->shader);
#endif
#ifdef ARM9
	for (int i = 0; i < 4; ++i){
		free(r->grp[i]);
	}
#endif
}


// Returns a data pointer for the resource
// Returns NULL, if the resource name was invalid
void* get_resource(struct ptc* p, char* name, int len){
	char c = name[0]; //category
	char id = name[1]; //id help for SCU/SPU
	char t = name[2]; //type (within category)
	
	char bank, upper;
	if (len == 5){
		bank = name[3] - '0';
		upper = name[4] == 'U';
	} else if (len == 4){
		if (name[3] >= '0' && name[3] < '7'){
			bank = name[3] - '0';
			upper = 0; //TODO:IMPL:MED depends on BG,SP,GRP page
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
		upper = 0; //TODO:IMPL:MED depends on BG,SP,GRP page
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
		// TODO:IMPL:LOW Does page matter here?
		return p->res.grp[(int)bank];
	}
	p->exec.error = ERR_INVALID_RESOURCE_TYPE;
	return NULL;
}

#ifdef PC
sfTexture* gen_col_texture(u16* src){
	u8 array[4*256*6]; // no {0} because it will all be generated by the end
	
	for (size_t i = 0; i < 256*6; ++i){
		u16 s = src[i];
//		s = ((s & 0xff00) >> 8) | ((s & 0x00ff) << 8);
		
		array[4*i+0] = (s & 0x001f) << 3; //TODO:IMPL:LOW Adjust values to match PTC
		array[4*i+1] = ((s & 0x03e0) >> 2) | ((src[2*i] & 0x8000) >> 13);
		array[4*i+2] = ((s & 0x7c00) >> 7);
		array[4*i+3] = ((i % 16) || (((i % 3) == 2) && (i != 0))) ? 255 : 0;
//		iprintf("%ld:(%d,%d,%d,%d)",i,array[4*i+0],array[4*i+1],array[4*i+2],array[4*i+3]);
	}
//	iprintf("\n");
	
	sfTexture* tex = sfTexture_create(256, 6);
	if (!tex){
		return NULL;
	}
	sfTexture_updateFromPixels(tex, array, 256, 6, 0, 0);
	return tex;
}

sfTexture* gen_chr_texture(u8* src, size_t size){
	u8 array[4*size*64];
	for (size_t i = 0; i < size; ++i){
		size_t x = i % 32;
		size_t y = i / 32;
		for (int cx = 0; cx < 8; ++cx){
			for (int cy = 0; cy < 8; ++cy){
				u8 c = src[32*i+cx/2+4*cy];
				array[4*(8*x+cx+256*(8*y+cy))] = ((cx & 1) ? (c >> 4) & 0x0f : c & 0x0f);
			}
		}
	}
	
	sfTexture* tex = sfTexture_create(256, size/4);
	if (!tex){
		return NULL;
	}
	sfTexture_updateFromPixels(tex, array, 256, size/4, 0, 0);
	return tex;
}

#endif
