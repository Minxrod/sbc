#include "vars.h"
#include "system.h"

#include <stdlib.h>
#include <stdio.h>

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
void init_mem_var(struct variables* v, int var_count){
	v->vars_max = var_count;
	v->vars = calloc(var_count, sizeof(struct named_var));
	for (u32 i = 0; i < v->vars_max; ++i){
		v->vars[i].type = VAR_EMPTY;
	}
}

/// Allocate memory for strs
void init_mem_str(struct strings* s, int str_count, enum string_type str_type){
	s->strs_max = str_count;
	s->strs = calloc(str_count, sizeof(struct string));
	if (str_type == STRING_CHAR){
		s->str_data = calloc(str_count, sizeof(struct string_data));
	} else if (str_type == STRING_WIDE) {
		s->str_data = calloc(str_count, sizeof(struct wstring_data));
	} else {
		// ERROR TODO
	}
}

/// Finds a usable string slot.
struct string* get_new_str(struct strings* s){
	++s;
	return NULL;
}

/// Retrieves a var from the variable table contained within the ptc struct.
/// If the variable does not exist, creates it and then returns it.
/// If the variable can not be created, returns NULL.
struct named_var* get_var(struct variables* v, char* name, u32 len, enum var_type type){
	struct named_var* var;
	int hash = var_name_hash(name, len, v->vars_max);
	u32 step = 0;
	
	do {
		// quad walk
		hash = (hash + step++) % v->vars_max;
		var = &v->vars[hash];
		if (step >= v->vars_max){
			return NULL;
		} 
	} while (var->type != VAR_EMPTY && (!namecmp(name, len, var->name) || var->type != type));
	
	//if VAR_EMPTY then create it?
	if (var->type == VAR_EMPTY){
		// set name at this index
		var->type = type;
		for (u32 i = 0; i < len; ++i){
			var->name[i]=name[i];
		}
		if (len != 16){
			var->name[len]='\0';
		}
		//TODO: SET VALUE!
		//Initialize string if needed
	}
	
	return var;
}

