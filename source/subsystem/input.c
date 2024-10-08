#include "input.h"

#include "common.h"
#include "system.h"
#include "error.h"
#include "stack.h"
#include "interpreter/strs.h"

void init_input(struct input* i){
	i->button = 0;
	i->old_button = 0;
	for (int j = 0; j < BUTTON_COUNT; ++j){
		i->times[j] = (struct button_time){0, 0, 0};
	}
#ifdef PC
	int status = mtx_init(&i->inkey_mtx, mtx_plain);
	if(status != thrd_success){
		ABORT("Error creating inkey_mtx mutex");
	}
	status = mtx_init(&i->touch_mtx, mtx_plain);
	if(status != thrd_success){
		ABORT("Error creating inkey_mtx mutex");
	}
#endif
	i->current_write = 0;
	i->current_base = 0;
}

void set_input(struct ptc* p, int b) {
	struct input* i = &p->input;
	i->old_button = i->button;
	i->button = b;
	
	for (int j = 0; j < BUTTON_COUNT; ++j){
		if (b & (1 << j)){
			i->times[j].frame++;
		} else {
			i->times[j].frame = 0;
		}
	}
	// check for SELECT break here
	// TODO:CODE:LOW is this thread-safe?
	// writing to p->exec.error: may be destroyed by a few specific commands?
	// no issues yet during testing
	if (i->times[BUTTON_ID_SELECT].frame == 1){
		p->exec.error = ERR_BREAK;
	}
}

bool set_inkey(struct input* i, u16 c){
	bool status = false;
#ifdef PC
	if (mtx_lock(&i->inkey_mtx) == thrd_error){
		ABORT("set_inkey mutex lock failure!");
	}
#endif
	if (i->current_write == INKEY_BUF_SIZE){
		//wait for more slots
	} else {
		u16 cur = (i->current_base + i->current_write) % INKEY_BUF_SIZE;
		i->current_write++;
		i->inkey_buf[cur] = c;
		status = true;
	}
#ifdef PC
	if (mtx_unlock(&i->inkey_mtx) == thrd_error){
		ABORT("set_inkey mutex unlock failure!");
	}
#endif
	return status;
}

u16 get_inkey(struct input* i){
#ifdef PC
	if (mtx_lock(&i->inkey_mtx) == thrd_error){
		ABORT("get_inkey mutex lock failure!");
	}
#endif
	u16 c;
	if (i->current_write == 0){
		c = 0;
	} else {
		c = i->inkey_buf[i->current_base];
		i->current_write--;
		i->current_base = (i->current_base + 1) % INKEY_BUF_SIZE;
	}
#ifdef PC
	if (mtx_unlock(&i->inkey_mtx) == thrd_error){
		ABORT("get_inkey mutex unlock failure!");
	}
#endif
	return c;
}

// Should also set keyboard, inkey!
void set_touch(struct input* i, bool t, u8 x, u8 y){
#ifdef PC
	if (mtx_lock(&i->touch_mtx) == thrd_error){
		ABORT("set_touch mutex lock failure!");
	}
#endif
	if (t){
		i->tchtime++;
		i->tchx = x;
		i->tchy = y;
	} else {
		i->tchtime = 0;
	}
#ifdef PC
	if (mtx_unlock(&i->touch_mtx) == thrd_error){
		ABORT("set_touch mutex unlock failure!");
	}
#endif
}

void set_repeat(struct input* i, int button, int start, int repeat){
	i->times[button].start = start;
	i->times[button].repeat = repeat;
}

bool check_pressed_manual(struct input* i, int id, int start, int repeat){
	int time = i->times[id].frame;
	return check_repeat(time, start, repeat);
}

bool check_pressed(struct input* i, int id){
	return check_repeat(i->times[id].frame, i->times[id].start, i->times[id].repeat);
}

void cmd_brepeat(struct ptc* p){
	// BREPEAT id[,start,repeat]
	// https://petitcomputer.fandom.com/wiki/BREPEAT_(Command)
	// Apparently 12 is a valid ID, even if it doesn't seem to do anything?
	int id;
	STACK_INT_RANGE(0,0,12,id);
	//TODO:IMPL:NONE Figure out what this does and add it maybe?
	if (id == 12) return;
	if (p->stack.stack_i == 1){
		p->input.times[id].start = 0;
		p->input.times[id].repeat = 0;
	} else {
		STACK_INT_MIN(1,0,p->input.times[id].start);
		STACK_INT_MIN(2,0,p->input.times[id].repeat);
	}
}

void func_btrig(struct ptc* p){
	struct input* i = &p->input;
	
	s32 b = 0;
	for (int j = 0; j < BUTTON_COUNT; ++j){
		b |= check_pressed(i, j) << j;
	}
//	iprintf("button: %d\n", (int)b);
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(b)}});
}

void func_button(struct ptc* p){
	struct input* i = &p->input;
	int mode = 0;
	if (p->exec.argcount == 1){
		struct stack_entry* e = stack_pop(&p->stack);
		mode = VALUE_INT(e);
	}
	int button;
	
	switch (mode){
		case 0:
			button = i->button;
			break;
		
		case 1:
			button = 0;
			for (int j = 0; j < BUTTON_COUNT; ++j){
				button |= check_pressed(i, j) << j;
			}
			break;
		
		case 2:
			button = i->button & ~i->old_button;
			break;
		
		case 3:
			button = ~i->button & i->old_button;
			break;
		
		default:
			ERROR(ERR_INVALID_ARGUMENT_VALUE);
	}
	
	stack_push(&p->stack, (struct stack_entry){VAR_NUMBER, {INT_TO_FP(button)}});
}

void func_inkey(struct ptc* p){
	struct input* i = &p->input;
	if (p->exec.argcount){
		p->exec.error = ERR_WRONG_ARG_COUNT;
		return;
	}
	u16 inkey = get_inkey(i);
	if (inkey){
		//TODO:CODE:LOW Doesn't handle wide characters
		stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = &single_char_strs[3 * to_char(inkey)]}});
	} else {
		stack_push(&p->stack, (struct stack_entry){VAR_STRING, {.ptr = empty_str}});
	}
}
