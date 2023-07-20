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
		struct ptc ptc = {0};
		struct console* c = &ptc.console;
		char* code = "?A,B,C,\r";
		
		run_code(code, &ptc);
		
		ASSERT(con_text_getc(c, 0, 0) == to_wide('0'), "[console] Console contents 0");
		ASSERT(con_text_getc(c, 4, 0) == to_wide('0'), "[console] Console contents 4");
		ASSERT(con_text_getc(c, 8, 0) == to_wide('0'), "[console] Console contents 8");
		ASSERT(c->x == 12, "[console] Trailing tab works");
		
		free_code(&ptc);
	}
	
	// Tabs test
	{
		struct ptc ptc = {0};
		struct console* c = &ptc.console;
		char* code = "?,,,,,,,,0\r";
		
		run_code(code, &ptc);
		
		ASSERT(con_text_getc(c, 0, 1) == to_wide('0'), "[console] After 8 tabs 0");
		ASSERT(c->x == 0, "[console] Tab works x");
		ASSERT(c->y == 2, "[console] Tab works y");
		
		free_code(&ptc);
	}
	
	// Mixed test I (ends in newline)
	{
		struct ptc ptc = {0};
		struct console* c = &ptc.console;
		char* code = "PRINT 7,8;9;10,11\r";
		
		run_code(code, &ptc);
		
		ASSERT(con_text_getc(c, 0, 0) == to_wide('7'), "[console] 7 at 0,0");
		ASSERT(con_text_getc(c, 4, 0) == to_wide('8'), "[console] 8 after tab");
		ASSERT(con_text_getc(c, 5, 0) == to_wide('9'), "[console] 9 after semi");
		ASSERT(con_text_getc(c, 6, 0) == to_wide('1'), "[console] 10 after semi");
		ASSERT(con_text_getc(c, 7, 0) == to_wide('0'), "[console] 10 after semi II");
		ASSERT(con_text_getc(c, 12, 0) == to_wide('1'), "[console] 11 after tab");
		ASSERT(con_text_getc(c, 13, 0) == to_wide('1'), "[console] 11 after tab II");
		
		ASSERT(c->x == 0, "[console] Mixed test I x");
		ASSERT(c->y == 1, "[console] Mixed test I y");
		
		free_code(&ptc);
	}
	
	SUCCESS("test_console success");
}
