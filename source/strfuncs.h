#pragma once

struct ptc;

void func_len(struct ptc* p);
void func_mid(struct ptc* p);
void func_val(struct ptc* p);
void func_instr(struct ptc* p);
void func_chr(struct ptc* p);
void func_subst(struct ptc* p);
void func_asc(struct ptc* p);

void cmd_dtread(struct ptc* p);
void cmd_tmread(struct ptc* p);
