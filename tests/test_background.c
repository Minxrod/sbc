#include "test_util.h"

#include "system.h"
#include "common.h"
#include "program.h"

#include "background.h"

int test_background(void){
	// Initialization
	MEM_CASE {
		struct background* b = init_background();
		
		ASSERT(b->page == 0, "[bg] Default page is zero");
		
		for (int p = 0; p <= 1; ++p){
			for (int l = 0; l <= 1; ++l){
				ASSERT(b->ofs[p][l].x == 0, "[bg] Default x-scroll is zero");
				ASSERT(b->ofs[p][l].y == 0, "[bg] Default y-scroll is zero");
			}
		}
		
		free_background(b);
	} MEM_CASE_END
	
	// Basic BGPAGE
	{
		struct ptc* p = run_code("BGPAGE 1\r");
		
		ASSERT(p->background.page == 1, "[bg] BGPAGE is set correctly");
		
		free_code(p);
	}
	
	// Simple tiledata BGPUT
	{
		struct ptc* p = run_code("BGPUT 0,1,2,3456\rBGPUT 1,33,34,1235\r");
		
		ASSERT(p->res.scr[2][bg_index(1,2)] == 3456, "[bg] BGPUT tiledata test");
		ASSERT(p->res.scr[3][bg_index(33,34)] == 1235, "[bg] BGPUT tiledata test II");
		
		free_code(p);
	}
	
	// Simple BGCLR
	{
		struct ptc* p = run_code("BGPUT 0,1,2,3456\rBGPUT 1,33,34,1235\rBGCLR\r");
		
		ASSERT(p->res.scr[2][bg_index(1,2)] == 0, "[bg] BGCLR test");
		ASSERT(p->res.scr[3][bg_index(33,34)] == 0, "[bg] BGCLR test II");
		
		free_code(p);
	}
	
	
	// Simple BGFILL
	{
		struct ptc* p = run_code("BGFILL 1,3,4,36,37,1024\r");
		
		ASSERT(p->res.scr[3][bg_index(3,4)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(36,4)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(36,37)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(3,37)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(20,20)] == 1024, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(2,4)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(3,3)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(37,37)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(36,38)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(2,37)] == 0, "[bg] BGFILL test");
		ASSERT(p->res.scr[3][bg_index(3,38)] == 0, "[bg] BGFILL test");
		
		free_code(p);
	}
	
	// Tiledata
	{
		ASSERT(to_tiledata(0,0,0,0) == 0, "[tile] Basic case");
		ASSERT(to_tiledata(256,0,0,0) == 256, "[tile] Basic case ");
		ASSERT(to_tiledata(512,0,0,0) == 512, "[tile] Basic case ");
		ASSERT(to_tiledata(768,0,0,0) == 768, "[tile] Basic case ");
		
		ASSERT(to_tiledata(0,0,1,0) == 1024, "[tile] Horizontal flip");
		ASSERT(to_tiledata(0,0,0,1) == 2048, "[tile] Vertical flip");
		ASSERT(to_tiledata(0,0,1,1) == 3072, "[tile] Rotate 180");
		
		ASSERT(to_tiledata(256,3,0,0) == 256+12288, "[tile] Palette 3");
		ASSERT(to_tiledata(512,5,0,0) == 512+20480, "[tile] Palette 5");
		ASSERT(to_tiledata(768,7,0,0) == 768+28672, "[tile] Palette 7");
		
		ASSERT(to_tiledata(1023,15,1,1) == 65535, "[tile] Everything");
	}
	
	SUCCESS("test_background success");
}
