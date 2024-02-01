#pragma once
///
/// @file
/// @brief Defines PTC's data reading instructions.
///

#define READ_ONE_ERR -1
#define BC_DATA_DELIM '\0'

#include "common.h"

struct ptc;
struct stack_entry;

// Reads from src up to len characters into e
// Returns characters read
//int read_one(struct ptc* p, u16* src, size_t len, struct stack_entry* e);
int read_one_u8(struct ptc* p, u8* src, size_t len, struct stack_entry* e);

void cmd_read(struct ptc* p);
void cmd_restore(struct ptc* p);

