#include "resources.h"

#include "common.h"
#include "header.h"
#include "program.h"
#include "stack.h"
#include "strs.h"
#include "system.h"
#include "error.h"
#include "extension/compress.h"
#include "vars.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILE_ERROR(msg) do {\
	iprintf(msg " [path=%s]\n", path);\
	return 0;\
} while (0)

#define FILE_OK() do {\
	fclose(f);\
	return size_read;\
} while (0)

/// Error indication and file cleanup
#define FILE_CLOSE(msg) do {\
	fclose(f);\
	FILE_ERROR(msg);\
} while (0)

/// Error checking after file operation
#define CHECK_FILE_ERROR(msg) do {\
if (ferror(f)){\
	FILE_CLOSE(msg);\
} } while(0)

const char* resource_path = "resources/";

const char* internal_type_str[] = {
	"PETC0300RPRG",
	"PETC0100RCHR",
	"PETC0100RSCR",
	"PETC0100RGRP",
	"PETC0200RMEM",
	"PETC0100RCOL",
};

bool create_path(char dest[MAX_FILEPATH_LENGTH+1], const char* base, const char* name, const char* ext){
	if (strlen(base) + strlen(name) + strlen(ext) > MAX_FILEPATH_LENGTH){
		return false; // name too long
	}
	strcpy(dest, base);
	strcpy(dest + strlen(base), name);
	strcpy(dest + strlen(base) + strlen(name), ext);
	return true; // success!
}

bool create_path_from_str(char dest[MAX_FILEPATH_LENGTH+1], const char* base, const char* name, const char* ext){
	if (strlen(base) + str_len(name) + strlen(ext) > MAX_FILEPATH_LENGTH){
		return false; // name too long
	}
	strcpy(dest, base);
	str_char_copy(name, (u8*)dest + strlen(base));
	strcpy(dest + strlen(base) + str_len(name), ext);
	return true; // success!
}

bool verify_file_type(const char* path, const int type){
	FILE* f = fopen(path, "rb");
	if (!f){
		FILE_ERROR("Failed to load file");
	}
	char check_type[4];
	fread(check_type, sizeof(char), 4, f);
	CHECK_FILE_ERROR("Failed to read file magic ID");
	fseek(f, 0, SEEK_SET);
	CHECK_FILE_ERROR("Failed to seek to file start");

	int size_read;
	if (!strncmp(check_type, "PX01", 4)){
		struct ptc_header h;

		assert(LITTLE_ENDIAN);
		// only works on little-endian devices...
		// (reading into header memory directly)
		int size_read = fread(&h, sizeof(char), PRG_HEADER_SIZE, f);
		CHECK_FILE_ERROR("Could not read header correctly");
		if (size_read < PRG_HEADER_SIZE){
			FILE_CLOSE("Could not read full header");
		}
		fclose(f);
		return !strncmp(h.type_str, internal_type_str[type], HEADER_TYPE_STR_SIZE);
	}
	if (!strncmp(check_type, "PETC", 4)){
		// internal format
		if (fseek(f, 0, SEEK_SET)){
			FILE_CLOSE("Failed to seek to internal type");
		}
		// read header
		char header_type[HEADER_TYPE_STR_SIZE];
		size_read = fread(header_type, sizeof(char), HEADER_TYPE_STR_SIZE, f);
		CHECK_FILE_ERROR("Could not read program header");
		if (size_read < HEADER_TYPE_STR_SIZE){
			FILE_CLOSE("Could not read complete program header");
		}
		fclose(f);
		return !strncmp(header_type, internal_type_str[type], HEADER_TYPE_STR_SIZE);
	}
	FILE_CLOSE("Unknown PRG file format");
}

bool verify_search_file_type(const char* search_path, const char* name, int type){
	char path[MAX_FILEPATH_LENGTH+1] = {0};
	if (!create_path(path, search_path, name, ".PTC")){
		FILE_ERROR("File path too long");
	}

	return verify_file_type(path, type);
}

int resource_size[6] = {
	0, CHR_SIZE, SCR_SIZE, GRP_SIZE, MEM_SIZE, COL_SIZE,
};

/// Load from a file, skipping past part of it if needed and reading only a certain length
int load_file(u8* dest, const char* path, int skip, int len){
	FILE* f = fopen(path, "rb");
	if (!f){
		FILE_ERROR("Failed to load file");
	}
	if (fseek(f, skip, SEEK_SET) == -1){
		FILE_CLOSE("Failed to fseek (skip)");
	}
	int read = fread(dest, sizeof(u8), len, f);
	CHECK_FILE_ERROR("Failed to fread file");
	fclose(f);
	return read;
}

