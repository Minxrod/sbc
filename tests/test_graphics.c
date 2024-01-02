#include "test_util.h"

#include "subsystem/graphics.h"
#include "system.h"

int test_graphics(void){
	// Simple construction test
	{
		struct graphics g = {0};
		init_graphics(&g);
		
		ASSERT(g.info[0].drawpage == 0, "[graphics] Screen 0 default draw page 0");
		ASSERT(g.info[0].displaypage == 0, "[graphics] Screen 0 default display page 0");
		ASSERT(g.info[0].prio == 3, "[graphics] Screen 0 default priorty 3");
		ASSERT(g.info[1].drawpage == 1, "[graphics] Screen 1 default draw page 1");
		ASSERT(g.info[1].displaypage == 1, "[graphics] Screen 1 default display page 1");
		ASSERT(g.info[1].prio == 3, "[graphics] Screen 1 default priorty 3");
	}
	
	// Simple GPAGE
	{
		struct ptc* p = run_code("GPAGE 1\r");
		
		ASSERT(p->graphics.screen == 1, "[graphics] Page set to 1");
		
		free_code(p);
	}
	
	// Simple GPSET
	{
		struct ptc* p = run_code("GPSET 0,0,1\rGPSET 8,8,2\rGPSET 16,16,3\r");
		
		ASSERT(p->res.grp[0][0] == 1, "[graphics] 0,0 pixel set to 1");
		ASSERT(p->res.grp[0][9*64] == 2, "[graphics] First pixel of small block 9 set to 2");
		ASSERT(p->res.grp[0][18*64] == 3, "[graphics] First pixel of small block 18 set to 3");
		
		free_code(p);
	}
	
	// GPSET with simple GPAGE
	{
		struct ptc* p = run_code("GPAGE 1\rGPSET 0,0,1\rGPSET 8,8,2\rGPAGE 0\rGPSET 16,16,3\r");
		
		ASSERT(p->res.grp[1][0] == 1, "[graphics] GRP1 0,0 pixel set to 1");
		ASSERT(p->res.grp[1][9*64] == 2, "[graphics] GRP1 pixel of small block 9 set to 2");
		ASSERT(p->res.grp[0][18*64] == 3, "[graphics] GRP0 pixel of small block 18 set to 3");
		
		free_code(p);
	}
	
	// GLINE tests
	{
		struct ptc* p = run_code("GLINE 0,0,7,0,1\rGLINE 0,0,7,7,2\rGLINE 0,0,0,7,3\r");
		
		u8 match[64] = {
			3,1,1,1,1,1,1,1,
			3,2,0,0,0,0,0,0,
			3,0,2,0,0,0,0,0,
			3,0,0,2,0,0,0,0,
			3,0,0,0,2,0,0,0,
			3,0,0,0,0,2,0,0,
			3,0,0,0,0,0,2,0,
			3,0,0,0,0,0,0,2,
		};
		
		for (int y = 0; y < 8; ++y){
			for (int x = 0; x < 8; ++x){
				iprintf("%d ", p->res.grp[0][x+8*y]);
				ASSERT(p->res.grp[0][x+8*y] == match[x+8*y], "[graphics] GLINE test");
			}
			iprintf("\n");
		}
		
		free_code(p);
	}
	
	// GLINE tests
	{
		struct ptc* p = run_code("GLINE 3,7,0,6,1\rGLINE 1,2,3,4,2\rGLINE 0,7,6,3,3\r");
		
		u8 match[64] = {
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,2,0,0,0,0,0,0,
			0,0,2,0,0,0,3,0,
			0,0,0,2,3,3,0,0,
			0,0,0,3,0,0,0,0,
			1,3,3,0,0,0,0,0,
			3,0,1,1,0,0,0,0,
		};
		
		for (int y = 0; y < 8; ++y){
			for (int x = 0; x < 8; ++x){
				iprintf("%d ", p->res.grp[0][x+8*y]);
				ASSERT(p->res.grp[0][x+8*y] == match[x+8*y], "[graphics] GLINE test");
			}
			iprintf("\n");
		}
		
		free_code(p);
	}
	
	SUCCESS("test_graphics success");
}
