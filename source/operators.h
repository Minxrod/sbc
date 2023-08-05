#pragma once

struct ptc;

void op_add(struct ptc* p);
void op_sub(struct ptc* p);
void op_mult(struct ptc* p);
void op_div(struct ptc* p);

void op_assign(struct ptc* p);

void op_comma(struct ptc* p);
void op_semi(struct ptc* p);

void op_negate(struct ptc* p);

void op_equal(struct ptc* p);
void op_inequal(struct ptc* p);
void op_less(struct ptc* p);
void op_greater(struct ptc* p);
void op_less_equal(struct ptc* p);
void op_greater_equal(struct ptc* p);
