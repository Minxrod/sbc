#include "timers.h"

#include "common.h"

#include <assert.h>

void inc_time(struct timers* t){
#ifdef PC
	atomic_fetch_add(&t->time, 1);
#endif
#ifndef PC
	t->time++;
#endif
}

int64_t get_time(struct timers* t){
#ifdef PC
	return atomic_load(&t->time);
#endif
#ifndef PC
	return t->time;
#endif
}