int check_save_res(void* src, const char* search_path, const char* name, int type){
	int size_read;
	char path[MAX_FILEPATH_LENGTH+1] = {0};
	if (!create_path(path, search_path, name, ".PTC")){
		FILE_ERROR("File name too long");
	}

	if (type == TYPE_PRG){
		FILE* f = fopen(path, "wb");
		if (!f){
			FILE_ERROR("Could not open file for writing");
		}
		struct program prg = *(struct program*)src;

		char data[12] = {0};
		data[8] = prg.size & 0xff;
		data[9] = (prg.size >> 8) & 0xff;
		data[10] = (prg.size >> 16) & 0xff;

		fwrite(internal_type_str[type], 1, 12, f);
		CHECK_FILE_ERROR("Failed to write file type");
		fwrite(data, 1, 12, f);
		CHECK_FILE_ERROR("Failed to write program header");
		size_read = fwrite(prg.data, 1, prg.size, f);
		CHECK_FILE_ERROR("Failed to write program data");
		FILE_OK(); // close and return size_read
	} else if (type > TYPE_PRG && type < TYPE_INVALID) {
		FILE* f = fopen(path, "wb");
		if (!f){
			FILE_ERROR("Could not open file for writing");
		}

		fwrite(internal_type_str[type], 1, 12, f);
		CHECK_FILE_ERROR("Failed to write file type");
		size_read = fwrite((u8*)src, 1, resource_size[type], f);
		CHECK_FILE_ERROR("Failed to write resource data");
		FILE_OK(); // close and return size_read
	} else {
		FILE_ERROR("Unimplemented or invalid resource save type");
	}
}

// TODO:ERR:LOW Add file type validation
int check_load_res(u8* dest, const char* search_path, const char* name, int type){
	int size_read;
	char path[MAX_FILEPATH_LENGTH+1] = {0};
	if (!create_path(path, search_path, name, ".PTC")){
		FILE_ERROR("File name too long");
	}

	if (type == TYPE_PRG){
		FILE* f = fopen(path, "rb");
		if (!f){
			FILE_ERROR("Failed to load file");
		}
		char check_type[4];
		fread(check_type, sizeof(char), 4, f);
		CHECK_FILE_ERROR("Failed to read file magic ID");
		fseek(f, 0, SEEK_SET);
		CHECK_FILE_ERROR("Failed to seek to file start");

		if (!strncmp(check_type, "PX01", 4)){
			struct ptc_header h;

			assert(LITTLE_ENDIAN);
			// only works on little-endian devices...
			// (reading into header memory directly)
			size_read = fread(&h, sizeof(char), PRG_HEADER_SIZE, f);
			CHECK_FILE_ERROR("Could not read header correctly");
			if (size_read < PRG_HEADER_SIZE){
				FILE_CLOSE("Could not read full header");
			}
			// Assumed successful header read - now load file contents into dest
			size_read = fread(dest, sizeof(char), h.prg_size, f);
			return size_read;
		} else if (!strncmp(check_type, "PETC", 4)){
			// internal format
			if (fseek(f, 12, SEEK_SET)){
				FILE_CLOSE("Failed to seek past type");
			}
			// read header
			u8 prg_header[12];
			size_read = fread(prg_header, sizeof(u8), 12, f);
			CHECK_FILE_ERROR("Could not read program header");
			if (size_read < 12){
				FILE_CLOSE("Could not read complete program header");
			}
			// TODO:IMPL:LOW Handle packaged program files
			// get size from header
			int prg_size = prg_header[8] | prg_header[9] << 8 | prg_header[10] << 16;

			size_read = fread(dest, sizeof(char), prg_size, f);
			return size_read;
		} else {
			FILE_ERROR("Unknown PRG file format");
		}
	} else if (type == TYPE_MEM){
		return check_load_file(dest, search_path, name, resource_size[type]);
	} else if (type >= TYPE_CHR && type <= TYPE_COL){
		return check_load_file(dest, search_path, name, resource_size[type]);
	}
	FILE_ERROR("Unknown resource type");
}

// Checks the file type and loads the correct data from the file.
// Note: path, name must be null-terminated
int check_load_file(u8* dest, const char* search_path, const char* name, int size){
	int size_read;
	char path[MAX_FILEPATH_LENGTH+1] = {0};
	if (!create_path(path, search_path, name, ".PTC")){
		FILE_ERROR("File name too long");
	}

	FILE* f = fopen(path, "rb");
	if (!f){
		FILE_ERROR("Failed to load file");
	}
	char check_type[4] = {0};
	fread(check_type, sizeof(char), 4, f);
	CHECK_FILE_ERROR("Failed to check file type");
	// Note: strncmp returns 0 if equal
	if (!strncmp(check_type, "PX01", 4)){
		// Petit Computer SD format file, header: skip header info
		if (fseek(f, HEADER_SIZE, SEEK_SET)){
			FILE_CLOSE("Failed to seek past header");
		}

		size_read = fread(dest, sizeof(u8), size, f);
		CHECK_FILE_ERROR("Failed to read file");
		FILE_OK();
	} else if (!strncmp(check_type, "PETC", 4)){
		// Petit Computer internal file format: skip type
		if (fseek(f, 12, SEEK_SET)){
			FILE_CLOSE("Failed to seek past type");
		}
		size_read = fread(dest, sizeof(u8), size, f);
		CHECK_FILE_ERROR("Failed to read file");
		FILE_OK();
	} else if (!strncmp(check_type, "SBCC", 4)){
		// SBC compressed format (something a bit more inventive)
		if (fseek(f, 4, SEEK_SET)){
			FILE_CLOSE("Failed to seek past file type id");
		}
		u8 header[4];
		fread(header, sizeof(u8), 4, f);
		CHECK_FILE_ERROR("Failed to read header");

		// Determine size of file
		if (fseek(f, 0, SEEK_END)){
			FILE_CLOSE("Failed to seek to end of file");
		}
		int file_size = ftell(f);
		if (file_size < 0){
			FILE_CLOSE("Failed to determine file size");
		}
		rewind(f); // resets errors apparently

		// Use destination as temporary storage
		// causes brief artifacting but requires zero extra memory
		u8* compressed = dest + size - file_size; // places file at end of destination
		size_read = fread(compressed, sizeof(u8), file_size, f);
		CHECK_FILE_ERROR("Failed to read file");
		compressed += 8; // skip header

		int bits = header[0];
		int expected_size = header[1] | (header[2] << 8) | (header[3] << 16);
		(void)expected_size; // for when assert fails
		assert(expected_size == size); // request == expected
		sbc_decompress(compressed, dest, size, bits);

		FILE_OK();
	} else {
		FILE_CLOSE("Failed to load file (Unknown format)");
	}
}

