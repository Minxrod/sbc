#pragma once

#include "common.h"

#define ARR_DIM1 0
#define ARR_DIM2 1
#define ARR_DIM2_UNUSED -1U

// The format of an array as stored in array data is:
// u32[2] sizes;
// union value[size[0]*size[1]];
// If 64-bit, there will be padding between the sizes and the 

struct arrays {
	/// Size of allocated array data block
	u32 arr_data_size;
	/// Next array data offset
	u32 arr_data_next;
	/// Pointer to array data (can contain 20.12fp numbers or string ptrs)
	void* arr_data;
};

void init_mem_arr(struct arrays* a, int array_count, int element_count);
void reset_arr(struct arrays* a);
void free_mem_arr(struct arrays* a);

//void* get_new_arr(struct arrays* a, u32 size1, u32 size2);
void* get_init_new_arr(struct arrays* a, u32 size1, u32 size2, int type);

u32 arr_size(void* array, int dim);

union value* arr_data(void* array);

union value* arr_entry(void* array, int ix1, int ix2);
