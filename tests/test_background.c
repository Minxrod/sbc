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
	
	
	SUCCESS("test_background success");
}
