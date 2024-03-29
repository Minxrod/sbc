#pragma once
///
/// @file
/// @brief Defines the resources struct, which contains all graphics resources.
///

#include "common.h"
#include "header.h"

#include <string.h>

#include "strs.h"

//PRG: already defined
//MEM: this is a string
//CHR: working on it
//GRP: working on it
//SCR: working on it (copy)
//COL: working on it (copy)

extern const char* resource_path;

#define GRP_WIDTH 256
#define GRP_HEIGHT 192
#define BG_WIDTH 64
#define BG_HEIGHT 64

// Format: width*height*unit_size
/// Size of one CHR bank
/// Sizes all in bytes
#define CHR_SIZE (256* 64/2)
#define GRP_SIZE (GRP_WIDTH*GRP_HEIGHT*1)
#define SCR_SIZE ( 64* 64*2)
#define COL_SIZE ( 16* 16*2)
/// Size of MEM file: 256 16-bit characters + 4 byte size
#define MEM_SIZE (256*  2+4)

#define CHR_BANKS (4+4+4+8+2)
#define SCR_BANKS 4
#define COL_BANKS 3

#define VISIBLE_CONSOLE 1
#define VISIBLE_BG0 2
#define VISIBLE_BG1 4
#define VISIBLE_SPRITE 8
#define VISIBLE_PANEL 16
#define VISIBLE_GRAPHICS 32
#define VISIBLE_ALL 0x3f

#ifdef ARM9
#define VRAM_BASE (0x06000000)
#define VRAM_BG_BASE (VRAM_BASE + 0x0)
#define VRAM_BG_CHR (VRAM_BG_BASE + 0x8000)
#define VRAM_BG_SCR (VRAM_BG_BASE + 0x0)

#define VRAM_SP_BASE (VRAM_BASE + 0x00400000)
#define VRAM_SP_CHR (VRAM_SP_BASE)
#define VRAM_GRP_CHR (VRAM_SP_BASE + (CHR_SIZE*10))

#define VRAM_LOWER_OFS (0x00200000)

#define VRAM_UPPER_PAL_BG 0x05000000
#define VRAM_UPPER_PAL_SP 0x05000200
#define VRAM_LOWER_PAL_BG 0x05000400
#define VRAM_LOWER_PAL_SP 0x05000600
#endif

enum resource_type {
	TYPE_PRG,
	TYPE_CHR,
	TYPE_SCR,
	TYPE_GRP,
	TYPE_MEM,
	TYPE_COL,
	TYPE_INVALID
};

extern int resource_size[6];

/// Struct containing resources to use
/// Some of these are only stored in VRAM on NDS
struct resources {
	//BGU,D,F,[SPU,S or SPD,K,S]
	const char* search_path;
	
	u8* chr[CHR_BANKS*2];
	// TODO:CODE:LOW Maybe move regen_chr to display?
	bool regen_chr[CHR_BANKS*2];
	
	u16* scr[4*2]; //8K*4*2 -> 64K (VRAM)
	
	u8* grp[4]; //48K*4 -> 192K (RAM) 96K VRAM
	
	u16* col[COL_BANKS*2]; //512*6 -> 3K (2K Palette + 1K VRAM) (may need RAM copy)
	bool regen_col;
	
	u8* key_chr[12]; // 8K*12 -> 96K (RAM)
	
	// 512K of VRAM in use total + 192K~256K RAM (via GRP~SCR)
	// All banks: CHR_SIZE*CHR_BANKS*2 -> 44*8 352K (VRAM)
	u8* all_banks;
	// Col banks: COL_SIZE*COL_BANKS*2 -> 3K
	u16* col_banks;
	
	u16* bg_upper;
	
	struct string mem_str;
	void* mem_ptr;
	union {
		struct {
			u16 chars[MAX_STRLEN];
			u32 size;
		};
		struct {
			u8 data[MEM_SIZE];
		};
	} mem;
	
	u8 visible;
	
	/// RESULT sysvar, set by file operations
	int result;
};

int check_load_res(u8* dest, const char* search_path, const char* name, int type);
int check_load_file(u8* dest, const char* search_path, const char* name, int size);

bool load_file(u8* dest, const char* name, int skip, int len);
bool load_chr(u8* dest, const char* path, const char* name);
bool load_col(u8* dest, const char* path, const char* name);
bool load_scr(u16* dest, const char* path, const char* name);

void init_resource(struct resources* r);
void free_resource(struct resources* r);

struct ptc;

int get_chr_index(struct ptc* p, char* res);

/// Returns a data pointer for the resource
/// Returns NULL, if the resource name was invalid
void* get_resource(struct ptc* p, char* name, int len);

static inline int get_resource_type(void* res){
	if (str_len(res) < 3) return TYPE_INVALID;
	u16 c1 = to_char(str_at_wide(res, 0));
	u16 c2 = to_char(str_at_wide(res, 1));
	if (c1 == 'G'){
		return TYPE_GRP;
	} else if (c1 == 'P'){
		return TYPE_PRG;
	} else if (c1 == 'S'){
		if (c2 == 'C') return TYPE_SCR;
		if (c2 == 'P') return TYPE_CHR;
		return TYPE_INVALID;
	} else if (c1 == 'B'){
		return TYPE_CHR;
	} else if (c1 == 'C'){
		return TYPE_COL;
	} else if (c1 == 'M'){
		return TYPE_MEM;
	} else {
		return TYPE_INVALID;
	}
}

/// Returns a data pointer for the resource
/// Returns NULL, if the resource name was invalid
void* str_to_resource(struct ptc* p, void* name_str);

// Character
void cmd_chrinit(struct ptc* p);
void cmd_chrset(struct ptc* p);
void cmd_chrread(struct ptc* p);

// Color
void cmd_colinit(struct ptc* p);
void cmd_colset(struct ptc* p);
void cmd_colread(struct ptc* p);

// File
void cmd_save(struct ptc* p);
void cmd_load(struct ptc* p);

// MEM$
void sys_mem(struct ptc* p);
void syschk_mem(struct ptc* p);
void sys_result(struct ptc* p);

