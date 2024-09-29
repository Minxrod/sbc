#pragma once
/// 
/// @file
/// @brief Various PTC mathematical functions.
/// 
#include "common.h"

struct sbc;

void func_rnd(struct sbc* p);

void func_floor(struct sbc* p);
void func_sgn(struct sbc* p);
void func_abs(struct sbc* p);

void func_pi(struct sbc* p);
void func_sin(struct sbc* p);
void func_cos(struct sbc* p);
void func_tan(struct sbc* p);
void func_atan(struct sbc* p);

void func_log(struct sbc* p);
void func_pow(struct sbc* p);
void func_sqr(struct sbc* p);
void func_exp(struct sbc* p);

void func_rad(struct sbc* p);
void func_deg(struct sbc* p);

fixp func_sin_internal(fixp);
fixp func_cos_internal(fixp);
fixp func_tan_internal(fixp);
fixp func_atan_internal(fixp);
fixp func_atan2_internal(fixp, fixp);

fixp func_log_internal(fixp);
fixp func_exp_internal(fixp);
fixp func_sqr_internal(fixp);
fixp func_pow_internal(fixp, fixp);

fixp func_rad_internal(fixp);
fixp func_deg_internal(fixp);

