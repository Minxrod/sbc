#pragma once
/// 
/// @file
/// @brief Various PTC string functions.
/// 

struct sbc;

void func_len(struct sbc* p);
void func_mid(struct sbc* p);
void func_val(struct sbc* p);
void func_instr(struct sbc* p);
void func_chr(struct sbc* p);
void func_subst(struct sbc* p);
void func_asc(struct sbc* p);
void func_str(struct sbc* p);
void func_right(struct sbc* p);
void func_left(struct sbc* p);
void func_hex(struct sbc* p);

void cmd_dtread(struct sbc* p);
void cmd_tmread(struct sbc* p);
