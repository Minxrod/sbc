#include "test_util.h"

#include "subsystem/sprites.h"

int test_sprite(){
	// Test sprite collision
	{
		struct sprite_info a = init_sprite_info(0,0,0,0,0,0,16,16);
		struct sprite_info b = init_sprite_info(1,0,0,0,0,0,16,16);
		
		ASSERT(is_hit(&a, &b), "[col] Default sprite collision");
		
		a.pos.x = 20;
		
		ASSERT(is_hit(&a, &b), "[col] Default sprite no collision");
	}
	
	SUCCESS("test_sprite success");
}
