#include "arrays.h"
#include "vars.h"

#include <stdlib.h>
#include <string.h>

void init_mem_arr(struct arrays* a, int array_count, int element_count){
	a->arr_data_size = element_count + 2 * array_count;
	a->arr_data_next = 0;
	iprintf("calloc=%d\n", (int)a->arr_data_size * (int)sizeof(union value));
	a->arr_data = calloc(a->arr_data_size, sizeof(union value));
}

void reset_arr(struct arrays* a){
	a->arr_data_next = 0; // starts allocating from beginning
	// Fill memory with zeroes so initial reads have the correct value
	memset(a->arr_data, 0, sizeof(union value) * a->arr_data_size);
}

void free_mem_arr(struct arrays* a){
	free(a->arr_data);
}

void* get_new_arr(struct arrays* a, u32 size1, u32 size2){
	u32* new_arr = (u32*)&((union value*)a->arr_data)[a->arr_data_next];
	if (size2 == ARR_DIM2_UNUSED){
		a->arr_data_next += size1;
	} else {
		a->arr_data_next += size1 * size2;
	}
	a->arr_data_next += 2;
	new_arr[0] = size1;
	new_arr[1] = size2;
	
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
