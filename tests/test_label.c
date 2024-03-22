#include "test_util.h"

#include "interpreter/label.h"

int test_label(void){
	// Add label to struct
	MEM_CASE {
		struct labels l = init_labels(MAX_LABELS);
		
		ASSERT(add_label(&l, "A", 1, 0), "[label] Label A was added successfully");
		ASSERT(add_label(&l, "B", 1, 16), "[label] Label B was added successfully");
		ASSERT(add_label(&l, "C", 1, 64), "[label] Label C was added successfully");
		ASSERT(add_label(&l, "D", 1, 527), "[label] Label D was added successfully");
		
		ASSERT(label_index(&l, "A", 1) == 0, "[label] A == 0");
		ASSERT(label_index(&l, "B", 1) == 16, "[label] B == 16");
		ASSERT(label_index(&l, "C", 1) == 64, "[label] C == 64");
		ASSERT(label_index(&l, "D", 1) == 527, "[label] D == 527");
		
		free_labels(l);
	} MEM_CASE_END
	
	// Add labels but prevent duplicates
	MEM_CASE {
		struct labels l = init_labels(MAX_LABELS);
		
		ASSERT(add_label(&l, "A", 1, 0), "[label] Label A was added successfully");
		ASSERT(add_label(&l, "AA", 2, 16), "[label] Label AA was added successfully");
		ASSERT(add_label(&l, "AAA", 3, 64), "[label] Label AAA was added successfully");
		ASSERT(!add_label(&l, "A", 1, 128), "[label] Label A was not re-added");
		
		ASSERT(label_index(&l, "A", 1) == 0, "[label] A == 0");
		ASSERT(label_index(&l, "AA", 2) == 16, "[label] AA == 16");
		ASSERT(label_index(&l, "AAA", 3) == 64, "[label] AAA == 64");
		
		free_labels(l);
	} MEM_CASE_END
	
	// Reset labels
	MEM_CASE {
		struct labels l = init_labels(MAX_LABELS);
		
		ASSERT(add_label(&l, "A", 1, 0), "[label] Label A was added successfully");
		ASSERT(add_label(&l, "B", 1, 16), "[label] Label B was added successfully");
		ASSERT(add_label(&l, "C", 1, 64), "[label] Label C was added successfully");
		ASSERT(add_label(&l, "D", 1, 527), "[label] Label D was added successfully");
		
		ASSERT(label_index(&l, "A", 1) == 0, "[label] A == 0");
		ASSERT(label_index(&l, "B", 1) == 16, "[label] B == 16");
		ASSERT(label_index(&l, "C", 1) == 64, "[label] C == 64");
		ASSERT(label_index(&l, "D", 1) == 527, "[label] D == 527");
		
		reset_label(&l);
		
		ASSERT(label_index(&l, "A", 1) == LABEL_NOT_FOUND, "[label] No A");
		ASSERT(label_index(&l, "B", 1) == LABEL_NOT_FOUND, "[label] No B");
		ASSERT(label_index(&l, "C", 1) == LABEL_NOT_FOUND, "[label] No C");
		ASSERT(label_index(&l, "D", 1) == LABEL_NOT_FOUND, "[label] No D");
		
		free_labels(l);
	} MEM_CASE_END
	
	SUCCESS("test_label success");
}
