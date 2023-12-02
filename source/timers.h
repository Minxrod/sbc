#pragma once

#include <threads.h>
#include <stdatomic.h>
#include <stdint.h>

struct timers {
	int64_t time;
	
#ifdef PC
	mtx_t time_mutex;
#endif
	
};

void inc_time(struct timers* t);
int64_t get_time(struct timers* t);
