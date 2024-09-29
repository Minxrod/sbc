#pragma once
/// 
/// @file
/// @brief Various PTC system variables.
/// 
struct sbc;

void sys_true(struct sbc* p);
void sys_false(struct sbc* p);
void sys_cancel(struct sbc* p);
void sys_version(struct sbc* p);

void sys_date(struct sbc* p);
void sys_time(struct sbc* p);
void sys_maincnth(struct sbc* p);
void sys_maincntl(struct sbc* p);

void sys_freemem(struct sbc* p);
void sys_freevar(struct sbc* p);

void sys_tchst(struct sbc* p);
void sys_tchtime(struct sbc* p);
void sys_tchx(struct sbc* p);
void sys_tchy(struct sbc* p);

void sys_funcno(struct sbc* p);
void sys_keyboard(struct sbc* p);
