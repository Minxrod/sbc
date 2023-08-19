#include "test_console.h"
#include "test_util.h"

#include "common.h"
#include "program.h"

int test_console(void){
	// Test console creation
	{
		struct console c = {0};
		con_init(&c);
		
		ASSERT(c.x == 0, "[console] Correct starting X");
		ASSERT(c.y == 0, "[console] Correct starting Y");
		ASSERT(c.col == 0, "[console] Correct starting color");
		ASSERT(c.tabstep == 4, "[console] Correct starting tabstep");
		
	}
	
	// Test console puts
	{
		struct console c = {0};
		con_init(&c);
		
		con_puts(&c, "S\006ABC123");
		
		ASSERT(con_text_getc(&c, 0, 0) == to_wide('A'), "[console] Console contents");
		ASSERT(con_text_getc(&c, 1, 0) == to_wide('B'), "[console] Console contents");
		ASSERT(con_text_getc(&c, 2, 0) == to_wide('C'), "[console] Console contents");
		ASSERT(con_text_getc(&c, 3, 0) == to_wide('1'), "[console] Console contents");
		ASSERT(con_text_getc(&c, 4, 0) == to_wide('2'), "[console] Console contents");
		ASSERT(con_text_getc(&c, 5, 0) == to_wide('3'), "[console] Console contents");
		
		ASSERT(c.x == 6, "[console] Correct ending coordinates");
		ASSERT(c.y == 0, "[console] Correct ending coordinates");
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
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == 5<<12, "[input] A=5");
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
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == 1<<12, "[input] A=1");
		ASSERT(test_var(&p->vars, "B", VAR_NUMBER)->value.number == 2<<12, "[input] B=2");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == 3<<12, "[input] C=3");
		free_code(p);
	}
	
	// Testing multiple variable entry
	{
		char* code = "INPUT \"Prompt\";A,B$,C\r";
		// run program
		
		struct ptc* p = run_code_keys(code, "12,ABC ,34\r", 11);
		// check output for correctness
		ASSERT(test_var(&p->vars, "A", VAR_NUMBER)->value.number == 12<<12, "[input] A=12");
		ASSERT(str_comp(test_var(&p->vars, "B", VAR_STRING)->value.ptr, "S\4ABC "), "[input] B$=\"ABC \"");
		ASSERT(test_var(&p->vars, "C", VAR_NUMBER)->value.number == 34<<12, "[input] C=34");
		free_code(p);
	}
	
	SUCCESS("test_console success");
}
