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

// Since t is not currently used for NDS, passing NULL is OK here
void start_time(struct timers* t){
	(void)t;
#if defined(ARM9) && defined(SBC_PROFILE)
	cpuStartTiming(0);
#endif
}

void check_time(struct timers* t, int id){
	assert(t);
	assert(id >= 0 && id < PROFILER_TIMERS);
#if defined(ARM9) && defined(SBC_PROFILE)
	// average of current + prev averages
	t->profile[id] /= 2;
	t->profile[id] += cpuEndTiming() / 2;
#else
	(void)t;
	(void)id;
#endif
}
