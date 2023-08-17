#include "input.h"

//#include "common.h"
#include "test_util.h"

int test_input(void){
	{
		struct input in = {0};
		
		ASSERT(in.button == 0, "[input] No button starts pressed");
		ASSERT(in.old_button == 0, "[input] No button starts pressed");
	}
	
	
	// Set single frame
	{
		struct input in = {0};
		
		set_input(&in, BUTTON_A);
		ASSERT(in.button == BUTTON_A, "[input] Button state is saved");
		
		ASSERT(in.times[BUTTON_ID_A].frame == 1, "[input] Pressed for one frame");
	}
	
	// Set two frames
	{
		struct input in = {0};
		
		set_input(&in, BUTTON_A);
		set_input(&in, BUTTON_A | BUTTON_B);
		
		ASSERT(in.button == (BUTTON_A | BUTTON_B), "[input] Button state is saved");
		ASSERT(in.old_button == BUTTON_A, "[input] Previous frame's state is saved");
		
		ASSERT(in.times[BUTTON_ID_A].frame == 2, "[input] A pressed for two frames");
		ASSERT(in.times[BUTTON_ID_B].frame == 1, "[input] B pressed for one frame");
	}
	
	// Check repeat
	{
		struct input in = {0};
		
		set_repeat(&in, BUTTON_ID_X, 0, 2);

		ASSERT(in.button == 0, "[input] No button is pressed");
		ASSERT(in.times[BUTTON_ID_X].frame == 0, "[input] X not pressed");
		DENY(check_pressed(&in, BUTTON_ID_X), "[input] X is not pressed (repeat)");
		
		set_input(&in, BUTTON_X);
		ASSERT(in.button == BUTTON_X, "[input] Button X is pressed");
		ASSERT(in.times[BUTTON_ID_X].frame == 1, "[input] X pressed for 1 frame");
		ASSERT(check_pressed(&in, BUTTON_ID_X), "[input] X is pressed via repeat rules on frame 1");
		
		set_input(&in, BUTTON_X);
		ASSERT(in.button == BUTTON_X, "[input] Button X is held (2)");
		ASSERT(in.times[BUTTON_ID_X].frame == 2, "[input] X pressed for 2 frames");
		DENY(check_pressed(&in, BUTTON_ID_X), "[input] X is NOT pressed via repeat rules on frame 2");
		
		set_input(&in, BUTTON_X);
		ASSERT(in.button == BUTTON_X, "[input] Button X is held (3)");
		ASSERT(in.times[BUTTON_ID_X].frame == 3, "[input] X pressed for 3 frames");
		DENY(check_pressed(&in, BUTTON_ID_X), "[input] X is NOT pressed via repeat rules on frame 3");
		
		set_input(&in, BUTTON_X);
		ASSERT(in.button == BUTTON_X, "[input] Button X is held (4)");
		ASSERT(in.times[BUTTON_ID_X].frame == 4, "[input] X pressed for 4 frames");
		ASSERT(check_pressed(&in, BUTTON_ID_X), "[input] X is pressed via repeat rules on frame 4");
	}
	
	// Check set_inkey, get_inkey
	{
		struct input in = {0};
		
		ASSERT(get_inkey(&in) == 0, "[input] Buffer is empty");
		
		set_inkey(&in, 'A');
		set_inkey(&in, 'B');
		set_inkey(&in, 'C');
		
		ASSERT(get_inkey(&in) == 'A', "[input] Get A from inkey buffer");
		ASSERT(get_inkey(&in) == 'B', "[input] Get B from inkey buffer");
		ASSERT(get_inkey(&in) == 'C', "[input] Get C from inkey buffer");
		
		ASSERT(get_inkey(&in) == 0, "[input] Buffer is empty");
	}
	
	// Check inkey buffer overflow behavior (discard new chars)
	{
		struct input in = {0};
		
		for (int i = 0; i < INKEY_BUF_SIZE; ++i)
			set_inkey(&in, 'D');
		set_inkey(&in, 'E');
		
		for (int i = 0; i < INKEY_BUF_SIZE; ++i){
			ASSERT(get_inkey(&in) == 'D', "[input] Get D from full buffer");
		}
		ASSERT(get_inkey(&in) == 0, "[input] Buffer is now empty");
	}
	
	SUCCESS("test_input successful");
}
