#pragma once

/// Maximum allowed memory allocation (PC)
/// Set to 8MB to allow 4MB RAM + ~660KB VRAM + extra for 64-bit pointers
/// Extra room allows for STRING_WIDE to be enabled easily
// TODO:IMPL:NONE make this configurable?
#define MAX_MEMORY (8*1024*1024)

// extern so sbc_memory can access value
extern void* sbc_membase;

void init_memory(int);
void reset_memory(void);
void free_memory(void);
// Intended for allocations that don't ever get free'd while the system is active
void* sbc_malloc(int size);
void* sbc_calloc(int count, int size);
void sbc_free(void* ptr);
