#include "resources.h"

#include "system.h"
#include "error.h"
#include "extension/compress.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char* resource_path = "resources/";

int resource_size[6] = {
	0, CHR_SIZE, SCR_SIZE, GRP_SIZE, 0, COL_SIZE,
};


/// Load from a file, skipping past part of it if needed and reading only a certain length
bool load_file(u8* dest, const char* name, int skip, int len){
	FILE* f = fopen(name, "rb");
	if (!f){
		iprintf("Failed to load file: %s\n", name);
		return false;
	}
	if (fseek(f, skip, SEEK_SET) == -1){
		fclose(f);
		iprintf("Failed to fseek to %d within %s\n", skip, name);
		return false;
	}
	fread(dest, sizeof(u8), len, f);
	if (ferror(f)){
		fclose(f);
		iprintf("Failed to read file %s\n", name);
		return false;
	}
	fclose(f);
	return true;
}

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

int check_load_res(u8* dest, const char* search_path, const char* name, int type){
	int size_read;
	char path[256] = {0};
	if (strlen(search_path) + strlen(name) >= 252){
		FILE_ERROR("File name too long");
	}
	strcpy(path, search_path);
	strcpy(path + strlen(search_path), name); // Note: includes null, overwrites null of search path
	
	if (type == TYPE_PRG){
		FILE* f = fopen(path, "rb");
		if (!f){
			iprintf("Failed to load file (retrying with extension): %s\n", path);
			// Try with file extension
			strcpy(path + strlen(search_path) + strlen(name), ".PTC");
			
			f = fopen(path, "rb");
			if (!f){
				FILE_ERROR("Failed to load file");
			}
		}
		struct ptc_header h;
		
		/// TODO:CODE:MED I think this only works on little-endian devices...
		/// (reading into header memory directly)
		size_read = fread(&h, sizeof(char), PRG_HEADER_SIZE, f);
		CHECK_FILE_ERROR("Could not read header correctly");
		if (size_read < PRG_HEADER_SIZE){
			FILE_CLOSE("Could not read full header");
		}
		// Assumed successful header read - now load file contents into dest
		size_read = fread(dest, sizeof(char), h.prg_size, f);
		return size_read;
		
	} else if (type >= TYPE_CHR && type <= TYPE_COL){
		return check_load_file(dest, "", path, resource_size[type]);
	}
	FILE_ERROR("Unknown resource type");
}

