#include "test_util.h"

#include "system.h"

int test_int_func(){
	// FLOOR
	{
		char* code = "A=FLOOR(3.5)\rB=FLOOR(4.67)\rC=FLOOR(2)\rD=FLOOR(-1.3)\rE=FLOOR(-2)\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(3), "[math] A=FLOOR(3.5)");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(4), "[math] B=FLOOR(4.67)");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(2), "[math] C=FLOOR(2)");
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == -INT_TO_FP(2), "[math] D=FLOOR(-1.3)");
		ASSERT(test_var(&p->vars, "E", VAR_NUMBER)->value.number == -INT_TO_FP(2), "[math] E=FLOOR(-2)");
		
		free_code(p);
	}
	
	// INSTR
	{
		char* code = "A$=\"ABCDEFGHI\"\rA=INSTR(A$,A$)\rB=INSTR(A$,\"\")\rC=INSTR(A$,\"DEF\")\rD=INSTR(A$,\"AAA\")\rE=INSTR(\"\",A$)\r"
		"F=INSTR(A$,\"A\",1)\rG=INSTR(A$,\"HI\")\r"
		"H=INSTR(\"AABC\",\"AB\")\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(0), "[instr] Find string in itself");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(0), "[instr] Find empty string in string");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(3), "[instr] Find string in middle of string");
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == -INT_TO_FP(1), "[instr] Can't find string in string");
		ASSERT(test_var(&p->vars, "E", VAR_NUMBER)->value.number == -INT_TO_FP(1), "[instr] Can't find string in empty string");
		ASSERT(test_var(&p->vars, "F", VAR_NUMBER)->value.number == -INT_TO_FP(1), "[instr] Can't find string past correct location");
		ASSERT(test_var(&p->vars, "G", VAR_NUMBER)->value.number == INT_TO_FP(7), "[instr] Match string that reaches end");
		ASSERT(test_var(&p->vars, "H", VAR_NUMBER)->value.number == INT_TO_FP(1), "[instr] Match string with repeat characters");
		
		free_code(p);
	}
	
	// SUBST
	{
		// add "" to force string alloc; check usages to ensure stack is not causing copies to live too long
		char* code = "A$=\"ABCDEFGHI\"+\"\"\rB$=SUBST$(A$,3,2,\"XYZ\")\rC$=SUBST$(A$,0,5,\"XY\")\rD$=SUBST$(A$,9,4,\"XYZW\")\rE$=SUBST$(A$,6,0,\"XY\")\rF$=SUBST$(A$,0,0,\"XY\")\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(str_comp(test_var(&p->vars, "A", VAR_STRING)->value.ptr, "S\11ABCDEFGHI"), "[subst] Original string unmodified");
		ASSERT(str_comp(test_var(&p->vars, "B", VAR_STRING)->value.ptr, "S\12ABCXYZFGHI"), "[subst] Substitute more characters than replaced");
		ASSERT(str_comp(test_var(&p->vars, "C", VAR_STRING)->value.ptr, "S\6XYFGHI"), "[subst] Substitute less characters than replaced");
		ASSERT(str_comp(test_var(&p->vars, "D", VAR_STRING)->value.ptr, "S\15ABCDEFGHIXYZW"), "[subst] Substitute past end of string");
		ASSERT(str_comp(test_var(&p->vars, "E", VAR_STRING)->value.ptr, "S\13ABCDEFXYGHI"), "[subst] Replace zero characters in string");
		ASSERT(str_comp(test_var(&p->vars, "F", VAR_STRING)->value.ptr, "S\13XYABCDEFGHI"), "[subst] Replace zero characters at beginning of string");
		
		ASSERT(((struct string*)test_var(&p->vars, "A", VAR_STRING)->value.ptr)->type == STRING_CHAR, "[str_uses] Correct original string type");
		ASSERT(((struct string*)test_var(&p->vars, "A", VAR_STRING)->value.ptr)->uses == 1, "[str_uses] Correct original string uses");
		
		free_code(p);
	}
	
	// ASC
	{
		char* code = "A=ASC(\"0\")\rB=ASC(\"A\")\rC=ASC(CHR$(0))\rD=ASC(\"abc\")\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(48), "[asc] Value of \"0\" string");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(65), "[asc] Value of \"A\" string");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(0), "[asc] Value of CHR$(0) string");
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(97), "[asc] Value of first char of \"abc\" string");
		
		free_code(p);
	}
	
	// CHR$
	{
		char* code = "A$=CHR$(48)\rB$=CHR$(65)\rC$=CHR$(0)\r";
		
		struct ptc* p = run_code(code);
		
		ASSERT(str_comp(test_var(&p->vars, "A", VAR_STRING)->value.ptr, "S\1\x30"), "[chr$] \"0\" string");
		ASSERT(str_comp(test_var(&p->vars, "B", VAR_STRING)->value.ptr, "S\1\x41"), "[chr$] \"A\" string");
		ASSERT(str_comp(test_var(&p->vars, "C", VAR_STRING)->value.ptr, "S\1\x00"), "[chr$] CHR$(0)");
		
		free_code(p);
	}
	
	// CHR$+ASC inverses
	{
		// ASC(CHR$())
		struct ptc* p = run_code(
			"S=0\r"
			"FOR I=0 TO 255\r"
			" IF ASC(CHR$(I))!=I THEN S=S+1\r"
			"NEXT\r"
		);
		
		CHECK_VAR_INT("S", 0); // No errors detected
		
		free_code(p);
		
		// CHR$(ASC())
		p = run_code(
			"DIM C$[256]\r"
			"S=0\r"
			"FOR I=0 TO 255\r"
			" C$[I]=CHR$(I)"
			" IF CHR$(ASC(C$[I]))!=C$[I] THEN S=S+1\r"
			"NEXT\r"
		);
		
		CHECK_VAR_INT("S", 0); // No errors detected
		
		free_code(p);
	}
	
	// STR$
	{
		char* code = "A$=STR$(7)\rB$=STR$(42.3)\rC$=STR$(-6.9)\rD$=STR$(423786)\rE$=STR$(524287)\r";
		
		struct ptc* p = run_code(code);
		
		CHECK_VAR_STR("A", "S\0017");
		CHECK_VAR_STR("B", "S\00442.3");
		CHECK_VAR_STR("C", "S\004-6.9");
		CHECK_VAR_STR("D", "S\006423786");
		CHECK_VAR_STR("E", "S\006524287");
		
		free_code(p);
	}
	
