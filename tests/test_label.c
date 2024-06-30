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

	// Low-memory case behaves correctly
	MEM_CASE {
		struct labels l = init_labels(4);

		ASSERT(add_label(&l, "A", 1, 0), "[label] Label A was added successfully");
		ASSERT(add_label(&l, "B", 1, 16), "[label] Label B was added successfully");
		ASSERT(add_label(&l, "C", 1, 64), "[label] Label C was added successfully");
		ASSERT(add_label(&l, "D", 1, 527), "[label] Label D was added successfully");
		DENY(add_label(&l, "E", 1, 530), "[label] Label E was not added");

		free_labels(l);
	} MEM_CASE_END

	// Specific case failure?
	MEM_CASE {
		struct labels l = init_labels(32);
#define LBL_ADD(lbl) ASSERT(add_label(&l, lbl, strlen(lbl), 0), "[label] Label "lbl"was added successfully");
#define LBL_CHK(lbl) ASSERT(LABEL_NOT_FOUND != label_index(&l, lbl, strlen(lbl)), "[label] Label "lbl"was found successfully");

		LBL_ADD("INIT");
		LBL_ADD("TESTSKP");
		LBL_ADD("NOINIT");
		LBL_ADD("LOOP");
		LBL_ADD("RESET");
		LBL_ADD("COPY");
		LBL_ADD("EXEC");
		LBL_ADD("RUN");
		LBL_ADD("INPUT");
		LBL_ADD("MOVC");
		LBL_ADD("BSPC");
		LBL_ADD("DELL");
		LBL_ADD("ADDC");
		LBL_ADD("ADDL");
		LBL_ADD("DSCROLL");
		LBL_ADD("DSCRDWNSKIP");
		LBL_ADD("DSCRDOWN");
		LBL_ADD("DSCRUP");
		LBL_ADD("READPRG");
		LBL_ADD("READPRGS");
		LBL_ADD("WRITEPRG");
		LBL_ADD("DINIT");
		LBL_ADD("DCODE");
		LBL_ADD("DLINE");
		LBL_ADD("DLSKP");
		LBL_ADD("DRLINE");
		LBL_ADD("DLNUM");
		LBL_ADD("HLLBL");
		LBL_ADD("HLLBLL");
		LBL_ADD("HLNAME");
		LBL_ADD("HLNAMEL");
		LBL_ADD("HLINIT");

		LBL_CHK("INIT");
		LBL_CHK("TESTSKP");
		LBL_CHK("NOINIT");
		LBL_CHK("LOOP");
		LBL_CHK("RESET");
		LBL_CHK("COPY");
		LBL_CHK("EXEC");
		LBL_CHK("RUN");
		LBL_CHK("INPUT");
		LBL_CHK("MOVC");
		LBL_CHK("BSPC");
		LBL_CHK("DELL");
		LBL_CHK("ADDC");
		LBL_CHK("ADDL");
		LBL_CHK("DSCROLL");
		LBL_CHK("DSCRDWNSKIP");
		LBL_CHK("DSCRDOWN");
		LBL_CHK("DSCRUP");
		LBL_CHK("READPRG");
		LBL_CHK("READPRGS");
		LBL_CHK("WRITEPRG");
		LBL_CHK("DINIT");
		LBL_CHK("DCODE");
		LBL_CHK("DLINE");
		LBL_CHK("DLSKP");
		LBL_CHK("DRLINE");
		LBL_CHK("DLNUM");
		LBL_CHK("HLLBL");
		LBL_CHK("HLLBLL");
		LBL_CHK("HLNAME");
		LBL_CHK("HLNAMEL");
		LBL_CHK("HLINIT");

		free_labels(l);
	} MEM_CASE_END


	SUCCESS("test_label success");
}
