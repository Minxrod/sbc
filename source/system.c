#include <stdlib.h>

#include "system.h"
#include "vars.h"


struct ptc* system_init(){
		
	//allocate ptc struct here?
	return NULL;
}

void init_mem_arr(struct ptc* s, int element_count){
	s->arr_data_size = element_count;
	s->arr_data_next = 0;
	s->arr_data = calloc(element_count, sizeof(union value));
}

