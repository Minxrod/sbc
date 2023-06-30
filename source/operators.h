#pragma once

struct ptc;

void op_add(struct ptc* p);
void op_sub(struct ptc* p);
void op_mult(struct ptc* p);
void op_div(struct ptc* p);

void op_assign(struct ptc* p);

void op_comma(struct ptc* p);
void op_semi(struct ptc* p);
