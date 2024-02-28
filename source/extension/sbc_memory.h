#pragma once

#include "common.h"

struct ptc;

struct memory_api {
	fixp sys_memsafe;
};

void cmd_poke(struct ptc* p);
void cmd_pokeh(struct ptc* p);
void cmd_pokeb(struct ptc* p);

void cmd_memcopy(struct ptc* p);
void cmd_memfill(struct ptc* p);

void func_peek(struct ptc* p);
void func_peekh(struct ptc* p);
void func_peekb(struct ptc* p);

void func_addr(struct ptc* p);
void func_ptr(struct ptc* p);

void sys_memsafe(struct ptc* p);
