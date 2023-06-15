#pragma once

#ifdef ARM9
#include <nds/ndstypes.h>
#endif

// silly hacks for testing without a DS emu
#ifndef ARM9
#include <stdbool.h>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
#define iprintf printf
#endif
