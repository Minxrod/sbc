#include "input.h"

#include "common.h"
#include "system.h"
#include "error.h"

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

void set_inkey(struct input* i, u8 c){
	// TODO: Does this take wide chars or only small chars?
	i->inkey_buf[i->current_write++] = c;
}

u8 get_inkey(struct input* i){
	// TODO mutex/flag with set_inkey(?)
	if (i->current_read >= i->current_write){
		return 0;
	}
	return i->inkey_buf[i->current_read++];
}

// Should also set keyboard, inkey!
void set_touch(struct input* i, bool t, u8 x, u8 y){
	// TODO: Does this one need a flag?
	if (t){
		i->tchtime++;
		i->tchx = x;
		i->tchy = y;
	} else {
		i->tchtime = 0;
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

void func_btrig(struct ptc* p){
	struct input* i = &p->input;
	if (p->exec.argcount){
		p->exec.error = ERR_WRONG_ARG_COUNT;
		return;
	}
	s32 b = 0;
	for (int j = 0; j < BUTTON_COUNT; ++j){
		b |= check_pressed(i, j) << j;
	}
//	iprintf("button: %d\n", (int)b);
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {b << 12}});
}
