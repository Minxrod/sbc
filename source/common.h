#pragma once

#include <stdint.h>
#include <stdio.h> // debugging

#ifdef ARM9
#include <nds/ndstypes.h>
#include <nds.h>
#endif

// silly hacks for testing without a DS emu
#ifndef ARM9
#include <stdbool.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
#define iprintf printf
#endif
