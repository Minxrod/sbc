#include "test_util.h"

#include "common.h"
#include "program.h"
#include "subsystem/console.h"

#include "system.h"

int test_console(void){
	// Test console creation
	{
		struct console* c = init_console();
		
		ASSERT(c->x == 0, "[console] Correct starting X");
		ASSERT(c->y == 0, "[console] Correct starting Y");
		ASSERT(c->col == 0, "[console] Correct starting color");
		ASSERT(c->tabstep == 4, "[console] Correct starting tabstep");
		free_console(c);
	}
	
	// Test console puts
	{
		struct console* c = init_console();
		
		con_puts(c, "S\006ABC123");
		
		ASSERT(con_text_getc(c, 0, 0) == to_wide('A'), "[console] Console contents");
		ASSERT(con_text_getc(c, 1, 0) == to_wide('B'), "[console] Console contents");
		ASSERT(con_text_getc(c, 2, 0) == to_wide('C'), "[console] Console contents");
		ASSERT(con_text_getc(c, 3, 0) == to_wide('1'), "[console] Console contents");
		ASSERT(con_text_getc(c, 4, 0) == to_wide('2'), "[console] Console contents");
		ASSERT(con_text_getc(c, 5, 0) == to_wide('3'), "[console] Console contents");
		
		ASSERT(c->x == 6, "[console] Correct ending coordinates");
		ASSERT(c->y == 0, "[console] Correct ending coordinates");
		free_console(c);
	}
	
	/*
	 * Console program tests
	 */
	// Tabs test
	{
		char* code = "?A,B,C,\r";
		
		struct ptc* p = run_code(code);
		struct console* c = &p->console;
		
		ASSERT(con_text_getc(c, 0, 0) == to_wide('0'), "[console] Console contents 0");
		ASSERT(con_text_getc(c, 4, 0) == to_wide('0'), "[console] Console contents 4");
		ASSERT(con_text_getc(c, 8, 0) == to_wide('0'), "[console] Console contents 8");
		ASSERT(c->x == 12, "[console] Trailing tab works");
		
		free_code(p);
	}
	
	// Tabs test
	{
		char* code = "?,,,,,,,,0\r";
		
		struct ptc* p = run_code(code);
		struct console* c = &p->console;
		
		ASSERT(con_text_getc(c, 0, 1) == to_wide('0'), "[console] After 8 tabs 0");
		ASSERT(c->x == 0, "[console] Tab works x");
		ASSERT(c->y == 2, "[console] Tab works y");
		
		free_code(p);
	}
	
	// Mixed test I (ends in newline)
	{
		char* code = "PRINT 7,8;9;10,11\r";
		
		struct ptc* p = run_code(code);
		struct console* c = &p->console;
		
		ASSERT(con_text_getc(c, 0, 0) == to_wide('7'), "[console] 7 at 0,0");
		ASSERT(con_text_getc(c, 4, 0) == to_wide('8'), "[console] 8 after tab");
		ASSERT(con_text_getc(c, 5, 0) == to_wide('9'), "[console] 9 after semi");
		ASSERT(con_text_getc(c, 6, 0) == to_wide('1'), "[console] 10 after semi");
		ASSERT(con_text_getc(c, 7, 0) == to_wide('0'), "[console] 10 after semi II");
		ASSERT(con_text_getc(c, 12, 0) == to_wide('1'), "[console] 11 after tab");
		ASSERT(con_text_getc(c, 13, 0) == to_wide('1'), "[console] 11 after tab II");
		
		ASSERT(c->x == 0, "[console] Mixed test I x");
		ASSERT(c->y == 1, "[console] Mixed test I y");
		
		free_code(p);
	}
	
	// Number input
	{
		char* code = "INPUT A\r";
		// run program
		
		struct ptc* p = run_code_keys(code, "5\r", 2);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(5), "[input] A=5");
		free_code(p);
	}
	
	// String input
	{
		char* code = "INPUT A$\r";
		// run program
		
		struct ptc* p = run_code_keys(code, "+\r", 2);
		// check output for correctness
		ASSERT(str_comp(test_var(&p->vars, "A", VAR_STRING)->value.ptr, "S\1+"), "[input] A$=+");
		free_code(p);
	}
	
	// Testing overwriting variable with input
	{
		char* code = "INPUT \"Test\";A$\rINPUT \"Test\";A$\r";
		// run program
		
		struct ptc* p = run_code_keys(code, "+\r-\r", 4);
		// check output for correctness
		ASSERT(str_comp(test_var(&p->vars, "A", VAR_STRING)->value.ptr, "S\1-"), "[input] A$=-");
		free_code(p);
	}
	
	// Testing multiple variable entry
	{
		char* code = "INPUT A,B,C\r";
		// run program
		
		struct ptc* p = run_code_keys(code, "1,2,3\r", 6);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(1), "[input] A=1");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == INT_TO_FP(2), "[input] B=2");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(3), "[input] C=3");
		free_code(p);
	}
	
	// Testing multiple variable entry
	{
		char* code = "INPUT \"Prompt\";A,B$,C\r";
		// run program
		
		struct ptc* p = run_code_keys(code, "12,ABC ,34\r", 11);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == INT_TO_FP(12), "[input] A=12");
		ASSERT(str_comp(test_var(&p->vars, "B", VAR_STRING)->value.ptr, "S\4ABC "), "[input] B$=\"ABC \"");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == INT_TO_FP(34), "[input] C=34");
		free_code(p);
	}
	
	// Locate silent failure on out of range
	{
		struct ptc* p = run_code("LOCATE 3,6\rLOCATE 32,9\r");
		
		ASSERT(p->console.x == 3, "[locate] CSRX == 3");
		ASSERT(p->console.y == 6, "[locate] CSRY == 6");
		
		free_code(p);
	}
	
	// Scrolling
	{
		struct ptc* p = run_code("LOCATE 0,23\r?0\r");
		
		ASSERT(to_wide('0') == con_text_getc(&p->console, 0, 22), "[print] Console scrolled up one");
		
		free_code(p);
	}
	
	// Not scrolling
	{
		struct ptc* p = run_code("LOCATE 0,23\r?\"ABC\";\r");
		
		ASSERT(to_wide('A') == con_text_getc(&p->console, 0, 23), "[print] Console does not scroll up (A)");
		ASSERT(to_wide('B') == con_text_getc(&p->console, 1, 23), "[print] Console does not scroll up (B)");
		ASSERT(to_wide('C') == con_text_getc(&p->console, 2, 23), "[print] Console does not scroll up (C)");
		
		free_code(p);
	}
	
	// Not scrolling (the stupid corner)
	{
		struct ptc* p = run_code("LOCATE 31,23\r?0;\r");
		
		ASSERT(to_wide('0') == con_text_getc(&p->console, 31, 23), "[print] Console does not scroll up (0)");
		
		free_code(p);
	}
	
	// Not scrolling (the stupid corner II)
	{
		struct ptc* p = run_code("LOCATE 28,23\r?0,\r");
		
		ASSERT(to_wide('0') == con_text_getc(&p->console, 28, 23), "[print] Console does not scroll up (0)");
		
		free_code(p);
	}
	
	// COLORing test I (fg only)
	{
		struct ptc* p = run_code("COLOR 3\r?\"TEST\"\r");
		
		ASSERT(0x03 == con_col_get(&p->console, 0, 0), "[color] Console contains correct color");
		ASSERT(0x03 == con_col_get(&p->console, 1, 0), "[color] Console contains correct color");
		ASSERT(0x03 == con_col_get(&p->console, 2, 0), "[color] Console contains correct color");
		ASSERT(0x03 == con_col_get(&p->console, 3, 0), "[color] Console contains correct color");
		ASSERT(0x00 == con_col_get(&p->console, 4, 0), "[color] Color does not extend past text");
		
		free_code(p);
	}
	
	// COLORing test II (fg+bg)
	{
		struct ptc* p = run_code("COLOR 3,8\r?\"TEST\"\r");
		
		ASSERT(0x83 == con_col_get(&p->console, 0, 0), "[color] Console contains correct colors");
		ASSERT(0x83 == con_col_get(&p->console, 1, 0), "[color] Console contains correct colors");
		ASSERT(0x83 == con_col_get(&p->console, 2, 0), "[color] Console contains correct colors");
		ASSERT(0x83 == con_col_get(&p->console, 3, 0), "[color] Console contains correct colors");
		ASSERT(0x00 == con_col_get(&p->console, 4, 0), "[color] Colors do not extend past text");
		
		free_code(p);
	}
	
	// COLOR + scrolling test
	{
		struct ptc* p = run_code("LOCATE 0,23\rCOLOR 7,2\r?\"TEST\"?\"TEST\"\r");
		
		const int col = 0x27;
		ASSERT(col == con_col_get(&p->console, 0, 21), "[color] Scrolled console contains correct colors 0");
		ASSERT(col == con_col_get(&p->console, 1, 21), "[color] Scrolled console contains correct colors 1");
		ASSERT(col == con_col_get(&p->console, 2, 21), "[color] Scrolled console contains correct colors 2");
		ASSERT(col == con_col_get(&p->console, 3, 21), "[color] Scrolled console contains correct colors 3");
		ASSERT(0x00 == con_col_get(&p->console, 4, 21), "[color] Scrolled colors do not extend past text 4");
		
		ASSERT(col == con_col_get(&p->console, 0, 22), "[color] Lowest console contains correct colors");
		ASSERT(col == con_col_get(&p->console, 1, 22), "[color] Lowest console contains correct colors");
		ASSERT(col == con_col_get(&p->console, 2, 22), "[color] Lowest console contains correct colors");
		ASSERT(col == con_col_get(&p->console, 3, 22), "[color] Lowest console contains correct colors");
		ASSERT(col == con_col_get(&p->console, 4, 22), "[color] New rows keep set color");
		ASSERT(col == con_col_get(&p->console, 30, 22), "[color] New rows keep set color");
		ASSERT(col == con_col_get(&p->console, 31, 22), "[color] New rows keep set color");
		
		ASSERT(con_text_getc(&p->console, 0, 23) == 0, "[print] Last row is empty");
		
		free_code(p);
	}
	
	// COLOR + tab test
	{
		struct ptc* p = run_code("COLOR 4,9\r?\"TEST\",\r");
		
		const int col = 0x94;
		ASSERT(col == con_col_get(&p->console, 0, 0), "[color] Correct colors 0");
		ASSERT(col == con_col_get(&p->console, 1, 0), "[color] Correct colors 1");
		ASSERT(col == con_col_get(&p->console, 2, 0), "[color] Correct colors 2");
		ASSERT(col == con_col_get(&p->console, 3, 0), "[color] Correct colors 3");
		ASSERT(col == con_col_get(&p->console, 4, 0), "[color] Colors extend through tab 4");
		ASSERT(col == con_col_get(&p->console, 5, 0), "[color] Colors extend through tab 5");
		ASSERT(col == con_col_get(&p->console, 6, 0), "[color] Colors extend through tab 6");
		ASSERT(col == con_col_get(&p->console, 7, 0), "[color] Colors extend through tab 7");
		
		free_code(p);
	}
	
	SUCCESS("test_console success");
}
