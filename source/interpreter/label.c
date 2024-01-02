#include "label.h"

#include "strs.h"

// Finds a location for a *new* label
int find_label_slot(struct labels* l, char* name, uint_fast8_t len){
	struct label_line* label;
	u32 hash = name_hash(name, len, MAX_LABELS);
	u32 step = 0;
	
	do {
		hash = (hash + step) % MAX_LABELS;
		label = &l->entry[hash];
		if (label->name[0] == '\0'){
			return hash; // available slot!
		}
		if (step++ >= MAX_LABELS){
			return -1;
		}
	} while (!namecmp(name, len, label->name));
	// Label is already defined here!
	return -1;
}

bool add_label(struct labels* l, char* name, uint_fast8_t len, idx index){
	int entry_index = find_label_slot(l, name, len);
	
	// Failed to find slot
	if (entry_index == -1){
		return false;
	}
	
	// Copy info into entry
	for (uint_fast8_t i = 0; i < len; ++i){
		l->entry[entry_index].name[i] = name[i];
	}
	l->entry[entry_index].index = index;
	return true;
}

void reset_label(struct labels* l){
	for (int i = 0; i < MAX_LABELS; ++i){
		l->entry[i].name[0] = '\0'; // this indicates unused slot
	}
}

// TODO:CODE:LOW Merge with find_label_slot?
// Finds the location of a previously defined label
int find_label(struct labels* l, char* name, uint_fast8_t len){
	struct label_line* label;
	u32 hash = name_hash(name, len, MAX_LABELS);
	u32 step = 0;
	
	do {
		hash = (hash + step) % MAX_LABELS;
		label = &l->entry[hash];
		if (label->name[0] == '\0'){
			return -1; // available slot -> didn't find it
		} else if (step++ >= MAX_LABELS){
			return -1; // didn't find it anywhere
		}
	} while (!namecmp(name, len, label->name));
	// Label is defined!
	return hash;
}

idx label_index(struct labels* l, char* name, uint_fast8_t len){
	int entry_index = find_label(l, name, len);
	if (entry_index == -1){
		return LABEL_NOT_FOUND;
	}
	return l->entry[entry_index].index;
}

