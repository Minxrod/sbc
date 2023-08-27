#include "vars.h"
#include "system.h"
#include "arrays.h"
#include "strs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Used as index into var table directly.
int var_name_hash(char* name, u32 len, u32 hmax){
	int hash = 0;
	for (u32 i=0; i<len; ++i){
		hash += (name[i]-'A')*i*179;
	}
	return hash % hmax;
}

bool namecmp(char* a, u32 len, char b[16]){
	for (u32 i = 0; i < len; ++i){
		if (a[i] != b[i]){
			return false;
		}
	}
	if (len == 16)
		return true;
	else {
		return b[len] == '\0';
	}
}

/// Allocate memory for vars.
void init_mem_var(struct variables* v, uint_fast16_t var_count){
	v->vars_max = var_count;
	iprintf("calloc=%d\n", (int)var_count * (int)sizeof(struct named_var));
	v->vars = calloc(var_count, sizeof(struct named_var));
	for (uint_fast16_t i = 0; i < v->vars_max; ++i){
		v->vars[i].type = VAR_EMPTY;
	}
}

void reset_var(struct variables* v){
	for (uint_fast16_t i = 0; i < v->vars_max; ++i){
		v->vars[i].type = VAR_EMPTY;
	}
}

/// Freee memory for vars
void free_mem_var(struct variables* v){
	free(v->vars);
}


/// Retrieves a var from the variable table contained within the ptc struct.
/// If the variable does not exist, creates it and then returns it.
/// If the variable can not be created, returns NULL.
struct named_var* test_var(struct variables* v, char* name, enum types type){
	return get_var(v, name, strlen(name), type);
}

struct named_var* search_name_type(struct variables* v, char* name, u32 len, enum types type){
	struct named_var* var = NULL;
	int hash = var_name_hash(name, len, v->vars_max);
	u32 step = 0;
	
	do {
		// quad walk
		// TODO:CODE Try to prove this?
		// Values that matter here (<65k) work, but does it work for all vars_max=2^n?
		hash = (hash + step++) % v->vars_max;
		var = &v->vars[hash];
		if (step >= v->vars_max){
			return NULL;
		} 
	} while (var->type != VAR_EMPTY && (!namecmp(name, len, var->name) || var->type != type));
	
	return var;
}

struct named_var* get_new_arr_var(struct variables* v, char* name, u32 len, enum types type, u32 dim1, u32 dim2){
	struct named_var* var = search_name_type(v, name, len, type);
	
	// make new array type
	//if VAR_EMPTY then create it?
	if (var->type == VAR_EMPTY){
		// set var type
		var->type = type;
		
		// set var name
		for (u32 i = 0; i < len; ++i){
			var->name[i]=name[i];
		}
		if (len != 16){
			var->name[len]='\0';
		}
		
		// set var ptr
		var->value.ptr = get_new_arr(v->arrs, dim1, dim2);
		
		return var;
	} else {
		// array already exists!
		iprintf("Error: Array already exists!\n");
		abort();
	}
}

struct named_var* get_var(struct variables* v, char* name, u32 len, enum types type){
	struct named_var* var = search_name_type(v, name, len, type);
	//if VAR_EMPTY then create it?
	if (var->type == VAR_EMPTY){
		// set var type
		var->type = type;
		// set var name
		for (u32 i = 0; i < len; ++i){
			var->name[i]=name[i];
		}
		if (len != 16){
			var->name[len]='\0';
		}
		
		//Set default values
		// Note: Checks must use exact VAR_NUMBER and VAR_STRING
		// to prevent arrays being initialized wrong
		if (type == VAR_NUMBER){
			var->value.number = 0;
		} else if (type == VAR_STRING) {
			var->value.ptr = NULL;
		} else if (type & VAR_ARRAY) {
			// Can't directly initialize here
			var->value.ptr = NULL;
		}
	}
	
	return var;
}

union value* get_arr_entry(struct variables* v, char* name, u32 len, enum types type, u32 ix, u32 iy){
	struct named_var* a = get_var(v, name, len, type);
	if (a == NULL){
		// out of memory
		iprintf("Error: Out of memory!\n");
		abort();
	}
	
	if (a->value.ptr == NULL){
		// array does not exist yet: must default initalize
		if (iy == ARR_DIM2_UNUSED){
			a->value.ptr = get_new_arr(v->arrs, 10, ARR_DIM2_UNUSED);
		} else {
			a->value.ptr = get_new_arr(v->arrs, 10, 10);
		}
	}
	// array exists and is stored in a->ptr
	if (ix < arr_size(a->value.ptr, ARR_DIM1)){
		if (iy == ARR_DIM2_UNUSED){
			return arr_entry(a->value.ptr, ix, 0);
		} else if (iy < arr_size(a->value.ptr, ARR_DIM2)){
			return arr_entry(a->value.ptr, ix, iy);
		} else {
			iprintf("Error: Index iy out of range\n");
			abort();
		}
	} else {
		iprintf("Error: Index ix out of range\n");
		abort();
	}
}
