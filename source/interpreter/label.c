#include "label.h"

#include "strs.h"
#include "system.h"
#include "ptc.h"

#include <assert.h>

struct labels init_labels(int label_count){
	assert((label_count & (label_count - 1)) == 0); // 2^n
	struct labels l;
	l.label_count = label_count;
	if (label_count){
		l.entry = calloc_log("init_labels", sizeof(struct label_line), label_count);
	} else {
		l.entry = NULL;
	}
	return l;
}

void free_labels(struct labels l){
	free_log("free_labels", l.entry);
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
	for (int i = 0; i < l->label_count; ++i){
		l->entry[i].name[0] = '\0'; // this indicates unused slot
	}
}

// Finds the location of a previously defined label
int find_label(struct labels* l, char* name, uint_fast8_t len){
	struct label_line* label;
	u32 hash = name_hash(name, len, l->label_count);
	u32 step = 0;
	
	do {
		hash = (hash + step) & (l->label_count - 1); // & label_count
		label = &l->entry[hash];
		if (label->name[0] == '\0'){
			return -1; // available slot -> didn't find it
		}
		if (step++ >= (u32)l->label_count){
			return -1; // didn't find it anywhere
		}
	} while (!namecmp(name, len, label->name));
	// Label is defined!
	return hash;
}

// Finds a location for a *new* label
int find_label_slot(struct labels* l, char* name, uint_fast8_t len){
	struct label_line* label;
	u32 hash = name_hash(name, len, l->label_count);
	u32 step = 0;

	do {
		hash = (hash + step) & (l->label_count - 1); // % label_count
		label = &l->entry[hash];
		if (label->name[0] == '\0'){
			return hash; // available slot!
		}
		if (step++ >= (u32)l->label_count){
			return -1;
		}
	} while (!namecmp(name, len, label->name));
	// Label is already defined here!
	return -1;
}

idx label_index(struct labels* l, char* name, uint_fast8_t len){
	int entry_index = find_label(l, name, len);
	if (entry_index == -1){
		return LABEL_NOT_FOUND;
	}
	return l->entry[entry_index].index;
}

idx search_label(struct ptc* p, void* label){
	assert(label); // label must exist
	// Labels must always be located at the beginning of a line.
	idx index = 0;

	u8 buf[MAX_LABEL_SIZE];
	char* buf_ptr = (char*)buf;
	int len = str_len(label);
	if (*(char*)label == BC_LABEL || *(char*)label == BC_LABEL_STRING){
		assert(len <= 16);
		str_char_copy(label, buf);
	} else { // assume regular string
		assert(len <= 17);
		str_char_copy(label, buf);
		++buf_ptr; // pointer past '@'
		--len; // remove '@'
	}
	assert(len <= 16);
	index = label_index(&p->exec.code.labels, buf_ptr, len);
	if (index == LABEL_NOT_FOUND){
		p->exec.error = ERR_LABEL_NOT_FOUND;
		return p->exec.code.size; // error'd anyways, skip to end
	}
	return index;
}