// Load from file `name` into dest
// returns false on failure
bool load_chr(u8* dest, const char* path, const char* name){
	return check_load_file(dest, path, name, CHR_SIZE);
}

// load from file `name` into dest
// returns false on failure
bool load_col(u8* dest, const char* path, const char* name){
	return check_load_file(dest, path, name, COL_SIZE);
}

// load from file `name` into dest
// returns false on failure
bool load_scr(u16* dest, const char* path, const char* name){
	return check_load_file((u8*)dest, path, name, SCR_SIZE);
}

bool load_grp(u8* dest, const char* path, const char* name){
	return check_load_file(dest, path, name, GRP_SIZE);
}

void init_col(struct resources* r){
	char name[] = "COLX.PTC";
	for (int i = 0; i < SCREEN_COUNT * COL_BANKS; ++i){
		name[3] = '0' + (i % COL_BANKS); // col_files[4*i+3];
		load_col((u8*)r->col[i], resource_path, name);
	}
}

void init_resource(struct resources* r){
	static_assert(sizeof(r->mem) == MEM_SIZE, "MEM data must be able to store entire file contents");

	r->search_path = "programs/"; // Program resource search path
#ifdef ARM9
	// Assign pointers into VRAM as needed
	for (int lower = 0; lower <= 1; ++lower){
		for (int i = 0; i < 12; ++i){
			r->chr[i + CHR_BANKS * lower] = (u8*)(VRAM_BG_CHR + CHR_SIZE * i + VRAM_LOWER_OFS * lower);
		}
		for (int i = 12; i < CHR_BANKS; ++i){
			r->chr[i + CHR_BANKS * lower] = (u8*)(VRAM_SP_CHR + CHR_SIZE * (i - 12) + VRAM_LOWER_OFS * lower);
		}
		for (int i = 0; i < SCR_BANKS; ++i){
			r->scr[i + SCR_BANKS * lower] = (u16*)(VRAM_BG_SCR + SCR_SIZE * i + VRAM_LOWER_OFS * lower);
		}
		for (int i = 0; i < COL_BANKS; ++i){
			r->col[i + COL_BANKS * lower] = calloc_log("init_resource col", COL_SIZE, 1);
		}
	}
#endif //ARM9
#ifdef PC
	// allocate memory for resources (needs ~512K)
	// this will serve as "emulated VRAM"
	r->all_banks = calloc_log("init_resource all_banks", CHR_SIZE, CHR_BANKS * SCREEN_COUNT);
	r->col_banks = calloc_log("init_resource col_banks", COL_SIZE, COL_BANKS * SCREEN_COUNT); //COL[0-2][U-L]
	// guarantee contiguous regions (useful for generating textures)
	for (int lower = 0; lower <= 1; ++lower){
		for (int i = 0; i < CHR_BANKS; ++i){
			r->chr[i + CHR_BANKS * lower] = &r->all_banks[(i + CHR_BANKS * lower) * CHR_SIZE];
		}
		for (int i = 0; i < SCR_BANKS; ++i){
			r->scr[i + SCR_BANKS * lower] = calloc_log("init_resource scr", 1, SCR_SIZE);
		}
		for (int i = 0; i < COL_BANKS; ++i){
			r->col[i + COL_BANKS * lower] = &r->col_banks[(i + COL_BANKS * lower) * COL_SIZE / sizeof(u16)];
//			iprintf("COL%d %p\n", i + COL_BANKS * lower, (void*)r->col[i + COL_BANKS * lower]);
		}
	}
#endif //PC
	// Shared code
	for (int i = 0; i < 4; ++i){
		r->grp[i] = calloc_log("init_resource grp", GRP_SIZE, 1);
	}
	// Store these in memory for faster load times on keyboard switching
	for (int i = 0; i < 12; ++i){
		r->key_chr[i] = calloc_log("init_resource key_chr", CHR_SIZE, 1);
	}
	const char* key_files[12] = {
		"resources/SPD6.PTC", "resources/SPD7.PTC",
		"resources/makeALPHA_SHIFT.NCGR","resources/makeALPHA_SHIFT.NCGR",
		"resources/makeKIGOU.NCGR","resources/makeKIGOU.NCGR",
		"resources/makeKIGOU_SHIFT.NCGR","resources/makeKIGOU_SHIFT.NCGR",
		"resources/makeKANA.NCGR","resources/makeKANA.NCGR",
		"resources/makeKANA_SHIFT.NCGR","resources/makeKANA_SHIFT.NCGR",
	};
	const int key_files_header_size[12] = {
		12, 12,
		48, 48,
		48, 48,
		48, 48,
		48, 48,
		48, 48,
	};

	// Load default resource files
	const char* chr_files =
	"BGF0BGF1BGF0BGF1BGD0BGD1BGD2BGD3BGU0BGU1BGU2BGU3"
	"SPU0SPU1SPU2SPU3SPU4SPU5SPU6SPU7SPS0SPS1"
	"BGF0BGF1BGF0BGF1BGD0BGD1BGD2BGD3BGU0BGU1BGU2BGU3"
	"SPD0SPD1SPD2SPD3SPD4SPD5SPD6SPD7SPS0SPS1";

	char name[] = "XXXX.PTC";

	for (int i = 0; i < CHR_BANKS * 2; ++i){
		for (int j = 0; j < 4; ++j){
			name[j] = chr_files[4*i+j];
		}
		load_chr(r->chr[i], resource_path, name);
	}

	// Initialize color palettes
	init_col(r);

	// Load keyboard sprites in-memory for fast access
	for (int i = 0; i < 12; ++i){
		load_file(r->key_chr[i], key_files[i], key_files_header_size[i] + (i >= 2 && i % 2 ? CHR_SIZE : 0), CHR_SIZE);
	}
	// Initialize MEM$
	r->mem_str = (struct string){
		.type=STRING_WIDE,
		.len=0,
		.uses=1,
		.ptr.w = r->mem.chars
	};
	r->mem_ptr = &r->mem_str;
	// Initialize RESULT
	r->result = 1;
	// Initialize VISIBLE status
	r->visible = VISIBLE_ALL;
}

