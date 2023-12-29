#include "test_util.h"

#include "subsystem/sprites.h"
#include "graphics/nds/display_nds.h" // Warning: Defines display, which screws up if you try to include system

int test_sprite(){
	// Test sprite collision
	{
		struct sprite_info a = init_sprite_info(0,0,0,0,0,0,16,16);
		struct sprite_info b = init_sprite_info(1,0,0,0,0,0,16,16);
		
		ASSERT(is_hit(&a, &b), "[col] Default sprite collision");
		
		a.pos.x = INT_TO_FP(20);
		
		DENY(is_hit(&a, &b), "[col] Default sprite no collision");
	}
	
	// Test sprite size calculation is correct
	{
		// Square sprites
		ASSERT(spr_calc_size(0, 8, 8) == 0, "[sprite] 8x8 size");
		ASSERT(spr_calc_size(0, 16, 16) == 1, "[sprite] 16x16 size");
		ASSERT(spr_calc_size(0, 32, 32) == 2, "[sprite] 32x32 size");
		ASSERT(spr_calc_size(0, 64, 64) == 3, "[sprite] 64x64 size");
		
		// Wide spirtes
		ASSERT(spr_calc_size(1, 16, 8) == 0, "[sprite] 16x8 size");
		ASSERT(spr_calc_size(1, 32, 8) == 1, "[sprite] 32x8 size");
		ASSERT(spr_calc_size(1, 32, 16) == 2, "[sprite] 32x16 size");
		ASSERT(spr_calc_size(1, 64, 32) == 3, "[sprite] 64x32 size");
		
		// Tall sprites
		ASSERT(spr_calc_size(2, 8, 16) == 0, "[sprite] 8x16 size");
		ASSERT(spr_calc_size(2, 8, 32) == 1, "[sprite] 8x32 size");
		ASSERT(spr_calc_size(2, 16, 32) == 2, "[sprite] 16x32 size");
		ASSERT(spr_calc_size(2, 32, 64) == 3, "[sprite] 32x64 size");
	}
	
	SUCCESS("test_sprite success");
}
