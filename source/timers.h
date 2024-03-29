#pragma once
/// 
/// @file
/// @brief Tracks the passage of time for the interpreter.
///
/// Tracks frame time for interpreter function and more precise time for 
/// profiling, if enabled.
/// 

#ifdef PC
#include <threads.h>
#include <stdatomic.h>
#endif

#include <assert.h>
#include <stdint.h>
#define PROFILER_TIMERS 24

struct timers {
#ifdef PC
	_Atomic int64_t time;
#endif
#ifndef PC // catchall for non-PC
	int64_t time;
#endif
	
	// Store performance data
	uint32_t profile[PROFILER_TIMERS];
};

void inc_time(struct timers* t);
int64_t get_time(struct timers* t);

#ifdef ARM9
#include <nds/timers.h>
#endif


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