void free_resource(struct resources* r){
#ifdef PC
	// Free memory here
	free_log("free_resource all_banks", r->all_banks);
	free_log("free_resource col_banks", r->col_banks);
	for (int i = 0; i < 2*SCR_BANKS; ++i){
		free_log("free_resource scr", r->scr[i]);
	}
#endif
	// Shared code
	for (int i = 0; i < 4; ++i){
		free_log("free_resource grp", r->grp[i]);
	}
	for (int i = 0; i < 12; ++i){
		free_log("free_resource key_chr", r->key_chr[i]);
	}
}

// Format of validation string is [resource string][maximum index][page status]
// resource string should match exactly.
// Any number between 0 and maximum index should be accepted.
// If maximum index is unspecified, then anything is permitted past the resource string.
//
// If page status is...
// - P, then both U and L are accepted.
// - U or L, then page must match U or L, or be unspecified
// - unspecified, then anything can follow.
// To unlock extra resources normally inaccessible in PTC, uncomment last line.
const char* valid_resource_types[] = {
	"PRG",
	"MEM",
	"BGU3P", "BGD1P", "BGF0P", "SPU7U", "SPS1P", "SPD3L",
	"COL2P",
	"SCU1P",
	"GRP3",
//	"SPK3L", "BGD3P", "BGF3P",
	NULL,
};

/// Validates a resource type string.
///
/// Checks that a resource type string is one of the expected values.
/// This includes all variations of valid resources, such as BGU0, GRP, SPS0L, etc.
/// The resource type string is expected to be terminated by a colon or (if part of a larger string) or a null-terminator (if standalone).
bool verify_resource_type(const char* resource_type){
	char* separator = strchr(resource_type, RESOURCE_SEPARATOR);
	if (separator && separator - resource_type > MAX_RESOURCE_TYPE_LENGTH){
		return false; // resource type is too long; always invalid
	}
	if (!separator && strlen(resource_type) > MAX_RESOURCE_TYPE_LENGTH){
		return false;
	}
	// Verify type component
	// resource type string must be validated
	bool valid = false;
	for (const char** valid_str_ptr = valid_resource_types; *valid_str_ptr != NULL; ++valid_str_ptr){
		const char* valid_str = *valid_str_ptr;
		if (strncmp(valid_str, resource_type, 3)) {
			continue; // base string doesn't match
		}
		if (valid_str[3] == '\0'){
			valid = true; // MEM or PRG allow nearly anything to follow
			break;
		}
		char index = resource_type[3];
		if (index == RESOURCE_SEPARATOR || index == '\0'){
			valid = true;
			break; // No index specified - this is fine and means string is completely verified
		}
		if ('0' > index || index > valid_str[3]){
			continue; // index is out of valid range
		}
		char page = resource_type[4];
		if (page == RESOURCE_SEPARATOR || page == '\0'){
			valid = true;
			break; // page can be omitted safely for any resource
		}
		if (valid_str[4] == '\0'){
			valid = true;
			break; // no page specification - anything is allowed, including pages that aren't defined
		}
		if (valid_str[4] == 'P'){
			if (page != 'L' && page != 'U'){
				continue; // neither valid page was specified
			}
			valid = true;
			break;
		}
		if (valid_str[4] != page){
			continue; // does not match valid page specification
		}
		// all checks passed
		valid = true;
		break;
	}
	return valid;
}

// Convenience wrapper to verify a resource in SBC string form by hiding the conversion to char[]
bool verify_resource_type_str(const void* res){
	if (str_len(res) > MAX_RESOURCE_TYPE_LENGTH){
		return false;
	}

	char res_str[MAX_RESOURCE_TYPE_LENGTH+1] = {0};

	str_char_copy(res, (u8*)res_str);
	return verify_resource_type(res_str);
}