// Checks the file type and loads the correct data from the file.
// Note: path, name must be null-terminated
int check_load_file(u8* dest, const char* search_path, const char* name, int size){
	int size_read;
	char path[256] = {0};
	if (strlen(search_path) + strlen(name) >= 252){
		FILE_ERROR("File name too long");
	}
	strcpy(path, search_path);
	strcpy(path + strlen(search_path), name); // Note: includes null, overwrites null of search path
	
	FILE* f = fopen(path, "rb");
	if (!f){
		iprintf("Failed to load file (retrying with extension): %s\n", path);
		// Try with file extension
		strcpy(path + strlen(search_path) + strlen(name), ".PTC");
		
		f = fopen(path, "rb");
		if (!f){
			FILE_ERROR("Failed to load file");
		}
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
		
		// Used destinations as temporary storage - may lead to artifacting but reduces memory use
		u8* compressed = dest;
		size_read = fread(dest, sizeof(u8), size, f);
		CHECK_FILE_ERROR("Failed to read file");
		
		int bits = header[0];
		int expected_size = header[1] | (header[2] << 8) | (header[3] << 16);
		(void)expected_size; // for when assert fails
		assert(expected_size == size); // request == expected
		unsigned char* decompressed = sbc_decompress(compressed, size, bits);
		memcpy(dest, decompressed, size);
//		free_log("load_file", compressed); // TODO:PERF:LOW See if zero-allocation version is feasible?
		free_log("load_file", decompressed);
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

void init_resource(struct resources* r){
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
	r->all_banks = calloc_log("init_resource all_banks", CHR_SIZE, CHR_BANKS * 2);
	r->col_banks = calloc_log("init_resource col_banks", COL_SIZE, 6); //COL[0-2][U-L]
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
	
	// Load default resource files
	const char* chr_files = 
	"BGF0BGF1BGF0BGF1BGD0BGD1BGD2BGD3BGU0BGU1BGU2BGU3"
	"SPU0SPU1SPU2SPU3SPU4SPU5SPU6SPU7SPS0SPS1"
	"BGF0BGF1BGF0BGF1BGD0BGD1BGD2BGD3BGU0BGU1BGU2BGU3"
	"SPD0SPD1SPD2SPD3SPD4SPD5SPD6SPD7SPS0SPS1";
	const char* col_files = "COL0COL1COL2COL0COL1COL2";
	
	char name[] = "XXXX.PTC";
	
	for (int i = 0; i < CHR_BANKS * 2; ++i){
		for (int j = 0; j < 4; ++j){
			name[j] = chr_files[4*i+j];
		}
		load_chr(r->chr[i], resource_path, name);
	}
	
	for (int i = 0; i < 6; ++i){
		for (int j = 0; j < 4; ++j){
			name[j] = col_files[4*i+j];
		}
		load_col((u8*)r->col[i], resource_path, name);
	}
	// Load keyboard sprites in-memory for fast access
	for (int i = 0; i < 12; ++i){
		load_file(r->key_chr[i], key_files[i], 48 + (i >= 2 && i % 2 ? CHR_SIZE : 0), CHR_SIZE);
	}
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

void* str_to_resource(struct ptc* p, void* name_str){
	assert(name_str);
	assert(str_len(name_str) <= 5);
	char name_char[5];
	str_char_copy(name_str, (u8*)name_char);
	return get_resource(p, name_char, str_len(name_str));
}

int get_chr_index(struct ptc* p, char* res){
	int res_len = strlen(res);
	assert(res_len >= 3 && res_len <= 5);
	
	int index = (res_len <= 3) ? 0 : (res[3] - '0');
	if (res[0] == 'S'){
		index += 12;
		index += CHR_BANKS * ((res_len <= 4) ? p->sprites.page : (res[4] == 'L'));
		index += (res[2] == 'S') ? 8 : 0;
	} else { // B
		index += 0;
		index += CHR_BANKS * ((res_len <= 4) ? p->background.page : (res[4] == 'L'));
		index += (res[2] == 'D') ? 4 : 0;
		index += (res[2] == 'U') ? 8 : 0;
	}
	
	iprintf("%s has index %d\n", res, index);
	return index;
}

// Returns a data pointer for the resource
// Returns NULL, if the resource name was invalid
// TODO:ERR:MED Should ONLY work for exact names (currently allows some invalid strings)
void* get_resource(struct ptc* p, char* name, int len){
	assert(name);
	assert(len <= 5);
	char c = name[0]; //category
	char id = name[1]; //id help for SCU/SPU
	char t = name[2]; //type (within category)
	
	int bank = 0, upper = -1; 
	if (len == 5){
		bank = name[3] - '0';
		upper = name[4] == 'L';
	} else if (len == 4){
		if (name[3] >= '0' && name[3] <= '7'){
			bank = name[3] - '0';
		} else {
			p->exec.error = ERR_INVALID_RESOURCE_TYPE;
			return NULL;
		}
	} else if (len == 3){
		bank = 0;
	} else {
		p->exec.error = ERR_INVALID_RESOURCE_TYPE;
		return NULL;
	}
	
	// upper depends on current BGPAGE, SPPAGE, or GPAGE
	if (upper == -1){
		if (c == 'B' || (c == 'S' && id == 'C') || (c == 'C' && bank == 0)){
			// BG resource of some variety
			// BGF BGU BGD SCU COL0
			upper = p->background.page;
		} else if ((c == 'S' && id == 'P') || (c == 'C' && bank == 1)){
			// SPU SPS SPD COL1
			upper = p->sprites.page;
		} else if (c == 'G' || (c == 'C' && bank == 2)){
			// GRP COL2
			upper = p->graphics.screen;
		} else {
			p->exec.error = ERR_INVALID_RESOURCE_TYPE;
			return NULL;
		}
	}
	
//	iprintf("get_resource %c %c %c %d %d\n", c, id, t, bank, upper);
	if (c == 'B' && id == 'G'){ //BGU, BGF, BGD
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
		}
		if (id != 'P'){
			return NULL; // name is invalid
		}
		if (t == 'S'){
			return p->res.chr[20+bank+CHR_BANKS*upper];
		} else if (t == 'D'){
			return p->res.chr[12+bank+CHR_BANKS*upper];
		} else if (t == 'K'){ // yeah, this one normally isn't accessible
			return p->res.chr[16+bank+CHR_BANKS*upper];
		}
	} else if (c == 'C' && id == 'O' && t == 'L'){ //COL
//		iprintf("COLn %d:%p\n", 3*upper + bank, (void*)p->res.col[3*upper+bank]);
		return p->res.col[bank+COL_BANKS*upper];
	} else if (c == 'G' && id == 'R' && t == 'P'){ //GRP
		// TODO:IMPL:LOW Does page matter here?
		return p->res.grp[(int)bank];
	}
	p->exec.error = ERR_INVALID_RESOURCE_TYPE;
	return NULL;
}

void cmd_load(struct ptc* p){
	void* res = value_str(ARG(0));
	// TODO:IMPL:LOW Dialog popups
	// TODO:ERR:LOW Check errors are correct
	// longest name:
	// RRRNP:ABCDEFGH
	char res_str[14+1] = {0}; // null terminated
	if (str_len(res) > 14){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	str_char_copy(res, (u8*)res_str);
	int index = -1;
	for (int i = 0; i <= (int)str_len(res); ++i){
		if (res_str[i] == ':'){
			res_str[i] = '\0'; // null-terminated for strlen later on
			index = i;
			break;
		}
	}
	if (index == -1 && str_len(res) >= 8) {
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	
	char* res_type = (index == -1) ? "PRG" : res_str;
	char* res_name = (index == -1) ? res_str : &res_str[index+1];
	void* res_data = get_resource(p, res_type, strlen(res_type));
	iprintf("Attempting load of type=%s (%d) name=%s at ptr=%p\n", res_type, (int)strlen(res_type), res_name, res_data);
	if (!res_data) return; // Note: error set in get_resource
	
	switch (get_resource_type(res)){
		case TYPE_CHR:
			// TODO:IMPL:MED Select path for searching
			if (!load_chr(res_data, "programs/", res_name)){
				ERROR(ERR_FILE_LOAD_FAILED);
			}
			
			p->res.regen_chr[get_chr_index(p, res_type)] = true;
			break;
			
		case TYPE_COL:
			if (!load_col(res_data, "programs/", res_name)){
				ERROR(ERR_FILE_LOAD_FAILED);
			}
			
			p->res.regen_col = true;
			break;
			
		case TYPE_GRP:
			if (!load_grp(res_data, "programs/", res_name)){
				ERROR(ERR_FILE_LOAD_FAILED);
			}
			break;
			
		// TODO:IMPL:HIGH Other file types
		default:
			iprintf("Error: Unimplemented/invalid resource type\n %s:%s\n", res_type, res_name);
			ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
}

void cmd_chrinit(struct ptc* p){
	// CHRINIT resource
	void* res_str = value_str(ARG(0));
	int res_str_len = str_len(res_str);
	u8 res[6] = {0};
	if (res_str_len > 5) {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
	str_char_copy(res_str, res);
	// res contains at most 5 characters
	// check that this is a CHR type resource:
	// *G*, *P* inclues all valid CHR types.
	if (res[1] == 'G' || res[1] == 'P'){
		// resource is valid
		u8* res_data = get_resource(p, (char*)res, res_str_len);
		if (!res_data) return;
		char res_name[] = "XXXX.PTC";
		for (int i = 0; i < 4; ++i){
			res_name[i] = res[i];
		}
		if (res_str_len == 3){
			res_name[3] = '0';
		}
		if (!load_chr(res_data, resource_path, res_name)){
			ERROR(ERR_FILE_LOAD_FAILED);
		}
		p->res.regen_chr[get_chr_index(p, (char*)res)] = true;
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
	
	int res_str_len = str_len(res_str);
	u8 res[6] = {0};
	if (res_str_len > 5) {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
	str_char_copy(res_str, res);
	// res contains at most 5 characters
	// check that this is a CHR type resource:
	// *G*, *P* inclues all valid CHR types.
	if (res[1] == 'G' || res[1] == 'P'){
		// resource is valid
		u8* res_data = get_resource(p, (char*)res, res_str_len);
		if (!res_data) {
			ERROR(ERR_INVALID_RESOURCE_TYPE);
		};
		
		u8* chr_data = &res_data[32*id];
		// convert this to string
		for (int i = 0; i < 32; ++i){
			u8 unit = chr_data[i];
			u8 ul, uh;
			ul = unit & 0x0f;
			uh = unit >> 4;
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
	if (str_len(data) != 64){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	
	int res_str_len = str_len(res_str);
	u8 res[6] = {0};
	if (res_str_len > 5) {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
	str_char_copy(res_str, res);
	// res contains at most 5 characters
	// check that this is a CHR type resource:
	// *G*, *P* inclues all valid CHR types.
	if (res[1] == 'G' || res[1] == 'P'){
		// resource is valid
		u8* res_data = get_resource(p, (char*)res, res_str_len);
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
		
		p->res.regen_chr[get_chr_index(p, (char*)res)] = true;
	} else {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
}

void cmd_colinit(struct ptc* p){
	// COLINIT [resource [color]]
	if (p->stack.stack_i == 0){
		// reset all
		const char* col_files = "COL0COL1COL2COL0COL1COL2";
		
		char name[] = "XXXX.PTC"; // TODO:PERF:NONE only set one character
		// TODO:CODE:LOW dedup with init_resource
		for (int i = 0; i < 6; ++i){
			for (int j = 0; j < 4; ++j){
				name[j] = col_files[4*i+j];
			}
			load_col((u8*)p->res.col[i], resource_path, name);
		}
	} else if (p->stack.stack_i == 1){
		// COLINIT resource
		void* res_str = value_str(ARG(0));
		if (str_comp(res_str, "S\2BG")){
			load_col(get_resource(p, "COL0", 4), resource_path, "COL0.PTC");
		} else if (str_comp(res_str, "S\2SP")){
			load_col(get_resource(p, "COL1", 4), resource_path, "COL1.PTC");
		} else if (str_comp(res_str, "S\3GRP")){
			load_col(get_resource(p, "COL2", 4), resource_path, "COL2.PTC");
		} else {
			ERROR(ERR_INVALID_RESOURCE_TYPE);
		}
	} else {
		// COLINIT resource index
		void* res_str = value_str(ARG(0));
		u16 from_file[256]; // TODO:PERF:LOW loading entire file is slow, store it in-mem?
		// TODO:CODE:MED this can be simplified
		u16* col_data;
		if (str_comp(res_str, "S\2BG")){
			col_data = get_resource(p, "COL0", 4);
			load_col((u8*)from_file, resource_path, "COL0.PTC");
		} else if (str_comp(res_str, "S\2SP")){
			col_data = get_resource(p, "COL1", 4);
			load_col((u8*)from_file, resource_path, "COL1.PTC");
		} else if (str_comp(res_str, "S\3GRP")){
			col_data = get_resource(p, "COL2", 4);
			load_col((u8*)from_file, resource_path, "COL2.PTC");
		} else {
			ERROR(ERR_INVALID_RESOURCE_TYPE);
		}
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
	fixp* r, * g, * b;
	r = ARG(2)->value.ptr;
	g = ARG(3)->value.ptr;
	b = ARG(4)->value.ptr;
	u16* col_data;
	if (str_comp(res_str, "S\2BG")){
		col_data = get_resource(p, "COL0", 4);
	} else if (str_comp(res_str, "S\2SP")){
		col_data = get_resource(p, "COL1", 4);
	} else if (str_comp(res_str, "S\3GRP")){
		col_data = get_resource(p, "COL2", 4);
	} else {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
	
	int c = col_data[col];
	// GGGRRRRR GBBBBBGG
	// Note: Loading as u16 only works on little endian device
	*r = INT_TO_FP(c & 0x1f) << 3;
	*g = INT_TO_FP(((c >> 5) & 0x1f)) << 3; // TODO:TEST:LOW Is lowest bit readable?
	*b = INT_TO_FP(((c >> 10) & 0x1f)) << 3; // TODO:TEST:LOW Correct values as they scale?
}

void cmd_colset(struct ptc* p){
	// COLSET resource$ color color$
	void* res_str = value_str(ARG(0));
	int col;
	STACK_INT_RANGE(1,0,255,col);
	void* col_str = value_str(ARG(2));
	u16* col_data;
	if (str_comp(res_str, "S\2BG")){
		col_data = get_resource(p, "COL0", 4);
	} else if (str_comp(res_str, "S\2SP")){
		col_data = get_resource(p, "COL1", 4);
	} else if (str_comp(res_str, "S\3GRP")){
		col_data = get_resource(p, "COL2", 4);
	} else {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
	// can't forget this one ;)
	if (str_len(col_str) != 6){
		ERROR(ERR_ILLEGAL_FUNCTION_CALL);
	}
	for (int i = 0; i < 6; ++i){
		int c = str_at_char(col_str, i);
		if (!is_number(c) && !(c >= 'A' && c <= 'F')){
			ERROR(ERR_ILLEGAL_FUNCTION_CALL); // TODO:TEST:LOW Check error code
		}
	}
	
	int r, g, b;
	r = digit_value(str_at_wide(col_str, 0)) << 1;
	g = digit_value(str_at_wide(col_str, 2)) << 1;
	b = digit_value(str_at_wide(col_str, 4)) << 1;
	r |= digit_value(str_at_wide(col_str, 1)) >> 3;
	g |= digit_value(str_at_wide(col_str, 3)) >> 3;
	b |= digit_value(str_at_wide(col_str, 5)) >> 3;
	
	col_data = &col_data[col]; // pointer to specific color we will set
	
	// GGGRRRRR GBBBBBGG
	// Note: Loading as u16 only works on little endian device
	// TODO:TEST:MED This loses the lowest bit of green?
	*col_data = r | (g << 5) | b << 10;
	p->res.regen_col = true;
}
