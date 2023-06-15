#include <stdlib.h>

#include "system.h"
#include "vars.h"


struct ptc* system_init(){
		
	//allocate ptc struct here?
	return NULL;
}

// allocate var table for `var_count` variables
void init_mem_var(struct ptc* s, int var_count){
	s->vars_max = var_count;
	s->vars = calloc(var_count, sizeof(struct variable));
}

void init_mem_arr(struct ptc* s, int element_count){
	s->arr_data_size = element_count;
	s->arr_data_next = 0;
	s->arr_data = calloc(element_count, sizeof(union value));
}

void init_mem_str(struct ptc* s, int string_count, char string_type){
	s->strs_max = string_count;
	s->strings = calloc(string_count, sizeof(struct string));
	if (string_type == STRING_CHAR){
		s->str_data = calloc(string_count, 1*MAX_STRLEN);
	} else if (string_type == STRING_WIDE) {
		s->str_data = calloc(string_count, 2*MAX_STRLEN);
	} else {
		// ERROR TODO
	}
}

