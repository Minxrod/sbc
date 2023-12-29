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
	
	SUCCESS("test_int_func success");
}
