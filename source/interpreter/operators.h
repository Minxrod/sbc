#pragma once
/// 
/// @file
/// @brief Various PTC operator functions. Arithmetic, comparison, etc.
/// 

struct ptc;

/// Implements the addition operation of PTC.
/// 
/// Addition is a binary operator. Can be used to add two numbers or 
/// concatenate two strings.
/// 
/// On errors such as overflow, sets the system error code in `p`.
/// 
/// @param p System struct
void op_add(struct ptc* p);

/// Implements the subtraction operation of PTC.
/// 
/// Subtraction is a binary operator.
/// 
/// On errors such as overflow, sets the system error code in `p`.
/// 
/// @param p System struct
void op_sub(struct ptc* p);

/// Implements the multiplication operation of PTC.
/// 
/// Multiplication is a binary operator.
/// Can be used to add two numbers, or repeat a string some number of times.
/// 
/// On errors such as overflow, sets the system error code in `p`.
/// 
/// @param p System struct
void op_mult(struct ptc* p);

/// Implements the division operation of PTC.
/// 
/// Division is a binary operator.
/// 
/// On errors such as overflow or division by zero, 
/// sets the system error code in `p`.
/// 
/// @param p System struct
void op_div(struct ptc* p);
void op_modulo(struct ptc* p);

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

void op_and(struct ptc* p);
void op_or(struct ptc* p);
void op_xor(struct ptc* p);
void op_not(struct ptc* p);
void op_logical_not(struct ptc* p);

