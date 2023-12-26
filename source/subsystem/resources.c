#include "resources.h"

#include "system.h"
#include "error.h"

#include <stdlib.h>
#include <stdio.h>

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

// Load from file `name` into dest
// returns false on failure
bool load_chr(u8* dest, const char* name){
	return load_file(dest, name, HEADER_SIZE, CHR_SIZE);
}

// load from file `name` into dest
// returns false on failure
bool load_col(u8* dest, const char* name){
	return load_file(dest, name, HEADER_SIZE, COL_SIZE);
}

// load from file `name` into dest
// returns false on failure
bool load_scr(u16* dest, const char* name){
	return load_file((u8*)dest, name, HEADER_SIZE, SCR_SIZE);
}

void init_resource(struct resources* r){
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
			r->col[i + COL_BANKS * lower] = calloc(COL_SIZE, 1);
		}
	}
	for (int i = 0; i < 4; ++i){
		iprintf("GRP%d calloc: %d\n", i, GRP_SIZE);
		r->grp[i] = calloc(GRP_SIZE, 1);
	}
#endif //ARM9
#ifdef PC
	// allocate memory for resources (needs ~512K)
	// this will serve as "emulated VRAM"
	r->all_banks = calloc(CHR_SIZE, CHR_BANKS * 2);
	r->col_banks = calloc(COL_SIZE, 6); //COL[0-2][U-L]
	// guarantee contiguous regions (useful for generating textures)
	for (int lower = 0; lower <= 1; ++lower){
		for (int i = 0; i < CHR_BANKS; ++i){
			r->chr[i + CHR_BANKS * lower] = &r->all_banks[(i + CHR_BANKS * lower) * CHR_SIZE];
		}
		for (int i = 0; i < SCR_BANKS; ++i){
			r->scr[i + SCR_BANKS * lower] = calloc(1, SCR_SIZE);
		}
		for (int i = 0; i < COL_BANKS; ++i){
			r->col[i + COL_BANKS * lower] = &r->col_banks[(i + lower * 3) * COL_SIZE / 2];
		}
	}
	for (int i = 0; i < 4; ++i){
		r->grp[i] = calloc(GRP_SIZE, 1);
	}
#endif //PC
	// Load default resource files
	const char* chr_files = 
	"BGF0BGF1BGF0BGF1BGD0BGD1BGD2BGD3BGU0BGU1BGU2BGU3"
	"SPU0SPU1SPU2SPU3SPU4SPU5SPU6SPU7SPS0SPS1"
	"BGF0BGF1BGF0BGF1BGD0BGD1BGD2BGD3BGU0BGU1BGU2BGU3"
	"SPD0SPD1SPD2SPD3SPD4SPD5SPD6SPD7SPS0SPS1";
	const char* col_files = "COL0COL1COL2COL0COL1COL2";
	
	char name[] = "resources/XXXX.PTC";
	
	for (int i = 0; i < CHR_BANKS * 2; ++i){
		for (int j = 0; j < 4; ++j){
			name[10+j] = chr_files[4*i+j];
		}
		load_chr(r->chr[i], name);
	}
	
	for (int i = 0; i < 6; ++i){
		for (int j = 0; j < 4; ++j){
			name[10+j] = col_files[4*i+j];
		}
		load_col((u8*)r->col[i], name);
	}
}

void free_resource(struct resources* r){
#ifdef PC
	// Free memory here
	free(r->all_banks);
	free(r->col_banks);
	for (int i = 0; i < 2*SCR_BANKS; ++i){
		free(r->scr[i]);
	}
#endif
	// Shared code
	for (int i = 0; i < 4; ++i){
		free(r->grp[i]);
	}
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

void cmd_chrinit(struct ptc* p){
	// CHRINIT resource
	void* res_str = value_str(ARG(0));
	int res_str_len = str_len(res_str);
	u8 res[5];
	if (res_str_len > 5) { ERROR(ERR_INVALID_RESOURCE_TYPE); }
	str_char_copy(res_str, res);
	// res contains at most 5 characters
	// check that this is a CHR type resource:
	// *G*, *P* inclues all valid CHR types.
	if (res[1] == 'G' || res[1] == 'P'){
		// resource is valid
		u8* res_data = get_resource(p, (char*)res, res_str_len);
		char res_name[] = "resources/XXXX.PTC";
		for (int i = 0; i < 4; ++i){
			res_name[10+i] = res[i];
		}
		if (res_str_len == 3){
			res_name[13] = '0';
		}
		if (!load_chr(res_data, res_name)){
			ERROR(ERR_FILE_LOAD_FAILED);
		}
	} else {
		ERROR(ERR_INVALID_RESOURCE_TYPE);
	}
}

