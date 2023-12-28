#include "test_util.h"

#include "subsystem/graphics.h"

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
	
	SUCCESS("test_graphics success");
}
