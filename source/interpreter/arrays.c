#include "arrays.h"
#include "vars.h"
#include "strs.h"

#include <stdlib.h>
#include <string.h>

#ifdef ARM9
#include <nds/system.h>
#endif

void init_mem_arr(struct arrays* a, int array_count, int element_count){
	a->arr_data_size = element_count + 2 * array_count;
	a->arr_data_next = 0;
	a->arr_data = calloc_log("init_mem_arr", a->arr_data_size, sizeof(union value));
}

void reset_arr(struct arrays* a){
	a->arr_data_next = 0; // starts allocating from beginning
	// Fill memory with zeroes so initial reads have the correct value
	memset(a->arr_data, 0, sizeof(union value) * a->arr_data_size);
}

void free_mem_arr(struct arrays* a){
	free_log("free_mem_arr", a->arr_data);
}

void* get_init_new_arr(struct arrays* a, u32 size1, u32 size2, int type){
	u32* new_arr = (u32*)&((union value*)a->arr_data)[a->arr_data_next];
	int size = size1;
	if (size2 != ARR_DIM2_UNUSED){
		size *= size2;
	}
	if (a->arr_data_next + size + 2 >= a->arr_data_size){
		return NULL; // not enough memory for this
	}
	a->arr_data_next += size + 2;
	new_arr[0] = size1;
	new_arr[1] = size2;
	
	// fill with empty elements
	// Note: Treats every array as 1D - this is safe because memory is contiguous
	union value* new_arr_dat = &((union value*)new_arr)[2];
	for (int i = 0; i < size; ++i){
		if (type & VAR_NUMBER){
			new_arr_dat[i].number = 0;
		} else if (type & VAR_STRING){
			new_arr_dat[i].ptr = empty_str;
		}
	}
	
	return new_arr;
}

u32 arr_size(void* array, int dim){
	return ((u32*)array)[dim];
}

union value* arr_data(void* array){
	return &((union value*)array)[2];
}

union value* arr_entry(void* array, int ix1, int ix2){
	return &arr_data(array)[ix1 + ix2 * *(u32*)array];
}
