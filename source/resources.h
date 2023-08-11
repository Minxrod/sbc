#pragma once

#include "common.h"

//PRG: already defined
//MEM: this is a string
//CHR: working on it
//GRP: working on it
//SCR: working on it (copy)
//COL: working on it (copy)

// Format: width*height*unit_size
/// Size of one CHR bank
/// Sizes all in bytes
#define CHR_SIZE (256* 64/2)
#define GRP_SIZE (256*192*1)
#define SCR_SIZE ( 64* 64*2)
#define COL_SIZE ( 16* 16*2)

struct ptc;

#define CHR_BANKS (4+4+4+8+2)

/// Struct containing resources to use
/// Some of these are only stored in VRAM on NDS
struct resources {
	// TODO: PRG type?
//	struct program* prg;
	// TODO: MEM type
	//BGU,D,F,[SPU,S or SPD,K,S]
	// 512K of VRAM in use total + 192K~256K RAM (via GRP~SCR)
	u8* chr[CHR_BANKS*2]; //8K*22*2 -> 44*8 352K (VRAM)
	// TODO: ordering
	
	u16* scr[2*2]; //8K*4*2 -> 64K (VRAM)
	
	u8* grp[4]; //48K*4 -> 192K (RAM) 96K VRAM
	
	u16* col[6]; //512*6 -> 3K (2K Palette + 1K VRAM) (may need RAM copy)
};

/// Struct containing graphics to be rendered
/// On an NDS system, these will all be pointers to VRAM
/// On PC, these are just pointers to regular memory that will be read by
/// SFML compatibility/conversion code ported from PTC-EmkII
struct graphics {
	u16* bg_upper;
	u8* chr_bg_upper;
	
	u16* bg_lower;
	u8* chr_bg_lower;
	
	u8* chr_sp_upper;
	u8* grp_sp_upper;
	
	u8* chr_sp_lower;
	u8* grp_sp_lower;
	// TODO: Do colors need to be here...?
};

void resource_init(struct resources* r);

/// Returns a data pointer for the resource
/// Returns NULL, if the resource name was invalid
void* get_resource(struct ptc* p, char* name, int len);

#ifdef PC
struct sfTexture;
/// PC-only function to generate SFML textures.
struct sfTexture* gen_chr_texture(u8* src, size_t size);
struct sfTexture* gen_col_texture(u16* src);
#endif

