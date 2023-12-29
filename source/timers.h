#pragma once

#ifdef PC
#include <threads.h>
#include <stdatomic.h>
#endif

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

void start_time(struct timers* t);
void check_time(struct timers* t, int id);
