#include "test_util.h"

#include "interpreter.h"
#include "system.h"
#include "program.h"

#include "ptc.h"
#include "error.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int test_int_code(){
	// Code searching
	{
		char* code = "FOR I=0 TO 9\r\rNEXT\r";
		struct program p = {
			strlen(code), code
		};
		struct bytecode o = init_bytecode();
		
		tokenize(&p, &o);
		
		u32 i = bc_scan(o, 0, BC_OPERATOR);
		ASSERT(i == 6, "[bc_scan] Find index of operator");
		free_bytecode(o);
	}
	
	// Code searching (not faked)
	{
		char* code = "?\"O=\"+\"O=\"\r";
		struct program p = {
			strlen(code), code,
		};
		struct bytecode o = init_bytecode();
		
		tokenize(&p, &o);
		
		u32 i = bc_scan(o, 0, BC_OPERATOR);
		ASSERT(i == 8, "[bc_scan] Find index of operator without being in string");
		free_bytecode(o);
	}
	
	// Code searching (Variable name)
	{
		char* code = "V\3ABC\0L\1A\0";
		struct bytecode o = {8, (u8*)code, NULL, NULL};
		u32 i = bc_scan(o, 0, BC_LABEL);
		ASSERT(i == 6, "[bc_scan] Find index of label past variable name");
	}
	
	// Actual FOR loop
	{
		char* code = "FOR I=0 TO 9\r\rNEXT\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(10), "[for] I=10");
		
		free_code(p);
	}
	
	// FOR loop with content
	{
		char* code = "DIM A[20]\rFOR I=0 TO 19\rA[I]=I\rNEXT\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(20), "[for] I=20");
		for (int i = 0; i < 20; ++i){
			ASSERT(get_arr_entry(&p->vars, "A", 1, VAR_NUMBER | VAR_ARRAY, i, ARR_DIM2_UNUSED)->number == (INT_TO_FP(i)), "[for] A[i]=i");
		}
		
		free_code(p);
	}
	
	// IF construct
	{
		char* code = "IF TRUE THEN A=0:B=1 ELSE B=0:A=1\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(0), "[if] A=0");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(1), "[if] B=1");
		
		free_code(p);
	}
	
	// IF construct
	{
		char* code = "IF FALSE THEN A=0:B=1 ELSE B=0:A=-1\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == -(INT_TO_FP(1)), "[if] A=1");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(0), "[if] B=0");
		
		free_code(p);
	}
	
	// GOTO test
	{
		char* code = "@TEST\rI=I+1\rIF I<5 GOTO @TEST\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(5), "[goto] I=5");
		free_code(p);
	}
	
	// ON GOTO test
	{
		char* code = "ON 2 GOTO @1,@2,@3,@4,@5\r@1\rI=I+1\r@2\rI=I+1\r@3\rI=I+1\r@4\rI=I+1\r@5\rI=I+1\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(3), "[goto] I=3");
		free_code(p);
	}
	
	// END, GOSUB test
	{
		char* code = "FOR I=0 TO 3\rGOSUB @TEST\rNEXT\rEND\r@TEST\rJ=J+I+1\rRETURN\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "J", VAR_NUMBER)->value.number == INT_TO_FP(10), "[goto] J=10");
		free_code(p);
	}
	
	// DATA, READ test
	// READ one number
	{
		char* code = "DATA 523\rREAD D\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(523), "[data] D=523");
		free_code(p);
	}
	
	// READ one number
	{
		char* code = "DATA 532,7\rREAD D,B\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(532), "[data] D=532");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(7), "[data] B=7");
		free_code(p);
	}
	
	// READ one number
	{
		char* code = "DATA 532,7\rREAD D$\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(str_comp(test_var(&p->vars, "D", VAR_STRING)->value.ptr, "S\003532"), "[data] D$=\"532\"");
		free_code(p);
	}
	
	// READ one number
	{
		char* code = "READ D\rDATA 532,7\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(532), "[data] D=532 (DATA past start)");
		free_code(p);
	}
	
	// READ string containing comma
	{
		struct ptc* p = run_code("READ X$\rDATA \"Hello, world!\"\r");
		
		ASSERT(str_comp(test_var(&p->vars, "X", VAR_STRING)->value.ptr, "S\15Hello, world!"), "[data] Read string containing comma");
		
		free_code(p);
	}
	
	// INKEY$ test
	{
		char* code = "A$=INKEY$()\r";
		struct ptc* p = run_code_keys(code, "B", 1);
		// Check inkey string generation method works
		ASSERT(str_comp(test_var(&p->vars, "A", VAR_STRING)->value.ptr, "S\1B"), "[input] A$=\"B\"");
		free_code(p);
	}
	
	// LEN tests
	{
		char* code = "A=LEN(\"\")\rB=LEN(\"ABC\")\rC=LEN(\"Aaa!\"*8)\rD=LEN(\"@\"*256)\r";
		struct ptc* p = run_code(code);
		// Check various lengths
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(0), "[len] Length is 0");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(3), "[len] Length is 3");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(32), "[len] Length is 32");
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(256), "[len] Length is 256");
		free_code(p);
	}
	
	// MID$ tests
	{
		char* code = "A$=\"Test String! :)\"\rB$=MID$(A$,0,4)\rC$=MID$(A$,0,32)\rD$=MID$(A$,20,3)\rE$=MID$(A$,5,0)\r";
		char* a_str= "S\17Test String! :)";
		char* b_str= "S\4Test";
		char* c_str= "S\17Test String! :)";
		char* d_str= "S\0";
		char* e_str= "S\0";
		
		struct ptc* p = run_code(code);
		// Check various substrings
		ASSERT(str_comp(test_var(&p->vars, "A", VAR_STRING)->value.ptr, a_str), "[mid$] Base string A$");
		ASSERT(str_comp(test_var(&p->vars, "B", VAR_STRING)->value.ptr, b_str), "[mid$] B$");
		ASSERT(str_comp(test_var(&p->vars, "C", VAR_STRING)->value.ptr, c_str), "[mid$] C$");
		ASSERT(str_comp(test_var(&p->vars, "D", VAR_STRING)->value.ptr, d_str), "[mid$] D$");
		ASSERT(str_comp(test_var(&p->vars, "E", VAR_STRING)->value.ptr, e_str), "[mid$] E$");
		
		free_code(p);
	}
	
	// VAL tests...
	{
		char* code = "A=VAL(\"\")\rB=VAL(\"1 0\")\rC=VAL(\"00000000000123456\")\r"
		"D=VAL(\"0.0005\")\rE=VAL(\"AF58 B\")\rF=VAL(\"DBHUG77\")\r"
		"G=VAL(\"4565FGHG\")\rH=VAL(\" \")\rI=VAL(\"..\")\rJ=VAL(\"01.99 88.\")\r"
		"K=VAL(\"&H4FE4HR6 77 6\")\rL=VAL(\"111111 1\")\rM=VAL(\"524287.999994\")\r"
		"N=VAL(\"-0.0005\")\rO=VAL(\"0.6666\")\rP=VAL(\"MN230\")\r";
		
		struct ptc* p = run_code(code);
		// Check various substrings
		CHECK_VAR_INT("A",0);
		CHECK_VAR_INT("B",1);
		CHECK_VAR_INT("C",123456);
		CHECK_VAR_NUM("D",2);
		CHECK_VAR_INT("E",0);
		CHECK_VAR_INT("F",0);
		CHECK_VAR_INT("G",4565);
		CHECK_VAR_INT("H",0);
		CHECK_VAR_INT("I",0);
		CHECK_VAR_NUM("J",8151);
		CHECK_VAR_INT("K",20452);
		CHECK_VAR_INT("L",111111);
		CHECK_VAR_NUM("M",0x7fffffff);
		CHECK_VAR_NUM("N",-2);
		CHECK_VAR_NUM("O",2730);
		CHECK_VAR_INT("P",0);
		
		free_code(p);
	}
	
	// DTREAD tests
	{
		char* code = "DTREAD(\"2023/09/19\"),Y,M,D\r";
		
		struct ptc* p = run_code(code);
		// Check various substrings
		CHECK_VAR_INT("Y",2023);
		CHECK_VAR_INT("M",9);
		CHECK_VAR_INT("D",19);
		
		free_code(p);
	}
	
	// FOR stack stuff test
	{
		char* code = "FOR I=0 TO 1\rFOR J=0 TO 1\r?I,J\rIF J==0 THEN NEXT J\rNEXT I\rNEXT J\r";
		
		struct ptc* p = run_code(code);
		
		CHECK_VAR_INT("I",2); // after FOR ends, this is the result
		CHECK_VAR_INT("J",2);
		ASSERT(p->exec.error == ERR_NONE, "[for] Check NEXT VAR behavior");
		
		free_code(p);
	}
	
	// FOR SAMPLE5 test
	// bugfixes for...
	// - when TO has variable argument (was only read as number)
	// - stack breaking on BC_BEGIN_LOOP (did not clear entries used)
	// - incorrect searching for BC_COMMAND NEXT/FOR (did not use bc_scan)
	{
		char code[] = "SPC = 8\rBGT = 4\rLINEH = 60\r"
		"LINEC = BGT + SPC + LINEH\rDAYW = 8:DRAWD = 30\r"
		"BGL = (256 - (DRAWD - 1) * DAYW - SPC * 2) / 2\r"
		"BGR = BGL + DAYW * (DRAWD - 1) + SPC * 2\r"
		"BGB = BGT + (SPC + LINEH) * 2 + SPC\r"
		"CLS\rGCLS 4\rGFILL BGL, BGT, BGR, BGB, 15\r"
		"GCOLOR 14\rGBOX BGL, BGT, BGR, BGB\r"
		"X = BGL + SPC\rGLINE X, LINEC, BGR - SPC, LINEC\r"
		"Y = BGT + SPC\rX2 = X + DAYW * (DRAWD - 1)\r"
		"GLINE X, Y, X2, Y\rY2 = Y + LINEH * 2\r"
		"FOR I = 1 TO DRAWD\r GLINE X, Y, X, Y2\r X = X + DAYW\r"
		"NEXT\r";
		
		struct ptc* p = run_code(code);
		
		// if loop doesn't run I will not be correct here
		CHECK_VAR_INT("I",31);
		
		free_code(p);
	}
	
	// FOR loop starting with negative value/expression for starting value
	{
		char code[] = "FOR I=-6 TO 0\rS=S+I\rNEXT\r";
		
		struct ptc* p = run_code(code);
		
		CHECK_VAR_NUM("S",-INT_TO_FP(21));
		
		free_code(p);
	}
	
	// FOR loop ending at maximum value (int)
	{
		ASSERT(check_code_error("FOR I=524286 TO 524287\rNEXT\r", ERR_OVERFLOW), "[for] Overfow on FOR ending at 524287 with step 1");
	}
	
	// FOR skip
	{
		char code[] = "FOR I=1 TO -1\rS=3\rNEXT\r";
		
		struct ptc* p = run_code(code);
		
		CHECK_VAR_INT("I",1);
		CHECK_VAR_INT("S",0);
		ASSERT(p->exec.error == ERR_NONE, "[for] No error involving next or for");
		
		free_code(p);
	}
	
	// FOR skip with ignored NEXT
	{
		char code[] = "FOR I=1 TO -1\rIF I THEN NEXT\rS=3\rNEXT\r";
		
		struct ptc* p = run_code(code);
		
		CHECK_VAR_INT("I",1);
		CHECK_VAR_INT("S",0);
		ASSERT(p->exec.error == ERR_NONE, "[for] No error involving next or for");
		
		free_code(p);
	}
	
	// Comments
	{
		char code[] = "' \x0aThis is a comment\x0a\rREM \x0aThis is also a comment\x0a\r";
		
		ASSERT(check_code_error(code, ERR_NONE), "[comment] No errors");
		ASSERT(check_code_error(" ' T%!Y!N%:3ym;;35L:QVY:5;3q;;v5Uwb];[[\\k.;\r", ERR_NONE), "[comment] No errors");
	}
	
	// ELSE comment
	{
		// TODO:IMPL:LOW Doesn't really matter
//		ASSERT(check_code_error("ELSE \x0aWeirdly, this behaves like a comment\x0a\r", ERR_NONE), "[comment] No errors");
	}
	
	// Tokenization errors integrate correctly
	{
		ASSERT(check_code_error("?I+\r", ERR_TEST_OPERATION), "[syntax] Check missing operator argument")
		ASSERT(check_code_error("?PI(6)\r", ERR_TEST_FUNCTION), "[syntax] Check extra function argument")
		ASSERT(check_code_error("LOCATE 7\r", ERR_TEST_COMMAND), "[syntax] Check missing command argument")
		ASSERT(check_code_error("Among Us\r", ERR_TEST_STACK), "[syntax] Check extra stack values")
		ASSERT(check_code_error("PI()\r", ERR_TEST_STACK), "[syntax] Check extra stack values")
		ASSERT(check_code_error("\x0a\r", ERR_SYNTAX), "[syntax] Check extra stack values")
	}
	
	// Simple IF ELSE check (which broke??)
	{
		struct ptc* p = run_code("ST = 1\rTY=69\rIF ST<21 THEN TY=13 ELSE TY=15\r?TY\r");
		
		ASSERT(p->exec.error == ERR_NONE, "[for] No error involving IF test");
		CHECK_VAR_INT("ST",1);
		CHECK_VAR_INT("TY",13);
		
		free_code(p);
	}
	
	// EXEC simple test
	{
		ASSERT(check_code_error("EXEC \"file_does_not_exist.PTC\"\r", ERR_FILE_LOAD_FAILED), "[exec] Check if file load fails correctly");
		struct ptc* p = run_code("EXEC \"tests/data/IFELSE.PTC\"\r"); // TODO:IMPL:MED How to handle search paths?
		ASSERT(p->exec.error == ERR_NONE, "[exec] Program ran with no errors");
		CHECK_VAR_INT("TY",13);
		
		free_code(p);
	}
	
	// DATA multiple lines
	{
		char* code = "DATA 523\rDATA 46\rDATA \"389\"\rREAD D,C,B$\r";
		// run program
		struct ptc* p = run_code(code);
		// check output for correctness
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(523), "[data] D=523");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(46), "[data] C=46");
		ASSERT(str_comp(test_var(&p->vars, "B", VAR_STRING)->value.ptr, "S\003389"), "[data] B$=\"389\"");
		free_code(p);
	}
	
	// DATA no fake bytecode
	{
		ASSERT(check_code_error("DATA ddddddddddddd,ddddddddddd,dddddd\rREAD A$,B$,C$\r", ERR_NONE), "[data] Don't run out of data");
	}
	
	// GOSUB after THEN/ELSE
	{
		struct ptc* p = run_code(
			"IF TRUE THEN GOSUB @L1\r"
			"IF FALSE THEN GOSUB @LT ELSE GOSUB @LF\r"
			"END\r"
			"@L1\rS=13\rRETURN\r"
			"@LT\rD=7\rRETURN\r"
			"@LF\rE=21\rRETURN\r"
		);
		// check output for correctness
		ASSERT(test_var(&p->vars, "S", VAR_NUMBER)->value.number == INT_TO_FP(13), "[gosub] Execute branch L1");
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(0), "[gosub] Don't execute branch LT");
		ASSERT(test_var(&p->vars, "E", VAR_NUMBER)->value.number == INT_TO_FP(21), "[gosub] Execute branch LF");
		
		free_code(p);
		
		p = run_code(
			"IF TRUE THEN GOSUB @L1\r"
			"IF TRUE THEN GOSUB @LT ELSE GOSUB @LF\r"
			"END\r"
			"@L1\rS=13\rRETURN\r"
			"@LT\rD=7\rRETURN\r"
			"@LF\rE=21\rRETURN\r"
		);
		// check output for correctness
		ASSERT(test_var(&p->vars, "S", VAR_NUMBER)->value.number == INT_TO_FP(13), "[gosub] Execute branch L1");
		ASSERT(test_var(&p->vars, "D", VAR_NUMBER)->value.number == INT_TO_FP(7), "[gosub] Execute branch LT");
		ASSERT(test_var(&p->vars, "E", VAR_NUMBER)->value.number == INT_TO_FP(0), "[gosub] Don't execute branch LF");
		
		free_code(p);
	}
	
	// More GOSUB testing
	{
		struct ptc* p = run_code(
			"IF 1 THEN GOSUB @L1\r"
			"GOSUB @L1\r"
			"END\r"
			"@L1\rGOSUB @L2\rRETURN\r"
			"@L2\rGOSUB @L3\rRETURN\r"
			"@L3\rE=E+1\rRETURN\r"
		);
		// check output for correctness
		ASSERT(p->exec.error == ERR_NONE, "[gosub] Nested GOSUB no error");
		ASSERT(test_var(&p->vars, "E", VAR_NUMBER)->value.number == INT_TO_FP(2), "[gosub] Nested GOSUB E=E+1 -> 2");
		
		free_code(p);
	}
	
	// GOSUB-IF interaction test
	{
		struct ptc* p = run_code(
			"IF 0 THEN GOSUB @L\r"
			"GOSUB @L2\r"
			"END\r"
//			"@L\rI=I+1\r"
//			"IF I<10 THEN @L\r"
//			"RETURN\r"
			"@L2\r"
//			"I=I-1\r"
			"RETURN\r"
//			"GOTO @L2\r"
		);
		// check output for correctness
		ASSERT(p->exec.error == ERR_NONE, "[gosub] GOSUB-IF no error");
//		ASSERT(test_var(&p->vars, "I", VAR_NUMBER)->value.number == INT_TO_FP(4), "[gosub] Correct loop value");
		
		free_code(p);
	}
	
	// SWAP
	{
		struct ptc* p = run_code(
			"A=2990\rB=123\rSWAP A,B\r"
			"A$=\"Hi\"\rB$=\"Bye\"\rSWAP A$,B$\r"
		);
		
		ASSERT(p->exec.error == ERR_NONE, "[swap] SWAP no error");
		CHECK_VAR_INT("A",123);
		CHECK_VAR_INT("B",2990);
		CHECK_VAR_STR("A","S\3Bye");
		CHECK_VAR_STR("B","S\2Hi");
		
		free_code(p);
	}
	
	SUCCESS("test_int_code success");
}
