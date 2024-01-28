#include "interpreter/arrays.h"

#include "test_util.h"
#include "common.h"
#include "strs.h"

int test_arrs(){
	// init_mem_arr works as expected
	{
		struct arrays arrs;
		init_mem_arr(&arrs, 2, 16);
		
		ASSERT(arrs.arr_data != NULL, "[init_mem_arr] Some allocated memory");
		ASSERT(arrs.arr_data_size == 20, "[init_mem_arr] Correct size of allocated memory");
		ASSERT(arrs.arr_data_next == 0, "[init_mem_arr] Correct next element");
		
		free_mem_arr(&arrs);
	}
	
	// can create a new array
	{
		struct arrays arrs;
		init_mem_arr(&arrs, 2, 16);
		
		void* d = get_init_new_arr(&arrs, 10, ARR_DIM2_UNUSED, VAR_NUMBER);
		ASSERT(arrs.arr_data == d, "[get_init_new_arr] First array has pointer to start of data");
		ASSERT(arrs.arr_data_next == 12, "[get_init_new_arr] Next points to after array + sizes");
		ASSERT(arr_size(d, ARR_DIM1) == 10, "[arr_size] Number of array elements");
		ASSERT(arr_size(d, ARR_DIM2) == ARR_DIM2_UNUSED, "[arr_size] No second dimension");
		
		free_mem_arr(&arrs);
	}
	
	// can create a new 2d array
	{
		struct arrays arrs;
		init_mem_arr(&arrs, 2, 16);
		
		void* d = get_init_new_arr(&arrs, 6, 2, VAR_NUMBER);
		ASSERT(arrs.arr_data == d, "[get_init_new_arr] First array has pointer to start of data");
		ASSERT(arrs.arr_data_next == 14, "[get_init_new_arr] Next points to correct size");
		ASSERT(arr_size(d, ARR_DIM1) == 6, "[arr_size] Size of dimension 1");
		ASSERT(arr_size(d, ARR_DIM2) == 2, "[arr_size] Size of dimension 2");
		
		free_mem_arr(&arrs);
	}
	
	// array indexing
	{
		struct arrays arrs;
		init_mem_arr(&arrs, 2, 16);
		
		void* d = get_init_new_arr(&arrs, 3, ARR_DIM2_UNUSED, VAR_NUMBER);
		ASSERT(arrs.arr_data == d, "[get_init_new_arr] First array has pointer to start of data");
		ASSERT(arrs.arr_data_next == 5, "[get_init_new_arr] Next points to correct size");
		ASSERT(arr_size(d, ARR_DIM1) == 3, "[arr_size] Size of dimension 1");
		ASSERT(arr_size(d, ARR_DIM2) == ARR_DIM2_UNUSED, "[arr_size] Size of dimension 2");
		
		ASSERT(arr_entry(d, 0, 0) == arr_data(d), "[arr_entry] Entry 0 matches array data pointer");
		ASSERT(arr_entry(d, 0, 0)->number == 0, "[arr_entry] Entry 0 matches expected zero default");
		ASSERT(arr_entry(d, 1, 0)->number == 0, "[arr_entry] Entry 1 matches expected zero default");
		ASSERT(arr_entry(d, 2, 0)->number == 0, "[arr_entry] Entry 2 matches expected zero default");
		
		arr_entry(d, 0, 0)->number = INT_TO_FP(2);
		arr_entry(d, 1, 0)->number = INT_TO_FP(5);
		arr_entry(d, 2, 0)->number = INT_TO_FP(11);
		
		ASSERT(arr_entry(d, 0, 0)->number == INT_TO_FP(2), "[arr_entry] Entry 0 set to 2");
		ASSERT(arr_entry(d, 1, 0)->number == INT_TO_FP(5), "[arr_entry] Entry 1 set to 5");
		ASSERT(arr_entry(d, 2, 0)->number == INT_TO_FP(11), "[arr_entry] Entry 2 set to 12");
		
		free_mem_arr(&arrs);
	}
	
	// Initialize string array
	{
		struct arrays arrs;
		init_mem_arr(&arrs, 2, 16);
		
		void* d = get_init_new_arr(&arrs, 3, ARR_DIM2_UNUSED, VAR_STRING);
		ASSERT(arrs.arr_data == d, "[get_init_new_arr] First array has pointer to start of data");
		ASSERT(arrs.arr_data_next == 5, "[get_init_new_arr] Next points to correct size");
		ASSERT(arr_size(d, ARR_DIM1) == 3, "[arr_size] Size of dimension 1");
		ASSERT(arr_size(d, ARR_DIM2) == ARR_DIM2_UNUSED, "[arr_size] Size of dimension 2");
		
		ASSERT(arr_entry(d, 0, 0) == arr_data(d), "[arr_entry] Entry 0 matches array data pointer");
		ASSERT(arr_entry(d, 0, 0)->ptr == empty_str, "[arr_entry] Entry 0 matches expected \"\" default");
		ASSERT(arr_entry(d, 1, 0)->ptr == empty_str, "[arr_entry] Entry 1 matches expected \"\" default");
		ASSERT(arr_entry(d, 2, 0)->ptr == empty_str, "[arr_entry] Entry 2 matches expected \"\" default");
		
		arr_entry(d, 1, 0)->ptr = "S\3zyx";
		
		ASSERT(str_comp(arr_entry(d, 1, 0)->ptr, "S\3zyx"), "[arr_entry] Entry 1 set to \"zyx\"");
		
		free_mem_arr(&arrs);
	}
	
	SUCCESS("test_arrs success");
}
