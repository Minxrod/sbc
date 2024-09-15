#pragma once
///
/// @file
/// @brief Defines PTC's data reading instructions.
///

/// Return value from read_one_u8 indicating a failure of some kind
#define READ_ONE_ERR -1
/// Delimeter for data chunks within a BC_DATA instruction.
#define BC_DATA_DELIM '\0'
/// Code to use within DATA to indicate an error when READ is used on it
/// (since newlines can't be used normally)
#define BC_DATA_ERROR '\r'
// I hope there doesn't end up needing to be a third state, I'm out of unique characters...

#include "common.h"

struct ptc;
struct stack_entry;

/// Reads from data block src up to len characters into dest
///
/// @return Number of characters read, or READ_ONE_ERR on error
int read_one_u8(struct ptc* p, const u8* src, size_t len, struct stack_entry* dest);

void cmd_read(struct ptc* p);
void cmd_restore(struct ptc* p);