#ifdef TEST_FULL
	// STR + VAL inverse integers
	{
		// Check every integer (without overflow) for correct STR$-VAL conversion
		char* code = "S=1\rFOR I=-524287 TO 524286\rIF VAL(STR$(I))!=I THEN S=0\rNEXT\r";
		
		struct ptc* p = run_code(code);
		
		CHECK_VAR_INT("S", 1);
		
		free_code(p);
	}
#endif // TEST_FULL
	
	// RIGHT$
	{
		struct ptc* p = run_code(
			"A$=\"ABCDEFGHIJKLMNOP\"\r"
			"B$=RIGHT$(A$,8)\r" // within range
			"C$=RIGHT$(A$,0)\r" // nothing
			"D$=RIGHT$(A$,26)\r" //larger than string
			"E$=RIGHT$(A$,257)\r" //larger than possible
			"F$=RIGHT$(A$,1)\r" //simple
		);
		
		ASSERT(p->exec.error == ERR_NONE, "[right$] No errors");
		CHECK_VAR_STR("A", "S\20ABCDEFGHIJKLMNOP");
		CHECK_VAR_STR("B", "S\10IJKLMNOP");
		CHECK_VAR_STR("C", "S\0");
		CHECK_VAR_STR("D", "S\20ABCDEFGHIJKLMNOP");
		CHECK_VAR_STR("E", "S\20ABCDEFGHIJKLMNOP");
		CHECK_VAR_STR("F", "S\1P");
		
		free_code(p);
	}
	
	// LEFT$
	{
		struct ptc* p = run_code(
			"A$=\"ABCDEFGHIJKLMNOP\"\r"
			"B$=LEFT$(A$,8)\r" // within range
			"C$=LEFT$(A$,0)\r" // nothing
			"D$=LEFT$(A$,26)\r" //larger than string
			"E$=LEFT$(A$,257)\r" //larger than possible
			"F$=LEFT$(A$,1)\r" //simple
		);
		
		ASSERT(p->exec.error == ERR_NONE, "[left$] No errors");
		CHECK_VAR_STR("A", "S\20ABCDEFGHIJKLMNOP");
		CHECK_VAR_STR("B", "S\10ABCDEFGH");
		CHECK_VAR_STR("C", "S\0");
		CHECK_VAR_STR("D", "S\20ABCDEFGHIJKLMNOP");
		CHECK_VAR_STR("E", "S\20ABCDEFGHIJKLMNOP");
		CHECK_VAR_STR("F", "S\1A");
		
		free_code(p);
	}
	
	// HEX$
	{
		struct ptc* p = run_code(
			"A$=HEX$(9)\r" // simple
			"B$=HEX$(29.7)\r" // decimal
			"C$=HEX$(89.6)\r"
			"D$=HEX$(-5)\r" //negative
			"E$=HEX$(-967)\r"
			"F$=HEX$(-2585.6)\r" //negative decimal
			"G$=HEX$(9,1)\r" //positive with digits
			"H$=HEX$(9,2)\r"
			"I$=HEX$(9,3)\r"
			"J$=HEX$(9,4)\r"
			"K$=HEX$(9,5)\r"
			"L$=HEX$(-9,2)\r" //negative with digits
			"M$=HEX$(-9,3)\r"
			"N$=HEX$(-9,4)\r"
			"O$=HEX$(-9,5)\r"
			"P$=HEX$(-524287)\r" // edge cases
			"Q$=HEX$(-524287.8)\r"
			"R$=HEX$(524287)\r"
			"S$=HEX$(524287.8)\r"
			"T$=HEX$(-8,1)\r" // edge cases 2-arg
			"U$=HEX$(15,1)\r"
		);
		
		CHECK_VAR_STR("A","S\0019");
		CHECK_VAR_STR("B","S\0021D");
		CHECK_VAR_STR("C","S\00259");
		CHECK_VAR_STR("D","S\005FFFFB");
		CHECK_VAR_STR("E","S\005FFC39");
		CHECK_VAR_STR("F","S\005FF5E6");
		
		CHECK_VAR_STR("G","S\0019");
		CHECK_VAR_STR("H","S\00209");
		CHECK_VAR_STR("I","S\003009");
		CHECK_VAR_STR("J","S\0040009");
		CHECK_VAR_STR("K","S\00500009");
		
		CHECK_VAR_STR("L","S\002F7");
		CHECK_VAR_STR("M","S\003FF7");
		CHECK_VAR_STR("N","S\004FFF7");
		CHECK_VAR_STR("O","S\005FFFF7");
		
		CHECK_VAR_STR("P","S\00580001");
		CHECK_VAR_STR("Q","S\00580000");
		CHECK_VAR_STR("R","S\0057FFFF");
		CHECK_VAR_STR("S","S\0057FFFF");
		
		CHECK_VAR_STR("T","S\0018");
		CHECK_VAR_STR("U","S\001F");
	}
	
	SUCCESS("test_int_func success");
}
