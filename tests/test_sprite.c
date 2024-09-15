#include "test_util.h"

#include "subsystem/sprites.h"
#include "system.h" // includes relevant display
#ifdef PC
#include "graphics/nds/display_nds.h" // Does not include display struct due to extra header guards
#endif

struct ptc;

int test_sprite(){
	// Test sprite collision
	{
		struct sprite_info a = init_sprite_info(0,0,0,0,0,0,16,16);
		struct sprite_info b = init_sprite_info(1,0,0,0,0,0,16,16);
		
		ASSERT(is_hit(&a, &b), "[col] Default sprite collision");
		
		a.pos.x = INT_TO_FP(20);
		DENY(is_hit(&a, &b), "[col] Default sprite no collision");

		b.pos.y = INT_TO_FP(20);
		DENY(is_hit(&a, &b), "[col] Default sprite no collision 2");
		
		a.pos.x = INT_TO_FP(0);
		b.pos.y = INT_TO_FP(20);
		DENY(is_hit(&a, &b), "[col] Default sprite no collision 3");

		a.hit.dx = INT_TO_FP(-20);
		a.hit.dy = INT_TO_FP(20);
		
		ASSERT(is_hit(&a, &b), "[col] Sprite colvec collision");
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
	
	// Simple character test
	{
		struct ptc* p = run_code(
			"SPSET 0,0,0,0,0,0\r"
			"SPSET 1,1,1,1,1,1\r"
		);
		
		ASSERT(p->sprites.info[0][0].chr == 0, "[sprite] Sprite 0 chr = 0");
		ASSERT(p->sprites.info[0][1].chr == 1, "[sprite] Sprite 1 chr = 1");
		ASSERT(get_sprite_chr(&p->sprites.info[0][0]) == 0, "[sprite] Sprite 0 get chr = 0");
		ASSERT(get_sprite_chr(&p->sprites.info[0][1]) == 1, "[sprite] Sprite 1 get chr = 1");
		
		free_code(p);
	}
	
	// Simple animation test
	{
		struct ptc* p = run_code(
			"SPSET 0,0,0,0,0,0\rSPANIM 0,4,1\r"
		);
		
		ASSERT(p->sprites.info[0][0].chr == 0, "[sprite] Sprite 0 chr 1 = 0");
		ASSERT(get_sprite_chr(&p->sprites.info[0][0]) == 0, "[sprite] Sprite 0 get chr 1");
		step_sprites(&p->sprites);
		ASSERT(p->sprites.info[0][0].chr == 0, "[sprite] Sprite 0 chr 2 = 0");
		ASSERT(get_sprite_chr(&p->sprites.info[0][0]) == 1, "[sprite] Sprite 0 get chr 2");
		step_sprites(&p->sprites);
		ASSERT(p->sprites.info[0][0].chr == 0, "[sprite] Sprite 0 chr 3 = 0");
		ASSERT(get_sprite_chr(&p->sprites.info[0][0]) == 2, "[sprite] Sprite 0 get chr 3");
		step_sprites(&p->sprites);
		ASSERT(p->sprites.info[0][0].chr == 0, "[sprite] Sprite 0 chr 4 = 0");
		ASSERT(get_sprite_chr(&p->sprites.info[0][0]) == 3, "[sprite] Sprite 0 get chr 4");
		step_sprites(&p->sprites);
		// loop around
		ASSERT(p->sprites.info[0][0].chr == 0, "[sprite] Sprite 0 chr 4 = 0");
		ASSERT(get_sprite_chr(&p->sprites.info[0][0]) == 0, "[sprite] Sprite 0 get chr 5 == 1");
		
		free_code(p);
	}
	
	// Simple animation test II
	{
		struct ptc* p = run_code(
			"SPSET 0,64,0,0,0,0\rSPANIM 0,4,5\r"
		);
		
		int chrs[] = {
			64,64,64,64,64,
			65,65,65,65,65,
			66,66,66,66,66,
			67,67,67,67,67,
			64,64,64,64,64,
			65,65,65,65,65,
		};
		
		ASSERT(p->sprites.info[0][0].chr == 64, "[sprite] Sprite 0 chr = 64");
		for (size_t i = 0; i < sizeof(chrs)/sizeof(chrs[0]); ++i){
			ASSERT(get_sprite_chr(&p->sprites.info[0][0]) == chrs[i], "[sprite] Sprite 0 get chr 1");
			step_sprites(&p->sprites);
		}
		
		free_code(p);
	}
	
	// Simple animation test III
	{
		struct ptc* p = run_code(
			"SPSET 0,64,0,0,0,0\rSPANIM 0,4,2,1\r"
		);
		
		int chrs[] = {
			64,64,
			65,65,
			66,66,
			67,67,
			67,67,
			67,67,
		};
		
		ASSERT(p->sprites.info[0][0].chr == 64, "[sprite] Sprite 0 chr 64");
		for (size_t i = 0; i < sizeof(chrs)/sizeof(chrs[0]); ++i){
			ASSERT(get_sprite_chr(&p->sprites.info[0][0]) == chrs[i], "[sprite] Sprite 0 get chr single cycle anim");
			step_sprites(&p->sprites);
		}
		
		free_code(p);
	}
	
	// Simple interpolated movement test
	{
		struct ptc* p = run_code(
			"SPSET 0,64,0,0,0,0\rSPOFS 0,64,0,4\r"
		);
		
		int xs[] = {
			0,16,32,48,64
		};
		
		for (size_t i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i){
			ASSERT(p->sprites.info[0][0].pos.x == INT_TO_FP(xs[i]), "[sprite] Sprite interpolated position");
			step_sprites(&p->sprites);
		}
		
		free_code(p);
	}
	
	SUCCESS("test_sprite success");
}
