#include "sbc_blockalloc.h"

#include "common.h" // debug logs + types

#include <string.h>
#include <assert.h>

void* sbc_membase = NULL; // must be initalized prior to use
static u8* sbc_memnext = NULL;

void init_memory(int memory_max){
	iprintf("init_memory block\n");
	assert(!sbc_membase); // do not leak existing memory
	assert(!sbc_memnext);
	if (!sbc_membase){
		sbc_membase = malloc(memory_max);
		sbc_memnext = sbc_membase;
	}
}

void free_memory(){
	iprintf("free_memory block\n");
	free(sbc_membase); // frees all memory at once
	sbc_membase = NULL; // allow re-alloc with init_memory again
	sbc_memnext = NULL; // safety...
}

void* sbc_malloc(int size){
	assert(sbc_membase);
	assert(sbc_memnext);
	void* alloc = sbc_memnext; // current alloc goes here
	sbc_memnext += size; // advance by size bytes
	iprintf("sbc_malloc'd %d at %p\n", size, alloc);
	return alloc;
}

void* sbc_calloc(int count, int size){
	assert(sbc_membase);
	assert(sbc_memnext);
	int alloc_size = count * size;
	void* alloc = sbc_memnext; // current alloc goes here
	sbc_memnext += alloc_size; // advance by size bytes
	memset(alloc, 0, alloc_size); // zero out memory
	iprintf("sbc_calloc'd %d(%d*%d) at %p\n", count*size, count, size, alloc);
	return alloc;
}

void sbc_free(void* ptr){
	(void)ptr;
	// doesn't need to do anything for the block allocation because
	// it all gets free'd at the end anyways
}
