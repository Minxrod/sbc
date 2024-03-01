#pragma once

/// Maximum allowed memory allocation (PC)
/// Set to 6MB to allow 4MB RAM + ~660KB VRAM + extra for 64-bit pointers
#define MAX_MEMORY (6*1024*1024)

// extern so sbc_memory can access value
extern void* sbc_membase;

void init_memory(int);
void free_memory();
// Intended for allocations that don't ever get free'd while the system is active
void* sbc_malloc(int size);
void* sbc_calloc(int count, int size);
void sbc_free(void* ptr);
