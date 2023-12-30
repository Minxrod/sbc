#pragma once

#ifdef PC
#include <threads.h>
#include <stdatomic.h>
#endif

#include <assert.h>
#include <stdint.h>
#define PROFILER_TIMERS 20

struct timers {
#ifdef PC
	_Atomic int64_t time;
#endif
#ifndef PC // catchall for non-PC
	int64_t time;
#endif
	
	// Store performance data
	uint32_t profile[24];
};

void inc_time(struct timers* t);
int64_t get_time(struct timers* t);

// Declare these as inlines so they get properly optimized out on NDS when profiler disabled
static inline void start_time(struct timers* t);
static inline void check_time(struct timers* t, int id);

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
