#pragma once
///
/// @file
/// @brief Defines label management struct and functions.
///
#include "common.h"

// If a program ever actually exceeds this, increase value by 2x
#define MAX_LABELS 2048
#define LABEL_NOT_FOUND ((idx)-1)

// Pair mapping labels to BC indexes
struct label_line {
	char name[16];
	idx index;
};

struct labels {
	// List of label -> index
	struct label_line entry[MAX_LABELS];
};

bool add_label(struct labels* l, char* name, uint_fast8_t len, idx index);

void reset_label(struct labels* l);

/// Finds a label already defined and returns the index of the entry
int find_label(struct labels* l, char* name, uint_fast8_t len);
/// Finds an empty label slot and returns the index of the entry
int find_label_slot(struct labels* l, char* name, uint_fast8_t len);

idx label_index(struct labels* l, char* name, uint_fast8_t len);
