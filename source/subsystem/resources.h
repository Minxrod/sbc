#pragma once
///
/// @file
/// @brief Defines the resources struct, which contains all graphics resources.
///

#include "common.h"
#include "header.h"

#include <string.h>

#include "strs.h"

extern const char* resource_path;

/// Number of screens supported by the interpreter.
#define SCREEN_COUNT 2
/// Width of the screen, in pixels
#define SCREEN_WIDTH 256
/// Height of the screen, in pixels
#define SCREEN_HEIGHT 192
/// Expected framerate (frames/second)
#define FRAMERATE 60

/// Width of GRP layer, in pixels.
#define GRP_WIDTH 256
/// Height of GRP layer, in pixels.
#define GRP_HEIGHT 192
/// Width of background layer, in tiles.
#define BG_WIDTH 64
/// Height of background layer, in tiles.
#define BG_HEIGHT 64
/// Width of a single CHR tile, in pixels.
#define CHR_WIDTH 8
/// Height of a single CHR tile, in pixels.
#define CHR_HEIGHT 8
/// Size of a CHR tile, in bytes.
#define CHR_UNIT_SIZE (CHR_WIDTH * CHR_HEIGHT / 2)

// Format: width*height*unit_size
/// Size of one CHR bank, in bytes
#define CHR_SIZE (256*CHR_UNIT_SIZE)
/// Size of one GRP layer, in bytes
#define GRP_SIZE (GRP_WIDTH*GRP_HEIGHT)
/// Size of one background layer, in bytes
#define SCR_SIZE (BG_WIDTH*BG_HEIGHT*sizeof(u16))
/// Size of one COL resource, in bytes
#define COL_SIZE ( 16* 16*2)
/// Size of MEM file: 256 16-bit characters + 4 byte size
#define MEM_SIZE (MAX_STRLEN*sizeof(u16)+4)

/// Number of unique CHR resource banks for a single screen.
#define CHR_BANKS (4+4+4+8+2)
/// Number of unique SCR resources for a single screen.
#define SCR_BANKS 4
/// Number of 256-color COL palettes for a single screen.
#define COL_BANKS 3
/// Number of GRP layers in total. These are shared between all screens.
#define GRP_BANKS 4

/// Bitmask for console visibility
#define VISIBLE_CONSOLE 1
/// Bitmask for panel visibility
#define VISIBLE_PANEL 2
/// Bitmask for foreground BG layer visibility
#define VISIBLE_BG0 4
/// Bitmask for background BG layer visibility
#define VISIBLE_BG1 8
/// Bitmask for sprite visibility
#define VISIBLE_SPRITE 16
/// Bitmask for graphics layer visibility
#define VISIBLE_GRAPHICS 32
/// Bitmask representing all visual systems being enabled
#define VISIBLE_ALL 0x3f

/// Maximum length of filename (without resource type or extension)
#define MAX_RESOURCE_NAME_LENGTH 8
/// Maximum length of resource type string (ex. BGD, SPU, MEM)
#define MAX_RESOURCE_TYPE_LENGTH 5
/// Maximum length of complete PTC-style resource identifier
#define MAX_RESOURCE_STR_LENGTH (MAX_RESOURCE_NAME_LENGTH + 1 + MAX_RESOURCE_TYPE_LENGTH)
/// Separator character between resource type and resource name
#define RESOURCE_SEPARATOR ':'

#define MAX_FILEPATH_LENGTH 255

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
	
	u8* chr[CHR_BANKS*SCREEN_COUNT];
	
	bool regen_chr[CHR_BANKS*SCREEN_COUNT];
	
	u16* scr[SCR_BANKS*SCREEN_COUNT]; //8K*4*2 -> 64K (VRAM)
	
	u8* grp[4]; //48K*4 -> 192K (RAM) 96K VRAM
	
	u16* col[COL_BANKS*SCREEN_COUNT]; //512*6 -> 3K (2K Palette + 1K VRAM) (may need RAM copy)
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

struct ptc;

bool verify_resource_type(const char* resource_type);
bool verify_resource_type_str(const void* res);
bool verify_resource_name(const char* resource_name);
void* get_resource_ptr(struct ptc* p, const char* resource_type);

bool verify_file_type(const char* path, int type);
bool verify_search_file_type(const char* search_path, const char* name, int type);
int check_load_res(u8* dest, const char* search_path, const char* name, int type);
int check_load_file(u8* dest, const char* search_path, const char* name, int size);

int load_file(u8* dest, const char* path, int skip, int len);
bool load_chr(u8* dest, const char* path, const char* name);
bool load_col(u8* dest, const char* path, const char* name);
bool load_scr(u16* dest, const char* path, const char* name);

void init_resource(struct resources* r);
void free_resource(struct resources* r);

int get_chr_index(struct ptc* p, const char* res);

/// Only works on valid resource names - does only minimal checks to determine type.
static inline int get_resource_type(const void* res){
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

// PRG Files
void cmd_new(struct ptc* p);
void cmd_append(struct ptc* p);
void cmd_rename(struct ptc* p);
void cmd_delete(struct ptc* p);

// MEM$
void sys_mem(struct ptc* p);
void syschk_mem(struct ptc* p);
void sys_result(struct ptc* p);

