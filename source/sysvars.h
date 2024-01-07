#pragma once

struct ptc;

void sys_true(struct ptc* p);
void sys_false(struct ptc* p);
void sys_cancel(struct ptc* p);
void sys_version(struct ptc* p);

void sys_date(struct ptc* p);
void sys_time(struct ptc* p);
void sys_maincnth(struct ptc* p);
void sys_maincntl(struct ptc* p);

void sys_tchst(struct ptc* p);
void sys_tchtime(struct ptc* p);
void sys_tchx(struct ptc* p);
void sys_tchy(struct ptc* p);

void sys_sphitno(struct ptc* p);
void sys_sphitx(struct ptc* p);
void sys_sphity(struct ptc* p);
void sys_sphitt(struct ptc* p);
