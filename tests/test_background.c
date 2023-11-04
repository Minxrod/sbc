#include "test_background.h"
#include "test_util.h"

#include "common.h"
#include "program.h"

#include "background.h"

int test_background(void){
	// Initialization
	{
		struct background* b = init_background();
		
		ASSERT(b->page == 0, "[bg] Default page is zero");
		
		for (int p = 0; p <= 1; ++p){
			for (int l = 0; l <= 1; ++l){
				ASSERT(b->ofs[p][l].x == 0, "[bg] Default x-scroll is zero");
				ASSERT(b->ofs[p][l].y == 0, "[bg] Default y-scroll is zero");
			}
		}
		
		free_background(b);
	}
	
	// Basic BGPAGE
	{
		struct ptc* p = run_code("BGPAGE 1\r");
		
		ASSERT(p->background.page == 1, "[bg] BGPAGE is set correctly");
		
		free_code(p);
	}
	
	// Simple tiledata BGPUT
	{
		struct ptc* p = run_code("BGPUT 0,1,2,3456\rBGPUT 1,33,34,1235\r");
		
		ASSERT(p->res.scr[0][bg_index(1,2)] == 3456, "[bg] BGPUT tiledata test");
		ASSERT(p->res.scr[1][bg_index(33,34)] == 1235, "[bg] BGPUT tiledata test II");
		
		free_code(p);
	}
	
	// Simple BGCLR
	{
		struct ptc* p = run_code("BGPUT 0,1,2,3456\rBGPUT 1,33,34,1235\rBGCLR\r");
		
		ASSERT(p->res.scr[0][bg_index(1,2)] == 0, "[bg] BGCLR test");
		ASSERT(p->res.scr[1][bg_index(33,34)] == 0, "[bg] BGCLR test II");
		
		free_code(p);
	}
	
	
	// Simple BGFILL
	{
		struct ptc* p = run_code("BGFILL 1,3,4,36,37,1024\r");
		
		ASSERT(p->res.scr[1][bg_index(3,4)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(36,4)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(36,37)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(3,37)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(20,20)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(2,4)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(3,3)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(37,37)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(36,38)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(2,37)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[1][bg_index(3,38)] == 0, "[bg] BGFILL test");
		
		free_code(p);
	}
	
	SUCCESS("test_background success");
}
