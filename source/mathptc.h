#pragma once

#include "common.h"

struct ptc;

void func_rnd(struct ptc* p);
void func_pi(struct ptc* p);
void func_sin(struct ptc* p);

fixp func_sin_internal(fixp);
