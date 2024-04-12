#pragma once
/// 
/// @file
/// @brief Various PTC system variables.
/// 
struct ptc;

void sys_true(struct ptc* p);
void sys_false(struct ptc* p);
void sys_cancel(struct ptc* p);
void sys_version(struct ptc* p);

void sys_date(struct ptc* p);
void sys_time(struct ptc* p);
void sys_maincnth(struct ptc* p);
void sys_maincntl(struct ptc* p);

void sys_freemem(struct ptc* p);
void sys_freevar(struct ptc* p);

void sys_tchst(struct ptc* p);
void sys_tchtime(struct ptc* p);
void sys_tchx(struct ptc* p);
void sys_tchy(struct ptc* p);

void sys_funcno(struct ptc* p);
void sys_keyboard(struct ptc* p);
