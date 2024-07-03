#include "system.h"

void cmd_clear(struct ptc* p){
	reset_var(&p->vars);
	reset_str(&p->strs);
	reset_arr(&p->arrs);
}

void cmd_swap(struct ptc* p){
	// Note: Both are variables and types are already the same via tok_test check
	// Note that swap is swapping values, but those values depend on types
	struct stack_entry* a = ARG(0);
	struct stack_entry* b = ARG(1);
	union value c;
	memcpy(&c, a->value.ptr, sizeof(union value)); // copies from pointed-to number or string
	memcpy(a->value.ptr, b->value.ptr, sizeof(union value));
	memcpy(b->value.ptr, &c, sizeof(union value));
}

#define MAX_SORT_ARRAYS 32

static inline void _sort_swap(union value* data, union value** extra, int i, int j){
	union value temp = data[i];
	data[i] = data[j];
	data[j] = temp;
	for (int k = 0; k < MAX_SORT_ARRAYS; ++k){
		if (extra[k]){
			temp = extra[k][i];
			extra[k][i] = extra[k][j];
			extra[k][j] = temp;
		} else {
			return;
		}
	}
}

typedef bool(*comparison)(union value, union value);

// based on
// https://en.wikipedia.org/wiki/Quicksort?useskin=monobook#cite_ref-:2_13-2
int _partition(union value* data, union value** extra, int start, int end, comparison comp){
	if (end - start <= 1){
		return start; // already sorted
	}

	union value pivot = data[start+(end - start)/2]; // select a random pivot
	--start;
	++end;

	while (true){
		do {
			++start;
		} while (comp(data[start], pivot));
		do {
			--end;
		} while (comp(pivot, data[end]));
		if (start >= end) return start;

		_sort_swap(data, extra, start, end);
	}
}

bool fixp_sort_comp(union value a, union value b){
	return a.number < b.number;
}

bool fixp_rsort_comp(union value a, union value b){
	return a.number > b.number;
}

bool str_sort_comp(union value a, union value b){
	int size_a = str_len(a.ptr);
	int size_b = str_len(b.ptr);
	int size = size_a < size_b ? size_a : size_b;
	for (int i = 0; i < size; ++i){
		u16 chr_a = str_at_wide(a.ptr, i);
		u16 chr_b = str_at_wide(b.ptr, i);
		if (chr_a < chr_b){
			return true;
		} else if (chr_b < chr_a){
			return false;
		}
	}
	return size_a < size_b;
}

bool str_rsort_comp(union value a, union value b){
	int size_a = str_len(a.ptr);
	int size_b = str_len(b.ptr);
	int size = size_a < size_b ? size_a : size_b;
	for (int i = 0; i < size; ++i){
		u16 chr_a = str_at_wide(a.ptr, i);
		u16 chr_b = str_at_wide(b.ptr, i);
		if (chr_a == chr_b) continue;
		return chr_a > chr_b;
	}
	return size_a > size_b;
}

void quick_sort(union value* data, union value** extra, int start, int end, comparison comp){
	// https://en.wikipedia.org/wiki/Quicksort?useskin=monobook#Hoare_partition_scheme
	if (end - start <= 1) return;

	int p = _partition(data, extra, start, end, comp);
	quick_sort(data, extra, start, p, comp);
	quick_sort(data, extra, p + 1, end, comp);
}

/// @note Does not correctly handle permuting the same array as is being sorted!
void _cmd_sort_shared(struct ptc* p, comparison comp){
	int start;
	int size;
	union value* extra[MAX_SORT_ARRAYS] = {0};
	if (p->stack.stack_i > MAX_SORT_ARRAYS+3){
		ERROR(ERR_SYNTAX); // 32 is max number of arrays
	}
	// SORT start, size, arr, [arr1, arr2, ...]
	union value* data = *(void**)stack_get(&p->stack, 2)->value.ptr;
//	iprintf("%p\n", (void*)data);
	int array_min_size = arr_size(data, ARR_DIM1);
	// In concept this is fine but PTC doesn't allow it
	if (arr_size(data, ARR_DIM2) != ARR_DIM2_UNUSED){
		ERROR(ERR_SORT_2D);
	}
	for (int k = 3; k < p->stack.stack_i; ++k){
		extra[k-3] = *(union value**)stack_get(&p->stack, k)->value.ptr;
		if ((int)arr_size(extra[k-3], ARR_DIM1) < array_min_size){
			array_min_size = arr_size(extra[k-3], ARR_DIM1);
		}
		if (arr_size(extra[k-3], ARR_DIM2) != ARR_DIM2_UNUSED){
			ERROR(ERR_SORT_2D);
		}
		if (extra[k-3] == data){
			// This does not work in this implemtation but would be valid in PTC
			// TODO:IMPL:NONE Implement this...? (Is it possible with remaining memory?)
			ERROR(ERR_SORT_SAME_ARRAY);
		}
		extra[k-3] += 2; // move past dimension info
	}
//	iprintf("%d\n", array_min_size);
	STACK_INT_RANGE(0,0,array_min_size-1,start);
	STACK_INT_RANGE(1,0,array_min_size-start,size);

	// now, begin sort
	// +2 to skip dimension info stored in array memory
	quick_sort(data+2, extra, start, start + size - 1, comp);
}

void cmd_sort(struct ptc* p){
	_cmd_sort_shared(p, ARG(2)->type & VAR_STRING ? str_sort_comp : fixp_sort_comp);
}

void cmd_rsort(struct ptc* p){
	_cmd_sort_shared(p, ARG(2)->type & VAR_STRING ? str_rsort_comp : fixp_rsort_comp);
}
