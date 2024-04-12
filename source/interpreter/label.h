#pragma once
///
/// @file
/// @brief Defines label management struct and functions.
///
#include "common.h"

/// The maximum number of labels allowed.
///
/// @note Value selected based on analysis of popular large programs.
/// Can be increased if needed. Value should always be a power of two.
#define MAX_LABELS 2048

/// Size of maximum label length, in characters
#define MAX_LABEL_SIZE 16

#define LABEL_NOT_FOUND ((idx)-1)

/// Pair mapping a label to a BC index
struct label_line {
	/// Label name
	char name[MAX_LABEL_SIZE];
	/// Index into program BC
	idx index;
};

/// Label management struct. Used as a hash table mapping names to indexes.
struct labels {
	/// Maximum number of labels.
	int label_count;
	/// List of label -> index pairs.
	struct label_line* entry;
//	struct label_line entry[MAX_LABELS];
};

struct labels init_labels(int);
void free_labels(struct labels);

bool add_label(struct labels* l, char* name, uint_fast8_t len, idx index);

void reset_label(struct labels* l);

/// Finds a label already defined and returns the index of the entry
int find_label(struct labels* l, char* name, uint_fast8_t len);
/// Finds an empty label slot and returns the index of the entry
int find_label_slot(struct labels* l, char* name, uint_fast8_t len);

idx label_index(struct labels* l, char* name, uint_fast8_t len);

struct ptc;
idx search_label(struct ptc* p, void* label);
