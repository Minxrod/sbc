#include "timers.h"

#include "common.h"

void inc_time(struct timers* t){
#ifdef PC
	if (mtx_lock(&t->time_mutex) == thrd_error){
		ABORT("inc_time mutex lock failure!");
	}
#endif
	t->time++;
#ifdef PC
	if (mtx_unlock(&t->time_mutex) == thrd_error){
		ABORT("inc_time mutex unlock failure!");
	}
#endif
}

int64_t get_time(struct timers* t){
#ifdef PC
	if (mtx_lock(&t->time_mutex) == thrd_error){
		ABORT("get_time mutex lock failure!");
	}
#endif
	int64_t time = t->time;
#ifdef PC
	if (mtx_unlock(&t->time_mutex) == thrd_error){
		ABORT("get_time mutex unlock failure!");
	}
#endif
	return time;
}