/// Validates a resource string.
/// This checks for a valid resource type if specified, and checks that the resource name is valid.
/// @note This function does NOT check for the existence of the resource file.
///
/// @param resource_name Resource name, provided as null-terminated string
/// @return true if resource name is valid, false otherwise
bool verify_resource_name(const char* resource_name){
	// Check if name has a type component
	bool valid = false;
	const char* separator = strchr(resource_name, RESOURCE_SEPARATOR);
	const char* name = separator ? separator + 1 : resource_name;
	if (separator){
		valid = verify_resource_type(resource_name);
		if (!valid) return valid; // resource type invalid
	}

	int name_len = strlen(name);
	if (!name_len || name_len > MAX_RESOURCE_NAME_LENGTH){
		return false; // to long or has no name
	}
	for (int i = 0; i < name_len; ++i){
		if (!is_name(name[i])){
			return false; // invalid character
		}
	}
	return true;
}

#define RESOURCE_TYPE_BASE_LENGTH 3
#define RESOURCE_TYPE_BANK_OFS RESOURCE_TYPE_BASE_LENGTH
#define RESOURCE_TYPE_SCREEN_OFS RESOURCE_TYPE_BANK_OFS + 1

// Resource type can be null-terminated or end with RESOURCE_SEPARATOR as part of a complete resource name.
// @note Assumes resource name to be valid, as validated by verify_resource_name.
void* get_resource_ptr(struct ptc* p, const char* resource_type){
	assert(resource_type);
	void* resource_ptr = NULL;

	char res_type_index = resource_type[RESOURCE_TYPE_BANK_OFS];
	int bank = 0;
	int screen = 0;
	if (res_type_index == '\0' || res_type_index == RESOURCE_SEPARATOR){
		bank = 0;
	} else {
		bank = res_type_index - '0';
		char res_type_screen = resource_type[RESOURCE_TYPE_SCREEN_OFS];
		if (res_type_screen == '\0' || res_type_screen == RESOURCE_SEPARATOR){
			screen = -1;
		} else {
			screen = res_type_screen == 'L';
		}
	}

	if (!strncmp("BGU", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		screen = screen < 0 ? p->background.page : screen;
		resource_ptr = p->res.chr + 8 + CHR_BANKS * screen + bank;
	} else if (!strncmp("BGF", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		screen = screen < 0 ? p->background.page : screen;
		resource_ptr = p->res.chr + 0 + CHR_BANKS * screen + bank;
	} else if (!strncmp("BGD", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		screen = screen < 0 ? p->background.page : screen;
		resource_ptr = p->res.chr + 4 + CHR_BANKS * screen + bank;
	} else if (!strncmp("SPD", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		resource_ptr = p->res.chr + 12 + CHR_BANKS + bank;
	} else if (!strncmp("SPK", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		resource_ptr = p->res.chr + 16 + CHR_BANKS + bank;
	} else if (!strncmp("SPS", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		screen = screen < 0 ? p->sprites.page : screen;
		resource_ptr = p->res.chr + 20 + CHR_BANKS * screen + bank;
	} else if (!strncmp("SPU", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		resource_ptr = p->res.chr + 12 + bank;
	} else if (!strncmp("SCU", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		screen = screen < 0 ? p->background.page : screen;
		resource_ptr = p->res.scr + SCR_BANKS * screen + 2 + bank;
	} else if (!strncmp("COL", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		if (bank == 0) screen = screen < 0 ? (int)p->background.page : screen;
		if (bank == 1) screen = screen < 0 ? (int)p->sprites.page : screen;
		if (bank == 2) screen = screen < 0 ? (int)p->graphics.screen : screen;
		resource_ptr = p->res.col + COL_BANKS * screen + bank;
	} else if (!strncmp("PRG", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		return p->exec.prg.data;
	} else if (!strncmp("MEM", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		return p->res.mem.data;
	} else if (!strncmp("GRP", resource_type, RESOURCE_TYPE_BASE_LENGTH)){
		return p->res.grp[bank];
	} else {
		iprintf("Resource type %s is invalid!\n", resource_type);
		abort();
	}
	return *(void**)resource_ptr;
}

// TODO:CODE:MED Remove all uses of this (replace with get_verified_resource instead)
void* str_to_resource(struct ptc* p, void* name_str){
	assert(name_str);
	assert(str_len(name_str) <= 5);
	char name_char[MAX_RESOURCE_TYPE_LENGTH+1] = {0};
	str_char_copy(name_str, (u8*)name_char);
	return get_resource_ptr(p, name_char);
}

int get_chr_index(struct ptc* p, const char* res){
	int res_len = strlen(res);
	assert(res_len >= 3 && res_len <= 5);

	int index = (res_len <= 3) ? 0 : (res[3] - '0');
	if (res[0] == 'S'){
		index += 12;
		if (res[2] == 'D') { return index + CHR_BANKS; } // always lower
		if (res[2] == 'K') { return index + CHR_BANKS + 4; } // always lower
		if (res[2] == 'U') { return index; } // always upper
		index += CHR_BANKS * ((res_len <= 4) ? p->sprites.page : (res[4] == 'L'));
		index += 8;
	} else { // B
		index += 0;
		index += CHR_BANKS * ((res_len <= 4) ? p->background.page : (res[4] == 'L'));
		index += (res[2] == 'D') ? 4 : 0;
		index += (res[2] == 'U') ? 8 : 0;
	}

//	iprintf("%s has index %d\n", res, index);
	return index;
}

///
struct res_info {
	// longest name:
	// RRRNP:ABCDEFGH
	char type[MAX_RESOURCE_TYPE_LENGTH+1];
	char name[MAX_RESOURCE_NAME_LENGTH+1];
	int type_id;
	void* data;
};

struct res_info get_verified_resource_type(struct ptc* p, const void* res){
	// TODO:ERR:LOW verify correct error codes
	struct res_info info = {0};
	if (str_len(res) > MAX_RESOURCE_TYPE_LENGTH){
		p->exec.error = ERR_INVALID_RESOURCE_TYPE;
		return info;
	}
	if ((info.type_id = get_resource_type(res)) == TYPE_INVALID){
		p->exec.error = ERR_INVALID_RESOURCE_TYPE;
		return info;
	}
	str_char_copy(res, (u8*)info.type);
	info.data = get_resource_ptr(p, info.type);
	return info;
}

/// Verifies that a given resource name is valid and assigns the values
/// necessary to use it.
struct res_info get_verified_resource(struct ptc* p, const void* res){
	struct res_info info = {0};
	if (str_len(res) > MAX_RESOURCE_STR_LENGTH){
		p->exec.error = ERR_ILLEGAL_FUNCTION_CALL;
		return info;
	}

	char res_str[MAX_RESOURCE_STR_LENGTH] = {0};
	str_char_copy(res, (u8*)res_str);
	if (!verify_resource_name(res_str)){
		p->exec.error = ERR_ILLEGAL_FUNCTION_CALL;
		return info;
	}
	// Warning ignored because we zero-initialize info, and thus we don't need to copy the null
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
	const char* sep = strchr(res_str, RESOURCE_SEPARATOR);
	if (sep){
		// Copy from type to separator
		strncpy(info.type, res_str, sep - res_str);
		info.type_id = get_resource_type(res);
		const char* name = sep + 1;
		strncpy(info.name, name, strlen(name));
	} else { // Default PRG type
		strncpy(info.type, "PRG", 3);
		info.type_id = TYPE_PRG;
		strncpy(info.name, res_str, strlen(res_str));
	}
#pragma GCC diagnostic pop
	info.data = get_resource_ptr(p, info.type);
	iprintf("get_resource of type=%s (%d) name=%s at ptr=%p\n", info.type, (int)strlen(info.type), info.name, info.data);

	return info;
}

void cmd_load(struct ptc* p){
	// TODO:IMPL:LOW Dialog popups
	void* res = value_str(ARG(0));
	struct res_info info = get_verified_resource(p, res);
	if (p->exec.error) return; // error condition already met, fail here

	int size = check_load_res(info.data, p->res.search_path, info.name, info.type_id);
	p->res.result = size != 0;

	switch (info.type_id){
		case TYPE_PRG:
			if (size) p->exec.prg.size = size;
			break;

		case TYPE_CHR:
			p->res.regen_chr[get_chr_index(p, info.type)] = true;
			break;

		case TYPE_COL:
			p->res.regen_col = true;
			break;

		case TYPE_SCR:
		case TYPE_GRP:
			// no special handling
			break;

		case TYPE_MEM:;
			assert(LITTLE_ENDIAN);
			p->res.mem_str.len = p->res.mem.size;
			break;

		default:
			iprintf("Error: invalid resource type\n %s:%s\n", info.type, info.name);
			assert(false);
	}
}

void cmd_save(struct ptc* p){
	// TODO:IMPL:LOW Dialog popups
	void* res = value_str(ARG(0));
	struct res_info info = get_verified_resource(p, res);
	if (p->exec.error) return; // error condition already met, fail here

	assert(LITTLE_ENDIAN); // relied on for TYPE_MEM
	int size = check_save_res(info.data, p->res.search_path, info.name, info.type_id);
	p->res.result = size != 0;
}

void cmd_chrinit(struct ptc* p){
	// CHRINIT resource
	void* res_str = value_str(ARG(0));
	struct res_info info = get_verified_resource_type(p, res_str);
	if (p->exec.error) return; // something went wrong

	// check that this is a CHR type resource:
	if (TYPE_CHR == info.type_id){
		// resource is valid
		char res_name[] = "XXXX";
		for (int i = 0; i < 4; ++i){
			res_name[i] = info.type[i];
		}
		if (info.type[3] == '\0') res_name[3] = '0'; // default bank

		if (!load_chr(info.data, resource_path, res_name)){
			ERROR(ERR_FILE_LOAD_FAILED);
		}
		p->res.regen_chr[get_chr_index(p, info.type)] = true;
	} else {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
}

void cmd_chrread(struct ptc* p){
	// CHRREAD resource$ id var$
	struct string* dest = get_new_str(&p->strs);
	// Note: If uses is never set to 1, then this string is available for taking again
	// (no resource cleanup needed here)
	void* res_str = value_str(ARG(0));
	int id;
	STACK_INT_RANGE(1,0,255,id);
	void** out = ARG(2)->value.ptr; // pointer to string (which is itself a pointer)

	struct res_info info = get_verified_resource_type(p, res_str);
	if (p->exec.error) return; // something went wrong

	if (TYPE_CHR == info.type_id){
		// resource is valid
		u8* chr_data = (u8*)info.data + 32*id;
		// convert this to string
		for (int i = 0; i < 32; ++i){
			u8 unit = chr_data[i];
			u8 ul = unit & 0x0f;
			u8 uh = unit >> 4;
			dest->ptr.s[2 * i + 0] = hex_digits[ul];
			dest->ptr.s[2 * i + 1] = hex_digits[uh];
		}
		dest->len = 64;
		dest->uses = 1;
		*out = dest;
	} else {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
}

void cmd_chrset(struct ptc* p){
	// CHRSET resource$ id var$
	// Note: If uses is never set to 1, then this string is available for taking again
	// (no resource cleanup needed here)
	void* res_str = value_str(ARG(0));
	int id;
	STACK_INT_RANGE(1,0,255,id);
	void* data = value_str(ARG(2)); // pointer to string (which is itself a pointer)
//	iprintf("str[%d]=%.*s\n", str_len(data), str_len(data), (char*)str_at(data, 0));
	if (str_len(data) != 64){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}

	struct res_info info = get_verified_resource_type(p, res_str);
	if (p->exec.error) return; // something went wrong

	if (TYPE_CHR == info.type_id){
		// resource is valid
		u8* res_data = get_resource_ptr(p, info.type);
		if (!res_data) return;

		u8 chr_data[32];

		// convert string to data
		for (int i = 0; i < 32; ++i){
			u8 ul = digit_value(str_at_wide(data, 2*i));
			u8 uh = digit_value(str_at_wide(data, 2*i+1));
			if (ul > 16 || uh > 16){ // out of valid range
				ERROR(ERR_ILLEGAL_FUNCTION_CALL);
			}
			u8 unit = (uh << 4) | ul;
			chr_data[i] = unit;
		}

		// NDS: VRAM copy works here because memcpy uses bigger blocks
		memcpy(&res_data[32*id], chr_data, 32);

		p->res.regen_chr[get_chr_index(p, info.type)] = true;
	} else {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
}

void* get_col_resource(struct ptc* p, const void* res_str){
	if (str_comp(res_str, "S\2BG")) return get_resource_ptr(p, "COL0");
	if (str_comp(res_str, "S\2SP")) return get_resource_ptr(p, "COL1");
	if (str_comp(res_str, "S\3GRP")) return get_resource_ptr(p, "COL2");

	// no match;
	p->exec.error = ERR_INVALID_RESOURCE_TYPE;
	return NULL;
}

// TODO:TEST:MED needs test
void cmd_colinit(struct ptc* p){
	// COLINIT [resource [color]]
	if (p->stack.stack_i == 0){
		// reset all colors
		init_col(&p->res);
	} else if (p->stack.stack_i == 1){
		// COLINIT resource
		void* res_str = value_str(ARG(0));
		char id = str_at_char(res_str, 0);
		if (id == 'B') id = '0';
		if (id == 'S') id = '1';
		if (id == 'G') id = '2';

		char name[] = "COLX.PTC";
		name[3] = id;

		u8* col_data = get_col_resource(p, res_str);
		if (!col_data) return;

		load_col(col_data, resource_path, name);
		// TODO:CODE:LOW dedup 1-arg and 2-arg
	} else {
		// COLINIT resource index
		void* res_str = value_str(ARG(0));
		char name[] = "COLX.PTC";
		u16* col_data = get_col_resource(p, res_str);
		if (!col_data) return;

		char id = str_at_char(res_str, 0);
		if (id == 'B') id = '0';
		if (id == 'S') id = '1';
		if (id == 'G') id = '2';

		name[3] = id;
		// Load from file into memory
		u16 from_file[256]; // TODO:PERF:LOW loading entire file is slow, store it in-mem?
		load_col((u8*)from_file, resource_path, name);
		// Extract specific color index
		int index;
		STACK_INT_RANGE(1,0,255,index);
		col_data[index] = from_file[index];
	}
	// needed for any combination
	p->res.regen_col = true;
}

void cmd_colread(struct ptc* p){
	// COLREAD resource color r g b
	void* res_str = value_str(ARG(0));
	int col;
	STACK_INT_RANGE(1,0,255,col);
	fixp* r = ARG(2)->value.ptr;
	fixp* g = ARG(3)->value.ptr;
	fixp* b = ARG(4)->value.ptr;
	u16* col_data;

	col_data = get_col_resource(p, res_str);
	if (!col_data) return;

	// Note: Loading directly as u16 only works on little endian device
	assert(LITTLE_ENDIAN);
	uint_fast16_t c = col_data[col];
	// 15              0
	// GBBBBBGG GGGRRRRR
	uint_fast8_t r_comp = (c & 0x1f) << 3;
	r_comp += r_comp >> 5; // scaling 0-248 -> 0-255
 	uint_fast8_t g_comp = ((((c >> 4) & 0x3e)) | (c >> 15)) << 2;
	g_comp += g_comp >> 6; // scaling 0-252 -> 0-255
	uint_fast8_t b_comp = ((c >> 10) & 0x1f) << 3;
	b_comp += b_comp >> 5;
	*r = INT_TO_FP(r_comp);
	*g = INT_TO_FP(g_comp);
	*b = INT_TO_FP(b_comp);
}

void cmd_colset(struct ptc* p){
	// COLSET resource$ color color$
	void* res_str = value_str(ARG(0));
	int col;
	STACK_INT_RANGE(1,0,255,col);
	void* col_str = value_str(ARG(2));
	u16* col_data;
	col_data = get_col_resource(p, res_str);
	if (!col_data) return;

	// can't forget this one ;)
	if (str_len(col_str) != 6){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	for (int i = 0; i < COL_BANKS*SCREEN_COUNT; ++i){
		int c = str_at_char(col_str, i);
		if (!is_number(c) && !(c >= 'A' && c <= 'F')){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL);
		}
	}

	int r, g, b;
	r = digit_value(str_at_wide(col_str, 0)) << 1;
	g = digit_value(str_at_wide(col_str, 2)) << 2;
	b = digit_value(str_at_wide(col_str, 4)) << 1;
	r |= digit_value(str_at_wide(col_str, 1)) >> 3;
	g |= digit_value(str_at_wide(col_str, 3)) >> 2;
	b |= digit_value(str_at_wide(col_str, 5)) >> 3;
//	iprintf("colset with %d,%d,%d\n", r, g, b);

	u16 color = r | (b << 10);
	color |= ((g >> 1) << 5);
	color |= (g & 0x1) << 15;
//	iprintf("%04x", color);

	// 15   10    5    0
	// GBBBBBGG GGGRRRRR
	// Note: Loading as u16 directly only works on little endian device
	// TODO:CODE:LOW implement big-endian store
	assert(LITTLE_ENDIAN);
	col_data[col] = color;
	p->res.regen_col = true;
}

void cmd_new(struct ptc* p){
	p->exec.prg.size = 0;
}

void cmd_append(struct ptc* p){
	// APPEND file$
	p->res.result = 0; // covers every failure path by setting early
	void* name = STACK_STR(0);
	// append behavior:
	// copies text content of file to end of program memory
	struct res_info info = get_verified_resource(p, name);
	if (!info.data) return;
	if (info.type_id != TYPE_PRG) ERROR(ERR_INVALID_RESOURCE_TYPE);

	// resource type is valid PRG
	if (!verify_search_file_type(p->res.search_path, info.name, TYPE_PRG)){
		return;
	}

	// file identified correctly as PRG, check remaining code size
	assert(info.data == p->exec.prg.data);

	char path[MAX_FILEPATH_LENGTH+1] = {0};
	if (!create_path(path, p->res.search_path, info.name, ".PTC")){
		iprintf("File name too long! (append)\n");
		return;
	}

	FILE* f;
	f = fopen(path, "rb");
	if (!f){
		iprintf("Failed to open file\n");
		return;
	}
	char magic[4];
	if (4 != fread(magic, sizeof(char), 4, f)){
		iprintf("Failed to read magic ID\n");
		return;
	}

	int skip;
	// The check here happens again because in theory the file could've changed
	// between verification and loading.
#define PRG_HEADER_OFFSET_PRG_SIZE 8
	if (!strncmp(magic, "PETC", 4)){
		// internal format: program header follows PETC type
		skip = HEADER_TYPE_STR_SIZE + PRG_HEADER_OFFSET_PRG_SIZE;
	} else if (!strncmp(magic, "PX01", 4)){
		// sd format: skip common header and PETC type
		skip = HEADER_SIZE + PRG_HEADER_OFFSET_PRG_SIZE;
	} else {
		// unknown format.
		// Error because it should never happen
		ERROR(ERR_FILE_FORMAT);
	}

	if (fseek(f, skip, SEEK_SET)){
		iprintf("File seek error\n");
		ERROR(ERR_FILE_INTERAL);
	}
	// Offset to useful program info is now known.
	// Read until offset:
	static_assert(LITTLE_ENDIAN, "read int directly requires LE format");
	u32 program_size;
	if (1 != fread(&program_size, sizeof(u32), 1, f)){
		// Failed to read program size
		iprintf("File size read error\n");
		ERROR(ERR_FILE_INTERAL);
	}

	// File is at program data and we have size to read.
	// Determine if enough space is left:
	u32 remaining = MAX_SOURCE_SIZE - p->exec.prg.size; // leave room for final newline
	// if contents larger than remaining space, truncate extra content
	u32 size_to_read = remaining < program_size ? remaining : program_size;
	if (size_to_read != fread((u8*)info.data + p->exec.prg.size, sizeof(char), size_to_read, f)){
		// Failed to read program data
		ERROR(ERR_FILE_FORMAT);
	}
	p->exec.prg.size += size_to_read;
	// Load success!
	p->res.result = 1;
}

void cmd_rename(struct ptc* p){
	// RENAME oldname$ newname$
	(void)p;
}

void cmd_delete(struct ptc* p){
	(void)p;
}

void sys_mem(struct ptc* p){
	struct value_stack* s = &p->stack;

	stack_push(s, (struct stack_entry){VAR_STRING | VAR_VARIABLE, {.ptr = &p->res.mem_ptr}});
}

void syschk_mem(struct ptc* p){
	assert(p->res.mem_ptr);
	// ensure that pointer assignment is turned into a copy
	str_copy(p->res.mem_ptr, &p->res.mem_str);
	// reduce uses of this string
	int type = *(char*)p->res.mem_ptr;
	if (type == STRING_CHAR || type == STRING_WIDE){
		((struct string*)p->res.mem_ptr)->uses--;
	}
	// restore regular pointer
	p->res.mem_ptr = &p->res.mem_str;
	// set length of stored str
	p->res.mem.size = p->res.mem_str.len;
}

void sys_result(struct ptc* p){
	struct value_stack* s = &p->stack;

	stack_push(s, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(p->res.result)}});
}

