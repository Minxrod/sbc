#include "input.h"


void init_input(struct input* i){
	i->button = 0;
	i->old_button = 0;
	for (int j = 0; j < BUTTON_COUNT; ++j){
		i->times[j] = (struct button_time){0, 0, 0};
	}
}

void set_input(struct input* i, int b) {
	i->old_button = i->button;
	i->button = b;
	
	for (int j = 0; j < BUTTON_COUNT; ++j){
		if (b & (1 << j)){
			i->times[j].frame++;
		} else {
			i->times[j].frame = 0;
		}
	}
}

void set_repeat(struct input* i, int button, int start, int repeat){
	i->times[button].start = start;
	i->times[button].repeat = repeat;
}

bool check_pressed(struct input* i, int id){
	int time = i->times[id].frame;
	int start = i->times[id].start;
	int repeat = i->times[id].repeat;
	if (time == 1) return true;
	if (repeat > 0){ //repeat=0 -> disabled
		if (time > start){ //repeat only applies past start time
			return ((time - 1 - start) % (repeat + 1) == 0);
		}
	}
	return false;
}
