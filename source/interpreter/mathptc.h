#pragma once

#include "common.h"

struct ptc;

void func_rnd(struct ptc* p);

void func_floor(struct ptc* p);

void func_pi(struct ptc* p);
void func_sin(struct ptc* p);

void func_log(struct ptc* p);

fixp func_sin_internal(fixp);
fixp func_cos_internal(fixp);
fixp func_tan_internal(fixp);
fixp func_atan_internal(fixp);
fixp func_atan2_internal(fixp, fixp);

fixp func_log_internal(fixp);
fixp func_exp_internal(fixp);
fixp func_sqr_internal(fixp);
fixp func_pow_internal(fixp);

